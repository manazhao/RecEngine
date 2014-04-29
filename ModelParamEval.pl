#!/usr/bin/perl


# Author: Qi Zhao (manazhao@gmail.com)
# Date: April 21th 2014

# evaluate Hierarchical Hybrid Matrix Factorization Model (HHMF) under different latent vector dimensionality
# the result for each parameter setting is dumped to separate file.
# usage: perl ModelParamEval.pl at the project root directory

use strict;
use warnings;

my @lat_dim = (10,20);
my @use_feature = (0,1);
my $max_iter = 20;
my $dataset = "amazon";
my $result_dir = "$dataset-result";
-d $result_dir  or  mkdir $result_dir;

foreach (@lat_dim){
my $dim = $_;
foreach(@use_feature){
	my $feature = $_;
	my $resultFile = "$result_dir/dim_$dim-$feature.txt";
	my $cmd = "./RecAlgorithmReleaseBuild/RecModel --dataset-name $dataset  --data-host localhost --data-port 9090 --lat-dim=$dim --max-iter=$max_iter --use-feature=$feature  --model HHMF --model-sel >$resultFile 2>&1 ";
	print "$cmd\n";
	`$cmd`;
}
}

