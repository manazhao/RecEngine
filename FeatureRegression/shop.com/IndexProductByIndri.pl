#!/usr/bin/perl
#
# identify similar products by text matching
# package dependence:
# 1) Lingua::StopwWords - remove stopwords from text
# 2) Lin
#
use strict;
use warnings;

my $product_file = '/data/jian-data/shop-data/orderdata.txt.productIndex';
my $title_corpus = '/data/jian-data/shop-data/product_nlp/corpus/title';
my $desc_corpus = '/data/jian-data/shop-data/product_nlp/corpus/desc';

-f $product_file and -d $title_corpus and -d $desc_corpus or die $!;

open my $product_fh, "<", $product_file or die $!;

my $cnt = 0;
while(<$product_fh>){
	# ProdID, CatalogName, OrderText, Description , LocalPrice,  Sizes , Colors,  Category ,  Attributes
	my($pid, $orig_pid, $cln, $ot, $d, $lp, $s, $colors, $cat, $attr) = split /\t/;
	index_text($title_corpus,$pid,$ot);
	index_text($desc_corpus,$pid,$d);
	$cnt++;
#	last if $cnt == 100;
}

sub index_text{
	my($corpus_dir,$doc_id,$text) = @_;
	my $doc = generate_trec_doc($doc_id,$text);
	my $doc_path = $corpus_dir . "/$doc_id";
	write_text_to_file($doc_path,$doc);
}

sub write_text_to_file{
	my($file_name, $text) = @_;
	open my $tmp_fh, ">", $file_name or die $!;
	print $tmp_fh $text;
	close $tmp_fh;

}

sub generate_trec_doc{
	my($doc_id,$text) = @_;
	return "<DOC>\n<DOCNO>$doc_id</DOCNO>\n<TEXT>$text</TEXT>\n</DOC>\n";
}
