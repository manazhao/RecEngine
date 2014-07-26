#!/usr/bin/perl 
# merge entity attributes stored in separte json files
use strict;
use warnings;
use Data::Dumper;
use JSON;

scalar @ARGV > 1 or die "please provide at least two json files: $!";

my %entity_attr_map = ();

foreach my $file(@ARGV){
	-f $file or die "file - $file does not exist: $!";
}

foreach my $file(@ARGV){
	open TMP_FILE, "<", $file or die "failed to open - $file: $!";
	while(<TMP_FILE>){
		chomp;
		my $json_obj = decode_json($_);
		my $id = $json_obj->{"id"};
		@{$entity_attr_map{$id}}{keys %$json_obj} = values %$json_obj;
	}
	close TMP_FILE;
}

while(my($id,$attr_map) = each %entity_attr_map){
	print encode_json($attr_map) . "\n";
}
