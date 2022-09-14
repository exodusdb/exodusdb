////#include <boost/archive/iterators/binary_from_base64.hpp>
//#include <boost/archive/iterators/base64_from_binary.hpp>
//#include <boost/archive/iterators/transform_width.hpp>
////#include <boost/algorithm/string.hpp>
//
//std::string decode64(const std::string &val) {
//    using namespace boost::archive::iterators;
//    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
//    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
//        return c == '\0';
//    });
//}
//
//std::string encode64(const std::string &val) {
//    using namespace boost::archive::iterators;
//    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
//    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
//    return tmp.append((3 - val.size() % 3) % 3, '=');
//}

// To avoid loading the whole attached file into memory for encoding we now use
// linux os cmd "openssl" with option "base64" to pipe the encoding to a temp file

#include <exodus/library.h>
libraryinit()

#include <sendmail.h>
#include <shell2.h>
#include <log.h>

#include <sys_common.h>

var ccaddress;
var subject;
var body;
var attachfilename;
var deletex;
var replyto;
var params;
var maxemailsize;
var toaddress;
var savesentence;
var v5;
var v6;
var v8;
var forcedemailx;
var tt;//num
var params2;
var osfilename;
var osfilesize;//num
var bodyfilename;
var paramfilename;
var errorfilename;
var cmd;
var bodyfile;
var errors;
var errorfile;
var home = "";

