package MLTask::Shared::Driver;

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
use MLTask::Amazon::WTP::Model;
use Data::Dumper;

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new{
	my ($class, $config_file) = @_;
	-f $config_file or die "configuration file must be provided and must be present";
	my $self = {};
	# parse the configuration file
	my $yaml = YAML::Tiny->read($config_file);
	my $cfg = $yaml->[0];
	$self->{yml_cfg} = $cfg;
	bless $self, $class;
	$self->_init();
	return $self;
}


sub _init{
	# intialize feature handler
	my $self = shift;
	my $cfg = $self->{yml_cfg};
	my $dataset_name = $cfg->{"dataset"};
	my $task_name = $cfg->{"task"};
	# import the feature handler 
	my $feature_handler_class = join("::",("MLTask",$dataset_name,$task_name,"FeatureHandler"));
	eval "use $feature_handler_class";
	warn $@ if $@;
	my $model_class = join("::",("MLTask",$dataset_name,$task_name,"Model"));
	eval "use $model_class";
	warn $@ if $@;
	my $input_folder = $cfg->{"input.folder"};
	my $result_folder = $cfg->{"result.folder"};
	my $feat_dict_file = $cfg->{"feature.dict.file"};
	-d $input_folder or die "input data folder - $input_folder does not exist: $!";
	-d $result_folder or die "result data folder - $result_folder does not exist: $!";
	$feat_dict_file = $result_folder . "/" . $feat_dict_file;
	my $entities = $cfg->{"entities"};
	my $feature_handler_config = $cfg->{"feature.handler.config"};
	my $feature_handler = eval "new $feature_handler_class()";
	# initialize feature handler
	if($feature_handler_config){
		$feature_handler->init(%$feature_handler_config);
	}
	# save model class name
	$self->{model_class} = $model_class;
	$self->{feature_handler} = $feature_handler;
}

# get machine learning task configuration
sub get_task_config{
	my $self = shift;
	if(not exists $self->{task_config}){
		my $yml_cfg = $self->{yml_cfg};
		my $task_config = {
			"dataset" => $yml_cfg->{dataset},
			"task" => $yml_cfg->{task},
			"input_folder" => $yml_cfg->{'input.folder'},
			"result_folder" => $yml_cfg->{'result.folder'},
			"feature_dict_file" => $yml_cfg->{'result.folder'} . "/" . $yml_cfg->{'feature.dict.file'},
			"rating_file" => $yml_cfg->{'input.folder'} . "/" . $yml_cfg->{'rating.file'}

		};
		my $input_folder = $task_config->{input_folder};
		my $result_folder = $task_config->{result_folder};
		# get entity configuration
		while(my($entity_name,$entity_config) = each %{$yml_cfg->{entities}}){
			my $type = $entity_config->{"type"};
			my $json_file = $entity_config->{"attribute.file"};
			my $feature_file = $entity_config->{"feature.file"};
			$type and $json_file and $feature_file or die "type, json_file, feature_file must be specified for each entity:$!";
			$json_file = $input_folder . "/" . $json_file;
			$feature_file = $result_folder . "/" . $feature_file;
			$task_config->{entity_config}->{$entity_name} = {
				name => $entity_name, type => $type, attribute_file => $json_file, feature_file => $feature_file};
		}
		$self->{task_config} = $task_config;
	}
	return $self->{task_config};
}

# original yml config
sub get_yml_config{
	my $self = shift;
	return $self->{yml_cfg};
}

sub get_entity_config{
	my $self = shift;
	my $entity_name = shift;
	my $task_config = $self->get_task_config();
	return $task_config->{entity_config}->{$entity_name};
}

# create feature indexer given feature file name
sub load_feature_indexer{
	my($self,$entity_type, $feature_file) = @_;
	my $task_config = $self->get_task_config();
	my $feature_indexer = new MLTask::Shared::FeatureIndexer(
		entity_type => $entity_type,
		attribute_file => "",
		feature_file => $feature_file,
		feature_handler => $self->{feature_handler},
		feature_dict_file => $task_config->{feature_dict_file}
	);
	return $feature_indexer;
}

# get feature indexer given entity name
sub get_feature_indexer{
	my($self,$entity_name) = @_;
	my $task_config = $self->get_task_config();
	if(not exists $self->{entity_feature_indexer}->{$entity_name}){
		# otherwise, create one
		my $entity_config = $self->get_entity_config($entity_name);
		my $feature_indexer = new MLTask::Shared::FeatureIndexer(
			entity_type => $entity_config->{type},
			attribute_file => $entity_config->{attribute_file},
			feature_file => $entity_config->{feature_file},
			feature_handler => $self->{feature_handler},
			feature_dict_file => $task_config->{feature_dict_file}
		);
		$self->{entity_feature_indexer}->{$entity_name} = $feature_indexer;
	}
	return $self->{entity_feature_indexer}->{$entity_name};
}

sub init_data{
	print ">>> initialize data\n";
	my $self = shift;
	my $task_config = $self->get_task_config();
	# process entity features
	my $entity_config_map = $task_config->{entity_config};
	while(my($entity_name,$entity_config) = each %$entity_config_map){
		my $entity_indexer = $self->get_feature_indexer($entity_name);
		my $feature_file = $entity_config->{feature_file};
		-f $entity_config->{feature_file} and (print "feature file - $feature_file already exists, skip" and next);
		$entity_indexer->index_file();
	}
	# load ratings
	$self->{user_rating_map} = $self->load_rating_from_file($task_config->{rating_file});
}

sub get_user_rating{
	my $self = shift;
	return $self->{user_rating_map};
}


# load ratings from file
sub load_rating_from_file{
	my ($self,$rating_file) = @_;
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

# write rating to file
sub write_rating_to_file{
	my($self,$file,$user_ratings) = @_;
	open FILE, ">", $file or die $!;
	while(my($uid,$ratings) = each %$user_ratings){
		map {print FILE join("\t",($uid,@$_))."\n"} @$ratings;
	}
	close FILE;
}



# only keep ratings of given items
sub filter_rating_by_items{
	my($self,$user_rating_map,$items) = @_;
	my %item_map = ();
	@item_map{@$items} = (1) x scalar @$items;
	my %result_rating_map = ();
	while(my($uid,$tmp_ratings) = each %$user_rating_map){
		# check whether the items rated by current user are present in $items
		# each of $tmp_ratings is tuple <item id, rating, time>
		my @filtered_ratings = grep {$item_map{$_->[0]}} @$tmp_ratings;
		if(scalar @filtered_ratings > 0){
			$result_rating_map{$uid} = \@filtered_ratings;
			$user_rating_map->{$uid} = \@filtered_ratings;
		}
	}
	return \%result_rating_map;
}

sub filter_rating_by_users{
	my($self,$user_rating_map,$users) = @_;
	my %user_map = ();
	@user_map{@$users} = (1) x scalar @$users;
	my %result_rating_map = ();
	while(my($uid,$tmp_ratings) = each %$user_rating_map){
		# check whether the items rated by current user are present in $items
		# each of $tmp_ratings is tuple <item id, rating, time>
		$user_map{$uid} and $result_rating_map{$uid} = $tmp_ratings;
	}
	return \%result_rating_map;
}

sub train_model{
	my $self = shift;
	# create model object
	my $model_obj = eval "new $self->{model_class}()";
	$model_obj->train($self);
}

1;
