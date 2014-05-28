#!/usr/bin/perl
# merge two feature files _feat.csv
#
use strict; 
use warnings;

my %entity_feat_map = ();

scalar @ARGV >= 2 or die "at least two feature files provided through command line:$!";

# verify the paths...
foreach my $file(@ARGV){
	-f $file or die "file - $file does not exist: $!";
	open TMP_FILE, "<" , $file or die "failed to open file - $file: $!";
	close TMP_FILE;
}

# read each file
foreach my $file(@ARGV){
	open TMP_FILE, "<" , $file or die "failed to open file - $file: $!";
	while(<TMP_FILE>){
		chomp;
		my ($entity_id, @feats) = split /\,/;
		push @{$entity_feat_map{$entity_id}}, @feats;
	}
	close TMP_FILE;
}

# dump the result to console
while(my($entity_id,$feats) = each %entity_feat_map){
	print join(",",($entity_id,@$feats)) . "\n";
}

