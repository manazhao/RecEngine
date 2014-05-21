#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

my @gender = ("Male","Female");
my @age = ("");

foreach my $gender (@gender){
    foreach my $age (@age){
        my $user_id = join("_", ($gender,$age));
        my $profile = { "id" => $user_id };
	$profile->{"age"} = $age if $age ne "";
	$profile->{"gender"} = $gender if $gender ne "";
        print encode_json($profile) . "\n";
    }
}
