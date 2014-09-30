#!/usr/bin/perl
#
# generate item-item similarity matrix
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use Hash::PriorityQueue;
use Data::Dumper;

my $orderdata_file;
my $result_file;
GetOptions("orderdata=s" => \$orderdata_file, "result=s" => \$result_file) or die $!;
$orderdata_file and $result_file or die $!;
-f $orderdata_file and -d dirname($result_file) or die $!;

my %item_user_map = ();
open ORDER_FILE, "<", $orderdata_file or die $!;

while(<ORDER_FILE>){
	chomp;
	my($user_id, $date, $item_id, $quantity,$price, $cat) = split /\,/;
	$item_user_map{$item_id}->{$user_id} ++;
}

close ORDER_FILE;


# evaluate item similarity

my @sort_item_ids = sort keys %item_user_map;

my %item_sim_map = ();
my $num_top = 50;

open RESULT_FILE, ">", $result_file or die $!;
for(my $i = 0; $i < @sort_item_ids; $i++){
	my $item_id1= $sort_item_ids[$i];
	my @item1_users = keys %{$item_user_map{$item_id1}};
	# keep item similarity
	my $prior = Hash::PriorityQueue->new();
	my %sim_map = ();
	my $cnt = 0;
	for (my $j = 0; $j < @sort_item_ids; $j++){
		my $item_id2 = $sort_item_ids[$j];
		my $item2_user_map = $item_user_map{$item_id2};	
		# evaluate the number of overlapping users
		my $num_common = 0;
		foreach my $user1(@item1_users){
			if(exists($item2_user_map->{$user1})){
				$num_common++;
			}
		}
		$cnt++;
		$prior->insert($item_id2,$num_common);
		if($cnt > $num_top){
			my $tmp_item_id = $prior->pop();
		}
		$sim_map{$item_id2} = $num_common;
	}
	# dump for verification
	print RESULT_FILE $item_id1;
	while(my $item_id = $prior->pop()){
		print RESULT_FILE "," . $item_id . ":" . $sim_map{$item_id};
	}
	print RESULT_FILE "\n";
}

