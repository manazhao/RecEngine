#!/usr/bin/perl 
#
# define functions for generating features given entity attributes
#
package Dataset::Shopcom;


use FindBin;
use lib "$FindBin::Bin/../";
use strict;
use warnings;
use Exporter;
use Date::Manip;
use Data::Dumper;
# use routines defined in Dataset::Comon module
use Dataset::Common;
use File::Basename;
use Dataset::FeatureDict;
use List::Util qw(shuffle);


our $VERSION = 1.0;
our @ISA = qw (Exporter);
our @EXPORT = ();


my %feature_handler_map = (
    "u" => {
    },
    "i" => {
        "c" => \&item_category_feature_handler,
    }
);

my %required_features = (
	"u" => ["age","gender"],
	# "c": category string
	"i" => [ "c"]
);

sub get_feature_handler{
    return \%feature_handler_map;
}

sub get_required_feature{
    return \%required_features;
}

# define order data loader

sub load_order_data{
	my($order_data_file) = @_;
	open ORDER_FILE, "<" , $order_data_file or die $!;
	my %user_purchase_map = ();
	my $line_cnt = 0;
	while(<ORDER_FILE>){
		chomp;
		my $line = $_;
		my($user_id,$datetime, $item_id, $quantity,$price, $category) = split /\t/;
		# convert to time stamp
		my $ts = UnixDate($datetime,"%s");		
		if(not defined $ts){
			print $line . "\n";
			die $!;
		}
		# process category
		$category =~ s/\s+\&\s+/\&/g;
		# remove comma, avoid confusing with comma used for csv file
		$category =~ s/\,//g;
		my $tmp_event = [$user_id,$item_id,$ts, $category];
		push @{$user_purchase_map{$user_id}}, $tmp_event;
		$line_cnt++;
		#last if $line_cnt == 5000;
	}
	close ORDER_FILE;
	return \%user_purchase_map;
}


# various scaled time windows in seconds unit
# e.g. daily, weekly, bi-weekly, monthly, bi-montyly, seasonly, half-year, yearly, bi-yearly
use constant DAY => 3600 * 24;
use constant WEEK => 7 * DAY;
use constant BI_WEEK =>2 * WEEK;
use constant MONTH => 30 * DAY;
use constant BI_MONTH => 2 * MONTH;
use constant SEASON => 3 * MONTH;
use constant HALF_YEAR => 2 * SEASON;
use constant YEAR => 2 * HALF_YEAR;
use constant BI_YEAR => 2 * YEAR;

our %time_windows = (
	"day" => DAY,
	"week" => WEEK,
	"bi_week" => BI_WEEK,
	"month" => MONTH,
	"bi_month" => BI_MONTH,
	"season" => SEASON,
	"half_year" => HALF_YEAR,
	"year" => YEAR,
	"bi_year" => BI_YEAR
);

