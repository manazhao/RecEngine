#!/usr/bin/perl 
## # feature handler for price regression task # 
package MLTask::Amazon::WTP::Model; 
use strict; 
use warnings;
use Exporter; 
use FindBin; 
use Data::Dumper; 
use List::Util qw(shuffle);
use File::Basename;
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

sub _split_train_test_index{
	my $self = shift;
	-f $self->{train_index_file} and -f $self->{test_index_file} and print "[info] train index file already exists, skip\n" and return;
	open TRAIN_IDX_FILE, ">", $self->{train_index_file} or die $!;
	open TEST_IDX_FILE, ">", $self->{test_index_file} or die $!;
	# split randomly by users
	my @rating_users = keys %{$self->{user_rating_map}};
	my @shuffled_users = shuffle @rating_users;
	# 80% used for training and the rest for testing
	my $num_users = scalar @rating_users;
	my $train_idx = int($num_users * 0.8);
	my @train_users = @shuffled_users[0 .. $train_idx];
	my @test_users = @shuffled_users[$train_idx + 1 .. $num_users - 1];
	# write to file
	print ">>> write train and test users to file\n";
	print TRAIN_IDX_FILE join("\n",@train_users);
	close TRAIN_IDX_FILE;
	print TEST_IDX_FILE join("\n",@test_users);
	close TEST_IDX_FILE;
}

sub _load_train_test_index{
	my $self = shift;
	open TRAIN_IDX_FILE, "<", $self->{train_index_file} or die $!;
	open TEST_IDX_FILE, "<", $self->{test_index_file} or die $!;
	print ">>> load train and testing users\n";
	$self->{train_users} = [];
	$self->{test_users} = [];
	while(<TRAIN_IDX_FILE>){
		chomp;
		push @{$self->{train_users}}, $_;
	}
	close TRAIN_IDX_FILE;
	while(<TEST_IDX_FILE>){
		chomp;
		push @{$self->{test_users}}, $_;
	}
	close TRAIN_IDX_FILE;
}

# generate feature given
# user id, user past ratings
sub _generate_rating_feature{
	my ($self, $user_id, $cur_rating, $past_ratings) = @_;
	# not using past information for now
	my $user_feature = $self->{driver}->get_feature_indexer("user")->get_feature($user_id);
	my $cur_item = $cur_rating->[0];
	my $item_feature = $self->{driver}->get_feature_indexer("item")->get_feature($cur_item);
	# generate interaction feature
	my @user_item_features = (@$user_feature,@$item_feature);
	my @int_features = $self->{feature_operator}->interact_feature( feats => \@user_item_features, rule_list => $self->{int_rule_list});
	my @final_features = (@user_item_features,@int_features);
	my $y_label = $cur_rating->[1] ? 1 : 0;
	return ($y_label, @final_features);
}


sub _generate_negative_samples{
	my ($self, $user_id, $cur_rating, $past_ratings) = @_;
	my $num_neg_samples = 10;
	my @items = @{$self->{items}};
	my @shuffled_items = shuffle @items;
	my $i = 0;
	return grep {$_ ne $cur_rating->[0] and ($i++ < $num_neg_samples) } @shuffled_items;
}


sub _train_sample_feature_callback{
	my($self,$user_id, $sample_idx, $user_samples) = @_;
	foreach my $tmp_sample (@$user_samples){
		$self->_write_sample_feature($self->{train_fh},$tmp_sample);
	}
}

sub _test_sample_feature_callback{
	my($self,$user_id, $sample_idx, $user_samples) = @_;
	my $sample_file = $self->{predict_result_dir} . "/$user_id" . "_$sample_idx.libsvm";
	open my $sample_fh, ">", $sample_file or die $!;
	foreach my $tmp_sample (@$user_samples){
		$self->_write_sample_feature($sample_fh,$tmp_sample);
	}
	close $sample_fh;
}

sub _generate_sample_feature{
	# $self: model reference
	# $users: array of user id
	# $callback: callback function for sample features
	my ($self,$users,$callback) = @_;
	my $user_rating_map = $self->{user_rating_map};
	foreach my $user_id(@$users){
		my $tmp_ratings = $user_rating_map->{$user_id};
		for(my $i = 0; $i < @$tmp_ratings; $i++){
			my @user_samples = ();
			my $cur_rating = $tmp_ratings->[$i];
			my @past_ratings = $i > 0 ? @{$tmp_ratings}[0 .. $i - 1] : ();
			# generate positive sample feature
			my @pos_sample = $self->_generate_rating_feature($user_id,$cur_rating,\@past_ratings);
			push @user_samples, \@pos_sample;
			# generate negative samples randomly
			my @neg_items = $self->_generate_negative_samples($user_id,$cur_rating,\@past_ratings);
			# generate ngetaive sample feature
			# get time of current oberved rating
			my $ts = $cur_rating->[2];
			foreach my $neg_item(@neg_items){
				my @tmp_neg_sample = $self->_generate_rating_feature($user_id,[$neg_item,0, $ts],\@past_ratings);
				push @user_samples,\@tmp_neg_sample;
			}
			$callback->($self,$user_id,$i,\@user_samples);
		}
	}
}

sub _write_sample_feature{
	my $self = shift;
	my($fh,$sample) = @_;
	# sort sample features
	my ($y_label, @feats) = @$sample;
	my @sort_feats = sort {$a->[0] <=> $b->[0]} @feats;
	print $fh join(" ",($y_label,map { $_->[0] . ":" . $_->[1] } @sort_feats))."\n";
}

sub init{
	my($self,%args) = @_;
	$self->{driver} = $args{driver};
	$self->{driver} or die "Driver reference missing:$!";
}

