package RecSys::Data::Common;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;

# library path for feature handlers
use lib "../../";

use Exporter;
use JSON;
use Dataset::Amazon::FeatureHandler;
use Data::Dumper;

use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

$VERSION = 1.00;
@ISA = qw(Exporter);
# list of functions export by default, empty here
@EXPORT = qw(load_entity_json generate_entity_feature index_entity_feature);
# list of functions to export
@EXPORT_OK = qw(load_entity_json generate_entity_feature index_entity_feature);


# load objects defined in json format
# return hash map which contains all objects
sub load_entity_json{
	my($type, $file) = @_;
	# make sure file exists
	-f $file or die "file - $file does not exist: $!";
	# read the file
	my %obj_feat_map = ();
	open FILE, "<" , $file or die "failed to open file - $file: $!";
	while(<FILE>){
		chomp;
		my $json_obj = decode_json($_);
		# get the id field
		exists $json_obj->{"id"} or (warn "object id is not defined" and next);
		my $id = join("_", ($type,$json_obj->{"id"}));
		delete $json_obj->{"id"};
		while( my ($feat_name, $feat_val) = each %$json_obj){
			$obj_feat_map{$id}->{$feat_name} = $feat_val;	
		}
	}
	close FILE;
	return \%obj_feat_map;
}


# generate feature given entity attributes which are loaded from json file
sub generate_entity_feature {
	# $entity_id: entity id
	# $attribute_map: attributes for the entity. key is attribute name and value is attribute value
	# $required_attributes: attributes that are required
	# $type: type of entity. e.g. user, item, etc.. 
	# $hf_ref: hash map which associates function handler to attributes
	my($type, $entity_id, $attribute_map, $required_attributes, $hf_ref_map) = @_;
	my %required_attr_map = ();
	@required_attr_map{@$required_attributes} = (0) x scalar @$required_attributes;
	my %result_feat_map = ();
	while(my($attr_name, $attr_value) = each %$attribute_map){
            if(exists $hf_ref_map->{$type}->{$attr_name} and $attr_value){
                my $hf_ref = $hf_ref_map->{$type}->{$attr_name};
                my ($names, $values) = $hf_ref->($type, $attr_name, $attr_value);
                $required_attr_map{$attr_name} = 1;
                @result_feat_map{@$names} = @$values;
            }
        }
        # generate missing features if any
        my @miss_attrs = grep { not $required_attr_map{$_} } keys %required_attr_map;
        @result_feat_map{map {join("_",($type,$_,"#miss"))} @miss_attrs} = (1) x scalar @miss_attrs;
        return \%result_feat_map;
}



# convert feature name to integers
sub index_entity_feature{
    my($feat_dict,$max_feat_id, $entity_feat_map, $dict_fid) = @_;
    my %result_feat_map = (); while(my ($feat_name, $feat_val) = each %$entity_feat_map){
        my $feat_id;
        if(exists $feat_dict->{$feat_name}){
            $feat_id = $feat_dict->{$feat_name};
        }else{
            $$max_feat_id++;
            $feat_id = $$max_feat_id;
            $feat_dict->{$feat_name} = $feat_id;
            print $dict_fid join(",",($feat_name,$feat_id)) . "\n" ;
        }
        $result_feat_map{$feat_id} = $feat_val;
    }
    return \%result_feat_map;
}

