#!/usr/bin/perl
#
#
use strict;
use warnings;
use lib '../packages';
use Dataset::Shopcom;


my $test_split_file = '/data/jian-data/shop-data/processed/model_test.csv';
my $input_orderdata_file = '/data/jian-data/shop-data/processed/orderdata.gt5.lt200';

print ">>> load test dataset split\n";
open TEST_FILE, "<", $test_split_file or die $!; 
my %user_test_map = (); 
while(<TEST_FILE>){
	chomp;
	my ($uid,$start_idx,$end_idx) = split /\,/;
	$user_test_map{$uid} = [$start_idx,$end_idx];
}
close TEST_FILE;

my $user_order_map = Dataset::Shopcom::load_order_data($input_orderdata_file);
Dataset::Shopcom::validate_dataset_split(\%user_test_map,$user_order_map);
