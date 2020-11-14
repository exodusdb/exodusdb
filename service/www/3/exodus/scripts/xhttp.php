<?php

//errors cannot be echoed to xmlhttp so force them off in case set in php.ini
ini_set('display_errors', 'Off');
//php.ini
//display_errors = off

ignore_user_abort(true);

//enable services to read/write/delete any files we create here without any further configuration
umask(0);

//debug in php like this

//1. change $debugging date below to some date in the future
//2. tail -f /var/log/apache2/error.log
//3. also see .2 and .3 files are left in interface directory

//debugging leaves the .2 and .3 files undeleted and puts messages in 
//$debugging = true;
//$debugging = false;
//debugging will automatically be turned off so you dont accidentally
//leave debugging on with the consequent excessive logging
$debugging = time() <= (strtotime("2020-11-30") + 86400);
//moved further down
//$debugging=$debugging || is_file($gdatalocation . 'debug.php');
$gdebug_data = true;
$gdebug_xml = true;

$gwindows = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN';
debug("PHP_OS        : " . PHP_OS);
debug("WINDOWS       : " . $gwindows);

$gslash = $gwindows ? '\\' : '/';

//constants

$gautostartdatabase = true; //NET.CFG must also contain AUTOSTART=YES
$gsecondstowaitforreceipt = 15;
$gsecondstowaitforstart = 30;

$cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER';
$databasestoppedresponse = "The database service is stopped.\r\rPlease try again later or ask your\rtechnical support staff to start it.";
$norequestresponse = 'ERROR: NO REQUEST';
$nodataresponse = 'ERROR: NO DATA';
$invaliddatapathresponse = 'ERROR: CANNOT WRITE TO ';

//defaults

//default timeout is 10 minutes (NB BACKEND timeout (in GIVEWAY) is hard coded to 10 mins?)
//actually the client sends the timeout (10 mins)
$defaulttimeoutmins = 10;
$timeout_ms = $defaulttimeoutmins * 60 * 1000;

// session

session_start();

//NEOSYS root path should point to the folder containing exodus, exodus.net, data, images etc
// eg D:\\hosts\\test or /var/www/exodus?
//$exodusrootpath = $_SERVER['DOCUMENT_ROOT'];
// __DIR__ = D:\\hosts\\test\\exodus.net\\3\\exodus\\scripts

//on linux
$exodusrootpath = preg_replace('!/[^/]+/3/exodus/scripts!', '/', __DIR__);
//$exodusrootpath = preg_replace('!/exodus.net/!', '/', $exodusrootpath);

//on windows
$exodusrootpath = str_replace('3\\exodus\\scripts', '', $exodusrootpath);
$exodusrootpath = str_replace('\\exodus.net\\', '\\', $exodusrootpath);

if ($gwindows)
	$exodusrootpath = str_replace('/', '\\', $exodusrootpath);

debug("exodusrootpath : $exodusrootpath");

// /var/www/html/exodus2//data/default.vol
// D:\hosts\test\exodus.net\3\exodus\scriptsNEOSYS/ADAGENCY.VOL
$gdatalocation = ($exodusrootpath . 'data' . $gslash);

if (!file_exists($gdatalocation)) {
	debug("ERROR: Datalocation does not exist:" . $gdatalocation);
	exit("ERROR: Datalocation does not exist:" . $gdatalocation);
	$debugging = false;
}

$debugging = $debugging || is_file($gdatalocation . 'debug.php');

debug("exodusdatapath :" . $gdatalocation);

//not used atm
if (($exodusrootpath . "UNICODE.INI"))
	$unicode = -1;
else
	$unicode = 0;

//get and cleanup hostname
$localhostname = $_SERVER['SERVER_NAME'];
//remove space . " ' characters
$localhostname = str_replace(array(".", ".", " "), "_", $localhostname);
//and take first and last four characters if longer than 8 characters. why?
if (strlen($localhostname) > 8)
	$localhostname = substr($localhostname, 0, 4) . substr($localhostname, -4);

if (isset($_SERVER["REMOTE_ADDR"]))
	$remoteaddr = $_SERVER['REMOTE_ADDR'];
