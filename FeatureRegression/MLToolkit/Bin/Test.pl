#!/usr/bin/perl
#
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../Lib";
use MLTask::Shared::FeatureOperator;
use Data::Dumper;


my $fo = new MLTask::Shared::FeatureOperator( feature_file => "/home/manazhao/AmazonBeauty/PriceRegression/model/item_merged_feat.csv",
	feature_dict_file => "/home/manazhao/AmazonBeauty/PriceRegression/model/feat_dict.csv"
);

# group entities by category id
$fo->group_by_feature("i_lc_");

# dump to file
$fo->dump_feature_group(result_folder => "/home/manazhao/AmazonBeauty/PriceRegression/model/", feature_prefix => "i_lc_");


