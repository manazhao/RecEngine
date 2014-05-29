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
# feature dictionary
my $feat_dict_file;

GetOptions("agg=s" => \$agg_freq_file, "item=s" => \$item_profile_file,"result=s" => \$result_file,"tree=s"=>\$cat_tree_file, "dict=s" => \$feat_dict_file) or die $!;

($agg_freq_file and -f $agg_freq_file) or die $!;
($item_profile_file and -f $item_profile_file) or die $!;
($result_file and -d dirname($result_file)) or die $!;
($cat_tree_file and -d dirname($cat_tree_file)) or die $!;
($feat_dict_file and -d dirname $feat_dict_file) or die $!;

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

my %feat_map = ();
my $max_feat_id = 0;
if(-f $feat_dict_file){
    print ">>> read feature dictionary\n";
    open DICT_FILE, "<", $feat_dict_file or die $!;
    while(<DICT_FILE>){
        chomp;
        my ($name,$id) = split /\,/;
        $feat_map{$name} = $id;
        if($id && $id > $max_feat_id){
            $max_feat_id = $id;
        }
    }
    close DICT_FILE;
}
$max_feat_id++;

open DICT_FILE, ">>", $feat_dict_file or die $!;

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
        $ipy_map{$iid}->{$year} = $freq;
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
        $cpy_map{$cid}->{$year} = $freq;
    }
}

close AGG_FILE;

# normalized category popularity and category popularity w.r.t year
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
    my %item_feat_map = ();

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
        my $feat_id = get_feature_id($feat_name);
        # round to three decimal digits...
        my $n_freq = sprintf("%.3f",$item_pop / $max_ip_freq);
        $n_freq > 0 and $item_feat_map{$feat_id} =  $n_freq;
    }else{
        my $mis_feat_name = "i_ip_#miss";
        my $feat_id = get_feature_id($mis_feat_name);
        $item_feat_map{$feat_id} = 1;
    }

    # add item popularity year feature
    if(exists $ipy_map{$item_id}){
        my %year_freq_map = %{$ipy_map{$item_id}};
        while(my($year,$freq) = each %year_freq_map){ 
            $tmp_ipy_map{$year} = $freq;
            my $max_ipy = $max_ipy_map{$year};
            my $feat_name = "i_ipy_" . $year;
            # normalize the frequency
            my $feat_id = get_feature_id($feat_name);
            my $n_freq = sprintf("%.3f",$freq / $max_ipy);
            $n_freq > 0 and $item_feat_map{$feat_id} = $n_freq;
        }
    }else{
        # if this information is missing
        my $mis_feat_name = "i_ipy_#miss";
        my $feat_id = get_feature_id($mis_feat_name);
        $item_feat_map{$feat_id} = 1;
    }

    # add normalized category popularity
    if(exists $item_json->{"c"} and $item_json->{"c"}){
        # get item categories
        @item_cats = item_category_feature_handler($item_json->{"c"});
        # get the normalized category frequence
        foreach my $cat(@item_cats){
            my $cat_freq = $freq_map{"cp_$cat"};
            # normalized frequency
            my $n_freq;
            # check whether it's already evaluated
            if(exists $n_cp_map{"cp_$cat"}){
                $n_freq = $n_cp_map{"cp_$cat"};
            }else{
                # for root category
                $n_freq = 1;
                if(exists $cat_parent_map{$cat}){
                    my $p_cat = $cat_parent_map{$cat};
                    my $p_cat_freq = $freq_map{"cp_$p_cat"};
                    if($cat_freq && $p_cat_freq){
                        $n_freq = $cat_freq / $p_cat_freq;
                    }
                }
                # save the normalized frequency
                $n_freq = $n_freq == 1? 1: sprintf("%.3f",$n_freq);
                $n_freq > 0 and $n_cp_map{"cp_$cat"} = $n_freq;
            }
            my $feat_name = "i_cp_$cat";
            my $feat_id = get_feature_id($feat_name);
            $item_feat_map{$feat_id} = $n_freq;

            # get the normalized cpy as well
            if(exists $cpy_map{$cat}){
                my %cpy_map = %{$cpy_map{$cat}};
                while(my ($year,$freq) = each %cpy_map){
                    my $n_freq;
                    if(exists $n_cpy_map{"cpy_$cat". "_$year"}){
                        $n_freq = $n_cpy_map{"cpy_$cat" . "_$year"};
                    }else{
                        $n_freq = 1;
                        if(exists $cat_parent_map{$cat}){
                            my $p_cat = $cat_parent_map{$cat};
                            my $p_cy_freq = $freq_map{"cpy_$p_cat" . "_$year"};
                            if($p_cy_freq){
                                $n_freq = $freq / $p_cy_freq;
                            }
                        }
                        $n_cpy_map{"cpy_$cat" . "_$year"} = $n_freq;
                    }
                    my $feat_name = "i_cpy_$cat". "_$year";
                    my $feat_id = get_feature_id($feat_name);
                $n_freq = $n_freq == 1? 1: sprintf("%.3f",$n_freq);
                $n_freq > 0 and $item_feat_map{$feat_id} = $n_freq;
                }
            }

            # add item category popularity feature
            if($cat_freq && $ip_freq){
                my $cip_freq = $ip_freq / $cat_freq;
                my $tmp_feat_key = "i_cip_$cat";
                # normalized category item popularity
                my $feat_id = get_feature_id($tmp_feat_key);
                $cip_freq = sprintf("%.3f",$cip_freq);
                $cip_freq > 0 and $item_feat_map{$feat_id} = $cip_freq;
            }

            # add item category year popularity feature
            while(my($year,$ipy) = each %tmp_ipy_map){
                my $n_freq = 1;
                my $cpy_freq = $freq_map{"cpy_$cat". "_$year"};
                if($cpy_freq){
                    $n_freq = $ipy / $cpy_freq;
                }
                my $feat_name = "i_cipy_$cat" . "_$year"; 
                my $feat_id = get_feature_id($feat_name);
                $n_freq = $n_freq == 1? 1: sprintf("%.3f",$n_freq);
                $n_freq > 0 and $item_feat_map{$feat_id} = $n_freq;
            }
        }
    }
    # generate feature values 
    my @feat_vals = map { join(":",($_ , $item_feat_map{$_})) } keys %item_feat_map;
    # write to result file
    print RESULT_FILE join(",",("i_" . $item_id,@feat_vals)) . "\n";
}

close RESULT_FILE;
close DICT_FILE;
close ITEM_FILE;

sub item_category_feature_handler{
    my($value) = @_;
    # split the category strigns
    my @cat_strs = split /\|/, $value;
    my %result_cats = ();
    foreach my $cat_str(@cat_strs){
        # further split by /
        my @sub_cats = split /\//, $cat_str;
        # up to three level categories 
        # slice 0..2
        @sub_cats = @sub_cats[0.. ($#sub_cats < 2? $#sub_cats : 2)];
        foreach(@sub_cats){
            my ($cat_id, $cat_name) = split /\-/ , $_;
            $result_cats{$cat_id} = 1;
        }
    }
    return keys %result_cats;
}

sub get_feature_id{
    my ($feat_name) = @_;
    my $feat_id = $feat_map{$feat_name};
    if(not defined $feat_id){
        # not exist
        $feat_id = $max_feat_id;
        $feat_map{$feat_name} = $feat_id;
        print DICT_FILE join(",", ($feat_name,$feat_id)) . "\n";
	$max_feat_id++;
    }
    return $feat_id;

}
