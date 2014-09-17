package Dataset::Common;

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
use Dataset::FeatureDict;

# use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

our $VERSION = 1.00;
our @ISA = qw(Exporter);
# use base Exporter;
# list of functions export by default, empty here
our @EXPORT = qw(load_entity_json generate_entity_feature index_entity_feature);

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
		$obj_feat_map{$id} = {};
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
	# $feat_dict_obj: Dataset::FeatureDict object
	# $entity_feat_map: entity features from json file
	my($feat_dict_obj, $entity_feat_map) = @_;
	my %result_feat_map = ();
	while(my ($feat_name, $feat_val) = each %$entity_feat_map){
		my $feat_id = $feat_dict_obj->index_feature($feat_name);
		$result_feat_map{$feat_id} = $feat_val;
	}
	return \%result_feat_map;
}

# entity indexed features given an entity
sub load_entity_feature{
	my($feature_file) = @_;
	open FEATURE_FILE, "<" , $feature_file or die $!;
	my %entity_feat_map = ();
	while(<FEATURE_FILE>){
		chomp;
		my ($type_entity_id, @feats) = split /\,/;
		my ($type,$id) = split /\_/, $type_entity_id;
		foreach(@feats){
			push @{$entity_feat_map{$id}}, [split /\:/, $_];
		}
	}
	close FEATURE_FILE;
	return \%entity_feat_map;
}

# classify entity features according to the pattern list
sub classify_feature{
	# $re_list: regular expression list
	# note: each of $re_list is string after quotemeta.
	# e.g. quote meta "^\d+?" (pattern for consecutive numbers)
	# $dict_map: feature dictionary
	# $features: indexed feature list
	my ($re_list, $dict_map, $features) = @_;
	my @re_features = ();
	$re_features[scalar @$re_list - 1] =  undef;
	foreach my $pat_idx(0 .. scalar @$re_list - 1){
		# get the pattern which is quotemeta format
		my $pat = $re_list->[$pat_idx];
		# match each feature against the pattern
		foreach my $tmp_feat (@$features){
			if(not exists $dict_map->{$tmp_feat->[0]}){
				print STDERR $tmp_feat->[0] . "\n";
			}
			if($dict_map->{$tmp_feat->[0]} =~ /$pat/){
				push @{$re_features[$pat_idx]}, $tmp_feat;
			}	
		}
	}
	return \@re_features;
}

sub classify_entity_feature{
	my($re_list,$dict_map, $entity_feat_map) = @_;
	my %result_feat_map = ();
	while(my($entity_id,$feats) = each %$entity_feat_map){
		$result_feat_map{$entity_id} = classify_feature($re_list,$dict_map,$feats);
	}
	return \%result_feat_map;
}

# generate interaction features
sub gen_int_feature{
	my($feat_dict_obj,$feats1,$feats2) = @_;
	my @int_feat_ids = ();
	my @int_feat_vals = ();
	foreach my $feat1 (@$feats1){
		foreach my $feat2 (@$feats2){
			my $feat_name = join("_",("int",join("|",($feat1->[0],$feat2->[0]))));
			my $feat_id = $feat_dict_obj->index_feature($feat_name);
			push @int_feat_ids, $feat_id;
			push @int_feat_vals, $feat1->[1] * $feat2->[1];
		}
	}
	return (\@int_feat_ids, \@int_feat_vals);
}

sub load_feat_dict{
	my($dict_file) = @_;
	open DICT_FILE, "<" , $dict_file or die $!;
	my %dict_map = ();
	while(<DICT_FILE>){
		chomp;
		my($name,$id) = split /\,/;
		$dict_map{$id} = $name;
	}
	close DICT_FILE;
	return \%dict_map;
}


# 
sub gen_entity_feature{
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
	my $required_feature_map = eval "Dataset::$dataset_name" . "::get_required_feature()";
	my $feature_handler_map = eval "Dataset::$dataset_name" . "::get_feature_handler()";

	# now process the feature
	print ">>> load entity json file: $entity_json_file\n";
	my $entity_attr_map = load_entity_json($type, $entity_json_file);
	# generate feature
	my $entity_required_features = $required_feature_map->{$type};
	my $entity_feature_map = {};
	while(my($entity_id, $entity_attrs) = each %$entity_attr_map){
		my $entity_features = generate_entity_feature($type, $entity_id, $entity_attrs, $entity_required_features, $feature_handler_map);
		$entity_feature_map->{$entity_id} = $entity_features;
	}
	print ">>> intialize feature dictionary: $dict_file\n";
	my $FEAT_DICT_OBJ = Dataset::FeatureDict::get_instance(file => $dict_file);
	# finish reading and append dictionary file with new features
	print ">>> generate entity features\n";
	open FEATURE_FILE, ">", $entity_feature_file or die "failed to open feature file fro writing: $!";
	while(my($entity_id,$entity_features) = each %$entity_feature_map){
		# get feature integers
		my $indexed_entity_features = index_entity_feature($FEAT_DICT_OBJ,$entity_features);
		# write to entity feature file
		print FEATURE_FILE join(",",($entity_id, map {join(":",($_,$indexed_entity_features->{$_}))} keys %$indexed_entity_features)) . "\n";
	}
	close FEATURE_FILE;
}

1;
