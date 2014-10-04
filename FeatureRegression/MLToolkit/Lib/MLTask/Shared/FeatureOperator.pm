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
	if($self->{feature_file}){
		$self->{entity_feature} = _load_feature($self->{feature_file});
	}
	# load the dictionary
	print ">>> load feature dictionary\n";
	$self->{feature_dict} = MLTask::Shared::FeatureDict::get_instance( file => $self->{feature_dict_file} );	
	# feature prefix => feature id mapping
	$self->{prefix_fid_map} = {};
	$self->{prefix_group_map} = {};
	bless $self, $class;
	return $self;
}


# get entity feature map
sub get_feature_map{
	my $self = shift;
	return $self->{entity_feature};
}


sub register_named_pattern{
	my($self,%args) = @_;
	my @required_args = qw(name pattern);
	my %default_args = ();
	@default_args{@required_args} = (undef) x scalar @required_args;
	@default_args{keys %args} = values %args;
	check_func_args("MLTask::Shared::FeatureOperator::register_named_pattern",\%default_args);
	# use quotemeta to escape the special characters properly
	my $name = $default_args{name};
	my $pattern = $default_args{pattern};
	$self->{named_pattern_map}->{$name} = $pattern;
	# find feature ids of the named pattern
	my $id_name_map = $self->{feature_dict}->get_name_map();
	while(my($fid,$fname) = each %$id_name_map){
		if($fname =~ m/$pattern/){
			$self->{named_pattern_fid_map}->{$name}->{$fid} = 1;		
			$self->{fid_pattern_map}->{$fid}->{$name} = 1;
		}
	}
}

sub check_interact_pattern{
	my($self,$rule_list) = @_;
	my $pattern_fid_map = $self->{named_pattern_fid_map};
	foreach my $one_rule(@$rule_list){
		my @names = @$one_rule;
		foreach(@names){
			exists $pattern_fid_map->{$_} or die "unregistered named pattern: $_";
		}
	}
}

sub _interact_feature_helper{
	my($self, $rule_feats,$idx_arr, $cur_idx,$result_feats) = @_;
	my $num_pats = scalar @$idx_arr;
	if($cur_idx == $num_pats){
		# final rule pattern
		# do the multiplication
		my @fids = ();
		my $fval = 1;
		for(my $i = 0; $i < $num_pats; $i++){
			my $tmp_idx = $idx_arr->[$i];
			my $tmp_fid = $rule_feats->[$i]->[$tmp_idx]->[0];
			my $tmp_fval = $rule_feats->[$i]->[$tmp_idx]->[1];
			push @fids, $tmp_fid;
			$fval *= $tmp_fval;
		}
		
		my $feature_name = join("_",("int",@fids));
		my $int_fid = $self->{feature_dict}->index_feature($feature_name);
		# save to result array
		push @{$result_feats}, [$int_fid,$fval];
	}else{
		my @cur_rule_feats = @{$rule_feats->[$cur_idx]};
		for(my $i = 0; $i <= $#cur_rule_feats; $i++){
			my @updated_idx_arr = @$idx_arr;
			$updated_idx_arr[$cur_idx] = $i;
			$self->_interact_feature_helper($rule_feats,\@updated_idx_arr,$cur_idx+1,$result_feats);
		}
	}
}

# generate interacted features
sub interact_feature{
	my ($self,%args) = @_;
	my @required_args = qw(feats rule_list);
	my %default_args = ();
	@default_args{@required_args} = (undef) x scalar @required_args;
	@default_args{@required_args} = @args{@required_args};
	check_func_args("FeatureOperator::interact_feature",\%default_args);
	# each rule of rule_list is a list of named patterns which are registered through
	# register_named_pattern
	my $pattern_fid_map = $self->{named_pattern_fid_map};
	my %pattern_fval_map = ();
	my @feats = @{$default_args{feats}};
	my @rules = @{$default_args{rule_list}};
	foreach my $feat(@feats){
		# each feat is tuple of feature id and value
		my($fid,$fval) = @$feat;
		my @pattern_names = keys %{$self->{fid_pattern_map}->{$fid}};
		foreach my $pattern_name (@pattern_names){
			push @{$pattern_fval_map{$pattern_name}}, $feat;
		}	
	}
	# resulted interaction features
	my @result_int_feats = ();
	foreach my $rule(@rules){
		# pattern names of the given rule
		my @rule_patterns = @$rule;
		my @rule_feats = @pattern_fval_map{@rule_patterns}; 
		foreach (@rule_feats){
			@$_ or return;
		}
		my @idx_arr = (0) x scalar @rule_patterns;
		my $cur_idx = 0;
		$self->_interact_feature_helper(\@rule_feats,\@idx_arr,$cur_idx,\@result_int_feats);
	}
}


# group entities by feature prefix
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
	while(my($entity_id,$tmp_feats) = each %{$self->{entity_feature}}){
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
	-d $result_folder or `mkdir -p $result_folder`;
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
	while(my($fname,$id) = each %$fname_id_map){
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
