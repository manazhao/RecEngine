#!/usr/bin/perl
# test modules
#
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin";
use Data::Dumper;
use Dataset::Shopcom;
use Dataset::Common;
use Dataset::FeatureDict;
use Dataset::TermVectorLoader;

my $indri_loader = Dataset::TermVectorLoader->new("index"=>"/data/jian-data/shop-data/product_nlp/index/title",
"result"=>'/data/jian-data/shop-data/product_nlp/result/title'
);
$indri_loader->read();
#$indri_loader->calc_tfidf();
#$indri_loader->write();
print "score:" .  $indri_loader->cosine_score(572,572) . "\n";
print "score:" .  $indri_loader->cosine_score(572,44772) . "\n";
print "score:" .  $indri_loader->cosine_score(572,164205) . "\n";


# my $indri_loader = Dataset::TermVectorLoader->new("index"=>"/data/jian-data/shop-data/product_nlp/index/desc",
# "result"=>'/data/jian-data/shop-data/product_nlp/result/desc'
# );
# $indri_loader->load();
# $indri_loader->write();


exit;

# my $shopcom_order_data = Dataset::Shopcom::load_order_data("/home/manazhao/data/jian-data/shop-data/orderdata.txt.perUser.ordered");

# print Dumper(\%Dataset::Shopcom::time_windows);

# my $window_voting = Dataset::Shopcom::_soft_voting(1,3,50,1);
# print Dumper($window_voting);

# test interaction feature

=pod
=head1 
my $cur_item = [50, 0];

my $past_items = [
];

my $gap = $Dataset::Shopcom::time_windows{"day"};

for(my $i = 1; $i <= 1; $i ++){
	push @$past_items, ["c$i", -$gap * $i];
}

# my @cat_int_features = Dataset::Shopcom::_past_category_interact($past_items,$cur_item);
my @cat_int_features = Dataset::Shopcom::_past_category_time_window_interact($past_items,$cur_item);
my %int_features = ();
@int_features{@{$cat_int_features[0]}} = @{$cat_int_features[1]};
print Dumper(\%int_features);
=cut

=head1 Load item feature from the csv file

my $dict_file = "/home/manazhao/data/jian-data/shop-data/processed/feat_dict.csv";
my $item_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/item_feat.csv";
my $item_feat_map = Dataset::Shopcom::load_item_feature($dict_file,$item_feat_file);

print Dumper($item_feat_map);


my $item_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/item_feat.csv";
my $item_feat_map = Dataset::Common::load_entity_feature($item_feat_file);
print Dumper($item_feat_map);


# classify features by pattern

my $re_list = [
quotemeta "u_",
quotemeta "u_gender",
quotemeta "u_age",
quotemeta "i_c",
quotemeta "i_p"
];

my $dict_map = {
	1 => "u_gender_male",
	2 => "u_gender_female",
	3 => "u_age_3",
	4 => "i_c_book",
	5 => "i_m_apple"

};

my $item_feats = [1,2,3,4,5];
my $classified_feats = Dataset::Common::classify_feature($re_list,$dict_map,$item_feats);
print Dumper($classified_feats);
my $item_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/item_feat.csv";
my $item_feat_map = Dataset::Common::load_entity_feature($item_feat_file);
my $dict_file = "/home/manazhao/data/jian-data/shop-data/processed/feat_dict.csv";
my $dict_map = Dataset::Common::load_feat_dict($dict_file);
my $re_list = [
quotemeta "i_c"
];
my $classified_item_feat = Dataset::Common::classify_entity_feature($re_list,$dict_map,$item_feat_map);

# generate interaction feature
my $user_feats = [
[1,1],
[2,1],
[3,1]
];
my $item_feats = [
[4,1],
[5,1],
[6,1]
];
my @user_item_int_feats = Dataset::Common::gen_int_feature($user_feats,$item_feats);
print Dumper(\@user_item_int_feats);

my $dict_file = "/home/manazhao/data/jian-data/shop-data/processed/test_feat_dict.csv";
my $FEAT_DICT = Dataset::FeatureDict::get_instance(file => $dict_file);
for (500 .. 1000){
	$FEAT_DICT->index_feature("feat_$_");
}
print "id for feature feat_1:" . $FEAT_DICT->query_by_name("feat_1") . "\n";
print "id for feature feat_111:" . $FEAT_DICT->query_by_name("feat_111") . "\n";
print "name for feature 1:" . $FEAT_DICT->query_by_id(1) . "\n";
print "name for feature 111:" . $FEAT_DICT->query_by_id(111) . "\n";
=cut

my $dict_file = "/home/manazhao/data/jian-data/shop-data/processed/feat_dict.csv";
my $user_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/user_feat.csv";
my $item_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/item_feat.csv";
my $order_data_file = "/home/manazhao/data/jian-data/shop-data/processed/orderdata.g5";
my $sample_feat_file = "/home/manazhao/data/jian-data/shop-data/processed/orderdata_feat.libsvm";

my $MODULE_REF = Dataset::Shopcom::get_module_ref();
$MODULE_REF->train($dict_file,$user_feat_file,$item_feat_file,$order_data_file,$sample_feat_file);

