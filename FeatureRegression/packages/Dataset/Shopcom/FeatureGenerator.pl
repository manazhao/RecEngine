#!/usr/bin/perl
#
package Dataset::Shopcom::FeatureGenerator;
use strict;
use warnings;
use Dataset::Shopcom;


sub new{
	my ($class) = shift;
	my $self = {@_};
	# must supply file to contain the result file
	die unless $self->{"result"};
	bless $self, $class;
}

# generate features
sub generate{
	my $self = shift;

}


