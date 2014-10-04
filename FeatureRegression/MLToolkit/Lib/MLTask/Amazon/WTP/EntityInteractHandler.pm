#!/usr/bin/perl 
## # feature handler for price regression task # 
package MLTask::Amazon::WTP::Model; 
use strict; 
use warnings;
use Exporter; 
use FindBin; 
use Data::Dumper; 
use lib "$FindBin::Bin/../../"; 
use MLTask::Shared::FeatureOperator;
use vars qw($VERSION @ISA @EXPORT);
our $VERSION = 1.0; 
our @ISA = qw (Exporter); 
our @EXPORT = (); # now train the model

sub new{
	return bless {};
}

sub _load_category_tree{
	my($self,$tree_file) = @_;
	open CTREE_FILE,"<",$tree_file or die $!;
	my $category_tree = {};
	while(<CTREE_FILE>){
		chomp;
		my($cid,$cname,$is_leaf,$path_str) = split /\t/;
		$path_str = substr($path_str,1);
		my @parent_nodes = split /\//, $path_str;
		$category_tree->{$cid} = {name => $cname, is_leaf => $is_leaf,parent_nodes=>\@parent_nodes};
	}
	$self->{category_tree} = $category_tree;
}



sub train{
	my($self,$driver) = @_;
	# task config
	my $yml_cfg = $driver->get_yml_config();
	my $task_cfg = $driver->get_task_config();
	# entity config
	my $user_entity_config = $driver->get_entity_config("user");
	my $item_entity_config = $driver->get_entity_config("item");
	# train model for each category
	my $dict_file = $task_cfg->{feature_dict_file};
	my $result_folder = $task_cfg->{result_folder};
	# user and feature file
	my $item_feat_file = $item_entity_config->{feature_file};
	my $user_feat_file = $user_entity_config->{feature_file};

	# write the grouped result
	my $group_result_folder = $result_folder . "/LeafCategory";
	if(not -d $group_result_folder){
		# feature operator
		my $feature_op = new MLTask::Shared::FeatureOperator(feature_file => $item_feat_file, feature_dict_file=>$dict_file);
		# group feature by leaf category
		$feature_op->group_by_feature("i_c-1");
		$feature_op->write_group_entity_feature(feature_prefix=>"i_c-1",result_folder=>$group_result_folder);
	}else{
		print ">>> group by leaf category is already done, skip\n";
	}

	# which category to work on
	my $category = $yml_cfg->{'model.config'}->{'category'};
	# load user and item features
	my $category_item_feat_file = $group_result_folder. "/group_i_c-1/$category" . "_feat.csv";
	print ">>> load user and item feature\n";

	my $user_feat_indexer = $driver->get_feature_indexer("user");
	my $item_feat_indexer = $driver->get_feature_indexer("item");

	my $item_feat_map = $item_feat_indexer->get_feature_map();
	my $user_feat_map = $user_feat_indexer->get_feature_map();

	my @category_items = keys %$item_feat_map;
	# load user item ratings
	print ">>> load user ratings\n";
	my $user_rating_map = $driver->load_rating($task_cfg->{rating_file});
	# filter ratings
	print ">>> filter ratings by category items\n";
	$driver->filter_rating_by_items($user_rating_map,\@category_items);
	# now generate aggregated feature
	# test interaction feature
	print ">>> generate interaction features\n";
	my $feature_op = new MLTask::Shared::FeatureOperator(feature_file => "", feature_dict_file=>$dict_file);
	print ">>> register named rule patterns\n";
	$feature_op->register_named_pattern(name => "USER", pattern => "u_");
	$feature_op->register_named_pattern(name => "ITEM", pattern => 'i_((br)|(c))');

	my @feat_int_rules = (["USER","ITEM"]);
	print ">>> check rule pattern names\n";
	$feature_op->check_interact_pattern(\@feat_int_rules);

	while(my($user_id,$tmp_ratings) = each %$user_rating_map){
		my ($item_id,$rate,$time)  = @{$tmp_ratings->[0]};
		if(not defined $user_feat_indexer->get_feature($user_id)){
			warn "new user entity - $user_id added to feature file";
			my $entity_json = {id => $user_id};
			$user_feat_indexer->index_entity($entity_json);
		}
		my $user_feats = $user_feat_indexer->get_feature($user_id);
		my $item_feats = $item_feat_indexer->get_feature($item_id);
		my @all_feats = (@$user_feats,@$item_feats);
		$feature_op->interact_feature(feats => \@all_feats, rule_list => \@feat_int_rules);
		last;
	}
}



1;
