#!/usr/bin/perl
#
use warnings;
use strict;
use Getopt::Long;

my $input_file;
my $output_file;

GetOptions("input=s" => \$input_file, "output=s" => \$output_file) or die $!;

$input_file or die $!;
$output_file or die $!;

open INPUT_FILE,"<", $input_file or die $!;
open OUTPUT_FILE, ">", $output_file or die $!;

while(<INPUT_FILE>){
	chomp;
	my ($userId,$item_id,$label, @feats) = split /\,/;
	# feats is array of sparse features which is in the format featId:value
	# sort feats by featId in ascend order
	my @feats_sort = sort { my ($id1,$id2) = ((split /\:/, $a)[0],(split /\:/,$b)[0]); $id1 <=> $id2} @feats;
	print OUTPUT_FILE join(" ", ($label, @feats_sort)) . "\n";
}
close OUTPUT_FILE;
close INPUT_FILE;

