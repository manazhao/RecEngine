#!/usr/bin/perl
# extract all features given an item
#
use strict;
use warnings;

while(<STDIN>){
	chomp;
	my ($entity_id, @features) = split /\,/;
	# get all features, detect duplication
	my %feat_map = ();
	foreach my $feature (@features){
		my ($fid,$val) = split /\:/, $feature;
		exists $feat_map{$fid} and die "duplicate feature - $fid: $!";
		print join(",",($fid,$val)). "\n";
	}	
}
