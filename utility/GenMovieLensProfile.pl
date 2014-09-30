#!/usr/bin/perl
#
# generate user demographic profile 
use strict;
use warnings;
use JSON;

my @age_features = ("", "age_1","age_18","age_35","age_45","age_50");
my @gender_features = ("", "gender_F", "gender_M");

my @all_features = (@age_features, @gender_features);
# generate unigram features

my $user_prefix = "test_fake_user_";
my $user_idx = 0;
# generate interaction features
foreach my $gender (@gender_features){
	foreach my $age(@age_features){
		my %profile = ("id" => $user_prefix . $user_idx, "features" => [$gender,$age]);
		$user_idx++;
		print encode_json(\%profile) . "\n";
}
}

