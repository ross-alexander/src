#!/usr/bin/perl

use NetAddr::IP;
use 5.42.2;

# ----------------------------------------------------------------------
#
# Output
#
# ----------------------------------------------------------------------

sub HasInt {
    my ($cf, $ip) = @_;

    for my $host (values(%$cf))
    {
	print $host->{'Hostname'}, "\n";

	for my $int (values(%{$host->{'iftbl'}}))
	{
	    print $int->{'desc'}, "\n";
	    for my $addr (@{$int->{'addr'}})
	    {
		my $a = NetAddr::IP->new($addr->{'addr'});
		print "^^^ $ip -- $a ^^^\n" if ($a == $ip);
		return $host if ($a == $ip);
	    }
	}
    }
    exit(1);
    return undef;
}

# ----------------------------------------------------------------------
#
# Output
#
# ----------------------------------------------------------------------

sub Output {
    my ($ocf, $id, $host, $ncf) = @_;

# --------------------
# Create loopback and multicast addresses
# --------------------

    my $loop = NetAddr::IP->new("127.0.0.1/8");
    my $multi = NetAddr::IP->new("224.0.0.0/4");
    
    my $nets = $ncf->{'nets'};

    print $id, " : ", $host->{'desc'}, "\n";

# --------------------
# Get interface table and set number of connected networks to 0
# --------------------

    my $iftbl = $host->{'iftbl'};
    $host->{'ncount'} = 0;

# --------------------
# Sort interfaces by index
# --------------------

    for my $intf (sort {$a->{'indx'} <=> $b->{'indx'}} values(%$iftbl))
    {
	if ($intf->{'addr'})
	{
	    my $local = 0;

# --------------------
# Loop over interface IP addresses
# --------------------

	    map {
		say "--- ", $_->{addr}, " - ", $_->{mask};
		
		my $ip = NetAddr::IP->new($_->{'addr'}, $_->{'mask'});

		say "+++ ", $_->{addr}, " - ", $_->{mask};

		if ($loop->contains($ip))
		{
		    $local = 1;
		} 
	    } @{$intf->{'addr'}};
	    next if ($local == 1);

	    print "  ", $intf->{'desc'}, "\n";

# --------------------
# Record good addresses (no local or multicast
# --------------------

	    my @ips;
	    map {
		my $ip = NetAddr::IP->new($_->{'addr'}, $_->{'mask'});
		push(@ips, $ip);
		print "    ", $ip, "\n";

		$nets->{$ip->network()} = {} if (!exists($nets->{$ip->network()}));
		$nets->{$ip->network()}->{$ip} = $host;
		$host->{'ncount'}++;
	    } @{$intf->{'addr'}};

	    if (0)
	    {
# --------------------
# Iterate over routing table
# --------------------

		map {
		    my $dest = NetAddr::IP->new($_->{'dest'}, $_->{'mask'});
		    my $next = NetAddr::IP->new($_->{'next'});
		    
		    next if ($multi->contains($dest));
		    next if ($dest->broadcast()->addr() eq $dest->addr());
		    
		    print "      ", $dest, " -> ", $next;
		    
		    my $local = 0;
		    
		    for my $j (@ips)
		    {
			$local = 1 if ($j->network() eq $dest->network());
		    }
		    if ($local)
		    {
			print " L";
		    }
		    else
		    {
			print " R";
			my $dest_id = $next->addr();
			my $next_id = $dest->network();
#			print "  $d\n";
			my $new_host = &HasInt($ocf, $next);
			if (!defined($new_host))
			{
			    my $new_host = "host_" . $dest_id;
			    $new_host =~ s:[./]:_:g;
			    
			    $ncf->{$dest_id} = {
				'Hostname' => $new_host,
				'ncount' => 2,
			    };
			    print "New host $dest_id\n";
			};
			$ncf->{$dest_id}->{'ncount'}++;
			$nets->{$next_id} = {} if (!exists($nets->{$next_id}));
			
			$nets->{$next_id}->{$dest_id} = $ncf->{$dest_id};
		    }
		    print "\n";
		} @{$intf->{'route'}};	
	    }
	}
    }
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

if (scalar(@ARGV) < 1)
{
    print STDERR "$0: [input dump].\n";
    exit(1);
}

my $cf = {};

for my $f (@ARGV)
{
    my $stream;
    open($stream, "<", $f) || die "Cannot open $f.\n";
    my $text = join("", <$stream>);
    close($stream);
    my $VAR1;
    eval($text);

    say $VAR1;
    
    $cf = {%$cf, %$VAR1};
}

my $ncf = {};
$ncf->{'nets'} = {};
$ncf->{'hosts'} = {};

while (my($k, $v) = each(%$cf))
{
    &Output($cf, $k, $v, $ncf);
}

my $dot;

open($dot, ">", "foo.dot");
print $dot "graph Acton {\n";
print $dot "\tnode [fontname=\"Corbel\",fontsize=10,shape=\"box\",style=\"filled\",color=\"red\"];\n";
print $dot "\tedge [len=0.2];\n";
while(my ($netid,$net) = (each(%{$ncf->{'nets'}})))
{
    my $k2 = 'net_' . $netid;
    $k2 =~ s:[\.\:/-]:_:g;
    print $dot "\t", $k2, "\n";
}

print $dot "\tnode [shape=\"box\",style=\"filled\",color=\"skyblue\"];\n";

while(my ($netid, $net) = (each(%{$ncf->{'nets'}})))
{
    while (my ($hostid, $host) = (each(%$net)))
    {
	my $k2 = 'net_' . $netid;
	$k2 =~ s:[\.\:/-]:_:g;
	my $v2 = $host->{'Hostname'};
	$v2 =~ s:[\.\:/-]:_:g;
	print $dot "\t", $k2, " -- ", $v2, "\n" if ($host->{'ncount'} > 1);
    }
}
print $dot "}\n";
