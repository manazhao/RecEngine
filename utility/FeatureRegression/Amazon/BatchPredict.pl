#!/usr/bin/perl
#
use warnings;
use strict;
use Getopt::Long;

my $pred_folder;
my $model_file;

GetOptions("pred=s" => \$pred_folder, "model=s" => \$model_file) or die $!;

($pred_folder and -d $pred_folder) or die $!;
($model_file and -f $model_file) or die $!;

# list prediction files
#
opendir(my $dh, $pred_folder) || die "can't opendir $pred_folder: $!";

my @pred_files = grep { /libsvm$/ && -f "$pred_folder/$_" } readdir($dh);
closedir $dh;

my $home_dir = $ENV{"HOME"};
my $item_file = $home_dir . "/Dropbox/data/amazon_book_rating/items.list";
-f $item_file or die $!;

foreach (@pred_files){
    my $tmp_file = $pred_folder . "/" . $_;
    my $pred_result_file = $tmp_file . ".pred";
    my $cmd = "liblinear_predict -b 1 $tmp_file $model_file $pred_result_file";
    print $cmd . "\n";
    `$cmd`;
    # now sort the result
    my $sort_result = $pred_result_file . ".sort";
    $cmd = "perl SortPredict.pl --pred=$pred_result_file --item=$item_file --result=$sort_result";
    print $cmd . "\n";
    `$cmd`;
    # generate recommendation list as html
    
    $cmd = "perl GenerateAmazonBanner.pl --pred=$sort_result --max=50";
    print $cmd . "\n";
    `$cmd`;
}