else
	$remoteaddr = "";

$remotehost = "";

if (isset($_SERVER["HTTPS"]))
	$https = $_SERVER['HTTPS'];
else
	$https = "";

//request

//client delivers a request in xml format
//$xml = simplexml_load_string($HTTP_RAW_POST_DATA);
$raw_xml = file_get_contents("php://input");
if ($gdebug_xml)
	debug("POST    :" . $raw_xml);

$xml = simplexml_load_string($raw_xml);
debug("REQUEST :" . $xml->request);
if ($xml->token)
	debug("TOKEN   :" . $xml->token);
if ($xml->database)
	debug("DATABASE:" . $xml->base);
if ($gdebug_data && strlen($xml->data))
	debug("DATA_IN :" . $xml->data);

//request contains token, request, database and data
$token = unescape($xml->token);
$request = unescape($xml->request);
$database = unescape($xml->dataset); //probably come as DATASET not DATABASE
$data_in = unescape($xml->data);

//convert any crlf to just cr
//exodus uses \r which is cr because \n and lf are messed about as either 0d0a or just 0a
$request = str_replace("\n", "\r", $request);
$token = str_replace("\n", "\r", $token);

$requests = explode("\r", $request . "\r\r\r\r\r\r\r\r");

if ($token) {

	//if login then save new details regardless of success or not
	if ($requests[0] == 'LOGIN') {

		$request = $requests[0];
		$username = $requests[1];
		$password = $requests[2];
		$database = $requests[3];
		$authno = $requests[4];
		$system = $requests[5];

		$requests[2] = '';
		error_log(implode(' ', array_slice($requests, 0, 6)));

		$_SESSION[$token . '_username'] = $username;
		$_SESSION[$token . '_password'] = $password;
		$_SESSION[$token . '_database'] = $database;
		$_SESSION[$token . '_system'] = $system;
		$_SESSION[$token . '_timeout'] = $timeout_ms;
	} else {

		//try and get the username, password and database from the session
		if (array_key_exists($token . '_username', $_SESSION)) {
			$username = $_SESSION[$token . '_username'];
			$password = $_SESSION[$token . '_password'];
			$database = $_SESSION[$token . '_database'];
			$system = $_SESSION[$token . '_system'];
			$timeout_ms = $_SESSION[$token . '_timeout'];
		} else {
			$username = '';
			$password = '';
			$database = '';
			$system = '';
			$timeout_ms = '';
		}
		debug("USERNAME:$username");
	}

	//if logout then clear session username, password and database from the session
	if ($requests[0] == 'LOGOUT') {
		$_SESSION[$token . '_username'] = "";
		$_SESSION[$token . '_password'] = "";
		$_SESSION[$token . '_database'] = "";
		$_SESSION[$token . '_system'] = "";
		$_SESSION[$token . '_timeout'] = "";
	}

	//seconds for script timeout is our timeout plus 60 seconds
	//TODO Server.ScriptTimeout = $timeout_ms / 1000 + 60;
	//echo "timeout " . $timeout_ms;
	if (!$timeout_ms)
		$timeout_ms = $defaulttimeoutmins * 60 * 1000;
	debug("timeout_ms=" . $timeout_ms);
	set_time_limit($timeout_ms / 1000 + 60);

	//append trailing '/' to databasedir if necessary
	$databasedir = $database . $gslash;

	$globalserverfilename = $gdatalocation . $databasedir . $database . '.SVR';
}

