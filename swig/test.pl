#!/usr/bin/perl
use exodus;

#currently exodus function arguments must be exodus variables
#and cannot be given as normal quoted string constants!!!
#that will be fixed soon
#apologies to perl monks, this is my 1st perl prog

$xo=new exodus::var();

$filename=new exodus::var("tempfileperl");
$options=new exodus::var("");

if ($xo->createfile($filename,$options)) {
	print "$filename file created\n";
}else{
	print "$filename file not created. maybe already exists. try deletefile $filename\n";
}

#write 10 records
for ($ii=1;$ii<=100;++$ii) {
	$record=$xo->date();
	$id=new exodus::var($ii);
	print $id," ";
	$filename->write($filename,$id);
}
print "\n";

if ($filename->deletefile()) {
	print "$filename file deleted\n";
}
$D=new exodus::var("D");
$MT=new exodus::var("MT");

print $D,"\n";
print $MT,"\n";

print $xo->date()->oconv($D), "\n";

print $xo->time()->oconv($MT),"\n";

