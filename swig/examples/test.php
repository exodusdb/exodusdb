<?php
echo "--- Testing ---\n";

//include("backtrace.php");
include("exo.php");

//glossary:
//file means database table
//record means table row

//some exodus compatible mvariables
//at the moment, all exodus function arguments must be exodus mvar's
//that wil be changed soon
$exo=new mvar();
$options=new mvar("");

if (!$exo->connect($options)) {
	echo "no default database connection\n";
} else {
	$filename=new mvar("tempfile");

	//exodus mvariables can convert to string automatically
	echo "filename is ", $filename->quote(),"\n";

	//create a file
	if ($exo->createfile($filename,$options))
		echo "created file ",$filename,"\n";
 	else
		echo "not created $filename\n";

	//create some records
	$exo->begintrans();
	$record=new mvar("X");
	$record=$record->str(1000);
	for ($ii=1;$ii<=100;$ii++) { 
		$key=new mvar($ii);
//		$record=new mvar($ii);
		if (!$record->write($filename,$key))
			break;
		echo $key," ";
}

	echo "\n";
	$exo->committrans();

	//see what db files are available
	echo "list of files:\n";
	$listoffiles=$exo->listfiles();
	echo $listoffiles,"\n";
	echo $listoffiles->extract(1),"\n";

	//delete a file
	if ($exo->deletefile($filename))
		echo "deleted file ",$filename,"\n";
	else
		echo "not deleted ",$filename,"\n";

}

//output current date and time using oconv
$dateconv=new mvar("D");
$timeconv=new mvar("MT");
echo "Pick date: ", $exo->date(), " ", $exo->date()->oconv($dateconv), "\n";
echo "Pick time: ", $exo->time(), " ", $exo->time()->oconv($timeconv), "\n";

echo "Finished\n";
?>
