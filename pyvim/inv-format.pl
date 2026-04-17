#!/usr/bin/env perl

# ----------------------------------------------------------------------
#
# inv-format
#
# Take inventory file and format it to either Excel or Markdown

# 2025-09-08: Ross Alexander
#   Minor changes to match vc-inv.py schema.

# 2023-09-15: Ross Alexander
#   Fix server report and add version

# 2023-09-05: Ross Alexander
#   Fix portgroup processing and add to NIC text

# 2022-06-24: Ross Alexander
#   Remove site and add host to excel, fix ip address issue

# 2022-06-21: Ross Alexander
#   Add simple server markdown table output

# 2022-01-27: Ross Alexander
#   Change Excel to create a single file with multiple tabs

# 2018-07-04: ralexand
#  Allow sites to be specified

# 2016-09-13: ralexand
#  Update for latest JSON schema

#
# ----------------------------------------------------------------------

use v5.32.1;
use File::Slurp;
use File::Spec::Functions;
use File::Path;
use Getopt::Long;
use Excel::Writer::XLSX;
use NetAddr::IP;
use Number::Bytes::Human qw(format_bytes parse_bytes);
use List::Util qw(sum);
use Time::Piece;
use Time::Seconds;
use JSON;
use utf8;
use locale;

# ----------------------------------------------------------------------
#
# host_descent
#
# ----------------------------------------------------------------------

sub host_descent {
    my ($tree, $path) = @_;

    my @p = (@{$path}, $tree->{name});
    $tree->{path} = join("/", @$path);

    return ($tree->{folder} || $tree->{hostsystem} || $tree->{cluster}) ? [ map { @{host_descent($_, \@p)}; } (@{$tree->{folder}}, @{$tree->{hostsystem}}, @{$tree->{cluster}}) ]
	: (($tree->{type} eq "HostSystem" || $tree->{type} eq "host") ? [$tree] : []);
}

# ----------------------------------------------------------------------
#
# vm_descent (into madness)
#
# ----------------------------------------------------------------------

sub vm_descent {
    my ($tree, $path) = @_;

    my @p = (@$path, $tree->{name});
    $tree->{path} = join("/", @$path);

    return [ map {@{vm_descent($_, \@p)}; } (@{$tree->{folder}}, @{$tree->{virtualmachine}}) ] if (($tree->{folder} || $tree->{virtualmachine}));
    return $tree->{type} eq "Folder" ? [] : [$tree];
}


# ----------------------------------------------------------------------
#
# pg_descent
#
# ----------------------------------------------------------------------

sub pg_descent {
    my ($tree, $path) = @_;

    my $res = exists($tree->{dvportgroup}) ? $tree->{dvportgroup} : [];
    $res = [@$res, map {@{pg_descent($_, $path)};} (@{$tree->{folder}})] if (($tree->{folder}));
    return $res;
}

# ----------------------------------------------------------------------
#
# vm_table
#
# ----------------------------------------------------------------------

