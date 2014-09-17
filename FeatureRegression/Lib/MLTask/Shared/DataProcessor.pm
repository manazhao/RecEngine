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
	my ($self, $entity_type, $attr_file, $feature_file, $feat_dict_file, $feature_handler) = @_;
	-f $attr_file or die "entity attribute file - $attr_file does not exist";
	-f $feat_dict_file or die "feature dictionary file - $feat_dict_file does not exist";
	# create feature dictionary
	my $feat_dict = MLTask::Shared::FeatureDict::get_instance(file => $feat_dict_file);
	my $feature_indexer = new MLTask::Shared::FeatureIndexer(
		type => $entity_type,
		attribute_file => $attr_file,
		feature_file => $feature_file,
		feature_handler => $feature_handler,
		feature_dict => $feat_dict_file
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

;
