#!/usr/bin/perl
#
use lib "./";

# test functions defined in Data::Common
#
use RecSys::Data::Common;
use Data::Dumper;

my $dataset_name = "Amazon";
my $type = "u";
my $user_profile_file = "/home/qzhao2/Dropbox/data/amazon_book_rating/test/author_profile.json";
my $feat_dict_file = "/home/qzhao2/Dropbox/data/amazon_book_rating/test/feat_dict.csv";
my $user_feature_file = "/home/qzhao2/Dropbox/data/amazon_book_rating/test/author_profile.feat.csv";

# 
# RecSys::Data::Common::process_entity($dataset_name,$type,$feat_dict_file, $user_profile_file, $user_feature_file);
my $config_file = "/home/qzhao2/Dropbox/data/amazon_book_rating/test/dataset_config.yml";
RecSys::Data::Common::process_dataset($config_file);
