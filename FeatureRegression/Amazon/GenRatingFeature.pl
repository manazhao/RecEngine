#!/usr/bin/perl
use File::Basename;
use Getopt::Long;
use JSON qw(decode_json);
use File::Basename;
# generate user feature given the provided json file

# feature dictionalry file
my $dict_file;
my $user_feature_file;
my $item_feature_file;
# user json file
my $rating_file;
# index user's feature and output to the provided file
my $output_feature_file;


GetOptions("dict=s" => \$dict_file, "user=s" => \$user_feature_file, "item=s" => \$item_feature_file, "rating=s" => \$rating_file,
 "output=s" => \$output_feature_file) or die "incorrect command line arguments";

$dict_file or die "dictionary file must be provided";
$user_feature_file or die "user feature file must be provided";
$item_feature_file or die "item feature file must be provided";
$rating_file or die "rating file must be provided";
$output_feature_file or die "ouput feature file must be provided";

# validate the file paths
-f $dict_file or die $!;
-f $user_feature_file or die $!;
-f $item_feature_file or die $!;
-f $rating_file or die $!;
-d dirname($output_feature_file) or die $!;

my %feature_idx_map = ();
# read in existing features
my $max_feature_idx = 0;
if(-f $dict_file){
	open DICT_FILE, "<", $dict_file or die "failed to open dictionary file";
	while(<DICT_FILE>){
		chomp;
		my ($name, $idx) = split ",";
		$feature_idx_map{$name} = $idx;
		if($idx > $max_feature_idx){
			$max_feature_idx = $idx;
		}	
	}
	close DICT_FILE;
}

my %user_feature_map = ();
open USER_FEATURE_FILE, "<", $user_feature_file or die $!;
while(<USER_FEATURE_FILE>){
	chomp;
	my ($user_id, @feat_ids) = split /\,/;
	$user_feature_map{$user_id} = \@feat_ids;
}
close USER_FEATURE_FILE;

my %item_feature_map = ();
open ITEM_FEATURE_FILE, "<", $item_feature_file or die $!;
while(<ITEM_FEATURE_FILE>){
	chomp;
	my ($item_id, @feat_ids) = split /\,/;
	$item_feature_map{$item_id} = \@feat_ids;
}
close ITEM_FEATURE_FILE;

# now read the input json file and index features for each user
# append to existing dictionary file
open DICT_FILE , ">>", $dict_file or die $!;
open RATING_FILE, "<" , $rating_file or die $!;
open OUTPUT_FILE, ">", $output_feature_file or die $!;

while(<RATING_FILE>){
	chomp;
	my $tmp_json = decode_json($_);
	my $user_id = "u_" . $tmp_json->{"u"};
	my $item_id = "i_" . $tmp_json->{"i"};
	my $rating = $tmp_json->{"r"};
	# generate interaction features
	(exists $user_feature_map{$user_id} and exists $item_feature_map{$item_id}) or next;
	my $user_feats = $user_feature_map{$user_id};
	my $item_feats = $item_feature_map{$item_id};
	# 
	my %int_feat_names = ();
	foreach my $tmp_u_feat (@$user_feats){
		foreach my $tmp_i_feat (@$item_feats){
			my $int_feat = join("|", ($tmp_u_feat, $tmp_i_feat));
			$int_feat_names{$int_feat} = 1;
		}
	}
	my @int_feat_ids = ();
	foreach my $int_feat_name (keys %int_feat_names){
		if(exists $feature_idx_map{$int_feat_name}){
			push @int_feat_ids, $feature_idx_map{$int_feat_name};
		}else{
			push @int_feat_ids, $max_feature_idx;
			$feature_idx_map{$int_feat_name} = $max_feature_idx;
			print DICT_FILE join(",", ($int_feat_name, $max_feature_idx)) . "\n";
			$max_feature_idx ++;
		}
	}
	my $all_feat_str = join(",", (@$user_feats, @item_feats, @int_feat_ids));
	print OUTPUT_FILE join(",", ($user_id, $item_id, $rating, $all_feat_str)) . "\n";
}


close RATING_FILE;
close DICT_FILE;
close OUTPUT_FILE;

