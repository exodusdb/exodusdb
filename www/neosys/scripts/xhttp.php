<?php

	//errors cannot be echoed to xmlhttp so force them off in case set in php.ini
	ini_set('display_errors', 'Off');
	//php.ini
	//display_errors = off

	//debug in php like this
	//tail -f /var/log/apache2/error.log
	//also .2 and .3 files are left in interface directory if debugging
	$debugging = false;

//constants

	//if mb php implemented
	//change explode to mb_split, mb_strlen, mb_substr
	//mb_internal_encoding('UTF-8');
	//mb_regex_encoding('UTF-8');

	//php5 doesnt recognise "\u255e" etc.
	$fm = '\x25\x5e';
	$vm = '\x25\x5d';
	$sm = '\x25\x5c';

	$gautostartdatabase = true;//unless ../exodus/net.cfg first line is AUTOSTART=NO
	$gsecondstowaitforreceipt = 15;
	$gsecondstowaitforstart = 30;

	$cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER';
	$databasestoppedresponse = "The database service is stopped.\r\rPlease try again later or ask your\rtechnical support staff to start it.";
	$norequestresponse = 'ERROR: NO REQUEST';
	$nodataresponse = 'ERROR: NO DATA';
	$invaliddatapathresponse = 'ERROR: CANNOT WRITE TO ';

//defaults

	//default timeout is 10 minutes (NB BACKEND timeout (in GIVEWAY) is hard coded to 10 mins?)
	$defaulttimeoutmins = 5;
	$timeout = $defaulttimeoutmins * 60;


// session

	session_start();

	$neosysrootpath = getneosysrootpath($_SERVER['DOCUMENT_ROOT']);
	debug("DOCUMENT_ROOT :".$neosysrootpath);

	$datalocation = ($neosysrootpath . '/data/');

	//not used atm
	if (($neosysrootpath . "UNICODE.INI"))
		$unicode = -1;
	else
		$unicode = 0;

	//get and cleanup hostname
	$localhostname = $_SERVER['SERVER_NAME'];
	//remove space . " ' characters
	$localhostname = str_replace(array(".", ".", " "),"_",$localhostname);
	//and take first and last four characters if longer than 8 characters. why?
	if (strlen($localhostname) > 8)
		$localhostname = substr($localhostname,0, 4) . substr($localhostname,-4);

	$remoteaddr = $_SERVER['REMOTE_ADDR'];
	$remotehost = $_SERVER['REMOTE_HOST'];
	$https = $_SERVER['HTTPS'];

