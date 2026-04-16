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
use JSON;
use Perl6::Slurp;
use File::Slurp;
use File::Spec::Functions;
use File::Path;
use MIME::Lite;
use Getopt::Long;
use Data::Dumper;
use Excel::Writer::XLSX;
use NetAddr::IP;
use Number::Bytes::Human qw(format_bytes parse_bytes);
use Net::IP::LPM;
use Time::Piece;
use List::Util qw(sum);
use Time::Seconds;
use Carp;
use utf8;
use locale;

# ----------------------------------------------------------------------
#
# vshield_subnets
#
# ----------------------------------------------------------------------

sub vshield_subnets {
    my ($conf, $sites) = @_;

    if (! -f $conf->{subnets})
    {
	return;
    }
    my $js = decode_json(slurp($conf->{subnets}));

    for my $client (keys(%$js))
    {
	for my $zone (keys(%{$js->{$client}->{zones}}))
	{
	    for my $dc (keys(%{$js->{$client}->{zones}->{$zone}}))
	    {
		my $subnets = $js->{$client}->{zones}->{$zone}->{$dc}->{subnets};
		for my $sn (@$subnets)
		{
		    $sn->{dc} = $dc;
		    push(@{$sites->{'client'}->{$client}->{nets}}, $sn);
		}
	    }
	}
    }
}


# ----------------------------------------------------------------------
#
# net_trie
#
# ----------------------------------------------------------------------

sub net_trie {
    my ($sites) = @_;

    for my $dc (keys(%{$sites}))
    {
	for my $client (keys(%{$sites->{client}}))
	{
	    my $nets = $sites->{client}->{$client}->{nets};
	    my $lpm = Net::IP::LPM->new();
	    my $map = {};
	    for my $net (@$nets)
	    {
		$map->{$net->{addr}}->{$net->{dc}} = $net;
	    }
	    while (my ($k, $v) = each(%$map))
	    {
		$lpm->add($k, $v);
	    }
	    $sites->{client}->{$client}->{lpm} = $lpm;
	}
    }
}


# ----------------------------------------------------------------------
#
# host_descent
#
# ----------------------------------------------------------------------

sub host_descent {
    my ($tree, $path) = @_;

    my @p = (@{$path}, $tree->{name});
    $tree->{path} = join("/", @$path);

#    say $tree->{path}, " + ", $tree->{name}, " + ", $tree->{type};
#    say $tree->{folder}, " * ", $tree->{hostsystem}, " * ", $tree->{cluster};

    return ($tree->{folder} || $tree->{hostsystem} || $tree->{cluster}) ? [ map { @{host_descent($_, \@p)}; } (@{$tree->{folder}}, @{$tree->{hostsystem}}, @{$tree->{cluster}}) ]
	: (($tree->{type} eq "HostSystem" || $tree->{type} eq "host") ? [$tree] : []);
}

# ----------------------------------------------------------------------
#
# descent (into madness)
#
# ----------------------------------------------------------------------

sub vm_descent {
    my ($tree, $path) = @_;
    my @p = (@$path, $tree->{name});

#    say $tree->{path}, " + ", $tree->{name}, " + ", $tree->{type};
#    say $tree->{folder}, " * ", $tree->{hostsystem}, " * ", $tree->{cluster};
    
#    return [] if (!$tree->{name});
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

    my $key_map = {};
    while (my ($k, $v) = each(%{$conf->{customfield}}))
    {
	$key_map->{$v->{key}} = $v->{name};
    }

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
	my $site = $conf->{host_site}->{$host->{name}};

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
		    if (defined($na) && ($na->version() == 4) && (!$ll_na->contains($na)) && (my $lpm = $conf->{sites}->{client}->{$client}->{lpm}))
		    {
			my $lookup = $lpm->lookup($na->addr());
			if ($lookup)
			{
			    if (!$lookup->{$site})
			    {
				say "$name: Incorrect site $site vs $na";
			    }
			    my ($dc_l, $net_l) = %$lookup;
			    my $vm_net = $na->network();
			    my $lu_net = NetAddr::IP->new($net_l->{addr});
			    say "$name: Bad subnet ", $vm_net, " ", $lu_net if ($vm_net <=> $lu_net);
			}
			else
			{
			    say "$name: Missing network $na client $client";
			}
		    }
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
	    site => $conf->{sites}->{$site}->{name},
	    tools_version => $i->{guest}->{tools_version},
	    tools_status => $i->{guest}->{tools_status},
	};
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
    my ($conf, $dc, $host_list, $vm_list) = @_;

    my @res = @{&vm_table($conf, $host_list, $vm_list)};