sub vm_table {
    my ($conf, $lists) = @_;

    # --------------------
    # map customfields from key to name
    # --------------------
    
    my $key_map = {};
    while (my ($k, $v) = each(%{$conf->{customfield}}))
    {
	$key_map->{$v->{key}} = $v->{name};
    }

    # --------------------
    # Obsolete field as there are no longer any multiclient environments
    # --------------------
        
    my $clientfield = $conf->{customfield}->{Client}->{key};

    if (!$lists->{virtualmachines})
    {
	printf(STDERR "vm_table requires list of virtual machines");
	exit(1);
    }
    my $vm_list = $lists->{virtualmachines};

    if (!$lists->{hosts})
    {
	printf(STDERR "vm_table requires list of hosts");
	exit(1);
    }
    my $host_list = $lists->{hosts};

    # --------------------
    # Map portgroups
    # --------------------

    my $portgroups = {};
    for my $pg (@{$lists->{portgroups} || []})
    {
	$portgroups->{$pg->{ref}} = $pg->{name};
    }

    # --------------------
    # Define IPv4 LinkLocal address
    # --------------------

    my $ll_na = NetAddr::IP->new('169.254.0.0/16');

    # --------------------
    # Create index of host references
    # --------------------

    my $host_ref = {};
    for my $h (@$host_list)
    {
	$host_ref->{$h->{ref}} = $h;
    }

    my @res;

    for my $i (sort({$a->{name} cmp $b->{name}} @$vm_list))
    {
	my $name = $i->{name};
        my $client = $i->{custom}->{$clientfield} || "ZZZ";
	my $fqdn = $i->{hostname};
	my @parts = split(m:\.:, $fqdn);
	my @ipv4;
	my @ipv6;

	# --------------------
	# host
	# --------------------
	
	my $host = $host_ref->{$i->{host}};

	# --------------------
	# guest networks
	# --------------------

	for my $n (@{$i->{guest}->{net} || []})
	{
	    if ($n->{connected})
	    {
		for my $ip (@{$n->{ip}})
		{
		    my $na = NetAddr::IP->new($ip->{ip}, $ip->{prefix});
		    push(@ipv4, $na->cidr()) if (defined($na) && ($na->version() == 4));
		}
	    }
	}

	# --------------------
	# guest disks
	# --------------------

	my $disks = join(" ", map {
	    sprintf("%s (%s / %s)", $_->{path}, format_bytes($_->{free}), format_bytes($_->{size}));
			 } @{$i->{guest}->{disks}});

	# --------------------
	# VirtualDisks and RDMs
	# --------------------
	
	my $vmdks = join(", ", map {
	    $_->{rdm} ? sprintf("RDM %s %s", $_->{summary}, $_->{file}) : sprintf("%s %s", $_->{summary}, $_->{file});
			 } @{$i->{devices}->{disks}});
	
	my $kb = sum(map { $_->{kb} } @{$i->{devices}->{disks}});
	
	# --------------------
	# NIC devices
	# --------------------

	my $nics = join(", ", map {
	    my $pg = exists($_->{portgroup}) ? (exists($portgroups->{$_->{portgroup}}) ? $portgroups->{$_->{portgroup}} : $_->{portgroup}) : "";
	    sprintf("%s <%s, %s> %s",$_->{address}, $_->{connected} ? "connected" : "disconnected", $_->{start} ? "onstart" : "no start", $pg);
			} (@{$i->{devices}->{nics}}));
	
	# --------------------
	# Fill in table
	# --------------------

	my $entry = {
	    path => $i->{path},
	    name => $i->{name},
	    state => $i->{state},
	    client => $client,
	    fqdn => $fqdn,
	    os => $i->{guest}->{os},
	    cpu => sprintf("%d × %d", int($i->{cpu} / $i->{cores}), $i->{cores}),
	    mem => int($i->{memory}),
	    ipv4 => join(" ", @ipv4),
	    host => $host->{name},
	    disks => $disks,
	    vmdks => $vmdks,
	    storage => $kb,
	    nics => $nics,
	    tools_version => $i->{guest}->{tools_version},
	    tools_status => $i->{guest}->{tools_status},
	};

	# --------------------
	# Add custom fields
	# --------------------
	
	if ($i->{custom})
	{
	    while (my ($k, $v) = each(%{$i->{custom}}))
	    {
		$entry->{tag}->{$key_map->{$k}} = $v;
	    }
	}
	push(@res, $entry);
    }
    return \@res;
}

# ----------------------------------------------------------------------
#
# vm_report_excel
#
# ----------------------------------------------------------------------

