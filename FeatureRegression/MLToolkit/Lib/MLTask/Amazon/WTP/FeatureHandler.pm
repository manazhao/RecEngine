#!/usr/bin/perl 
#
# feature handler for price regression task
#
package MLTask::Amazon::WTP::FeatureHandler;

use strict;
use warnings;
use Exporter;
use FindBin;
use lib "$FindBin::Bin/../../../";
use MLTask::Shared::Utility;
use Data::Dumper;

our $VERSION = 1.0;
our @ISA = qw (Exporter);
# export nothing. avoid clash
our @EXPORT = ();

# feature handlers registration
my %FEATURE_HANDLER_MAP = (
    "i" => {
        "ar" => \&numerical_feature_handler, # average rating
        "br" => \&categorical_feature_handler, # brand
#	"dt" => \&item_production_date_feature_handler, # release date
	"pop" => \&numerical_feature_handler, # item popularity
	"br_pop" => \&numerical_feature_handler, # brand popularity
#	"quantity_tt" => \&numerical_feature_handler, # item quantity
	"sp" => \&numerical_feature_handler, # sales price
#	"pr" => \&numerical_feature_handler, # original price
	"tf_" => \&item_tf_feature_handler, # tf for bow
	#"tfidf_" => \&numerical_feature_handler, # tfidf for bow
	"lc" => \&item_category_feature_handler# leaf category
    },
    "u" => {
	    "age" => \&user_age_feature_handler,
	    "gender" => \&user_gender_feature_handler,
	    "race" => \&user_race_feature_handler
    }
);

my %REQUIRED_FEATURES = (
	"i" => [ "br","pop","br_pop", "ar", "sp"],
	"u" => [ "age","gender","race"]
);

# process feature name by replacing multiple spaces with single space
# to lowercase
sub process_text_value{
	my($fname) = @_;
	$fname =~ s/\s+/ /g;
	return lc $fname;
}

# generic feature handler
# categorical feature by simply joining entitytype_feature_value
sub categorical_feature_handler{
    my($self,$type, $feature, $value) = @_;
    $value = process_text_value($value);
    return ([join("_", ($type,$feature,$value))],[1]);
}

# numerical feature handlers
# the feature name is combination of type and feature name, the value is the numerical value passed in
sub numerical_feature_handler{
    my($self, $type, $feature, $value) = @_;
    return ([join("_",($type,$feature))],[$value]);
}


sub item_tf_feature_handler{
    my($self, $type, $feature, $value) = @_;
    return ([join("_",($type,$feature))],[1]);
}


sub item_category_feature_handler{
	# $type: entity type
	# $feature: feature name, "lc"
	# $value: leaf node id
	my($self, $type,$feature,$value) = @_;
	# category tree map
	my $cat_map = $self->{category_tree};
	my @features = ([],[]);
	if(not exists $cat_map->{$value}){
		print STDERR "unknown category node id: $value\n";
	}else{
		# features to generate
		my @cat_fnames = ();
		my @parent_nodes = @{$cat_map->{$value}->{parent_nodes}};
		push @parent_nodes, $value;
		# generate features for all category nodes
		my $depth = scalar @parent_nodes;
		for(my $i = 1; $i <= $depth; $i++){
			my $tmp_node = $parent_nodes[$i-1];
			my $top_cname = "c" . $i;
			my $bottom_cname = "c-" . ($depth - $i + 1);
			my $feat_name = join("_",($type,$top_cname,$tmp_node));
			push @cat_fnames, $feat_name;
			$feat_name = join("_",($type,$bottom_cname,$tmp_node));
			push @cat_fnames, $feat_name;
		}
		$features[0] = [@cat_fnames];
		$features[1] = [(1) x scalar @cat_fnames];
	}
	return @features;
}

# date is treated as categorical feature
#
sub item_production_date_feature_handler{
	my($self,$type, $feature, $value) = @_;
	# split by - and subtract 1900 from the year
	my($year,$month,$mday) = split /\-/, $value;
	$year -= 1900;
	return ([join("_", ($type,$feature,$year))],[1]);
}


sub user_age_feature_handler{
	my($self,$type, $feature, $value) = @_;
	# get age range
	my @splits = split /\_/, $value;
	my $age = $splits[0];
	$age = int($age/5);
	return ([join("_", ($type,$feature,$age))],[1]);
}

sub user_gender_feature_handler{
	my($self,$type, $feature, $value) = @_;
	# get age range
	my ($gender,$confidence) = split /\_/, $value;
	return ([join("_", ($type,$feature,$gender))],[$confidence/100]);
}

sub user_race_feature_handler{
	my($self,$type, $feature, $value) = @_;
	# get age range
	my ($race,$confidence) = split /\_/, $value;
	return ([join("_", ($type,$feature,$race))],[$confidence/100]);
}


