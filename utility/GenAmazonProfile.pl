#!/usr/bin/perl
#
# generate user demographic profile 
use strict;
use warnings;
use JSON;

my @gender_features = ("", "gender_male", "gender_female");
my @age_features = ("","age_4","age_6","age_8");

my $user_prefix = "test_amazon_user_";
my $user_idx = 0;
# generate interaction features
foreach my $gender (@gender_features){
	foreach my $age(@age_features){
		my %profile = ("id" => $user_prefix . $user_idx, "features" => [$gender,$age]);
		$user_idx++;
		print encode_json(\%profile) . "\n";
}
}

