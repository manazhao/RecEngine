#!/usr/bin/perl
#
use strict;
use warnings;

my %freq_cnt_map = ();
while(<>){
	chomp;
	my ($item_id,@sims) = split /\,/;
	foreach(@sims){
		my($item_id,$cnt) = split /\:/;
		$freq_cnt_map{$cnt}++;
	}
}

my @sort_freqs = sort {$freq_cnt_map{$b} <=> $freq_cnt_map{$a}} keys %freq_cnt_map;
foreach(@sort_freqs){
	print join(",",($_,$freq_cnt_map{$_})) . "\n";
}
