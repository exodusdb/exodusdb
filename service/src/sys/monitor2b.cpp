#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <shell2.h>

#include <gen_common.h>

var httpsbug;//num
var authurl;
var params;
var text;
var hashcode;//num
var errors;
var log;

function main(in mode, in request, in tempfilename, out datax, out msg) {
	//c sys in,in,in,out,out

	#include <general_common.h>

	//mode is WRITE or READ
	//request is required sent to the server and
	// used to save the results in definitions CONTROL*request
	//tempfilename must be a valid dos style file eg '99999999' WITHOUT EXTENSION
	//data is required for WRITE and returned for READ
	//return 1 for ok or 0 for failure
	//msg returns any error information

	//http://www.gnu.org/software/wget/manual/wget.html
	//windows version at http://users.ugent.be/~bpuype/wget/

	var post = 1;
	//cleanup=0;*@username<>'EXODUS'
	var cleanup = mode eq "READ";
	if (mode eq "READ") {
		datax = "";
	}

	if (not tempfilename) {
		msg = "tempfilename parameter is missing";
		return 0;
	}

	//make cygwin command
	var wgetrc = "";
	//look for local or cygwin wget.exe otherwise quit
	//WARNING TODO: check ternary op following;
	var exe = oscwd().index(":") ? ".exe" : "";
	var cmd = SYSTEM.a(50) ^ "wget" ^ exe;
	if (cmd.osfile()) {
		httpsbug = 0;
	} else {
		httpsbug = 1;
		cmd = "wget" ^ exe;
	}

	if (not(cmd.osfile())) {
		//avoid error in READ/UPGRADE phase of MONITOR2
		//msg='wget.exe not available in CONTROL()'
		msg = "";
		return 0;
	}

	//!!!
	//flush is REQUIRED and dir(xxx.exe) otherwise get an error in other processes
	//The process cannot access the file because it is being used by another process.
	var().osflush();

	msg = "unknown error in control";

	var logfilename = tempfilename ^ ".$WG";
	var tempfilename2 = tempfilename ^ ".$RE";

	//user and pass currently not required for some reason
	//but leave configured in case they are re-instated on the server
	var httpuser = "exodusclient";
	//Only one way to make a good password Memorable & Randomish"
	var httppass = "OowtmagpM&R";

	//stay within 31 bit integers for php sake
	var salt = 3923517;
	var max = 4752137;

	var sessionid = (var().date().pwr(3) * 7 + 5).substr(9999, -9999);
	var cidx = cid();

	if (mode eq "WRITE") {

		//baseurl='http://monitor.neosys.com/'
		//baseurl='https://monitor.hosts.neosys.com:4428/'
		var baseurl = "https://monitor.hosts.neosys.com";
		//add configured or default port
		var monitorport = SYSTEM.a(131);
		if (not monitorport) {
			monitorport = 4428;
		}
		if (monitorport ne 443) {
			baseurl ^= ":" ^ monitorport;
		}
		baseurl ^= "/";

		if (request eq "UPGRADE") {

			//only get if newer
			cmd ^= " -N";

			//nagios
			//authurl='http://monitor.neosys.com/upgrade.php'
			authurl = baseurl ^ "upgrade.php";

			//actually will be appended to URL because
			//http POST and -N option are incompatible with some
			//proxies that demand an HTTP LENGTH header for POST
			//but -N is supposed to not be used with POST
			params = datax;

		}

		if (request eq "UPDATE") {

			authurl = SYSTEM.a(55);
			if (not authurl) {
				//authurl='http://monitor.neosys.com/monitor.php'
				authurl = baseurl ^ "monitor.php";
			}

			//cmd:=' --no-cache'

			//databaseid is eight character hexadecimal from DEFINITIONS GLOBALDATASETID
			//databasecode is the data folder
			//installationid is the computer no
			//sessionid is a hashcode of the day
			//eg databaseid=FFFFFFFF&databasecode=XXXXXXXX&installationid=999999&sessionid=

			var hostname = SYSTEM.a(57);

			//prepare params
			params = "databaseid=" ^ SYSTEM.a(45);
			params ^= "&databasecode=" ^ SYSTEM.a(17);
			params ^= "&installationid=" ^ cidx;
			params ^= "&sessionid=" ^ sessionid;
			params ^= "&request=" ^ request;
			if (hostname) {
				params ^= "&hostname=" ^ hostname;
			}
			params ^= "&data=" ^ datax;

			text = params;
			gosub hash(salt, max, hashcode);
			params.splicer(1, 0, "hashcode=" ^ hashcode ^ "&");

		}

		//add basic wget commands
		var referer = "exodusclient-" ^ SYSTEM.a(17) ^ "-" ^ cidx;
		if (post) {
			cmd ^= " --no-cache";
			if (httpsbug) {
				cmd ^= " --no-check-certificate";
			}
			//cmd:=' -O ':tempfilename2
			cmd ^= " -o " ^ logfilename;
			cmd ^= " --referer=" ^ referer;
			cmd ^= " -t 3";
			cmd ^= " -T 60";
			cmd ^= " -b";
		} else {
			//wgetrc<-1>='output_document=':tempfilename2
			if (httpsbug) {
				wgetrc.r(-1, "check-certificate=off");
			}
			wgetrc.r(-1, "logfile=" ^ logfilename);
			wgetrc.r(-1, "cache=off");
			wgetrc.r(-1, "referer=" ^ referer);
			wgetrc.r(-1, "tries=3");
			wgetrc.r(-1, "timeout=10");
			wgetrc.r(-1, "background=on");
		}

		//http proxy
		//!cmd:=' --http-proxy=':httpproxyhost
		//above option doesnt exist - must have http_proxy environment variable
		//or in a .wgetrc file
		//TODO exclude from proxy if in bypasslist in system<56,2>
		if (SYSTEM.a(56)) {

			if (post) {
				cmd ^= " --proxy=on";
				cmd ^= " --execute http_proxy=" ^ SYSTEM.a(56, 1);
			} else {
				wgetrc.r(-1, "use_proxy=on");
				wgetrc.r(-1, "http_proxy=" ^ SYSTEM.a(56, 1));
			}

			if (SYSTEM.a(56, 3)) {
				if (post) {
					cmd ^= " --proxy-user=" ^ SYSTEM.a(56, 3);
					cmd ^= " --proxy-password=" ^ SYSTEM.a(56, 4);
				} else {
					wgetrc.r(-1, "proxy_user=" ^ SYSTEM.a(56, 3));
					wgetrc.r(-1, "proxy_password=" ^ SYSTEM.a(56, 4));
				}
			}

		}
		//cmd:=' --execute https_proxy=xyz'

		//authorisation
		if (httpuser) {
			if (post) {
				cmd ^= " --user=" ^ httpuser;
				cmd ^= " --password=" ^ httppass;
			} else {
				wgetrc.r(-1, "user=" ^ httpuser);
				wgetrc.r(-1, "password=" ^ httppass);
				}
			}

		if (cleanup) {
			logfilename.osdelete();
		}

		//UPDATE also gets a response
		if (request eq "UPDATE") {
			if (post) {
				cmd ^= " -O " ^ tempfilename2;
			} else {
				wgetrc.r(-1, "output_document=" ^ tempfilename2);
			}
		}

		//always post at the moment
		if (post) {

			if (params) {
				if (cmd.index(" -N ")) {
					params = "?data=" ^ params;
				} else {
					var datafilename = tempfilename ^ ".$DA";

					//encodeuri
					params.swapper("%", "%25");
					params.swapper("&", "%26");
					params.swapper("`", "%60");
					params.swapper("+", "%2B");

					var(params).oswrite(datafilename);
					cmd ^= " --post-file=" ^ datafilename;
					//not added to authurl
					params = "";
				}
			}

			//encodeuri
			cmd.swapper("%", "%25");
			cmd.swapper("&", "%26");
			cmd.swapper("`", "%60");
			cmd.swapper("+", "%2B");

			//add url
			cmd ^= " " ^ ((authurl ^ params).quote());

	//if cleanup else oswrite cmd on tempfilename:'.CMD'

			//run the wget command
			output(at(-40), var().time().oconv("MTS"), " CONTROL ", cmd.substr(1, 50), "... ");

			//garbagecollect;
			var().osflush();
			var result = shell2(cmd, errors);
			var().osflush();

			print("done ");
			if (errors) {
				printl(errors);
			}

			if (cleanup) {
				(tempfilename ^ ".CMD").osdelete();
				//cannot delete postdata until command has executed
				//osdelete datafilename
			}

		} else {

			//finally add location to get
			cmd ^= " " ^ ((authurl ^ "?" ^ params).quote());

			wgetrc.swapper(FM, "\r\n");
			var wgetrcfilename = oscwd() ^ tempfilename ^ ".$RC";
			var(wgetrc).oswrite(wgetrcfilename);

			if (wgetrcfilename[2] eq ":") {
				wgetrcfilename.splicer(2, 1, "");
				wgetrcfilename.splicer(1, 0, "/cygdrive/");
			}
			wgetrcfilename.converter(OSSLASH, "/");

			var cmdfilename = tempfilename ^ ".cmd";
			var errorfilename = tempfilename ^ ".$ER";
			var cmdfile = "set WGETRC=" ^ wgetrcfilename;
			cmdfile ^= "\r\n" ^ cmd ^ " 2>" ^ errorfilename;
			var(cmdfile).oswrite(cmdfilename);

			printl();
			print("CONTROL cmd /c ", cmdfilename, " ... ");

			//garbagecollect;
			var().osflush();
			("cmd /c " ^ cmdfilename).osshell();
			var().osflush();

			printl("done");

			if (not(errors.osread(errorfilename))) {
				errors = "";
			}

			if (cleanup) {
				cmdfilename.osdelete();
				wgetrcfilename.osdelete();
				errorfilename.osdelete();
			}

		}

		//catches option errors
		if (errors) {
			errors.converter("\r\n", FM ^ FM);
			call note(trim(errors, FM));
		}

		msg = "";

	} else if (mode eq "READ") {

		var tt = tempfilename2.osfile().a(1);
		//if tt gt 32000 then
		if (tt gt maxstrsize_ / 2) {
			datax = "";
			gosub getlog(logfilename, cleanup, log);
			msg = "Excessive response (" ^ tt ^ ")" ^ log;
			return 0;
		}
		if (datax.osread(tempfilename2)) {
			if (cleanup) {
				tempfilename2.osdelete();
			}
		} else {
			datax = "";
			gosub getlog(logfilename, cleanup, log);
			msg = "No response file" ^ log;
			return 0;
		}

		if (not datax) {
			gosub getlog(logfilename, cleanup, log);
			msg = "No response" ^ log;
			return 0;
		}

		//verify echo
		var expect = SYSTEM.a(45) ^ "," ^ SYSTEM.a(17) ^ "," ^ cidx ^ "," ^ sessionid;
		if (datax.field(",", 2, 4) ne expect) {
			msg = "Bad response";
			goto badresponse;
		}

		//verify response or abort
		text = datax.field(",", 2, 999999);
		gosub hash(salt, max, hashcode);
		hashcode = hashcode.substr(9999, -9999);
		if (not(datax.field(",", 1) eq hashcode)) {
			msg = "wrong response";
badresponse:
			datax.swapper("<br />", FM);
			gosub getlog(logfilename, cleanup, log);
			msg ^= FM ^ FM ^ datax ^ log;
			msg = msg.oconv("T#60");
			msg.converter(TM, FM);
			msg.trimmer();
			return 0;
		}

		//from here on the response is ok
		msg = "";
		if (cleanup) {
			logfilename.osdelete();
		}

		datax = datax.field(",", 14, 999999);

		msg = "";

	}

	return 1;
}

subroutine getlog(in logfilename, in cleanup, out log) {

	if (log.osread(logfilename)) {
		if (cleanup) {
			logfilename.osdelete();
		}
		log.converter("\r\n", FM ^ FM);
		log.swapper("<br />", FM);
		log = trim(log, FM);
		log = field2(log, FM, -1);
		if (log) {
			log.splicer(1, 0, FM ^ FM);
		}
	} else {
		log = "";
	}
	return;

	//request is
	//the computer number
	//a request number is a hashcode of the date

	//response is
	//various authorisation data
	//a magic number

	//the magic number is calculated as
	//1. the computer number plus the request number plus a hash of the auth data
	//2. the first eight digits of the sin of the above
	//3. reversed

}

subroutine hash(in salt, in max, out hashcode) {

	hashcode = salt;
	for (var ii = 1; ii <= text.length(); ++ii) {
		hashcode = (hashcode * (text[ii]).seq()) % max;
	} //ii;
	return;
}

libraryexit()