# smooth time window voting by extroploate values in adjacent windows
# the weights of the extroplated windows decay exponentially and 
# the decay rate is controlled by a given constant
# e.g. w_i = 2^{-0.5|i|}, namely, the weight halfs by moving one window away from the center
sub _soft_voting{
	# $decay_rate: how fast weight decays away from center window
	# $range: size of adjacent window
	# $window_idx: the given window index
	# $window_weight: weight of the window
	my ($decay_rate, $range, $window_idx, $window_weight) = @_;
	my @adj_windows = (-$range .. $range);
	return [map {[$window_idx + $adj_windows[$_], $window_weight * 2 ** (-$decay_rate * abs($adj_windows[$_]))]} (0..$#adj_windows)];
}

sub _past_category_interact{
	# $past_items: items purchased prior to $cur_item
	# $cur_item: currently purchased item
	# note: each item is hash of the following fields:
	# {"c" => $category, "t" => $timestamp}
	my($feat_dict_obj, $past_items,$cur_item) = @_;
	my $cur_item_c = $cur_item->[0];
	# category interaction map
	# key: category interaction
	# value: frequency
	my %cat_int_map = ();
	# note: category is an integer
	# "ci": category interaction
	my @int_cats = map {join("_",("ci",join("|",($_->[0], $cur_item_c))))} @$past_items;
	foreach(@int_cats){
		$cat_int_map{$_} ++;
	}
	my @int_feature_names = keys %cat_int_map;
	my @int_feature_ids = ();
	foreach(@int_feature_names){
		push @int_feature_ids, $feat_dict_obj->index_feature($_);
	}
	# return the features and the corresponding values(frequency)
	return (\@int_feature_ids, [@cat_int_map{@int_feature_names}]);
}


use constant NEG_SAMPLE_RATIO => 20;

sub get_top_category{
	my($cat_pop_map,$num_cats) = @_;
	# sort by popularity
	my @rank_cats = sort { $cat_pop_map->{$b} <=> $cat_pop_map->{$a} } keys %$cat_pop_map;
	my @top_cats = @rank_cats[0 .. $num_cats];
	my %top_cat_map = ();
	@top_cat_map{@top_cats} = (1) x $num_cats;
	return \%top_cat_map;
}

# sampling the dominating categories as the negative categories
sub _gen_negative_purchases{
	# $cat_pop_map: category popularity map
	# $item_cat: item category
	# $item_id: currently purchased item
	my ($top_cat_map, $item_cat) = @_;
	my @neg_cats;
	if(exists $top_cat_map->{$item_cat}){
		delete $top_cat_map->{$item_cat};
		@neg_cats = keys %$top_cat_map;
		$top_cat_map->{$item_cat} = 1;
	}else{
		@neg_cats = keys %$top_cat_map;
	}
	return @neg_cats;
}

# generate negative samples at the category level
sub _gen_negative_purchases_random{
	# $cat_pop_map: category popularity map
	# $item_cat: item category
	# $item_id: currently purchased item
	my($cat_pop_map,$item_cat) = @_;
	my $item_cat_pop = $cat_pop_map->{$item_cat};
	delete $cat_pop_map->{$item_cat};
	my @all_cats = keys %$cat_pop_map;
	# set the value back
	$cat_pop_map->{$item_cat} = $item_cat_pop;
	my @all_cats_rnd = shuffle @all_cats;
	return \@all_cats_rnd[0 .. NEG_SAMPLE_RATIO - 1];
}



# interact current item with past item by considering time window voting
sub _past_category_time_window_interact{
	# $past_items: items purchased prior to $cur_item
	# $cur_item: currently purchased item
	# note: each item is hash of the following fields:
	# {"c" => $category, "t" => $timestamp}
	my($feat_dict_obj, $past_items,$cur_item) = @_;
	my ($cur_item_c,$cur_item_t) = @$cur_item;
	# voting on the time windows of different scales
	my %window_voting_map = ();
	foreach(@$past_items){
		# temporary past item
		my ($tmp_past_c, $tmp_past_t) = @$_;
		# vote on time windows at diffent scales
		# calculate the time difference
		my $time_diff = $cur_item_t - $tmp_past_t;
		while(my($name,$size) = each %time_windows){
			# $name: window name, e.g. day, week, month, etc
			# $size: window size in seconds
			my $window_idx = int($time_diff / $size);
			# smooth the voting by extroploate in adjacent windows
			# _soft_voting($decay_factor, $range, $window_idx, $window_weight)
			# in this case, we set,
			# $decay_factor = 0.5, $range = 3, $window_idx = $window_idx, $window_weight = 1
			my $extended_voting = _soft_voting(1,3,$window_idx,1);
			# $extended_voting is array of the voting on the ajdacented windows
			# $extended_voting[0] = [$idx,$weight] 
			my $int_cat = join("|",($tmp_past_c,$cur_item_c));
			foreach(@$extended_voting){
				my ($tmp_window_idx,$tmp_weight) = @$_;
				# gengerate the feature
				# "ciw": category interaction with time window
				my $tmp_window_feature = join("_",("ciw",$name,$int_cat,$tmp_window_idx));
				# increase the (soft) voting
				$window_voting_map{$tmp_window_feature} += $tmp_weight;
			}
		}
	}
	my @int_feature_names = keys %window_voting_map;
	my @int_feature_ids = ();
	foreach(@int_feature_names){
		push @int_feature_ids, $feat_dict_obj->index_feature($_);
	}
	return (\@int_feature_ids, [@window_voting_map{@int_feature_names}]);
}

sub gen_item_popularity{
	# $item_cfeat_map: item category feature map (l2)
	# $user_order_map: user order data
	my ($item_cfeat_map, $user_order_map) = @_;
	# item popularity under the category
	my %cat_pop_map = ();
	my %item_pop_map = ();
	while(my($user_id,$orders) = each %$user_order_map){
		foreach(@$orders){
			my($uid,$iid,$ts,$cat) = @$_;
			my $item_cat = $item_cfeat_map->{$iid}->[0]->[0]->[0];
			$cat_pop_map{$item_cat} ++;
			$item_pop_map{$iid}++;
		}
	}	
	# normalize the item category popularity
	while(my($iid,$pop) = each %item_pop_map){
		my $cat_pop = $cat_pop_map{$item_cfeat_map->{$iid}->[0]->[0]->[0]};
		$item_pop_map{$iid} /= $cat_pop;
	}	
	# normalize category popularity
	my $max_cat_pop = 0;
	foreach(values %cat_pop_map){
		$max_cat_pop = $_ if $_ > $max_cat_pop;
	}

	while(my($cid,$pop) = each %cat_pop_map){
		$cat_pop_map{$cid} /= $max_cat_pop;
	}

	return (\%cat_pop_map,\%item_pop_map);
}

sub merge_features{
	my ($existing_features, $more_features) = @_;
	$existing_features->[0] = [@{$existing_features->[0]}, @{$more_features->[0]}];
	$existing_features->[1] = [@{$existing_features->[1]}, @{$more_features->[1]}];
}

# generate features given a purchase event
sub gen_purchase_feature{
	# $user_feat_map: user feature map
	# $item_feat_map: item feature map
	# $item_cfeat_map: item category feature map. 
	# $item_pop_map: item popularity 
	# $cat_pop_map: category popularity
	# $prev_purchases: previous purchase events
	# $cur_purchase: current purchase
	# note: 1) each purchase event is tuple <uid,iid,timestamp>
	#	2) assume the events are sorted by time	
	my($feat_dict_obj, $user_feat_map, $item_feat_map,$item_cfeat_map,$cat_pop_map, $prev_purchases, $cur_purchase) = @_;
	my @prev_purchases_feat_arr = ();
	foreach(@$prev_purchases){
		my($user_id,$item_id,$t) = @$_;
		push @prev_purchases_feat_arr, [$item_cfeat_map->{$item_id}->[0]->[0]->[0], $t];
	}
	my $cur_purchase_feat = [$item_cfeat_map->{$cur_purchase->[1]}->[0]->[0]->[0],$cur_purchase->[2]];

	# get category popularity
	my $cat_pop_fname = "i_cp";
	my $cat_pop_fid = $feat_dict_obj->index_feature($cat_pop_fname);
	my $cur_cat_id = $cur_purchase_feat->[0];
	my $cur_cat_pop = $cat_pop_map->{$cur_cat_id};
	my @cur_cat_feature = ([$cat_pop_fid], [$cur_cat_pop]);	

	# interaction features with previous purchased items
	my @past_int_features = _past_category_interact($feat_dict_obj,\@prev_purchases_feat_arr, $cur_purchase_feat);
	my @past_time_int_features = _past_category_time_window_interact($feat_dict_obj,\@prev_purchases_feat_arr, $cur_purchase_feat);
	
	# merge all features
	my @all_features = @cur_cat_feature;
 	merge_features(\@all_features, \@past_int_features);
	merge_features(\@all_features, \@past_time_int_features);
	return @all_features;
}


# get the class label for each product category
# the class label is integer
sub _get_category_class_label{
	# only the level-2 category considered e.g. Baby|Feeding
	my($item_cfeat_map) = @_;
	my %cat_label_map = ();
	my $label = 1;
	while(my($item_id,$category) = each %$item_cfeat_map){
		my $cid = $category->[0]->[0]->[0];
		if(not exists $cat_label_map{$cid}){
			$cat_label_map{$cid} = $label; 
			$label++;
		}
	}
	return \%cat_label_map;
}

# generate positive samples
#
sub _gen_pos_sample_per_user{
	my($feat_dict_obj, $user_feat_map, $item_feat_map,$item_cfeat_map,$cat_pop_map, $user_purchases) = @_;
	# positive samples
	my @pos_samples = ();
	foreach my $i(0 .. scalar @$user_purchases - 1){
		my $cur_purchase = $user_purchases->[$i];
		my @past_purchases = ($i == 0) ? () : @$user_purchases[0 .. $i - 1 ];
		my @cur_purchase_features = gen_purchase_feature($feat_dict_obj,$user_feat_map, $item_feat_map,$item_cfeat_map,$cat_pop_map, \@past_purchases, $cur_purchase);
		push @pos_samples, [1, 1, @cur_purchase_features];
	}
	# structure of each sample in @samples 
	# [<weight>, <class_label>, [<feature_ids>], [<feature_values>] ]
	return \@pos_samples;
}

sub _gen_neg_sample_per_user{
	my($feat_dict_obj, $user_feat_map, $item_feat_map, $item_cfeat_map, $cat_pop_map, $top_cat_map, $user_purchases) = @_;
	# positive samples
	my @neg_samples = ();
	foreach my $i(0 .. scalar @$user_purchases - 1){
		my $cur_purchase = $user_purchases->[$i];
		# generate negative samples given $cur_purchase
		# print Dumper($item_cfeat_map->{$cur_purchase->[1]});
		my $cur_purchase_cat = $item_cfeat_map->{$cur_purchase->[1]}->[0]->[0]->[0];
		my @cur_neg_purchase_items = _gen_negative_purchases($top_cat_map,$cur_purchase_cat);
		my @past_purchases = ($i == 0) ? () : @$user_purchases[0 .. $i - 1 ];
		foreach my $cur_neg_item(@cur_neg_purchase_items){
			my @cur_neg_purchase = @$cur_purchase;
			# replace the item id
			$cur_neg_purchase[1] = $cur_neg_item;
			my @cur_neg_features = gen_purchase_feature($feat_dict_obj,$user_feat_map, $item_feat_map,$item_cfeat_map, $cat_pop_map, \@past_purchases, \@cur_neg_purchase);
			# generate the weight
			push @neg_samples, [1/NEG_SAMPLE_RATIO, -1,  @cur_neg_features];
		}
	}
	# structure of each sample in @samples 
	# [<weight>, <class_label>, [<feature_ids>], [<feature_values>] ]
	return \@neg_samples
}

sub _write_sample_feature{
	my($file_id,$samples) = @_;
	foreach my $sample(@$samples){
		my($weight, $label,$feat_ids,$feat_vals) = @$sample;
		# order $feat_ids in increasing order
		my @sort_idx = sort {$feat_ids->[$a] <=> $feat_ids->[$b]} 0 .. scalar @$feat_ids - 1;
		my @sort_ids = @$feat_ids[@sort_idx];
		my @sort_vals = @$feat_vals[@sort_idx];
		my $feat_str = join(" ", (map { $sort_ids[$_] . ":" . $sort_vals[$_]} (0..$#sort_ids)));
		print $file_id join(" ", ($weight, $label, $feat_str)) . "\n";
	}
}

sub _get_cat_item_map{
	my($item_cfeat_map) = @_;
	my %cat_item_map = ();
	while(my($item_id,$feats) = each %$item_cfeat_map){
		# $feats is array of the following structure:
		# [ #all features 
		#   [ # category feature(s)
		#     [ # feature id and value
		#      <category_feature_id>,
		#      <feature_value>
		#     ]
		#   ]
		# ]      
		# $feats->[0]: category features
		# $feats->[0]->[0]: first category feature
		# $feats->[0]->[0]->[0]: category feature id
		# $feats->[0]->[0]->[1]: category feature value
		my $tmp_cat = $feats->[0]->[0]->[0];
		push @{$cat_item_map{$tmp_cat}}, $tmp_cat;
	}
	return \%cat_item_map;
}

# genrate training and testing datasets
# purchase event index for each user
use constant TRAIN_RATIO => 0.90;
sub _gen_train_test_orderdata{
	# $train_file: output training order data
	# $test_file: output testing order data
	# $user_purchase_map: user purchases
	my($train_file, $test_file, $user_purchase_map) = @_;
	open TRAIN_FILE, ">", $train_file or die $!;
	open TEST_FILE, ">", $test_file or die $!;
	while(my($user_id, $user_purchases) = each %$user_purchase_map){
		# number of purchases for $user_id
		my $num_purchases = scalar @$user_purchases;
		my $num_train = int (TRAIN_RATIO * $num_purchases);
		print TRAIN_FILE join(",",($user_id, 0, $num_train - 1)) . "\n";
		print TEST_FILE join(",",($user_id,$num_train, $num_purchases - 1)) . "\n";
	}
	close TRAIN_FILE;
	close TEST_FILE;
}

# load training/testing order data
sub _load_train_or_test_idx{
	my ($file) = @_;
	my %user_idx_map = ();
	open FILE, "<", $file or die $!;
	while(<FILE>){
		chomp;
		my($user_id, $begin_idx,$end_idx) = split /\,/;
		# inclusive index
		$user_idx_map{$user_id} = [$begin_idx,$end_idx];
	}
	close FILE;
	return \%user_idx_map;
}

my $MODULE_REF = bless {};

sub get_module_ref{
	return $MODULE_REF;
}

sub train{
	# $dict_file: feature dictionary file
	# $user_feat_file: user feature file
	# $item_feat_file: item feature file
	# $order_data_file: file containing purchase information for each user
	# $sample_feat_file: output file which prepares what's needed by libsvm
	my $self = shift;
	my($dict_file, $user_feat_file, $item_feat_file, $order_data_file,$sample_feat_file) = @_;
	-d dirname($sample_feat_file) or die "invalid directory for feature output file:$sample_feat_file";
	# load feature dictionary
	print ">>> load feature dictionary from $dict_file \n";
	my $FEAT_DICT_OBJ = Dataset::FeatureDict::get_instance(file => $dict_file);
	# load user features 
	print ">>> load user feature from: $user_feat_file \n";
	my $user_feat_map = Dataset::Common::load_entity_feature($user_feat_file);
	# load item features
	print ">>> load item feature from: $item_feat_file \n";
	my $item_feat_map = Dataset::Common::load_entity_feature($item_feat_file);
	# load user order information
	print ">>> load order data from: $order_data_file \n";
	my $user_order_map = load_order_data($order_data_file);
	# only extract the leaf category
	my $re_list = [quotemeta "i_2c"];
	# get the classified features for item
	print ">>> prepare item category feature \n";
	my $item_cfeat_map = Dataset::Common::classify_entity_feature($re_list,$FEAT_DICT_OBJ->get_name_map(),$item_feat_map);
	# category -> class label
	my $cat_label_map = _get_category_class_label($item_cfeat_map);
	# write the categoyr -> class label mapping
	my $cat_label_file = $order_data_file . ".label.csv";
	open TMP_FILE, ">", $cat_label_file or die $!;
	while(my($cat_id,$label) = each %$cat_label_map){
		print TMP_FILE join(",",($cat_id,$label)) . "\n";
	}
	close TMP_FILE;
	# get category -> items map
	print ">>> prepare category item map\n";
	my $cat_item_map = _get_cat_item_map($item_cfeat_map);
	# generate categorpy popularity map
	my ($cat_pop_map, $item_pop_map) = gen_item_popularity($item_cfeat_map, $user_order_map);
	my $top_cat_map  = get_top_category($cat_pop_map, NEG_SAMPLE_RATIO);
	# generate training samples if not yet
	my $train_file = $order_data_file . ".train";
	my $test_file = $order_data_file . ".test";
	_gen_train_test_orderdata($train_file, $test_file, $user_order_map) unless -f $train_file and $test_file;
	# load train order data (index)
	print ">>> load training order data index\n";
	my $user_train_orderdata = _load_train_or_test_idx($train_file);	
	#print Dumper($item_cfeat_map);
	# now generate purchase information for each user
	print ">>> generate training samples: $sample_feat_file\n";
	open my $result_file_id, ">", $sample_feat_file or die $!;
	my $user_cnt = 0;
	while(my($user_id, $user_purchases) = each %$user_order_map){
		# generate positive samples
		# get index range for training order data
		# print ">>> user: $user_id\n";
		my ($bi,$ei) = @{$user_train_orderdata->{$user_id}};
		my @user_train_purchases = @$user_purchases[($bi .. $ei)];
		# use multinomial logistic regression
		# my $pos_samples = _gen_mlr_sample_per_user($FEAT_DICT_OBJ, $user_feat_map,$item_feat_map,$item_cfeat_map,$cat_label_map,\@user_train_purchases);
		my $pos_samples = _gen_pos_sample_per_user($FEAT_DICT_OBJ, $user_feat_map,$item_feat_map,$item_cfeat_map,$cat_pop_map,\@user_train_purchases);
		# generate negative samples
		my $neg_samples = _gen_neg_sample_per_user($FEAT_DICT_OBJ, $user_feat_map,$item_feat_map,$item_cfeat_map,$cat_pop_map,$top_cat_map,\@user_train_purchases);
		_write_sample_feature($result_file_id, $pos_samples);
		_write_sample_feature($result_file_id, $neg_samples);
		$user_cnt ++;
	}
	close $result_file_id;
}	


# generic feature handler
sub default_feature_handler{
	my($type, $feature, $value) = @_;
	# remove , from the value
	$value =~ s/\,//g;
	return ([join("_", ($type,$feature,$value))],[1]);
}

sub item_category_feature_handler{
	my($type, $feature, $value) = @_;
	# remove comma which will confuse csv file
	$value =~ s/\,//g;
	my @parts = split /\|/, $value;
	my @cats = ();
	for(my $i = 1; $i <= scalar @parts; $i++){
		my $tmp_cat = join("|",(@parts[0..$i - 1]));
		push @cats, join("_",($type, $i . $feature, $tmp_cat));
	}
	return (\@cats, [(1) x scalar @cats]);
}

# generate category popularity feature
sub category_pop_feature_handler{
	my ($cat_id, $cat_pop_map) = @_;
	my $pop_val = $cat_pop_map->{$cat_id};
	my $feat_name = join("_",("i","cp"));
	return ([$feat_name],[$pop_val]);
}

1;
