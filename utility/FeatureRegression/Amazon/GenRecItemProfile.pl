#!/usr/bin/perl
#
use strict;
use warnings;
use File::Basename;
use Getopt::Long;

my $item_feat_file;
my $feat_dict_file;
my $rec_file;
my $result_file;

GetOptions("item-feat=s" => \$item_feat_file, "dict=s" => \$feat_dict_file, "rec=s" => \$rec_file, "result=s" => \$result_file) or die $!;

($item_feat_file && -f $item_feat_file ) or die $!;
($feat_dict_file && -f $feat_dict_file ) or die $1;
($rec_file && -f $rec_file) or die $!;
($result_file && -d dirname($result_file)) or die $!;

my %feature_map = ();

open DICT_FILE, "<", $feat_dict_file or die $!;
while(<DICT_FILE>){
	chomp;
	my ($name,$idx) = split /\,/;
	$feature_map{$idx} = $name;
}
close DICT_FILE;

my %item_feat_map = ();
open ITEM_FEAT_FILE, "<", $item_feat_file or die $!;

while(<ITEM_FEAT_FILE>){
	chomp;
	my ($item_id, @item_feats) = split /\,/;
	my ($type,$item_id1) = split /\_/, $item_id;
	$item_feat_map{$item_id1} = \@item_feats;
}
close ITEM_FEAT_FILE;

open REC_FILE, "<", $rec_file or die $!;

my %rec_feat_map = ();
while(<REC_FILE>){
	chomp;
	my ($asin, $score) = split /\,/;
	my $item_feats = $item_feat_map{$asin};
	foreach my $tmp_feat (@$item_feats){
		$rec_feat_map{$feature_map{$tmp_feat}}++;
	}
}
close REC_FILE;

# output the result
my @sort_feats = sort {$rec_feat_map{$b} <=> $rec_feat_map{$a}} keys %rec_feat_map;
open RESULT_FILE, ">", $result_file or die $!;
foreach (@sort_feats){
	print RESULT_FILE join(",", ($_,$rec_feat_map{$_})) . "\n";
}
close RESULT_FILE;


