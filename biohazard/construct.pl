#!/usr/bin/env perl

use 5.42.0;
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

sub construct {
    my ($surface) = @_;
    
    my $cr = Cairo::Context->create($surface);

    $cr->set_source_rgb(1, 1, 1);
    $cr->paint();

    $cr->select_font_face('URW Gothic L', 'normal', 'bold');
    $cr->set_font_size(10);

    $cr->set_source_rgb(0, 0, 0);
    $cr->translate(300, 300);
    $cr->set_line_width(1.0);

# Vertical axis
    
    my $scale = 10.0;

# Draw x,y axis

    my $a = 10.0;
    my $b = 35.0;
    my $c = 40.0;
    my $d = 60.0;
    my $e = 110.0; # outer circle offset
    my $f = 150.0; # inner circle offset
    my $g = 210.0; # inner circle diameter
    my $h = 300.0; # outer circle diameter
    

# Invert y-axis

    $cr->save();
    $cr->scale(1.0, -1.0);

# Inner ring, diameter d

    $cr->new_path();
    $cr->arc(0, 0, $d/2.0, 0, 2*pi);
    $cr->close_path();
    $cr->set_source_rgb(0, 0, 0);
    $cr->stroke();

# Outer circle of horn, center e, diameter h

    $cr->set_source_rgb(0,0,0);
    $cr->new_path();
    $cr->arc(0, $e, $h/2.0, 0, 2*pi);
    $cr->close_path();
    $cr->stroke();

# Inner circle of horn, center f, diameter g

    $cr->set_source_rgb(0,0,0);
    $cr->new_path();
    $cr->arc(0, $f, $g/2.0, 0, 2*pi);
    $cr->close_path();
    $cr->stroke();
    
# Ring outer edge, center 0, radius e-a+b

    $cr->set_source_rgb(0,0,0);
    $cr->arc(0, 0, $e - $a + $b, 0, 2*pi);
    $cr->arc(0, 0, $e - $a, 0, 2*pi);
    $cr->stroke();
    
    $cr->restore();

    $cr->set_source_rgb(0,0,1);
    
    $cr->new_path();
    $cr->move_to(-$d/2, 0);
    $cr->rel_line_to(0, -$d*1.5);
    $cr->move_to(+$d/2, 0);
    $cr->rel_line_to(0, -$d*1.5);
    $cr->move_to(-$d/2, -$d*1.4);
    $cr->rel_line_to(0.3*$d, 0);
    $cr->move_to(+$d/2, -$d*1.4);
    $cr->rel_line_to(-0.3*$d, 0);
    $cr->stroke();
    $cr->move_to(-5, -$d*1.35);
    $cr->show_text($d);

    # top of horn
    
    $cr->new_path();
    $cr->move_to(-20, -300);
    $cr->rel_line_to(0, 100);
    $cr->move_to(20, -300);
    $cr->rel_line_to(0, 100);
    $cr->move_to(-20, -290);
    $cr->rel_line_to(10, 0);
    $cr->move_to(20, -290);
    $cr->rel_line_to(-10, 0);
    $cr->stroke();
    $cr->move_to(-5, -286);
    $cr->show_text(40);


    # 100
    
    $cr->move_to(-300, -($e - $a));
    $cr->rel_line_to(600, 0);
    $cr->stroke();
    $cr->move_to(160, 0);
    $cr->rel_line_to(0, -35);
    $cr->move_to(160, -100);
    $cr->rel_line_to(0, 35);
    $cr->stroke();
    $cr->move_to(150, -45);
    $cr->show_text(100.0);

    # E = 11 / 110
    
    $cr->move_to(-300.0, -$e);
    $cr->rel_line_to(600.0, 0);
    $cr->move_to(180, 0);
    $cr->rel_line_to(0, -40);
    $cr->move_to(180, -$e);
    $cr->rel_line_to(0, 40);
    $cr->move_to(172, -52);
    $cr->show_text($e);
    $cr->stroke();

    # 135
    
    $cr->move_to(-300, -($e - $a + $b));
    $cr->rel_line_to(600, 0);
    $cr->stroke();
    $cr->move_to(200, 0);
    $cr->rel_line_to(0, -55);
    $cr->move_to(200, -135);
    $cr->rel_line_to(0, 55);
    $cr->stroke();
    $cr->move_to(192, -65);
    $cr->show_text(135.0);


    # F = 15 / 150
    
    $cr->move_to(-300.0, -$f);
    $cr->rel_line_to(600.0, 0);
    $cr->move_to(220, 0);
    $cr->rel_line_to(0, -60);
    $cr->move_to(220, -$f);
    $cr->rel_line_to(0, 60);
    $cr->move_to(210, -70);
    $cr->show_text($f);
    $cr->stroke();

    $cr->move_to(-300, -($f + $g/2));
    $cr->rel_line_to(600, 0);

    # F + G/2 = 255
    
    $cr->move_to(240, 0);
    $cr->rel_line_to(0, -160);
    $cr->move_to(240, -($f + $g/2));
    $cr->rel_line_to(0, 80);
    $cr->move_to(232, -164);
    $cr->show_text(255);
    $cr->stroke();

    # E + H/2 = 260
    
    $cr->move_to(-300, -($e + $h/2));
    $cr->rel_line_to(600, 0);
    $cr->move_to(260, 0);
    $cr->rel_line_to(0, -180);
    $cr->move_to(260,  -($e + $h/2));
    $cr->rel_line_to(0, 70);
    $cr->move_to(252, -180);
    $cr->show_text(260);
    $cr->stroke();

    $cr->set_source_rgb(1, 0, 0);
    my ($x1, $y1, $x2, $y2);
    my $theta = 2.0/3.0 * pi;
    
    $x1 = cos($theta) * 0.0 - sin($theta) * 110;
    $y1 = sin($theta) * 0.0 + cos($theta) * 110;

    # Flip y axis
    
    $y1 = -$y1;
    
    say ($x1,", ",$y1);
    
    $cr->arc($x1, $y1, 150, 0, 2*pi);
    $cr->stroke();

    ($x1, $y1, $x2, $y2) = circle_circle_intersection(0, 110, 150, $x1, -$y1, 150);
    $cr->arc($x2, -$y2, 3, 0, 2*pi);
    $cr->fill();
	

    $cr->move_to(-20, 0);
    $cr->line_to(-20, -300);
    $cr->stroke();
    $cr->arc(0, -150, 105, 0, 2*pi);
    $cr->stroke();
    
    ($x1, $y1, $x2, $y2) = circle_line_intersection(-20, 0, -20, 300, 0, 150, 105);
    $cr->arc($x1, -$y1, 3, 0, 2*pi);
    $cr->fill();

    ($x1, $y1, $x2, $y2) = circle_line_intersection(20, 0, 20, 300, 0, 150, 105);
    $cr->arc($x1, -$y1, 3, 0, 2*pi);
    $cr->fill();

    ($x1, $y1, $x2, $y2) = circle_line_intersection(-20, 0, -20, 300, 0, 110, 150);
    $cr->arc($x1, -$y1, 3, 0, 2*pi);
    $cr->fill();

    ($x1, $y1, $x2, $y2) = circle_line_intersection(20, 0, 20, 300, 0, 110, 150);
    $cr->arc($x1, -$y1, 3, 0, 2*pi);
    $cr->fill();

# Axis
    
    $cr->new_path();
    $cr->move_to(-300, 0);
    $cr->line_to(300, 0);
    $cr->move_to(0, -300.0);
    $cr->line_to(0, 300.0);
    $cr->set_source_rgb(0, 1, 0);
    $cr->stroke();


    print(join(" ", circle_line_intersection(-2, ($e+$f)/2, -2, 2*($e+$f), 0, $e, 15)));

    $surface->write_to_png('construct.png');
    $cr->show_page();
}
my $stream;
open($stream, "<", "bio.js");
my $vars = decode_json(join("", <$stream>));
close($stream);

my $surface = Cairo::SvgSurface->create("construct.svg", 600, 600);
construct($surface);