sub vm_report_excel {
    my ($conf, $dc, $lists) = @_;

    # --------------------
    # Get list of VMs
    # --------------------
    
    my @res = @{&vm_table($conf, $lists)};

    # --------------------
    # Sort into clients
    # --------------------

    my %clients;
    map {
        push(@{$clients{$_->{client}}}, $_);
    } @res;

    if (!-d $conf->{excel_dir})
    {
	mkdir($conf->{excel_dir}) || die;
    }
    
    my $file = catfile($conf->{excel_dir}, sprintf("%s-%s.xlsx", $dc->{name}, Time::Piece->new($conf->{meta}->{epoch})->strftime("%Y%m%d")));

    # --------------------
    # Create Excel workbook
    # --------------------
    
    my $wb = Excel::Writer::XLSX->new($file);

    # --------------------
    # Add fix for PSC/psc
    # --------------------

    my $client_lc_map = {};

    my $formats = {
	body => $wb->add_format(
	    border => 0,
	    #	bg_color => 42,
	    size => 9,
	    text_wrap => 1,
	    font => 'Aptos Mono',
	    align => 'top'
	    ),
	heading => $wb->add_format(
	    border => 0,
	    #	bg_color => 'orange',
	    bold => 1,
	    size => 10,
	    text_wrap => 0,
	    font => 'Aptos Mono',
	    align => 'top',
	    )
    };

    # --------------------
    # Create new worksheet per client
    # --------------------

    my $schema = [
	{ title => 'VM Name',          width => 24, field => 'name', },
	{ title => 'State',            width => 10, field => 'state' },
	{ title => 'Hostname',         width => 36, field => 'fqdn'  },
	{ title => 'Operating System', width => 45, field => 'os' },
	{ title => 'IPv4 Address',     width => 16, field => 'ipv4' },
	{ title => 'CPU',              width => 10, field => 'cpu' },
	{ title => 'Memory (MB)',      width => 15, field => 'mem' },
	{ title => 'Disks (OS)',       width => 50, field => 'disks' },
	{ title => 'Disks (VMDK)',     width => 50, field => 'vmdks' },
	{ title => 'Host',             width => 40, field => 'host' },
	{ title => 'Path',             width => 40, field => 'path' },
	];

   
#    $ws->write(0, 0, ['VM Name', 'State', 'Hostname', 'Operating System', 'IP Addresses', 'CPU', 'Memory (MB)', 'Disks (OS)', 'VMDKs', 'Host', 'Path', 'Tools Version', 'Tools Status'], $heading);

    
    for my $client (keys(%clients))
    {
	my $client_lc = lc($client);
	my $wb_name = $client;
	if ($client_lc_map->{$client_lc})
	{
	    $wb_name = sprintf("%s-%d", $client, ++$client_lc_map->{$client_lc});
	}
	else
	{
	    $client_lc_map->{$client_lc} = 1;
	}

	# --------------------
	# Add new worksheet
	# --------------------
	
        my $ws = $wb->add_worksheet($wb_name);

	# --------------------
	# Set column widths
	# --------------------

	for my $i (0 .. scalar(@$schema))
	{
	    $ws->set_column($i, $i, $schema->[$i]->{width});
	}

	# --------------------
	# Add headings
	# --------------------
	
	my @titles = map {$_->{title}} @$schema;
	
        $ws->write(0, 0, \@titles, $formats->{heading});

	# --------------------
	# Add body rows
	# --------------------
	
        for (my $row = 0; $clients{$client}->[$row]; $row++)
        {
            my $t = $clients{$client}->[$row];
	    my @line = map { $t->{$_->{field}} } @$schema;
	    $ws->write($row+1, 0, \@line, $formats->{body});
	}
    }
    return [$file];
}


# ----------------------------------------------------------------------
#
# markdown
#
# ----------------------------------------------------------------------

