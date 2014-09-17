package RecSys::Data;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;
use FindBin;

# library path for feature handlers
use lib "$FindBin::Bin/..";

use Dataset::Common;
use Exporter;
use JSON;
use Data::Dumper;

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub process_dataset {
	# $ini_file: dataset configuration file which defines all entity files and feature handlers
	my($ini_file) = @_;
	$ini_file and -f $ini_file or die "configuration file must be provided and must be present in file system:$!";
	my $yaml = YAML::Tiny->read($ini_file);
	my $cfg = $yaml->[0];
	my $dataset_name = $cfg->{"dataset"};
	# import the feature handler properly
	eval "use Dataset::$dataset_name";
	my $input_folder = $cfg->{"input.folder"};
	my $result_folder = $cfg->{"result.folder"};
	my $feat_dict_file = $cfg->{"feature.dict.file"};
	# rating (purchasing) information file
	my $rating_data_file = $cfg->{"rating.file"};

	$dataset_name and $feat_dict_file and $input_folder and $result_folder or die "dataset_name, feat_dict_file, input_folder, result_folder must be specified in the configuration file:$!";
	-d $input_folder or die "input data folder - $input_folder does not exist: $!";
	-d $result_folder or die "result data folder - $result_folder does not exist: $!";
	$feat_dict_file = $result_folder . "/" . $feat_dict_file;
	# file name
	my $rating_data_fn = $rating_data_file;
	$rating_data_file = $input_folder . "/" . $rating_data_file;
	my $user_feat_file;
	my $item_feat_file;

	my $entities = $cfg->{"entities"};
	foreach my $entity (@$entities){
		# index each type of entity
		my $type = $entity->{"type"};
		my $json_file = $entity->{"json.file"};
		my $feature_file = $entity->{"feature.file"};
		$type and $json_file and $feature_file or die "type, json_file, feature_file must be specified for each entity:$!";
		$json_file = $input_folder . "/" . $json_file;
		$feature_file = $result_folder . "/" . $feature_file;
		if($type eq 'u'){
			$user_feat_file = $feature_file;
		}else{
			$item_feat_file = $feature_file;
		}
		-f $feature_file and next;
		Dataset::Common::gen_entity_feature($dataset_name,$type,$feat_dict_file,$json_file,$feature_file);
	}
	# import the dataset related module
	eval "use lib Dataset::$dataset_name;";
	# generate training information
	my $MODULE_REF = eval "Dataset::$dataset_name->get_module_ref()";
	my $train_sample_file = $result_folder . "/" .$rating_data_fn . ".train.libsvm";
	$MODULE_REF->train($feat_dict_file, $user_feat_file, $item_feat_file, $rating_data_file, $train_sample_file);

}

1;
