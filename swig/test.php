<?php
include("backtrace.php");
include("php/exodus.php");

//file=database table
//record=table row

//some exodus compatible variables
//at the moment, all exodus function arguments must be exodus var's
//that wil be changed soon
$filename=new c_var("tempfile");
$options=new c_var("");
$temp=new c_var();

//exodus variables can convert to string automatically
echo "filename is ->", $filename,"<-\n";

//delete a file
if ($filename->deletefile())
	echo "deleted file $filename\n";
else
	echo "not deleted $filename\n";

//crceate a file
if ($temp->createfile($filename,$options))
	echo "created file $filename\n";
else
	echo "not created $filename\n";

//create some records
for ($ii=1;$ii<=10;$ii++) {
	$record=new c_var($ii);
	$key=new c_var($ii);
	$ok=$record->write($filename,$key);
	echo "write $record on $filename,$key result:$ok\n";
}

//see what files are available
echo "list of files:\n";
echo $temp->listfiles(),"\n";

//output current date and time using oconv
$dateconv=new c_var("D");
$timeconv=new c_var("MT");
echo "Date: ", $temp->date()->oconv($dateconv), " Time:", $temp->time()->oconv($timeconv),"\n";

echo "\nAll Done!\n";
?>
