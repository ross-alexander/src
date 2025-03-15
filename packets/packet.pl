#!/usr/bin/perl

# ----------------------------------------------------------------------
#
# 2025-03-03: Update for latest perl
#
# ----------------------------------------------------------------------

use 5.40.0;
use Graphics::ColorUtils;
use Cairo;
use Data::Dumper;
use Perl6::Slurp;
use JSON;

# ----------------------------------------------------------------------
#
# Layout_Cairo
#
# ----------------------------------------------------------------------
sub Layout_Cairo {
    my ($surface, $bpr, $collapse, $fontsize, $p) = @_;

# --------------------
# Setup cairo context
# --------------------

    my $cr = Cairo::Context->create($surface);
    $cr->set_font_size($fontsize);
    $cr->select_font_face('Arial', 'normal', 'normal');
    $cr->translate(10, 10);

# --------------------
# Set points per bit
# --------------------

    my $scale = 20;

    printf("\n");

# --------------------
# Current offset (bit)
# --------------------

    my $offset = 0;

    my @parts = @{$p->{'_order_'}};

    for my $i (@parts)
    {
	my $part = $p->{$i};
	
	my $bits = $part->{'bits'} // 0;
	my $text = $part->{'name'};

# --------------------
# If not fixed size then assume it is the payload
# --------------------

	if ($bits == 0)
	{
	    $bits = $collapse * $bpr;
	    $text = sprintf("Payload (%d octets)", $part->{'octets'});
	}

# --------------------
# Calculate
# --------------------

	my $row = int($offset / $bpr);
	my $col = $offset % $bpr;

	my $col_end = $col + $bits;
	my $h = int($col_end / ($bpr+1));
#	my $ho = ($col_end % $bpr) ? 1 : 0;

	my $n = $part->{name};
	$n =~ s: :_:g;

	printf("%-30s: bpr=%d bits=%2d offset=%2d fbits=%2d height=%d\n", $n, $bpr, $bits, $col, $col_end, $h);
	
	my $bit_first = $col;
	my $bit_last = $col_end % ($bpr+1); # ? $column_end % $bpr : $bpr;
	my $line_top = $row;
	my $line_bottom = $row + $h + 1;
	
	if ($h == 0)
	{
	    $cr->move_to($scale * $bit_first, $scale * $line_top);
	    $cr->line_to($scale * $bit_last,  $scale * $line_top);
	    $cr->line_to($scale * $bit_last,  $scale * $line_bottom);
	    $cr->line_to($scale * $bit_first, $scale * $line_bottom);
	    $cr->line_to($scale * $bit_first, $scale * $line_top);
	    $cr->close_path();
	}
	else
	{
	    $cr->move_to($scale * $bit_first, $scale * $line_top);
	    $cr->line_to($scale * $bpr, $scale * $line_top);
	    $cr->line_to($scale * $bpr, $scale * ($line_bottom));
	    $cr->line_to($scale * $bit_last, $scale * ($line_bottom));
	    $cr->line_to($scale * $bit_last, $scale * $line_bottom);
	    $cr->line_to($scale * 0, $scale * $line_bottom);
	    $cr->line_to($scale * 0, $scale * ($line_top + 1));
	    $cr->line_to($scale * $bit_first, $scale * ($line_top + 1));
	    $cr->line_to($scale * $bit_first, $scale * $line_top);
	    $cr->close_path();
	}
	my $level = $part->{'level'};
	my ($x1, $y1, $x2, $y2) = $cr->fill_extents();
	my ($colr, $colg, $colb) = hsv2rgb($level * 30, 0.2, 0.9);
	$cr->set_source_rgb($colr / 256, $colg / 256, $colb / 256);
	$cr->fill_preserve();
	$cr->set_source_rgb(0.0, 0.0, 0.0);
	$cr->set_line_width(1.0);
	$cr->stroke();
	
	if ($text)
	{
	    my $leading = 4;
	    my @lines = split(m:/:, $text);
	    my $th = scalar(@lines) * $fontsize + (scalar(@lines) - 1) * $leading;
	    my $ttop = $y1 + ($y2 - $y1 - $th) / 2;
	    map {
		my $extents = $cr->text_extents($_);
		$cr->move_to(($x1 + ($x2 - $x1 - $extents->{'width'})/2), $ttop - $extents->{'y_bearing'});
		$cr->show_text($_);
		$ttop += $fontsize + $leading;
	    } @lines;
	}

	for my $j (0 .. $bits-1)
	{
	    my $k = $offset + $j;
	    my $r = int($k / $bpr);

	    my $c = int($k % $bpr);

# Vertical ticks

	    $cr->move_to($c * $scale, $r * $scale);
	    $cr->rel_line_to(0, 2);
	    $cr->move_to($c * $scale, ($r+1) * $scale);
	    $cr->rel_line_to(0, -2);


	    $cr->move_to($c * $scale, $r * $scale);
	    $cr->line_to(($c+1) * $scale, $r * $scale);

	    $cr->set_line_width(0.3);
	    $cr->stroke();
	}

	$offset += $bits;
    }
    $cr->show_page();
}

# ----------------------------------------------------------------------
#
# Layout
#
# ----------------------------------------------------------------------

