#!/usr/bin/perl -CA

# -CA tells perl that ARGV will be UTF-8 encoded

# ----------------------------------------------------------------------
#
# m3u-meta.pl
#
# Recurse through directories to find m3u files, then extract
# metadata from referenced files using ffprobe.

# 2021-11-19: Ross Alexander
#   Rename to m3u-meta.pl
#   Clean up code
#   Ensure ARGV treated as UTF-8

# 2021-11-01: Ross Alexander
#   Fix for perl 5.34.0
#   Lots of fixes for utf8.
#   Add ffprobe to get stream meta data for tags.
#
# ----------------------------------------------------------------------

use 5.34.0;
use utf8;
use open qw(:std :utf8);
use Encode qw(decode_utf8 encode_utf8);
use File::Spec::Functions;
use File::stat;
use Carp::Assert;
use JSON;
use Getopt::Long;

# ----------------------------------------------------------------------
#
# m3u_find
#
# Recursively find all .m3u files in a directory.
#
# ----------------------------------------------------------------------

sub m3u_find {
    my ($base, $path) = @_;

    # --------------------
    # $path may be undefined
    # --------------------
    
    my $dir = catdir($base, $path);

    # --------------------
    # decode_utf8 added as filenames are encoded utf8
    # --------------------
    
    my $dirp;
    opendir($dirp, $dir) || return;
    my @files = map({decode_utf8($_)} grep(!(m:(^\.$)|(^\.\.$):), readdir($dirp)));
    closedir($dirp);

    my $res = [];

    for my $f (sort(@files))
    {
	my $full = catfile($dir, $f);
	my $st = stat($full);
	if (!$st)
	{
	    printf(STDERR "Failed to stat $full: $!\n");
	    exit(1);
	}

	# --------------------
	# Recurse if directory
	# --------------------

	push(@$res, @{m3u_find($base, $path ? catdir($path, $f) : $f)}) if (-d $st);

	# --------------------
	# If .m3u then add to list
	# --------------------

	push(@$res, {
	    base => $base,
	    dir => $path,
	    file => $f,
	    mtime => $st->mtime,
	     }) if (-f $st && $f =~ m:\.m3u$:);
    }
    return $res;
}

# ----------------------------------------------------------------------
#
# m3u_check
#
# ----------------------------------------------------------------------

sub m3u_check {
    my ($m3u) = @_;

    # --------------------
    # Somewhat complicated three part directory structure
    # --------------------

    my $path = catdir($m3u->{base}, $m3u->{dir});
    my $file = catfile($path, $m3u->{file});
    print $file, "\n";

    # --------------------
    # Read in .m3u file
    # --------------------
    
    my $stream;
    open($stream, "<", $file) || die;
    my @files = grep !(m:^#:), <$stream>;
    close($stream);
    chomp(@files);
    
    # --------------------
    # Loop over each line
    # --------------------

    my $index = 1;
    
    $m3u->{files} = [ map {
	my $file = catfile($path, $_);
	my $st = stat($path) || die;
	my $meta = {
	    file => $_,
	    index => $index++,
	    mtime => $st->mtime,
	};

	# --------------------
	# Use ffprobe to get meta information
	# --------------------
	
	my $ffprobe = qx(ffprobe -v error -show_streams -show_format -print_format json $file);
	my $probe = decode_json(encode_utf8($ffprobe));

	# --------------------
	# Different formats have tags in different places, with FLAC, M4A and Matroska/WEBM
	# having the tags in the format while OGG has the tags in the stream.
	# --------------------

	my $format = $probe->{format};
	my $tags = {};
	while(my ($k, $v) = each(%{$format->{tags} // {}}))
	{
	    $tags->{lc($k)} = $v;
	}
	for my $s (@{$probe->{streams}})
	{
	    while(my ($k, $v) = each(%{$s->{tags} // {}}))
	    {
		$tags->{lc($k)} = $v;
	    }
	}
	for my $k ('artist', 'album', 'title')
	{
	    $meta->{tags}->{$k} = $tags->{$k};
	}
	$meta;
    } @files ];

    return $m3u;
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

# --------------------
# Using unicode internally so need to "decode" back to utf-8 on output
# --------------------

binmode(STDOUT, ":utf8");

# --------------------
# Options processing
# --------------------

my $opts = {};
GetOptions(
    'in=s@' => \$opts->{in},
    'out=s' => \$opts->{out}
    );

# --------------------
# Find all m3u files
# --------------------

my $res = [];

for my $dir (@{$opts->{in} || []})
{
    push(@$res, {
	dir => $dir,
	m3u => [map { m3u_check($_); } @{m3u_find($dir)}]
	 });
}

# --------------------
# Write out as JSON.  This will be in utf8 format
# --------------------

if (defined($opts->{out}))
{
    my $stream;
    open($stream, ">", $opts->{out});
    print $stream to_json($res, { pretty => 1 });
    close($stream);
}
