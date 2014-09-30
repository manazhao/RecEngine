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
	$feature_indexer->index();
}

# entity indexed features given an entity
sub load_feature{
	my($self,$feature_file) = @_;
	open FEATURE_FILE, "<" , $feature_file or die $!;
	my %entity_feat_map = ();
	while(<FEATURE_FILE>){
		chomp;
		my ($type_entity_id, @feats) = split /\,/;
		my ($type,$id) = split /\_/, $type_entity_id;
		foreach(@feats){
			push @{$entity_feat_map{$id}}, [split /\:/, $_];
		}
	}
	close FEATURE_FILE;
	return \%entity_feat_map;
}

1;
