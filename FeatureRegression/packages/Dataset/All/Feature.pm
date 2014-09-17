#!/usr/bin/perl 
#
# define functions for generating features given entity attributes
#
package Dataset::All::Feature;

use strict;
use warnings;
use Exporter;

use vars qw($VERSION @ISA @EXPORT);

$VERSION = 1.0;
@ISA = qw (Exporter);
# export nothing. avoid clash
@EXPORT = ();


# generate interaction data based on the pattern
#


sub interact_feature{
    # each pattern is specified as a list of regular expression

    my ($, $entity_features) = @_;


}


# generate interaction feature given two set of features
sub interact_two_features{
    # extract the two sets of features
    my($first_set, $second_set) = @_;
    # resulted interaction features
    my @int_features = ();
    # perform Cartesian product
    foreach(@$first_set){
        my $first_feature = $_;
        foreach(@$second_set){
            my $second_feature = $_;
            my $int_feature = join("|",($first_feature,$second_feature));
            push @int_features, $int_feature;
        }
    }
    return \@int_features;
}

1;


