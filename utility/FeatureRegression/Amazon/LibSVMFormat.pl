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
	if($label eq "1"){
		$label = "+1";
	}
	# sort feats in ascending order
	@feats = sort {$a <=> $b} @feats;
	my @feat_strs = map {join ":", ($_ + 1,1)} @feats;
	print OUTPUT_FILE join(" ", ($label, @feat_strs)) . "\n";
}
close OUTPUT_FILE;
close INPUT_FILE;

