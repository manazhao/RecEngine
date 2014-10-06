# generate feature file from a given entity attribute file
package MLTask::Shared::FeatureIndexer;

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
# feature dictionary object
use MLTask::Shared::FeatureDict;
use MLTask::Shared::Utility;

# use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new {
	my($class) = @_;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub init{
	my ($self,%args)  = @_;
	my $driver = $args{driver};
	my $entity_name = $args{entity_name};
	$driver or die "MLTask::Shared::Driver reference missing";
	$entity_name or die "entity name missing";
	$self->{driver} = $driver;
	$self->{entity} = $entity_name;
	my $task_config = $driver->get_task_config();
	$self->{entity_type} = $task_config->{entities}->{$self->{entity}}->{type};
	$self->{feature_handler} = $driver->{feature_handler};
	$self->{feature_dict} = $driver->get_feature_dict();
	# set entity feature file
	$self->{feature_file} = $driver->get_full_path($task_config->{entities}->{$self->{entity}}->{feature_file});
	# hold all existing indexted feature
	$self->{entity_feature} = {};
	# load existing features
	$self->_load_indexed_feature();
	# for new features
	open $self->{feature_fh}, ">>", $self->{feature_file} or die $!;
}

# load features that are already indexed
sub _load_indexed_feature{
	my $self = shift;
	if(-f $self->{feature_file}){
		open FEATURE_FILE, "<", $self->{feature_file} or die $!;
		while(<FEATURE_FILE>){
			chomp;
			my ($type_entity_id, @feats) = split /\,/;
			my ($type,$id) = split /\_/, $type_entity_id;
			$self->{entity_feature}->{$id} = [map { [split /\:/] } @feats]
		}
		close FEATURE_FILE;
	}
}

# retrieve feature
sub get_feature{
	my($self,$id) = @_;
	# in case of new entity with no attributes introduced
	if(not exists $self->{entity_feature}->{$id}){
		$self->index_entity({id => $id});
	}
	return $self->{entity_feature}->{$id};
}

sub get_feature_map{
	my $self = shift;
	return $self->{entity_feature};
}

sub index_entity{
	my($self,$json_obj) = @_;
	my $id = $json_obj->{id};
	# make sure at least entity id is provided
	defined $id or (warn "entity id not specified" and return);
	# check whether feature is indexed
	my $feature_fh = $self->{feature_fh};
	if(not exists $self->{entity_feature}->{$id}){
		my $entity_id = join("_", ($self->{entity_type},$json_obj->{"id"}));
		# remove entity id so only attributes remain
		delete $json_obj->{"id"};
		my $featname_val_map = $self->{feature_handler}->generate_feature($self->{entity_type},$json_obj);
		my %featidx_val_map = ();
		while(my ($feat_name, $feat_val) = each %{$featname_val_map}){
			my $feat_id = $self->{feature_dict}->index_feature($feat_name);
			$featidx_val_map{$feat_id} = $feat_val;
		}
		# flatten to array
		my @feat_arr = map {[$_, $featidx_val_map{$_}]} keys %featidx_val_map;
		# add to feature map
		$self->{entity_feature}->{$id} = \@feat_arr;
		# construct feature string in LIBSVM format
		my $feat_str = join(",", map { join(":", @$_) } @feat_arr);
		# append to feature file
		print  $feature_fh join(",",($entity_id, $feat_str)) . "\n";
	}
}

sub index_file{
	my $self = shift;
	my $driver = $self->{driver};
	my $entity_attr_map = $driver->get_entity_attribute($self->{entity});
	while(my ($entity_id,$json_obj) = each %$entity_attr_map){
		$self->index_entity($json_obj);
	}
}

sub DESTROY{
	my $self = shift;
	if($self->{feature_fh}){
		close $self->{feature_fh};
	}
}

1;