function main(in toaddress0, in ccaddress0, in subject0, in body0, in attachfilename0, in delete0, out errormsg, in replyto0=var(), in params0=var()) {
	//c sys in,in,in,in,in,in,out,=var(),=var()

	//uses sendmail.js

	//global tt,maxemailsize,forcedemailx

	#include <system_common.h>

	if (ccaddress0.unassigned()) {
		ccaddress = "";
	} else {
		ccaddress = ccaddress0;
	}
	if (subject0.unassigned()) {
		subject = "";
	} else {
		subject = subject0;
	}
	if (body0.unassigned()) {
		body = "";
	} else {
		body = body0;
	}
	if (attachfilename0.unassigned()) {
		attachfilename = "";
	} else {
		attachfilename = attachfilename0;
	}
	if (delete0.unassigned()) {
		deletex = "";
	} else {
		deletex = delete0;
	}
	if (replyto0.unassigned()) {
		replyto = "";
	} else {
		replyto = replyto0;
	}
	if (params0.unassigned()) {
		params = "";
	} else {
		params = params0;
	}
	if (not deletex) {
		deletex = "";
	}

	//send files as links if too large (>10MB) for email to handle efficiently
	maxemailsize = 10 * 1024 * 1024;

	//sendmail is silent
	errormsg = "";

	if (SENTENCE.field(" ", 1).ucase() eq "SENDMAIL" and toaddress0.unassigned()) {
		toaddress = SENTENCE.field(" ", 2);
		if (not toaddress) {
			toaddress = "support@neosys.com";
		}
		SENTENCE.transfer(savesentence);
		call sendmail(toaddress, "", "test email test subject", "test body line 1" "\r" "test body line2", v5, v6, errormsg, v8, params);
		savesentence.transfer(SENTENCE);
		//if errormsg then call msg(toaddress:' ':@fm:errormsg)
		//call msg(toaddress:' ':@fm:errormsg)
		//if errormsg then call msg(toaddress:' ':@fm:errormsg)
		if (errormsg) {
			errormsg = (toaddress ^ " " ^ FM ^ errormsg).oconv("T#60");
			errormsg.converter(TM, FM);
			call mssg(errormsg);
		} else {
			var msg = "STEP 1 OK. Mail for " ^ toaddress ^ " accepted by mail server.";
			params.converter("\r\n", FM);
			msg(-1) = FM ^ "Sent using:" ^ FM ^ params;
			msg(-1) = "STEP 2. Now check if actually received by recipient to verify";
			msg(-1) = " that the mail server can actually deliver email to " ^ toaddress;
			msg(-1) = " and that " ^ toaddress ^ " can receive email from the server" ^ FM;
			msg = msg.oconv("T#75");
			msg.converter(TM, FM);
			call note(msg);
		}
		stop();
	} else {
		toaddress = toaddress0;

		// TODO All these neosys.com emails to be configurable in SYSTEM

		if (toaddress eq "TBQBB")
			toaddress = "support@neosys.com";

		//development systems ALWAYS email hardcoded in next line
		//1. exodus.id always indicates a test system (dos or exodus)
		//2. on exodus lack of live.txt file indicates a test system
		//if (var("exodus.id").osfile() or ((not(VOLUMES) and not(var("live.txt").osfile()))) or subject.contains("mvdbpostgres")) {
		if (var("exodus.id").osfile() or subject.contains("mvdbpostgres")) {
			forcedemailx = "dev@neosys.com";
			//toaddress=xlate('USERS','EXODUS',7,'X')
			//if toaddress else toaddress='dev@neosys.com'
			goto forcedemail;
		}

		//testdata and user exodus - always email sysmsg@neosys.com
		//if system<61> or (@username='EXODUS' and system<17,1>[-4,4]='TEST') then

		//testdata and user exodus - always email sysmsg@neosys.com
		home.osgetenv("HOME");
		//if (osfile(home ^ "/hosts/disabled.cfg") or (SYSTEM.f(61) and USERNAME eq "EXODUS") or osfile(home ^ "/hosts/serve_agy.disabled")) {
		if (not osfile("live.txt") or (SYSTEM.f(61) and USERNAME.ucase() eq "EXODUS")) {
			forcedemailx = "sysmsg@neosys.com";

forcedemail:
			subject ^= " (to:" ^ toaddress;
			toaddress = forcedemailx;
			if (ccaddress) {
				subject ^= " cc:" ^ ccaddress;
			}
			ccaddress = "";
			subject ^= ")";

		//each request can set a global forced email address
		//for the duration of the request
		} else if (SYSTEM.f(117)) {
			forcedemailx = SYSTEM.f(117);
			goto forcedemail;
		}

	}

	//limit development system to steve
	if (SYSTEM.contains("1EEC633B")) {
		toaddress = "steve.bush@neosys.com";
		ccaddress = "";
	}

	//cut off any cc addresses from the toaddress (after ;;)
	tt = toaddress.index(";;");
	if (tt) {
		if (ccaddress) {
			ccaddress ^= ";";
		}
		ccaddress ^= toaddress.substr(tt + 2, 9999);
		toaddress = toaddress.substr(1, tt - 1);
	}

	//send mail requires confirmation if user is EXODUS
	//CANNOT because backup in exodus.net would be interrupted
	//declare function decide
	//if @username='EXODUS' then
	// s33=system<33>
	// q='You are EXODUS.|Send mail to ':toaddress:'|':subject:'|':body0
	// convert "\r\n" to @fm in q
	// if decide(q,'',reply) else reply=2
	// if reply=1 else return
	// system<33>=s33
	// end

	var params1 = "";
	if (DEFINITIONS.open("DEFINITIONS", "")) {
		if (not(params1.read(DEFINITIONS, "SMTP.CFG"))) {
			{}
		}
	}

	//global override
	var filenamesx = "smtp.cfg" ^ VM ^ "../../smtp.cfg";
	filenamesx.converter("/", OSSLASH);
	for (const var filen : range(1, 2)) {
		call osread(params2, filenamesx.f(1, filen));
		//cut off after end of file character
		//params2=field(params2,char(26),1)
		params2.swapper("\r\n", "\r");
		params2.converter("\r\n", _FM _FM);
		for (const var ii : range(1, 9)) {
			if (params2.f(ii)) {
				params1(ii) = params2.f(ii);
			}
		} //ii;
	} //filen;

	//default exodus smtp parameters
	if (not(params1.f(1))) {
		var sysname = SYSTEM.f(57);
		if (not sysname) {
			sysname = "unknown";
		}
		params1(1) = sysname ^ "@neosys.com";
	}
	if (params1.f(2) eq "") {
		params1(2) = "mailout.neosys.com";
	}
	if (params1.f(3) eq "") {
		params1(3) = "2500";
	}

	params = "";
	params(-1) = "fromaddress=" ^ params1.f(1);
	params(-1) = "smtphostname=" ^ params1.f(2);
	params(-1) = "smtpportno=" ^ params1.f(3);
	params(-1) = "smtptimeoutsecs=" ^ params1.f(4);
	params(-1) = "smtpusessl=" ^ params1.f(5);
	params(-1) = "smtpauthtype=" ^ params1.f(6);
	params(-1) = "smtpuserid=" ^ params1.f(7);
	params(-1) = "smtppassword=" ^ params1.f(8);

	if (replyto) {
		params(-1) = "replyto=" ^ replyto;
	}

	var msgsize = 0;

	if (body[1] eq "@") {

		osfilename = body.substr(2, 99999);
		osfilesize = osfilename.osfile().f(1);

		//convert to link if file is too big to email
		if (osfilesize gt maxemailsize) {
			body = "";
			gosub addlinks2osfilename();
		} else {
			msgsize += osfilesize;
		}

	}

	msgsize += body.len();

	//convert to links if attachfile is too big to email
	//assumes attachfilename is in data dir accessible by web
	if (attachfilename) {
		attachfilename.osclose();

		osfilesize = attachfilename.osfile().f(1);
		if (osfilesize gt maxemailsize) {
			osfilename = attachfilename;
			gosub addlinks2osfilename();
			attachfilename = "";
		}
	}

	//for CDO attachfilename must be full path
	if (attachfilename) {

		attachfilename.swapper("\\\\", "\\");
		var cwd = oscwd();
		if (not(VOLUMES)) {
			cwd ^= OSSLASH;
		}
		if (attachfilename.substr(1, 2) eq "..") {
			attachfilename.splicer(1, 2, cwd.field(OSSLASH, 1, oscwd().count(OSSLASH) - 1));
		} else if (attachfilename.substr(1, 2) eq ".") {
			attachfilename.splicer(1, 1, cwd);
		}
		msgsize += attachfilename.osfile().f(1);

		attachfilename.swapper("\\", "\\\\");
	}

	//body=body0
	body.swapper(FM, "\r\n");

	//if index(body,' ',1) or len(body)>10 or index(body,\0D\,1) or index(body,\0A\,1) then
	if (body and (body[1] ne "@")) {
		bodyfilename = var(999999999).rnd().substr(1, 7) ^ ".tmp";
		//solve stupid outlook joining lines together if > 40 chars
		//by adding tab on the end of every line
		body.swapper("\r", "\t\r");
		call oswrite(body, bodyfilename);
		bodyfilename.osclose();
		body = "@" ^ bodyfilename;
	} else {
		body.converter(DQ, "'");
		bodyfilename = "";
	}

	//params='/t ':quote(toaddress):' /s ':quote(subject):' /b ':quote(body):' /a ':quote(attachfilename)
	//if delete then params:=' /d ':delete

	//condition subject start standard with 'EXODUS: '
	if (subject.substr(1, 8) ne "EXODUS: ") {
		if (subject.substr(1, 6) eq "EXODUS") {
			subject = subject.substr(7, 9999).trimf();
		}
		if (subject.substr(1, 7) eq "System:") {
			subject = subject.substr(8, 9999).trimf();
		}
		subject.splicer(1, 0, "EXODUS: ");
	}

	params(-1) = "toaddress=" ^ (toaddress.quote());
	if (ccaddress) {
		params(-1) = "ccaddress=" ^ (ccaddress.quote());
	}
	params(-1) = "subject=" ^ (subject.quote());
	params(-1) = "body=" ^ (body.quote());

	if (attachfilename) {
		params(-1) = "attachfilename=" ^ (attachfilename.quote());
	}
	if (deletex) {
		params(-1) = "deleteaftersend=" ^ (deletex.quote());
	}
	params ^= FM;

	//DOS uses EXODUS sendmail.js script which uses windows CDO object
	if (VOLUMES) {

		params.swapper(FM, "\r\n");
		paramfilename = var(999999999).rnd().substr(1, 7) ^ ".tmp";
		call oswrite(params, paramfilename);

		errorfilename = var(999999999).rnd().substr(1, 7) ^ ".tmp";

		//cmd='START /w'
		//using CSCRIPT because of difficulty reading errorfilename contents
		cmd = "cscript //Nologo";
		cmd ^= " sendmail.js /e " ^ errorfilename ^ " /p " ^ paramfilename;

	//otherwise use standard linux/posix mail program
	} else {

		cmd = "mail";

		paramfilename = "";
		errorfilename = "";

		//subject
		cmd ^= VM ^ "-s " ^ (subject.quote());

		//from
		var fromaddress = params1.f(1);
		cmd ^= VM ^ "-r " ^ (fromaddress.quote());

		//optional cc address
		if (ccaddress) {
			//cmd ^= VM ^ "-c " ^ (ccaddress.convert(";", ",").quote()); 
			cmd ^= VM ^ "-aCC: " ^ (ccaddress.convert(";", ",").quote());
		}

		//optional attach file
		if (attachfilename) {
			cmd ^= VM ^ "-A " ^ (attachfilename.quote());
		}

		//mark html formatted messages as such
		if (body[1] eq "@") {
			tt = "";
			if (bodyfile.osopen(body.substr(2, 999))) {
				//osbread tt from bodyfile at 0 length 100
				var offset = 0;
				call osbread(tt, bodyfile, offset, 100);
			}
		} else {
			tt = body.substr(1, 20);
		}
		tt.ucaser();
		if ((not(attachfilename) and tt.contains("<!DOCTYPE")) or tt.contains("<HTML")) {
			cmd ^= VM ^ "-a \"Content-Type: text/html\"";
			cmd ^= VM ^ "-a \"MIME-Version: 1.0\"";
		}

		//if there is a specific reply email address then give it
		if (replyto) {
			cmd ^= VM ^ "-a \"Reply-To: " ^ replyto ^ DQ;

		//otherwise request suppression of replies, particularly automatic ones
		} else {
			cmd ^= VM ^ "-a \"X-Auto-Response-Suppress: RN, NRN, OOF\"";
			//maybe Precedence: list would have better results
			cmd ^= VM ^ "-a \"Precedence: bulk\"";
		}

		//to address(es) go last
		cmd ^= VM ^ " " ^ toaddress.convert(";", " ");

		//ensure the email body is in a file
		if (bodyfilename eq "") {

			//body may already be in a file
			if (body[1] eq "@") {
				bodyfilename = body.substr(2, 99999);

			//otherwise generate a random filename and write it
			} else {
				bodyfilename = var(999999999).rnd().substr(1, 7) ^ ".tmp";
				var(body).oswrite(bodyfilename);
				bodyfilename.osclose();
			}

		}

		//use sendmail instead of mail if attachment
		//because strangely ubuntu mail doesnt support the -A option
		if (attachfilename) {
			var headers = "";
			var nn = cmd.count(VM) + 1;
			for (const var ii : range(2, nn)) {
				var line = cmd.f(1, ii);
				var opt = line.field(" ", 1);
				var arg = line.field(" ", 2, 9999);
				if (arg[1] eq DQ) {
					arg.splicer(1, 1, "");
					arg.popper();
				}

				if (opt eq "-r") {
					headers ^= VM ^ "From: " ^ arg;
					headers ^= VM ^ "To: " ^ toaddress;

				} else if (opt eq "-s") {
					headers ^= VM ^ "Subject: " ^ arg;

				} else if (opt eq "-c") {
					headers ^= VM ^ "cc: " ^ arg;

				} else if (opt eq "-a") {
					headers ^= VM ^ arg;

				//case opt='-A'
				}
			} //ii;

			headers.splicer(1, 1, "");
			headers ^= VM;
			headers.converter("'", "");
			headers.swapper(VM, "\r\n");

			//cmd = "neomail " ^ (toaddress.quote()) ^ " " ^ (attachfilename.quote()) ^ " " "'" ^ headers ^ "'";

			// -t  Extract recipients from message headers. These are added to any recipients specified on the command line.
			cmd = "sendmail -t " ^ toaddress;

			var delimiter = "--------------5723BF0875E398DEC19D9328--";

			// Use linux 'file' utility to determine mimetype
			var mimetype = osshellread("file --mime-type '" ^ attachfilename ^ "' | sed 's/.*: //'").convert("\r\n","");
			//TRACE(mimetype)

			var attachfilename_only = attachfilename.field2(OSSLASH, -1);
			var mimetext =
                "MIME-Version: 1.0\r\n"
                "Content-Type: multipart/mixed;\r\n"
                " boundary=\"------------5723BF0875E398DEC19D9328\"\r\n"
                "Content-Language: en-GB\r\n"
				"\r\n"
                "This is a multi-part message in MIME format.\r\n"
                "--------------5723BF0875E398DEC19D9328\r\n"
                "Content-Type: text/plain; charset=utf-8; format=flowed\r\n"
                "Content-Transfer-Encoding: 7bit\r\n"
				"\r\n"
                "--------------5723BF0875E398DEC19D9328\r\n"
                "Content-Type: " ^ mimetype ^ ";\r\n"
                " name=" ^ attachfilename_only.quote() ^ "\r\n"
                "Content-Transfer-Encoding: base64\r\n"
                "Content-Disposition: attachment;\r\n"
                " filename=" ^ attachfilename_only.quote() ^ "\r\n\r\n";
			headers ^= mimetext;

			//TRACE(headers)

			// Output the headers
			var tempfilename = var().ostempfilename();
			oswrite(headers, tempfilename);

//TRACE(headers);
			// Append the attached file as base64
			osshell("openssl base64 -in " ^ attachfilename.quote() ^ " >> " ^ tempfilename ^ " && printf \"\r\n" ^ delimiter ^ "\r\n\" >> " ^ tempfilename);
/*
			// Append a closing delimiter
			var fileinfo = osfile(tempfilename);
			var offset = fileinfo.f(1);
TRACE(offset)
			var osfile;
			if (osfile.osopen(tempfilename)) {

				offset = -1;//append
TRACE(offset)
				delimiter.osbwrite(osfile, offset);
TRACE(offset)

				var("\r\n").osbwrite(osfile, offset);
				headers.osbwrite(osfile, offset);
TRACE(offset)

			}
*/
			// Reconstruct the complete input for sendmail
			//tempfilename.osrename(bodyfilename);
			tempfilename.oscopy(bodyfilename);
			tempfilename.osremove();

		}

		//TRACE(cmd)
		cmd.converter(VM, " ");

		//and pipe the body file into the program as standard input
		tt = "\\";
		bodyfilename.converter(tt, OSSLASH);
		bodyfilename.swapper("$", "\\$");
		cmd ^= " < " ^ (bodyfilename.quote());

	}

	//printl();
	//TRACE(cmd)
	errormsg = shell2(cmd, errors);
	if (errormsg)
		TRACE(errormsg)
	if (errors)
		TRACE(errors)
	//pcperform 'CMD /c ':cmd

	if (bodyfilename) {
		bodyfilename.osremove();
	}
	if (paramfilename) {
		paramfilename.osremove();
	}

	if (errormsg) {
		{}
	} else if (errorfilename) {
		var().osflush();
		errorfilename.osclose();
		if (errorfile.osopen(errorfilename)) {
			printl(errorfilename, " ok");
		} else {
			printl(errorfilename, " ko");
		}
		if (not(errormsg.osread(errorfilename))) {
			errormsg = "Unknown error in sendmail.js Failed to complete";
			errormsg(-1) = cmd;
			//errormsg<-1>=params 'T#60'
			errormsg(-1) = params;
		}
	} else {
		if (errormsg eq "") {
			if (errors) {
				errormsg = errors;
				//call sysmsg(errors);
				errputl(errors);
			} else {
				errormsg = "OK";
			}
		}
	}
	errormsg.converter(TM, FM);

	if (errorfilename) {
		errorfilename.osremove();
	}

	var details = "To:       " ^ toaddress;
	if (ccaddress) {
		details(-1) = "cc:       " ^ ccaddress;
	}
	details(-1) = "Subject:  " ^ subject;
	details(-1) = "Size:     " ^ oconv(msgsize, "[XBYTES]");

	errormsg(1) = errormsg.f(1).trim();
	//in what situation can it return OK+message ??
	if (errormsg.f(1) eq "OK") {
		errormsg.remover(1);
		call log("SENDMAIL", details);
	} else {
		errormsg = trim(errormsg, FM, "B");
		//errormsg<-1>=@fm:'Size:     ':msgsize '[XBYTES]'
		//errormsg<-1>='From:     ':params1<1>
		//errormsg<-1>='To:       ':toaddress
		//if ccaddress then
		// errormsg<-1>='cc:       ':ccaddress
		// end
		errormsg(-1) = FM ^ details;
		errormsg(-1) = FM ^ "Server:   " ^ params1.f(2);
		errormsg(-1) = "Port:     " ^ params1.f(3);
		errormsg(-1) = "UseSSL:   " ^ params1.f(5);
		errormsg(-1) = "AuthType: " ^ params1.f(6);
		errormsg(-1) = "UserID:   " ^ params1.f(7);
		errormsg(-1) = "Password:";
		if (params1.f(8)) {
			errormsg ^= "********";
		}

		call log("SENDMAIL", errormsg);
		return 0;
	}

	return 1;
}

