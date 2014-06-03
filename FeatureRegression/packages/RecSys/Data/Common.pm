package RecSys::Data::Common;

use strict;
use warnings;

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
sub generate_entity_feature{
	# $id: entity id
	# $attribute_map: attributes for the entity. key is attribute name and value is attribute value
	# $required_attributes: attributes that are required
	# $type: type of entity. e.g. user, item, etc.. 
	# $hf_ref: hash map which associates function handler to attributes
	my($type, $attribute_map, $required_attributes, $hf_ref_map) = @_;
	my %required_attr_map = ();
	@required_attr_map{@$required_attributes} = (0) x scalar @$required_attributes;
	my %result_feat_map = ();
	while(my($attr_name, $attr_value) = each %$attribute_map){
            if(exists $hf_ref_map->{$type}->{$attr_name}){
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
sub process_entity{
    my($type,$entity_file, $dict_file) = @_;
    $type and $entity_file and $dict_file or die "type, entity_file, dict_file must be provided";
    -f $entity_file or die "entity file - $entity_file does not exist: $!";
    -f $dict_file or die "feature dictionary file - $dict_file does not exist: $!";
    my $dataset_required_features = Dataset::Amazon::FeatureHandler::get_required_feature();
    my $dataset_feature_handlers = Dataset::Amazon::FeatureHandler::get_feature_handler();
    # now process the feature
    my $entity_attr_map = load_entity_json($type, $entity_file);
    # generate feature
    my $user_required_features = $dataset_required_features->{$type};
    my $user_feature_map = {};
    while(my($entity_id, $entity_attrs) = each %$entity_attr_map){
        my $entity_feature_map = generate_entity_feature($type, $entity_attrs, $user_required_features, $dataset_feature_handlers);
        $user_feature_map->{$entity_id} = $entity_feature_map;
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
    while(my($entity_id,$entity_feats) = each %$user_feature_map){
        # get feature integers
        my $indexed_entity_feats = index_entity_feature(\%feat_map,\$max_feat_id,$entity_feats,*DICT_FILE);
    }
    close DICT_FILE;
}



1;
