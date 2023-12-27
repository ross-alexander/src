my $stream;

my @t;
open($stream, "<", "table.txt");
@t = <$stream>;
close($stream);


open($stream, ">", "table.dmp");

for my $t (@t)
{
    chomp $_;
    $t =~ m:([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)/([0-9]+):;
    my $net = $1;
    my $prefix = $2;
    my $local = "F";
    my $next;
    my $if;
    if ($t =~ "directly connected")
    {
	$local = "L";
    }
    else
    {
	$t =~ m:via ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+):;
	$next = $1;
    }
    $t =~ m:,\ (Vlan [0-9]+|Lo):;
    $if = $1;
    printf($stream "%s,%s,%s,%s,%s\n", $local, $if, $net, $prefix, $next);
}
