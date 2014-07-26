#!/usr/bin/perl
#
use strict;
use warnings;
use JSON;

while(<>){
	chomp;
	my $json_obj = decode_json($_);
	print join(",",values %$json_obj)."\n";
}
