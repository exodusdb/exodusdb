<?php
echo "Started\n";

//include("backtrace.php");
include("php/exo.php");

//file means database table
//record means table row

//some exodus compatible variables
//at the moment, all exodus function arguments must be exodus var's
//that wil be changed soon
$xo=new c_var();

$filename=new c_var("tempfile");
$options=new c_var("");

//exodus variables can convert to string automatically
echo "filename is ->", $filename,"<-\n";

//delete a file
if ($filename->deletefile())
	echo "deleted file $filename\n";
else
	echo "not deleted $filename\n";

//crceate a file
if ($xo->createfile($filename,$options))
	echo "created file $filename\n";
else
	echo "not created $filename\n";

//create some records
for ($ii=1;$ii<=100;$ii++) {
	$key=new c_var($ii);
	$record=new c_var($ii);
	$ok=$record->write($filename,$key);
	echo "$key ";
}
echo "\n";

//see what files are available
echo "list of files:\n";
echo $xo->listfiles(),"\n";

//output current date and time using oconv
$dateconv=new c_var("D");
$timeconv=new c_var("MT");
echo "Pick date: ", $xo->date(), " ", $xo->date()->oconv($dateconv), "\n";
echo "Pick time: ", $xo->time(), " ", $xo->time()->oconv($timeconv), "\n";

echo "Finished\n";
?>
