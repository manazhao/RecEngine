#!/usr/bin/perl
#
# generate description given a feature
#
#
use strict;
use warnings;
use Data::Dumper;
use Getopt::Long;

my $dict_file;
# category name file
my $cat_path_file;

GetOptions("dict=s" => \$dict_file, "cat=s" => \$cat_path_file) or die $!;

$dict_file and $cat_path_file or die "--dict=<feature dictionary file> --cat=<category path file>: $!";

-f $dict_file and $cat_path_file or die $!;

my %feat_map = ();
my %cat_map = ();

open DICT_FILE, "<", $dict_file or die $!;
open CAT_FILE, "<", $cat_path_file or die $!;

while(<DICT_FILE>){
	chomp;
	my ($name,$id) = split /\,/;
	$feat_map{$id} = $name;
}
close DICT_FILE;

while(<CAT_FILE>){
	chomp;
	my($name,$path) = split /\,/;
	# $name is formed by joining category id and name by hypen
	my($id,$c_name) = split /\-/, $name;
	$cat_map{$id} = $c_name;
}

close CAT_FILE;

while(<>){
	chomp;
	my $fid = $_;
	# get feature name
	if(exists $feat_map{$fid}){
		my $fname = $feat_map{$fid};
		# interaction feature
		my @feats = $fname =~ m/(\d+)\|(\d+)/g;
		my $fdesc;
		if(scalar @feats > 0){
			# print Dumper(@feats);
			my @feat_desc = ();
			foreach(@feats){
				my $fdesc = get_description($feat_map{$_});
				push @feat_desc, $fdesc;	
			}
			$fdesc = join("|",@feat_desc);
		}
		else{
			$fdesc = get_description($fname);
		}
		print $fdesc . "\n"; 
	}else{
		print $fid . ": does not exist\n";
	}

}


sub get_description{
	my($fname) = @_;
	if($fname =~ m/i_c_/){
		return get_cat_name($fname);
	}
	return $fname;
}

sub get_cat_name{
	my($fname) = @_;
	my($type,$name,$value) = split /\_/,$fname;
	return "i_c_$cat_map{$value}";
}
