#include <exodus/library.h>
libraryinit()

#include <htmllib2.h>
#include <log.h>
#include <getbackpars.h>
#include <htmllib.h>
#include <elapsedtimetext.h>
#include <roundrobin.h>
#include <sendmail.h>

#include <gen.h>

var subjectin;
var username;
var bakpars;
var body;
var ver;
var result;
var xx;
var errormsg;
var exceededmsg;

function main(in msg0, in subject0="", in username0="") {
	//c sys in,"",""

	//logs a message and sends it to all the technical support emails (backup)
	//DO NOT call msg or note etc, otherwise may be recursive

	//if msg0 starts with @@something then sendmail sends file @something
	//not tested or used currently

	var interactive = not SYSTEM.a(33);
	var datasetcode = SYSTEM.a(17);

	//NB this routine is called automatically from msg()
	//whenever the msg starts with SYSTEM ERROR
	//??? nice idea but it isnt in the MSG() routine???

	var msg = msg0;
	if (subject0.unassigned()) {
		subjectin = "";
		}else{
		subjectin = subject0;
	}
	if (username0.unassigned()) {
		username = "";
	}else{
		username = username0;
	}

	msg.swapper(var().chr(0), "%00");
	subjectin.swapper(var().chr(0), "%00");

	if (not interactive) {
		//print msg:', ':subjectin:', ':username
		printl("sysmsg: ", subjectin, ", ", username, ", ", msg.a(1, 1).field("|", 1).a(1, 1));
	}

	if ((msg == "") and subjectin) {
		msg = subjectin;
	}

	if (not username) {
		username = USERNAME;
	}

	//remove html tags from message and decode things like &nbsp;
	if (msg.substr(1,2) ne "@@") {
		call htmllib2("STRIPTAGS", msg);
		call htmllib2("DECODEHTML", msg);
	}

	//log the system message first in case sendmail fails
	var logmsg = msg;
	if (subjectin) {
		logmsg.splicer(1, 0, subjectin ^ FM);
	}
	call log("SYSMSG", logmsg);

	//get backup parameters
	call getbackpars(bakpars);

	//get technical emailaddrs to send to
	//nb if any emailaddrs and neosys.com not in them
	//then neosys will not receive any message
	var useremail = username.xlate("USERS", 7, "X").lcase();
	var userfullname = username.xlate("USERS", 1, "X");
	//if username='NEOSYS' and @username<>'NEOSYS' then
	var emailaddrs = bakpars.a(6);
	var ccaddrs = "";
	if (bakpars.a(10)) {
		if (emailaddrs) {
			emailaddrs ^= "/";
		}
		emailaddrs ^= bakpars.a(10);
	}
	emailaddrs = emailaddrs.field("/", 1);
	//if emailaddrs='' then readv emailaddrs from definitions,'REPLICATION',12 else emailaddrs=''
	emailaddrs.swapper("backups@neosys.com", "sysmsg@neosys.com");

	//suppress login failure messages
	if ((APPLICATION ne "ACCOUNTS" and username ne "NEOSYS") and (subjectin.substr(1,13) == "Login Failure")) {
		emailaddrs.swapper("sysmsg@neosys.com", "");
		emailaddrs = trim(emailaddrs, ";");
	}

	//prevent a weird error going to users
	var addhw = 0;
	if (msg.index("The process cannot access the file because it is being")) {
		addhw = 1;
		emailaddrs = "";
		subjectin ^= " V2";
	}

	//username NEOSYS and @username<>NEOSYS means email NEOSYS ONLY!
	if ((username == "NEOSYS") and USERNAME ne "NEOSYS") {
		emailaddrs = "sysmsg@neosys.com";
		//so we know who caused the message
		username = USERNAME;
	}

	//sysmsg is not emailed to admins if testdata or user is NEOSYS
	if ((USERNAME == "NEOSYS") or (SYSTEM.a(17).substr(-4,4) == "TEST")) {

		//this is disabled to ensure that all errors caused by NEOSYS support
		//are logged normally
		//also, User File Amendments currently use sysmsg() and should be reported
		//to admins even if done by NEOSYS
		//sysmsg is not emailed to admins if testdata or user is NEOSYS
		//if @username='NEOSYS' or system<17>[-4,4]='TEST' then

		//sysmsg is not emailed to admins if testdata
		//if system<17>[-4,4]='TEST' then
		//never email user
		useremail = "";
		//never email anybody but steve.bush@neosys.com or sysmsg@neosys.com
		if (emailaddrs ne "sb2@neosys.com" and emailaddrs ne "steve.bush@neosys.com") {
			emailaddrs = "";
		}
	}

	//if no email addresses then always email neosys
	if (emailaddrs == "") {
		emailaddrs = "sysmsg@neosys.com";
	}

	//determine subject
	//subject='NEOSYS System Message: ':datasetcode
	var subject = "NEOSYS System: " ^ datasetcode;
	var tt = msg.index("ERROR NO:");
	if (tt) {
		subject ^= " " ^ msg.substr(tt,9999).a(1, 1, 1);
	}
	if (subjectin) {
		subject ^= " " ^ subjectin;
	}

	if (msg.substr(1,2) == "@@") {
		body = msg.substr(2,999999);

	}else{
		var l9 = "L#9";
		body = "";
		//body<-1>='Message=':fm:msg
		body.r(-1, msg);
		body ^= FM;
		body.r(-1, var("Server:").oconv(l9) ^ SYSTEM.a(44).trim());
		body.r(-1, var("Install:").oconv(l9) ^ oscwd());
		if (ver.osread("general\\version.dat")) {
			body.r(-1, var("Version:").oconv(l9) ^ ver.a(1));
		}
		body.r(-1, var("Database:").oconv(l9) ^ SYSTEM.a(45).trim() ^ " " ^ SYSTEM.a(17));
		body.r(-1, var("Process:").oconv(l9) ^ SYSTEM.a(24));
		body.r(-1, var("Client:").oconv(l9) ^ STATION.trim());
		//blank IP No indicates not in a web request
		if (SYSTEM.a(40, 2)) {
			body.r(-1, var("IP No:").oconv(l9) ^ SYSTEM.a(40, 2));
		}
		body.r(-1, var("User:").oconv(l9) ^ username.trim());
		if (userfullname and userfullname.trim().ucase() ne username) {
			body ^= " (" ^ userfullname ^ ")";
		}
		if (useremail) {
			//if user email is not in the list of people being sent to then
			if (emailaddrs.lcase().index(useremail)) {
				//body<-1>='Email:' l9:useremail
			}else{
				ccaddrs = useremail;
			}
		}

		var agent = SYSTEM.a(40, 6);
		if (agent) {

			call htmllib2("OCONV.AGENT.OS", agent);
			body.r(-1, var("O/S:").oconv(l9) ^ agent);

			agent = SYSTEM.a(40, 6);
			call htmllib("OCONV.AGENT.BROWSER", agent);
			body.r(-1, var("Browser:").oconv(l9) ^ agent);

		}

		if (addhw) {
			body.r(-1, var("@hw:").oconv(l9) ^ HW);
		}

		var temp = "";
		//call returnstacktemp);
		body.r(-1, var("Stack:").oconv(l9) ^ temp);

		if (USER0) {
			temp = USER0;
			temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, "`^]}\\~");
			body.r(-1, var("Request:").oconv(l9) ^ temp);
		}

		//body<-1>='Message:' l9:fm:msg
		if (ID) {
			body.r(-1, var("@ID:").oconv(l9) ^ ID);
		}

		if (USER1) {
			temp = USER1;
			temp.swapper("%", "%25");
			temp.swapper("\'", "%27");
			temp.swapper("`", "%60");
			temp.swapper("^", "%5E");
			temp.swapper("]", "%5D");
			temp.swapper("}", "%7D");
			temp.swapper("\\", "%5C");
			temp.swapper("~", "%7E");
			temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, "`^]}\\~");
			body.r(-1, var("Data:").oconv(l9) ^ temp);
		}

		var requeststarttime = SYSTEM.a(25);
		if (requeststarttime) {
			body.r(-1, var("Duration:").oconv(l9) ^ elapsedtimetext(var().date(), requeststarttime));
		}

		body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
		body.swapper("\r", "\r\n");

	}

	//limit max 60 sysmsg emails per hour
	var params = "";
	params.r(1, 60);
	params.r(2, 60);
	params.r(3, 60);
	params.r(4, "DOS");
	params.r(5, "SYSMSG.$RR");
	var exceedmsg = "SYSMSG email suppressed because > 60 in last 60 mins";
	//call roundrobin('ONEVENT',params,result,errormsg)
	call roundrobin("ONEVENT", params, result, xx);
	if (result) {
		//sendmail - if it fails, there will be an entry in the log
		call sendmail(emailaddrs, ccaddrs, subject, body, "", "", errormsg);

		//log any sendmail failure or excessive emails
		if (errormsg and errormsg ne "OK") {
			call log("SENDMAIL", errormsg);
		}

	}else{
	//print 'EXCEEDED'
		//log excessive sysmsg
		call log("SYSMSG", exceededmsg);
	}

	return 0;

}


libraryexit()
