<?php

//defaults

	//mb_internal_encoding('UTF-8');
	//mb_regex_encoding('UTF-8');

	//php5 doesnt recognise "\u255e" etc.
	$fm = '\x25\x5e';
	$vm = '\x25\x5d';
	$sm = '\x25\x5c';

	$gautostartdatabase = true;//unless ../exodus/net.cfg first line is AUTOSTART=NO
	$gsecondstowaitforreceipt = 10;
	$gsecondstowaitforstart = 30;

	$cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER';
	$databasestoppedresponse = "The database service is stopped.\r\rPlease try again later or ask your\rtechnical support staff to start it.";
	$norequestresponse = 'ERROR: NO REQUEST';
	$nodataresponse = 'ERROR: NO DATA';
	$invaliddatapathresponse = 'ERROR: CANNOT WRITE TO ';

	$loginlocation = '../default.htm';

	//default timeout is 10 minutes (NB BACKEND timeout (in GIVEWAY) is hard coded to 10 mins?)
	$defaulttimeoutmins = 10;

// session

	$neosysrootpath = getneosysrootpath($_SERVER['DOCUMENT_ROOT']);
	$datalocation = ($neosysrootpath . '/data/');

	if (($neosysrootpath . "UNICODE.INI"))
		$unicode = -1;
	else
		$unicode = 0;

	//get and cleanup hostname
	//remove space . " ' characters
	//and take first and last four characters if longer than 8 characters. why?
	$localhostname = $_SERVER['SERVER_NAME'];
	$localhostname = str_replace(array(".", ".", " "),"_",$localhostname);
	if (mb_strlen($localhostname) > 8)
		$localhostname = mb_substr($localhostname,0, 4) . mb_substr($localhostname,-4);

	$remoteaddr = $_SERVER['REMOTE_ADDR'];
	$remotehost = $_SERVER['REMOTE_HOST'];
	$https = $_SERVER['HTTPS'];

//request

	//client delivers a request in xml format
	$xml = simplexml_load_string($HTTP_RAW_POST_DATA);
//	debug("POST   ---> ".$HTTP_RAW_POST_DATA);

debug("REQUEST :".$xml->request);

	//request contains token, request, database and data
	$token=rawurldecode($xml->token);
	$request=rawurldecode($xml->request);
	$database=rawurldecode($xml->dataset);//probably come as DATASET not DATABASE
	$data=rawurldecode($xml->data);

	//convert any crlf to just cr
	//neosys uses \r which is \cr because \n and lf are messed about as either 0d0a or just 0a
	$request = str_replace("\n","\r",$request);
	$token = str_replace("\n","\r",$token);

	$requests = mb_split("\r",$request . "\r\r\r\r\r\r\r\r");

	//try and get the username, password and database from the session
	session_start();//
	$username = $_SESSION[$token . '_username'];
	$password = $_SESSION[$token . '_password'];
	$database = $_SESSION[$token . '_database'];
	$system = $_SESSION[$token . '_system'];
	$timeout = $_SESSION[$token . '_timeout'];

	//if ($username=='NEOSYS')
		$defaulttimeoutmins=5;
	if (!$timeout)
		$timeout = $defaulttimeoutmins * 60;

	//seconds for script timeout is our timeout plus 60 seconds
	//TODO Server.ScriptTimeout = $timeout / 1000 + 60;

	//either login
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
	}

	//append trailing '/' to databasedir if necessary
	$databasedir = $database . "/";

	$globalserverfilename = $datalocation . $databasedir . $database . '.SVR';

