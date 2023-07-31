#!/usr/bin/perl

# ----------------------------------------------------------------------
#
# reflac
#
# 2021-11-01: Ross Alexander
#   Rename format ogg to vorbis.

# 2018-05-12
#
# ----------------------------------------------------------------------

use 5.34.0;
use File::Spec::Functions;
use File::Basename;
use File::Glob ':bsd_glob';
use File::stat;
use File::Temp qw:tempfile:;
use Perl6::Slurp;
use Getopt::Long;

# ----------------------------------------------------------------------
#
# directory_create
#
# ----------------------------------------------------------------------

sub directory_create {
    my ($dir) = @_;

    my @parts = File::Spec->splitdir($dir);

    my $dir = File::Spec->rootdir();;
    map {
	$dir = File::Spec->catfile($dir, $_);
	if (-e $dir && !-d $dir)
	{
	    return 0;
	}
	if (!-e $dir)
	{
	    return if (!mkdir($dir));
	}

    } @parts;
    return 1;
}

# ----------------------------------------------------------------------
#
# directory_read
#
# ----------------------------------------------------------------------

sub directory_read {
    my $args = {@_};
    my $dir = $args->{dir};

    my @entries;
    
    if ($args->{glob})
    {
	@entries = sort(bsd_glob(catdir($dir, $args->{glob})));
	map { $_ = basename($_) } @entries;
    }
    else
    {
	my $dirp;
	if (!opendir($dirp, $dir))
	{
	    printf(STDERR "Cannot open directory $dir: $!\n");
	    return undef;
	}
	@entries = sort(grep(!m:^\.$|^\.\.$:, readdir($dirp)));
	closedir($dirp);
    }
    return \@entries;
}

# ----------------------------------------------------------------------
#
# m3u_check
#
# ----------------------------------------------------------------------

