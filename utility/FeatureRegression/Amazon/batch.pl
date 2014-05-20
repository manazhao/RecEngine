#!/usr/bin/perl
#
use strict;
use warnings;
use File::Basename;

my $root_dir = "/home/qzhao2/Dropbox/data/amazon_book_rating";
my $dict_file = $root_dir . "/feature_dict.csv";
my $user_json_file = $root_dir . "/author_profile.json";
my $item_json_file = $root_dir . "/item_profile.json";
my $rating_json_file = $root_dir . "/book_rating_filter.json";

# generate output file
-d dirname($dict_file) or die "invalid dictionary file path$!";
-f $user_json_file or die "user feature file does not exist $!";
-f $item_json_file or die "item feature file does not exist $!";
-f $rating_json_file or die "rating file does not exist $!";

my $user_feature_file = $user_json_file . "_feat.csv";
my $item_feature_file = $item_json_file . "_feat.csv";

# generate user and item feature file

my $cmd = "perl GenEntityFeature.pl --input=$user_json_file --dict=$dict_file --output=$user_feature_file --type=u";
print $cmd . "\n";
`$cmd`;

$cmd = "perl GenEntityFeature.pl --input=$item_json_file --dict=$dict_file --output=$item_feature_file --type=i";
print $cmd . "\n";
`$cmd`;


# generate purchase data from the ratings

my $purchase_json_file = $rating_json_file . "_purchase.json";
$cmd = "perl GenPurchase.pl --rating=$rating_json_file --ratio=1 --output=$purchase_json_file";
print $cmd . "\n";
`$cmd`;

# generate feature
my $purchase_feature_file = $purchase_json_file . "_feat.csv";

$cmd = "perl GenRatingFeature.pl --dict=$dict_file --user=$user_feature_file --item=$item_feature_file --rating=$purchase_json_file --output=$purchase_feature_file";
print $cmd . "\n";
`$cmd`;

# convert to libsmv format
my $libsvm_fmt_file = $purchase_json_file . "_feat.libsvm";
$cmd = "perl LibSVMFormat.pl --input=$purchase_feature_file --output=$libsvm_fmt_file";
print $cmd . "\n";
`$cmd`;
