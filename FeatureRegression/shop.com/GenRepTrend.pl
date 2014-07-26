#!/usr/bin/perl
use lib "../packages";
use Dataset::Shopcom;
use strict;
use warnings;
use JSON;
#
# generate product repurchase tendency

my $order_data_file = '/data/jian-data/shop-data/processed/orderdata.gt5.lt200';
my $result_file = '/data/jian-data/shop-data/processed/item_rep_tend.json';
# load order data
#
print ">>> load user order data\n";
my $user_order_map = Dataset::Shopcom::load_order_data($order_data_file);

# product repurchase tendency
my %product_rep_map = ();
my %product_ucnt_map = ();
my %cat_rep_map = ();
my %cat_ucnt_map = ();
my %product_cat_map = ();


while(my($uid, $orders) = each %$user_order_map){
	my %rep_map = ();
	my %tmp_cat_rep_map = ();
	foreach(@$orders){
		my($uid1,$item_id,$ts,$cat) = @$_;
		# top 2-level category
		$cat = join("|", (split /\|/, $cat)[0..1]);
		$product_cat_map{$item_id} = $cat;
		$rep_map{$item_id} = 1;	
		$product_rep_map{$item_id}++;
		$tmp_cat_rep_map{$cat} = 1;
		$cat_rep_map{$cat} ++;	
	}
	map {$product_ucnt_map{$_} += 1} keys %rep_map;
	map {$cat_ucnt_map{$_} += 1} keys %tmp_cat_rep_map;
}

# devide each product count by the number of users

map {$product_rep_map{$_} *= (log($product_ucnt_map{$_})/$product_ucnt_map{$_})} keys %product_rep_map;
map {$cat_rep_map{$_} *= (log($cat_ucnt_map{$_})/$cat_ucnt_map{$_})} keys %cat_rep_map;

# normalize the repurchase tendency
my $max_product_rep = 0;
my $max_cat_rep = 0;
map {  ($product_rep_map{$_} > $max_product_rep) and $max_product_rep = $product_rep_map{$_} } keys %product_rep_map;
map {  ($cat_rep_map{$_} > $max_cat_rep) and $max_cat_rep = $cat_rep_map{$_} } keys %cat_rep_map;
# normalize
map {  ($product_rep_map{$_} /= $max_product_rep) } keys %product_rep_map;
map {  ($cat_rep_map{$_} /= $max_cat_rep) } keys %cat_rep_map;

print ">>> write the product repurchase tendency information\n";
open my $fh, ">", $result_file or die $!;
while(my($item_id, $item_rep) = each %product_rep_map){
	my $cat = $product_cat_map{$item_id};
	my $cat_rep = $cat_rep_map{$cat};
	my $json = {
		id => $item_id,
		cat_rep => sprintf("%.4f",$cat_rep),
		rep => sprintf("%.4f",$item_rep),
		p_cat => $cat
	};
	print $fh encode_json($json)."\n";
}

close $fh;


