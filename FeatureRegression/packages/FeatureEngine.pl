#!/usr/bin/perl
#
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin";
use RecSys::Data;
use Data::Dumper;
use Getopt::Long;
use File::Basename;
use Dataset::Shopcom;

my $config_file;

GetOptions("config=s" => \$config_file) or die $!;
my $app_name = basename($0);
$config_file or die "$app_name --config=<configuration file>:$!";
-f $config_file or die "configuration file - $config_file does not exist: $!";

RecSys::Data::process_dataset($config_file);
