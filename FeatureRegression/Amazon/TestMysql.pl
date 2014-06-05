#!/usr/bin/perl
#
use strict;
use warnings;
use Data::Dumper;

use DBI;

my $dsn = "DBI:mysql:database=amazon; host=localhost";
my $dbh = DBI->connect($dsn,"root",'StU8e@uh');


my $sth = $dbh->prepare("select * from rating_user");
$sth->execute;
while(my $ref = $sth->fetchrow_hashref()){
	print Dumper($ref);
}

