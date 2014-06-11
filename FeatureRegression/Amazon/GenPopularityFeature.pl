#!/usr/bin/perl
# generate features from aggregated data
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;
use POSIX;

my $rating_file;
my $feature_file;
# store the popularity information
my $result_file;

GetOptions(
 "rating=s" => \$rating_file,
"dict=s" => \$feature_file,
"result=s" => \$result_file,
) or die $!;

$rating_file and $feature_file and $result_file or die "--rating=<rating file> --dict=<feature dictionary file> --result=<result file> is required:$!";
-f $rating_file and -f $feature_file and -d dirname($result_file) or die "rating file, feature file and result file must exist: $!";

my %feat_map = ();

open FEAT_FILE, "<" ,$feature_file or die $!;
my $max_feat_id = 0;
while(<FEAT_FILE>){
	chomp;
	my($fname,$fval) = split /\,/;
	if($fval > $max_feat_id){
		$max_feat_id = $fval;
	}
	$feat_map{$fname} = $fval;
}
close FEAT_FILE;

my $pop_feat_name = "i_pop";
my $pop_feat_id = $feat_map{$pop_feat_name};
if(not exists $feat_map{$pop_feat_name}){
	$max_feat_id++;
	$feat_map{$pop_feat_name} = $max_feat_id;
	$pop_feat_id = $max_feat_id;
	open FEAT_FILE, ">>", $feature_file or die $!;
	print FEAT_FILE join(",",($pop_feat_name, $pop_feat_id)) . "\n";
	close FEAT_FILE;
}

my %item_cnt_map = ();
open RATING_FILE, "<", $rating_file or die $1;


while(<RATING_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $item_id = $json_obj->{"i"};
	$item_cnt_map{$item_id}++;
}

close RATING_FILE;

# generate the feature
open RESULT_FILE, ">", $result_file or die $1;

while(my($item_id, $cnt) = each %item_cnt_map){
	my $cnt_fmt = sprintf("%.4f", $cnt/6000);
	$cnt_fmt > 0 && print RESULT_FILE join(",", ("i_" . $item_id, join(":",($pop_feat_id,$cnt_fmt)))) . "\n";
}
close RESULT_FILE;

