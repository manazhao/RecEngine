#!/usr/bin/perl
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;

my $item_file;
# category file
my $index_file;
my $result_file;

GetOptions("item=s" => \$item_file, "index=s" => \$index_file, "result=s" => \$result_file) or die $1;

my $app_name = basename($0);
$item_file and $index_file and $result_file or die "$0 --item=<item json file> --index=<category name, id mapping> --result=<category id and item id mapping>:$!";

-f $item_file and -d dirname($index_file) and -d dirname($result_file) or die $!;

open ITEM_FILE, "<", $item_file or die $!;
open INDEX_FILE, ">" , $index_file or die $!;
open RESULT_FILE, ">" , $result_file or die $!;

my %cat_item_map = ();
while(<ITEM_FILE>){
	chomp;
	my $item_json = decode_json($_);
	my $item_id = $item_json->{"id"};
	if(exists $item_json->{"c"}){
		push @{$cat_item_map{$item_json->{"c"}}} , $item_id;
	}
}

close ITEM_FILE;

my $cat_index = 1;
while(my($cat_name, $items) = each %cat_item_map){
	print INDEX_FILE join(",",($cat_name,$cat_index)) . "\n";	
	print RESULT_FILE join(",",($cat_index,join(":",@$items))) . "\n";
	$cat_index++;
}
close INDEX_FILE;
close RESULT_FILE;


