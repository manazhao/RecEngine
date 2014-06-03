package Data::Common;

use strict;
use warnings;

use Exporter;
use JSON;
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
	# $hf_ref: function reference which will output feature name given entity type and entity attributes
	my($attribute_map, $required_attributes, $type, $hf_ref) = @_;
	my %required_attr_map = ();
	@required_attr_map{@$required_attributes} = (0) x scalar @$required_attributes;
	my %result_feat_map = ();
	while(my($attr_name, $attr_value) = each %$attribute_map){
		my ($names, $values) = $hf_ref->($type, $attr_name, $attr_value);
		$required_attr_map{$attr_name} = 1;
		@result_feat_map{@$names} = @$values;
	}
	# generate missing features if any
	my @miss_attrs = grep { not $required_attr_map{$_} } keys %required_attr_map;
	@result_feat_map{map {join("_",($type,$_))} @miss_attrs} = (1) x scalar @miss_attrs;
	return \%result_feat_map;
}

# convert feature name to integers
sub index_entity_feature{
	my($feat_dict, $entity_feat_map) = @_;
	my $max_feat_id = scalar keys %$feat_dict;
	$max_feat_id++;
	my %result_feat_map = ();
	while(my ($feat_name, $feat_val) = each %$entity_feat_map){
		my $feat_id = (exists $feat_dict->{$feat_name}? $feat_dict->{$feat_name} : $feat_dict->{$feat_name} = $max_feat_id);
		$result_feat_map{$feat_id} = $feat_val;
	}
	return \%result_feat_map;
}

# 
sub process_entity{

}

1;
