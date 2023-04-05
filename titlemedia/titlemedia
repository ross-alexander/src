#!/usr/bin/perl -CA

# ----------------------------------------------------------------------
#
# 2023-02-19
#
# ----------------------------------------------------------------------

use 5.36.0;
use Encode;
use JSON::XS;
use Data::Dumper;
use Perl6::Slurp;
use File::Basename;
use File::Spec::Functions;

# ----------------------------------------------------------------------
#
# get_js_file
#
# ----------------------------------------------------------------------

sub get_js_file {
    my ($path) = @_;
    return decode_json(slurp($path));
}


# ----------------------------------------------------------------------
#
# mangle
#
# ----------------------------------------------------------------------

sub mangle {
    my ($t) = @_;

    # Remove special characters

    $t =~ s:[\!\$\(\)+/?.,\:'&]::g;

    # Convert spaces and hyphens to underscores

    $t =~ s:[\- ]:_:g;

    # Remove multiple _

    $t =~ s:[_]+:_:g;
    
    return $t;
}

# ----------------------------------------------------------------------
#
# rename_files
#
# ----------------------------------------------------------------------

sub rename_files {
    my ($series_list, $files) = @_;
    
# --------------------
# Build regex list
# --------------------

    my @rxlist;
    
    for my $d (@$series_list)
    {
	my $series = $d->{series};

	# --------------------
	# Have seasons
	# --------------------
	
	if (my $seasons = $d->{seasons})
	{ 
	    foreach my $season (sort(keys(%$seasons)))
	    {
		my $v = $seasons->{$season};
		my $episode = exists($v->{start}) ? $v->{start} : 1;
		foreach my $title (@{$v->{titles}})
		{
		    my $regex = sprintf("%s.%02d.%02d", mangle($series), $season, $episode);
		    push(@rxlist, [ $regex, $series, $season, $episode, $title ]);
		    $episode++;
		}
	    }
	}
	else
	{
	    # --------------------
	    # Single season / No seaons
	    # --------------------
	    
	    my $titles = $d->{titles};
	    my $i = (exists($d->{start})) ? $d->{start} : 1;
	    for my $t (@$titles)
	    {
		my $regex;
		if (my $season = $d->{season})
		{	    
		    $regex = sprintf("%s.%02d.%02d", mangle($series), $season, $i);
		    push(@rxlist, [ $regex, $series, $season, $i, $t ]);
		}
		else
		{
		    $regex = sprintf("%s.%02d", mangle($series), $i);
		    push(@rxlist, [ $regex, $series, undef, $i, $t ]);
		}
		$i++;
	    }
	}
    }

# --------------------
# Match regex to list of files
# --------------------

    foreach my $path (@ARGV)
    {
	next if (! -f $path);
    
	my $src = basename($path);
	my $dir = dirname($path);

	for my $rx (@rxlist)
	{
	    if (grep(m:$rx->[0]:, $src))
	    {
		$src =~ m:\.([A-Za-z0-9]+)$:;
		my $suffix = $1;
		my $dst = sprintf("%s.%s.%s", $rx->[0], mangle($rx->[4]), $suffix);
		if ($src cmp $dst)
		{
		    $src = catfile($dir, $src);
		    $dst = catfile($dir, $dst);
		    rename($src, $dst);
		    printf("%s -> %s\n", $src, $dst);
		}
	    }
	}
    }
}


# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

binmode(STDOUT, ":utf8");

my $js = get_js_file("/locker/dvd/titles.js");

    rename_files($js, [@ARGV]);
    