# features of a group, e.g. tfidf features. 
# each word counts as a feature - tons of features are generated
# but all these features use the same feature handler (numerical_feature_handler)
# the feature name prefix (tfidf_) is mapped to the feature handler
sub map_feature_name{
	my ($feature) = @_;	
	$feature =~ m/tf_/ and return "tf_";
	$feature =~ m/tfidf_/ and return "tfidf_";
	return $feature;
}

#### invariant across tasks
sub new {
	my($class, %args) = @_;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub init{
	my($self,%args) = @_;
	my $driver = $args{driver};
	$driver or die "MLTask::Shared::Driver reference missing";
	$self->{driver} = $driver;
	# get feature handler configuration parameters
	my $handler_config = $driver->{config}->{feature_handler_config};
	$self->{category_tree_file} = $driver->get_full_path($handler_config->{category_tree_file});
	$self->_load_category_tree();
	# load item popularity and brand popularity
	$self->_load_item_global_feature();
	$self->_load_brand_global_feature();
	# attach brand popularity to each item
	$self->_set_item_brand_pop();
}

sub _load_item_global_feature{
	my $self = shift;
	my $driver = $self->{driver};
	my $item_attr_map = $driver->get_entity_attribute("item");
	my $item_global_file = $driver->get_full_path($driver->get_task_config()->{feature_handler_config}->{item_global_file});
	open FILE, "<", $item_global_file or die $!;
	print ">>> load item popularity feature\n";
	while(<FILE>){
		chomp;
		my($item_id,$pop,$ar) = split /\t/;
		$item_attr_map->{$item_id}->{pop} = $pop;
		# normalized average rating
		$item_attr_map->{$item_id}->{ar} = $ar/5.0;
	}
	close FILE;
}

sub _load_brand_global_feature{
	my $self = shift;
	my $driver = $self->{driver};
	my $brand_global_file = $driver->get_full_path($driver->get_task_config()->{feature_handler_config}->{brand_global_file});
	open FILE, "<", $brand_global_file or die $!;
	print ">>> load brand popularity feature\n";
	while(<FILE>){
		chomp;
		my($brand,$pop) = split /\t/;
		$self->{br_pop_map}->{$brand} = $pop;
	}
	close FILE;
}

sub _set_item_brand_pop{
	my $self = shift;
	my $driver = $self->{driver};
	my $item_attr_map = $driver->get_entity_attribute("item");
	my $brand_pop_map = $self->{brand_pop_map};
	print ">>> attach brand popularity to item\n";
	while(my($id,$tmp_attr) = each %$item_attr_map){
		my $brand = $tmp_attr->{"br"};
		$brand or next;
		my $brand_pop = $brand_pop_map->{$brand};
		$brand_pop and $tmp_attr->{brand_pop} = $brand_pop;
	}
}

sub _load_category_tree{
	my $self = shift;
	my $tree_file = $self->{'category_tree_file'};
	open TREE_FILE, "<", $tree_file or die $!;
	# category tree map
	my $cat_map = {};
	while(<TREE_FILE>){
		chomp;
		my($cid,$cname,$is_leaf,$path_str) = split /\t/;
		# remove the leading /
		$path_str = substr($path_str,1);
		my @pnodes = split /\//, $path_str;
		$cat_map->{$cid} = { name => $cname, is_leaf => $is_leaf, parent_nodes => \@pnodes};
	}
	$self->{category_tree} = $cat_map;
	close TREE_FILE;
}


sub generate_feature{
	my($self,$type,$attribute_map) = @_;
	my %required_attr_map = ();
	my $entity_required_features = $REQUIRED_FEATURES{$type};
	@required_attr_map{@$entity_required_features} = (0) x scalar @$entity_required_features;
	my %result_feat_map = ();
	while(my($attr_name, $attr_value) = each %$attribute_map){
		# mapped name
		my $mname = map_feature_name($attr_name);
		if(exists $FEATURE_HANDLER_MAP{$type}->{$mname} and defined $attr_value and $attr_value){
			# mapped name is used to look up the function reference of feature handler
			my $hf_ref = $FEATURE_HANDLER_MAP{$type}->{$mname};
			# but the actual attribute name is used to generate feature name 
			my ($names, $values) = $hf_ref->($self, $type, $attr_name, $attr_value);
			exists $required_attr_map{$mname} and $required_attr_map{$attr_name} = 1;
			# set the feature values
			@result_feat_map{@$names} = @$values;
		}
	}
	# generate missing features if any
	my @miss_attrs = grep { not $required_attr_map{$_} } keys %required_attr_map;
	@result_feat_map{map {join("_",($type,$_,"#miss"))} @miss_attrs} = (1) x scalar @miss_attrs;
	return \%result_feat_map;
}

1;
