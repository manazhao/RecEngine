#!/usr/bin/perl
#
# convert libsvm data format to sparse matrix format
#
use strict;
use warnings;

my $input_file = '/data/jian-data/shop-data/mlr/orderdata.gt5.lt200.train.libsvm';
my $output_x_file = '/data/jian-data/shop-data/mlr/orderdata.gt5.lt200.train.glmnet.x.csv';
my $output_y_file = '/data/jian-data/shop-data/mlr/orderdata.gt5.lt200.train.glmnet.y.csv';
open INPUT_FILE, "<" , $input_file or die $!;
open X_FILE, ">", $output_x_file or die $!;
open Y_FILE, ">", $output_y_file or die $!;

my $row_idx = 1;
while(<INPUT_FILE>){
	chomp;
	my($weight,$class,@feats) = split /\s+/;
	next if scalar @feats == 0;
	# skip empty features
	# next if not scalar @feats;
	foreach(@feats){
		my($fid,$fval) = split /\:/;
		print X_FILE join(",",($row_idx,$fid,$fval)) . "\n";
	}
	print Y_FILE $class . "\n";
	$row_idx++;
}

close INPUT_FILE;
close X_FILE;
close Y_FILE;
