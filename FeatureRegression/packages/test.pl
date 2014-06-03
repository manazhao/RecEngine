#!/usr/bin/perl
#
use lib "./";

# test functions defined in Data::Common
#
use Data::Common;
use Data::Dumper;

my $user_profile_file = "/home/qzhao2/Dropbox/data/amazon_book_rating/author_profile.json";

my $user_attr_map = load_entity_json("u",$user_profile_file);

while(my($key,$value) = each %$user_attr_map){
	print Dumper($value);
}


