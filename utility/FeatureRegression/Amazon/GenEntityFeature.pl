#!/usr/bin/perl
use File::Basename;
use Getopt::Long;
use JSON qw(decode_json);
# generate user feature given the provided json file

# feature dictionalry file
my $dict_file;
# user json file
my $input_json_file;
# index user's feature and output to the provided file
my $output_feature_file;

my $type;

GetOptions("dict=s" => \$dict_file, "input=s" => \$input_json_file, "output=s" => \$output_feature_file, "type=s" => \$type) or die "incorrect command line arguments";

$dict_file or die "dictionary file must be provided";
$input_json_file or die "input json file must be provided";
$output_feature_file or die "ouput feature file must be provided";
$type or die "the type of the entity must be specified [user,item]";

# validate the file paths
-d dirname($dict_file) or die "dictionary file must be a valid path";
-f $input_json_file or die "input json file does not exist";
-d dirname($output_feature_file) or die "output feature file must be a valid path";

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

# now read the input json file and index features for each user
# append to existing dictionary file
open DICT_FILE , ">>", $dict_file;
open INPUT_FILE, "<" , $input_json_file or die "failed to open the input json feature file";
open OUTPUT_FILE, ">", $output_feature_file or die "failed to open the output feature file";


my %feature_handler_map = (
    "u" => {
        "gender" => \&default_feature_handler,
        "age" => \&user_age_feature_handler
    },
    "i" => {
        "m" => \&default_feature_handler,
        "c" => \&item_category_feature_handler
    
    }
);

while(<INPUT_FILE>){
    chomp;
    my $tmp_json = decode_json($_);
    # generate features
    my $entity_id = join("_",($type , $tmp_json->{"id"}));
    my %features = ();
    while(my ($key, $value) = each %{$tmp_json}) {
        if(exists $feature_handler_map{$type}->{$key}){
            my $handler_ref = $feature_handler_map{$type}->{$key};
            my @tmp_features = $handler_ref->($type, $key, $value);
            @features{@tmp_features} = (1) x @tmp_features;
        }
    }

    # convert to integers
    my @feature_ids;
    foreach my $feature(keys %features){
        if(exists $feature_idx_map{$feature}){
            push @feature_ids, $feature_idx_map{$feature};
        }else{
            push @feature_ids, $max_feature_idx;
            $feature_idx_map{$feature} = $max_feature_idx;
            print DICT_FILE join(",", ($feature,$max_feature_idx)) . "\n";
            $max_feature_idx++;
        }
    }
    # generate the user feature ids
    if(scalar @feature_ids > 0){
        my $feature_str = join "," , @feature_ids;
        print OUTPUT_FILE join(",", ($entity_id, $feature_str)) . "\n";
    }
}

close DICT_FILE;
close INPUT_FILE;
close OUTPUT_FILE;



sub default_feature_handler{
    my($type, $feature, $value) = @_;
    return (join("_", ($type,$feature,$value)));
}

sub user_age_feature_handler{
    my($type, $feature, $value) = @_;
    my $age = int $value;
    $age = int ($age / 5);
    return (join("_", ($type, $feature, $age)));
}

sub item_merchant_feature_handler{
    my($type, $feature, $value) = @_;
    my($name,$id) = split /\,/, $value;
    return (join("_", ($type,$feature,$name)));
}

sub item_category_feature_handler{
    my($type, $feature, $value) = @_;
    # split the category strigns
    my @cat_strs = split /\|/, $value;
    my %result_cats = ();
    foreach my $cat_str(@cat_strs){
        # further split by /
        my @sub_cats = split /\//, $cat_str;
        my $i = 0;
        foreach my $cat (@sub_cats){
            my ($cat_id, $cat_name) = split /\-/ , $cat;
            $result_cats{join("_",($type,$feature,$cat_id))} = 1;
            $i ++;
            last if $i == 2;
        }
    }
    return keys %result_cats;
}
