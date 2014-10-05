#!/usr/bin/perl 
#
# aggregate feature handler
# generate features based on aggregated data like ratings, reviews
# e.g. item popularity, brand popularity, brand popularity, average rating and so on
# the extracted aggregated feature will be integrated in FeatureHandler
#
package MLTask::Amazon::WTP::AggFeatureHandler;

use strict;
use warnings;
use Exporter;
use FindBin;
use lib "$FindBin::Bin/../../../";
use MLTask::Shared::Utility;
use Data::Dumper;
use File::Basename;

our $VERSION = 1.0;
our @ISA = qw (Exporter);
# export nothing. avoid clash
our @EXPORT = ();

sub new{
    my ($class, %args) = @_;
    my @required_args = (user_rating_map,item_attribute_map,result_folder);
    my %default_args = ();
    @default_args{@required_args} = (undef) x scalar @required_args;
    @default_args{keys %args} = values %args;
    check_func_args("new MLTask::Amazon::WTP::AggFeatureHandler", \%default_args);
    # check the existence of result file folder
    my $self = \%default_args;
    bless $self, $class;
    -d $result_folder or die $!;
}




# generate the aggregated feature for entities
sub run{
    # generate the following features
    # item popularity, namely, number of reviews
    # brand popularity, namely, number of ratings for the brand
    # output to file
    my $item_agg_file = $self->{result_folder} . "/item_agg.csv";
    my $brand_agg_file = $self->{result_folder} . "/brand_agg.csv";

    open ITEM_AGG_FILE, ">", $item_agg_file or die $!;
    open BRAND_AGG_FILE, ">", $brand_agg_file or die $!;
    
    my $self = shift;
    my %item_pop_map = ();
    # average rating
    my %item_ar_map = ();
    my %brand_pop_map = ();
    my $user_ratings = $self->{user_rating_map};
    my $item_attribute_map = $self->{item_attribute_map};
    my $max_brand_cnt = 0;
    my $max_item_cnt = 0;
    while(my($user_id, $tmp_ratings) = each %$user_ratings){
        foreach my $item_rating (@$tmp_ratings){
            my($item_id,$rating,$time) = @$item_rating;
            $item_pop_map{$item_id}++;
            $item_pop_map{$item_id} > $max_item_cnt and $max_item_cnt = $item_pop_map{$item_id};
            $item_ar_map{$item_id} += $rating;
            my $tmp_brand = $item_attribute_map->{$item_id}->{"br"};
            $brand_pop_map{$tmp_brand}++;
            $brand_pop_map{$tmp_brand} > $max_brand_cnt and $max_brand_cnt = $brand_pop_map{$tmp_brand};
        }
    }
    # get the average rating
    map { print ITEM_AGG_FILE join("\t", ($_, sprintf("%.4f",$item_pop_map{$_}/$max_item_cnt), sprintf("%.4f",$item_ar_map{$_}/$item_pop_map{$_}))) . "\n" } keys %item_ar_map;
    map { print BRAND_AGG_FILE join("\t",($_,sprintf("%.4f",$brand_pop_map{$_}/$max_brand_cnt))."\n" } keys %brand_pop_map;
    close ITEM_AGG_FILE;
    close BRAND_AGG_FILE;
}

1;