# --------------------
# Sort into clients
# --------------------

    my %clients;
    map {
        push(@{$clients{$_->{client}}}, $_);
    } @res;

    if (!-d $conf->{exceldir})
    {
	mkdir($conf->{exceldir}) || die;
    }
    my @files;
    
    my $file = catfile($conf->{exceldir}, sprintf("%s-%s.xlsx", $dc->{name}, Time::Piece->new($conf->{meta}->{epoch})->strftime("%Y%m%d")));
    push(@files, $file);
    my $wb = Excel::Writer::XLSX->new($file);

    # --------------------
    # Add fix for PSC/psc
    # --------------------

    my $client_lc_map = {};

        my $format = $wb->add_format(
            border => 0,
            bg_color => 42,
            size => 9,
            text_wrap => 1,
            font => 'Consolas',
            align => 'top'
            );

        my $heading = $wb->add_format(
            border => 2,
            bg_color => 'orange',
            bold => 1,
            size => 12,
            text_wrap => 0,
            font => 'Consolas',
            align => 'top',
            );

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
        my $ws = $wb->add_worksheet($wb_name);
        $ws->set_column(0, 0, 20);
        $ws->set_column(1, 1, 12);
        $ws->set_column(2, 2, 40);
        $ws->set_column(3, 3, 50);
        $ws->set_column(4, 4, 50);
        $ws->set_column(5, 5, 15);
        $ws->set_column(6, 6, 15);
        $ws->set_column(7, 7, 120);
        $ws->set_column(8, 8, 120);
        $ws->set_column(9, 9, 50);
        $ws->set_column(10, 10, 100);
        $ws->set_column(11, 12, 30);

        $ws->write(0, 0, ['VM Name', 'State', 'Hostname', 'Operating System', 'IP Addresses', 'CPU', 'Memory (MB)', 'Disks (OS)', 'VMDKs', 'Host', 'Path', 'Tools Version', 'Tools Status'], $heading);

        for (my $row = 0; $clients{$client}->[$row]; $row++)
        {
            my $t = $clients{$client}->[$row];
            $ws->write($row+1, 0, [$t->{name}, $t->{state}, $t->{fqdn}, $t->{os}, $t->{ipv4}, $t->{cpu}, $t->{mem}, $t->{disks}, $t->{vmdks}, $t->{host}, $t->{path}, $t->{tools_version}, $t->{tools_status}], $format);
        }
    }
    return \@files;
}


# ----------------------------------------------------------------------
#
# markdown
#
# ----------------------------------------------------------------------

sub markdown {
    my ($conf, $dc, $host_list, $vm_list) = @_;
    my @res = @{&vm_table($conf, $host_list, $vm_list)};
    my $res = {};
    for my $i (sort({$a->{name} cmp $b->{name}} @res))
    {
	my $name = $i->{name};
        my $client = $i->{client} || "ZZZ";
	push(@{$res->{$client}}, $i);
    }

    my $schema = [
	{
	    title => 'VM Name',
	    width => 24,
	    field => 'name',
	},
	{
	    title => 'FQDN',
	    width => 36,
	    field => 'fqdn'
	},
	{
	    title => 'State',
	    width => 10,
	    field => 'state'
	},
	{
	    title => 'IPv4 Address',
	    width => 16,
	    field => 'ipv4'
	},
	{
	    title => 'NICs',
	    width => 20,
	    field => 'nics'
	},
	{
	    title => 'Site',
	    width => 5,
	    field => 'site'
	},
	{
	    title => 'Host',
	    width => 20,
	    field => 'host'
	},
	{
	    title => 'Operating System',
	    width => 45,
	    field => 'os'
	},
	{
	    title => 'CPU',
	    width => 10,
	    field => 'cpu'
	},
	{
	    title => 'RAM (GB)',
	    width => 10,
	    field => 'mem'
	},
	{
	    title => 'Disks (Guest)',
	    width => 50,
	    field => 'disks',
	},
	{
	    title => 'Disks (VMDK)',
	    width => 50,
	    field => 'vmdks',
	},
	{
	    title => 'Storage (KB)',
	    width => 15,
	    field => 'storage'
	},
	];

    my @paths;
    
    for my $k (keys(%$res))
    {
	my $stream;

	my $path = catdir("md", $dc->{name}, Time::Piece->new($conf->{dc}->{epoch})->strftime("%Y%m%d"));
	mkpath($path);
	$path = catfile($path, "$k.md");
	push(@paths, $path);
	open($stream, ">:utf8", $path);
	my @t;
	my @f;
	for my $s (@$schema)
	{
	    my $w = $s->{width};
	    push(@f, sprintf("%-${w}s", $s->{title}));
	}
	push(@t, join(" | ", @f));

	@f = ();

	for my $s (@$schema)
	{
	    my $w = $s->{width};
	    push(@f, sprintf("%-${w}s", "-" x $w));
	}
	push(@t, join(" | ", @f));

	for my $v (@{$res->{$k}})
	{
	    my @f;
	    for my $s (@$schema)
	    {
		my $w = $s->{width};
		push(@f, sprintf("%-${w}s", $v->{$s->{field}}));
	    }
	    push(@t, join(" | ", @f));
	}

	map {
	    printf($stream "%s\n", $_);
	} @t;
    }
    return \@paths;
}


