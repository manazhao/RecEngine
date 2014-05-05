#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

# generate JSON objects for user feature
#
my $data_root = "/home/manazhao/data/ml-1m";
my $item_file = $data_root . "/movies.dat";
my $item_json_file = $data_root . "/movies.json";
open ITEM_FILE, "<", $item_file or die $!;
open JSON_FILE, ">", $item_json_file or die $!;

while(<ITEM_FILE>){
	chomp;
	my ($id, $name, $genre) = split '::';
	my %feature_map = ("id" => $id, "name" => $name);
	# split $genre
	my @genres = split '\|', $genre;
	$feature_map{"genres"} = \@genres;
	print JSON_FILE encode_json(\%feature_map) . "\n";
}

close ITEM_FILE;
close JSON_FILE;

