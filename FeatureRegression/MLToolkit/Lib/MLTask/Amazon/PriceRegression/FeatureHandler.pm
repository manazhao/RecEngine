#!/usr/bin/perl 
#
# feature handler for price regression task
#
package MLTask::Amazon::PriceRegression::FeatureHandler;

use strict;
use warnings;
use Exporter;

our $VERSION = 1.0;
our @ISA = qw (Exporter);
# export nothing. avoid clash
our @EXPORT = ();

# feature handlers registration
my %FEATURE_HANDLER_MAP = (
    "i" => {
        "rc" => \&numerical_feature_handler, # review count
        "ar" => \&numerical_feature_handler, # average rating
        "br" => \&categorical_feature_handler, # brand
	"dt" => \&item_production_date_feature_handler, # release date
	"quantity_tt" => \&numerical_feature_handler, # item quantity
	"sp" => \&numerical_feature_handler, # sales price
	"pr" => \&numerical_feature_handler, # original price
	"tf_" => \&numerical_feature_handler, # tf for bow
	"tfidf_" => \&numerical_feature_handler, # tfidf for bow
	"lc" => \&categorical_feature_handler
    }
);

my %REQUIRED_FEATURES = (
	"i" => [ "br","dt", "ar", "rc"]
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
    my($type, $feature, $value) = @_;
    $value = process_text_value($value);
    return ([join("_", ($type,$feature,$value))],[1]);
}
# numerical feature handlers
# the feature name is combination of type and feature name, the value is the numerical value passed in
sub numerical_feature_handler{
    my($type, $feature, $value) = @_;
    return ([join("_",($type,$feature))],[$value]);
}

# date is treated as categorical feature
#
sub item_production_date_feature_handler{
	my($type, $feature, $value) = @_;
	# split by - and subtract 1900 from the year
	my($year,$month,$mday) = split /\-/, $value;
	$year -= 1900;
	return ([join("_", ($type,$feature,$year))],[1]);
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
			my ($names, $values) = $hf_ref->($type, $attr_name, $attr_value);
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
