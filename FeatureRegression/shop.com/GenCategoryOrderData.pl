#!/usr/bin/perl
# map each product to its finest category 
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;
use Data::Dumper;

my $orderdata_file;
my $item_file;
my $category_map_file;
my $item_category_file;
my $category_order_file;

GetOptions("order=s" => \$orderdata_file, "item=s" => \$item_file, "category_map=s" => \$category_map_file, "item_map=s" => \$item_category_file, "result=s" => \$category_order_file) or die $!;

$orderdata_file and $item_file and  $category_map_file and $item_category_file and $category_order_file or die $!;

-f $orderdata_file  and -f $item_file and dirname($item_category_file) and -d dirname($category_map_file) and -d dirname($category_order_file) or die $!;

open ITEM_FILE, "<" , $item_file or die $!;
my %item_category_map = ();
my %category_id_map = ();

my $max_cat_id = 0;

while(<ITEM_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $item_id = $json_obj->{"id"};
	my $item_category = $json_obj->{"c"};
	if(not exists $category_id_map{$item_category}){
		# add to the category map
		$max_cat_id++;
		$category_id_map{$item_category}  = $max_cat_id;
	}
	# now add to product category map
	if(not exists $item_category_map{$item_id}){
		$item_category_map{$item_id} = $category_id_map{$item_category};
	}
}

close ITEM_FILE;
open CATEGORY_FILE, ">", $category_map_file or die $!;
while(my($name,$id) = each %category_id_map){
	print CATEGORY_FILE join(",",($name,$id)) . "\n";
}
close CATEGORY_FILE;
open ITEM_CATEGORY_FILE, ">", $item_category_file or die $!;
while(my($item_id,$category_id) = each %item_category_map){
	print ITEM_CATEGORY_FILE join(",",($item_id,$category_id)) . "\n";
}
close ITEM_CATEGORY_FILE;

open ORDER_FILE, "<", $orderdata_file or die $!;
open RESULT_FILE, ">", $category_order_file or die $!;
while(<ORDER_FILE>){
	chomp;
	my @fields = split /\t/, $_;
	$fields[2] = $item_category_map{$fields[2]};
	print RESULT_FILE join(",",@fields) . "\n";
}

close ORDER_FILE;


