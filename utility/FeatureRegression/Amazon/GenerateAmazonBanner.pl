#!/user/bin/perl
#
use strict;
use warnings;
use JSON;
use Getopt::Long;
use File::Basename;

my $pred_result_file;
my $max_num_rec = 20;

GetOptions("pred=s" => \$pred_result_file, "max=i" => \$max_num_rec) or die $!;

-f $pred_result_file or die $!;

open PRED_FILE , "<" , $pred_result_file or die $!;
my $html_file = $pred_result_file."_rec.html";
open HTML_FILE, ">", $html_file or die $!;

my $rec_cnt = 0;
while(<PRED_FILE>){
    chomp;
    my ($asin, $score) = split /\,/;
    print HTML_FILE "<html><body>";
    my $iframe_str = '<iframe style="width:120px;height:240px;" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" src="http://ws-na.amazon-adsystem.com/widgets/q?ServiceVersion=20070822&OneJS=1&Operation=GetAdHtml&MarketPlace=US&source=ac&ref=tf_til&ad_type=product_link&tracking_id=buybuytoget0e-20&marketplace=amazon&region=US&placement=0321714113&asins=' . $asin . '&linkId=1586176242&show_border=true&link_opens_in_new_window=true"> </iframe>';
    print HTML_FILE $iframe_str . "\n";
    $rec_cnt ++;
    last if $rec_cnt == $max_num_rec;
}
print HTML_FILE "</body></html>";
close HTML_FILE;

close PRED_FILE;
