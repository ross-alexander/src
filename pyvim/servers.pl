#!/usr/bin/perl

# ----------------------------------------------------------------------
#
# 2025-02-07: servers.pl

# Q&D perl script mostly to replicate the inv-format script for
# servers.  Uses the JSON output of vc-inv.py, which is similar
# to vmw-inventory.pl just different enough to be problematic.

#
# ----------------------------------------------------------------------

use 5.32.1;
use JSON;
use File::Slurp;
use Getopt::Long;

# ----------------------------------------------------------------------
#
# flatten
#
# ----------------------------------------------------------------------

sub flatten {
    my ($obj) = @_;
    if ((ref($obj) eq "HASH") && exists($obj->{folder}))
    {
	# my $flat = [];
	# for my $i (@{$obj->{folder}})
	# {
	#     my $f = flatten($i);
	#     push(@$flat, ref($f) eq "HASH" ? $f : @$f);
	# }
        # return $flat
	my $flat = [ map { my $f = flatten($_); ref($f) eq "HASH" ? $f : @$f; } @{$obj->{folder}} ];
    }
    else
    {
	return $obj;
    }
}

# ----------------------------------------------------------------------
#
# report
#
# ----------------------------------------------------------------------

sub report {
    my ($list) = @_;

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
            version => $_->{product}->{version},
        };
        $t;
    } @{$list};

    my $schema = [
        {
            title => 'VM Name',
            width => 35,
            field => 'name',
        },
        {
            title => 'Version',
            width => 8,
            field => 'version',
        },
        {
            title => 'Model',
            width => 24,
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
            width => 20,
            field => 'path'
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
}

# ----------------------------------------------------------------------
#
# servers
#
# ----------------------------------------------------------------------

sub servers {
    my ($path) = @_;

    my $json = read_file($path);
    my $data = from_json($json);

    if (!exists $data->{datacenters})
    {
	printf(STDERR "$0: 'datacenters' missing from %s\n", $path);
	exit(1);
    }
    for my $dc (@{$data->{datacenters}})
    {
	if (!exists $dc->{hosts})
	{
	    printf(STDERR "$0: 'hosts' missing from %s\n", $path);
	    exit(1);
	}
	my $hosts = $dc->{hosts};
	my $host_list = flatten($hosts);
	
	report($host_list);
    }
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my $opts = {};

GetOptions(
    'path=s' => \$opts->{path}
    );

if (!exists($opts->{path}) || !$opts->{path})
{
    printf(STDERR "$0: --path missing.\n");
    exit(1);
}

my $server_list = servers($opts->{path});
