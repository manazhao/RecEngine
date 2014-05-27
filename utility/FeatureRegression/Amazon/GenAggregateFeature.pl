#!/usr/bin/perl
# generate features from aggregated data
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;
use POSIX;

my $item_profile_file;
my $rating_file;
# store the popularity information
my $result_file;
# store category tree
my $cat_tree_file;

GetOptions("item=s" => \$item_profile_file,
 "rating=s" => \$rating_file,
"result=s" => \$result_file,
"tree=s" => \$cat_tree_file) or die $!;

($item_profile_file and -f $item_profile_file) or die "item feature file - $item_profile_file does not exist : $!";
($rating_file and -f $rating_file) or die "rating file - $rating_file does not exist : $!";
($result_file and -d dirname($result_file)) or die "result file - $result_file does not exist : $!";
($cat_tree_file and -d dirname($cat_tree_file)) or die "result file - $cat_tree_file does not exist : $!";

# read in the item features

my %item_feature_map = ();
my %cat_parent_map = ();

open ITEM_FEAT_FILE, "<" , $item_profile_file or die "failed to open - $item_profile_file: $!";
while(<ITEM_FEAT_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $item_id = $json_obj->{"id"};
	delete $json_obj->{"id"};
	$item_feature_map{$item_id} = $json_obj;
	$json_obj->{"c"} or next;
	my @cat_strs = split /\|/, $json_obj->{"c"};
	my %result_cats = ();
	foreach my $cat_str(@cat_strs){
		# further split by /
		my @sub_cats = split /\//, $cat_str;
		my @cat_ids = ();
		foreach my $cat (@sub_cats){
			my ($cat_id, $cat_name) = split /\-/ , $cat;
			push @cat_ids, $cat_id;
			$result_cats{$cat_id} = 1;
		}
		for(my $i = 0; $i < $#cat_ids; $i++){
			my $cur_cat = $cat_ids[$i];
			my $p_cat = $cat_ids[$i+1];
			$cat_parent_map{$cur_cat}->{$p_cat} = 1;
		}
	}
}
close ITEM_FEAT_FILE;

open TREE_FILE, ">", $cat_tree_file or die $!;
while(my($cat_id,$p_cats) = each %cat_parent_map){
	print TREE_FILE join(",", ($cat_id, join("|", keys %{$p_cats}))) . "\n";
}

close TREE_FILE;
# generate aggregated feature for each item which includes
# ip: item_popularity
# ipy: item_year_popularity
# cpy: category_year
# cp: category

# feature frequence map
my %feat_freq_map = ();

open RATING_FILE, "<", $rating_file or die "failed to open rating file - $rating_file: $!";


while(<RATING_FILE>){
	chomp;
	my($user_id,$item_id,$rating, $ts) = split /\s+/;
	# get the item features
	my $item_feats = $item_feature_map{$item_id};
	# item popularity
	my $feat_key = "ip_" . $item_id;
	$feat_freq_map{$feat_key}++;
	# item year popularity
	$ts or next;
	my($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = localtime($ts);
	# keep year and month
	$feat_key = join("_", ("ipy",$item_id,$year));
	$feat_freq_map{$feat_key}++;
	# keep year and month
	$feat_key = join("_", ("ipym",$item_id, $year.$month));
	$feat_freq_map{$feat_key}++;
	
	# get category
	$item_feats->{"c"} or next;
	my @cat_strs = split /\|/, $item_feats->{"c"};
	my %result_cats = ();
	foreach my $cat_str(@cat_strs){
		# further split by /
		my @sub_cats = split /\//, $cat_str;
		my @cat_ids = ();
		foreach my $cat (@sub_cats){
			my ($cat_id, $cat_name) = split /\-/ , $cat;
			push @cat_ids, $cat_id;
			$result_cats{$cat_id} = 1;
		}
	}
	# now generate the category frequency feature
	foreach my $cat(keys %result_cats){
		$feat_key = join("_", ("cp", $cat));
		$feat_freq_map{$feat_key}++;
		$feat_key = join("_", ("cpy",$cat,$year));
		$feat_freq_map{$feat_key}++;
		$feat_key = join("_", ("cpym",$cat,$year.$month));
		$feat_freq_map{$feat_key}++;
	}
}

close RATING_FILE;
# dump the frequency to file
open RESULT_FILE, ">", $result_file or die "failed to write to result file - $result_file:$!";
while(my ($key,$freq) = each %feat_freq_map){
	print RESULT_FILE join(",",($key,$freq)) . "\n";
} 

close RESULT_FILE;









