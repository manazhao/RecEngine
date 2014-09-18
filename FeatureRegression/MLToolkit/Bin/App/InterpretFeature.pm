package App::InterpretFeature;
use strict;
use warnings;
use Data::Dumper;
use Getopt::Long;
use Exporter;
use FindBin;
use lib "$FindBin::Bin/../../Lib/";
# use feature dictionary
use MLTask::Shared::FeatureDict;

our $VERSION=1.00;
our @ISA=qw(Exporter);

sub run{
	my $dict_file;
	GetOptions("dict=s" => \$dict_file) or die $!;
	$dict_file or usage();
	-f $dict_file or die $!;
	# load the dictionary
	my $feat_dict_obj = MLTask::Shared::FeatureDict::get_instance(file=>$dict_file);
	# read feature id from the standard input
	while(<>){
		chomp;
		my $fid = $_;
		my $fname = $feat_dict_obj->query_by_id($fid);
		$fname or (warn "feature - $fid : does not exist" and next);
		# interaction feature
		my @feats = $fname =~ m/(\d+)\|(\d+)/g;
		if(scalar @feats > 0){
			# print Dumper(@feats);
			my @feat_desc = ();
			foreach(@feats){
				my $tmpname = $feat_dict_obj->query_by_id($_);
				push @feat_desc, $tmpname;	
			}
			$fname = join("|",@feat_desc);
		}
		print $fid . "\t" . $fname . "\n";
	}
}


sub usage{
	print <<EOF;
$0:	[options]
	--dict		feature dictionary file path
EOF
	exit(1);
}

1;
