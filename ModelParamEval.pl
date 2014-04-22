#!/usr/bin/perl

use strict;
use warnings;

my @lat_dim = (5,10,20,50);
my $max_iter = 20;

-d 'result' or  mkdir 'result';

foreach (@lat_dim){
my $dim = $_;
my $resultFile = "result/dim_$dim.txt";
my $cmd = "./RecAlgorithmReleaseBuild/RecEngine --lat-dim=$dim --max-iter=20 >$resultFile 2>&1 ";
print "$cmd";
`$cmd`;
}

