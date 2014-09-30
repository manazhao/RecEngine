#!/usr/bin/perl
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;

my $dict_file;
my $feat_coef_file;
my $result_file;

GetOptions("dict=s" => \$dict_file, "coef=s" => \$feat_coef_file, "result=s" => \$result_file) or die $!;
-f $dict_file and -f $feat_coef_file and -d dirname($result_file) or die $!;

open DICT_FILE, "<" ,$dict_file or die $!;
open COEF_FILE, "<" ,$feat_coef_file or die $!;
open RESULT_FILE, ">" , $result_file or die $!;

my %feat_map = ();

while(<DICT_FILE>){
	chomp;
	my($name,$id) = split /\,/;
	$feat_map{$id} = $name;
}
close DICT_FILE;

while(<COEF_FILE>){
	chomp;
	my($id,$weight) = split /\,/;
	$id = int ($id);
	$weight = sprintf("%.4f", $weight);
	print RESULT_FILE join(",",($feat_map{$id},$weight)) . "\n";
}

close COEF_FILE;
close RESULT_FILE;
