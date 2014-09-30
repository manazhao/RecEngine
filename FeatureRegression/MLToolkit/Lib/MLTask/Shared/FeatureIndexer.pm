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
use MLTask::Shared::Utility;

# use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

our $VERSION = 1.00;
our @ISA = qw(Exporter);

sub new {
	my($class,%args) = @_;
	my @arg_keys = qw(entity_type attribute_file feature_file feature_dict_file feature_handler);
	my %class_members = ();
	@class_members{@arg_keys} = @args{@arg_keys};
	MLTask::Shared::Utility::check_func_args("new", \%class_members);
	
	-f $class_members{attribute_file} or die "attribute file must exist";
	-d dirname($class_members{feature_file}) or die "invalid folder for feature file";
	my $self = \%class_members;
	$self->{feature_dict} = MLTask::Shared::FeatureDict::get_instance( file => $self->{feature_dict_file} );
	bless $self, $class;
	return $self;
}


sub index{
	my $self = shift;
	open FILE, "<" , $self->{attribute_file} or die "failed to open file - $self->{attribute_file}";
	open RESULT_FILE, ">", $self->{feature_file} or die "failed to open file - $self->{feature_file}";
	print ">>> load entity profile from - " . $self->{attribute_file} . " and index attribute as features\n";
	while(<FILE>){
		chomp;
		my $json_obj = decode_json($_);
		# get the id field
		exists $json_obj->{"id"} or (warn "object id is not defined" and next);
		my $entity_id = join("_", ($self->{entity_type},$json_obj->{"id"}));
		# remove entity id so only attributes remain
		delete $json_obj->{"id"};
		my $featname_val_map = $self->{feature_handler}->generate_feature($self->{entity_type},$json_obj);
		my %featidx_val_map = ();
		while(my ($feat_name, $feat_val) = each %{$featname_val_map}){
			my $feat_id = $self->{feature_dict}->index_feature($feat_name);
			$featidx_val_map{$feat_id} = $feat_val;
		}
		my $feat_str = join(",", map {$_ . ":" . $featidx_val_map{$_}} keys %featidx_val_map);
		print RESULT_FILE join(",",($entity_id, $feat_str)) . "\n";
	}
	close RESULT_FILE;
	close FILE;
}

1;
