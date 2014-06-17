#!/usr/bin/perl
#
# make recommendation by counting given gender and age
#
use strict;
use warnings;
use Getopt::Long;
use JSON;

use Data::Dumper;
use DBI;

# connect to database


my $user_feat_file;
my $rating_file;
my $dict_file;
my $result_folder;

GetOptions("dict=s" => \$dict_file,"user=s" => \$user_feat_file, "rating=s" => \$rating_file, "result=s" => \$result_folder) or die $!;

($dict_file && $user_feat_file && $rating_file && $result_folder) or die "feature dictionary file, user feature file, rating file and result folder must be provided: $!";

my %feat_map = ();
my %user_feat_map = ();

-f $dict_file and -f $user_feat_file and -f $rating_file and -d $result_folder or die $!;
open DICT_FILE, "<", $dict_file or die $!;
open USER_FILE, "<", $user_feat_file or die $!;
open RATING_FILE, "<", $rating_file or die $!;

while(<DICT_FILE>){
	chomp;
	my($feat_name, $feat_id) = split /\,/;
	$feat_map{$feat_id} = $feat_name;
}
close DICT_FILE;

while(<USER_FILE>){
	chomp;
	my($user_id, @feats) = split /\,/;
	$user_id = (split /_/, $user_id)[1];
	my @feat_ids = map { (split /\:/, $_)[0] } @feats;
	foreach my $feat(@feat_ids){
		if(exists $feat_map{$feat}){
			my $feat_name = $feat_map{$feat};
			if($feat_name =~ /age/){
				# $user_feat_map{$user_id}->{"age"} = $feat_name;
			}
			if($feat_name =~ /gender/){
				$user_feat_map{$user_id}->{"gender"} = $feat_name;
			}
		}
	}
}
close USER_FILE;


my %filter_criteria_map = ();

while(<RATING_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $uid = $json_obj->{"u"};
	my $iid = $json_obj->{"i"};
	my $rating = $json_obj->{"r"};
	my $age = $user_feat_map{$uid}->{"age"};
	my $gender = $user_feat_map{$uid}->{"gender"};
	# $age = "" unless $age;
	$gender = "" unless $gender;
	my $criteria = $gender;
	next if $criteria eq "" or $criteria =~ /miss/;
	$filter_criteria_map{$criteria}->{$iid} ++;
}

while(my($criteria,$item_cnt_map) = each %filter_criteria_map){
	# sort by cnt
	my @sort_items = sort {$item_cnt_map->{$b} <=> $item_cnt_map->{$a} } keys %$item_cnt_map;
	my $rec_file = $result_folder . "/$criteria.csv";
	print $rec_file . "\n";
	open REC_FILE, ">", $rec_file or die $!;
	my $lines = join("\n", (map {join(",",($_,$item_cnt_map->{$_}))} @sort_items));
	print REC_FILE $lines;
	close REC_FILE;
}

close RATING_FILE;


