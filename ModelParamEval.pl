#!/usr/bin/perl


# Author: Qi Zhao (manazhao@gmail.com)
# Date: April 21th 2014

# evaluate Hierarchical Hybrid Matrix Factorization Model (HHMF) under different latent vector dimensionality
# the result for each parameter setting is dumped to separate file.
# usage: perl ModelParamEval.pl at the project root directory

use strict;
use warnings;

my @lat_dim = (5,10,20,50);
my @use_feature = (0,1);
my $max_iter = 50;

-d 'amazon_result' or  mkdir 'amazon_result';

foreach (@lat_dim){
my $dim = $_;
foreach(@use_feature){
	my $feature = $_;
	my $resultFile = "amazon_result/dim_$dim-$feature.txt";
	my $cmd = "./RecAlgorithmReleaseBuild/RecEngine --lat-dim=$dim --max-iter=$max_iter --use-feature=$feature >$resultFile 2>&1 ";
	print "$cmd\n";
	`$cmd`;
}
}