/*
WESTERN UNITED STATES -> CP437
administrator@adlineb.hosts.neosys.com	437
administrator@adlinem.hosts.neosys.com	437
administrator@tdaddb.hosts.neosys.com	437
administrator@mediaone.hosts.neosys.com	437
administrator@ums.hosts.neosys.com	437
administrator@hosts2.neosys.com		437

ARABIC -> CP1256
administrator@adlinec.hosts.neosys.com	720
administrator@adlined.hosts.neosys.com	720
administrator@adlined2.hosts.neosys.com	720
administrator@adlinek.hosts.neosys.com	720
administrator@amc.hosts.neosys.com	720
administrator@bates.hosts.neosys.com	720
administrator@asha.hosts.neosys.com	720
administrator@win3.neosys.com		720

GREEK -> CP1253
administrator@ptcy.hosts.neosys.com	737

WESTERN UNITED KINGDOM -> CP850 (server should be configured as US)
administrator@win10.neosys.com		850
administrator@win10b.neosys.com		850
*/
$gweb_codepage = 'UTF-8';
$gdatabase_codepage = '';
if ($database) {

	$database_config_filename = $gdatalocation . $databasedir . 'DATA.CFG';
	$database_config_filename2 = $gdatalocation . 'DATA.CFG';
	//$database_config=file_get_contents($database_config_filename);
	$database_config = ReadAll($database_config_filename);
	if (!$database_config) {
		$database_config = ReadAll($database_config_filename2);
	}
	//split lines either dos or unix line separators
	$database_config = explode("\n", preg_replace('~\r\n?~', "\n", $database_config));
	//get first line only
	$gdatabase_codepage = $database_config[0];
	debug('CODEPAGE:' . "'" . $gdatabase_codepage . "'");

	//convert DOS/Windows MODE codepage codes to iconv codepage codes
	if ($gdatabase_codepage == '720')
		$gdatabase_codepage = 'CP1256'; //Arabic

	else if ($gdatabase_codepage == '737')
		$gdatabase_codepage = 'CP1253'; //Greek

	else if ($gdatabase_codepage == '850')
		$gdatabase_codepage = 'CP850'; //US English

	else if ($gdatabase_codepage == '437')
		$gdatabase_codepage = 'CP437'; //UK English

	else if ($gdatabase_codepage == 'UTF-8' || ! $gwindows)
		$gdatabase_codepage = 'UTF-8'; //probably exodus/c++

	else if (!$gdatabase_codepage) {
		//no codepage configured then do no conversion and assume that database code page is UTF-8, that of the web ui
		exit($database_config_filename . " and\n" . $database_config_filename2 . "\nare missing or empty\nLine 1 should be 850/437/720/737/UTF-8 for US/UK/Arabic/Greek/UTF-8 codepages");
	} //else assume that the codepage code is understood by php iconv
}
// attempt(s)

