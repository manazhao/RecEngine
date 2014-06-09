#!/usr/bin/perl
#
# generate category tree
use strict;
use warnings;
use Data::Dumper;
use Getopt::Long;
use File::Basename;

# category name <-> id mapping file
my $id_name_file;
# category parent relationship file
my $cat_pr_file;
# store the result
my $cat_tree_file;
GetOptions("name=s" => \$id_name_file, "parent=s" => \$cat_pr_file, "result=s" => \$cat_tree_file) or die $!;

$id_name_file and $cat_pr_file and $cat_tree_file or die "--name=<category name id file> --parent=<category parent map file> --result=<category tree file>: $!";
-f $id_name_file and -f $cat_pr_file and -d dirname($cat_tree_file) or die $!;
open ID_NAME_FILE, "<", $id_name_file or die $!;
open PARENT_FILE , "<", $cat_pr_file or die $!;
open RESULT_FILE, ">", $cat_tree_file or die $!;
my %cat_path_map = ();
my %id_name_map = ();

while(<ID_NAME_FILE>){
	chomp;
	my($id,$name) = split /\,/;
	$id_name_map{$id} = $name;
}

close ID_NAME_FILE;

my %id_parent_map = ();
while(<PARENT_FILE>){
	chomp;
	my($id,$pid) = split /\,/;
	$id_parent_map{$id} = $pid;
}

close PARENT_FILE;

while(my($id,$name) = each %id_name_map){
	my $parent_nodes = node_helper($id);
	my @not_exist_ids = grep {not exists $id_name_map{$_}} @$parent_nodes;
	my @parent_node_names = map {join "-", ($_, $id_name_map{$_})} @$parent_nodes;
	print RESULT_FILE join(",",($id."-".$name,join("/",@parent_node_names))) . "/\n";
}

close RESULT_FILE;

sub node_helper{
	my ($cur_node_id) = @_;
	my $parent_nodes = [];
	if(not exists $cat_path_map{$cur_node_id}){
		if(exists $id_parent_map{$cur_node_id}){
			my $parent_id = $id_parent_map{$cur_node_id};
			my $parent_path = node_helper($parent_id);
			$parent_nodes = [$parent_id,@$parent_path];
			$cat_path_map{$cur_node_id} = $parent_nodes;
		}else{
			$cat_path_map{$cur_node_id} = [];
		}
	}
	return $cat_path_map{$cur_node_id};
}
