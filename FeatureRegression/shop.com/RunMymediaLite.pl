#!/usr/bin/perl
#
# run mymedialite implicit feedback algorithms
#
use strict;
use warnings;


my %algorithm_param_map = (
	"WRMF" => [
	#"num_factors=10",
	#	"num_factors=20",
		"num_factors=50",
	],
	"BPRMF" => [
	"num_factors=50"
	]
);

#my $algorithm = 'WRMF';
my $algorithm = 'BPRMF';


my $train_file = '/data/jian-data/shop-data/processed/mymedialite/train.csv';
my $test_file = '/data/jian-data/shop-data/processed/mymedialite/test.csv';
-f $train_file and -f $test_file or die $!;

my $result_folder = "/data/jian-data/shop-data/processed/mymedialite/$algorithm";
my $test_user_file = "/data/jian-data/shop-data/processed/test_users.csv";
mkdir $result_folder unless -d $result_folder;
my $prediction_file = $result_folder . "/prediction.csv";

# run the command
foreach(@{$algorithm_param_map{$algorithm}}){
	my $rec_options = $_;
	my $rec_options_name = $rec_options;
	$rec_options_name =~ s/\s+/\|/g;
		
	my $model_file = $result_folder . "/$rec_options_name" . ".model";
	my $command = "item_recommendation --no-id-mapping --training-file=$train_file --test-file=$test_file --test-users=$test_user_file --prediction-file=$prediction_file --save-model=$model_file --recommender=$algorithm --recommender-options=\"$rec_options\"";
	print $command . "\n";
	`$command`; 
}