//using "loop" to avoid use of function/subroutine
//break = failures (or at end, success)
//continue = possible 2nd attempt after requesting database to start
//actually php 5.3 has goto commands which might be used instead and avoid 1 indent
while (1) {

	//MUST be sent before anything else
	header("Content-Type:text/xml; charset=utf-8");
	//? like xhttp.asp let client guess encoding because NEOSYS client data might not be in UTF-8 format eg DIO (Dior) in TEST database
	//header ("Content-Type:text/xml");

	//failure
	$data_out = '';
	$response = '';
	$result = 0;
	$linkfilename = '';

	//keepalive
	if ($requests[0] == 'KEEPALIVE' || $requests[0] == 'LOOPBACK') {

		debug($requests[0]);

		$response = "OK";
		$result = 1;
		break;
	}

	//fail if could not locate the database
	if (!$exodusrootpath) {
		$response = $cannotfinddatabaseresponse;
		break;
	}

	//fail if no request
	if ($request == '') {
		$response = $norequestresponse;
		break;
	}

	//special request that doesnt require anything but a request and exodusrootpath
	if ($requests[0] == 'GETDATASETS') {
		$data_out = getdatabases($exodusrootpath, $requests[1]);
		$result = 1;
		break;
	}

	//fail regardless of database if GLOBAL.END file is present
	//TODO check other locations for GLOBAL.END (multiinstallation and database)
	$proglocation = $exodusrootpath . '/exodus/';
	if (is_file($proglocation . 'global.end')) {
		$response = $databasestoppedresponse;
		break;
	}

	//check username is present - session may have timed out
	if (!$username) {
		$response = 'Error: Please login - Session not established or timed out';
		break;
	}

	//check password is present
	if (!$password) {
		$response = 'Error: Password parameter is missing';
		break;
	}

	//check database is present
	if (!$database) {
		$response = 'Error: Dataset parameter is missing';
		break;
	}

	//check details and token
	if ($token && $token != $database . '*' . $username . '*') {
		$response = 'token is ' . $token . ' but should be ' . $database . '*' . $username . '*';
		break;
	}

	//make a random ~9999999.x file name for the request
	do {
		//~*.htm files are not backed up in FILEMAN
		$linkfilename = '~' . exodusrnd(9999999, 1000000);
		$linkfilename0 = $linkfilename;
		$linkfilename = $gdatalocation . $databasedir . $linkfilename;
	} while (glob($linkfilename . '.*'));

	debug("DATA_IN : $data_in");

	//write data (if any) before request so that there is no sharing violation with server reader
	if ($data_in) {

		//tf.Write(escape($data))
		//allow field marks to pass through unicode conversion untouched to the backend as escaped characters
		//only really need the six characters FA-FF
		if ($gwindows)
			$data_in = escapefms($data_in);
		//unicode to codepage conversion used to take place here in win/IIS
		//if (!file_put_contents($linkfilename . '.2', $data_in)) {
		if (!WriteAll($linkfilename . '.2', $data_in)) {
			$response = $invaliddatapathresponse . ' "' . $linkfilename . '.2" ';
			break;
		}
		//ensure server can write it for returned data
		chmod($linkfilename . '.2', 0775);
	}

	//more info in request
	if (!$remoteaddr)
		$remoteaddr = '';
	if (!$remotehost)
		$remotehost = $remoteaddr; //if no hostname then use ip
	if (!$https)
		$https = '';
	if (isset($_SERVER["HTTP_USER_AGENT"]))
		$agent = $_SERVER['HTTP_USER_AGENT'];
	else
		$agent = "";

	//prepare a request file to "send" to the server as a polled file
	$fullrequest = '';
	$fullrequest .= $linkfilename . '.1' . "\r";
	$fullrequest .= 'VERSION 3' . "\r";
	$fullrequest .= '7' . "\r"; //npre-database connection info related fields
	$fullrequest .= $remoteaddr . "\r";
	$fullrequest .= $remotehost . "\r";
	$fullrequest .= $https . "\r";
	$fullrequest .= session_id() . "\r"; //php
	$fullrequest .= $agent . "\r"; //browser info

	$fullrequest .= $database . "\r";
	$fullrequest .= $username . "\r";
	$fullrequest .= $password . "\r";

	//allow field marks to pass through unicode conversion untouched
	if ($gwindows)
		$request = escapefms($request);
	//$fullrequest .= $temp
	$fullrequest .= $request;

	//write cmd (not as a .1 file otherwise the listener may
	//try to read it before it is ready
	//codepage used to happen here conversion happens here in win/iis
	//otherwise abort

	//if (!file_put_contents($linkfilename . '.1$', $fullrequest)) {
	if (!WriteAll($linkfilename . '.1$', $fullrequest)) {
		$response = $invaliddatapathresponse . ' "' . $linkfilename . '.1$" ';
		break;
	}
	//ensure server can delete it after renaming/processing it
	chmod($linkfilename . '.1$', 0775);

	//now that everything is in place (request and data files)
	//rename the request file to end in .1
	//so that server/listener pick it up
	//otherwise abort
	if (!rename($linkfilename . '.1$', $linkfilename . '.1')) {
		$response = 'Cannot rename "' . $linkfilename . ".1$\"\rto " . $linkfilename . '.1"';
		break;
	}

	$runrequestfilename = '';

	debug("REQUEST_FILE : $linkfilename.1");

	//wait briefly for 10 seconds for the request to disappear
	//sleeping between checks every 10 ms
	$waituntil = time() + $gsecondstowaitforreceipt;
	while (is_file($linkfilename . '.1')) {

		//TODO increase sleep if not fast result
		usleep(10000);

		// abort if request file has not been taken within 10 seconds
		//TODO start the database and try again
		//TODO really should rename to take control back of request file to ensure it isnt processed
		if (time() >= $waituntil) {

			//AUTOSTART
			if ($gautostartdatabase) {

				//only attempt to autostart once
				$gautostartdatabase = false;

				//no autostart
				$config = ReadAll("{$exodusrootpath}NEOSYS//NET.CFG");
				if (strpos($config, "AUTOSTART=YES") !== false) {

					//extend waiting time to allow database to start (by 30 seconds?)
					$waituntil = time() + $gsecondstowaitforreceipt * 2;

					//build the connection string
					$connectstring = "$database\r\n$username\r\n\r\n\r\n$system\r\n$linkfilename\r\n";

					//where to write the run request
					//D:\exodus\NEOSYS\~9337586.RUN
					$runrequestfilename = $exodusrootpath . "NEOSYS" . $gslash . $linkfilename0 . ".RUN";

					if (!WriteAll($runrequestfilename, $connectstring)) {
						$response = "Could not write $runrequestfilename";
						break 2;
					}


					//$cmd = "C:\Windows\System32\CMD.EXE /C START NEOSYS.JS /system $system /database $database 2>&1";
					//debug($cmd);
					//$handle = popen($cmd, 'r');
					//debug("'$handle'; " . gettype($handle) . "\n");
					//debug(fread($handle, 2096));
					//pclose($handle);

					//wait for start and disappearance of request
					continue;
				}
			}

			if (is_file($runrequestfilename))
				unlink($runrequestfilename);

			$response = 'Error: No response in ' . $gsecondstowaitforreceipt . ' seconds from database server at ' . $linkfilename . '.1';
			break 2;
		}
		clearstatcache();
	}

	//wait patiently for 10? minutes for the response to appear otherwise quit
	//sleep between checkds every 10 ms
	$waituntil = time() + $timeout_ms / 1000;

	$nskips = 0;
	while (!is_file($linkfilename . '.3')) {

		//check for disconnected only once every 2 seconds (50 loops) since sends something client?
		$nskips++;
		if ($nskips > 50) {
			//debug("connection checking $linkfilename.3");
			$nskips = 0;
			//Echo chr(0);
			echo "\n";
			//after this, it is not possible to send any headers
			ob_flush();
			flush();
		}

		if (connection_aborted()) {
			debug("connection aborted");
			//var elapsedseconds = Math.floor((new Date() - timestarted)/10)/100
			//this.response = 'Error: Client disconnected after ' + elapsedseconds + ' seconds in NEOSYS xhttp.asp ' + linkfilename
			//exodusoswrite(this.response,linkfilename + '.5',unicode)//just so we can see interrupted requests on the server more easily
			//dbready(dbwaitingwindow)
			//this.request = ''
			//return (0)

			$response = 'Error: Client disconnected after n seconds in NEOSYS xhttp.php ' . $linkfilename . ".5";
			error_log($response);

			//file_put_contents($linkfilename . '.5', $response);
			WriteAll($linkfilename . '.5', $response);

			//ensure server can write it for returned data
			chmod($linkfilename . '.5', 0775);

			break 2;
		}

		//sleep for 10ms before checking for response again
		//TODO make it sleep longer the long the delay
		// so response is fast if the server is fast but saves cpu if not
		//(after waiting 10 seconds it is pointless to check every 10ms)
		usleep(50000); //10,000 microseconds = 5 ms

		//abort if no response within 10? minutes
		if (time() >= $waituntil) {
			$response = 'Error: No response in ' . $timeout_ms / 1000 . ' seconds from database server at ' . $linkfilename . ".3";
			break 2;
		}
	}

	//read response in .3 file

	//response determines true or false
	//$response=file_get_contents($linkfilename . '.3');
	$response = ReadAll($linkfilename . '.3');
	if (!$response) {
		$response = 'Error: Cannot read response file ' . $linkfilename . ".3";
		break;
	}
	if ($gwindows)
		$response = unescapefms($response);

	debug("RESPONSE_FILE : $linkfilename.3");
	debug("RESPONSE      : $response");

	//read the data .2 file or empty data if not
	//can return data even if not "OK")
	//$data_out=file_get_contents($linkfilename . '.2');
	$data_out = ReadAll($linkfilename . '.2');
	if (!$data_out)
		$data_out = '';
	if ($gwindows)
		$data_out = unescapefms($data_out);

	//using a "loop" as a way to jump forward to abort or succeed anywhere or at end (break statements)
	//only looping if starting database
	break;
} //end of attempt "loop"