sub generate_samples{
	my $self = shift;
	my $driver = $self->{driver};
	# task config
	my $task_cfg = $driver->get_task_config();
	$self->{train_file} = $driver->get_full_path($task_cfg->{model_config}->{train_file});
	my $done_file = dirname($self->{train_file}) . "/.done";
	-f $done_file and print "[info] sample feature files exist, skip\n" and return;

	# entity config
	my $user_entity_config = $driver->get_entity_config("user");
	my $item_entity_config = $driver->get_entity_config("item");

	# get item feature indexer
	my $item_feature_indexer = $driver->get_feature_indexer("item");
	my $item_feature_map = $item_feature_indexer->get_feature_map();

	my $category_items;
	{
		# group entities by leaf category
		my $feature_op = new MLTask::Shared::FeatureOperator(entity_feature => $item_feature_map, feature_dict => $driver->get_feature_dict());
		# group feature by leaf category
		my $leaf_category_prefix = "i_c-1";
		$feature_op->group_by_feature($leaf_category_prefix);
		# which category to work on
		my $category = $task_cfg->{'model_config'}->{'category'};
		# get the ratings of the category
		$category_items = $feature_op->get_group_entities($leaf_category_prefix,$category);
	}
	print ">>> load user rating\n";
	my $user_rating_map = $driver->get_user_rating();
	print ">>> filter ratings by item id\n";
	my $category_rating_map = $driver->filter_rating_by_items($user_rating_map,$category_items);
	$self->{user_rating_map} = $category_rating_map;
	$self->{items} = $category_items;
	my $leaf_category_dir = $driver->get_task_config()->{leaf_category_dir};
	my $category = $driver->get_task_config()->{model_config}->{category};
	my $category_rating_file = $driver->get_full_path($leaf_category_dir . "/$category" . "_rating.csv");
	$driver->write_rating_to_file($category_rating_file, $category_rating_map);
	# scale item features
	my $scaled_feature_file = $driver->get_full_path($leaf_category_dir . "/$category" . "/item_feat.csv.scaled");
	print ">>> scale item feature - $scaled_feature_file\n";
	if(not -f $scaled_feature_file) {
		# normalize features
		my $category_item_feature = {};
		map {$category_item_feature->{$_} = $item_feature_map->{$_}} @{$self->{items}};
		my $feature_op = new MLTask::Shared::FeatureOperator(entity_feature => $category_item_feature, feature_dict => $driver->get_feature_dict());
		# normalize price
		$feature_op->normalize_feature("i_pr");
		# write the result
		$feature_op->write_entity_feature($scaled_feature_file);
	}else{
		print "[info] item feature already scaled, skipped\n";
	}

	my $train_index_file = $self->{driver}->get_task_config()->{model_config}->{train_index_file};
	my $test_index_file = $self->{driver}->get_task_config()->{model_config}->{test_index_file};
	$train_index_file = $self->{driver}->get_full_path($train_index_file);
	$test_index_file = $self->{driver}->get_full_path($test_index_file);
	$self->{train_index_file} = $train_index_file;
	$self->{test_index_file} = $test_index_file;
	$self->_split_train_test_index();
	$self->_load_train_test_index();

	# FeatureOperator for generating interaction features
	$self->{feature_operator} = new MLTask::Shared::FeatureOperator(feature_dict => $driver->get_feature_dict());
	$self->{feature_operator}->register_named_pattern(name => "USER", pattern => "u_");
	$self->{feature_operator}->register_named_pattern(name => "ITEM", pattern => "i_((br)|(br_pop)|(pop)|(pr))");
	$self->{int_rule_list} = [["USER","ITEM"]];

	# open training sample feature file 
	$self->{model_file} = $driver->get_full_path($driver->get_task_config()->{model_config}->{model_file});
	# put prediction sample feature files
	$self->{predict_result_dir} = $driver->get_full_path($driver->get_task_config()->{model_config}->{predict_result_dir});
	# now generate training and testing sample features
	print ">>> generate training sample features\n";
	if(not -f $self->{train_file}){
		open $self->{train_fh} , ">", $self->{train_file} or die $!;
		$self->_generate_sample_feature($self->{train_users},\&_train_sample_feature_callback);
	}else{
		print "[info] training sample feature already exists, skip\n";
	}
	-d $self->{predict_result_dir} or (print "[warn] prediction result directory not exist and will be created\n" and `mkdir -p $self->{predict_result_dir}`);
	print ">>> generate test sample features\n";
	# remove test users which are already handled
	my @rest_test_users = grep { not -f $self->{predict_result_dir} . "/" . $_ . "_0.libsvm" } @{$self->{test_users}};
	$self->_generate_sample_feature($self->{test_users},\&_test_sample_feature_callback);
	print ">>> done with sample feature generation!\n";
	# put a .done file under the directory
	`touch $done_file`;
}

sub train{
	my $self = shift;
	my $driver = $self->{driver};
	my $config = $driver->get_task_config();
	my $model_file = $driver->get_full_path($config->{model_config}->{model_file});
	-f $model_file and ( print "[info] model file already exists, skip\n" and return );
	my $train_feature_file = $driver->get_full_path($config->{model_config}->{train_file});
	my $log_file = $driver->get_full_path(dirname($config->{model_config}->{train_file}) . "/train.log");
	my $train_cmd = "train -s 6 -B 1 -v 5  $train_feature_file $model_file 1>$log_file 2>&1 ";
	print ">>> $train_cmd\n";
	`$train_cmd`;

}


sub DESTROY{
	my $self = shift;
	$self->{train_fh} and close $self->{train_fh};
}

1;
