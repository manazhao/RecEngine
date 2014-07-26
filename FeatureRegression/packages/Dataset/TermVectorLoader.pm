#!/usr/bin/perl
#
package Dataset::TermVectorLoader;

use Exporter;
use Data::Dumper;
use JSON;
use strict;
use warnings;

our $VERSION = 1.0;
our @ISA = qw(Exporter);
my $indri_cmd;
sub new {
	my $class = shift;
	# object attributes
	my $obj_attrs = {
		@_
	};
	# make sure the index file is provided
	die "index and result parameter must be supplied" unless $obj_attrs->{"index"} and $obj_attrs->{"result"};
	$indri_cmd = "dumpindex " . $obj_attrs->{"index"} . " ";
	my $self = bless $obj_attrs;
	return $self;
}

sub _doc_tfidf{
	my($doc_id, $tv_map, $term_idf_map) = @_;
	my $max_freq = 0;
	foreach(values %$tv_map){
		$max_freq = $_ if $_ > $max_freq;
	}
	# normalize term frequency by dividing by the maximum frequency
	foreach(keys %$tv_map){
		# tf(term) * idf(term)
		# tf(t,d) = 0.5 + 0.5 * tf/max_freq
		if(!$term_idf_map->{$_}){
			print $doc_id . "\n";
			print Dumper($tv_map);
		}	
		$tv_map->{$_} = (0.5 + 0.5 * $tv_map->{$_}/$max_freq) * $term_idf_map->{$_};
	}
}

sub _term_idf{
	my $self = shift;
	print ">>> evaluating IDF for each term\n";
	my $num_doc = $self->{stats}->{num_docs};
	my $term_map = $self->{vacabulary};
	my %term_idf_map = ();
	map {$term_idf_map{$_} = log($num_doc/$term_map->{$_})} keys %$term_map;
	$self->{term_idf} = \%term_idf_map;
	return \%term_idf_map;
}


sub _doc_norm{
	print ">>> get document norm \n";
	my $self = shift;
	my $num_doc = $self->{stats}->{num_docs};
	$self->{doc_len_map} = {};
	foreach(1 .. $num_doc){
		my $doc_id = $_;
		my $tv_map = $self->{doc_tfidf}->{$doc_id};
		my $l2norm = 0;
		map {$l2norm += ($_ * $_)} values %$tv_map;
		$self->{doc_norm_map}->{$doc_id} = sqrt($l2norm);
	}
}

# the document similarity by okapi bm25
sub doc_sim_bm25{
	my $self = shift;
	my ($doc_id1, $doc_id2) = @_;
	my $doc_vec1 = $self->{doc_tfidf}->{$doc_id1};
	my $doc_vec2 = $self->{doc_tfidf}->{$doc_id2};
	my $len2 = $self->{doc_len_map}->{$doc_id2};
	my $avg_len = $self->{stats}->{avg_doc_len};
	my $k1 = 1.5;
	my $b = 0.75;
}

sub cosine_score{
	my ($self,$doc1_name,$doc2_name) = @_;
	my $doc1 = $self->{doc_name_id_map}->{$doc1_name};
	my $doc2 = $self->{doc_name_id_map}->{$doc2_name};

	my $doc1_tfidf = $self->{doc_tfidf}->{$doc1};
	my $doc2_tfidf = $self->{doc_tfidf}->{$doc2};
	my $doc1_norm = $self->{doc_norm_map}->{$doc1};
	my $doc2_norm = $self->{doc_norm_map}->{$doc2};
	if($doc1_norm == 0 or $doc2_norm == 0){
		return 0;
	}
	my $score = 0;
	while(my($term,$tfidf) = each %$doc1_tfidf){
		next unless exists $doc2_tfidf->{$term};
		$score += ($tfidf * $doc2_tfidf->{$term});
	}
	
	$score /= ($doc1_norm * $doc2_norm);
	return $score;
}

sub calc_tfidf{
	my $self = shift;
	my $term_idf_map = $self->_term_idf();
	$self->{doc_tfidf} = {};
	print ">>> evaluating tfidf for each document in the corpus\n";
	while(my($doc_id,$tv_map) = each %{$self->{doc_tv_map}}){
		# normalize by the maximum frequency
		my %tmp_tv_map = %$tv_map;
		_doc_tfidf($doc_id,\%tmp_tv_map, $term_idf_map);
		$self->{doc_tfidf}->{$doc_id} = \%tmp_tv_map;
	}
}

sub get_orig_doc_id{
	my $self = shift;
	my($id) = @_;
	return $self->{doc_id_name_map}->{$id};
}

