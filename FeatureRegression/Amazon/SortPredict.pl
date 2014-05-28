#!/usr/bin/perl
#
use strict;
use warnings;
use Getopt::Long;
use File::Basename;

my $pred_result_file;
my $item_list_file;
my $sort_result_file;

GetOptions("pred=s" => \$pred_result_file, "item=s" => \$item_list_file, "result=s" => \$sort_result_file) or die $!;

-f $pred_result_file or die $!;
-f $item_list_file or die $!;
-d dirname($sort_result_file) or die $!;

# read the result
open ITEM_FILE, "<" , $item_list_file or die $!;
my @items = ();
while(<ITEM_FILE>){
    chomp;
    push @items, $_;
}
close ITEM_FILE;

# read the prediction result
open PRED_FILE , "<", $pred_result_file or die $!;

my $dummy_line = <PRED_FILE>;
my %item_score = ();

my $item_idx = 0;
while(<PRED_FILE>){
    chomp;
    my ($label, $pos_prob, $neg_prob) = split /\s+/;
    # get the item id
    my $item_id = $items[$item_idx];
    $item_score{$item_id} = $pos_prob;
    $item_idx ++;
}
close PRED_FILE;
# sort by score
#
my @sort_items = sort {$item_score{$b} <=> $item_score{$a}} keys %item_score;

open RESULT_FILE, ">", $sort_result_file or die $!;
foreach my $item_id (@sort_items){
    print RESULT_FILE join(",", ($item_id,$item_score{$item_id})) . "\n";
}

close RESULT_FILE;

