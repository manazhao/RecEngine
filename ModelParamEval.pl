#!/usr/bin/perl


# Author: Qi Zhao (manazhao@gmail.com)
# Date: April 21th 2014

# evaluate Hierarchical Hybrid Matrix Factorization Model (HHMF) under different latent vector dimensionality
# the result for each parameter setting is dumped to separate file.
# usage: perl ModelParamEval.pl at the project root directory

use strict;
use warnings;

my @lat_dim = (5,10,20,50);
my $max_iter = 50;

-d 'result' or  mkdir 'result';

foreach (@lat_dim){
my $dim = $_;
my $resultFile = "result/dim_$dim.txt";
my $cmd = "./RecAlgorithmReleaseBuild/RecEngine --lat-dim=$dim --max-iter=$max_iter >$resultFile 2>&1 ";
print "$cmd\n";
`$cmd`;
}

