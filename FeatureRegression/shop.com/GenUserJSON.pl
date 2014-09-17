#/usr/bin/perl
# generate user json profile file
#
use strict;
use warnings;
use JSON;
while(<>){
	chomp;
	print encode_json({"id" => $_})."\n";
}