subroutine addlinks2osfilename() {
    tt = osfilename;
    //tt.converter("\\", "/");
    //if (tt.substr(1, 3) eq "../") {
    //  tt.splicer(1, 3, "");
    //}

    //attachment path must start with '/data/'
    //Apache will replace the alias /data/ to absolute path from /
    //eg /root/hosts/test/work/../data/test/xxxxxx.xls
    if (tt.contains("../")){
        //becomes: /data/test/xxxxxx.xls
        tt.splicer(1,tt.index("../")+2,"");
    }
    if (body) {
        body ^= FM;
    }
	if (body) {
		body ^= FM;
	}
	body(-1) = "Your report is too large to email. (" ^ oconv(osfilesize, "[XBYTES]") ^ ", max " ^ oconv(maxemailsize, "[XBYTES]") ^ ")";
	body(-1) = "but you can download it by clicking the following link.";
	body(-1) = FM ^ "*Link is only available for ONE HOUR from creation*";
	var nlinks = SYSTEM.f(114).count(VM) + 1;
	for (const var linkn : range(1, nlinks)) {
		body ^= FM;
		var linkdesc = SYSTEM.f(115, linkn);
		if (linkdesc) {
			body(-1) = linkdesc;
		}
		body(-1) = SYSTEM.f(114, linkn) ^ tt;
	} //linkn;
	return;
}

libraryexit()