//result

if (explode(' ', $response)[0] == 'OK')
	$result = 1;
else
	$result = 0;

//convert data path to web path
if ($requests[0] == "EXECUTE") {
	$leadin = substr($data_out, 0, strlen($exodusrootpath)) . "/";
	debug("EXECUTE LEADIN " . $leadin);
	if (substr($data_out, 0, strlen($leadin)) == $exodusrootpath . "/") {
		//$data_out = "../" . substr($data_out,strlen($leadin));
		$data_out = str_replace($leadin, "../", $data_out);
	}
}

//clean up

//normally .1 will already have been consumed
//(renamed, read and then deleted)
//by the exodus database and cannot be deleted here
if (is_file($linkfilename . '.1'))
	unlink($linkfilename . '.1');

$deletefailed = false;

//dont delete the .2 and .3 data and response files if debugging
if (!$debugging) {

	//often there is no .2 data file returned and even then usually we dont have privileges to delete it
	if (is_file($linkfilename . '.2') && !unlink($linkfilename . '.2'))
		$deletefailed = true;

	//sometimes we dont have the privileges to delete the response file
	if (is_file($linkfilename . '.3') && !unlink($linkfilename . '.3'))
		$deletefailed = true;
}

//if we cannot delete one or other of the .2 data and .3 response files
//then creating a .4 file instructs the db server to do it
if ($deletefailed)
	//file_put_contents($linkfilename . '.4', '');
	WriteAll($linkfilename . '.4', '');

