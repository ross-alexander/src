#!/usr/bin/env perl

# ----------------------------------------------------------------------
#
# mime.pl
#
# 2021-10-01: Ross Alexander
#  Update to allow type after filename ie filename,type
#  Paths can be put on command line without --file

# 2018-01-18: ralexand
#  Add this header section
#
# ----------------------------------------------------------------------

use 5.32.1;
use MIME::Lite;
use Getopt::Long;
use POSIX;
use File::Basename;
use Sys::Hostname;
use MIME::Types;


my @uid = getpwuid($<);
my $sender = lc($uid[0]);
# $sender =~ s:[ ]+:.:g;
my $hostname = (split(m:\.:, hostname()))[0];

my $conf = {
    from => sprintf('%s%%%s@agilisys.co.uk', $sender, $hostname),
    inline => 0,
    subject => 'See attached files',
    type => 'text/plain',
    to => 'ross.alexander@agilisys.co.uk',
    debug => 0
};

my @paths;

GetOptions(
    'inline' => \$conf->{inline},
    'subject=s' => \$conf->{subject},
    'type=s' => \$conf->{type},
    'from=s' => \$conf->{from},
    'to=s' => \$conf->{to},
    'file=s@' => \@paths,
    'auto' => \$conf->{autotype},
    'debug' => \$conf->{debug},
);

# --------------------
# Include any remaining arguments as file paths
# --------------------

push(@paths, @ARGV);

# --------------------
# Check there is something to do
# --------------------

if (scalar(@paths) == 0)
{
    printf "$0: --file=<file> ... --file=<file>\n";
    exit 0;
}

# --------------------
# Check files exist
# --------------------

my $mt = MIME::Types->new();
my @attachments;

for my $p (@paths)
{
    my ($path, $type) = split(m:,:, $p);

    if (! -f $path)
    {
	print STDERR "$0: File $path not found.\n";
	exit 1;
    }

    if ((length($type) == 0) && $conf->{autotype})
    {
	$type = $mt->mimeTypeOf($path);
    }

    push(@attachments, {
	path => $path,
	type => length($type) ? $type : $conf->{type},
	base => basename($path)
	 });
}

# --------------------
# Create container message
# --------------------

my $msg = MIME::Lite->new(
    Type     => 'multipart/mixed',
    From     => $conf->{from},
    To       => $conf->{to},
    Subject  => sprintf("Attached file: %s", join(" ", map {$_->{base}} @attachments)),
    );

# --------------------
# Do not add additional text if inline
# --------------------

if (!$conf->{inline})
{
    $msg->attach(
	Type => 'text/plain',
	Data => 'See attachments'
	);
}

for my $attachment (@attachments)
{
    $msg->attach(
	Type     => $attachment->{type},
	Path     => $attachment->{path},
#	Disposition => $inline ? 'inline' : 'attachment',
#	Encoding => 'base64',
	);
}
$msg->send('smtp','10.255.4.18', Debug => $conf->{debug});
