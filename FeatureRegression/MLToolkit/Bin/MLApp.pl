#!/usr/bin/perl
#
use strict;
use warnings;
use FindBin;
# the path of modules
use lib "$FindBin::Bin/../Lib";
use lib "$FindBin::Bin/";
use Getopt::Long;
use MLTask::Shared::Driver;
use File::Basename;
use App::InterpretFeature;
use App::InterpretEntityFeature;
use App::ML;

my $app_name = shift @ARGV;

$app_name or usage();

my %app_support = (
	"ML" => 1,
	"InterpretFeature" => 1,
	"InterpretEntityFeature" => 1,
);

$app_support{$app_name} or die "unsupported application provided";

# run the appplication
eval "App::$app_name" . "::run()";
$@ and warn $@;

sub usage{
	print <<EOF;
	$0	<one of the following application>
		ML:			machine learning experiment		
		IntepretFeature		view item feature	
		InterpretEntityFeature	interpret entity features	

EOF
	exit 1;
}

