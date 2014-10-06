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
our @EXPORT = qw(check_func_args normalize_text traverse_variable_recursive);

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

sub traverse_variable_recursive{
	my ($var,$callback_sub) = @_;
	my $ref_type = ref($var);
	if($ref_type eq "HASH"){
		# traverse each of key -value pairs
		while(my($key,$value) = each %$var){
			my $val_type = ref($value);
			if(not $val_type){
				# call the callback function
				$var->{$key} = $callback_sub->{$value};
			}else{
				_traverse_variable_recursive($value,$callback_sub);
			}
		}
	}elsif($ref_type eq "ARRAY"){
		foreach my $value(@$var){
			my $val_type = ref($value);
			if(not $val_type){
				# call the callback function
				$value = $callback_sub->{$value};
			}else{
				_traverse_variable_recursive($value,$callback_sub);
			}
		}
	}
}
