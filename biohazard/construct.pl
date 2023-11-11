#!/usr/bin/env perl

use 5.34.0;
use Cairo;
use Math::Trig;
use JSON;

sub circle_circle_intersection {
    my ($x0, $y0, $r0, $x1, $y1, $r1) = @_;
    
    my ($a, $dx, $dy, $d, $h, $rx, $ry);
    my ($x2, $y2);

    $dx = $x1 - $x0;
    $dy = $y1 - $y0;
    
    $d = sqrt(($dy*$dy) + ($dx*$dx));

#    $d = hypot($dx, $dy);
    
    if ($d > ($r0 + $r1))
    {
	return;
    }
    
    if ($d - abs($r0 - $r1) < 0.0)
    {
	return;
    }
    
    $a = (($r0*$r0) - ($r1*$r1) + ($d*$d)) / (2.0 * $d) ;

    $x2 = $x0 + ($dx * $a/$d);
    $y2 = $y0 + ($dy * $a/$d);

    $h = sqrt(($r0*$r0) - ($a*$a));

    $rx = -$dy * ($h/$d);
    $ry = $dx * ($h/$d);

    my $xi = $x2 + $rx;
    my $xi_prime = $x2 - $rx;
    my $yi = $y2 + $ry;
    my $yi_prime = $y2 - $ry;

    return ($xi, $yi, $xi_prime, $yi_prime);
}

sub circle_line_intersection {
    my ($x1, $y1, $x2, $y2, $x3, $y3, $r) = @_;

    say(join(" ", @_));
    
    my ($x, $y, $z);
    my ($a, $b, $c, $mu, $i);
    
    my @p;
    
    $a = ($x2 - $x1)*($x2 - $x1) + ($y2 - $y1)*($y2 - $y1);
    $b = 2 * (($x2 - $x1)*($x1 - $x3) + ($y2 - $y1)*($y1 - $y3));
    $c =  ($x3 * $x3) + ($y3 * $y3) + ($x1 * $x1) + 2 * ( $x3*$x1 + $y3*$y1) - ($r * $r);
    $i =  $b * $b - 4 * $a * $c;
  
    if ($i < 0.0)
    {
	return;
    }
  
  if ($i == 0.0)
    {
	$mu = -$b/(2*$a) ;
	my $rx1 = $x1 + $mu*($x2-$x1);
        my $ry1 = $y1 + $mu*($y2-$y1);
	return ($rx1, $ry1);
    }
    if ($i > 0.0)
    {
	$mu = (-$b + sqrt(($b * $b) - 4*$a*$c)) / (2*$a);
	my $rx1 = $x1 + $mu*($x2-$x1);
	my $ry1 = $y1 + $mu*($y2-$y1);
	
	$mu = (-$b - sqrt(($b * $b) - 4*$a*$c)) / (2*$a);
	
	my $rx2 = $x1 + $mu*($x2-$x1);
	my $ry2 = $y1 + $mu*($y2-$y1);
	return ($rx1, $ry1, $rx2, $ry2);
    }
}

my $stream;
open($stream, "<", "bio.js");
my $vars = decode_json(join("", <$stream>));
close($stream);



my $surface = Cairo::SvgSurface->create("construct.svg", 600, 600);
my $cr = Cairo::Context->create($surface);

$cr->select_font_face('URW Gothic L', 'normal', 'bold');
$cr->set_font_size(8);

$cr->translate(300, 300);
$cr->scale(1.0, -1.0);
$cr->scale(10.0, 10.0);
$cr->set_line_width(0.2);

# Vertical axis

$cr->new_path();
$cr->move_to(-30, 0);
$cr->line_to(30, 0);
$cr->move_to(0, -30);
$cr->line_to(0, 30);
$cr->stroke();


my $e = 11; # outer circle
my $f = 15;
# Outer circle

$cr->new_path();
$cr->arc(0, $e, 15, 0, 2*pi);
$cr->close_path();
$cr->stroke();

# Inner circle
$cr->new_path();
$cr->arc(0, $f, 10.5, 0, 2*pi);
$cr->close_path();
$cr->stroke();

$cr->new_path();
$cr->move_to(-2, ($e+$f)/2);
$cr->line_to(-2, ($e+$f));
$cr->move_to(2, ($e+$f)/2);
$cr->line_to(2, ($e+$f));
$cr->stroke();

print(join(" ", circle_line_intersection(-2, ($e+$f)/2, -2, 2*($e+$f), 0, $e, 15)));

$cr->show_page();