sub m3u_check {
    my ($conf, @args) = @_;

    my $args = {@args};

    my $srcdir = catfile($args->{source}, $args->{subdir});
    my $dstdir = catdir($args->{target}, $args->{subdir});

    # --------------------
    # Get path of m3u directory
    # --------------------
    
    my $path = catfile($srcdir, $args->{file});

    my $stream;
    if (!open($stream, $path))
    {
	printf(STDERR "m3u file $path not readable: $!\n");
	return 0;
    }
    my @entries = grep(!m:^#:, <$stream>);
    close($stream);

    if (!scalar(@entries))
    {
	printf(STDERR "m3u file $path seems to be empty.\n");
	exit 1;
    }
        
    # --------------------
    # get subdir if exists
    # --------------------

    my $childdir;
    
    for my $i (@entries)
    {
	chomp $i;
	if (!(dirname($i) eq $i))
	{
	    my ($n, $d) = fileparse($i);
	    if ($childdir && !($childdir eq $d))
	    {
		printf(STDERR "Multiple child directories in m3u file $path.\n");
		exit 1;
	    }
	    $childdir = $d;
	}
    }

    # --------------------
    # Do extra directory sanity checks
    # --------------------

    my @tracks;
    my %tracks;
    
    my $files = directory_read(dir => catdir($srcdir, $childdir));
    for my $file (@$files)
    {
	$file =~ m:\.([A-Za-z0-9]+)$:;
	my $suffix = $1;
	$file =~ s:\.$suffix::;
	$file =~ m:([0-9]+)_(.*):;

	if (defined($tracks[$1]))
	{
	    printf(STDERR "Duplicate track $1 $2 in $srcdir/$childdir\n");
	    exit 1;
	}
	if (length($1))#  && length($2))
	{
	    $tracks[$1] = $2;
	    $tracks{$1} = $2;
	}
	else
	{
	    print STDERR "Strange file $file\n";
	    exit(1);
	}
    }

    if (scalar(keys(%tracks) != scalar(@$files)))
    {
	printf(STDERR "Something wrong in $path: number of tracks [%d] != number of files [%d]\n", keys(%tracks), scalar(@$files));
	exit 1;
    }
    
    for my $i (1 .. scalar(@$files))
    {
	if (! exists($tracks[$i]))
	{
	    printf(STDERR "Track $i in $path missing.\n");
	    exit 1;
	}
    }

    # --------------------
    # Check format mapping
    # --------------------

    my $fmap = $conf->{formats};
     
    if (!exists($fmap->{$args->{format}}))
    {
	print "Cannot find format ", $args->{format}, "\n";
	return 0;
    }

    my $suffix = $fmap->{$args->{format}}->{suffix};


    if (!&directory_create($dstdir))
    {
	printf("Cannot make directory $dstdir\n");
	return 0;
    }

    my @m3u;
    my @cmds;
    map {
	chomp $_;
	my $src = $_;
	my $dst = $_;
	$dst =~ s:\.fla$:.$suffix:;
	my $srcpath = catfile($srcdir, $src);
	my $dstpath = catfile($dstdir, $dst);

#	print $srcpath, " -> ", $dstpath, "\n";
	
	if (!-f $srcpath)
	{
	    print "$srcpath missing.\n";
	    exit 1;
	}
	
	if (directory_create(dirname($dstpath)))
	{
	    if ((!-f $dstpath) || (stat($srcpath)->mtime > stat($dstpath)->mtime))
	    {
		my $codec = $conf->{formats}->{$args->{format}}->{codec};
		my $cmd = "ffmpeg -v error -threads 16 -i $srcpath -acodec $codec -ab 192k -ac 2 -y $dstpath";
		say $cmd;
		push(@cmds, $cmd);
	    }
	}
	push(@m3u, $dst);
    } @entries;

    if (scalar(@cmds))
    {
	my ($tfh, $tname) = tempfile();
	print $tfh join("\n", @cmds, "");
	close($tfh);

	my $res = `cat $tname | parallel`;
	my $rc = $?;
	if ($rc == 0)
	{
	    unlink($tname);
	}
	else
	{
	    die;
	}
    }

    # --------------------
    # Update resulting m3u
    # --------------------

    my $base = basename($path);
    my $out = catfile($dstdir, $base);

    my $update_m3u = 0;
    
    if (-f $out)
    {
	open($stream, "<", $out) || die;
	my @m3u_new = <$stream>;

#	if (scalar(@m3u) != scalar(@m3u_new))
#	{
#	    $update_m3u = 1;
#	    continue;
#	}

	map { chomp $_; } @m3u_new;
	
	for my $i (0 .. $#m3u)
	{
	    my $a = $m3u[$i];
	    my $b = $m3u_new[$i];
	    if (($a cmp $b) != 0)
	    {
		$update_m3u = 1;
	    }
	}
    }

    # --------------------
    # Update if new .m3u isn't here
    # --------------------

    if (! -f "$out")
    {
	$update_m3u = 1;
    }
	
    if ($update_m3u)
    {
	if (open($stream, ">$out"))
	{
	    map {
		print $stream $_,"\n";
	    } @m3u;
	    close($stream);
	    print "Writing $out\n";
	}
    }
    
    # --------------------
    # Check destination
    # --------------------

    my $filesdst = directory_read(dir => catdir($dstdir, $childdir));

    if (scalar(@$filesdst) != scalar(@$files))
    {
	printf("Directories $srcdir/$childdir & $dstdir/$childdir don't match\n");
	exit 1;
    }
}

# ----------------------------------------------------------------------
#
# CheckDir
#
# ----------------------------------------------------------------------

sub directory_check {
    my ($conf, @args) = @_;

    my $args = {@args};

    my $dirp;

    my $sdir = $args->{source};
    my $tdir = $args->{target};
    my $glob = $args->{glob};
    
# --------------------
# If no subdir then assume blank
# --------------------

#    my $subdir = exists($args->{subdir}) ? $args->{subdir} : "";

    my $subdir = $args->{subdir} // "";

    my $dir = File::Spec->catdir($sdir, $subdir);
    my @entries;

    if ($args->{glob})
    {
	@entries = sort(bsd_glob(catdir($dir, $args->{glob})));
	map { $_ = basename($_) } @entries;
    }
    else
    {
	if (!opendir($dirp, $dir))
	{
	    printf(STDERR "Cannot open directory $dir: $!\n");
	    return 0;
	}
	@entries = sort(grep(!m:^\.$|^\.\.$:, readdir($dirp)));
	closedir($dirp);
    }
    
# --------------------
# Iterate over directory entries
# --------------------
    
    map {
	my $path = catdir($dir, $_);

	if ((-f $path) && ($_ =~ m:\.m3u$:))
	{
	    &m3u_check($conf, subdir => $subdir, file => $_, source => $args->{source}, target => $args->{target}, format => $args->{format});
	}

	if (-d $path)
	{
	    &directory_check($conf, source => $sdir, target => $tdir, subdir => $subdir ? catdir($subdir, $_) : $_, format => $args->{format}, glob => $glob);
	}
    } sort(@entries);
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my $opts = {
    source => '/locker/flac',
    target => '/locker/m4a',
    format => 'aac',
    glob => '*'
};

my $conf = {
    formats => {
	vorbis => {
	    suffix => 'ogg',
	    codec => 'libvorbis',
	},
	aac => {
	    suffix => 'm4a',
	    codec => 'libfdk_aac',
	},
	opus => {
	    suffix => 'ogg',
	    codec => 'libopus',
	},
    },
};

GetOptions(
    'source=s' => \$opts->{source},
    'target=s' => \$opts->{target},
    'format=s' => \$opts->{format},
    'glob=s' => \$opts->{glob},
    );

&directory_check($conf, %$opts);
