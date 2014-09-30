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
my $repurchase_result_file;
my $result_file;
my $cat_level = 2;

GetOptions("order=s" => \$orderdata_file, "item=s" => \$item_file, "rp-result=s" => \$repurchase_result_file, "cc-result=s" => \$result_file, "level=i" => \$cat_level) or die $!;

$orderdata_file and $item_file and $result_file or die $!;
-f $orderdata_file and -f $item_file and -d dirname($repurchase_result_file) and -d dirname($result_file) or die $!;

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
	if($cat_level > 0){
		$item_category = join("|",@cats[0..$cat_level - 1]);
	}
	if(not exists $item_category_map{$item_id}){
		$item_category_map{$item_id} = $item_category;
	}
}
close ITEM_FILE;


# go through each user

my %time_gap_map = ();
my %cat_repurchase_map = ();

print ">>> start to generate product-to-product time gap information...\n";
use constant DAY_SECONDS => 3600 * 24;

while(my($user_id,$orders) = each %$user_order_map){
	# $orders are sorted by time in increasing order
	# only keep baby category orders
	# record the timestamps of the occurences of each category
	my %cat_time_map = ();
	foreach(@$orders){
		my ($user_id,$item_id,$ts) = @$_;
		my $item_cat = $item_category_map{$item_id};
		# only keep orders under baby category
		if($item_cat =~ m/^Baby/g){
			$cat_time_map{$item_cat}->{$ts} = 1;
		}
	}
	# now generate the time gap information
	# construct cat-cat pairs
	my @cats = keys %cat_time_map;
	for(my $i = 0; $i < scalar @cats; $i++){
		my $cati = $cats[$i];
		# order timestamps for $cati
		my @cati_timestamps = sort keys %{$cat_time_map{$cati}};
		# generate repurchase period for each category
		for(my $j = 0; $j < $#cati_timestamps; $j++){
			my $tmp_diff = $cati_timestamps[$j+1] - $cati_timestamps[$j];
			# express in day basis
			my $tmp_diff_day = sprintf("%.3f",$tmp_diff/DAY_SECONDS);
			push @{$cat_repurchase_map{$cati}}, $tmp_diff_day;
		}
		# time difference between the first purchasement of each category
		my $cati_first_ts = $cati_timestamps[0];
		for(my $j = $i + 1; $j < scalar @cats; $j++){
			my $catj = $cats[$j];
			# first occurence of purchasing category j
			my @catj_timestamps = sort keys %{$cat_time_map{$catj}};
			my $catj_first_ts = $catj_timestamps[0];
			my $key = join("-",($cati,$catj));
			my $first_ts_diff = sprintf("%.3f",($catj_first_ts - $cati_first_ts)/DAY_SECONDS);
			push @{$time_gap_map{$key}},$first_ts_diff;
			$key = join("-",($catj,$cati));
			push @{$time_gap_map{$key}}, -$first_ts_diff;

		}
	}

}

open RP_FILE, ">" , $repurchase_result_file or die $!;
while(my($key,$value) = each %cat_repurchase_map){
	my @sort_ts = sort @$value;
	print RP_FILE join(",",($key,@sort_ts))."\n";
}
close RP_FILE;

open RESULT_FILE, ">", $result_file or die $!;
while(my($key,$value) = each %time_gap_map){
	my @sort_ts = @$value;
	print RESULT_FILE join(",",($key,@sort_ts))."\n";
}
close RESULT_FILE;
