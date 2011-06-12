#!/usr/bin/perl
use exodus;
$filename=new exodus::var("tempfileperl");
$options=new exodus::var("");
#print $x->date()->tostring(),"\n";
$x=new exodus::var("");
if ($x->createfile($filename,$options)) {
	print "file created\n";
}else{
	print "file not created\n";
}
