#!/usr/bin/perl

use File::Spec::Functions;
use File::stat;
use Carp::Assert;
use JSON;
use Encode;

# ----------------------------------------------------------------------
#
# m3u_find
#
# ----------------------------------------------------------------------

sub m3u_find {
    my ($base, $path) = @_;

    my $dir = catdir($base, $path);

    my $dirp;
    opendir($dirp, $dir) || return;
    my @files = grep(!(m:(^\.$)|(^\.\.$):), readdir($dirp));
    closedir($dirp);

    my $res = [];

    for my $f (sort(@files))
    {
	my $full = catfile($dir, $f);

	my $st = stat($full) || die;
	if (-d $st)
	{
	    push($res, @{m3u_find($base, catdir($path, $f))});
	}
	if (-f $st && $f =~ m:\.m3u$:)
	{
	    push($res, {
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

    my $e = $args->{entry};
    assert($e);

    my $stream;
    my $file = catfile($e->{base}, $e->{dir}, $e->{file});
    print $file, "\n";
    open($stream, "<", $file) || die;
    my @files = grep(!(m:^#:), <$stream>);
    close($stream);
    map { chomp $_ } @files;

    my $res = [];

    for my $f (@files)
    {
	my $path = catfile($e->{base}, $e->{dir}, $f);
	my $st = stat($path) || die;
	push($res, {
	    file => $f,
	    mtime => $st->mtime,
	     });
    }
    $e->{files} = $res;
}


my $res = m3u_find("/locker/flac", ".");
map {
    m3u_check(entry => $_);
} @$res;

my $stream;
open($stream, ">", "media.js");
print $stream to_json($res, { pretty => 1 });
close($stream);
