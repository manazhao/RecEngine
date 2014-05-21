<?php

# $archive = new PharData("/media/020C57B30C57A109/AmazonCrawledData/AmazonBookData/Books_Reviews_part9.tar.gz");
$tar_gz_file = 'phar:///media/020C57B30C57A109/AmazonCrawledData/AmazonBookData/Books_Reviews_part6.tar.gz';
$list = array("/000724634X/1.html.gz","/000724634X/2.html.gz");
foreach ($list as $file){
	$contents = file_get_contents("$tar_gz_file/$file","r");
	echo $contents . "\n";
}




?>



