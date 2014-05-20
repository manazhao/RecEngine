#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

my @gender = ("Male","Female");
my @age = (25,35,45);

foreach my $gender (@gender){
    foreach my $age (@age){
        my $user_id = join("_", ($gender,$age));
        my $profile = { "id" => $user_id, "age" => $age, "gender" => $gender};
        print encode_json($profile) . "\n";
    }
}
