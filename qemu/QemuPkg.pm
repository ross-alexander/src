package QemuPkg;

use 5.34.0;
use YAML qw(LoadFile);
use Carp;
use File::Spec::Functions;
use JSON;

sub pkgs_recc {
    my ($obj, $base, $pkg_conf, $pkg) = @_;

    my @res;

    # --------------------
    # Get package value
    # --------------------
    
    my $pp = $pkg_conf->{$pkg};

    # --------------------
    # If a list of dependant packages
    # --------------------
    
    if (ref($pp) eq "ARRAY")
    {
	@res = +( map { pkgs_recc($obj, $base, $pkg_conf, $_); } @$pp );
	push(@res, {
	    name => $pkg,
	    deps => [ map {$_->{name}} @res ]
	     }
	    );
    }

    # --------------------
    # Or structured data
    # --------------------
    
    elsif (ref($pp) eq "HASH")
    {
	my $res = {
	    name => $pkg
	};
	if ($pp->{path})
	{
	    if ($pp->{deps})
	    {
		push(@res, +( map { pkgs_recc($obj, $base, $pkg_conf, $_); } @{$pp->{deps}} ));
		$res->{deps} = [ map {$_->{name}} @res ];
	    }
	    my $path = catdir($base, $pp->{path});
	    if (-d $path)
	    {
		$res->{path} = $path;
	    }
	    else
	    {
		if (!$obj->{missing}->{$pkg})
		{
		    printf(STDERR "Package %s cannot be found.\n", length($pp) ? "$pkg [$path]" : $pkg);
		    $obj->{missing}->{$pkg} = 1;
		}
	    }
	    push(@res, $res);
	}
    }
    # --------------------
    # Else pkg : directory
    # --------------------
    
    elsif (length($pp) && (-d catdir($base, $pp)))
    {
	push(@res, { name => $pkg, path => catdir($base, $pp) });
    }
    elsif (-d catdir($base, $pkg))
    {
	push(@res, { name => $pkg, path => catdir($base, $pkg) });
    }
    else
    {
	if (!$obj->{missing}->{$pkg})
	{
	    printf(STDERR "Package %s cannot be found.\n", length($pp) ? "$pkg [$pp]" : $pkg);
	    $obj->{missing}->{$pkg} = 1;
	    return @res;
	}
    }
    return @res;
}

# ----------------------------------------------------------------------
#
# depth_first
#
# ----------------------------------------------------------------------

sub depth_first {
    my ($tree, $pkgs)= @_;

    # --------------------
    # Check if exists
    # --------------------

    my @pkgs = ref($pkgs) eq "ARRAY" ? @$pkgs : $pkgs;
    my $res = [];

    for my $p (@pkgs)
    {
        next if (!exists($tree->{$p}));
	
	my $pp = $tree->{$p};
	if ($pp->{deps})
	{
	    my @t = map { &depth_first($tree, $_) } @{$pp->{deps}};
	    push(@$res, map { @$_ } @t);
	}
	delete $tree->{$p};
	push(@$res, $pp);
    }
    return $res;
}

# ----------------------------------------------------------------------
#
# files
#
# ----------------------------------------------------------------------

sub files {
    my ($obj, $pkgs) = @_;

    # --------------------
    # Convert to list from ref if required
    # --------------------
    
    my @pkgs = ref($pkgs) eq "ARRAY" ? @$pkgs : $pkgs;

    my @files = map { &pkgs_recc($obj, $obj->{base}, $obj->{conf}, $_); } @pkgs;
    
# --------------------
# Create hash
# --------------------

    my $tree = {};
    for my $r (@files)
    {
	$tree->{$r->{name}} = $r;
    }

    my $files = &depth_first($tree, $pkgs);
    my @res;
    
    for my $f (@$files)
    {
#	printf("%s -> %s\n", $f->{name}, join(",", @{$f->{deps}})) if ($f->{deps});
#	printf("%s -> %s\n", $f->{name}, $f->{path}) if ($f->{path});
	push(@res, $f) if ($f->{path});
    }
    return @res;    
}


# ----------------------------------------------------------------------
#
# new
#
# ----------------------------------------------------------------------

sub new {
    my ($class, $opts, $file) = @_;

    my $res = {};

    my $pkg_conf_file = catfile($opts->{conf_base}, $file);
    croak "$0: Package configuration file $pkg_conf_file missing" if (! -f $pkg_conf_file);

    my ($pkg_conf) = LoadFile($pkg_conf_file);
    if (!$pkg_conf)
    {
	printf STDERR "$0: Could not load YAML file %s\n", $pkg_conf_file;
	exit 1;
    }

    $res = {
	conf => $pkg_conf->{pkg},
	base => $opts->{pkg_base},
	missing => {}
    };
    
    bless $res, $class;
    return $res;
}


return 1;
