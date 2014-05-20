#!/usr/bin/perl
#
use warnings;
use strict;
use Getopt::Long;
use JSON;
use File::Basename;

my $rating_file;
my $sample_ratio = 1;
my $output_file;
GetOptions("rating=s" => \$rating_file, "ratio=f" => \$sample_ratio, "output=s" => \$output_file) or die $!;

$rating_file or die $!;
$output_file or die $1;

-f $rating_file or die $!;
-d dirname($output_file) or die $!;

my %item_map = ();
my %user_rating_map = ();

open RATING_FILE, "<", $rating_file or die $!;


print ">>> read ratings\n";
while(<RATING_FILE>){
	chomp;
	my $json = decode_json($_);
	my $user_id = $json->{"u"};
	my $item_id = $json->{"i"};
 	push @{$user_rating_map{$user_id}}, $item_id;
	$item_map{$item_id} = 1;
}

close RATING_FILE;

my @item_ids = keys %item_map;
my $total_num_items = scalar @item_ids;

print ">> generate purchase data, be patient\n";

open OUTPUT_FILE, ">", $output_file or die $!;
while(my ($user_id, $items) = each %user_rating_map){
	my $num_items = scalar @$items;
	my %tmp_item_map = ();
	@tmp_item_map{@$items} = (1) x $num_items;
	my $num_neg = $sample_ratio * $num_items;
	# randomly select from the unseen items
	my @neg_items = ();
	while(scalar @neg_items < $num_neg){
		my $tmp_idx = rand($total_num_items);
		my $tmp_item = $item_ids[$tmp_idx];
		exists $tmp_item_map{$tmp_item} and next;
		push @neg_items, $tmp_item;
	}
	# now generate the purchase response
	foreach my $tmp_item (@$items){
		print OUTPUT_FILE encode_json({"u" => $user_id, "i" => $tmp_item, "r" => 1}) . "\n";
		#print encode_json({"u" => $user_id, "i" => $tmp_item, "r" => 1}) . "\n";
	}	
	foreach my $tmp_item (@neg_items){
		print OUTPUT_FILE encode_json({"u" => $user_id, "i" => $tmp_item, "r" => -1}) . "\n";
		#print encode_json({"u" => $user_id, "i" => $tmp_item, "r" => -1}) . "\n";
	}	
}

close OUTPUT_FILE;
