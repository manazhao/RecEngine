#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

# generate JSON objects for user feature
#
my $data_root = "/home/manazhao/data/ml-1m";
my $user_file = $data_root . "/users.dat";
my $user_json_file = $data_root . "/users.json";
open USER_FILE, "<", $user_file or die $!;
open JSON_FILE, ">", $user_json_file or die $!;

my @feature_name = ("gender","age","occupation","zip");
while(<USER_FILE>){
	chomp;
	my @fields = split '::';
	my $user_id = $fields[0];
	# generate the json
	my %user_features_map = ("id" => $user_id);
	@user_features_map{@feature_name} = @fields[1..$#fields];
	my $json_str = encode_json(\%user_features_map);
	print JSON_FILE $json_str . "\n";
}

close USER_FILE;
close JSON_FILE;

