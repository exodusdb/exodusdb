#include <exodus/library.h>
libraryinit()

#include <log.h>
#include <sendmail.h>

var ccaddress;
var subject;
var body;
var attachfilename;
var deletex;
var replyto;
var toaddress;
var savesentence;
var v5;
var v6;
var v8;
var params;
var params2;
var bodyfilename;

function main(in toaddress0, in ccaddress0, in subject0, in body0, in attachfilename0, in delete0, out errormsg, in replyto0=var(), in params0=var()) {
	//uses sendmail.js

	if (ccaddress0.unassigned()) {
		ccaddress = "";
	}else{
		ccaddress = ccaddress0;
	}
	if (subject0.unassigned()) {
		subject = "";
	}else{
		subject = subject0;
	}
	if (body0.unassigned()) {
		body = "";
	}else{
		body = body0;
	}
	if (attachfilename0.unassigned()) {
		attachfilename = "";
		}else{
		attachfilename = attachfilename0;
	}
	if (delete0.unassigned()) {
		deletex = "";
	}else{
		deletex = delete0;
	}
	if (replyto0.unassigned()) {
		replyto = "";
	}else{
		replyto = replyto0;
	}
	if (params0.unassigned()) {
		params = "";
	}else{
		params = replyto0;
	}
	if (not deletex) {
		deletex = "";
	}

	//sendmail is silent
	errormsg = "";
	var forcedemail;

	if (SENTENCE.field(" ", 1) == "SENDMAIL" and toaddress0.unassigned()) {
		toaddress = SENTENCE.field(" ", 2);
		if (not toaddress) {
			toaddress = "sb2@neosys.com";
		}
		SENTENCE.transfer(savesentence);
		call sendmail(toaddress, "", "test email test subject", "test body line 1" "\r" "test body line2", v5, v6, errormsg, v8, params);
		savesentence.transfer(SENTENCE);
		//if errormsg then call msg(toaddress:' ':fm:errormsg)
		//call msg(toaddress:' ':fm:errormsg)
		//if errormsg then call msg(toaddress:' ':fm:errormsg)
		if (errormsg) {
			errormsg = (toaddress ^ " " ^ FM ^ errormsg).oconv("T#60");
			errormsg.converter(TM, FM);
			call mssg(errormsg);
		}else{
			var msg = "Step 1 OK. Mail for " ^ toaddress ^ " accepted by mail server.";
			msg.r(-1, FM ^ "Step 2. Now check if actually received by recipient");
			msg.r(-1, "to verify that the mail server can actually");
			msg.r(-1, "deliver email to " ^ toaddress ^ FM);
			params.converter("\r\n", FM);
			msg.r(-1, "Sent using:" ^ FM ^ params);
			msg = msg.oconv("T#75");
			msg.converter(TM, FM);
			call note(msg);
		}
		return 0;
	}else{

		toaddress = toaddress0;

		//development systems ALWAYS email hardcoded SB2@NEOSYS.COM in next line
		if (var("NEOSYS.ID").osfile()) {
			forcedemail = "sb2@neosys.com";
			//toaddress=xlate('USERS','NEOSYS',7,'X')
			//if toaddress else toaddress='sb2@neosys.com'
			goto forcedemail;
		}

		//testdata and user neosys ... always email sysmsg@neosys.com
		//if system<61> or (@username='NEOSYS' and system<17,1>[-4,4]='TEST') then

		//testdata and user neosys ... always email sysmsg@neosys.com
		if (SYSTEM.a(61) and USERNAME == "NEOSYS") {
			forcedemail = "sysmsg@neosys.com";

forcedemail:
			subject ^= " (to:" ^ toaddress;
			toaddress = forcedemail;
			if (ccaddress) {
				subject ^= " cc:" ^ ccaddress;
			}
			ccaddress = "";
			subject ^= ")";

			//each request can set a global forced email address
			//for the duration of the request
		//	goto 513;
		} else if (SYSTEM.a(117)) {
			forcedemail = SYSTEM.a(117);
			goto forcedemail;
		}
//L513:
	}

	//limit neosys accounts email to chrystalla and steve
	if (SYSTEM.index("PT9115", 1) or SYSTEM.index("1EEC633B", 1) or SYSTEM.index("PRAGMATIC", 1)) {
		toaddress = "chrystallabush@neosys.com";
		ccaddress = "steve.bush@neosys.com";
	}

	//cut off any cc addresses from the toaddress (after ;;)
	var tt = toaddress.index(";;", 1);
	if (tt) {
		if (ccaddress) {
			ccaddress ^= ";";
		}
		ccaddress ^= toaddress.substr(tt + 2, 9999);
		toaddress = toaddress.substr(1, tt - 1);
	}

	//send mail requires confirmation if user is NEOSYS
	//CANNOT because backup in neosys.net would be interrupted
	//declare function decide
	//if @username='NEOSYS' then
	// s33=system<33>
	// q='You are NEOSYS.|Send mail to ':toaddress:'|':subject:'|':body0
	// convert \0D0A\ to @fm in q
	// if decide(q,'',reply) else reply=2
	// if reply=1 else return 0
	// system<33>=s33
	// end

	var params1 = "";
	var _definitions;
	if (_definitions.open("DEFINITIONS", "")) {
		if (not(params1.read(_definitions, "SMTP.CFG"))) {
			{}
		}
	}

	//global override
	var FILENAMES = "SMTP.CFG" ^ VM ^ "..\\..\\SMTP.CFG";
	for (var filen = 1; filen <= 2; ++filen) {
		call osread(params2, FILENAMES.a(1, filen));
		//cut off after end of file character
		params2 = params2.field(var().chr(26), 1);
		params2.swapper("\r\n", "\r");
		params2.converter("\r\n", "\xFE\xFE");
		for (var ii = 1; ii <= 9; ++ii) {
			if (params2.a(ii)) {
				params1.r(ii, params2.a(ii));
			}
		};//ii;
	};//filen;

	//default neosys smtp parameters
	if (not params1.a(1)) {
		var sysname = SYSTEM.a(57);
		if (not sysname) {
			sysname = "unknown";
		}
		params1.r(1, sysname ^ "@neosys.com");
	}
	if (params1.a(2) == "") {
		params1.r(2, "mailout.neosys.com");
	}
	if (params1.a(3) == "") {
		params1.r(3, "2500");
	}

	params = "";
	params.r(-1, "fromaddress=" ^ params1.a(1));
	params.r(-1, "smtphostname=" ^ params1.a(2));
	params.r(-1, "smtpportno=" ^ params1.a(3));
	params.r(-1, "smtptimeoutsecs=" ^ params1.a(4));
	params.r(-1, "smtpusessl=" ^ params1.a(5));
	params.r(-1, "smtpauthtype=" ^ params1.a(6));
	params.r(-1, "smtpuserid=" ^ params1.a(7));
	params.r(-1, "smtppassword=" ^ params1.a(8));

	if (replyto) {
		params.r(-1, "replyto=" ^ replyto);
	}

	//body=body0

	//if index(body,' ',1) or len(body)>10 or index(body,\0D\,1) or index(body,\0A\,1) then
	if (SLASH_IS_BACKSLASH and body and body[1] ne "@") {
		bodyfilename = "$" ^ (var("999999999999").rnd()).substr(1, 7) ^ ".TXT";
		//solve stupid outlook joining lines together if > 40 chars
		//by adding tab on the end of every line
		body.swapper("\r", "\t\r");
		call oswrite(body, bodyfilename);
		bodyfilename.osclose();
		body = "@" ^ bodyfilename;
	}else{
		body.converter(DQ, "\'");
		bodyfilename = "";
	}

	//for CDO attachfilename must be full path
	if (attachfilename) {
		attachfilename.osclose();
		attachfilename.swapper("\\\\", "\\");
		if (attachfilename.substr(1, 2) == "..") {
			attachfilename.splicer(1, 2, var().osdir().field("\\", 1, (var().osdir()).count("\\") - 1));
		} else if (attachfilename.substr(1, 2) == ".") {
			attachfilename.splicer(1, 1, var().osdir());
		}
		attachfilename.swapper("\\", "\\\\");
	}

	var paramfilename = "$" ^ (var("999999999999").rnd()).substr(1, 7) ^ ".TXT";

	var errorfilename = var("99999999").rnd() ^ ".$$$";

	var cmd;
	if (SLASH_IS_BACKSLASH) {
		cmd = "START /w";
		//option to de-bug
		//cmd=' WSCRIPT //X'
		cmd ^= " sendmail.js /e " ^ errorfilename ^ " /p " ^ paramfilename;

		//params='/t ':quote(toaddress):' /s ':quote(subject):' /b ':quote(body):' /a ':quote(attachfilename)
		//if delete then params:=' /d ':delete

		params.r(-1, "toaddress=" ^ (DQ ^ (toaddress ^ DQ)));
		if (ccaddress) {
			params.r(-1, "ccaddress=" ^ (DQ ^ (ccaddress ^ DQ)));
		}
		params.r(-1, "subject=" ^ (DQ ^ (subject ^ DQ)));
		params.r(-1, "body=" ^ (DQ ^ (body ^ DQ)));

		if (attachfilename) {
			params.r(-1, "attachfilename=" ^ (DQ ^ (attachfilename ^ DQ)));
		}
		if (deletex) {
			params.r(-1, "deleteaftersend=" ^ (DQ ^ (deletex ^ DQ)));
		}
		params ^= FM;

		params.swapper(FM, "\r\n");
		call oswrite(params, paramfilename);

		osshell(cmd);
		/////////////

	//use mail/mailx
	} else {

		ossetenv("MAILFROM",params1.a(1));
		if (replyto) {
			ossetenv("replyto",replyto);
		}

		cmd="mail";
		cmd^=" -r "^params1.a(1);//mailfrom but doesnt work via smtp according to man mailx
		cmd^=" -s "^subject.quote();
		if (ccaddress) {
			cmd^=" -c "^ccaddress;
		}
		if (attachfilename) {
			cmd^=" -a "^attachfilename;
		}
		cmd^=" "^toaddress;

		cmd^=" 2>"^errorfilename;
asm("int $3");
		var mailresult=cmd.osshellwrite(body);
		/////////////////////////////////////

//		oswrite(mailresult,"mailresult");

		//cmd^="-- -F $MAILFROM -f ${MAILFROM}@somedomain.com";

	}

	if (bodyfilename) {
		bodyfilename.osdelete();
	}

	paramfilename.osdelete();

	if (errormsg.osread(errorfilename)) {
		errormsg.trimmer();
		errorfilename.osdelete();
		errormsg.converter("\r\n", "\xFE\xFE");
		//errormsg=errormsg 'T#60'
	}else{
		errormsg = "Unknown error in sendmail.js Failed to complete";
		errormsg.r(-1, cmd);
		//errormsg<-1>=params 'T#60'
		errormsg.r(-1, params);
	}
	errormsg.converter(TM, FM);

	errormsg.r(1, errormsg.a(1).trim());
	//in what situation can it return OK+message ??
	if (errormsg.a(1) == "OK") {
		errormsg.eraser(1);
	}else{
		errormsg.trimmerb(FM);
		errormsg.r(-1, FM ^ "From:     " ^ params1.a(1));
		errormsg.r(-1, "To:       " ^ toaddress);
		if (ccaddress) {
			errormsg.r(-1, "cc:       " ^ ccaddress);
		}
		errormsg.r(-1, "Server:   " ^ params1.a(2));
		errormsg.r(-1, "Port:     " ^ params1.a(3));
		errormsg.r(-1, "UseSSL:   " ^ params1.a(5));
		errormsg.r(-1, "AuthType: " ^ params1.a(6));
		errormsg.r(-1, "UserID:   " ^ params1.a(7));
		errormsg.r(-1, "Password:");
		if (params1.a(8)) {
			errormsg ^= "********";
		}

		call log("SENDMAIL", errormsg);
		return 0;
	}

	return 1;

}


libraryexit()
