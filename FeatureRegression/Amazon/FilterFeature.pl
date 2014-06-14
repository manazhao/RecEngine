#!/usr/bin/perl
# extract all features given an item
#
use strict;
use warnings;
use Getopt::Long;
use Data::Dumper;

my $keep_pattern_list;
my $dict_file;

GetOptions("pat=s" => \$keep_pattern_list, "dict=s" => \$dict_file) or die $!;

($keep_pattern_list) or die "regex list for features to remain musth be specified: $!";
($dict_file  && -f $dict_file) or die "feature dictionary file must be provided and exist: $!";

my @pattern_list = map {quotemeta $_} split /\s+/, $keep_pattern_list;



# which features will be kept
my %feat_map = ();
open DICT_FILE, "<", $dict_file or die "failed to open - $dict_file: $!";
while(<DICT_FILE>){
	chomp;
	my($name,$id) = split /\,/;
	my @match = grep { $name =~ /$_/g} @pattern_list;
	$feat_map{$id} =  scalar @match > 0 ? 1 : 0;
}


while(<STDIN>){
	chomp;
	my ($entity_id, @features) = split /\,/;
	@features = grep {$feat_map{ (split /\:/, $_)[0] }} @features;
	print join(",",($entity_id,@features))."\n";
}
