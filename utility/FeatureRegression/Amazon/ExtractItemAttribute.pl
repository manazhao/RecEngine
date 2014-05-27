#!/usr/bin/perl
# extract product attributes from Amazon api response
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON ;

my $inputFile;
my $outputFile;
my %attributes = (
	"p_date" => "PublicationDate",
	"au" => "Author",
);

GetOptions( "input=s" => \$inputFile, "output=s" => \$outputFile) or die $!;

($inputFile && -f $inputFile) || die $!;
($outputFile && -d dirname($outputFile) ) or die $!;

open IN_FILE, "<", $inputFile or die $!;
open OUT_FILE, ">", $outputFile, or die $!;

while(<IN_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	# get item attributes
	my $item_attributes = $json_obj->{"ItemAttributes"};
	my $id = $json_obj->{"ASIN"};
	my %result_json = ("id" => $id);
	while(my($sn, $attr_name) = each %attributes){
		if(exists $item_attributes->{$attr_name}){
		$result_json{$sn} = $item_attributes->{$attr_name};	
		}
	}
	print OUT_FILE encode_json(\%result_json) . "\n";
}
close IN_FILE;
close OUT_FILE;
