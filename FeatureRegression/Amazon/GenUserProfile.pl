#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

my @gender = ("","Male","Female");
my @age = ("","20","30","40","50","60");

foreach my $gender (@gender){
    foreach my $age (@age){
        my $user_id = join("_", ($gender,$age));
        my $profile = { "id" => $user_id };
	$profile->{"age"} = $age if $age ne "";
	$profile->{"gender"} = $gender if $gender ne "";
        print encode_json($profile) . "\n";
    }
}
