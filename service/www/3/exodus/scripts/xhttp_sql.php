<?php

//errors cannot be echoed to xmlhttp so force them off in case set in php.ini
ini_set('display_errors', 'Off');
//php.ini
//display_errors = off

ignore_user_abort(true);

// debug in php like this
// ----------------------
//1. change $debugging date below to some date in the future
//2. tail -f /var/log/apache2/error.log
//3. also see .2 and .3 files are left in interface directory

//debugging leaves the .2 and .3 files undeleted and puts messages in 
//$debugging = true;
//$debugging = false;
//debugging will automatically be turned off so you dont accidentally
//leave debugging on with the consequent excessive logging
$debugging = time() <= (strtotime("2020-09-12") + 86400);
//moved further down
//$debugging=$debugging || is_file($gdatalocation . 'debug.php');
$gdebug_data = false; //true;
$gdebug_xml = true;

$gwindows = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN';
//debug("PHP_OS        : " . PHP_OS);
//debug("WINDOWS       : " . $gwindows);

$gslash = $gwindows ? '\\' : '/';

// constants
// ---------

define("FM", "\u{001E}");
define("VM", "\u{001D}");
define("SM", "\u{001C}");

//echo "VM=" . bin2hex(VM) . "\n";
//exit;

$gautostartdatabase = true; //NET.CFG must also contain AUTOSTART=YES
$gsecondstowaitforreceipt = 15;
$gsecondstowaitforstart = 30;

$cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER';
$databasestoppedresponse = "The database service is stopped.\r\rPlease try again later or ask your\rtechnical support staff to start it.";
$norequestresponse = 'ERROR: NO REQUEST';
$nodataresponse = 'ERROR: NO DATA';
$invaliddatapathresponse = 'ERROR: CANNOT WRITE TO ';

// defaults
// --------

//default timeout is 10 minutes (NB BACKEND timeout (in GIVEWAY) is hard coded to 10 mins?)
//actually the client sends the timeout (10 mins)
$defaulttimeoutmins = 10;
$timeout_ms = $defaulttimeoutmins * 60 * 1000;

// session
// -------
session_start();
//session_destroy();

//provides $config['databases'][];
include_once('../../../../config.php');

//EXODUS root path should point to the folder containing exodus, exodus.net, data, images etc
// eg D:\\hosts\\test or /var/www/exodus?
//$exodusrootpath = $_SERVER['DOCUMENT_ROOT'];
// __DIR__ = D:\\hosts\\test\\exodus.net\\3\\exodus\\scripts

//on linux
$exodusrootpath = preg_replace('!3/exodus/scripts!', '', __DIR__);
$exodusrootpath = preg_replace('!/exodus.net/!', '/', $exodusrootpath);

//on windows
$exodusrootpath = str_replace('3\\exodus\\scripts', '', $exodusrootpath);
$exodusrootpath = str_replace('\\exodus.net\\', '\\', $exodusrootpath);

if ($gwindows)
	$exodusrootpath = str_replace('/', '\\', $exodusrootpath);

//debug("exodusrootpath : $exodusrootpath");

// /var/www/html/exodus2//data/default.vol
// D:\hosts\test\exodus.net\3\exodus\scriptsEXODUS/ADAGENCY.VOL
$gdatalocation = ($exodusrootpath . 'data' . $gslash);

