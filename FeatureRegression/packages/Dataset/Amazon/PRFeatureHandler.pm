#!/usr/bin/perl 
#
# feature handler for price regression task
#
package Dataset::Amazon::PRFeatureHandler;

use strict;
use warnings;
use Exporter;

use vars qw($VERSION @ISA @EXPORT);

$VERSION = 1.0;
@ISA = qw (Exporter);
# export nothing. avoid clash
@EXPORT = ();

my %feature_handler_map = (
    "i" => {
        "rc" => \&numerical_feature_handler, # review count
        "ar" => \&numerical_feature_handler, # average rating
        "br" => \&categorical_feature_handler, # brand
	"dt" => \&item_production_date_feature_handler, # release date
	"tf_" => \&numerical_feature_handler,
	"idf_" => \&numerical_feature_handler
    }
);

my %required_features = (
	"i" => [ "br","dt"]
);

sub get_feature_handler{
    return \%feature_handler_map;
}

sub get_required_feature{
    return \%required_features;
}

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

sub numerical_feature_handler{
    my($type, $feature, $value) = @_;
    return ([join("_",($type,$feature))],[$value]);
}

sub item_production_date_feature_handler{
	my($type, $feature, $value) = @_;
	# split by - and subtract 1900 from the year
	my($year,$month,$mday) = split /\-/, $value;
	$year -= 1900;
	return ([join("_", ($type,$feature,$year))],[1]);
}


1;
