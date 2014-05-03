#!/user/bin/perl
#
use strict;
use warnings;

my $rec_folder = "amazon-rec";
chdir $rec_folder;

my $file_list = `ls test*.list`;
my @files = split '\s+', $file_list;

foreach(@files){
	my $orig_file = $_;
	print $orig_file . "\n";
	my $html_file = $orig_file.".html";
	open HTML_FILE, ">", $html_file or die $!;
	open ORIG_FILE, "<", $orig_file or die $!;
	print HTML_FILE "<html><body>";
	while(<ORIG_FILE>){
		chomp;
		my $line = $_;
		my @matches = ($line  =~ m/dp\/([\d\w]+)/g);
		my $asin = $matches[0];
		my $iframe_str = '<iframe style="width:120px;height:240px;" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" src="http://ws-na.amazon-adsystem.com/widgets/q?ServiceVersion=20070822&OneJS=1&Operation=GetAdHtml&MarketPlace=US&source=ac&ref=tf_til&ad_type=product_link&tracking_id=buybuytoget0e-20&marketplace=amazon&region=US&placement=0321714113&asins=' . $asin . '&linkId=1586176242&show_border=true&link_opens_in_new_window=true"> </iframe>';
			print HTML_FILE $iframe_str . "\n";
	}
	print HTML_FILE "</body></html>";
	close ORIG_FILE;
	close HTML_FILE;
}
