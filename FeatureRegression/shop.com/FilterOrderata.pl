#!/usr/bin/perl
use FindBin;
use warnings;
use strict;

my $input_data_file = "/data/jian-data/shop-data/orderdata.txt.perUser.ordered";
my $output_data_file = "/data/jian-data/shop-data/processed/orderdata.gt5.lt200";
open INPUT_FILE, "<", $input_data_file or die $1;
open RESULT_FILE, ">", $output_data_file or die $!;

my %user_purchase_map = ();
my $last_user_id = 0;

my @lines = ();
while(<INPUT_FILE>)
{
	my $line = $_;
	chomp;
	my($user_id,@rest_fields) = split /\t/;
	# a new user starts
	if($user_id != $last_user_id){
		#
		if(scalar @lines >= 5 && @lines < 200){
			print RESULT_FILE join("",@lines);
		}
		@lines = ();
		$last_user_id = $user_id;
	}
	push @lines, $line;
}
close INPUT_FILE;
close RESULT_FILE;
