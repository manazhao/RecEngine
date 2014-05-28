#!/usr/bin/perl
use File::Basename;
use Getopt::Long;
use JSON qw(decode_json);
use Data::Dumper;
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
        "age" => \&user_age_feature_handler,
    },
    "i" => {
        "m" => \&default_feature_handler,
        "c" => \&item_category_feature_handler,
	"au" => \&default_feature_handler,
	"p_date" => \&production_date_feature_handler,
    }
);

my %required_features = (
	"u" => { "gender" => 1, "age" => 1},
	"i" => { "m" => 1, "c" => 1, "au" => 1, "p_date" => 1}
);

print ">>> read input feature file in json format\n";

my $type_required_features = $required_features{$type};

while(<INPUT_FILE>){
    chomp;
    my $tmp_json = decode_json($_);
    # generate features
    my $entity_id = join("_",($type , $tmp_json->{"id"}));
    my %features = ();
    my %missing_features = %$type_required_features;
    while(my ($key, $value) = each %{$tmp_json}) {
        if(exists $feature_handler_map{$type}->{$key}){
	    # remove from the missing feature list
	    delete $missing_features{$key};
            my $handler_ref = $feature_handler_map{$type}->{$key};
	    # the return value of feature handler is array which contains reference to key array and value array
	    # ($key_arr_ref, $val_arr_ref)
            my ($feat_names,$feat_vals) = $handler_ref->($type, $key, $value);
	    @features{@$feat_names} = @$feat_vals;
        }
    }
    # generate missing feature if there is any
    foreach my $mis_feature( keys %missing_features){
	    my $feat_name = join("_", ($type,$mis_feature,"#miss"));
	    $features{$feat_name} = 1;
    }

    # convert to integers
    my @feat_names = keys %features;
    my @feat_ids;
    my @feat_values = @features{@feat_names};

    foreach my $feature(@feat_names){
        if(exists $feature_idx_map{$feature}){
            push @feat_ids, $feature_idx_map{$feature};
        }else{
            push @feat_ids, $max_feature_idx;
            $feature_idx_map{$feature} = $max_feature_idx;
            print DICT_FILE join(",", ($feature,$max_feature_idx)) . "\n";
            $max_feature_idx++;
        }
    }
    # generate the user feature ids
    if(scalar @feat_ids > 0){
        my $feature_str = join "," , map {$feat_ids[$_] . ":" . $feat_values[$_]} (0 .. $#feat_values);
        print OUTPUT_FILE join(",", ($entity_id, $feature_str)) . "\n";
    }
}

close DICT_FILE;
close INPUT_FILE;
close OUTPUT_FILE;


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

sub item_merchant_feature_handler{
    my($type, $feature, $value) = @_;
    my($name,$id) = split /\,/, $value;
    return ([join("_", ($type,$feature,$name))],[1]);
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
            # only use the leaf category
#            $i ++;
#            last if $i == 2;
        }
    }
    my @feat_names = keys %result_cats;
    my @feat_values = @result_cats{@feat_names};
    return (\@feat_names, \@feat_values);
}

sub production_date_feature_handler{
    my($type, $feature, $value) = @_;
    # split by - and subtract 1900 from the year
    my($year,$month,$mday) = split /\-/, $value;
    $year -= 1900;
    return ([join("_", ($type,$feature,$year))],[1]);
}

