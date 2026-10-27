#!/usr/bin/env perl6

# ----------------------------------------------------------------------
#
# Convert text file to address-set
#
# 2026-10-26: Migrate to mojo
#  - zef install IP::Addr

#
# 2018-06-26
#   Make --set optional.
#
# ----------------------------------------------------------------------

use v6;
use IP::Addr;

sub MAIN(Str :$file!, Str :$set, Bool :$inv, Str :$zone, Str :$prefix)
{
    my @res;

    # --------------------
    # split file by LF
    # --------------------
    
    for slurp($file).chomp.split("\n") -> $line
    {
        next if ($line.chars() == 0);

	# --------------------
	# Split by whitespace (\s) [https://docs.raku.org/language/regexes#\s_and_\S]
	# --------------------
	
        my ($addr, $ip) = $line.split(rx /\s+/);

	# --------------------
	# Invert order if --inv set
	# --------------------
	
        ($addr, $ip) = ($ip, $addr) if ($inv);

	# --------------------
	# Check if only IP
	# --------------------
	
        if (!$ip && IP::Addr.new($addr))
        {
            $ip = $addr;
        }

	# --------------------
	# Replace space with hyphen
	# --------------------
	
        $addr .= subst(" ", "-", :g);

	# --------------------
	# Add prefix if set
	# --------------------

	if ($prefix)
	{
	    $addr = $prefix ~ $addr;
	}

	# --------------------
	# Push result onto list
	# --------------------
	
        push(@res, {
            name => $addr,
            addr => $ip
             });
    }

# --------------------
# Duplicate name check
# --------------------

    my $names = {};

    for @res -> $i {
        if (!($names{$i<name>}:exists))
        {
            $names{$i<name>} = 0;
        }
        $names{$i<name>}++;
    }

# --------------------
# Remove singletons (keep those with multiple entries)
# --------------------

    for $names.kv -> $name, $count
    {
        $names{$name}:delete if ($count < 2);
    }

# --------------------
# Rename hosts with multiple entries
# --------------------

    for @res.reverse -> $i {
        if ($names{$i<name>}:exists)
        {
            $i<name> = sprintf("%s_%02d", $i<name>, $names{$i<name>}--);
        }
    }

# --------------------
# Report
# --------------------

    my $verb = "set";
    my $zone_text;
    $zone_text = sprintf("security zone security-zone %s address-book", $zone) if ($zone);

    for @res -> $r {
        if ($r<addr>)
        {
            my @list = ($verb);
            @list.push($zone_text) if ($zone_text);
            @list.push("address", $r<name>, $r<addr>);
            printf("%s\n", @list.join(" "));
        }
    }

    if ($set)
    {
        for @res -> $r {
            my @list = ($verb);
            @list.push($zone_text) if ($zone_text);
            @list.push("address-set", $set, "address", $r<name>);
            printf("%s\n", @list.join(" "));
        }
    }
}
