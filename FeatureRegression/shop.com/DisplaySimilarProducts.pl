#!/usr/bin/perl
#
# display similar products identified by IdentifySimilarProducts.pl
#
use strict;
use warnings;

my $product_file = '/data/jian-data/shop-data/orderdata.txt.productIndex';

while(<>){
	chomp;
	my($query_pid, @matched_products) = split /\,/;
	my $num_matched = int($#matched_products / 2);
	my @matched_pids = @matched_products[0 .. $num_matched];
	my @matched_scores = @matched_products[$num_matched + 1 .. $#matched_products];
	# add score of query product to itself
	@matched_scores = (1,@matched_scores);
	# pid is the line number of the product file
	my $all_pids = join(",", ($query_pid, @matched_pids));
	my $cmd = "echo $all_pids | tr ',' '\n' | xargs -I {} sed -n \"{}p\" $product_file | cut -f1,4";
	my $output = `$cmd`;
	my @lines = split /\n/, $output;
	my @merged_lines = map { join("\t",($lines[$_],$matched_scores[$_])) } 0 .. $#lines;
	print "=============================================================================\n";
	print join("\n",@merged_lines) . "\n";
}
