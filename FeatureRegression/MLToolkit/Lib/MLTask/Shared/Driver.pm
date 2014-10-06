package MLTask::Shared::Driver;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;
use FindBin;

# library path for feature handlers
use lib "$FindBin::Bin/../../"; use Exporter;
use JSON;
use Data::Dumper;
use MLTask::Shared::FeatureDict;
use MLTask::Shared::FeatureIndexer;
use MLTask::Shared::Utility qw(traverse_variable_recursive);
use MLTask::Amazon::WTP::Model;
use File::Basename;

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new{
	my ($class, $config_file) = @_;
	-f $config_file or die "configuration file must be provided and must be present";
	my $self = {};
	# parse the configuration file
	my $yaml = YAML::Tiny->read($config_file);
	$self->{config} = $yaml->[0];
	bless $self, $class;
	$self->_init();
	return $self;
}

sub _init{
	# intialize feature handler
	my $self = shift;
	my $cfg = $self->{config};
	my $work_dir = $cfg->{work_dir};
	-d $work_dir or die $!;
	# validate the file paths
	$self->_validate_task_config();
	my $dataset_name = $cfg->{dataset};
	my $task_name = $cfg->{"task"};
	# import the feature handler 
	my $feature_handler_class = join("::",("MLTask",$dataset_name,$task_name,"FeatureHandler"));
	eval "use $feature_handler_class";
	die $@ if $@;
	my $feature_handler = eval "new $feature_handler_class()";
	die  $@ if $@;
	$feature_handler->init(driver => $self);
	# import Model module
	my $model_class = join("::",("MLTask",$dataset_name,$task_name,"Model"));
	eval "use $model_class";
	die $@ if $@;
	$self->{model} = eval "new $model_class()";
	$self->{feature_handler} = $feature_handler;
	my $global_feature_handler_class = join("::",("MLTask",$dataset_name,$task_name,"GlobalFeatureHandler"));
	eval "use $global_feature_handler_class";
	die $@ if $@;
	my $global_feature_handler = eval "new $global_feature_handler_class()";
	die $@ if $@;
	$self->{global_feature_handler} = $global_feature_handler;
	$self->{global_feature_handler}->init(driver => $self);
}

sub get_full_path{
	my ($self,$config_path) = @_;
	my $cfg = $self->{config};
	return $cfg->{work_dir} . "/" . $config_path;
}

# get machine learning task configuration
sub _validate_task_config{
	my $self = shift;
	my $cfg = $self->{config};
	# get entity configuration
	while(my($entity_name,$entity_config) = each %{$cfg->{entities}}){
		my $type = $entity_config->{"type"};
		my $attribute_file = $entity_config->{"attribute_file"};
		my $feature_file = $entity_config->{"feature_file"};
		$type and $attribute_file and $feature_file or die "type, attribute_file, feature_file must be specified for each entity:$!";
	}
}


sub get_task_config{
	my $self = shift;
	return $self->{config};
}

sub get_entity_config{
	my $self = shift;
	my $entity_name = shift;
	my $cfg = $self->{config};
	return $cfg->{entities}->{$entity_name};
}

# get feature indexer given entity name
sub get_feature_indexer{
	my($self,$entity_name) = @_;
	my $task_config = $self->get_task_config();
	if(not exists $self->{entity_feature_indexer}->{$entity_name}){
		# otherwise, create one
		my $entity_config = $self->get_entity_config($entity_name);
		my $feature_indexer = new MLTask::Shared::FeatureIndexer();
		$feature_indexer->init(driver => $self, entity_name => $entity_name);
		$self->{entity_feature_indexer}->{$entity_name} = $feature_indexer;
	}
	return $self->{entity_feature_indexer}->{$entity_name};
}

sub init_data{
	my $self = shift;
	print ">>> initialize data\n";
	print ">>> generate global feature\n";
	$self->{global_feature_handler}->run();
	my $entity_config_map = $self->{config}->{entities};
	print ">>> generate entity features\n";
	while(my($entity_name,$entity_config) = each %$entity_config_map){
		my $entity_indexer = $self->get_feature_indexer($entity_name);
		my $feature_file = $entity_config->{feature_file};
		# already indexed
		if(scalar keys %{$entity_indexer->get_feature_map()}){
			print "[info] attribute file already indexed, skip\n";
		}else{
			$entity_indexer->index_file();
		}
	}
}

sub get_entity_attribute{
	my ($self,$entity_name) = @_;
	my $entity_config = $self->get_entity_config($entity_name);
	if(not exists $self->{entity_attribute_map}->{$entity_name}){
		my $entity_file = $self->get_full_path($entity_config->{attribute_file});
		open FILE , "<", $entity_file or die $!;
		print ">>> load entity attribute file - $entity_name\n";
		while(<FILE>){
			chomp;
			my $json_obj = decode_json($_);
			my $id = $json_obj->{id};
			defined $id or (warn "[warn] entity id not specified" and next);
			$self->{entity_attribute_map}->{$entity_name}->{$id} = $json_obj;
		}
	}
	return $self->{entity_attribute_map}->{$entity_name};
}

sub get_user_rating{
	my $self = shift;
	if(not exists $self->{user_rating_map}){
		# load from file
		$self->{user_rating_map} = $self->load_rating_from_file($self->get_full_path($self->get_task_config()->{rating_file}));
	}
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

sub get_feature_dict{
	my $self = shift;
	if(not exists $self->{feature_dict}){
		my $feature_dict_file = $self->get_full_path($self->get_task_config()->{feature_dict_file});
		$self->{feature_dict} = MLTask::Shared::FeatureDict::get_instance( file => $feature_dict_file );
	}
	return $self->{feature_dict};
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
	print ">>> generate sample features\n";
	$self->{model}->init(driver => $self);
	$self->{model}->generate_samples();
	$self->{model}->train();
}

1;
