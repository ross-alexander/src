#!/usr/bin/perl

# ----------------------------------------------------------------------
#
# libclean
#
# Try to find multiple libraries with the same SONAME
#
# 2026-06-09: Add Getopt::Long
#
# 2022-08-15
#
# ----------------------------------------------------------------------

use 5.42.2;
use File::Basename;
use File::stat;
use Time::Piece;
use JSON;
use Getopt::Long;

# ----------------------------------------------------------------------
#
# multiple_shared_libraries
#
# ----------------------------------------------------------------------

sub multiple_shared_libraries {
    my @files = </usr/lib64/lib*.so*>;

    @files = map {
	+{ path => $_,
	   file => qx(file -b $_),
	} } @files;
    
    @files = grep({$_->{file} =~ m:^ELF:} @files);
    
    map {chomp $_->{file}} @files;
    
    for my $f (@files)
    {
	my $dyn = qx(readelf -d $f->{path});
	my @dyn = split(m:\n:, $dyn);
	@dyn = grep(m:Library soname\::, @dyn);
	if (@dyn)
	{
	    $dyn[0] =~ m:Library soname\: \[([A-Za-z0-9+_.-]+)\]:;
	    if (length($1))
	    {
		$f->{soname} = $1;
	    }
	    else
	    {
		warn @dyn;
	    }
	}
	else
	{
	    printf("%s missing SONAME\n", $f->{path});
	}
    }
    
    my $soname = {};

    for my $f (@files)
    {
	push(@{$soname->{$f->{soname}}}, $f) if ($f->{soname});
    }
    
    for my $k (sort(keys(%$soname)))
    {
	my $list = $soname->{$k};
	if (scalar(@$list) > 1)
	{
	    printf("%s:\n", $k);
	    for my $l (@$list)
	    {
		printf("\t%s\n", $l->{path});
	    }
	}
    }
}

# ----------------------------------------------------------------------
#
# check_old_la
#
# ----------------------------------------------------------------------

sub check_old_la {

    my @files = </usr/lib64/*.so>;

    my $sec_diff = 300;
    
    for my $f (@files)
    {
	my $base = $f;
	$base =~ s:\.so$::;
	my $dot_so = $f;
	my $dot_a = $base . ".a";
	my $dot_la = $base . ".la";
	my $stat_a;
	my $stat_la;
	my $bad = 0;
	my $stat_so = stat($dot_so);
	if (-f $dot_a || -f $dot_la)
	{
	    if (-f $dot_a)
	    {
		$stat_a = stat($dot_a);
		$bad = 1 if (abs($stat_so->mtime - $stat_a->mtime) > $sec_diff);
	    }
	    if (-f $dot_la)
	    {
		$stat_la = stat($dot_la);
		$bad = 1 if (abs($stat_so->mtime - $stat_la->mtime) > $sec_diff);
	    }
	}
	if ($bad)
	{
	    say Time::Piece->new($stat_so->mtime), " ", $dot_so;
	    say Time::Piece->new($stat_la->mtime), " ", $dot_la if ($stat_la);
	    say Time::Piece->new($stat_a->mtime), " ", $dot_a if ($stat_a);
	    say "";
	}
    }
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my %opts;

GetOptions(
    "--multiple"	=> \$opts{m},
    "--la"		=> \$opts{la}
    );

check_old_la() if ($opts{la});
multiple_shared_libraries() if ($opts{m});

