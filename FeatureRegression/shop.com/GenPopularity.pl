#!/usr/bin/perl
use lib "../packages";
use strict;
use warnings;
use Dataset::Shopcom;
use JSON;
#
# generate the number of products for each category

my $order_data_file = '/data/jian-data/shop-data/processed/orderdata.gt5.lt200';
my $result_file = '/data/jian-data/shop-data/processed/item_popularity.json';

print ">>> load user order data\n";
my $user_order_map = Dataset::Shopcom::load_order_data($order_data_file);

open my $result_fh, ">", $result_file or die $!;
# category size

my %product_pop_map = ();
my %product_cat_map = ();
my %cat_max_map = ();
my %cat_pop_map = ();

# total size
my $max_product = 0;
my $max_category = 0;

while(my($uid, $orders) = each %$user_order_map){
	foreach(@$orders){
		my($uid1,$item_id,$ts,$cat) = @$_;
		# map product to category
		# top 2-level category
		$cat = join("|",(split /\|/,$cat)[0..1]);
		$product_cat_map{$item_id} = $cat;
		$product_pop_map{$item_id} ++;
		if($product_pop_map{$item_id} > $max_product){
			$max_product = $product_pop_map{$item_id};
		}
		$cat_pop_map{$cat} ++;
		if($cat_pop_map{$cat} > $max_category){
			$max_category = $cat_pop_map{$cat};
		}
		if(!$cat_max_map{$cat} or $product_pop_map{$item_id} > $cat_max_map{$cat}){
			$cat_max_map{$cat} = $product_pop_map{$item_id};
		}
	}
}

# normalized product popularity
#
my %norm_product_pop_map = ();
map {$norm_product_pop_map{$_} = $product_pop_map{$_} / $max_product} keys %product_pop_map;

# normalized product category popularity
my %norm_product_cat_pop_map = ();
map {$norm_product_cat_pop_map{$_} = $product_pop_map{$_} / $cat_max_map{$product_cat_map{$_}}} keys %product_pop_map;

# normalized category popularity
my %norm_cat_pop_map = ();
map {$norm_cat_pop_map{$_} = $cat_pop_map{$_} / $max_category} keys %cat_pop_map;

# write result to file

print ">>> write popularity to file: $result_file \n";
while(my($item_id, $pop) = each %norm_product_pop_map){
	my $p_cat_pop = $norm_product_cat_pop_map{$item_id};
	my $cat = $product_cat_map{$item_id};
	my $cat_pop = $norm_cat_pop_map{$cat};
	my $json = { 
		# product popularity
		"p_pop" => sprintf("%.4f",$pop),
		# product category popularity
		"pc_pop" => sprintf("%.4f",$p_cat_pop),
		# category popularity
		"c_pop" => sprintf("%.4f",$cat_pop),
		# product category
		"p_cat" =>$cat,
		# product id
		"id" => $item_id
	};
	print $result_fh encode_json($json) . "\n";
}
close $result_fh;
