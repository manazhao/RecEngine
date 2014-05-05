#!/usr/bin/perl
#
# generate movie names based on item id

use strict;
use warnings;
use JSON;

my $item_file = "/home/qzhao2/data/ml-1m/movies.dat";

open ITEM_FILE , "<", $item_file or die $!;

my %item_id_name_map = ();

while(<ITEM_FILE>){
	chomp;
	my @fields = split "\t";
	$item_id_name_map{$fields[0]} = $fields[1];
}

close ITEM_FILE;

while(<STDIN>){
	chomp;
	my $json_obj = decode_json($_);
	my $user_id = $json_obj->{"userId"};
	my $feature_names = join '|', @{$json_obj->{"featureNames"}};
	my $rec_item_ids = $json_obj->{"recNameList"};
	my @rec_item_names = ();
	foreach my $rec_item (@$rec_item_ids){
		my ($ent_type, $item_id) = split "_", $rec_item;
		my $item_name = $item_id_name_map{$item_id};
		push @rec_item_names, $item_name;	
	}
	# dump to command line
	print join("\t", ($user_id, $feature_names, join("|",@rec_item_names)))."\n";
}
