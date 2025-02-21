#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <shell2.h>

#include <service_common.h>

#include <srv_common.h>

var wgetrc;
var authurl;
var params;
var text;
//var hashcode;  // num
var errors;
//var log;

function main(in mode, in request, in tempfilename, out datax, out msg) {

	// mode is WRITE or READ
	// request is required sent to the server and
	// used to save the results in definitions CONTROL*request
	// tempfilename must be a valid dos style file eg '99999999' WITHOUT EXTENSION
	// data is required for WRITE and returned for READ
	// return 1 for ok or 0 for failure
	// msg returns any error information

	// http://www.gnu.org/software/wget/manual/wget.html
	// windows version at http://users.ugent.be/~bpuype/wget/

	let post = 1;
	// cleanup=0;*@username<>'EXODUS'
	let cleanup = mode == "READ";
	if (mode == "READ") {
		datax = "";
	}

	if (not tempfilename) {
		msg = "tempfilename parameter is missing";
		return 0;
	}

	// make linux/cygwin command
	var cmd		 = "wget";
	let httpsbug = 0;
//	if (VOLUMES) {
//		wgetrc = "";
//		// look for local or cygwin wget.exe otherwise quit
//		let exe = oscwd().contains(":") ? ".exe" : "";
//		cmd		= SYSTEM.f(50) ^ "wget" ^ exe;
//		if (not cmd.osfile()) {
//			httpsbug = 1;
//			cmd		 = "wget" ^ exe;
//		}
//		if (not cmd.osfile()) {
//			// avoid error in READ/UPGRADE phase of MONITOR2
//			// msg='wget.exe not available in CONTROL()'
//			msg = "";
//			return 0;
//		}
//	}

	// !!!
	// flush is REQUIRED and dir(xxx.exe) otherwise get an error in other processes
	// The process cannot access the file because it is being used by another process.
	var().osflush();

	msg = "unknown error in control";

	let logfilename	  = tempfilename ^ ".XWG";
	let tempfilename2 = tempfilename ^ ".XRE";

	// user and pass currently not required for some reason
	// but leave configured in case they are re-instated on the server
	let httpuser = "exodusclient";
	// Only one way to make a good password Memorable & Randomish"
	let httppass = "OowtmagpM&R";

	// stay within 31 bit integers for php sake
	let salt = 3923517;
	let max	 = 4752137;

	let sessionid = (date().pwr(3) * 7 + 5).b(9999, -9999);
	let cidx	  = cid();

	if (mode == "WRITE") {

		// baseurl='http://monitor.neosys.com/'
		// baseurl='https://monitor.hosts.neosys.com:4428/'
		var baseurl = "https://monitor.hosts.neosys.com";
		// add configured or default port
		var monitorport = SYSTEM.f(131);
		if (not monitorport) {
			monitorport = 4428;
		}
		if (monitorport != 443) {
			baseurl ^= ":" ^ monitorport;
		}
		baseurl ^= "/";

		if (request == "UPGRADE") {

			// only get if newer
			cmd ^= " -N";

			// nagios
			// authurl='http://monitor.neosys.com/upgrade.php'
			authurl = baseurl ^ "upgrade.php";

			// actually will be appended to URL because
			// http POST and -N option are incompatible with some
			// proxies that demand an HTTP LENGTH header for POST
			// but -N is supposed to not be used with POST
			params = datax;
		}

		if (request == "UPDATE") {

			authurl = SYSTEM.f(55);
			if (not authurl) {
				// authurl='http://monitor.neosys.com/monitor.php'
				authurl = baseurl ^ "monitor.php";
			}

			// cmd:=' --no-cache'

			// databaseid is eight character hexadecimal from DEFINITIONS GLOBALDATASETID
			// databasecode is the data folder
			// installationid is the computer no
			// sessionid is a hashcode of the day
			// eg databaseid=FFFFFFFF&databasecode=XXXXXXXX&installationid=999999&sessionid=

			let hostname = SYSTEM.f(57);

			// prepare params
			params = "databaseid=" ^ SYSTEM.f(45);
			params ^= "&databasecode=" ^ SYSTEM.f(17);
			params ^= "&installationid=" ^ cidx;
			params ^= "&sessionid=" ^ sessionid;
			params ^= "&request=" ^ request;
			if (hostname) {
				params ^= "&hostname=" ^ hostname;
			}
			params ^= "&data=" ^ datax;

			text = params;
			var hashcode;
			gosub hash(salt, max, hashcode);
			params.prefixer("hashcode=" ^ hashcode ^ "&");
		}

		// add basic wget commands
		let referer = "exodusclient-" ^ SYSTEM.f(17) ^ "-" ^ cidx;
		if (post) {
			cmd ^= " --no-cache";
			if (httpsbug) {
				cmd ^= " --no-check-certificate";
			}
			// cmd:=' -O ':tempfilename2
			cmd ^= " --output-file=" ^ logfilename;
			cmd ^= " --referer=" ^ referer;
			cmd ^= " --tries=3";
			cmd ^= " --timeout=60";
			cmd ^= " --background";
		} else {
			// wgetrc<-1>='output_document=':tempfilename2
			if (httpsbug) {
				wgetrc(-1) = "check-certificate=off";
			}
			wgetrc(-1) = "logfile=" ^ logfilename;
			wgetrc(-1) = "cache=off";
			wgetrc(-1) = "referer=" ^ referer;
			wgetrc(-1) = "tries=3";
			wgetrc(-1) = "timeout=10";
			wgetrc(-1) = "background=on";
		}

		// http proxy
		// !cmd:=' --http-proxy=':httpproxyhost
		// above option doesnt exist - must have http_proxy environment variable
		// or in a .wgetrc file
		// TODO exclude from proxy if in bypasslist in system<56,2>
		if (SYSTEM.f(56)) {

			if (post) {
				cmd ^= " --proxy=on";
				cmd ^= " --execute http_proxy=" ^ SYSTEM.f(56, 1);
			} else {
				wgetrc(-1) = "use_proxy=on";
				wgetrc(-1) = "http_proxy=" ^ SYSTEM.f(56, 1);
			}

			if (SYSTEM.f(56, 3)) {
				if (post) {
					cmd ^= " --proxy-user=" ^ SYSTEM.f(56, 3);
					cmd ^= " --proxy-password=" ^ SYSTEM.f(56, 4);
				} else {
					wgetrc(-1) = "proxy_user=" ^ SYSTEM.f(56, 3);
					wgetrc(-1) = "proxy_password=" ^ SYSTEM.f(56, 4);
				}
			}
		}
		// cmd:=' --execute https_proxy=xyz'

		// authorisation
		if (httpuser) {
			if (post) {
				cmd ^= " --user=" ^ httpuser;
				cmd ^= " --password=" ^ httppass;
			} else {
				wgetrc(-1) = "user=" ^ httpuser;
				wgetrc(-1) = "password=" ^ httppass;
			}
		}

		if (cleanup) {
			// logfilename.osremove();
			if (logfilename.osfile() and not logfilename.osremove()) {
				// abort(lasterror());
				loglasterror();
			}
		}

		// UPDATE also gets a response
		if (request == "UPDATE") {
			if (post) {
				cmd ^= " -O " ^ tempfilename2;
			} else {
				wgetrc(-1) = "output_document=" ^ tempfilename2;
			}
		}

		// always post at the moment
		if (post) {

			if (params) {
				if (cmd.contains(" -N ")) {
					params = "?data=" ^ params;
				} else {
					let datafilename = tempfilename ^ ".XDA";

					// encodeuri
					params.replacer("%", "%25");
					params.replacer("&", "%26");
					params.replacer("`", "%60");
					params.replacer("+", "%2B");

					//var(params).oswrite(datafilename);
					if (not var(params).oswrite(datafilename)) {
						loglasterror();
					}
					cmd ^= " --post-file=" ^ datafilename;
					// not added to authurl
					params = "";
				}
			}

			// encodeuri
			cmd.replacer("%", "%25");
			cmd.replacer("&", "%26");
			cmd.replacer("`", "%60");
			cmd.replacer("+", "%2B");

			// add url
			cmd ^= " " ^ ((authurl ^ params).quote());

			// if cleanup else oswrite cmd on tempfilename:'.CMD'

			// run the wget command
			// print @(0):@(-4):time() 'MTS':' CONTROL ':cmd[1,50]:'... ':

			var().osflush();
			let result = shell2(cmd, errors);
			var().osflush();

			// print 'done ':
			if (errors) {
				printl(errors);
			}

			if (cleanup) {
				//(tempfilename ^ ".CMD").osremove();
				if (osfile(tempfilename ^ ".CMD") and not (tempfilename ^ ".CMD").osremove()) {
					abort(lasterror());
				}
				// cannot delete postdata until command has executed
				// osremove datafilename
			}

		} else {

			// finally add location to get
			cmd ^= " " ^ ((authurl ^ "?" ^ params).quote());

			wgetrc.replacer(_FM, _EOL);
			var wgetrcfilename = oscwd() ^ tempfilename ^ ".XRC";
			//var(wgetrc).oswrite(wgetrcfilename);
			if (not var(wgetrc).oswrite(wgetrcfilename)) {
				loglasterror();
			}

			if (wgetrcfilename.at(2) == ":") {
				wgetrcfilename.paster(2, 1, "");
				wgetrcfilename.prefixer("/cygdrive/");
			}
			wgetrcfilename.converter(OSSLASH, "/");

			let cmdfilename	  = tempfilename ^ ".cmd";
			let errorfilename = tempfilename ^ ".XER";
			var cmdfile		  = "set WGETRC=" ^ wgetrcfilename;
			cmdfile ^= _EOL ^ cmd ^ " 2>" ^ errorfilename;
			//var(cmdfile).oswrite(cmdfilename);
			if (not var(cmdfile).oswrite(cmdfilename)) {
				loglasterror();
			}

			// print
			// print 'CONTROL cmd /c ':cmdfilename:' ... ':

			var().osflush();
			//("cmd /c " ^ cmdfilename).osshell();
			if (not ("cmd /c " ^ cmdfilename).osshell()) {
				loglasterror();
			}
			var().osflush();

			// print 'done'

			if (not errors.osread(errorfilename)) {
				errors = "";
			}

			if (cleanup) {
				//cmdfilename.osremove();
				if (cmdfilename.osfile() and not cmdfilename.osremove()) {
					abort(lasterror());
				}
				//wgetrcfilename.osremove();
				if (wgetrcfilename.osfile() and not wgetrcfilename.osremove()) {
					abort(lasterror());
				}
				//errorfilename.osremove();
				if (errorfilename.osfile() and not errorfilename.osremove()) {
					abort(lasterror());
				}
			}
		}

		// catches option errors
		if (errors) {
			errors.converter("\r\n", _FM _FM);
			errors.trimmer(_FM);
			call note(trim(errors, _FM));
		}

		msg = "";

	} else if (mode == "READ") {

		var log;
		let tt = tempfilename2.osfile().f(1);
		// if tt > 32000 then
		if (tt > maxstrsize_ / 2) {
			datax = "";
			gosub getlog(logfilename, cleanup, log);
			msg = "Excessive response (" ^ tt ^ ")" ^ log;
			return 0;
		}
		if (datax.osread(tempfilename2)) {
			if (cleanup) {
				// tempfilename2.osremove();
				if (tempfilename2.osfile() and not tempfilename2.osremove()) {
					// abort(lasterror());
					loglasterror();
				}

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

		var hashcode;

		// verify echo
		let expect = SYSTEM.f(45) ^ "," ^ SYSTEM.f(17) ^ "," ^ cidx ^ "," ^ sessionid;
		if (datax.field(",", 2, 4) != expect) {
			msg = "Bad response";
			goto badresponse;
		}

		// verify response or abort
		text = datax.field(",", 2, 999999);
		gosub hash(salt, max, hashcode);
		hashcode = hashcode.b(9999, -9999);
		if (datax.field(",", 1) != hashcode) {
			msg = "wrong response";
badresponse:
			datax.replacer("<br />", _FM);
			gosub	   getlog(logfilename, cleanup, log);
			msg ^= _FM _FM ^ datax ^ log;
			msg = msg.oconv("T#60");
			msg.converter(_TM, _FM);
			msg.trimmer();
			return 0;
		}

		// from here on the response is ok
		msg = "";
		if (cleanup) {
			// logfilename.osremove();
			if (logfilename.osfile() and not logfilename.osremove()) {
				// abort(lasterror());
				loglasterror();
			}
		}

		datax = datax.field(",", 14, 999999);

		msg = "";
	}

	return 1;
}

subroutine getlog(in logfilename, in cleanup, out log) {

	if (log.osread(logfilename)) {
		if (cleanup) {
			// logfilename.osremove();
			if (logfilename.osfile() and not logfilename.osremove()) {
				// abort(lasterror());
				loglasterror();
			}
		}
		log.converter("\r\n", _FM _FM);
		log.trimmer(_FM);
		log.replacer("<br />", _FM);
		log = trim(log, _FM);
		log = field(log, _FM, -1);
		if (log) {
			log.prefixer(_FM _FM);
		}
	} else {
		log = "";
	}
	return;

	// request is
	// the computer number
	// a request number is a hashcode of the date

	// response is
	// various authorisation data
	// a magic number

	// the magic number is calculated as
	// 1. the computer number plus the request number plus a hash of the auth data
	// 2. the first eight digits of the sin of the above
	// 3. reversed
}

subroutine hash(in salt, in max, out hashcode) {

	hashcode = salt;
	for (const int ii : range(1, text.len())) {
		hashcode = (hashcode * (text.at(ii)).seq()).mod(max);
	}  // ii;
	return;
}

libraryexit()
