#!/usr/bin/perl
#
# convert product information file to json format
#
use strict;
use warnings;
use JSON;

while(<>){
	chomp;
	my($id, $product_id, $store_name, $product_name, $description, $price, $tmp1,$tmp2, $category, $attribute) = split "\t";
	my $product_json = { "id" => $id,"p" => $price, "c" => $category };
	print encode_json($product_json) . "\n";
}
