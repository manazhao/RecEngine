#!/usr/bin/perl
#
# generate files for mymedialite tool
#
use strict;
use warnings;
use lib '../packages';
use Dataset::Shopcom;
use Dataset::Common;
use Dataset::FeatureDict;

my $input_orderdata_file = '/data/jian-data/shop-data/processed/orderdata.gt5.lt200';
my $item_feat_file = '/data/jian-data/shop-data/processed/item_feat.csv';
my $feat_dict_file = '/data/jian-data/shop-data/processed/feat_dict.csv';
# feature calculation split dataset
my $fc_split_file = '/data/jian-data/shop-data/processed/feature_calculation.csv';
my $test_split_file = '/data/jian-data/shop-data/processed/model_test.csv';

my $result_folder = '/data/jian-data/shop-data/processed/mymedialite';
# store user item information
my $train_user_file = $result_folder . '/train.csv';
my $test_user_file = $result_folder . '/test.csv';
# store item attributes
my $item_attr_file = $result_folder .  '/item_attr.csv';
# store model

-f $input_orderdata_file and -f $item_feat_file and -f $feat_dict_file and -f $fc_split_file or die $!;

# load the feature dictionary
# print ">>> intialize feature dictionary object\n";
# my $feat_dict_obj = Dataset::FeatureDict::get_instance(file => $feat_dict_file);

# print ">>> load item features\n";
# my $item_feat_map = Dataset::Common::load_entity_feature($item_feat_file);

# load training sample index range
print ">>> load training dataset split\n";
open FC_FILE, "<", $fc_split_file or die $!;
my %user_train_map = ();
while(<FC_FILE>){
	chomp;
	my ($uid,$start_idx,$end_idx) = split /\,/;
	$user_train_map{$uid} = [$start_idx,$end_idx];
}
close FC_FILE;

print ">>> load test dataset split\n";
open TEST_FILE, "<", $test_split_file or die $!;
my %user_test_map = ();
while(<TEST_FILE>){
	chomp;
	my ($uid,$start_idx,$end_idx) = split /\,/;
	$user_test_map{$uid} = [$start_idx,$end_idx];
}
close TEST_FILE;


# load the order data
print ">>> load user order data\n";
my $user_order_map = Dataset::Shopcom::load_order_data($input_orderdata_file);

# now generate the user-item file
print ">>> generating training user file\n";
Dataset::Shopcom::generate_dataset_split($train_user_file,\%user_train_map, $user_order_map);

print ">>> generating test user file\n";
Dataset::Shopcom::generate_dataset_split($test_user_file,\%user_test_map, $user_order_map);



# generate item attribute file
# my $re_list = [quotemeta "i_2c"];
# get the classified features for item
# print ">>> prepare item category feature \n";
# my $item_cfeat_map = Dataset::Common::classify_entity_feature($re_list,$feat_dict_obj->get_name_map(),$item_feat_map);

# open ITEM_ATTR_FILE, ">", $item_attr_file or die $!;
# while(my($item_id,$feats) = each %$item_cfeat_map){
#	my $feat_id = $feats->[0]->[0]->[0];
#	print ITEM_ATTR_FILE join(",",($item_id,$feat_id)) . "\n";
# }

# close ITEM_ATTR_FILE;




