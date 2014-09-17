#!/usr/bin/perl
#
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/";
use Getopt::Long;
use MLTask::Shared::Driver;
use File::Basename;

my $config_file;

GetOptions("config=s" => \$config_file) or die $!;
my $app_name = basename($0);
$config_file or usage();
-f $config_file or die "configuration file - $config_file does not exist: $!";

my $driver = new MLTask::Shared::Driver($config_file);
# process data
$driver->process_data();

sub usage{
	print <<EOF;
$0	[options]
	--config	data configuration file

EOF
	exit 1;
}
