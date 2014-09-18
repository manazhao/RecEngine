package MLTask::Shared::FeatureOperator;

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
use MLTask::Shared::Utility qw(check_func_args);

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new {
	my($class,%args) = @_;
	my @member_names = qw(feature_file feature_dict_file);
	my %class_members = ();
	my $self = \%class_members;
	@class_members{@member_names} = (undef) x scalar @member_names;
	@class_members{@member_names} = @args{@member_names};
	my $err_msg = check_func_args("new $class", $self);
	$err_msg and die $err_msg;
	# now load the features
	$self->{entity_feature} = _load_feature($self->{feature_file});
	# load the dictionary
	print ">>> load feature dictionary\n";
	$self->{feature_dict} = MLTask::Shared::FeatureDict::get_instance( file => $self->{feature_dict_file} );	
	# feature prefix => feature id mapping
	$self->{prefix_fid_map} = {};
	$self->{prefix_group_map} = {};
	bless $self, $class;
	return $self;
}


sub group_by_feature{
	my($self,$feature_prefix) = @_;
	# check whether the grouping already exist
	exists $self->{prefix_group_map}->{$feature_prefix} && return $self->{prefix_group_map}->{$feature_prefix};
	# 
	if(not exists $self->{prefix_fid_map}->{$feature_prefix}){
		# build the prefix to id map
		$self->{prefix_fid_map}->{$feature_prefix} = $self->_build_fprefix_fid_map($feature_prefix);
	}
	my $tmp_prefix_map = $self->{prefix_fid_map}->{$feature_prefix};
	# go through the entity feature map
	$self->{prefix_group_map}->{$feature_prefix} = {};
	while(my($entity_id,$entity_features) = each %{$self->{entity_feature}}){
		foreach my $feat(@$entity_features){
			my ($fid, $fval) = @$feat;
			if($tmp_prefix_map->{$fid}){
				push @{$self->{prefix_group_map}->{$feature_prefix}->{$fid}}, $entity_id;
			}
		}	
	}
	return $self->{prefix_group_map}->{$feature_prefix};
}


sub write_entity_feature{
	my($self,$file) = @_;
	open FILE, ">", $file or die $!;
	print ">>> write modified entity features to - $file\n";
	while(my($entity_id,$feats) = each %{$self->{entity_feature}}){
		print FILE join(",", ($entity_id, map { join(":",@$_) } @$tmp_feats)) . "\n";
	}	
	print ">>> writing done\n";
	close FILE;
}

sub write_group_entity_feature{
	my($self,%args) = @_;
	my @arg_names = qw(feature_prefix result_folder);
	my %default_args = ();
	@default_args{@arg_names} = (undef) x scalar @arg_names;
	@default_args{@arg_names} = @args{@arg_names};
	my $err_msg = check_func_args("dump_feature_group",\%default_args);
	$err_msg and die $err_msg;
	# extract hash values
	my($prefix,$result_folder) = @default_args{@arg_names};
	-d $result_folder or die "folder for feature group result - $result_folder does not exist";
	exists $self->{prefix_group_map}->{$prefix} or die "group feature - $prefix is not available";
	$result_folder = $result_folder . "/group_$prefix";
	-d $result_folder or `mkdir -p $result_folder`;
	while(my($group, $ids) = each %{$self->{prefix_group_map}->{$prefix}}){
		my $group_file = $result_folder . "/$group" . "_feat.csv";
		print ">>> writing group: $group to $group_file\n";
		open FILE, ">", $group_file or die $!;
		foreach my $entity_id (@$ids){
			my $tmp_feats = $self->{entity_feature}->{$entity_id};
			# join feature id and value by ":" and write after entity id
			print FILE join(",", ($entity_id, map { join(":",@$_) } @$tmp_feats)) . "\n";
		}
		close FILE;
	}
}


sub normalize_feature{
	my($self,%args) = @_;
	my @arg_names = qw(feature_list);
	my %default_args = ();
	@default_args{@arg_names} = (undef) x scalar @arg_names;
	@default_args{@arg_names} = values %args;
	my ($feature_list_str) = @default_args{@arg_names};
	my @feature_list = split /\,/, $feature_list_str;
	# feature name to id map
	my $fname_id_map = $self->{feature_dict}->get_id_map();
	# find all feature ids that match to the provided names
	my %norm_fid_map = ();
	while(my($name,$id) = each %$fname_id_map){
		my @matched_names = grep { $fname =~ m/$_/ } @feature_list;
	}
		
	

	
}

sub _build_fprefix_fid_map{
	my($self,$prefix) = @_;
	my $fid_fname_map = $self->{feature_dict}->get_name_map();
	my $pl = length $prefix;
	my %prefix_id_map = ();
	while(my($id,$name) = each %$fid_fname_map){
		my $sub_name = substr $name, 0, $pl;
		$sub_name eq $prefix and $prefix_id_map{$id} = 1;
	}
	#print Dumper(\%prefix_id_map);
	return \%prefix_id_map;
}


# entity indexed features given an entity
sub _load_feature{
	my($feature_file) = @_;
	print ">>> load features from file - $feature_file\n";
	open FEATURE_FILE, "<" , $feature_file or die $!;
	my %entity_feat_map = ();
	while(<FEATURE_FILE>){
		chomp;
		my ($type_entity_id, @feats) = split /\,/;
		$entity_feat_map{$type_entity_id} = [map { [split /\:/] } @feats]
	}
	close FEATURE_FILE;
	print ">>> loading done\n";
	return \%entity_feat_map;
}

1;
