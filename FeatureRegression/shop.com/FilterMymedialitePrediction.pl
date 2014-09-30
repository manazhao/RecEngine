#!/usr/bin/perl
#
use strict;
use warnings;

# number of products to keep
my $num_to_keep = 200;

while(<>){
	chomp;
	my($user_id,$predictions) = split /\s+/;
	# split the predictions
	$predictions =~ s/\[|\]//g;
	my @fields = split /\,/,$predictions;
	next if scalar @fields == 0;
	print join(",",($user_id,@fields[0..$num_to_keep - 1])) . "\n";
}
