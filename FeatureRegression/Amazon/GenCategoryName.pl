#!/usr/bin/perl
#
# generate category name <-> id mapping
#
use strict;
use warnings;
use JSON;
use Data::Dumper;

my %id_name_map = ();
while(<>){
	chomp;
	my $item_json = decode_json($_);
	if($item_json->{"c"}){
		my $cat_str = $item_json->{"c"};
		# split by |
		my @cat_strs = split /\|/, $cat_str;
		foreach my $tmp_cat (@cat_strs){
			# split by /
			my @sub_cats = split /\//,$tmp_cat;
			foreach(@sub_cats){
				my($id,$name) = split /\-/;
				$id_name_map{$id} = $name if not exists $id_name_map{$id};
			}
		}
	}
}

# dump to console
while(my($name,$id) = each %id_name_map){
	print join(",",($name,$id))."\n";
}
