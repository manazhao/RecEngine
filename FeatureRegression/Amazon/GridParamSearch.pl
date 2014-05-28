#!/usr/bin/perl
#
use strict;
use warnings;

# run cross-validation using Logistic Regression under different cost paramters
#
# my @c_param_vals = (0.001,0.01,0.1,1,2);
my @c_param_vals = (4,8);
my $train_cmd = "liblinear_train";
my $home_dir = $ENV{"HOME"};
my $data_root = $home_dir . "/Dropbox/data/amazon_book_rating";
my $train_file = $data_root . "/book_rating_filter.json_purchase.json_feat.libsvm";
my $model_file = $train_file . "_lr.model";
my $num_cv = 5;
my $epsilon = 0.001;

foreach my $c_param (@c_param_vals){
	my $log_file = $data_root . "/train_lr_c_$c_param.log";
	my $cv_opt = ($num_cv == 0 ? "" :"-v $num_cv");
	my $cmd = "nohup ". $train_cmd . " -s 6 -B 1 -c $c_param -e $epsilon $cv_opt $train_file $model_file 1>$log_file 2>&1 &";
	print $cmd . "\n";
	`$cmd`;
}

