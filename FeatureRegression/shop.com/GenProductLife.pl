#!/usr/bin/perl # generate product-to-product time gap information use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;
use Data::Dumper;
# need to use Shopcom module function
use lib  '../packages';
use Dataset::Shopcom;

my $orderdata_file;
my $item_file;
my $result_file;
my $level = 0;

GetOptions("order=s" => \$orderdata_file,"item=s" => \$item_file,  "result=s" => \$result_file, "level=i" => \$level) or die $!;
$orderdata_file and $item_file and $result_file or die $!;
-f $orderdata_file and -f $item_file and -d dirname($result_file) or die $!; 

print ">>> load order data...\n";
my $user_order_map = Dataset::Shopcom::load_order_data($orderdata_file);


my %item_category_map = ();
print ">>> read in item file...\n";
open ITEM_FILE, "<" , $item_file or die $!;
while(<ITEM_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $item_id = $json_obj->{"id"};
	my $item_category = $json_obj->{"c"};
	# remove ,
	$item_category =~ s/\,//g;
	# remove space around &
	$item_category =~ s/\s+\&\s+/\&/g;
	# now add to product category map
	my @cats = split /\|/, $item_category;
	# only takes the 2 most upper categories
	if($level > 0){
		$item_category = join("|",@cats[0..$level - 1]);
	}
	$item_category_map{$item_id} = $item_category;
}
close ITEM_FILE;

# product life-span
my $product_life_map = ();
print ">>> start to generate product life span information...\n";
use constant DAY_SECONDS => 3600 * 24;

while(my($user_id,$orders) = each %$user_order_map){
	# $orders are sorted by time in increasing order
	# only keep baby category orders
	my %tmp_product_life_map = ();
	foreach(@$orders){
		my ($user_id,$item_id, $ts) = @$_;
		my $category = $item_category_map{$item_id};
		if(not exists $tmp_product_life_map{$category}){
			$tmp_product_life_map{$category}->[0] = $ts;
		}else{
			$tmp_product_life_map{$category}->[1] = $ts;
		}
	}
	# get the life span for each category
	while(my($category,$life_span) = each %tmp_product_life_map){
		# make sure start and end time
		@$life_span < 2 and next;
		my($start,$end) = @$life_span;
		# remove products ordered at the same time
		$end == $start and next;
		push @{$product_life_map{$category}}, sprintf("%.3f",($end - $start)/DAY_SECONDS);
	}
}

open RESULT_FILE, ">", $result_file or die $!;
while(my($key,$value) = each %product_life_map){
	print RESULT_FILE join(",",($key,@$value))."\n";
}
close RESULT_FILE;
