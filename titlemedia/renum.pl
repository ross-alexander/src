#!/usr/bin/env perl

# ----------------------------------------------------------------------
#
# 2024-09-04: Add safety
#
# 2023-06-05
#
# ----------------------------------------------------------------------

use Getopt::Long;
use 5.36.0;
use File::Spec::Functions;
use File::Basename;

sub renum {
    my ($index, $diff, $renum, $paths) = @_;
    my $rx = sprintf('^([A-Za-z0-9_]+\.[0-9]+)\.(%02d)\.(.*)', $index);
    for my $path (@$paths)
    {
	my $dir = dirname($path);
	my $src = basename($path);

	if ($src =~ m:$rx:)
	{
	    my $dst = sprintf("%s.%02d.%s", $1, $2 - $diff, $3);
	    if ($src cmp $dst)
	    {
		printf("%s → %s : ", $src, $dst);
		if ($renum)
		{
		    my $src_path = catdir($dir, $src);
		    my $dst_path = catdir($dir, $dst);
		    if (-f $dst_path)
		    {
			printf("%s exists - renum aborted\n", $dst_path);
		    }
		    else
		    {
			my $res = rename(, catdir($dir, $dst));
			printf("%s", $res ? "done" : "failed");
		    }
		}
		else
		{
		    printf("not run");
		}
		printf("\n");
	    }
	}
    }
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my $start;
my $end;
my $new;
my $renum = 0;

GetOptions(
    'start=i' => \$start,
    'end=i' => \$end,
    'new=i' => \$new,
    'renum' => \$renum
);

if (!defined($start))
{
    printf("--start must be passed\n");
    exit 1;
}

if (!defined($end))
{
    printf("--end must be passed\n");
    exit 1;
}
if (!defined($new))
{
    printf("--new must be passed\n");
    exit 1;
}

if ($end < $start)
{
    printf("--end must be >= --start\n");
    exit 1;
}

my $diff = $start - $new;

for my $index (0 ... $end - $start)
{
    &renum($new > $start ? $end - $index : $start + $index, $diff, $renum, \@ARGV);
}
