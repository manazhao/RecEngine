#!/usr/bin/perl
#
# get category item popularity distribution for each category
#
use strict;
use warnings;

my %cip_map = ();

while(<>){
	chomp;
	my ($item_id, @feats) = split /\,/;
	# skip 
	next unless @feats;
	foreach(@feats){
		my($fid,$fval) = split /\:/;
		push @{$cip_map{$fid}}, $fval;
	}
}

while(my($fid,$fvals) = each %cip_map){
	print join(",",($fid,join(":",@$fvals)))."\n";
}