//output

//data and response needs to be encoded so that it is valid xml
//eg % become %25 etc

//encode xml related characters since message to client is in xml format
//will be decoded in client after unpacking xml message

$xmltext = "<root>";
//$xmltext .= "<data>" . rawurlencode($data_out) . "</data>";
//$xmltext .= "<response>" . rawurlencode($response) . "</response>";
//$xmltext .= "<result>" . rawurlencode($result) . "</result>";
$xmltext .= "<data>" . escape($data_out) . "</data>";
$xmltext .= "<response>" . escape($response) . "</response>";
$xmltext .= "<result>" . escape($result) . "</result>";
$xmltext .= "</root>";

//Response.Expires = -1000
//Response.Buffer = 0

//headers must be sent before ob_flush, so the following line is moved up to before waiting for db response
//if ob_flush done beforehand, then headers will be IGNORED ... which causes FAILURE IN THE CLIENT (not xml)
//header ("Content-Type:text/xml; charset=utf-8");

//file_put_contents($linkfilename . '.xmltextout', $xmltext);
//output the xml to the browser
print($xmltext);

global $gdebug_data;
global $gdebug_xml;
debug("RESPONSE:$response");
if ($gdebug_data && $data_out)
	debug("DATA_OUT:$data_out");
//	if (!$result)
//		debug("RESULT  :$result");
if ($gdebug_xml)
	debug("XMLOUT  :$xmltext");

/// finished

function exodusrnd($max, $min)
{
	//return a random integer between 0 (or min) and max-1
	if (!$min)
		$min = 0;
	$max = $max - 1;
	//return (floor($min + rand(0,1) * ($max - $min + 1)))
	return mt_rand($min, $max);
};

function debug($msg)
{
	global $debugging;
	if ($debugging) {
		$msg = str_replace("\xdf\xbe", "^", $msg);
		$msg = str_replace("\xdf\xbd", "]", $msg);
		error_log("=\"$msg\"", 0);
	}
}

