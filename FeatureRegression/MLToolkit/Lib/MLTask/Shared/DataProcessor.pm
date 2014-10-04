package MLTask::Shared::DataProcessor;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;
use FindBin;

# library path for feature handlers
use lib "$FindBin::Bin/../../";
use Exporter;
use JSON;
use Data::Dumper;
use MLTask::Shared::FeatureDict;
use MLTask::Shared::FeatureIndexer;
use MLTask::Shared::Utility;

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new {
	my($class,%args) = @_;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub index_feature{
	# $self: object itself
	# $entity_type: entity type e.g. user or item
	# $attr_file: entity attribute file
	# $feat_dict_file: feature dictionary file
	# $feature_handler: feature handler object
	my ($self, %args) = @_;
	my @arg_keys = qw(entity_type attribute_file feature_file feature_dict_file feature_handler);
	my %default_args = ();
	@default_args{@arg_keys} = (undef) x scalar @arg_keys;
	# overwrite the values
	@default_args{@arg_keys} = @args{@arg_keys};
	my $err_msg = MLTask::Shared::Utility::check_func_args("index_feature",\%default_args);
	$err_msg and die $err_msg;

	my ($entity_type, $attribute_file, $feature_file, $feature_dict_file, $feature_handler) = @default_args{@arg_keys};

	-f $attribute_file or die "entity attribute file - $attribute_file does not exist";
	-f $feature_dict_file or die "feature dictionary file - $feature_dict_file does not exist";
	# create feature dictionary
	my $feat_dict = MLTask::Shared::FeatureDict::get_instance(file => $feature_dict_file);
	# create feature handler
	my $cfg = $self->{cfg};
	my $feature_indexer = new MLTask::Shared::FeatureIndexer(
		entity_type => $entity_type,
		attribute_file => $attribute_file,
		feature_file => $feature_file,
		feature_handler => $feature_handler,
		feature_dict_file => $feature_dict_file
	);
	$feature_indexer->index_file();
}


sub load_rating{
	my($self,$rating_file) = @_;
	open FILE, "<", $rating_file or die $!;
	my $user_rating_map = {};
	while(<FILE>){
		chomp;
		my($uid,$iid,$rate,$time) = split /\t/;
		push @{$user_rating_map->{$uid}}, [$iid,$rate,$time];
	}
	# sort ratings
	while(my($uid,$tmp_ratings) = each %$user_rating_map){
		my @sort_ratings = sort {$a->[2] <=> $b->[2]} @$tmp_ratings;
		$user_rating_map->{$uid} = \@sort_ratings;
	}
	return $user_rating_map;
}


# only keep ratings of given items
sub filter_rating_by_items{
	my($self,$user_rating_map,$items) = @_;
	my %item_map = ();
	@item_map{@$items} = (1) x scalar @$items;
	while(my($uid,$tmp_ratings) = each %$user_rating_map){
		# check whether the items rated by current user are present in $items
		# each of $tmp_ratings is tuple <item id, rating, time>
		my @filtered_ratings = grep {$item_map{$_->[0]}} @$tmp_ratings;
		if(scalar @filtered_ratings > 0){
			$user_rating_map->{$uid} = \@filtered_ratings;
		}else{
			# delete user with none ratings
			delete $user_rating_map->{$uid};
		}
	}
}
1;
