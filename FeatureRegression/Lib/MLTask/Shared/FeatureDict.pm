#!/usr/bin/perl
#
package MLTask::Shared::FeatureDict;

use Exporter;
use Data::Dumper;

our $VERSION = 1.0;
our @ISA = qw(Exporter);

my $file;
my $fd;
my $id_map = {};
my $name_map = {};
my $max_id = 0;
my $inited = 0;
my $OBJ_REF = bless {};

sub _load_from_file{
	my $self = shift;
	-f $file or die "file not exist:$!";
	# read from file
	open my $fd1, "<" , $file or die $!;
	while(<$fd1>){
		chomp;
		my($name,$id) = split /\,/;
		if($id > $max_id){
			$max_id = $id;
		}
		$id_map->{$name} = $id;
		$name_map->{$id} = $name;
	}
	close $fd1;
	$inited = 1;
}

sub get_instance{
	my %params = @_;
	if(not $inited){
		die "missing feature file:$!" unless exists $params{"file"};
		# 
		$file = $params{"file"};
		$OBJ_REF->_load_from_file();
		open $fd, ">>" ,$file or die $!;
	}
	return $OBJ_REF;
}


sub index_feature{
	my($self,$name) = @_;
	if(not exists($id_map->{$name})){
		$max_id ++;
		$id_map->{$name} = $max_id;
		$name_map->{$max_id} = $name;
		# print to file
		print $fd join(",",($name,$max_id)) . "\n";
	}
	return $id_map->{$name};
}

sub query_by_name{
	my($self,$name) = @_;
	return $id_map->{$name};
}

sub query_by_id{
	my($self,$id) = @_;
	return $name_map->{$id};
}

sub dump{
	my $self = shift;
	print Dumper($id_map);

}

sub get_id_map{
	return $id_map;
}

sub get_name_map{
	return $name_map;
}

sub DESTROY{
	# close file
	my $self = shift;
	close $fd;
	$inited = 0;
}

