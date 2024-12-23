# ----------------------------------------------------------------------
#
# QemuVm
#
# 2021-10-13: Ross Alexander
#
# ----------------------------------------------------------------------

package QemuVm;

use 5.24.0;
use File::Spec::Functions;
use File::Temp qw(tempdir);
use Perl6::Slurp;

sub new {
    my ($class, $parent, $vm) = @_;

    my $obj = {
	parent => $parent,
	vm => $vm
    };

    bless $obj, $class;
    return $obj;
}

sub configuration_get {
    my ($object) = @_;
    return $object->{vm};
}

sub ssh_add_keys {
    my ($obj) = @_;
    my $dir = File::Temp->newdir();
    my $vm = $obj->{vm};

    my $fqdn = $vm->{fqdn};
    
    for my $t ('rsa', 'ecdsa', 'ed25519')
    {
	my $keyfile = catfile($dir, sprintf("ssh_host_%s_key", $t));
	if (!-f $keyfile)
	{
	    my $cmd = sprintf("ssh-keygen -t %s -N \"\" -f %s -C %s", $t, $keyfile, $fqdn);
	    print $cmd, "\n";
	    print qx/$cmd/;
	    my $pubfile = $keyfile.".pub";
	    if (-f $keyfile && -f $pubfile)
	    {
		$vm->{ssh}->{$t}->{key} = slurp($keyfile);
		$vm->{ssh}->{$t}->{pub} = slurp($pubfile);
	    }
	}
    }
}

return 1;
