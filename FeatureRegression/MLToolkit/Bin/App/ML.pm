# machine learning application
package App::ML;
use strict;
use warnings;
use FindBin;
# the path of modules
use lib "$FindBin::Bin/../../Lib";
use Getopt::Long;
use MLTask::Shared::Driver;
use File::Basename;
use Exporter;

our $VERSION = 1.0;
our @ISA = qw(Exporter);

sub run{
	my $config_file;
	GetOptions("config=s" => \$config_file) or die $!;
	my $app_name = basename($0);
	$config_file or usage();
	-f $config_file or die "configuration file - $config_file does not exist: $!";
	my $driver = new MLTask::Shared::Driver($config_file);
# process data
	$driver->init_data();
	$driver->train_model();
}

sub usage{
	print <<EOF;
	ML:	machine learning application with the following arguments
	--config	data configuration file

EOF
	exit 1;
}

1;
