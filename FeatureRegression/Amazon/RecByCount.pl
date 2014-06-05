#!/usr/bin/perl
#
# make recommendation by counting given gender and age
#
use strict;
use warnings;
use Getopt::Long;
use JSON;

use Data::Dumper;
use DBI;

# connect to database


my $user_feat_file;
my $rating_file;
my $dict_file;

GetOptions("dict=s" => \$dict_file,"user=s" => \$user_feat_file, "rating=s" => \$rating_file) or die $!;

($dict_file && $user_feat_file && $rating_file) or die "feature dictionary file, user feature file,  and rating file must be provided: $!";

my %feat_map = ();
my %user_feat_map = ();

-f $dict_file and -f $user_feat_file and -f $rating_file or die $!;
open DICT_FILE, "<", $dict_file or die $!;
open USER_FILE, "<", $user_feat_file or die $!;
open RATING_FILE, "<", $rating_file or die $!;

while(<DICT_FILE>){
	chomp;
	my($feat_name, $feat_id) = split /\,/;
	$feat_map{$feat_id} = $feat_name;
}
close DICT_FILE;

while(<USER_FILE>){
	chomp;
	my($user_id, @feats) = split /\,/;
	my @feat_ids = map { (split /\:/, $_)[0] } @feats;
	foreach my $feat(@feat_ids){
		if(exists $feat_map{$feat}){
			my $feat_name = $feat_map{$feat};
			if($feat_name =~ /age/){
				$user_feat_map{$user_id}->{"age"} = $feat;
			}
			if($feat_name =~ /gender/){
				$user_feat_map{$user_id}->{"gender"} = $feat;
			}
		}
	}
}
close USER_FILE;

my $dsn = "DBI:mysql:database=amazon; host=localhost";
my $dbh = DBI->connect($dsn,"root",'StU8e@uh');
my $sth = $dbh->prepare("INSERT INTO rating_user (user_id,item_id,age,gender) VALUES (?, ?, ?, ?)");
while(<RATING_FILE>){
	chomp;
	my $json_obj = decode_json($_);
	my $uid = $json_obj->{"u"};
	my $iid = $json_obj->{"i"};
	my $rating = $json_obj->{"r"};
	my $age = $user_feat_map{$uid}->{"age"};
	my $gender = $user_feat_map{$uid}->{"gender"};
	$sth->execute($uid,$iid,$age,$gender);
}
$sth->finish();
$dbh->disconnect();
close RATING_FILE;


