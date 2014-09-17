#!/usr/bin/perl
#
# ranking category by their purchasing order
#
use strict;
use warnings;
use Data::Dumper;

# score for each category
my %cat_score_map = ();
my %pair_time_map = ();

while(<>){
	chomp;
	my($cat_pair,@ts) = split /\,/;
	my ($from_cat,$to_cat) = split /\-/, $cat_pair;
	my $score = 0;	
	# get the number of positive time gaps
	map { $_ >= 0 and $score++} @ts;
	$score /= scalar @ts;
	$pair_time_map{$cat_pair} = $score;
	$cat_score_map{$to_cat} += $score;
}

my @sorted_cats = sort {$cat_score_map{$a} <=> $cat_score_map{$b}} keys %cat_score_map;

#print Dumper(\%pair_time_map);
if(0){
	$cat_score_map{$sorted_cats[0]} = 0;
	for(my $i = 1; $i < scalar @sorted_cats; $i++){
		my $prev_cat = $sorted_cats[$i-1];
		my $cur_cat = $sorted_cats[$i];
		my $time_gap = $pair_time_map{join("-",($prev_cat,$cur_cat))};
		$cat_score_map{$cur_cat} = $time_gap ;
	}
#	@sorted_cats = sort {$cat_score_map{$a} <=> $cat_score_map{$b}} keys %cat_score_map;
}

foreach my $cat(@sorted_cats){
	my $score = $cat_score_map{$cat};
	print join(",",($cat,sprintf("%.2f",$score))) . "\n";
}
