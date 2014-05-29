#!/usr/bin/perl
# convert rating in TSV format to json format which is required for existing procedure
use strict;
use warnings;
use JSON;

while(<STDIN>){
	chomp;
	my($user_id,$item_id,$rating,$r_date) = split /\s+/;
	print encode_json({ "u" => $user_id, "i" => $item_id, "r" => $rating, "d" => $r_date}) . "\n";
}