if (!file_exists($gdatalocation)) {
	debug("ERROR: Datalocation does not exist:" . $gdatalocation);
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

// request
// -------

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
	debug("data_io :" . $xml->data);

//request contains token, request, database and data
$token = unescape($xml->token);
$request = unescape($xml->request);
$database = unescape($xml->dataset); //probably come as DATASET not DATABASE
//debug("d1 $database", 5);
$data_io = unescape($xml->data);
$system = '';

//convert any crlf to just cr
//exodus uses \r which is cr because \n and lf are messed about as either 0d0a or just 0a
$request = str_replace("\n", "\r", $request);
$token = str_replace("\n", "\r", $token);

$requests = explode("\r", $request . "\r\r\r\r\r\r\r\r");
$request0 = $requests[0];

//
$signin_user_code = $_SESSION['USER_CODE'];

//get database from URL if not posted
if (!$database) {
	$database = explode('/', $_SERVER['REQUEST_URI'])[1];
	//debug("d2 $database",5);
}

//token is supposed to be "database*usercode*"
//designed to allow multiple sessions from one browser
//by acting as a prefix to cookies
if (!$token) {
	$token = "$database*$signin_user_code*";
}

if ($signin_user_code) {
	$_SESSION[$token . '_username'] = $signin_user_code;
	$_SESSION[$token . '_password'] = $signin_user_code;
	$_SESSION[$token . '_database'] = strstr($token, '*', true);
	$_SESSION[$token . '_system'] = 'default';
	$_SESSION[$token . '_timeout'] = 60 * 20 * 1000;
}

if ($token) {

	//if login then save new details regardless of success or not
	if ($request0 == 'LOGIN') {

		$request = $request0;
		$username = $requests[1];
		$password = $requests[2];
		$database = $requests[3];
		//debug("d3 $database",5);
		$authno = $requests[4];
		$system = $requests[5];

		//suppress password from logs, screens etc.
		$requests[2] = '';
		error_log(implode(' ', array_slice($requests, 0, 6)));

		//remove legacy default
		//			if ($system == 'ADAGENCY')
		$system = '';

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
			//debug("d4 $database",5);
			$system = $_SESSION[$token . '_system'];
			$timeout_ms = $_SESSION[$token . '_timeout'];
		} else {
			$username = '';
			$password = '';
			$database = '';
			//debug("d5 $database",5);
			$system = '';
			$timeout_ms = '';
		}
		debug("USERNAME:$username");
	}

	//remove legacy default
	//		if ($system == 'ADAGENCY')
	$system = '';

	//if logout then clear session username, password and database from the session
	if ($request0 == 'LOGOUT') {
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
	//debug("d6 $database",5);

	$globalserverfilename = $gdatalocation . $databasedir . $database . '.SVR';
}

//MUST be sent before anything else
header("Content-Type:text/xml; charset=utf-8");
//? like xhttp.asp let client guess encoding because EXODUS client data might not be in UTF-8 format eg DIO (Dior) in TEST database
//header ("Content-Type:text/xml");

//failure
//$data_io = '';
$response = '';
$result = 0;
$linkfilename = '';

$proglocation = $exodusrootpath . '/exodus/';

// keepalive
if ($request0 == 'KEEPALIVE' || $request0 == 'LOOPBACK') {

	debug($request0);

	$response = "OK";
	$result = 1;

	// fail if could not locate the database
} elseif (!$exodusrootpath) {
	$response = $cannotfinddatabaseresponse;

	// fail if no request
} elseif ($request == '') {
	$response = $norequestresponse;

	// special request that doesnt require anything but a request and exodusrootpath
} else if ($request0 == 'GETDATASETS') {
	$data_io = '<records>';
	//front end currently requires xml TODO convert to JSON
	foreach ($config['databases'] as $dbid => $database) {
		$data_io .= '<record><name>' . $database['name'] . '</name><code>' . $dbid . '</code></record>';
	}
	$data_io .= '</records>';
	$result = 1;

	// fail regardless of database if GLOBAL.END file is present
	//TODO check other locations for GLOBAL.END (multiinstallation and database)
} elseif (is_file($proglocation . 'global.end')) {
	$response = $databasestoppedresponse;

	// check username is present - session may have timed out
} elseif (!$username) {
	$response = 'Error: Please login - Session not established or timed out';

	// check password is present
} elseif (!$password) {
	$response = 'Error: Password parameter is missing';

	// check database is present
} elseif (!$database) {
	$response = 'Error: Dataset parameter is missing';

	// check details and token
} elseif ($token && $token != $database . '*' . $username . '*') {
	$response = 'token is ' . $token . ' but should be ' . $database . '*' . $username . '*';

	// request response
} else {

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

	// make the request, get the response
	// ---------------------------------------------
	//debug("d7 $database",5);
	$response = request_response($request, $data_io, $remoteaddr, $remotehost, $https, session_id(), $agent, $database, $username, $password);
	debug("RESPONSE      : $response");

	if (explode(' ', $response)[0] == 'OK')
		$result = 1;
	else
		$result = 0;

	//convert data path to web path
	if ($request0 == "EXECUTE") {
		$leadin = substr($data_io, 0, strlen($exodusrootpath)) . "/";
		debug("EXECUTE LEADIN " . $leadin);
		if (substr($data_io, 0, strlen($leadin)) == $exodusrootpath . "/") {
			//$data_io = "../" . substr($data_io,strlen($leadin));
			$data_io = str_replace($leadin, "../", $data_io);
		}
	}
}

// output
// ------

//data and response needs to be encoded so that it is valid xml
//eg % become %25 etc

//encode xml related characters since message to client is in xml format
//will be decoded in client after unpacking xml message

$xmltext = "<root>";
$xmltext .= "<data>"     . escape($data_io)  . "</data>";
$xmltext .= "<response>" . escape($response) . "</response>";
$xmltext .= "<result>"   . escape($result)   . "</result>";
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
if ($gdebug_data && $data_io)
	debug("DATA_IO:$data_io");
//	if (!$result)
//		debug("RESULT  :$result");
if ($gdebug_xml)
	debug("XMLOUT  :$xmltext");

// finished
// --------

function getdatabases($exodusrootpath, $systemcode)
{

	//will look in /var/www/exodus/ for default.vol, adagency.vol or accounts.vol

	//return an array of available database codes and names
	//or an empty array

	global $response;

	if (!$systemcode)
		$systemcode = 'default';

	//get an array of databases
	//$gdatalocation = $exodusrootpath . "/data/";
	global $gdatalocation;
	$volfilename = $gdatalocation . $systemcode . '.vol';
	//$volfilename=$exodusrootpath . "EXODUS/" . $systemcode . '.VOL';
	debug("volfilename:" . $volfilename);
	if (!is_file($volfilename)) {
		$response = "Cannot see vol file $volfilename";
		debug("Cannot see vol file $volfilename", 5);
		return "";
	}
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

function ReadAll($filename)
{
	if (!is_file($filename))
		return '';
	return file_get_contents($filename);
}

function WriteAll($filename, $str)
{
	return file_put_contents($filename, $str);
}

function debug($msg, $level = 2)
{
	global $debugging;
	if ($debugging || $level > 2) {
		//$msg=str_replace("\x1e","^",$msg);
		//$msg=str_replace("\x1d","]",$msg);
		//if ($level>2) {
		error_log(json_encode($msg), 0);
		//}
	}
}

function request_response($request, &$data_io, $remoteaddr, $remotehost, $https, $sessionid, $agent, $database, $username, $password)
{

	$requests  = explode("\r", $request);
	debug($requests, 5);

	$request1 = $requests[0];

	// open a new db connection
	//$db = new PDO('pgsql:host=localhost;dbname=test', $user, $pass);
	try {
		global $db, $dbcmd, $config;
		//$db = new PDO('pgsql:host=localhost port=5432 dbname=exodus','exodus','somesillysecret');
		//debug("d8 $database",5);
		$dbconfig = $config['databases'][$database];
		if (!$dbconfig) {
			$response = "Error: Session lost. Please login again.";
			return $response;
		}
		$db = new PDO($dbconfig['conn'], $dbconfig['user'], $dbconfig['pass']);
	} catch (PDOException $error) {
		$response = "Error: PDO says " . $dbconfig['conn'] . $error->getMessage() . "<br/>";
		return $response;
	}

	$read = substr($request1, 0, 4) == 'READ';
	$write = substr($request1, 0, 5) == 'WRITE';
	$delete = $request1 == 'DELETE';
	$select = $request1 == 'SELECT';

	if ($read || $write || $delete || $select) {
		$filespec = explode('|', $requests[1]);
		if (count($filespec) < 2) {
			$filespec[1] = '';
			$filespec[2] = '*';
		} elseif (count($filespec) < 3) {
			$filespec[2] = '*';
		}
		$file = (object) array_combine(['name', 'primary_key', 'colids'], $filespec);
		$key = $requests[2];
	}

	if ($request1 == 'SELECT') {
		//["SELECT","EVENTS","BY-DSND CURRENT_YEAR","eventname eventid ID","XML","2000"]

		$file->name = strtolower($file->name);
		$file->primary_key = strtolower($file->primary_key);

		//read selective columns only
		$file->colids = str_replace(' ', ',', $requests[3]);

		//option to return the whole record with FMs or multiple records separated by RMs
		//this is called to update or clear cached records from client.js
		//with a list of keys in data_io
		//TODO implement in readrec something like XML
		if ($file->colids == 'RECORD') {
			$data_io = '';
			$response = 'OK';
		} else {

			//replace "ID" column with the "primary_key as ID"
			$file->colids = preg_replace('/\bID\b/', $file->primary_key . ' AS ID', $file->colids);

			//debug($file);

			//TODO implement sort/select argument
			//for now return all records
			$file->primary_key = '';
			$key = '';

			$options = $requests[4];
			$limit = $requests[5];

			if (readrec($file, $key, $data_io, $options, $limit)) {
				$response = 'OK';
			} else {
				$response = 'Error: NO RECORD';
			}
		}
	} elseif ($read) {

		if (readrec($file, $key, $data_io)) {
			$response = 'OK';
		} else {
			$response = 'Error: NO RECORD';
		}

		if (substr($request1, -1) == 'U') {
			$response .= ' SESSIONID ' . rand(10000000, 99999999);
		}
	} elseif ($write) {

		// start a new transaction and rely on automatic rollback
		// unless commit is reached below
		$db->beginTransaction();

		if (!writerec($file, $key, $data_io)) {
			$db->rollBack();
			return "ERROR: " . implode(' - ', $dbcmd->errorInfo());
		}

		$db->commit();

		$response = 'OK';
	} elseif ($delete) {

		//disallow plain delete without key since it clears the file
		if (!$file->primary_key) {
			$response = "Error: Delete all is disabled.";
			return $response;
		}

		// start a new transaction and rely on automatic rollback
		// unless commit is reached below
		$db->beginTransaction();

		if (!deleterec($file, $key)) {
			$db->rollBack();
			return "ERROR: " . implode(' - ', $dbcmd->errorInfo());
		}

		$db->commit();

		$response = 'OK';
	} elseif ($request1 == 'LOCK') {

		$response = 'OK';
	} elseif ($request1 == 'UNLOCK') {

		$response = 'OK';
	} elseif ($request1 == 'RELOCK') {

		$response = 'OK';
	} elseif ($request1 == 'LOGIN') {

		/*
		//prepare session cookie
		cookie='m=':convert(vm,',',menus)
	
		*current datasetname
		temp=system<23>
		swap '&' with ' and ' in temp
		cookie:='&db=':temp
	
		*form color, font and fontsize
		cookie:='&fc=':system<46,5>
		cookie:='&ff=':system<46,6>
		cookie:='&fs=':system<46,7>
	
		*date format
		cookie:='&df=':company<10>
	
		*first day of week
		tt=agp<13>+1
		if tt>7 then tt=1
		cookie:='&fd=':tt
		*/

		$cookie = 'm=MAIN&db=' . $dbconfig['name'];
		$data_io = $cookie;

		$response = 'OK';
	} elseif ($request1 == 'EXECUTE') {
		$data_io = "none";
		$response = 'OK';
	}

	if ($response != 'OK') {
		debug($response, 5);
	}

	return $response;
}

function readrec($file, $key, &$data_out, $options = '', $limit = false)
{

	global $db;

	// prepare sql statement to select record(s)
	$sql = 'SELECT ' . $file->colids . ' FROM ' . $file->name;
	if ($file->primary_key) {
		$sql .= ' WHERE ' . $file->primary_key . ' = ?';
	}
	if ($limit) {
		$sql .= " LIMIT " . intval($limit);
	}
	$dbcmd = $db->prepare($sql);
	if (!$dbcmd) {
		die("ERROR: " . implode(' - ', $db->errorInfo()) . '<br>' . $sql);
	}

	//execute it with args
	if (!$dbcmd->execute([$key])) {
		die("ERROR: " . implode(' - ', $db->errorInfo()) . '<br>' . $sql);
	}

	//get all columns (and all rows if more than one) into an array
	if ($options == 'XML') {
		$rows = $dbcmd->fetchAll(PDO::FETCH_ASSOC); //associative array per record
	} else {
		$rows = $dbcmd->fetchAll(PDO::FETCH_NUM); //ordinary numeric array per record
	}

	// not found - return false
	$nrows = count($rows);
	if ($nrows == 0) {
		$data_out = '';
		return false;
	}

	// get colinfo
	$cols = get_cols($file);

	// convert dates to gui intdate format
	// and json to mv or sv
	for ($coln = 0; $coln < count($cols); ++$coln) {
		//debug([$cols[$coln], $rows[0][$coln]],5);
		if ($cols[$coln] == 'D') {
			for ($rown = 0; $rown < $nrows; ++$rown) {
				$cell = $rows[$rown][$coln];
				if ($cell) {
					$rows[$rown][$coln] = sql2intdate($cell);
				}
			}
		}
		//convert json array to multivalues (or sv if selected many rows)
		elseif ($cols[$coln] == 'J') {
			$sep = $nrows > 1 ? SM : VM;
			for ($rown = 0; $rown < $nrows; ++$rown) {
				$cell = $rows[$rown][$coln];
				if ($cell) {
					$cell = json_decode($cell, false);
					$rows[$rown][$coln] = implode($sep, $cell);
				}
			}
		}
	}

	if ($options == 'XML') {
		foreach ($rows as &$cols) {
			foreach ($cols as $colid => &$col) {
				$col = "<$colid>$col</$colid>";
			}
			$cols = implode('', $cols);
			//debug($cols);
		}
		$data_out = "<records><RECORD>" . implode("</RECORD><RECORD>", $rows) . "</RECORD></records>";
		return true;
	}

	// found one - convert columns to fields
	if (count($rows) == 1) {
		$data_out = implode(FM, $rows[0]);
		return true;
	}

	// found many - convert to multivalued fields

	// transpose rows and columns of $rows[][]
	$fields = array_map(null, ...$rows);

	// fixed array to fm/vm string array
	$data_out = '';
	foreach ($fields as $field) {
		$data_out .= rtrim(implode(VM, $field), VM) . FM;
	}
	// $data_out = rtrim($data_out,FM);
	$data_out = substr($data_out, 0, -1);

	// all is well that ends well
	return true;
}

function deleterec($file, $key)
{

	global $db, $dbcmd;

	// prepare sql statement to delete record(s)
	$sql = 'DELETE FROM ' . $file->name;
	if ($file->primary_key) {
		$sql .= ' WHERE ' . $file->primary_key . ' = ?';
	}
	$dbcmd = $db->prepare($sql);

	//execute delete statement
	$result = $dbcmd->execute([$key]);
	//debug([$dbcmd,$key]);

	return !!$result;
}

function writerec($file, $key, &$data_io)
{

	// $stmt = $conn->prepare('INSERT INTO customer_info (user_id, fname, lname) VALUES(:user_id, :fname, :lname)
	// ON DUPLICATE KEY UPDATE fname= :fname2, lname= :lname2');

	global $db, $dbcmd;

	// delete everything first
	deleterec($file, $key);

	// get colinfo
	$cols      = get_cols($file);
	$ncols     = count($cols);
	//not needed for now?
	//$nullables = $file->nullables;

	// add timestamp
	//TODO

	// prepare args and place holders for insertion
	$fields  = explode(FM, $data_io);
	$nfields = count($fields);

	//$place_holders = substr(str_repeat('?,',$nfields),0,-1);
	//$place_holders = substr(str_repeat('?,', $ncols), 0, -1);
	$place_holders = '';
	for ($fn = 0; $fn < $ncols; ++$fn) {
		$colname = $file->colnames[$fn];
		if ($colname == 'full_text')
			$place_holders .= 'DEFAULT,';
		else
			$place_holders .= '?,';
	}
	//remove surplus trailing comma
	$place_holders = substr($place_holders, 0, -1);

	//multivalued key field indicates multiple records to be updated/inserted
	$nrecs   = substr_count($fields[$file->keycoln], VM);

	// prepare for insertion
	$dbcmd = $db->prepare(
		'INSERT INTO ' . $file->name .
			//		' WHERE ' . $file->primary_key . '= ? ' .
			' VALUES (' . $place_holders . ')'
	);
	// convert all fields to multivalues and find maxvn
	//if only one key/one record then still treat as array of arrays of size 1
	$maxvn = 1;
	for ($fn = 0; $fn < $nfields; ++$fn) {
		$value = $fields[$fn];
		if ($nrecs > 1) {
			$values = explode(VM, $value);
			$nmv = count($values);
			if ($nmv > $maxvn)
				$maxvn = $nmv;
		} else {
			$values = [$value];
		}
		$fields[$fn] = $values;
	}

	// insert records (or just one)
	// TODO fill in key fields
	$offset = 0;
	for ($vn = 0; $vn < $maxvn; ++$vn) {
		//for ($fn=0;$fn<$nfields;++$fn) {
		for ($fn = 0; $fn < $ncols; ++$fn) {

			$colname = $file->colnames[$fn];

			if ($fn == $file->keycoln)
				$cell = $key;
			else
				$cell = $fields[$fn][$vn] ?? '';

			//debug([$cols[$fn], $cell]);

			$bind_fn = $fn + 1 + $offset;

			// full text
			if ($colname == 'full_text') {
				//$dbcmd->bindValue($fn + 1, 'DEFAULT');
				//reduce following fields bind number by 1 because this column cannot be bound (has no ?) and is set to DEFAULT
				$offset = -1;
				continue;
			}

			//(i)nteger
			elseif ($cols[$fn] == 'I') {
				//debug('I '.json_encode($cell));
				$dbcmd->bindValue($bind_fn, !strlen($cell) ? NULL : $cell, PDO::PARAM_INT);
			}

			// (d)ate: replace '' with NULL
			// and convert gui intdate to sql date YYYY-MM-DD
			elseif ($cols[$fn] == 'D') {
				//debug('D '.json_encode($cell));
				// maybe add || !$nullables[$fn]
				if (substr($colname, -8) == '_updated' || (!strlen($cell) && substr($colname, -8) == '_created')) {
					$dbcmd->bindValue($bind_fn, sqlnow(), PDO::PARAM_STR);
				} else {
					$dbcmd->bindValue($bind_fn, !strlen($cell) ? NULL : intdate2sql($cell), PDO::PARAM_STR);
				}
			}

			// (f)loat replace '' with NULL
			elseif ($cols[$fn] == 'F') {
				//debug('F '.json_encode($cell));
				$dbcmd->bindValue($bind_fn, !strlen($cell) ? NULL : $cell, PDO::PARAM_STR);
			}

			// (j)son put []for null
			elseif ($cols[$fn] == 'J') {
				//debug('J '.json_encode($cell));
				$dbcmd->bindValue($bind_fn, json_encode(explode(VM, $cell)), PDO::PARAM_STR);
			}

			// (v)archar do not replace '' with NULL
			else {
				//debug('? '.json_encode($cell));
				$dbcmd->bindValue($bind_fn, $cell, PDO::PARAM_STR);
			}
		}

		$result = $dbcmd->execute();
		if (!$result) {
			//debug($dbcmd->errorInfo());
			debug($dbcmd->errorInfo() . " " . var_export($cols, true), 5);
		}
		if (!$result) {
			return false;
		}
	}

	//debug($dbcmd);
	//debug($fields);
	//debug($file);
	//debug($result);
	//debug($dbcmd->errorInfo());

	return !!$result;
}

function get_cols(&$file)
{

	//input file_name and $file->primary_key
	//output 1. array eg ['V','I','F',D'] where (V)archar (I)nt (D)ate (F)loat
	//output 2. $file->keycoln if $file->primary_key column is found
	//TODO handle multi-part keys
	$cols = [];
	$file->nullables = [];
	$file->colnames = [];
	$file->keycoln = -1;
	global $db;
	$result = $db->query('SELECT * FROM ' . $file->name . ' LIMIT 0');
	for ($coln = 0; $coln < $result->columnCount(); $coln++) {
		$col = $result->getColumnMeta($coln);
		//debug(json_encode($col));
		/*
		//debug($col['native_type']);
		VARCHAR "VAR_STRING"
		INT "LONG"
		INT "LONG"
		JSON null
		TEXT "BLOB"
		DATE "DATE"
		DATETIME "DATETIME"
		*/
		$file->nullables[$coln] = !in_array("not_null", $col['flags']);
		$file->colnames[$coln]  = $col['name'];
		$col_type = strtoupper($col['native_type'] ?? '');

		//		debug($col_type);
		if (strpos($col_type, 'INT') !== false || $col_type == 'LONG')
			$col_type = 'INT';

		else if (strpos($col_type, 'BLOB') || strpos($col_type, 'TEXT') !== false)
			$col_type = 'VARCHAR';

		else if (strpos($col_type, 'TINY') !== false || strpos($col_type, 'SMALL') !== false || strpos($col_type, 'MEDIUM') !== false || strpos($col_type, 'BIG') !== false)
			$col_type = 'INT';

		elseif ($col_type == '')
			$col_type = 'JSON';
		//		debug($col_type);

		$cols[] = $col_type[0];
		if ($col['name'] == $file->primary_key) {
			$file->keycoln = $coln;
		}
		//$colsx[] = $col;
	}
	//debug($cols);
	return $cols;
}

function sqlnow()
{
	# return an sql time stamp string something like '2020-12-31 23:59:59'
	return date('Y-m-d H:i:s');
}

// integer date -> YYYY-MM-DD
function intdate2sql($intdate)
{
	if (strpos($intdate, '.') !== false) {
		$dayn = floor($intdate);
		$secs = round(100000 * fmod($intdate, 1));
		$sqldatetime = DateTime::createFromFormat('U', 86400 * (floor($intdate) - 732) + $secs)->format('Y-m-d H:i:s');
	} else {
		return DateTime::createFromFormat('U', 86400 * ($intdate - 732))->format('Y-m-d');
	}
	return $sqldatetime;
}

// YYYY-MM-DD          -> integer date
// YYYY-MM-DD HH:MM:SS -> integer date . integer seconds
function sql2intdate($sqldate)
{
	//WITH SPACE then assume datetime and convert to DDDDD.SSSSS format
	//where DDDDD is number of days since 31 DEC 1967 (01 JAN 1968 = Day 1)
	//and SSSSS is 00000-86399 indicating seconds from midnight
	//NOTE that SSSSS is not a decimal fraction of a day
	if (strpos($sqldate, ' ') != false) {
		$days = ((DateTime::createFromFormat('Y-m-d H:i:s', $sqldate)->format('U') + 63244800) / 86400);
		$dayn = floor($days);
		$secs = round(86400 * fmod($days, 1));
		return $dayn . "." . str_pad($secs, 5, "0", STR_PAD_LEFT);
	}
	//without space it is just an integer date in DDDDD format described above
	else {
		$sqldate .= ' 00:00:00';
		return floor((DateTime::createFromFormat('Y-m-d H:i:s', $sqldate)->format('U') + 63244800) / 86400);
	}
}
