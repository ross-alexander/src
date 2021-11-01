#!/usr/bin/env perl

# ----------------------------------------------------------------------
#
# 2018-05-13
#
# Update to set named fields using --set field=value
#
# ----------------------------------------------------------------------

use 5.26.1;
use Audio::FLAC::Header;
use Music::Tag (traditional => 1);
use Getopt::Long;

sub fix {
    my ($set, $filename) = @_;
    return undef if (! -f $filename);

    my $flac = Audio::FLAC::Header->new($filename);

    my $info = $flac->info();
    
    foreach (keys %$info) {
	print "$_: $info->{$_}\n";
    }
    
    my $tags = $flac->tags();
    
    foreach (keys %$tags)
    {
	printf("%s: %s", $_, $tags->{$_});
	if ($set->{$_})
	{
	    my $v = $tags->{$_};
	    printf(" -> %s\n", $set->{$_});
	    $tags->{$_} = $set->{$_};
	}
	printf("\n");
    }
    $flac->write();
}

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

my %opts;

GetOptions(
    'set=s@' => \$opts{set}
    );


my $set = {};
map { my ($key, $value) = split(m:=:, $_); $set->{$key} = $value; } @{$opts{set} || []};

for my $filename (@ARGV)
{
    fix($set, $filename);
}

exit 0;

for my $filename (@ARGV)
{
    my $info = Music::Tag->new($filename, {verbose => 0} , 'FLAC');
    $info->get_tag() || die;
    printf "Artist is %s\n", $info->artist();
    printf "Album is %s\n", $info->album();
}
