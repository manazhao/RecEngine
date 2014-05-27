#!/usr/bin/perl
# generate aggregated level features for item

use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use JSON;
use Data::Dumper;

# aggregated frequency file 
my $agg_freq_file;
# contain item feature
my $item_profile_file;
# store the result
my $result_file;
# category tree
my $cat_tree_file;

GetOptions("agg=s" => \$agg_freq_file, "item=s" => \$item_profile_file,"result=s" => \$result_file,"tree=s"=>\$cat_tree_file) or die $!;

($agg_freq_file and -f $agg_freq_file) or die $!;
($item_profile_file and -f $item_profile_file) or die $!;
($result_file and -d dirname($result_file)) or die $!;
($cat_tree_file and -d dirname($cat_tree_file)) or die $!;

my %freq_map = ();
my %ipy_map = ();
my %cpy_map = ();
my %cat_parent_map = ();
my $max_ip_freq = 0;
my %max_ipy_map = ();

print ">>> category tree file\n";
open TREE_FILE, "<", $cat_tree_file or die $!;
while(<TREE_FILE>){
	chomp;
	my ($cat,$p_cat) = split /\,/;
	$cat_parent_map{$cat} = $p_cat;
}
close TREE_FILE;

print ">>> read aggregated frequence file\n";

open AGG_FILE, "<", $agg_freq_file or die "failed to open file - $agg_freq_file: $!";

while(<AGG_FILE>){
	chomp;
	my($key,$freq) = split /\,/;
	$freq_map{$key} = $freq;
	# item popularity
	if($key =~ m/ip_.*?/g){
		if($freq > $max_ip_freq){
			$max_ip_freq = $freq;
		}
	}
	my @matches = $key =~ m/ipy_([^_]+)_(\d+)/g;
	if(scalar @matches > 0){
		my ($iid, $year) = @matches[0..1];
		push @{$ipy_map{$iid}}, [$year, $freq];
		if(not exists $max_ipy_map{$year}){
			$max_ipy_map{$year} = $freq;
		}else {
			if ($freq > $max_ipy_map{$year}){
				$max_ipy_map{$year} = $freq;
			}
		}
	}
	# category year popularity
	@matches = $key =~ m/cpy_([^_]+)_(\d+)/g;
	if(scalar @matches > 0){
		my ($cid, $year) = @matches[0..1];
		push @{$cpy_map{$cid}}, [$year, $freq];
	}
}

close AGG_FILE;

my %n_cp_map = ();
my %n_cpy_map = ();

open ITEM_FILE, "<", $item_profile_file or die "failed to open file - $item_profile_file: $!";
open RESULT_FILE, ">", $result_file or die "failed to open file - $result_file:$!";
print ">>> read item profile file\n";
while(<ITEM_FILE>){
	chomp;
	my $item_json = decode_json($_);
	# get the item id
	my $item_id = $item_json->{"id"}; 
	# get the its popularity
	my $ip_key = "ip_$item_id";
	my %result_json = ();
	my @item_cats = ();

	# item popularity
	my $ip_freq;
	my %tmp_ipy_map = ();
	if(exists $freq_map{$ip_key}){
		my $item_pop = $freq_map{$ip_key};		
		$ip_freq = $item_pop;
		# generate feature
		my $feat_name = "i_ip";
		# normalize the frequency
		$result_json{$feat_name} = sprintf("%.5f",$item_pop / $max_ip_freq);
	}

	# add item popularity year feature
	if(exists $ipy_map{$item_id}){
		my @year_freq_arr = @{$ipy_map{$item_id}};
		foreach my $year_freq (@year_freq_arr){
			my ($year, $freq) = @$year_freq;
			$tmp_ipy_map{$year} = $freq;
			my $max_ipy = $max_ipy_map{$year};
			my $feat_name = "i_ipy_" . $year;
			# normalize the frequency
			$result_json{$feat_name} = sprintf("%.5f",$freq / $max_ipy);
		}
	}

	# add normalized category popularity
	if(exists $item_json->{"c"} and $item_json->{"c"}){
		@item_cats = item_category_feature_handler($item_json->{"c"});
		# get the normalized category frequence
		foreach my $cat(@item_cats){
			my $n_freq;
			if(exists $n_cp_map{"cp_$cat"}){
				$n_freq = $n_cp_map{"cp_$cat"};
			}else{
				my $n_freq = 1;
				if(exists $cat_parent_map{$cat}){
					my $p_cat = $cat_parent_map{$cat};
					my $p_cat_freq = $freq_map{"cp_$p_cat"};
					my $cat_freq = $freq_map{"cp_$cat"};
					if($cat_freq){
						$n_freq = $cat_freq / $p_cat_freq;
					}
				}
				$n_cp_map{"cp_$cat"} = sprintf("%.5f",$n_freq);
			}	
			$result_json{"i_cp_$cat"} = $n_freq;
			# get the normalized cpy as well
			if(exists $cpy_map{$cat}){
				my @cpy_arr = @{$cpy_map{$cat}};
				foreach(@cpy_arr){
					my ($year,$freq) = @$_;
					my $n_freq;
					if(exists $n_cpy_map{"cpy_$cat". "_$year"}){
						$n_freq = $n_cpy_map{"cpy_$cat" . "_$year"};
					}else{
						$n_freq = 1;
						if(exists $cat_parent_map{$cat}){
							my $p_cat = $cat_parent_map{$cat};
							my $p_cy_freq = $freq_map{"cpy_$p_cat" . "_$year"};
							my $cy_freq = $freq_map{"cpy_$cat" . "_$year"};
							$n_freq = $cy_freq / $p_cy_freq;
						}
						$n_cpy_map{"cpy_$cat" . "_$year"} = $n_freq;
					}
					$result_json{"i_cpy_$cat". "_$year"} = sprintf("%.5f",$n_freq);
				}
			}

			# add item category popularity feature
			if(exists $freq_map{"cp_$cat"} && $ip_freq){
				my $cat_freq = $freq_map{"cp_$cat"};
				my $cip_freq = $ip_freq / $cat_freq;
				my $tmp_feat_key = "i_cip_$cat";
				# normalized category item popularity
				$result_json{$tmp_feat_key} = sprintf("%.5f",$cip_freq);
			}
			# add item category year popularity feature
			while(my($year,$freq) = each %tmp_ipy_map){
				my $n_freq = 1;
				if(exists $freq_map{"cpy_$cat". "_$year"}){
					my $cat_freq = $freq_map{"cpy_$cat". "_$year"};
					$n_freq = $freq / $cat_freq;
				}
				$result_json{"i_cipy_$cat" . "_$year"} = sprintf("%.5f", $n_freq);
			}
		}
	}
	$result_json{"id"} = $item_id;
	print RESULT_FILE encode_json(\%result_json) . "\n";
}

close RESULT_FILE;
close ITEM_FILE;

sub item_category_feature_handler{
	my($value) = @_;
	# split the category strigns
	my @cat_strs = split /\|/, $value;
	my %result_cats = ();
	foreach my $cat_str(@cat_strs){
		# further split by /
		my @sub_cats = split /\//, $cat_str;
		foreach(@sub_cats){
			my ($cat_id, $cat_name) = split /\-/ , $_;
			$result_cats{$cat_id} = 1;
		}
	}
	return keys %result_cats;
}
