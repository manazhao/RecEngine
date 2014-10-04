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
	my($class,%args) = @_;
	my @arg_keys = qw(entity_type attribute_file feature_file feature_dict_file feature_handler);
	my %class_members = ();
	@class_members{@arg_keys} = @args{@arg_keys};
	MLTask::Shared::Utility::check_func_args("new", \%class_members);
	
	$class_members{attribute_file} and (-f $class_members{attribute_file} or die "attribute file must exist");
	-d dirname($class_members{feature_file}) or die "invalid folder for feature file";
	my $self = \%class_members;
	$self->{feature_dict} = MLTask::Shared::FeatureDict::get_instance( file => $self->{feature_dict_file} );
	# hold all existing indexted feature
	$self->{entity_feature} = {};
	# hold original entity attribute
	$self->{entity_attribute} = {};
	
	bless $self, $class;
	# initialize properly
	$self->_init();
	return $self;
}


sub _init{
	my $self = shift;
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
	open FILE, "<" , $self->{attribute_file} or die "failed to open file - $self->{attribute_file}";
	print ">>> load entity profile from - " . $self->{attribute_file} . " and index attribute as features\n";
	while(<FILE>){
		chomp;
		my $json_obj = decode_json($_);
		$self->index_entity($json_obj);
	}
	close FILE;
}

sub DESTROY{
	my $self = shift;
	if($self->{feature_fh}){
		close $self->{feature_fh};
	}
}

1;