# 
sub process_entity_file{
	# dataset_name: dataset name, e.g. Amazon. used to load feature handler
	# $type: type of entity. e.g. "u" for user and "i" for item, etc.
	# $entity_json_file: json file which contains the entity information.
	# $dict_file: feature dictionary file. new features will be appended to the file
	# $entity_feature_file: entity represented by integer features and values, the result of feature indexer
	my($dataset_name, $type,$dict_file,$entity_json_file, $entity_feature_file) = @_;
	$dataset_name and $type and $dict_file and $entity_json_file and $entity_feature_file or die "dataset_name, type, dict_file, entity_json_file, entity_feature_file must be provided";
	-f $entity_json_file or die "entity file - $entity_json_file does not exist: $!";
	-f $dict_file or die "feature dictionary file - $dict_file does not exist: $!";
	-d dirname($entity_feature_file) or die "directory of $entity_feature_file does not exist: $!";

	# dynamically load the required features and feature handlers given the dataset name
	my $required_feature_map = eval "Dataset::$dataset_name" . "::FeatureHandler::get_required_feature()";
	my $feature_handler_map = eval "Dataset::$dataset_name" . "::FeatureHandler::get_feature_handler()";
	# now process the feature
	my $entity_attr_map = load_entity_json($type, $entity_json_file);
	# generate feature
	my $entity_required_features = $required_feature_map->{$type};
	my $entity_feature_map = {};
	while(my($entity_id, $entity_attrs) = each %$entity_attr_map){
		my $entity_features = generate_entity_feature($type, $entity_id, $entity_attrs, $entity_required_features, $feature_handler_map);
		$entity_feature_map->{$entity_id} = $entity_features;
	}
	# read in existing features
	open DICT_FILE , "<", $dict_file or die "failed to open dictionary file for reading: $!";
	my %feat_map = ();
	my $max_feat_id = 0;
	while(<DICT_FILE>){
		chomp;
		my($feat_name,$feat_id) = split /\,/;
		if($feat_id > $max_feat_id){
			$max_feat_id = $feat_id;
		}
		$feat_map{$feat_name} = $feat_id;
	}
	close DICT_FILE;
	# finish reading and append dictionary file with new features
	open DICT_FILE, ">>", $dict_file or die "failed to open dictionary file for writing : $!";
	open FEATURE_FILE, ">", $entity_feature_file or die "failed to open feature file fro writing: $!";
	while(my($entity_id,$entity_features) = each %$entity_feature_map){
		# get feature integers
		my $indexed_entity_features = index_entity_feature(\%feat_map,\$max_feat_id,$entity_features,*DICT_FILE);
		# write to entity feature file
		print FEATURE_FILE join(",",($entity_id, map {join(":",($_,$indexed_entity_features->{$_}))} keys %$indexed_entity_features)) . "\n";
    }
    close FEATURE_FILE;
    close DICT_FILE;
}

sub process_dataset {
	# $ini_file: dataset configuration file which defines all entity files and feature handlers
	my($ini_file) = @_;
	$ini_file and -f $ini_file or die "configuration file must be provided and must be present in file system:$!";
	my $yaml = YAML::Tiny->read($ini_file);
	my $cfg = $yaml->[0];
	my $dataset_name = $cfg->{"dataset"};
	my $input_folder = $cfg->{"input.folder"};
	my $result_folder = $cfg->{"result.folder"};
	my $feat_dict_file = $cfg->{"feature.dict.file"};
	$dataset_name and $feat_dict_file and $input_folder and $result_folder or die "dataset_name, feat_dict_file, input_folder, result_folder must be specified in the configuration file:$!";
	-d $input_folder or die "input data folder - $input_folder does not exist: $!";
	-d $result_folder or die "result data folder - $result_folder does not exist: $!";

	$feat_dict_file = $result_folder . "/" . $feat_dict_file;
	my $entities = $cfg->{"entities"};
	foreach my $entity (@$entities){
		# index each type of entity
		my $type = $entity->{"type"};
		my $json_file = $entity->{"json.file"};
		my $feature_file = $entity->{"feature.file"};
		$type and $json_file and $feature_file or die "type, json_file, feature_file must be specified for each entity:$!";
		$json_file = $input_folder . "/" . $json_file;
		$feature_file = $result_folder . "/" . $feature_file;
		process_entity_file($dataset_name,$type,$feat_dict_file,$json_file,$feature_file);
	}
}

1;
