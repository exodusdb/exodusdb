<?php

/* no longer required since we call upload.php directly now.

put the following text in .htaccess in the folder where
upload.dll and upload.php are. Then on apache requests to
upload.dll will be handled by upload.php.

<IfModule mod_rewrite.c>
    RewriteEngine On
    RewriteRule ^upload\.dll$ upload.php
</IfModule>
*/

/*
mkdir /var/www/html/exodus/images
chown www-data:www-data /var/www/html/exodus/images
chmod g+s /var/www/html/exodus/images
*/

//debug in php like this

	//1. change $debugging date below to some date in the future
	//2. tail -f /var/log/apache2/error.log
	//3. also see .2 and .3 files are left in interface directory

	//debugging leaves the .2 and .3 files undeleted and puts messages in 
	//$debugging = true;
	//$debugging = false;
	$debugging=time()<=(strtotime("2020-01-16")+86400);
	//$debugging=$debugging || is_file($gdatalocation . 'debug.php');
	if ($debugging)
		error_reporting(E_ALL);

function debug($msg) {
	global $debugging;
	if ($debugging) {
		$msg=str_replace("\xdf\xbe","^",$msg);
		$msg=str_replace("\xdf\xbd","]",$msg);
		error_log("=\"$msg\"" ,0);
	}
}
//10 minutes max upload time and 100Mb file size
ini_set('upload_max_filesize', '100M');
ini_set('post_max_size', '100M');
ini_set('max_input_time', 600);
ini_set('max_execution_time', 600);

$redirectpage=$_REQUEST ['redirectpage'];
$pathdata    =$_REQUEST ["pathdata"];
$filename    =$_REQUEST ["filename"];
$localdir    =__DIR__;
$origfilename=$_FILES   ['filedata']['name'];
$filesize    =$_FILES   ['filedata']['size'];
$tmpfilename =$_FILES   ['filedata']['tmp_name'];
$hostdomain  =$_SERVER  ['HTTP_HOST'];
$https       =$_SERVER  ['HTTPS'];
$upload_time =time() - $_SERVER['REQUEST_TIME'];

//=D:/hosts/testimages/TEST/upload/jobs/5400/7.EXODUS_signature.jpg", referer: https://localhost/3/exodus/upload.htm

//$target_path = preg_replace('!exodus/dll!','images',$localdir) . "/" . basename( $filename);
//$target_path = preg_replace('!exodus/dll!','images',$localdir) . "/" . $filename;
//$target_path = $localdir . "/" . $filename;

//get physical path that matches virtual upload path. usually /images -> d:/exodus/images (from apache conf file)
//$realpath = realpath($pathdata .'/'); //realpath() does not work with virtual directories
//WARNING apache_lookup_uri() requires php to be installed as an apache module
//if we do not have the apache_lookup_uri function then php will not be able
// to determine the real image directory location from the apache virtual directory configuration.
// and will be limited to a fixed location relative to exodus.net, or require manual configuration
//https://www.php.net/manual/en/function.apache-lookup-uri.php
//https://hotexamples.com/examples/-/-/apache_lookup_uri/php-apache_lookup_uri-function-examples.html
$realpath = apache_lookup_uri($pathdata . '/')->filename;
//debug(print_r(apache_lookup_uri($pathdata),true));

$target_path = $realpath .  '/' . $filename;
$target_path = str_replace("\\","/",$target_path);

// localdir=D:\\hosts\\test\\exodus.net\\exodus\\dll"
// target_path=debug("---------- upload.php ----------");
debug("upload.php localdir    =$localdir");
debug("upload.php pathdata    =$pathdata");
debug("upload.php realpath    =$realpath");
debug("upload.php filename    =$filename");
debug("upload.php origfilename=$origfilename");
debug("upload.php tmpfilename =$tmpfilename");
debug("upload.php target_path =$target_path");

//FOR EXAMPLE
//localdir     = D:\\exodus\\exodus.net\\exodus\\dll
//pathdata     = /images
//realpath     = d:/exodus/images/
//filename     = DEVDTEST\\upload\\jobs\\mo5070\\4.gg.png
//origfilename = gg.png
//tmpfilename  = C:\\Windows\\Temp\\php1FD4.tmp
//target_path  = D:/exodus/images/DEVDTEST/upload/jobs/mo5070/4.gg.png

if ($https=="on") $protocol="https"; else $protocol="http";
$redirect_url=$protocol . "://$hostdomain" . $redirectpage . "?FileName=$filename" . "&FileSize=$filesize" . "&TimeElapsed=$upload_time";

$target_dir=pathinfo($target_path,PATHINFO_DIRNAME);
debug("upload.php target_dir  =$target_dir");
if (! is_dir($target_dir) && ! mkdir($target_dir,0774,true)) {
    echo "There was an error creating dir '$target_dir' Get technical support.<br />";
};


if (move_uploaded_file($tmpfilename, $target_path)) {
    //echo "The file ".  basename( $_FILES['filedata']['name'])." has been uploaded";
	//ob_start();
	chmod($target_path,0666);
    header('Location: '.$redirect_url,true,302);
    //ob_end_flush();
    //die();
	//echo "redirect_url=" . $redirect_url . "<br />";
	exit();
} else {
    echo "There was an error moving the file. Get technical support.<br />";
}
echo "hostdomain="   . $hostdomain   . "<br />";
echo "https="        . $https        . "<br />";
echo "redirectpage=" . $redirectpage . "<br />";
echo "pathdata    =" . $pathdata     . "<br />";
echo "localdir    =" . $localdir     . "<br />";
echo "filename    =" . $filename     . "<br />";
echo "origfilename=" . $origfilename . "<br />";
echo "filesize    =" . $filesize     . "<br />";
echo "tmpfilename =" . $tmpfilename  . "<br />";
echo "<br />";
echo "target_path =" . $target_path  . "<br />";
echo "target_dir  =" . $target_dir   . "<br />";
echo "umask()  r  =" . umask()       . "<br />";
echo "redirect_url=" . $redirect_url . "<br />";

/*
There was an error creating dir '/var/www/html/exodus/images/exodus/UPLOAD/SCHEDULES' Get technical support.
There was an error moving the file. Get technical support.
hostdomain=exodus.hosts.neosys.com:44325
https=on
redirectpage=//exodus/3/exodus/upload2.htm?sourcefilename=adminredball.jpg&targetpath=/exodus/images&autofit=true&remainder=
pathdata =/exodus/images
localdir =/var/www/html/exodus/exodus/dll
filename =exodus\UPLOAD\SCHEDULES\MO1792.jpg
origfilename=adminredball.jpg
filesize =40160
tmpfilename =/tmp/phpJd7J2U

target_path =/var/www/html/exodus/images/exodus/UPLOAD/SCHEDULES/MO1792.jpg
target_dir =/var/www/html/exodus/images/exodus/UPLOAD/SCHEDULES
redirect_url=https://exodus.hosts.neosys.com:44325//exodus/3/exodus/upload2.htm?sourcefilename=adminredball.jpg&targetpath=/exodus/images&autofit=true&remainder=?FileName=exodus\UPLOAD\SCHEDULES\MO1792.jpg&FileSize=40160&TimeElapsed=0
*/

?>
