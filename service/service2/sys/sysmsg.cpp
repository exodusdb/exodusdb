#include <exodus/library.h>
libraryinit()

#include <log.h>
#include <readbakpars.h>
#include <roundrobin.h>
#include <sendmail.h>

#include <gen.h>

var subjectin;
var username;
var bakpars;
var ver;
var result;
var errormsg;
var exceededmsg;

function main(in msg0, in subject0="", in username0="") {

	//logs a message and sends it to all the technical support emails (backup)
	//do not call msg or note etc, otherwise may be recursive

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
		printl(msg, ", ", username, ", ", subjectin);
	}

	if (msg == "" and subjectin) {
		msg = subjectin;
	}

	if (not username) {
		username = USERNAME;
	}

	//log the system message first in case sendmail fails
	var logmsg = msg;
	if (subjectin) {
		logmsg.splicer(1, 0, subjectin ^ FM);
	}
	call log("SYSMSG", logmsg);

	//get backup parameters
	call readbakpars(bakpars);

	//get technical emailaddrs to send to
	//nb if any emailaddrs and neosys.com not in them
	//then neosys will not receive any message
	var useremail = username.xlate("USERS",7, "X");
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
	//if dir('NEOSYS.ID') and @username='NEOSYS' then emailaddrs='backups@neosys.com'
	//if dir('NEOSYS.ID') then emailaddrs='backups@neosys.com'
	emailaddrs.swapper("backups@neosys.com", "sysmsg@neosys.com");

	//suppress login failure messages
	if (ACCOUNT ne "ACCOUNTS" and USERNAME ne "NEOSYS" and subjectin.substr(1, 13) == "Login Failure") {
		emailaddrs.swapper("sysmsg@neosys.com", "");
		emailaddrs.trimmer(";");
	}

	//sysmsg is not emailed to admins if testdata or user is NEOSYS
	if (USERNAME == "NEOSYS" or (SYSTEM.a(17)).substr(-4, 4) == "TEST") {

		//this is cancelled to ensure that all errors caused by NEOSYS support
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
	var tt = msg.index("ERROR NO:", 1);
	if (tt) {
		subject ^= " " ^ msg.substr(tt, 9999).a(1, 1, 1);
	}
	if (subjectin) {
		subject ^= " " ^ subjectin;
	}

	var body = "";
	//body<-1>='Message=':fm:msg
	body.r(-1, msg);
	body ^= FM;
	body.r(-1, "Server=" ^ SYSTEM.a(44).trim());
	body.r(-1, "Install=" ^ oscwd());
	if (ver.osread("general\\version.dat")) {
		body.r(-1, "Version=" ^ ver.a(1));
	}
	body.r(-1, "Database=" ^ SYSTEM.a(45).trim() ^ " " ^ SYSTEM.a(17));
	body.r(-1, "Process=" ^ SYSTEM.a(24));
	body.r(-1, "Client=" ^ mv.STATION.trim());
	body.r(-1, "User=" ^ username.trim());
	if (useremail) {
		//if user email is not in the list of people being sent to then
		if ((emailaddrs.lcase()).index(useremail, 1)) {
			//body<-1>='Email=':useremail
		}else{
			ccaddrs = useremail;
		}
	}

	var temp = USER0;
	temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, "`^]}\\~");
	body.r(-1, "Request=" ^ temp);

	//body<-1>='Message=':fm:msg
	body.r(-1, "@Id=" ^ ID);

	temp = USER1;
	temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, "`^]}\\~");
	body.r(-1, "Data=" ^ temp);

	body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
	body.swapper("\r", "\r\n");

	//limit max 60 sysmsg emails per hour
	var params = "";
	params.r(1, 60);
	params.r(2, 60);
	params.r(3, 60);
	params.r(4, "DOS");
	params.r(5, "SYSMSG.$RR");
	var exceedmsg = "SYSMSG email suppressed because > 60 in last 60 mins";
	//call roundrobin('ONEVENT',params,result,errormsg)
	//call roundrobin("ONEVENT", params, result);
	var xx;
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
