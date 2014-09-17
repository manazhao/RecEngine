#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

my $file = '/data/jian-data/shop-data/processed/gt5_lt200_products';

open my $fh, "<", $file or die $!;
my %product_map = ();
while(<$fh>){
	chomp;
	$product_map{$_} = 1;
}
close $fh;

while(<>){
	chomp;
	my $line = $_;
	my $json_obj = decode_json($_);
	next unless exists $product_map{$json_obj->{id}};
	print $line . "\n";
}
