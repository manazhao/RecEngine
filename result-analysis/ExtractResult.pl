#!/usr/bin/perl
#
use strict;
use warnings;

my $result_dir = "../amazon/amazon-result";

chdir $result_dir;
# process each result file
my $result_file_str = `ls *.txt`;
my @result_files = split '\s+', $result_file_str;

foreach(@result_files){
	my $result_file = $_;
	my $output_file= $result_file;
	$output_file =~ s/\.txt/_extract.txt/;
	open RESULT_FILE, "<", $result_file or die $!;
	open OUTPUT_FILE, ">", $output_file or die $!;
	while(<RESULT_FILE>){
		chomp;
		if(/train rmse:([\d\.]+).*test rmse:([\d\.]+).*cs rmse:([\d\.]+)/){
			print OUTPUT_FILE join("\t",($1,$2,$3)) . "\n";	
		}
	}
	close RESULT_FILE;
	close OUTPUT_FILE;
}
