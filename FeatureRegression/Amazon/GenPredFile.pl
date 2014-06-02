#!/usr/bin/perl
use File::Basename;
use Getopt::Long;
use Data::Dumper;
use JSON qw(decode_json);
use File::Basename;
# generate user feature given the provided json file

# feature dictionalry file
my $dict_file;
my $user_json_file;
my $item_feature_file;
# user json file
my $rating_file;
# index user's feature and output to the provided file
my $result_folder;


GetOptions("dict=s" => \$dict_file, "user=s" => \$user_json_file, "item=s" => \$item_feature_file,
 "result=s" => \$result_folder) or die "incorrect command line arguments";

$dict_file or die "dictionary file must be provided";
$user_json_file or die "user json file must be provided";
$item_feature_file or die "item feature file must be provided";
$result_folder or die "result folder must be provided";

# validate the file paths
-f $dict_file or die $!;
-f $user_json_file or die $!;
-f $item_feature_file or die $!;
-d $result_folder or die $!;

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

my %feature_handler_map = (
    "u" => {
        "gender" => \&default_feature_handler,
        "age" => \&user_age_feature_handler
    }
);

my %user_feature_map = ();
open USER_FEATURE_FILE, "<", $user_json_file or die $!;
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
        # duplicate item detection
        # print $item_id . "\n" if exists $item_feature_map{$item_id};
	$item_feature_map{$item_id} = \@feat_ids;
}
close ITEM_FEATURE_FILE;

#while(my($item_id, $item_feats) = each %item_feature_map){
#    my ($type, $asin) = split /\_/, $item_id;
#    print $asin ."\n";
#}

# now read the input json file and index features for each user
# append to existing dictionary file
open DICT_FILE , ">>", $dict_file or die $!;
open USER_FILE, "<" , $user_json_file or die $!;

while(<USER_FILE>){
    chomp;
    my $tmp_json = decode_json($_);
    my $user_id = "u_" . $tmp_json->{"id"};
    # generate user features
    my %user_feat_map = ();
    while(my ($key, $value) = each %$tmp_json){
        if(exists $feature_handler_map{"u"}->{$key}){
            my ($feat_names, $feat_vals) = $feature_handler_map{"u"}->{$key}->("u",$key,$value);
            @user_feat_map{@$feat_names} = @$feat_vals;
        }
    }
    my @feat_names = keys %user_feat_map;
    my @feat_values = @user_feat_map{@feat_names};
    my @feat_ids;
    foreach my $tmp_feat (@feat_names){
        push @feat_ids, $feature_idx_map{$tmp_feat} if exists $feature_idx_map{$tmp_feat};
    }

    # generate interaction features for all items
    my $pred_file = $result_folder . "/" . "$user_id.pred.libsvm";
    open PRED_FILE , ">", $pred_file or die $!;
    while(my($item_id, $item_feats) = each %item_feature_map){
        my @int_feats = ();
        foreach my $i (0 .. $#feat_ids){
            my $user_feat_id = $feat_ids[$i];
            my $user_feat_val = $feat_values[$i];
            foreach my $item_feat (@$item_feats){
                my ($item_feat_id,$item_feat_val) = split /\:/,$item_feat;
                my $int_feat = join("|", ($user_feat_id, $item_feat_id));
                push @int_feats, join(":",($feature_idx_map{$int_feat},$user_feat_val * $item_feat_val)) if exists $feature_idx_map{$int_feat};
            }
        }
        # merge user feats, item feats and interactive features
        my @all_feats = ((map {join(":",($feat_ids[$_],$feat_values[$_]))} (1..$#feat_ids)), @$item_feats, @int_feats);
	my @all_feats_sort = sort { my ($id1,$id2) = ((split /\:/, $a)[0],(split /\:/,$b)[0]); $id1 <=> $id2} @all_feats;
        print PRED_FILE join(" ", (1, @all_feats_sort)) . "\n";
    }
    close PRED_FILE;
}


close USER_FILE;



sub default_feature_handler{
    my($type, $feature, $value) = @_;
# remove , from the value
    $value =~ s/\,//g;
    return ([join("_", ($type,$feature,$value))],[1]);
}

sub user_age_feature_handler{
    my($type, $feature, $value) = @_; 
    my $age = int $value;
    $age = int ($age / 5); 
    return ([join("_", ($type, $feature, $age))],[1]);
}

