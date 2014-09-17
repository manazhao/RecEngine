#!/usr/bin/perl
# generate three datasets and their respective sizes
# 1) feature calculation: 70%
# 2) model training: 20%
# 3) model testing: 10%
#

use warnings;
use strict;
use Date::Manip;
use File::Basename;
use lib  '../packages';
use Dataset::Shopcom;

my $input_orderdata_file = '/data/jian-data/shop-data/processed/orderdata.gt5.lt200';
# output files for the dataset splits
my $output_fc_file = '/data/jian-data/shop-data/processed/feature_calculation.csv';
my $output_mt_file = '/data/jian-data/shop-data/processed/model_training.csv';
my $output_test_file = '/data/jian-data/shop-data/processed/model_test.csv';

# the datasets are split by timestamps
# feature calculation split timestamp
my $fc_split_ts = UnixDate('2008-09-18 19:17:00',"%s");;
# model training split timestamp
my $mt_split_ts = UnixDate('2008-12-09 08:48:00',"%s");;

-f $input_orderdata_file and dirname($output_fc_file) and dirname($output_mt_file) and dirname($output_test_file) or die $!;

print ">>> load order data\n";
my $user_order_map = Dataset::Shopcom::load_order_data($input_orderdata_file);

open FC_FILE, ">", $output_fc_file or die $!;
open MT_FILE, ">", $output_mt_file or die $!;
open TEST_FILE, ">", $output_test_file or die $!;

print ">>> split datasets\n";

while(my($user_id,$user_orders) = each %$user_order_map){
	my @fc_idx = ();
	my @mt_idx = ();
	my @test_idx = ();
	for(my $i = 0; $i < scalar @$user_orders; $i++){
		my($uid,$iid,$ts) = @{$user_orders->[$i]};
		if($ts <= $fc_split_ts){
			push @fc_idx, $i;
		}
		if($ts > $fc_split_ts and $ts <= $mt_split_ts){
			push @mt_idx, $i;
		}
		if($ts > $mt_split_ts){
			push @test_idx, $i;
		}
	}

	if(@fc_idx > 0){
		print FC_FILE join(",",($user_id,$fc_idx[0],$fc_idx[$#fc_idx])) . "\n";
	}

	if(@mt_idx > 0){
		print MT_FILE join(",",($user_id,$mt_idx[0],$mt_idx[$#mt_idx])) . "\n";
	}

	if(@test_idx > 0){
		print TEST_FILE join(",",($user_id,$test_idx[0],$test_idx[$#test_idx])) . "\n";
	}

}

close FC_FILE;
close MT_FILE;
close TEST_FILE;
