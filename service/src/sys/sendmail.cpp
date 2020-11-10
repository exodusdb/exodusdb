#include <exodus/library.h>
libraryinit()

#include <sendmail.h>
#include <shell2.h>
#include <log.h>

#include <gen_common.h>

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

function main(in toaddress0, in ccaddress0, in subject0, in body0, in attachfilename0, in delete0, out errormsg, in replyto0=var(), in params0=var()) {
	//c sys in,in,in,in,in,in,out,=var(),=var()

	//uses sendmail.js

	//global tt,maxemailsize,forcedemailx

	#include <general_common.h>

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
		params = params0;
	}
	if (not deletex) {
		deletex = "";
	}

	//send files as links if too large (>10MB) for email to handle efficiently
	maxemailsize = 10 * 1024 * 1024;

	//sendmail is silent
	errormsg = "";

	if ((SENTENCE.field(" ", 1).ucase() == "SENDMAIL") and toaddress0.unassigned()) {
		toaddress = SENTENCE.field(" ", 2);
		if (not toaddress) {
			toaddress = "sb2@neosys.com";
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
		}else{
			var msg = "STEP 1 OK. Mail for " ^ toaddress ^ " accepted by mail server.";
			params.converter("\r\n", FM);
			msg.r(-1, FM ^ "Sent using:" ^ FM ^ params);
			msg.r(-1, "STEP 2. Now check if actually received by recipient to verify");
			msg.r(-1, " that the mail server can actually deliver email to " ^ toaddress);
			msg.r(-1, " and that " ^ toaddress ^ " can receive email from the server" ^ FM);
			msg = msg.oconv("T#75");
			msg.converter(TM, FM);
			call note(msg);
		}
		stop();
	}else{
		toaddress = toaddress0;

		//development systems ALWAYS email hardcoded SB2@NEOSYS.COM in next line
		if (var("neosys.id").osfile() or not(VOLUMES)) {
			forcedemailx = "sb2@neosys.com";
			//toaddress=xlate('USERS','NEOSYS',7,'X')
			//if toaddress else toaddress='sb2@neosys.com'
			goto forcedemail;
		}

		//testdata and user neosys - always email sysmsg@neosys.com
		//if system<61> or (@username='NEOSYS' and system<17,1>[-4,4]='TEST') then

		//testdata and user neosys - always email sysmsg@neosys.com
		if (SYSTEM.a(61) and (USERNAME == "NEOSYS")) {
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
		} else if (SYSTEM.a(117)) {
			forcedemailx = SYSTEM.a(117);
			goto forcedemail;
		}

	}

	//limit development system to steve
	if (SYSTEM.index("1EEC633B")) {
		toaddress = "steve.bush@neosys.com";
		ccaddress = "";
	}

	//cut off any cc addresses from the toaddress (after ;;)
	tt = toaddress.index(";;");
	if (tt) {
		if (ccaddress) {
			ccaddress ^= ";";
		}
		ccaddress ^= toaddress.substr(tt + 2,9999);
		toaddress = toaddress.substr(1,tt - 1);
	}

	//send mail requires confirmation if user is NEOSYS
	//CANNOT because backup in neosys.net would be interrupted
	//declare function decide
	//if @username='NEOSYS' then
	// s33=system<33>
	// q='You are NEOSYS.|Send mail to ':toaddress:'|':subject:'|':body0
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
	for (var filen = 1; filen <= 2; ++filen) {
		call osread(params2, filenamesx.a(1, filen));
		//cut off after end of file character
		//params2=field(params2,char(26),1)
		params2.swapper("\r\n", "\r");
		params2.converter("\r\n", _FM_ _FM_);
		for (var ii = 1; ii <= 9; ++ii) {
			if (params2.a(ii)) {
				params1.r(ii, params2.a(ii));
			}
		};//ii;
	};//filen;

	//default neosys smtp parameters
	if (not(params1.a(1))) {
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

	var msgsize = 0;

	if (body[1] == "@") {

		osfilename = body.substr(2,99999);
		osfilesize = osfilename.osfile().a(1);

		//convert to link if file is too big to email
		if (osfilesize > maxemailsize) {
			body = "";
			gosub addlinks2osfilename();
		}else{
			msgsize += osfilesize;
		}

	}

	msgsize += body.length();

	//convert to links if attachfile is too big to email
	//assumes attachfilename is in data dir accessible by web
	if (attachfilename) {
		attachfilename.osclose();

		osfilesize = attachfilename.osfile().a(1);
		if (osfilesize > maxemailsize) {
			osfilename = attachfilename;
			gosub addlinks2osfilename();
			attachfilename = "";
		}
	}

	//for CDO attachfilename must be full path
	if (attachfilename) {

		attachfilename.swapper("\\\\", "\\");
		if (attachfilename.substr(1,2) == "..") {
			attachfilename.splicer(1, 2, oscwd().field(OSSLASH, 1, oscwd().count(OSSLASH) - 1));
		} else if (attachfilename.substr(1,2) == ".") {
			attachfilename.splicer(1, 1, oscwd());
		}
		msgsize += attachfilename.osfile().a(1);

		attachfilename.swapper("\\", "\\\\");
	}

	//body=body0
	body.swapper(FM, "\r\n");

	//if index(body,' ',1) or len(body)>10 or index(body,\0D\,1) or index(body,\0A\,1) then
	if (body and (body[1] ne "@")) {
		bodyfilename = var(999999999).rnd().substr(1,7) ^ ".tmp";
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

	//params='/t ':quote(toaddress):' /s ':quote(subject):' /b ':quote(body):' /a ':quote(attachfilename)
	//if delete then params:=' /d ':delete

	//condition subject start standard with 'NEOSYS: '
	if (subject.substr(1,8) ne "NEOSYS: ") {
		if (subject.substr(1,6) == "NEOSYS") {
			subject = subject.substr(7,9999).trimf();
		}
		if (subject.substr(1,7) == "System:") {
			subject = subject.substr(8,9999).trimf();
		}
		subject.splicer(1, 0, "NEOSYS: ");
	}

	params.r(-1, "toaddress=" ^ (toaddress.quote()));
	if (ccaddress) {
		params.r(-1, "ccaddress=" ^ (ccaddress.quote()));
	}
	params.r(-1, "subject=" ^ (subject.quote()));
	params.r(-1, "body=" ^ (body.quote()));

	if (attachfilename) {
		params.r(-1, "attachfilename=" ^ (attachfilename.quote()));
	}
	if (deletex) {
		params.r(-1, "deleteaftersend=" ^ (deletex.quote()));
	}
	params ^= FM;

	//AREV uses NEOSYS sendmail.js script which uses windows CDO object
	if (VOLUMES) {

		params.swapper(FM, "\r\n");
		paramfilename = var(999999999).rnd().substr(1,7) ^ ".tmp";
		call oswrite(params, paramfilename);

		errorfilename = var(999999999).rnd().substr(1,7) ^ ".tmp";

		//cmd='START /w'
		//using CSCRIPT because of difficulty reading errorfilename contents
		cmd = "cscript //Nologo";
		cmd ^= " sendmail.js /e " ^ errorfilename ^ " /p " ^ paramfilename;

	//otherwise use standard linux/posix mail program
	}else{

		cmd = "mail";

		paramfilename = "";
		errorfilename = "";

		//subject
		cmd ^= " -s " ^ (subject.quote());

		//from
		var fromaddress = params1.a(1);
		cmd ^= " -r " ^ (fromaddress.quote());

		//optional cc address
		if (ccaddress) {
			cmd ^= " -c " ^ (ccaddress.convert(";", ",").quote());
		}

		//optional attach file
		if (attachfilename) {
			cmd ^= " -A " ^ (attachfilename.quote());
		}

		//mark html formatted messages as such
		if (body[1] == "@") {
			tt = "";
			if (bodyfile.osopen(body.substr(2,999))) {
				//osbread tt from bodyfile at 0 length 100
				call osbread(tt, bodyfile,  0, 100);
			}
		}else{
			tt = body.substr(1,20);
		}
		tt.ucaser();
		if (tt.index("<!DOCTYPE") or tt.index("<HTML")) {
			cmd ^= " -a \"Content-Type: text/html\"";
			cmd ^= " -a \"MIME-Version: 1.0\"";
		}

		//if there is a specific reply email address then give it
		if (replyto) {
			cmd ^= " -a \"Reply-To: " ^ replyto ^ DQ;

		//otherwise request suppression of replies, particularly automatic ones
		}else{
			cmd ^= " -a \"X-Auto-Response-Suppress: RN, NRN, OOF\"";
			//maybe Precedence: list would have better results
			cmd ^= " -a \"Precedence: bulk\"";
		}

		//to address(es) go last
		cmd ^= " " ^ toaddress.convert(";", " ");

		//ensure the email body is in a file
		if (bodyfilename == "") {

			//body may already be in a file
			if (body[1] == "@") {
				bodyfilename = body.substr(2,99999);

			//otherwise generate a random filename and write it
			}else{
				bodyfilename = var(999999999).rnd().substr(1,7) ^ ".tmp";
				var(body).oswrite(bodyfilename);
				bodyfilename.osclose();
			}

		}

		//and pipe the body file into the program as standard input
		tt = "\\";
		bodyfilename.converter(tt, OSSLASH);
		bodyfilename.swapper("$", "\\$");
		cmd ^= " < " ^ (bodyfilename.quote());

	}

	//print
	//print cmd
	errormsg = shell2(cmd, errors);
	//pcperform 'CMD /c ':cmd

	if (bodyfilename) {
		bodyfilename.osdelete();
	}
	if (paramfilename) {
		paramfilename.osdelete();
	}

	if (errormsg) {
		{}
	} else if (errorfilename) {
		var().osflush();
		errorfilename.osclose();
		if (errorfile.osopen(errorfilename)) {
			printl(errorfilename, " ok");
		}else{
			printl(errorfilename, " ko");
		}
		if (not(errormsg.osread(errorfilename))) {
			errormsg = "Unknown error in sendmail.js Failed to complete";
			errormsg.r(-1, cmd);
			//errormsg<-1>=params 'T#60'
			errormsg.r(-1, params);
		}
	}else{
		if (errormsg == "") {
			if (errors) {
				errormsg = errors;
			}else{
				errormsg = "OK";
			}
		}
	}
	errormsg.converter(TM, FM);

	if (errorfilename) {
		errorfilename.osdelete();
	}

	errormsg.r(1, errormsg.a(1).trim());
	//in what situation can it return OK+message ??
	if (errormsg.a(1) == "OK") {
		errormsg.remover(1);
	}else{
		errormsg = trim(errormsg, FM, "B");
		errormsg.r(-1, FM ^ "Size:     " ^ oconv(msgsize, "[XBYTES]"));
		errormsg.r(-1, "From:     " ^ params1.a(1));
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

subroutine addlinks2osfilename() {
	tt = osfilename;
	tt.converter("\\", "/");
	if (tt.substr(1,3) == "../") {
		tt.splicer(1, 3, "");
	}
	if (body) {
		body ^= FM;
	}
	body.r(-1, "Your report is too large to email. (" ^ oconv(osfilesize, "[XBYTES]") ^ ", max " ^ oconv(maxemailsize, "[XBYTES]") ^ ")");
	body.r(-1, "but you can download it by clicking the following link.");
	body.r(-1, FM ^ "*Link is only available for ONE HOUR from creation*");
	var nlinks = SYSTEM.a(114).count(VM) + 1;
	for (var linkn = 1; linkn <= nlinks; ++linkn) {
		body ^= FM;
		var linkdesc = SYSTEM.a(115, linkn);
		if (linkdesc) {
			body.r(-1, linkdesc);
		}
		body.r(-1, SYSTEM.a(114, linkn) ^ tt);
	};//linkn;
	return;
}

libraryexit()