// attempt(s)

	//continue=repeated attempts after attempting to start database
	//break = failures or success
	while (1)
	{

		//failure
		$data = '';
		$response = '';
		$result = 0;
		$linkfilename = '';

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
			$data=getdatabases($neosysrootpath,$requests[1]);
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

		//write data if any
		//(before request so that there is no sharing violation with server reader)
		if ($data) {

			//tf.Write(escape($data))
			//allow field marks to pass through unicode conversion untouched to the backend as escaped characters
			//only really need the four characters FB-FE
			//$temp = unicode2escapedfms($data)
			//unicode to codepage conversion used to take place here in win/IIS

			if (!file_put_contents($linkfilename . '.2', $data)) {
				$response = $invaliddatapathresponse . ' "' . $linkfilename . '.2" ';
				break;
			}

		}

		//more info in request			
		if (!$remoteaddr)
			$remoteaddr = '';
		if (!$remotehost)
			$remotehost = '';
		if (!$https)
			$https = '';

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

		//now that everything is in place
		//rename the command file to end in .1
		//so that server/listener pick it up
		//otherwise abort
		if (!rename($linkfilename . '.1$', $linkfilename .'.1')) {
			$response = 'Cannot rename "' . $linkfilename . ".1$\"\rto " . $linkfilename . '.1"';
			break;
		}

		//wait for 10 seconds for the request to disappear
		//sleeping between checks every 10 ms
		$waituntil = time() + $gsecondstowaitforreceipt;
		while (is_file($linkfilename . '.1') && time() < $waituntil ){
			//TODO increase sleep if not fast result
			usleep(10000);
		}

		// otherwise abort
		//TODO start the database and try again
		//TODO really should rename to take control back of request file to ensure it isnt processed
		if (time() >= $waituntil) {
			$response = 'Error: No response in ' . $gsecondstowaitforreceipt . ' seconds from database server at ' . $linkfilename . '.1';
			break;
		}

		//wait patiently for 10 minutes for the response to appear otherwise quit
		//sleep between checking every 10 ms
		$waituntil = time() + $timeout;
		while (!is_file($linkfilename . '.3') && time() < $waituntil) {

			//sleep for 10ms before checking for response again
			//TODO make it sleep longer the long the delay
			// so response is fast if the server is fast but saves cpu if not
			//(after waiting 10 seconds it is pointless to check every 10ms)
			usleep(10000);//10,000 microseconds = 10 ms

		}

		//abort if no response within x seconds
		if (time() >= $waituntil) {
			$response = 'Error: No response in ' . $timeout . ' seconds from database server at ' . $linkfilename;
			break;
		}

		//read response in .2 file

		//response determines true or false
		if (!file_get_contents($linkfilename . '.3')) {
			$response = 'Error: Cannot read response file ' . $linkfilename;
			break;
		}
		//$response = escapedfms2unicode(tf.ReadAll())
		if (mb_split(' ',$response)[0] == 'OK') 
			$result = 1;
		else
			$result = 0;
		
		//read the data .2 file or empty data if not

		//(moved from below so can return data even if not "OK")
		if (!file_get_contents($linkfilename . '.2',$data))
			$data='';
		//$data = escapedfms2unicode(tf.ReadAll())

		//using a "loop" as a way to jump forward to abort or succeed anywhere or at end (break statements)
		//only looping if starting database
		break;

	}//end of attempt "loop"

// clean up

	//normally .1 will have been consumed (renamed, read and then deleted) by the neosys database and cannot be deleted
	if (is_file($linkfilename . '.1'))
		unlink($linkfilename . '.1');

	//if we cannot delete one or other of the .2 data and .3 response files
	//then creating a .4 file instructs the db server to do it
	$deletefailed=false;

	//often there is no .2 data file returned and and even then usually we dont have privileges to delete it
	if (is_file($linkfilename . '.2') && !unlink($linkfilename . '.2'))
		$deletefailed=true;

	//sometimes we dont have the privileges to delete the response file
	if (is_file($linkfilename . '.3') && !unlink($linkfilename . '.3'))
		$deletefailed=true;

	if ($deletefailed)
		file_put_contents($linkfilename . '.4', '');

// output

	$xmltext = "<root>";
	$xmltext .= "<data>" . rawurlencode($data) . "</data>";
	$xmltext .= "<response>" . rawurlencode($response) . "</response>";
	$xmltext .= "<result>" . rawurlencode($result) . "</result>";
	$xmltext .= "</root>";

	//Response.ContentType = "text/xml"
	//Response.Expires = -1000
	//Response.Buffer = 0

	header ("Content-Type:text/xml");
	print($xmltext);

//debug("RESPONSE:$response");
//if ($data)
//debug("DATA    :$data");
//if (!$result)
//debug("RESULT  :$result");
	
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

//debug in php like this
//tail -f /var/log/apache2/error.log
function debug($msg) {
	error_log($msg,0);
}

function getdatabases($neosysrootpath, $systemcode) {

	//will look in /var/www/exodus/ for adagency.vol or accounts.vol

	//return an array of available database codes and names
	//or an empty array

	global $fm,$vm,$sm;

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
	//$databases = mb_split($fm,$databases)[0];
	$databases = explode($fm,$databases)[0];
	//$databases=mb_split($vm,$databases);
	$databases=explode($vm,$databases);

	$xmltext = "<records>\r";
	foreach ($databases as $database) {
		//$database=mb_split($sm,$database.$sm);
		$database=explode($sm,$database.$sm);
		$databasename=$database[0];
		$databasecode=$database[1];

		if (!is_dir($datalocation . '/' . $databasecode))
			continue;
		$xmltext .= "<record>\r";
		$xmltext .= "<name>$databasename</name>\r";
		$xmltext .= "<code>$databasecode</code>\r";
		$xmltext .= "</record>\r";
	}
	$xmltext .= "</records>\r";

	return $xmltext;

}

?>