function getdatabases($exodusrootpath, $systemcode)
{

	//will look in /var/www/exodus/ for adagency.vol or accounts.vol

	//return an array of available database codes and names
	//or an empty array

	global $response;

	if (!$systemcode)
		$systemcode = 'default';

	//get an array of databases
	//$gdatalocation = $exodusrootpath . "/data/";
	global $gdatalocation;
	//$volfilename=$gdatalocation . $systemcode . '.vol';
	$volfilename = $exodusrootpath . "NEOSYS/" . $systemcode . '.VOL';
	if (!is_file($volfilename)) {
		$volfilename2 = $exodusrootpath . 'data/default.vol';
		if (is_file($volfilename2)) {
			$volfilename = $volfilename2;
		}
		else {
			$response = "Cannot see vol file $volfilename or $volfilename2";
			return "";
		}
	}
	debug("volfilename:" . $volfilename);
	//$databases=file_get_contents($volfilename);
	$databases = ReadAll($volfilename);
	if (!$databases) {
		$response = "Vol file is empty or cannot be read $volfilename";
		return "";
	}
	debug("getdatabases text:" . $databases);

	$linesep = "\n";
	$dbsep = '*';
	$dbnamecodesep = ",";

	$databases = str_replace("\r", $linesep, $databases);

	//the first line contains the pairs of dbname sm dbcode vm ...
	$databases = explode($linesep, $databases)[0];

	//first word of first line is an unwanted code
	$databases = join(" ", array_slice(explode(" ", $databases), 1));

	//split the databases into an array
	$databases = explode($dbsep, $databases);
	//debug("getdatabases list1:".$databases);

	//split the array into an xml list of db name and code
	$xmltext = "<records>\r";
	foreach ($databases as $database) {
		//$database=explode($sm,$database.$sm);
		$database = explode($dbnamecodesep, $database . $dbnamecodesep);
		$databasename = $database[0];
		$databasecode = $database[1];

		//skip databases that dont exist. requests will be written there
		if (!is_dir($gdatalocation . '/' . $databasecode))
			continue;

		$xmltext .= "<record>\r";
		$xmltext .= "<name>$databasename ($databasecode)</name>\r";
		$xmltext .= "<code>$databasecode</code>\r";
		$xmltext .= "</record>\r";
	}
	$xmltext .= "</records>\r";

	$response = "OK";

	return $xmltext;
}

//outbound to gui to allow wrapping in xml for transport
function escape($str)
{

	//cannot use this because it treats UTF-8 as bytes
	//	return rawurlencode($str);

	$str = str_replace("%", "%25", $str);
	$str = str_replace("<", "%3C", $str);
	$str = str_replace('>', "%3E", $str);
	$str = str_replace('&', "%26", $str);

	$str = str_replace(':', "%3A", $str);
	$str = str_replace(',', "%2C", $str);
	$str = str_replace(' ', "%20", $str);
	$str = str_replace('"', "%22", $str);
	$str = str_replace("\x1C", "%1C", $str);

	//TODO this must be very slow. Speed it up

	$str = str_replace("\x00", "%00", $str);
	$str = str_replace("\x01", "%01", $str);
	$str = str_replace("\x02", "%02", $str);
	$str = str_replace("\x03", "%03", $str);
	$str = str_replace("\x04", "%04", $str);
	$str = str_replace("\x05", "%05", $str);
	$str = str_replace("\x06", "%06", $str);
	$str = str_replace("\x07", "%07", $str);
	$str = str_replace("\x08", "%08", $str);
	$str = str_replace("\x09", "%09", $str);
	$str = str_replace("\x0A", "%0A", $str);
	$str = str_replace("\x0B", "%0B", $str);
	$str = str_replace("\x0C", "%0C", $str);
	$str = str_replace("\x0D", "%0D", $str);
	$str = str_replace("\x0E", "%0E", $str);
	$str = str_replace("\x0F", "%0F", $str);

	$str = str_replace("\x10", "%10", $str);
	$str = str_replace("\x11", "%11", $str);
	$str = str_replace("\x12", "%12", $str);
	$str = str_replace("\x13", "%13", $str);
	$str = str_replace("\x14", "%14", $str);
	$str = str_replace("\x15", "%15", $str);
	$str = str_replace("\x16", "%16", $str);
	$str = str_replace("\x17", "%17", $str);
	$str = str_replace("\x18", "%18", $str);
	$str = str_replace("\x19", "%19", $str);

	//GUI is currently using 001A-001F for field marks
	//but XML cant handle them
	$str = str_replace("\x1A", "%1A", $str);
	$str = str_replace("\x1B", "%1B", $str);
	$str = str_replace("\x1C", "%1C", $str);
	$str = str_replace("\x1D", "%1D", $str);
	$str = str_replace("\x1E", "%1E", $str);
	$str = str_replace("\x1F", "%1F", $str);

	$str = str_replace("\x7F", "%7F", $str);

	return $str;
}

