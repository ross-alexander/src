#!/usr/bin/perl

use 5.42.2;
use SNMP;
use YAML;
use File::Slurp;
use Data::Dumper;
use NetAddr::IP;
use NetAddr::IP::Util qw(inet_ntop AF_INET AF_INET6);
use JSON;

# ----------------------------------------------------------------------
#
# Fetch
#
# ----------------------------------------------------------------------

sub Fetch {
    my ($conf, $host) = @_;
    my @vals;

# --------------------
# Check config exists
# --------------------

    if (!exists($conf->{$host}))
    {
	return 0;
    }

    my $cf = $conf->{$host};
    my $snmp = {
	'Version' => $cf->{'Version'},
	'DestHost' => $cf->{'Hostname'},
	'Community' => $cf->{'Community'},
    };

    my $sess = SNMP::Session->new(%$snmp);
    my $val = $sess->get('sysDescr.0');
    $conf->{$host}->{'desc'} = $val;

    print "$host: ";

    my $res_iftbl = {};

    # --------------------
    # Get basic interface details (ifTable)
    #
    # RFC 1066: Management Information Base for network management of TCP/IP-based internets
    # RFC 1155: STD 16: Structure and identification of management information for TCP/IP-based internets
    # RFC 1156: Management Information Base for network management of TCP/IP-based internets
    # RFC 1158: Management Information Base for network management of TCP/IP-based internets: MIB-II
    # RFC 1213: STD 17: Management Information Base for Network Management of TCP/IP-based internets: MIB-II
    # --------------------

    my $iftbl = $sess->gettable('ifTable');
    
    return 0 if (scalar(keys(%$iftbl)) == 0);
    print "ifTable ";

    while (my ($k, $v) = each(%$iftbl))
    {
	$res_iftbl->{$k} = {
	    indx => $v->{ifIndex},
	    desc => $v->{ifDescr},
	    type => $v->{ifType},
	    oper => $v->{ifOperStatus},
	    admin => $v->{ifAdminStatus},
	};
    }

# --------------------
# ipAddressTable
# --------------------

    my $pref_tbl = $sess->gettable('ipAddressPrefixTable');
    my $addr_tbl = $sess->gettable('ipAddressTable');

    if (scalar(keys(%$addr_tbl)) > 0)
    {
	print "ipAddressTable ";
	while (my ($k, $v) = each(%$addr_tbl))
	{
	    my $idx = $v->{ipAddressIfIndex};
	    my $family = $v->{ipAddressAddrType};
	    my $prefix_key = $v->{ipAddressPrefix};
	    $prefix_key =~ s:ipAddressPrefixOrigin\.::;
	    my $length = $pref_tbl->{$prefix_key}->{ipAddressPrefixLength};
	    
	    my $addr = $v->{ipAddressAddr};
	    
	    $addr = sprintf("%s", inet_ntop(AF_INET, $addr)) if ($family == 1);
	    $addr = sprintf("%s", inet_ntop(AF_INET6, $addr)) if ($family == 2);
					    
	    $res_iftbl->{$idx}->{'addr'} = [] if (!exists($res_iftbl->{$idx}->{'addr'}));
	    my $t = $res_iftbl->{$idx}->{'addr'};
	    push(@$t, { index => $idx, addr => $addr, mask => $length });
	}
    }
    else
    {

# --------------------
# Get IP address table (ipAddrTable)
# --------------------
    
	my $tbl = $sess->gettable('ipAddrTable');
	if (scalar(keys(%$tbl)) > 0)
	{
	    print "ipAddrTable ";
	    while (my ($k, $v) = each(%$tbl))
	    {
		my $idx = $v->{ipAdEntIfIndex};
		$iftbl->{$idx}->{'addr'} = [] if (!exists($iftbl->{$idx}->{'addr'}));
		my $t = $iftbl->{$idx}->{'addr'};
		my $addr = NetAddr::IP->new($v->{ipAdEntAddr}, $v->{ipAdEntNetMask});
		push(@$t, { index => $idx, addr => $addr->cidr() });
	    }
	}
	
    }
    
    $conf->{$host}->{'iftbl'} = $res_iftbl;

    # --------------------
    # Stop if routes not set in configuration
    # --------------------
    
    return 1 if ($conf->{$host}->{'routes'} == 0);

    # --------------------
    # Try for inetCidr (RFC4292 & RFC4001)
    # --------------------
    
    if ($sess->get('inetCidrRouteNumber.0') > 0)
    {
	print "inetCidrRoute ";
	my $tbl = $sess->gettable('inetCidrRouteTable');
	while (my ($k, $v) = each(%$tbl))
	{
#	    while (my ($kk, $vv) = each(%$v)) { print "$kk -> $vv\n"; } print "--------------------\n";

	    my $idx = $v->{inetCidrRouteIfIndex};
	    my $family;
	    $family = AF_INET6 if ($v->{inetCidrRouteDestType} == 4); # Non global IPv6
	    $family = AF_INET6 if ($v->{inetCidrRouteDestType} == 2); # Global IPv6
	    $family = AF_INET if ($v->{inetCidrRouteDestType} == 1);

	    if ($family)
	    {
		$res_iftbl->{$idx}->{route} = [] if (!exists($res_iftbl->{$idx}->{route}));
	    
		my $rtype = $v->{inetCidrRouteType};
		my $prefix = $v->{inetCidrRoutePfxLen};
		my $dest = bytes::substr($v->{inetCidrRouteDest}, 0, 16);
		my $t = $res_iftbl->{$idx}->{route};
		
		push(@$t, {
		    'dest' => NetAddr::IP->new(inet_ntop($family, $dest), $prefix)->cidr(),
		    'next' => NetAddr::IP->new(inet_ntop($family, $v->{inetCidrRouteNextHop}))->addr(),
		    type => $rtype,
		     }) if ($rtype == 4);
		
		push(@$t, {
		    'dest' => NetAddr::IP->new(inet_ntop($family, $dest), $prefix)->cidr(),
		    type => $rtype,
		 }) if ($rtype == 3);
	    }
	}
    }

# --------------------
# Try for ipCidr (RFC2096)
# --------------------
    
    elsif ($sess->get('ipCidrRouteNumber.0') > 0)
    {
	print "ipCidrRoute ";
	my $tbl = $sess->gettable('ipCidrRouteTable');
	while (my ($k, $v) = each(%$tbl))
	{
#	    while (my ($kk, $vv) = each($v)) { print "$kk -> $vv\n"; } print "--------------------\n";
	    my $index = $v->{ipCidrRouteIfIndex};
	    $res_iftbl->{$index}->{'route'} = [] if (!exists($iftbl->{$index}->{'route'}));
	    my $t = $res_iftbl->{$index}->{'route'};
	    push(@$t, {
		'dest' => NetAddr::IP->new($v->{ipCidrRouteDest}, $v->{ipCidrRouteMask})->cidr(),
		'next' => $v->{ipCidrRouteNextHop},
		'type' => $v->{ipCidrRouteType}});
	}
    }

# --------------------
# Try for ipForward (RFC1354)
# --------------------

    elsif  ($sess->get('ipForwardNumber.0') > 0)
    {
	print "ipForward ";

	my $vars = SNMP::VarList->new(['ipForwardIfIndex'],['ipForwardDest'],['ipForwardMask'],['ipForwardNextHop'], ['ipForwardType']);
    
	for (@vals = $sess->getnext($vars); $vars->[0]->tag =~ /ipForwardIfIndex/ and not $sess->{ErrorStr}; @vals = $sess->getnext($vars))
	{
	    $iftbl->{$vals[0]}->{'route'} = [] if (!exists($iftbl->{$vals[0]}->{'route'}));
	    my $t = $iftbl->{$vals[0]}->{'route'};
	    push(@$t, {
		'dest' => $vals[1],
		'mask' => $vals[2],
		'next' => $vals[3],
		'type' => $vals[4]});
	}
    }
    else
    {
	print "ipRoute ";
	my $vars = SNMP::VarList->new(['ipRouteIfIndex'],['ipRouteDest'],['ipRouteMask'],['ipRouteNextHop'],['ipRouteType']);
	for (@vals = $sess->getnext($vars); $vars->[0]->tag =~ /ipRouteIfIndex/ and not $sess->{ErrorStr}; @vals = $sess->getnext($vars))
	{
	    $iftbl->{$vals[0]}->{'route'} = [] if (!exists($iftbl->{$vals[0]}->{'route'}));
	    my $t = $iftbl->{$vals[0]}->{'route'};
	    push(@$t, {
		'dest' => $vals[1],
		'mask' => $vals[2],
		'next' => $vals[3],
		'type' => $vals[4]});
	}
    }
    print "\n";
    return 1;
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------
if (scalar(@ARGV) < 2)
{
    print STDERR "$0: [input dump] [output dump].\n";
    exit(1);
}

my $conf = YAML::LoadFile($ARGV[0]);

for my $k (keys(%$conf))
{
    &Fetch($conf, $k);
}

write_file($ARGV[1], to_json($conf, {pretty => 1}));
