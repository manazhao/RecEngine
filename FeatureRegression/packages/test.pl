#!/usr/bin/perl
#
use lib "./";

# test functions defined in Data::Common
#
use RecSys::Data::Common;
use Data::Dumper;

my $user_profile_file = "/home/manazhao/Dropbox/data/amazon_book_rating/test/author_profile.json";
my $feat_dict_file = "/home/manazhao/Dropbox/data/amazon_book_rating/test/feat_dict.csv";

# 
RecSys::Data::Common::process_entity("u",$user_profile_file,$feat_dict_file);