//inbound from gui. gui used javascript escape() to allow wrapping in xml for transport
function unescape($str)
{

	//cant use this because it treats if doesnt understand %uFFFF and doesnt convert %80-%FF to unicode multibyte UTF-8
	//return rawurldecode($str);

	//	%uFFFF decoded to UTF-8
	$str = preg_replace_callback('/%u([0-9a-fA-F]{4})/', function ($match) {
		return json_decode('"\u' . $match[1] . '"');
	}, $str);

	//	%FF decoded to UTF-8
	$str = preg_replace_callback('/%([0-9a-fA-F]{2})/', function ($match) {
		return json_decode('"\u00' . $match[1] . '"');
	}, $str);

	return $str;
}

//see client.js for unicode user interface strategy
//writing to backend
//which currently decodes %F9-%FF in REQUEST
//hexcode(3,REQUEST in)
//and decodes all %00-%FF in DATA
//hexcode(2,DATA in)
//TODO? change backend to only decode %FA-%FF
function escapefms($str)
{

	//NB encode % to %25 FIRST
	$str = str_replace("%", "%25", $str);

	//GUI is using \x1A-1F for field marks
	$str = str_replace("\x1F", "%FF", $str);
	$str = str_replace("\x1E", "%FE", $str);
	$str = str_replace("\x1D", "%FD", $str);
	$str = str_replace("\x1C", "%FC", $str);
	$str = str_replace("\x1B", "%FB", $str);
	$str = str_replace("\x1A", "%FA", $str);

	return $str;
}

//reading from backend
//which currently encodes %->%25 %00-%0F %18-%1F %F9-%FF
//hexcode(1,DATA out)
//hexcode(1,RESPONSE out)
//TODO? change backend to only encode %FA-%FF
function unescapefms($str)
{

	//GUI is using \x1A-1F for field marks
	$str = str_replace("%FF", "\x1F", $str);
	$str = str_replace("%FE", "\x1E", $str);
	$str = str_replace("%FD", "\x1D", $str);
	$str = str_replace("%FC", "\x1C", $str);
	$str = str_replace("%FB", "\x1B", $str);
	$str = str_replace("%FA", "\x1A", $str);

	//return $str;
	//consider changing backend to *not* encode anything except the top six STM TM SM VM FM RM


	//NB unencode %25 to % and any other encoded characters LAST
	//$str=str_replace("%25","%",$str);
	//TODO work out exactly what should unescaped
	$str = rawurldecode($str);

	return $str;
}

function ReadAll($filename)
{

	if (!is_file($filename))
		return '';

	$str = file_get_contents($filename);

	//convert from database codepage to unicode
	global $gdatabase_codepage, $gweb_codepage;
	if ($gdatabase_codepage != $gweb_codepage)
		$str = convert_codepage($gdatabase_codepage, $gweb_codepage, $str, "reading " . substr($filename, -3, 2));

	return $str;
}

function WriteAll($filename, $str)
{

	//convert from unicode to database codepage
	global $gdatabase_codepage, $gweb_codepage;
	if ($gdatabase_codepage != $gweb_codepage)
		$str = convert_codepage($gweb_codepage, $gdatabase_codepage, $str, "writing " . substr($filename, -3, 2));

	return file_put_contents($filename, $str);
}

function convert_codepage($from_codepage, $to_codepage, $str, $mode)
{
	if ($str == '')
		return '';
	$str2 = iconv($from_codepage, $to_codepage, $str);
	//handle conversion error. actual error might be in php error.log
	if ($str2 == '') {
		debug("FROM STR: $str");
		exit("Error while $mode and converting from $from_codepage to $to_codepage or code page not known to php iconv()");
	}
	return $str2;
}
