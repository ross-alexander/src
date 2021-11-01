#!/usr/bin/perl

use 5.34.0;
use Music::Tag;
use Audio::Opusfile;

for my $filename (@ARGV)
{
    #    my $info = Music::Tag->new($filename, {verbose => 1});
#    $info->get_tag() || die;
#    printf "Artist is %s\n", $info->artist();
#    printf "Album is %s\n", $info->album();

    my $of = Audio::Opusfile->new_from_file($filename);
    my $tags = $of->tags;
    say $tags->query('TITLE'); # Cellule
    
}
