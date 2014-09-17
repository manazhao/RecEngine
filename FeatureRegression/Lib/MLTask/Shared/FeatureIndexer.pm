# generate feature file from a given entity attribute file
package MLTask::Shared::FeatureIndexer;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;
use FindBin;

# library path for feature handlers
use lib "$FindBin::Bin/../../";
use Exporter;
use JSON;
use Data::Dumper;
# feature dictionary object
use MLTask::Shared::FeatureDict;

# use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new {
	my($class,%args) = @_;
	my $self = {};
	# set the arguments, 
	@{$self->{keys %args}} = values %args;
	# entity type and file
	$self->{type} and $self->{attribute_file} and $self->{feature_file} and $self->{feature_dict} and $self->{feature_handler} or usage();
	-f $self->{attribute_file} or die "attribute file must exist";
	-d dirname($self->{feature_file}) or die "invalid folder for feature file";
	bless $self, $class;
	return $self;
}

sub usage{
	my %args = (
		type => "entity type",
		attribute_file => "attribute_file",
		feature_file => "resutled feature file",
		feature_handler => "feature handler object",
		"feature_dict" => "feature dictionary object"
	);
	my $arg_str = join(" ",map { "$_=<$args{$_}>"} keys %args);
	print "new FeatureIndexer($arg_str)";
	exit 1;
	
}

sub index{
	my $self = shift;
	open FILE, "<" , $self->{attribute_file} or die "failed to open file - $self->{attribute_file}";
	open RESULT_FILE, ">", $self->{feature_file} or die "failed to open file - $self->{feature_file}";
	while(<FILE>){
		chomp;
		my $json_obj = decode_json($_);
		# get the id field
		exists $json_obj->{"id"} or (warn "object id is not defined" and next);
		my $entity_id = join("_", ($self->{type},$json_obj->{"id"}));
		delete $json_obj->{"id"};
		my $featname_val_map = $self->{feature_handler}->generate_feature($self->{type},$json_obj);
		my %featidx_val_map = ();
		while(my ($feat_name, $feat_val) = each %{$featname_val_map}){
			my $feat_id = $self->{feature_dict}->index_feature($feat_name);
			$featidx_val_map{$feat_id} = $feat_val;
		}
		my $feat_str = join(",", map {$_ . ":" . $featidx_val_map{$_}} keys %featidx_val_map);
		print RESULT_FILE join(",",($self->{type}. "_" . $entity_id, $feat_str)) . "\n";
	}
	close RESULT_FILE;
	close FILE;
}

1;
