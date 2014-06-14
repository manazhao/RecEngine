#!/usr/bin/perl 
#
# define functions for generating features given entity attributes
#
package Dataset::Amazon::FeatureHandler;

use strict;
use warnings;
use Exporter;

use vars qw($VERSION @ISA @EXPORT);

$VERSION = 1.0;
@ISA = qw (Exporter);
# export nothing. avoid clash
@EXPORT = ();


my %feature_handler_map = (
    "u" => {
        "gender" => \&default_feature_handler,
        "age" => \&user_age_feature_handler,
    },
    "i" => {
        # popularity feature
        "pop" => \&default_feature_handler,
        "m" => \&default_feature_handler,
        "c" => \&item_category_feature_handler,
#"au" => \&item_author_feature_handler, # 
	"pdate" => \&item_production_date_feature_handler,
        "id" => \&default_feature_handler
    }
);

my %required_features = (
	"u" => [ "gender", "age"],
	"i" => [ "m", "c", "au", "pdate","pop","id"]
);

sub get_feature_handler{
    return \%feature_handler_map;
}

sub get_required_feature{
    return \%required_features;
}

# generic feature handler
#
sub default_feature_handler{
    my($type, $feature, $value) = @_;
    # remove , from the value
    $value =~ s/\,//g;
    return ([join("_", ($type,$feature,$value))],[1]);
}

sub item_author_feature_handler{
    my($type, $feature, $value) = @_;
    # remove , from the value
    my @authors = ();
    if(ref $value eq "ARRAY"){
	    @authors = @$value;
    }else{
	    push @authors, $value;
    }
    @authors = map {$_ =~ s/\,//g; join("_",($type,$feature,$_));} @authors;
    return (\@authors, [(1) x scalar @authors]);
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

sub item_production_date_feature_handler{
	my($type, $feature, $value) = @_;
	# split by - and subtract 1900 from the year
	my($year,$month,$mday) = split /\-/, $value;
	$year -= 1900;
	return ([join("_", ($type,$feature,$year))],[1]);
}


1;