sub write{
	my $self = shift;
	$self->_write_stats();
	$self->_write_doc_tv();
	$self->_write_vacabulary();
	$self->_write_docid_map();
	$self->_write_tfidf();
	$self->_write_term_idf();
}

sub read{
	my $self = shift;
	$self->_read_stats();
	$self->_read_vacabulary();
	$self->_read_doc_tv();
	$self->_read_docid_map();
	$self->_read_tfidf();
	$self->_read_term_idf();
	$self->_doc_norm();
}

sub _read_stats{
	my $self = shift;
	my $stat_file = $self->{"result"} . "/stats.json";
	print ">>> read index statistic information\n";
	open my $fh, "<", $stat_file or die $!;
	while(<$fh>){
		chomp;
		my $stat_json = decode_json($_);
		$self->{stats} = $stat_json;
		last;
	}
	close $fh;
}

sub _write_stats{
	my $self = shift;
	# write stat info
	my $stat_file = $self->{"result"} . "/stats.json";
	my $stat_json = encode_json(
		$self->{stats}
	);
	print ">>> write index statistic information\n";
	open my $fh, ">", $stat_file or die $!;
	print $fh $stat_json;
	close $fh;
}

sub _read_tfidf{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/doc_tfidf.csv";
	return unless -f $file;
	print ">>> read document tfidf vector\n";
	open my $fh, "<", $file or die $!;
	my %doc_tfidf = ();
	while(<$fh>){
		chomp;
		my ($doc_id,@fields) = split /\,/;
		if(scalar @fields > 0){
			my $h_idx = int($#fields/2);
			my @terms = @fields[0.. $h_idx];
			my @freqs = @fields[$h_idx + 1.. $#fields];
			my %tmp_map = ();
			@tmp_map{@terms} = @freqs;
			$doc_tfidf{$doc_id} = \%tmp_map;
		}
	}
	$self->{doc_tfidf} = \%doc_tfidf;
	close $fh;
}

sub _write_tfidf{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/doc_tfidf.csv";
	print ">>> write document tfidf\n";
	open my $fh, ">", $file or die $!;
	my $doc_tfidf = $self->{doc_tfidf};
	while(my($doc_id, $tv_map) = each %$doc_tfidf){
		my @terms = keys %$tv_map;
		my @freqs = map {sprintf("%.3f",$tv_map->{$_})} @terms;
		print $fh join(",",($doc_id,@terms,@freqs)) . "\n";
	}
	close $fh;
}

sub _read_doc_tv{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/doc_tv.csv";
	print ">>> read document vector\n";
	open my $fh, "<", $file or die $!;
	my %doc_tv_map = ();
	while(<$fh>){
		chomp;
		my ($doc_id,@fields) = split /\,/;
		my %tmp_tv_map = ();
		if(@fields){
			my $h_idx = int($#fields/2);
			my @terms = @fields[0.. $h_idx];
			my @freqs = @fields[$h_idx + 1.. $#fields];
			my @filter_idx = grep {$self->{vacabulary}->{$terms[$_]}} 0 .. $#terms;
			@terms = @terms[@filter_idx];
			@freqs = @freqs[@filter_idx];
			@tmp_tv_map{@terms} = @freqs;
		}
		$doc_tv_map{$doc_id} = \%tmp_tv_map;
	}
	$self->{doc_tv_map} = \%doc_tv_map;
	close $fh;
}

sub _write_doc_tv{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/doc_tv.csv";
	print ">>> write document vector\n";
	open my $fh, ">", $file or die $!;
	my $doc_tv_map = $self->{"doc_tv_map"};
	while(my($doc_id, $tv_map) = each %$doc_tv_map){
		my @terms = keys %$tv_map;
		my @freqs = map {$tv_map->{$_}} @terms;
		print $fh join(",",($doc_id,@terms,@freqs)) . "\n";
	}
	close $fh;
}

sub _read_term_idf{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/term_idf.csv";
	return if not -f $file;
	print ">>> read term idf \n";
	open my $fh, "<", $file or die $!;
	my %v_map = ();
	while(<$fh>){
		chomp;
		my($term,$num_doc) = split /\,/;
		$v_map{$term} = $num_doc;
	}
	$self->{term_idf} = \%v_map;
	close $fh;
}

sub _write_term_idf{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/term_idf.csv";
	print ">>> write term idf\n";
	open my $fh, ">", $file or die $!;
	my $v_map = $self->{term_idf};
	while(my($term, $num_doc) = each %$v_map){
		$num_doc = sprintf("%.3f",$num_doc);
		print $fh join(",",($term,$num_doc)) . "\n";
	}
	close $fh;
}

sub _read_vacabulary{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/vacabulary.csv";
	return if not -f $file;
	print ">>> read vacabulary map\n";
	open my $fh, "<", $file or die $!;
	my %v_map = ();
	while(<$fh>){
		chomp;
		my($term,$num_doc) = split /\,/;
		$v_map{$term} = $num_doc;
	}
	$self->{vacabulary} = \%v_map;
	close $fh;
}

sub _write_vacabulary{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/vacabulary.csv";
	print ">>> write vacabulary map\n";
	open my $fh, ">", $file or die $!;
	my $v_map = $self->{"vacabulary"};
	while(my($term, $num_doc) = each %$v_map){
		print $fh join(",",($term,$num_doc)) . "\n";
	}
	close $fh;
}

sub _read_docid_map{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/docid_name.csv";
	print ">>> read document ID to document name mapping\n";
	open my $fh, "<", $file or die $!;
	my %doc_id_name_map = ();
	my %doc_name_id_map = ();
	while(<$fh>){
		chomp;
		my($doc_id,$doc_name) = split /\,/;
		$doc_id_name_map{$doc_id} = $doc_name;
		$doc_name_id_map{$doc_name} = $doc_id;
	}
	$self->{doc_id_name_map} = \%doc_id_name_map;
	$self->{doc_name_id_map} = \%doc_name_id_map;
	close $fh;
}

sub _write_docid_map{
	my $self = shift;
	# write stat info
	my $file = $self->{"result"} . "/docid_name.csv";
	print ">>> write document ID to document name mapping\n";
	open my $fh, ">", $file or die $!;
	my $id_name_map = $self->{"doc_id_name_map"};
	while(my($doc_id, $doc_name) = each %$id_name_map){
		print $fh join(",",($doc_id,$doc_name)) . "\n";
	}
	close $fh;
}

sub query_index{
	my $self = shift;
	# load overall stats information
	print(">>> load stats\n");
	$self->_query_stat();
	print(">>> load vacabulary\n");
	$self->_query_vacabulary();
	print(">>> load document term vector\n");
	$self->_query_doc_tv();
	print(">>> load document id mapper\n");
	$self->_query_docid_name_map();
	print ">>> done!\n";
}

sub _run_cmd{
	my $cmd_str = join(" ", ($indri_cmd, @_));
	# print $cmd_str . "\n";
	return `$cmd_str`;
}

# get the general statistic information of the index
sub _query_stat{
	my $self = shift;
	my $output = _run_cmd("stats");
	my @lines = split /\n/, $output;
	my @parsed_lines = ();
	foreach(@lines[1..$#lines]){
		my($name,$value) = split /\:\s+/;
		push @parsed_lines, [$name,$value];
	}	
	my $stats = {};
	$stats->{num_docs} = $parsed_lines[0]->[1];
	$stats->{num_uniq_terms} = $parsed_lines[1]->[1];
	$stats->{num_terms} = $parsed_lines[2]->[1];
	$self->{stats} = $stats;
}

sub _query_vacabulary{
	my $self = shift;
	my $output = _run_cmd("v");
	my @lines = split /\n/, $output;
	my @terms = map {(split /\s+/, $_)[0]} @lines[1..$#lines];
	my @d_counts = map {(split /\s+/, $_)[2]} @lines[1..$#lines];
	my %term_map = ();
	@term_map{@terms} = @d_counts;
	$self->{"vacabulary"} = \%term_map;
}

# load term vector information for each document in the corpus
sub _query_doc_tv{
	my $self = shift;
	# manage the term vector information for each document
	my %doc_tv_map = ();
	print "Totoal number of documents: " . $self->{"num_docs"} . "\n";
	for(my $i = 1; $i <= $self->{"num_docs"}; $i++){
		if($i % 50 == 0){
			print $i . "\n";
		}
		my $output = _run_cmd("dv",$i);
		my @lines = split /\n/, $output;
		# term frequency map
		my %t_freq_map = ();
		foreach(@lines[2..$#lines]){
			my($idx,$term_id,$term) = split /\s+/;
			$t_freq_map{$term} ++;
		}
		$doc_tv_map{$i} = \%t_freq_map;
	}
	$self->{"doc_tv_map"} = \%doc_tv_map;
}

sub _query_docid_name_map {
	my $self = shift;
	my %id_name_map= ();
	for(my $i = 1; $i <= $self->{"num_docs"}; $i++){
		my $output = _run_cmd("dn",$i);
		# remove the trailing \n
		chomp $output;
		$id_name_map{$i} = $output;
	}
	$self->{"doc_id_name_map"} = \%id_name_map;
}

1;
