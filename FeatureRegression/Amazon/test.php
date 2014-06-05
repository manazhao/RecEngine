<?php


$a = array();
$other_groups = array(1,3,5);
foreach($other_groups as $grp){
	$a[$grp] = 0;
}

$a[3] = 10;

print_r($a);
?>