sub Layout {
    my ($p) = @_;

    my $name = $p->{'_name_'};

# --------------------
# Check _order_ field exists (list of strings)
# --------------------

    if (!exists($p->{'_order_'}))
    {
	print "Layout order missing from $name.\n";
	return 0;
    }

# --------------------
# Check each part as defined in _order_ exists
# --------------------

    my @parts = @{$p->{'_order_'}};
    for my $i (@parts)
    {
	if (!exists($p->{$i}))
	{
	    print "Layout subfield $i missing from $name.\n";
	}
    }

    printf("%s: ", $name);

# --------------------
# Set number of rows for collapsed region and bits per row
# --------------------

    my $collapse = 6;
    my $bits_per_row = 16;

# --------------------
# Count total number of octets
# --------------------

    my $total = 0;
    for my $i (@parts)
    {
	my $j = $p->{$i};
	printf("%s (%d) ", $i, $j->{'level'});
	$total += $j->{'collapse'} ? $bits_per_row * $collapse : $j->{'bits'};
    }
    print "\n";
    my $rows = int($total / $bits_per_row) + (($total % $bits_per_row) ? 1 : 0);
    my $width = 20 * $bits_per_row + 20;
    my $height = 20 * $rows + 20;
    my $fontsize = 10;

    &Layout_Cairo(Cairo::SvgSurface->create("$name.svg", $width, $height), $bits_per_row, $collapse, $fontsize, $p);
    &Layout_Cairo(Cairo::PsSurface->create("$name.eps", $width, $height), $bits_per_row, $collapse, $fontsize, $p);
    &Layout_Cairo(Cairo::PdfSurface->create("$name.pdf", $width, $height), $bits_per_row, $collapse, $fontsize, $p);
    my $image = Cairo::ImageSurface->create('argb32', $width, $height);
    &Layout_Cairo($image, $bits_per_row, $collapse, $fontsize, $p);
    $image->write_to_png("$name.png");
}

# ----------------------------------------------------------------------
#
# BuildRec
#
# ----------------------------------------------------------------------

sub BuildRec {
    my ($data, $level, $size, @names) = @_;

# --------------------
# Return if at end of list
# --------------------

    return undef if (scalar(@names) == 0);

    my $r = {
	'_name_' => join("_", @names),
    };
    my $name = shift(@names);
    my $p = $data->{$name};
    my $seq = $p->{'seq'};

    printf "%s: (%d octets)\n", $name, $size;

# --------------------
# Count fixed size components in bits
# --------------------

    my $hsize = 0;
    map {
	my $bits = exists($_->{'bits'}) ? $_->{'bits'} : ($_->{'octets'}//0) * 8;
	printf "%s: %s (%d bits)\n", $name, $_->{'name'}, $bits;
	$hsize += $bits;
    } @$seq;

    if (($hsize % 8) != 0)
    {
	print "Header not on octet boundary\n";
	exit 1;
    }

# --------------------
# Convert header size from bits to octets
# --------------------

    $hsize /= 8;

    my @pnames;
    for my $i (0 .. scalar(@$seq) - 1)
    {
	my $pname = sprintf("%s_%d_%d", $name, $level, $i);
	my $src = $seq->[$i];
	my $dst = $r->{$pname} = {};
	$dst->{'name'} = $src->{'name'};
	$dst->{'level'} = $level;
	if ($src->{'octets'} || $src->{'bits'})
	{
	    $dst->{'bits'} = $src->{'bits'} ? $src->{'bits'} : (($src->{'octets'}//0) * 8);
	    push(@pnames, $pname);
	}
	else
	{
	    my $payload = $size - $hsize;
	    if (scalar(@names) && ($payload > 0))
	    {
		my $q = BuildRec($data, $level+1, $payload, @names);
		push(@pnames, @{$q->{'_order_'}});
		map {
		    $r->{$_} = $q->{$_};
		} @{$q->{'_order_'}};
	    }
	    else
	    {
		$dst->{'octets'} = $payload;
		$dst->{'collapse'} = 1;
		push(@pnames, $pname);
	    }
	}
    }
    $r->{'_order_'} = \@pnames;
    return $r;
}

# ----------------------------------------------------------------------
#
# Build
#
# ----------------------------------------------------------------------

sub Build {
    my ($data, @names) = @_;

# --------------------
# Check requested formats exist
# --------------------

    for my $name (@names)
    {
	if (!exists($data->{$name}))
	{
	    print "No data for $name.\n";
	    return undef;
	}
    }

# --------------------
# Check initial format has MTU
# --------------------

    my $name = $names[0];
    my $p = $data->{$name};
    if (!exists($p->{'mtu'}))
    {
	print "No mtu set for $name.\n";
	return undef;
    }

# --------------------
# Call recursive build
# --------------------

    my $size = $p->{'mtu'};
    my $r = &BuildRec($data, 1, $size, @names);
    return $r;
}

# ----------------------------------------------------------------------
#
# Description of frame formats
#
# ----------------------------------------------------------------------

my $config = from_json(slurp('packet.js'));

if (!exists($config->{headers}))
{
    printf(STDERR "Key 'headers' missing\n");
    exit(1);
}

my $headers = $config->{headers};

if (exists($config->{layouts}))
{
    for my $layout (@{$config->{layouts}})
    {
	&Layout(&Build($headers, @$layout));
    }
}
else
{
    &Layout(&Build($headers, "EthernetII"));
    &Layout(&Build($headers, "EthernetII", "802.1q"));
    &Layout(&Build($headers, "EthernetII", "ARP"));
    &Layout(&Build($headers, "EthernetII", "IP"));
    &Layout(&Build($headers, "EthernetII", "IPv4", "GRE", "IPv6", "UDP"));
    &Layout(&Build($headers, "EthernetII", "IPv6"));
    &Layout(&Build($headers, "EthernetII", "IPv4", "UDP"));
}
