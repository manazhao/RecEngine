#!/usr/bin/perl
#
use strict;
use warnings;

# run cross-validation using Logistic Regression under different cost paramters
#
my @c_param_vals = (0.01,0.1,1,2,4);
#my @c_param_vals = (8,16,32,64);
#my @c_param_vals = (4);
my $train_cmd = "ll_train";
#my $home_dir = $ENV{"HOME"};
my $home_dir = "";
my $data_root = "/data/jian-data/shop-data/mlr";
my $result_root = $data_root . "/train";
my $train_file = $data_root . "/orderdata.gt5.lt200.train.nw.libsvm";
 my $weight_file = $data_root. "/orderdata.gt5.lt200.train.w";
my $num_cv = 5;
my $epsilon = 0.01;

foreach my $c_param (@c_param_vals){
	my $log_file = $result_root . "/train_lr_c_$c_param" . ($num_cv == 0? "model":"") . ".log";
	my $cv_opt = ($num_cv == 0 ? "" :"-v $num_cv");
	my $model_file = $result_root . "/lr_$c_param.model";
	my $cmd = $train_cmd . " -s 6 -B 1 -c $c_param -e $epsilon $cv_opt  -W $weight_file $train_file $model_file 1>$log_file 2>&1  ";
	print $cmd . "\n";
	`$cmd`;
}


