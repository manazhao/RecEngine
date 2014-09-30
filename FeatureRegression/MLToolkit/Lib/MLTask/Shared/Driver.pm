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
use MLTask::Shared::DataProcessor;
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
	$self->{cfg} = $cfg;
	bless $self, $class;
	return $self;
}


# get machine learning task configuration
sub get_task_cfg{
	my $self = shift;
	return $self->{cfg};
}

sub process_data{
	my $self = shift;
	my $cfg = $self->{cfg};
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
	my $data_processor = new MLTask::Shared::DataProcessor();
	my $feature_handler = eval "new $feature_handler_class()";
	# save model class name
	$self->{model_class} = $model_class;
	# generate entity features
	foreach my $entity (@$entities){
		# index each type of entity
		my $type = $entity->{"type"};
		my $json_file = $entity->{"attribute.file"};
		my $feature_file = $entity->{"feature.file"};
		$type and $json_file and $feature_file or die "type, json_file, feature_file must be specified for each entity:$!";
		$json_file = $input_folder . "/" . $json_file;
		$feature_file = $result_folder . "/" . $feature_file;
		-f $feature_file and warn "feature file alrady exist - $feature_file, skip\n" and next;
		$data_processor->index_feature(
			entity_type => $type,
			attribute_file => $json_file,
			feature_file => $feature_file,
			feature_handler => $feature_handler,
			feature_dict_file => $feat_dict_file
		);
	}
}


sub train_model{
	my $self = shift;
	# create model object
	my $model_obj = eval "new $self->{model_class}()";
	$model_obj->train($self->{cfg});
}

1;
