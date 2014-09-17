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

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new{
	my ($class, $config_file) = @_;
	-f $config_file or die "configuration file must be provided and must be present";
	my $self = {};
	# parse the configuration file
	my $yaml = YAML::Tiny->read($config_file);
	my $cfg = $yaml->[0];
	# copy to class members
	@{$self->{keys %$cfg}} = values %$cfg;
	$self->{cfg} = $cfg;
}


sub process_data{
	my $self = shift;
	my $dataset_name = $self->{"dataset"};
	my $task_name = $self->{"task"};
	# import the feature handler 
	my $feature_handler_class = join("::",("MLTask",$dataset_name,$task_name,"FeatureHandler"));
	eval "use " . $feature_handler_class;
	my $model_class = join("::",("MLTask",$dataset_name,$task_name,"Model"));
	eval "use " . $feature_handler_class;
	# import the Model
	eval "use " . join("::",("MLTask",$dataset_name,$task_name,"Model"));
	my $input_folder = $self->{"input.folder"};
	my $result_folder = $self->{"result.folder"};
	my $feat_dict_file = $self->{"feature.dict.file"};
	-d $input_folder or die "input data folder - $input_folder does not exist: $!";
	-d $result_folder or die "result data folder - $result_folder does not exist: $!";
	$feat_dict_file = $result_folder . "/" . $feat_dict_file;
	my $entities = $self->{"entities"};
	my $data_processor = new MLTask::Shared::DataProcessor();
	my $feature_handler = eval "new $feature_handler_class()";
	# save model class name
	$self->{model_class} = $model_class;
	# generate entity features
	foreach my $entity (@$entities){
		# index each type of entity
		my $type = $entity->{"type"};
		my $json_file = $entity->{"json.file"};
		my $feature_file = $entity->{"feature.file"};
		$type and $json_file and $feature_file or die "type, json_file, feature_file must be specified for each entity:$!";
		$json_file = $input_folder . "/" . $json_file;
		$feature_file = $result_folder . "/" . $feature_file;
		-f $feature_file and print "feature file alrady exist - $feature_file, skip" and next;
		$data_processor->index_feature(
			type => $type,
			attribute_file => $json_file,
			feature_file => $feature_file,
			feature_handler => $feature_handler,
			feature_dict => $feat_dict_file
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