sub markdown {
    my ($conf, $dc, $lists) = @_;

    # --------------------
    # Get table (list of hashes) of the virtual machines
    # --------------------
    
    my @res = @{&vm_table($conf, $lists)};

    # --------------------
    # Sort by name and add to client specific lists
    # --------------------
    
    my $vms_per_client = {};
    for my $i (sort({$a->{name} cmp $b->{name}} @res))
    {
	my $name = $i->{name};
        my $client = $i->{client} || "ZZZ";
	push(@{$vms_per_client->{$client}}, $i);
    }

    my $schema = [
	{ title => 'VM Name',          width => 24, field => 'name',   },
	{ title => 'FQDN',             width => 36, field => 'fqdn'    },
	{ title => 'State',            width => 10, field => 'state'   },
	{ title => 'Operating System', width => 45, field => 'os'      },
	{ title => 'IPv4 Address',     width => 16, field => 'ipv4'    },
	{ title => 'NICs',             width => 20, field => 'nics'    },
	{ title => 'Host',             width => 20, field => 'host'    },
	{ title => 'CPU',              width => 10, field => 'cpu'     },
	{ title => 'RAM (MB)',         width => 10, field => 'mem'     },
	{ title => 'Disks (Guest)',    width => 50, field => 'disks'   },
	{ title => 'Disks (VMDK)',     width => 50, field => 'vmdks'   },
	{ title => 'Storage (KB)',     width => 15, field => 'storage' },
	];

    my @filenames;

    # --------------------
    # Loop over each client
    # --------------------
    
    for my $k (keys(%$vms_per_client))
    {
	my @lines;

	push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", $_->{title})} @$schema ]);
	push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", "-" x $w)} @$schema ]);
	
	for my $v (@{$vms_per_client->{$k}})
	{
	    push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", $v->{$_->{field}})} @$schema ]);
	}

	# --------------------
	# Construct path and create with mkpath
	# --------------------
	
	my $path = catdir($conf->{md_dir}, $dc->{name}, Time::Piece->new($conf->{dc}->{epoch})->strftime("%Y%m%d"));
	mkpath($path);
	
	my $filename = catfile($path, "$k.md");
	push(@filenames, $filename);

	# --------------------
	# Open output with utf8, although it unlikely any non ASCII characters will be present
	# --------------------

	my $stream;
	open($stream, ">:utf8", $filename);
	map { printf($stream "%s\n", join(" | ", @$_)); } @lines;
	close($stream);
    }
    return \@filenames;
}

# ----------------------------------------------------------------------
#
# servers_report_md
#
# ----------------------------------------------------------------------

