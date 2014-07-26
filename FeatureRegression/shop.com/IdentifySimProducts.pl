#!/usr/bin/perl
#
# identify similar products witin the same category
#
use strict;
use warnings;

use lib "../packages";
use Dataset::TermVectorLoader;
use Dataset::Shopcom;

my $product_file = '/data/jian-data/shop-data/orderdata.txt.productIndex';
-f $product_file or die $!;

# indri index loader parameters
my $title_index = "/data/jian-data/shop-data/product_nlp/index/title";
my $desc_index = "/data/jian-data/shop-data/product_nlp/index/desc";
my $title_result = "/data/jian-data/shop-data/product_nlp/result/title";
my $desc_result = "/data/jian-data/shop-data/product_nlp/result/desc";

my $title_sim_result = "/data/jian-data/shop-data/product_nlp/result/title_sim.csv";
my $desc_sim_result = "/data/jian-data/shop-data/product_nlp/result/desc_sim.csv";

open my $title_sim_fh , ">", $title_sim_result or die $!;
open my $desc_sim_fh, ">", $desc_sim_result or die $!;

my $title_indri_index = Dataset::TermVectorLoader->new( index => $title_index, result => $title_result);
$title_indri_index->read();
#my $desc_indri_index = Dataset::TermVectorLoader->new( index => $desc_index, result => $desc_result);
#$desc_indri_index->read();

my %cat_product_map = ();
my %product_map = ();


print ">>> load product information..\n";
open my $product_fh, "<", $product_file or die $!;

while(<$product_fh>){
	# ProdID, CatalogName, OrderText, Description , LocalPrice,  Sizes , Colors,  Category ,  Attributes
	my($pid, $orig_pid, $cln, $ot, $d, $lp, $s, $colors, $cat, $attr) = split /\t/;
	$product_map{$pid}->{title} = $ot;
	push @{$cat_product_map{$cat}}, $pid;
}

close $product_fh;

# calculate product similarity within each category
my %prod_sim_map = ();
print ">>> calculate product similarity within each category\n";
my $cnt = 0;
while(my($cname,$products) = each %cat_product_map){
	my $num_products = scalar @$products;
	# next if $num_products < 10;
	print "processing category: $cname with " . $num_products . " products\n";
	for(my $i = 0; $i < scalar @$products; $i++){
		my $prod_i = $products->[$i];
		for(my $j = $i + 1; $j < scalar @$products; $j++){
			my $prod_j = $products->[$j];
			my $title_sim = sprintf("%.3f",$title_indri_index->cosine_score($prod_i,$prod_j));
			# my $desc_sim = $desc_indri_index->cosine_score($prod_i,$prod_j);
			$prod_sim_map{$prod_i}->{$prod_j} = $title_sim;
			$prod_sim_map{$prod_j}->{$prod_i} = $title_sim;
		}
	}	
	#last if $cnt++ >= 10;
}

# write to file
print ">>> write product similarity result to file: $title_sim_result\n";
while(my($pid,$sim_map) = each %prod_sim_map){
	# sort by score in descending order
	my @other_prods = keys %$sim_map;
	my @sorted_prods = sort {$sim_map->{$b} <=> $sim_map->{$a}} @other_prods;
	my @sorted_scores = map {$sim_map->{$_} } @sorted_prods;
	print $title_sim_fh join(",",($pid,@sorted_prods, @sorted_scores)) . "\n";
}

close $title_sim_fh;