//request

	//client delivers a request in xml format
	$xml = simplexml_load_string($HTTP_RAW_POST_DATA);

	//debug("POST   ---> ".$HTTP_RAW_POST_DATA);
	debug("REQUEST :".$xml->request);
	if ($xml->token)
		debug("TOKEN   :".$xml->token);
	if ($xml->database)
		debug("DATABASE:".$xml->base);
	if (strlen($xml->data))
		debug("DATA_IN :".$xml->data);

	//request contains token, request, database and data
	$token=rawurldecode($xml->token);
	$request=rawurldecode($xml->request);
	$database=rawurldecode($xml->dataset);//probably come as DATASET not DATABASE
	$data_in=rawurldecode($xml->data);

	//convert any crlf to just cr
	//neosys uses \r which is cr because \n and lf are messed about as either 0d0a or just 0a
	$request = str_replace("\n","\r",$request);
	$token = str_replace("\n","\r",$token);

	debug("token $token");

	$requests = explode("\r",$request . "\r\r\r\r\r\r\r\r");

	if ($token) {

		//if login then save new details regardless of success or not
		if ($requests[0] == 'LOGIN') {

			$username = $requests[1];
			$password = $requests[2];
			$database = $requests[3];
			$authno = $requests[4];
			$system = $requests[5];
			$request = $requests[0];//remove the password from the request string

			$_SESSION[$token . '_username'] = $username;
			$_SESSION[$token . '_password'] = $password;
			$_SESSION[$token . '_database'] = $database;
			$_SESSION[$token . '_system'] = $system;
			$_SESSION[$token . '_timeout'] = $timeout;
		} else {

			//try and get the username, password and database from the session
			$username = $_SESSION[$token . '_username'];
			$password = $_SESSION[$token . '_password'];
			$database = $_SESSION[$token . '_database'];
			$system = $_SESSION[$token . '_system'];
			$timeout = $_SESSION[$token . '_timeout'];

			debug("username from session: $username");

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
		//TODO Server.ScriptTimeout = $timeout / 1000 + 60;

		//append trailing '/' to databasedir if necessary
		$databasedir = $database . "/";

		$globalserverfilename = $datalocation . $databasedir . $database . '.SVR';
	}

// attempt(s)

	//using "loop" to avoid use of function/subroutine
	//break = failures (or at end, success)
	//continue = possible 2nd attempt after requesting database to start
	//actually php 5.3 has goto commands which might be used instead and avoid 1 indent
	while (1)
	{

		//failure
		$data_out = '';
		$response = '';
		$result = 0;
		$linkfilename = '';

		//keepalive
		if ($requests[0] == 'KEEPALIVE') {

			debug("KEEPALIVE");

			$response = "OK";
			$result = 1;
			break;
		}

		//fail if could not locate the database
		if (!$neosysrootpath) {
			$response = $cannotfinddatabaseresponse;
			break;
		}

		//fail if no request
		if ($request == '') {
			$response = $norequestresponse;
			break;
		}

		//special request that doesnt require anything but a request and neosysrootpath
		if ($requests[0]=='GETDATASETS') {
			$data_out=getdatabases($neosysrootpath,$requests[1]);
			$result=1;
			break;
		}

		//fail regardless of database if GLOBAL.END file is present
		//TODO check other locations for GLOBAL.END (multiinstallation and database)
		$proglocation = $neosysrootpath . '/exodus/';
		if (is_file($proglocation . 'GLOBAL.END')){
			$response = databasestoppedresponse;
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
			$response='token is ' . $token . ' but should be ' . $database . '*' . $username . '*';
			break;
		}

		//make a random ~9999999.x file name for the request
		do {
			//~*.htm files are not backed up in FILEMAN
			$linkfilename = '~' . neosysrnd(9999999, 1000000);
			$linkfilename0 = $linkfilename;
			$linkfilename = $datalocation . $databasedir . $linkfilename;
		} while (glob($linkfilename .'.*'));

		debug("data_in $data_in");

		//write data (if any) before request so that there is no sharing violation with server reader
		if ($data_in) {

			//tf.Write(escape($data))
			//allow field marks to pass through unicode conversion untouched to the backend as escaped characters
			//only really need the four characters FB-FE
			//$temp = unicode2escapedfms($data)
			//unicode to codepage conversion used to take place here in win/IIS
			if (!file_put_contents($linkfilename . '.2', $data_in)) {
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
			$remotehost = $remoteaddr;//if no hostname then use ip
		if (!$https)
			$https = '';

		//prepare a request file to "send" to the server as a polled file
		$fullrequest = '';
		$fullrequest .= $linkfilename .'.1'."\r";
		$fullrequest .= 'VERSION 3'."\r";
		$fullrequest .= '6'."\r"; //npre-database connection info related fields
		$fullrequest .= $remoteaddr."\r";
		$fullrequest .= $remotehost."\r";
		$fullrequest .= $https."\r";
		$fullrequest .= session_id()."\r";//php

		$fullrequest .= $database."\r";
		$fullrequest .= $username."\r";
		$fullrequest .= $password."\r";

		//allow field marks to pass through unicode conversion untouched
//		$temp = unicode2escapedfms($request)
		//$fullrequest .= $temp
		$fullrequest .= $request;

		//write cmd (not as a .1 file otherwise the listener may
		//try to read it before it is ready
		//codepage used to happen here conversion happens here in win/iis
		//otherwise abort

		if (!file_put_contents($linkfilename . '.1$', $fullrequest)) {
			$response = $invaliddatapathresponse . ' "' . $linkfilename . '.1$" ';
			break;
		}
		//ensure server can delete it after renaming/processing it
		chmod($linkfilename . '.1$', 0775);

		//now that everything is in place (request and data files)
		//rename the request file to end in .1
		//so that server/listener pick it up
		//otherwise abort
		if (!rename($linkfilename . '.1$', $linkfilename .'.1')) {
			$response = 'Cannot rename "' . $linkfilename . ".1$\"\rto " . $linkfilename . '.1"';
			break;
		}

		//wait briefly for 10 seconds for the request to disappear
		//sleeping between checks every 10 ms
		$waituntil = time() + $gsecondstowaitforreceipt;
		while (is_file($linkfilename . '.1')){

			//TODO increase sleep if not fast result
			usleep(10000);

			// abort if request file has not been taken within 10 seconds
			//TODO start the database and try again
			//TODO really should rename to take control back of request file to ensure it isnt processed
			if (time() >= $waituntil){
				$response = 'Error: No response in ' . $gsecondstowaitforreceipt . ' seconds from database server at ' . $linkfilename . '.1';
				break 2;
			}
			clearstatcache();
		}

		//wait patiently for 10 minutes for the response to appear otherwise quit
		//sleep between checkds every 10 ms
		$waituntil = time() + $timeout;
		while (!is_file($linkfilename . '.3')) {

			//sleep for 10ms before checking for response again
			//TODO make it sleep longer the long the delay
			// so response is fast if the server is fast but saves cpu if not
			//(after waiting 10 seconds it is pointless to check every 10ms)
			usleep(10000);//10,000 microseconds = 10 ms

			//abort if no response within 10 minutes
			if (time() >= $waituntil) {
				$response = 'Error: No response in ' . $timeout . ' seconds from database server at ' . $linkfilename.".3";
				break 2;
			}

		}

		//read response in .3 file

		//response determines true or false
		$response=file_get_contents($linkfilename . '.3');
		if (!$response) {
			$response = 'Error: Cannot read response file ' . $linkfilename.".3";
			break;
		}
		//$response = escapedfms2unicode(tf.ReadAll())

		//read the data .2 file or empty data if not
		//can return data even if not "OK")
		$data_out=file_get_contents($linkfilename . '.2');
		if (!$data_out)
			$data_out='';
		//$data_out = escapedfms2unicode(tf.ReadAll())

		//using a "loop" as a way to jump forward to abort or succeed anywhere or at end (break statements)
		//only looping if starting database
		break;

	}//end of attempt "loop"

//result

	if (explode(' ',$response)[0] == 'OK') 
		$result = 1;
	else
		$result = 0;

	//convert data path to web path
	if ($requests[0] == "EXECUTE") {
		$leadin=substr($data_out, 0, strlen($neosysrootpath)) . "/";
		debug("EXECUTE LEADIN ". $leadin);
		if (substr($data_out, 0, strlen($leadin)) == $neosysrootpath . "/") {
			$data_out = "../" . substr($data_out,strlen($leadin));
		}
	}

//clean up

	//normally .1 will already have been consumed
	//(renamed, read and then deleted)
	//by the neosys database and cannot be deleted here
	if (is_file($linkfilename . '.1'))
		unlink($linkfilename . '.1');

	$deletefailed=false;

	if (!$debugging) {

		//often there is no .2 data file returned and even then usually we dont have privileges to delete it
		if (is_file($linkfilename . '.2') && !unlink($linkfilename . '.2'))
			$deletefailed=true;

		//sometimes we dont have the privileges to delete the response file
		if (is_file($linkfilename . '.3') && !unlink($linkfilename . '.3'))
			$deletefailed=true;

	}

	//if we cannot delete one or other of the .2 data and .3 response files
	//then creating a .4 file instructs the db server to do it
	if ($deletefailed)
		file_put_contents($linkfilename . '.4', '');

//output

	//data and response is already preencoded by exodus server engine
	//so can be encapsulated in xml tags here without issue

	$xmltext = "<root>";
	$xmltext .= "<data>$data_out</data>";
	$xmltext .= "<response>$response</response>";
	$xmltext .= "<result>$result</result>";
	$xmltext .= "</root>";

	//Response.Expires = -1000
	//Response.Buffer = 0

	header ("Content-Type:text/xml; charset=utf-8");

	print($xmltext);

	debug("RESPONSE:$response");
	if ($data_out)
		debug("DATA_OUT:$data_out");
	if (!$result)
		debug("RESULT  :$result");
	//debug("XMLOUT  :$xmltext");

	/// finished

function getneosysrootpath($documentlocation) {
	return $_SERVER['DOCUMENT_ROOT'];
};

function neosysrnd($max, $min) {
	//return a random integer between 0 (or min) and max-1
	if (!$min)
		$min = 0;
	$max = $max - 1;
	//return (floor($min + rand(0,1) * ($max - $min + 1)))
	return mt_rand($min,$max);
};

function debug($msg) {
	global $debugging;
	if ($debugging)
		error_log("\"$msg\"" ,0);
}

function getdatabases($neosysrootpath, $systemcode) {

	//will look in /var/www/exodus/ for adagency.vol or accounts.vol

	//return an array of available database codes and names
	//or an empty array

	global $fm,$vm,$sm,$response;

	//if (!$systemcode)
		$systemcode = 'default';

	//get an array of databases
	$datalocation = $neosysrootpath . "/data/";
	$volfilename=$datalocation . $systemcode . '.vol';
	if (is_file($volfilename))
		$databases=file_get_contents($volfilename);
	else
		$databases='';

	//convert text format to mv format
	$databases=str_replace("\n",$fm,$databases);
	$databases=str_replace("\r",$fm,$databases);
	$databases=str_replace('*',$vm,$databases);
	$databases=str_replace(',',$sm,$databases);

	//the first line contains the pairs of dbname sm dbcode vm ...
	//$databases = explode($fm,$databases)[0];
	$databases = explode($fm,$databases)[0];
	//$databases=explode($vm,$databases);
	$databases=explode($vm,$databases);

	$xmltext = "<records>\r";
	foreach ($databases as $database) {
		//$database=explode($sm,$database.$sm);
		$database=explode($sm,$database.$sm);
		$databasename=$database[0];
		$databasecode=$database[1];

		//skip databases that dont exist
		if (!is_dir($datalocation . '/' . $databasecode))
			continue;

		$xmltext .= "<record>\r";
		$xmltext .= "<name>$databasename</name>\r";
		$xmltext .= "<code>$databasecode</code>\r";
		$xmltext .= "</record>\r";
	}
	$xmltext .= "</records>\r";

	//encode xml related characters since message to client is in xml format
	//will be decoded in client after unpacking xml message
	$xmltext=str_replace("%","%25",$xmltext);
	$xmltext=str_replace("<","%3C",$xmltext);
	$xmltext=str_replace('>',"%3E",$xmltext);
	$xmltext=str_replace('&',"%26",$xmltext);

	$response = "OK";

	return $xmltext;

}

?>
