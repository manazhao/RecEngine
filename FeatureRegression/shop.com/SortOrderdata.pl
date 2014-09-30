#!/usr/bin/perl
#
# sort order data by timestamp and user id
# convert data time to timestamp
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use Date::Manip;
use JSON;
my $input_file;
my $output_file;

my $script_name = basename($0);
GetOptions("input=s" => \$input_file, "output=s" => \$output_file) or die $1;

$input_file and $output_file or die "$script_name --input=<original order data> --output=<sorted order data file>:$!";

-f $input_file or die $!;
-d dirname($output_file) or die $!;

open INPUT_FILE, "<", $input_file or die $!;
open OUTPUT_FILE, ">", $output_file or die $!;

# input data format:
#  orderid, invoiceid, purchaseid, shopperid, datetimestamp, sessionid, catalogid, prodid, SKU, price, quantity, desc    ription, CookieID

my %user_order_map = ();

while(<INPUT_FILE>){
	chomp;
	my @row = split "\t";
	my($order_id, $invoice_id, $purchase_id, $shopper_id, $time, $session_id, $catalog_id, $product_id, $sku,$price, $quantity, $desciption, $cookie_id) = @row;
	# convert time to timestamp
	my $ts = UnixDate($time, "%s");
	push @{$user_order_map{$shopper_id}->{$ts}} , \@row;
}

close INPUT_FILE;

while(my($shopper_id, $order_info) = each %user_order_map){
	# sort $order_info by increasing timestamp
	my @sort_ts = sort keys %$order_info;
	my $json_data = {"uid" => $shopper_id, "session" => []};
	foreach my $i(0 .. $#sort_ts){
		my $tmp_ts = $sort_ts[$i];
		my $tmp_orders = $order_info->{$tmp_ts};
		$json_data->[$i] = {"ts" => $tmp_ts, "orders" => $tmp_orders};
	}
	my $json_str = encode_json($json_data);
	print OUTPUT_FILE $json_str . "\n";
}

close OUTPUT_FILE;

