#!/usr/bin/perl

# ----------------------------------------------------------------------
#
# rf.pl
#
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
	my $st = stat($full) || die;

	# --------------------
	# Recurse if directory
	# --------------------

	if (-d $st)
	{
	    push(@$res, @{m3u_find($base, catdir($path, $f))});
	}

	# --------------------
	# If .m3u then add to list
	# --------------------

	if (-f $st && $f =~ m:\.m3u$:)
	{
	    push(@$res, {
		base => $base,
		dir => $path,
		file => $f,
		mtime => $st->mtime,
		 });
	}
    }
    return $res;
}

# ----------------------------------------------------------------------
#
# m3u_check
#
# ----------------------------------------------------------------------

sub m3u_check {
    my $args = {@_};

    # --------------------
    # Somewhat complicated params with entry => HASH
    # --------------------
    
    my $e = $args->{entry};
    assert($e);

    my $file = catfile($e->{base}, $e->{dir}, $e->{file});
    print $file, "\n";

    # --------------------
    # Read in .m3u file
    # --------------------
    
    my $stream;
    open($stream, "<", $file) || die;
    my @files = grep(!(m:^#:), <$stream>);
    close($stream);

    # --------------------
    # Chomp lines
    # --------------------
    
    map { chomp $_ } @files;

    my $res = [];

    # --------------------
    # Loop over each line
    # --------------------
    
    for my $f (@files)
    {
	my $path = catfile($e->{base}, $e->{dir}, $f);
	my $st = stat($path) || die;
	my $meta = {
	    file => $f,
	    mtime => $st->mtime,
	};

	# --------------------
	# Use ffprobe to get meta information
	# --------------------
	
	my $ffprobe = qx(ffprobe -v error -show_streams -show_format -print_format json $path);
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
	push(@$res, $meta);
    }

    # --------------------
    # Update HASH with list of media files
    # --------------------
    
    $e->{files} = $res;
}

# --------------------
# Using unicode internally so need to "decode" back to utf-8 on output
# --------------------

binmode(STDOUT, ":utf8");

# --------------------
# Find all m3u files
# --------------------

my $res = m3u_find("/locker/opus");
map {
    m3u_check(entry => $_);
} @$res;

# --------------------
# Write out as JSON.  This will be in utf8 format
# --------------------

my $stream;
open($stream, ">", "media.js");
print $stream to_json($res, { pretty => 1 });
close($stream);
