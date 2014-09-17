#!/usr/bin/perl 
#
# feature handler for price regression task
#
package MLTask::Amazon::PriceRegression;

use strict;
use warnings;
use Exporter;

use vars qw($VERSION @ISA @EXPORT);

$VERSION = 1.0;
@ISA = qw (Exporter);
# export nothing. avoid clash
@EXPORT = ();

# feature handlers registration
my %FEATURE_HANDLER_MAP = (
    "i" => {
        "rc" => \&numerical_feature_handler, # review count
        "ar" => \&numerical_feature_handler, # average rating
        "br" => \&categorical_feature_handler, # brand
	"dt" => \&item_production_date_feature_handler, # release date
	"tf_" => \&numerical_feature_handler,
	"idf_" => \&numerical_feature_handler
    }
);

my %REQUIRED_FEATURES = (
	"i" => [ "br","dt", "ar", "rc"]
);

# generic feature handler
# categorical feature by simply joining entitytype_feature_value
sub categorical_feature_handler{
    my($type, $feature, $value) = @_;
    # remove, and space from the value
    $value =~ s/\s\,//g;
    # to lowercase
    $value = lc $value;
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


sub map_feature_name{
	my ($feature) = @_;	
	$feature =~ m/tf_/ and return "tf";
	$feature =~ m/tfidf_/ and return "tfidf";
	return $feature;
}

sub new {
	my($class, %args) = @_;
	my $self = {};
	bless $self, $class;
	return $self;
}


sub generate_feature{
	my($self,$type,$attribute_map) = @_;
	my %required_attr_map = ();
	@required_attr_map{@REQUIRED_FEATURES} = (0) x scalar @REQUIRED_FEATURES;
	my %result_feat_map = ();
	while(my($attr_name, $attr_value) = each %$attribute_map){
		$attr_name = map_feature_name($attr_name);
		if(exists $FEATURE_HANDLER_MAP{$type}->{$attr_name} and defined $attr_value){
			my $hf_ref = $FEATURE_HANDLER_MAP{$type}->{$attr_name};
			my ($names, $values) = $hf_ref->($type, $attr_name, $attr_value);
			$required_attr_map{$attr_name} = 1;
			@result_feat_map{@$names} = @$values;
		}
	}
	# generate missing features if any
	my @miss_attrs = grep { not $required_attr_map{$_} } keys %required_attr_map;
	@result_feat_map{map {join("_",($type,$_,"#miss"))} @miss_attrs} = (1) x scalar @miss_attrs;
	return \%result_feat_map;
}

1;
