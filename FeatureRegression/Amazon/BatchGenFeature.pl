#!/usr/bin/perl
#
use strict;
use warnings;
use File::Basename;

my $home_dir = $ENV{"HOME"};
my $input_folder = "$home_dir/Dropbox/data/amazon_book_rating";
my $result_folder = $input_folder . "/popularity_result2";
my $user_json_file = $input_folder . "/author_profile.json";
my $item_json_file = $input_folder . "/item_profile.json";
my $rating_json_file = $input_folder . "/rating.json";

# output
my $dict_file = $input_folder . "/feature_dict.csv";
# aggregated feature dictionary
my $item_agg_dict_file = $input_folder . "/item_agg_feat_dict.csv";
# category tree
my $item_cat_tree_file = $input_folder . "/cat_tree.csv";
# item aggregated feature
my $item_agg_feature_file = $result_folder . "/item_agg_feat.csv";
# merge individual feature and aggregated level feature for item
my $item_merge_feature_file = $result_folder . "/item_merge_feat.csv";

# generate output file
-d $result_folder or die "result folder - $result_folder does not exist: $!";
-d dirname($dict_file) or die "invalid dictionary file path$!";
-d dirname($item_cat_tree_file) or die "invalid directory for product category tree: $!";
-d dirname($item_agg_dict_file) or die "invalid directory for item aggregation feature dictionary: $!";
-d dirname($item_agg_feature_file) or die "invalid directory for item aggregation feature: $!";
-d dirname($item_merge_feature_file) or die "invalid directory for item merge feature: $!";
-f $user_json_file or die "user feature file does not exist $!";
-f $item_json_file or die "item feature file does not exist $!";
-f $rating_json_file or die "rating file does not exist $!";

my $user_feature_file = $result_folder . "/" . remove_suffix(basename($user_json_file)) . "_feat.csv";
my $item_feature_file = $result_folder . "/" . remove_suffix(basename($item_json_file)) . "_feat.csv";

my $cmd ;
# generate user and item feature file
unless (-f $user_feature_file){
    my $cmd = "perl GenEntityFeature.pl --input=$user_json_file --dict=$dict_file --output=$user_feature_file --type=u";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}

unless(-f $item_feature_file){
    $cmd = "perl GenEntityFeature.pl --input=$item_json_file --dict=$dict_file --output=$item_feature_file --type=i";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}

# generate aggreated feature if not exist yet
unless( -f $item_agg_dict_file){
    $cmd = "perl GenAggregateFeature.pl --item=$item_json_file --rating=$rating_json_file --result=$item_agg_dict_file --tree=$item_cat_tree_file";
    print $cmd . "\n";
    `$cmd`;
}

# generate aggregation level features for each item
unless (-f $item_agg_feature_file){
    $cmd = "perl GenItemAggFeature.pl --agg=$item_agg_dict_file --item=$item_json_file --result=$item_agg_feature_file --tree=$item_cat_tree_file --dict=$dict_file";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}


# merge the invidual level and aggregation level features for item
unless(-f $item_merge_feature_file){
    $cmd = "perl MergeFeature.pl $item_feature_file $item_agg_feature_file > $item_merge_feature_file";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}

# generate purchase data from the ratings
my $purchase_json_file = $result_folder . "/" . remove_suffix(basename($rating_json_file)) . "_purchase.json";
unless(-f $purchase_json_file){
    $cmd = "perl GenPurchase.pl --rating=$rating_json_file --ratio=1 --output=$purchase_json_file";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}

# generate feature
my $purchase_feature_file = $result_folder . "/" . remove_suffix(basename($purchase_json_file)) . "_feat.csv";
unless(-f $purchase_feature_file) {
    $cmd = "perl GenRatingFeature.pl --dict=$dict_file --user=$user_feature_file --item=$item_merge_feature_file --rating=$purchase_json_file --output=$purchase_feature_file";
    print ">>> " . $cmd . "\n\n";
    `$cmd`;
}

# convert to libsmv format
my $libsvm_fmt_file = $result_folder . "/" . remove_suffix(basename($purchase_json_file)) . "_feat.libsvm";
unless(-f $libsvm_fmt_file){
    $cmd = "perl LibSVMFormat.pl --input=$purchase_feature_file --output=$libsvm_fmt_file";
    print ">>> " . $cmd . "\n";
    `$cmd`;
}


sub remove_suffix{
    my ($fn) = @_;
    $fn =~ s/\..*?$//g;
    return $fn;
}