# ----------------------------------------------------------------------
#
# billable
#
# ----------------------------------------------------------------------

sub billable {
    my ($conf, $host_list, $vm_list) = @_;
    my @res = @{&vm_table($conf, $host_list, $vm_list)};
    my $res = {};

    my $billable;
    if ($conf->{customfield} && $conf->{customfield}->{billable})
    {
	$billable = $conf->{customfield}->{billable}->{key};
	say $billable;
    }
    for my $entry (@res)
    {
	say($entry->{name}, $entry->{tags}->{$billable});
    }
#    say to_json([@res], {pretty=>1});
}


# ----------------------------------------------------------------------
#
# servers_report_md
#
# ----------------------------------------------------------------------

sub servers_report_md {
    my ($conf, $dc, $lists) = @_;

    my @res = map {
	my $t = {
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
	$t;
    } @{$lists->{hosts}};

    my $schema = [
	{
	    title => 'HostName',
	    width => 35,
	    field => 'name',
	},
	{
	    title => 'Version',
	    width => 16,
	    field => 'version',
	},
	{
	    title => 'Model',
	    width => 54,
	    field => 'model'
	},
	{
	    title => 'ServiceTag',
	    width => 12,
	    field => 'tag'
	},
	{
	    title => 'Memory',
	    width => 10,
	    field => 'memory'
	},
	{
	    title => 'Skt',
	    width => 4,
	    field => 'sockets'
	},
	{
	    title => 'Cores',
	    width => 6,
	    field => 'cores'
	},
	{
	    title => 'Threads',
	    width => 8,
	    field => 'threads'
	},
	{
	    title => 'CPU',
	    width => 46,
	    field => 'cpu'
	},
	{
	    title => 'Path',
	    width => 28,
	    field => 'path'
	},
	{
	    title => 'Uptime',
	    width => 10,
	    field => 'uptime'
	},
	];

    for my $s (@$schema)
    {
	my $w = $s->{width};
	printf("%-${w}s", $s->{title});
    }
    printf("\n");

    for my $s (@$schema)
    {
	my $w = $s->{width};
	printf("%-${w}s", "---");
    }
    printf("\n");

    for my $h (sort({$a->{name} cmp $b->{name}} @res))
    {
	for my $s (@$schema)
	{
	    my $field_name = $s->{field};
	    my $w = $s->{width};
	    printf("%-${w}s", $h->{$field_name} // "-");
	}
	printf("\n");
    }
    printf("\n");
}


# ----------------------------------------------------------------------
#
# servers_report_md
#
# ----------------------------------------------------------------------

sub json_dump {
    my ($conf, $lists) = @_;
    my $stream;
    my $table = vm_table($conf, $lists);
    open($stream, ">:utf8", "inventory.js");
    print $stream to_json($table, {pretty=>1});
    close($stream);
};


# ----------------------------------------------------------------------
#
# process_conf
#
# ----------------------------------------------------------------------

sub process_conf {
    my ($conf) = @_;

    # --------------------
    # Make excel files
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
	billable => {
	    function => \&billable,
	},
	servers => {
	    function => \&servers_report_md,
	},
	json => {
	    function => \&json_dump,
	},
    };

    croak "Format $conf->{format} not supported." if (!$dispatch->{$conf->{format}});

    my $format = $dispatch->{$conf->{format}};

    # --------------------
    # Check sites
    # --------------------

    my $sites = from_json(slurp($conf->{sites}));

    # --------------------
    # Map hosts to sites
    # --------------------

    my $host_site_map = {};
    for my $s (keys(%$sites))
    {
	for my $h (@{$sites->{$s}->{hosts}})
	{
	    $host_site_map->{$h} = $s;
	}
    }
    $conf->{sites} = $sites;
    $conf->{host_site} = $host_site_map;

    # --------------------
    # Check inventory is passed
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
    # If inventory file
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

	# --------------------
	# Check all hosts accounted for
	# --------------------
	
	for my $h (@$host_list)
	{
	    if ($h->{name} && !$host_site_map->{$h->{name}})
	    {
		printf(STDERR "Host %s missing\n", $h->{name});
		exit(1);
	    }
	}

	# --------------------
	# Get network information
	# --------------------

	&vshield_subnets($conf, $sites);
	&net_trie($sites);

	my $files = $format->{function}->($conf, $dc, $lists);
	push(@output_files, @$files);
    }
    
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
    'exceldir' => 'excel',
    'format' => 'excel',
    'sites' => 'sites.js',
    'subnets' => 'vshield/vshield-subnets.js',
};

# --------------------
# Place holders of hosta (host array) and vma (VM array)
# --------------------

GetOptions(
    'mail' => \$conf->{mail},
    'inventory=s' => \$conf->{inventory},
    'format=s' => \$conf->{format},
    'sites=s' => \$conf->{sites},
    );

    process_conf($conf);
