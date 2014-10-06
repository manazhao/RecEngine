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
	my($self,%args) = @_;
	$self->{driver} = $args{driver};
	$self->{driver} or die "Driver reference missing:$!";
	my $driver = $self->{driver};
	# task config
	my $task_cfg = $driver->get_task_config();
	# entity config
	my $user_entity_config = $driver->get_entity_config("user");
	my $item_entity_config = $driver->get_entity_config("item");

	# get item feature indexer
	my $item_feature_indexer = $driver->get_feature_indexer("item");
	my $item_feature_map = $item_feature_indexer->get_feature_map();
	# group entities by leaf category
	my $feature_op = new MLTask::Shared::FeatureOperator(entity_feature => $item_feature_map, feature_dict => $driver->get_feature_dict());
	# group feature by leaf category
	my $leaf_category_prefix = "i_c-1";
	$feature_op->group_by_feature($leaf_category_prefix);
	# which category to work on
	my $category = $task_cfg->{'model_config'}->{'category'};
	# get the ratings of the category
	my $category_items = $feature_op->get_group_entities($leaf_category_prefix,$category);
	print ">>> load user rating\n";
	my $user_rating_map = $driver->get_user_rating();
	print ">>> filter ratings by item id\n";
	my $category_rating_map = $driver->filter_rating_by_items($user_rating_map,$category_items);
	my $num_rating = scalar keys %$category_rating_map;
	print "[info] # of rating after filtering: $num_rating\n";
	# now generate rating feature
}

1;
