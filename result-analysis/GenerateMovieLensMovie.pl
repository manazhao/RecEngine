#!/usr/bin/perl
#
# generate movie names based on item id

use strict;
use warnings;
use JSON;

my $home_dir = $ENV{"HOME"};
my $item_file = "$home_dir/data/ml-1m/movies.dat";

open ITEM_FILE , "<", $item_file or die $!;

my %item_id_name_map = ();

while(<ITEM_FILE>){
	chomp;
	my($id,$name,$genre_str) = split '::';
	# get the year of the movie
	my $year = "";
	if($name =~ m/\((\d+)\)/g){
		$year = $1;	
	}
	my @genres = split '\|', $genre_str;
	$item_id_name_map{$id} = {"name" => $name, "year" => $year, "genre" => \@genres};
}

close ITEM_FILE;

while(<STDIN>){
	chomp;
	my $json_obj = decode_json($_);
	my $user_id = $json_obj->{"userId"};
	my $feature_names = join '|', @{$json_obj->{"featureNames"}};
	my $rec_item_ids = $json_obj->{"recNameList"};
	my @rec_item_names = ();
	my %item_year_map = ();
	my %item_genre_map = ();
	foreach my $rec_item (@$rec_item_ids){
		my ($ent_type, $item_id) = split "_", $rec_item;
		my $item_name = $item_id_name_map{$item_id}->{"name"};
		push @rec_item_names, $item_name;	
		$item_year_map{$item_id_name_map{$item_id}->{"year"}}++;
		# get the genre stats
		my @item_genres = @{$item_id_name_map{$item_id}->{"genre"}};
		map {$item_genre_map{$_}++} @item_genres;
		# add to json
	}
	# dump to command line
	# join the year and genres
	$json_obj->{"year"} = \%item_year_map;
	$json_obj->{"genre"} = \%item_genre_map;
	my $item_year_str = join '|', map {join "_", ($_, $item_year_map{$_})} sort keys %item_year_map;
	my $item_genre_str = join '|', map {join "_", ($_, $item_genre_map{$_})} sort {$item_genre_map{$b} <=> $item_genre_map{$a}} keys %item_genre_map;
	print join("\t", ($user_id, $feature_names,$item_year_str, $item_genre_str, join("|",@rec_item_names)))."\n";
}
