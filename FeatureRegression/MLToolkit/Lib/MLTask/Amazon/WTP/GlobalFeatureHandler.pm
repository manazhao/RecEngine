#!/usr/bin/perl 
#
# aggregate feature handler
# generate features based on aggregated data like ratings, reviews
# e.g. item popularity, brand popularity, brand popularity, average rating and so on
# the extracted aggregated feature will be integrated in FeatureHandler
package MLTask::Amazon::WTP::GlobalFeatureHandler;
use strict;
use warnings; 
use Exporter; 
use FindBin;
use lib "$FindBin::Bin/../../../";
use MLTask::Shared::Utility;
use Data::Dumper;
use File::Basename; 
our $VERSION = 1.0; our @ISA = qw (Exporter); # export nothing. avoid clash our @EXPORT = (); 

sub new{
	my ($class, %args) = @_;
	my $self = {};
	bless $self, $class;
}


sub init{
	my($self,%args) = @_;
	my $driver = $args{driver};
	$driver or die "MLTask::Shared::Driver reference missing";
	$self->{driver} = $driver;
	my $task_config = $driver->get_task_config();
	my $handler_config = $task_config->{global_feature_handler_config};
	# get user rating map
	my $user_rating_map = $driver->get_user_rating();
	# item attribute profile
	my $item_attribute_file = $driver->get_full_path($task_config->{entities}->{item}->{attribute_file});
	# load 
	$self->{item_global_file} = $driver->get_full_path($handler_config->{item_global_file});
	$self->{brand_global_file} = $driver->get_full_path($handler_config->{brand_global_file});
	(-f $self->{item_global_file} or -f $self->{brand_global_file}) and return;
	open $self->{item_global_fh}, ">", $self->{item_global_file} or die $!;
	open $self->{brand_global_fh}, ">", $self->{brand_global_file} or die $!;
}

# generate the aggregated feature for entities
sub run{
	# generate the following features
	# item popularity, namely, number of reviews
	# brand popularity, namely, number of ratings for the brand
	# output to file
	my $self = shift;
	($self->{item_global_fh} and $self->{brand_global_fh}) or return;
	my %item_pop_map = ();
	# average rating
	my %item_ar_map = ();
	my %brand_pop_map = ();
	my $user_ratings = $self->{driver}->get_user_rating();
	my $item_attribute_map = $self->{driver}->get_entity_attribute("item");
	my $max_brand_cnt = 0;
	my $max_item_cnt = 0;
	while(my($user_id, $tmp_ratings) = each %$user_ratings){
		foreach my $item_rating (@$tmp_ratings){
			my($item_id,$rating,$time) = @$item_rating;
			$item_pop_map{$item_id}++;
			$item_pop_map{$item_id} > $max_item_cnt and $max_item_cnt = $item_pop_map{$item_id};
			$item_ar_map{$item_id} += $rating;
			my $tmp_brand = $item_attribute_map->{$item_id}->{"br"};
			$tmp_brand or next;
			$brand_pop_map{$tmp_brand}++;
			$brand_pop_map{$tmp_brand} > $max_brand_cnt and $max_brand_cnt = $brand_pop_map{$tmp_brand};
		}
	}
	# get the average rating
	my $item_fh = $self->{item_global_fh};
	my $brand_fh = $self->{brand_global_fh};
	map { print $item_fh join("\t", ($_, sprintf("%.4f",$item_pop_map{$_}/$max_item_cnt), sprintf("%.4f",$item_ar_map{$_}/$item_pop_map{$_}))) . "\n" } keys %item_ar_map;
	map { print $brand_fh join("\t",($_,sprintf("%.4f",$brand_pop_map{$_}/$max_brand_cnt)))."\n" } keys %brand_pop_map;
}

sub DESTROY{
	my $self = shift;
	$self->{item_global_fh} and close $self->{item_global_fh};
	$self->{brand_pop_fh} and close $self->{brand_global_fh};
}

1;

