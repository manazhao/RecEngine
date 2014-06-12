#!/usr/bin/perl
#
use strict;
use warnings;

# run cross-validation using Logistic Regression under different cost paramters
#
 my @c_param_vals = (0.001,0.01,0.1,1,2,4,8,16,32,64);
# my @c_param_vals = (64);
my $train_cmd = "liblinear_train";
my $home_dir = $ENV{"HOME"};
my $data_root = $home_dir . "/Dropbox/data/amazon_book_rating";
my $result_root = $data_root . "/popularity_result2";
my $train_file = $result_root . "/rating_purchase_feat.libsvm";
my $model_file = $result_root . "/lr.model";
my $num_cv = 0;
my $epsilon = 0.01;

foreach my $c_param (@c_param_vals){
	my $log_file = $result_root . "/train_lr_c_$c_param.log";
	my $cv_opt = ($num_cv == 0 ? "" :"-v $num_cv");
	my $cmd = "time " .  $train_cmd . " -s 6 -B 1 -c $c_param -e $epsilon $cv_opt $train_file $model_file 1>$log_file 2>&1 ";
	print $cmd . "\n";
	`$cmd`;
}