sub servers_report_md {
    my ($conf, $dc, $lists) = @_;

    # --------------------
    # Construct list of hashes
    # --------------------

    my @res = map {
	+{
	    name => $_->{name},
	    model => $_->{model},
	    tag => $_->{servicetag},
	    memory => int($_->{memory} / 1024 / 1024 / 1024),
	    cores => $_->{cores},
	    threads => $_->{threads},
	    sockets => $_->{packages},
	    cpu => $_->{cpu}->[0]->{description},
	    path => $_->{path},
	    version => $_->{product}->{version}.".".$_->{product}->{build},
	    uptime => Time::Seconds->new($_->{uptime})->pretty(),
	};
    } @{$lists->{hosts}};

    # --------------------
    # Define column schema
    # --------------------
    
    my $schema = [
	{ title => 'HostName',   width => 35, field => 'name'    },
	{ title => 'Version',    width => 16, field => 'version' },
	{ title => 'Model',      width => 54, field => 'model'   },
	{ title => 'ServiceTag', width => 12, field => 'tag'     },
	{ title => 'Memory',     width => 10, field => 'memory'  },
	{ title => 'Skt',        width => 4,  field => 'sockets' },
	{ title => 'Cores',      width => 6,  field => 'cores'   },
	{ title => 'Threads',    width => 8,  field => 'threads' },
	{ title => 'CPU',        width => 46, field => 'cpu'     },
	{ title => 'Path',       width => 40, field => 'path'    },
	{ title => 'Uptime',     width => 10, field => 'uptime'  },
	];

    # --------------------
    # Create list of lists
    # --------------------
    
    my @lines;
    push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", $_->{title})} @$schema ]);
    push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", "-" x $w)} @$schema ]);
    
    for my $h (sort({$a->{name} cmp $b->{name}} @res))
    {
	push(@lines, [ map { my $w = $_->{width}; sprintf("%-${w}s", $h->{$_->{field}} // "-")} @$schema ]);
    }

    # --------------------
    # print output to stdout with delimeter
    # --------------------

    map {
	printf("%s\n", join("  ", @$_));
    } @lines;
    return undef;
}


# ----------------------------------------------------------------------
#
# json_dump
#
# ----------------------------------------------------------------------

sub json_dump {
    my ($conf, $dc, $lists) = @_;
    my $stream;

    my $table = vm_table($conf, $lists);

    my $dc_name = $dc->{name};
    
    # --------------------
    # Dump JSON to file
    # --------------------

    my $filename = "$dc_name.js";
    open($stream, ">:utf8", $filename);
    print $stream to_json($table, {pretty => 1});
    close($stream);
    return [$filename];
};


# ----------------------------------------------------------------------
#
# process_conf
#
# ----------------------------------------------------------------------

sub process_conf {
    my ($conf) = @_;

    # --------------------
    # There are multiple formatting functions, create dispatch table for them
    # --------------------

    my $dispatch = {
	excel => {
	    function => \&vm_report_excel,
	    mime => 'application/excel',
	},
	md => {
	    function => \&markdown,
	    mime => 'text/plain',
	},
	servers => {
	    function => \&servers_report_md,
	},
	json => {
	    function => \&json_dump,
	},
    };

    if (!$dispatch->{$conf->{format}})
    {
	printf(STDERR "Format $conf->{format} not supported.\n");
	exit(1);
    }
    my $format = $dispatch->{$conf->{format}};

    # --------------------
    # Check inventory is passed and exists
    # --------------------

    my $path = $conf->{inventory};

    if (!$path)
    {
	printf(STDERR "$0: --inventory <path> required\n");
	exit(1);
    }

    if (!-f $path)
    {
	printf(STDERR "$0: inventory file %s not found\n", $path);
	exit(1);
    }
    
    # --------------------
    # Read in file and covert from JSON
    # --------------------
    
    my $js = from_json(read_file($path));

    $conf->{meta} = $js->{meta};

    if (!exists $js->{datacenters})
    {
	printf(STDERR "$0: No datacenters found in inventory file %s\n", $path);
	exit(1);
    }

    if (!exists($js->{customfield}))
    {
	printf(STDERR "No customfield found.\n");
	exit(1);
    }
    $conf->{customfield} = $js->{customfield};

    my @output_files;
    
    # --------------------
    # Loop over datacenters
    # --------------------

    for my $dc (@{$js->{datacenters}})
    {
	my $lists = {};

	$lists->{hosts} = &host_descent($dc->{hosts}, []) if ($dc->{hosts});
	$lists->{virtualmachines} = &vm_descent($dc->{virtualmachines}, []) if ($dc->{virtualmachines});

	if (exists($dc->{net}))
	{
	    $lists->{networks} = $dc->{net}->{network} if (exists($dc->{net}->{network}));
	    $lists->{portgroups} = &pg_descent($dc->{net}, []);
	}
	
	# --------------------
	# Check hosts
	# --------------------

	my $host_list = $lists->{hosts};
	my $vm_list = $lists->{virtualmachines};

	if (!defined($host_list) || (scalar(@$host_list) == 0))
	{
	    print STDERR "No hosts found.\n";
	    exit 1;
	}

	my $files = $format->{function}->($conf, $dc, $lists);
	push(@output_files, @$files) if (defined($files));
    }

    # --------------------
    # Use MIME::Lite to email result
    # --------------------
    
    if ($conf->{mail} && @output_files)
    {
	my $msg = MIME::Lite->new(
	    From    => 'ross.alexander@agilisys.co.uk',
	    To      => 'ross.alexander@agilisys.co.uk',
	    Subject => sprintf('%s Active VMs', $conf->{meta}->{vcenter}),
	    Type    => 'multipart/mixed',
	    );

	$msg->attach(
	    Type => 'text/plain',
	    Data => 'See attachments'
	    );
	    
	for my $f (@output_files)
	{
	    $msg->attach(
		Encoding    => 'base64',
		Type        => $format->{mime}, # 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
		Disposition => 'attachment',
		Path        => $f,
		);
	}
	$msg->send();
	printf("Excel file %s sent\n", join(" ", @output_files));
    }
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my $conf = {
    excel_dir => 'excel',
    md_dir => 'md',
    format => 'excel',
};

# --------------------
# Place holders of hosta (host array) and vma (VM array)
# --------------------

GetOptions(
    'mail' => \$conf->{mail},
    'inventory=s' => \$conf->{inventory},
    'format=s' => \$conf->{format},
    );

process_conf($conf);
