package MLTask::Shared::Utility;

use strict;
use warnings;
use File::Basename;
use YAML::Tiny;
use FindBin;

# library path for feature handlers
use lib "$FindBin::Bin/../../";
use Exporter;
our $VERSION = 1.00;
our @ISA = qw(Exporter);
our @EXPORT = qw(check_func_args normalize_text);

sub check_func_args{
	my ($func_name,$arg_map) = @_;
	my $num_undef = 0;
	map { defined $arg_map->{$_} or $num_undef++} keys %$arg_map;
	if($num_undef){
		# print function signature
		my $arg_str = join(",", map { "$_ =>" . (not defined $arg_map->{$_} and "NA") } keys %$arg_map);
		return  "$func_name($arg_str)";
	}
	return "";
}


sub normalize_text{
    my($text) = @_;
    # remove extra spaces
    $text =~ s/\s+/ /g;
    # lower case
    $text = lc $text;
    return $text;
}
