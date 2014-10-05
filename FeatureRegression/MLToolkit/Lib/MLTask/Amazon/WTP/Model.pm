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
	$self->{driver} = $driver;
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
	# load item feature indexer
	print ">>> load item feature\n";
	my $item_feat_indexer = $driver->load_feature_indexer("i",$category_item_feat_file);
	my $item_feat_map = $item_feat_indexer->get_feature_map();
	my $filtered_user_rating_file = $group_result_folder. "/group_i_c-1/$category" . "_rating.csv";
	my $category_user_rating_map;
        # generate category rating file if not exists yet
	if(not -f $filtered_user_rating_file){
		print ">>> generating category rating file\n";
		my @category_items = keys %$item_feat_map;
		# load user item ratings
		print ">>> retrieve user rating map\n";
		$category_user_rating_map = $driver->get_user_rating();
		# filter ratings
		print ">>> filter ratings by category items\n";
		$category_user_rating_map = $driver->filter_rating_by_items($category_user_rating_map,\@category_items);
		$driver->write_rating_to_file($filtered_user_rating_file,$category_user_rating_map);
	}else{
		$category_user_rating_map = $driver->load_rating_from_file($category_user_rating_map);
	}

	print ">>> load user feature\n";
	my $user_feat_indexer = $driver->get_feature_indexer("user");
	my $user_feat_map = $user_feat_indexer->get_feature_map();
}


1;
