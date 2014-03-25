#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <openfile.h>
#include <sysmsg.h>
#include <holiday.h>
#include <agencysubs.h>
#include <select2.h>
#include <generalsubs2.h>
#include <generalsubs.h>
#include <singular.h>
#include <timesheetsubs.h>
#include <log2.h>

#include <win.h>
#include <gen.h>
#include <agy.h>

#include <window.hpp>

var msg;
var usercode;
var xml;
var cols;
var data;
var reply;
var idate;//num
var timesheet;
var getholidaytypedate;
var holidaytype;
var USER;
var marketcode;
var market;
var personn;
var activityn;//num
var oldactivity;
var tt;//num
var v69;
var v70;
var v71;
var jobno;
var ln;//num
var userisadmin;
var t2;
var wsmsg;

var mode;
var logtime;

function main(in mode0) {

	//confuses with existing msg
	//equ msg to @user4

	//why was this changed? <10> is supposed to be days grace before ts reminders
	//equ maxdaysdelay to register(1)<4>
	//equ jobs to register(5)
	//equ security.net to register(9)

	log2("-----timesheetsubs," ^ mode0^" init", logtime);

	var interactive = not SYSTEM.a(33);
	mode=mode0;

	win.registerx(7) = authorised("TIMESHEET ADMINISTRATION", msg, "TA");
	//register(8)=msg
	var notadminmsg = msg;

	//if jobs='' then if openfile('JOBS',jobs) else jobs=''

	if (mode == "OPTIONSETUP") {
		//used to setup dos mode window

		if (not(authorised("TIMESHEET ADMINISTRATION", msg, ""))) {
			return invalid();
		}

		gosub getregisterparams();

		mv.PSEUDO = "";
		mv.PSEUDO.r(1, win.registerx(1).a(1));
		mv.PSEUDO.r(2, win.registerx(1).a(2));
		mv.PSEUDO.r(3, win.registerx(1).a(3));
		mv.PSEUDO.r(4, win.registerx(1).a(4));
		mv.PSEUDO.r(5, win.registerx(1).a(5));

	} else if (mode == "OPTIONSAVE") {
		//used to update from dos mode window

		RECORD.write(gen._definitions, "TIMESHEET.PARAMS");

	} else if (mode.substr(1, 8) == "POSTINIT") {

		win.valid = 1;
		gosub security(mode);
		if (not win.valid) {
			return invalid();
		}

		gosub getregisterparams();

		if (mode == "POSTINIT2") {

			usercode = USERNAME;
			xml = "";
			cols = "CODE NAME";
			gosub getactivities();

			call log2("select users by rank user_code user_and_dept_name", logtime);
			if (win.registerx(7)) {
				//equ response to @user3
				//call select2('USERS','','BY DEPT_AND_USER_NAME WITH DEPARTMENT_CODE NE @ID','USER_CODE USER_AND_DEPT_NAME','',data,response)
				call select2("USERS", "", "BY RANK WITH DEPARTMENT_CODE NE @ID", "USER_CODE USER_AND_DEPT_NAME", "", data, USER3, "", "", "");

//printl("timesheetsubs data ", data);
				data.transfer(win.registerx(8));
			}

			//security.net=security
			//*except passwords
			//register(8)<4>=''

		}

/*
	} else if (mode == "F2.PERSON") {
		//prevent others
		if (not win.registerx(7)) {
			var().chr(7).output();
			call mssg(win.registerx(8));
			return 0;
		}
		if (not(decide("", gen._security.a(1), reply))) {
			return 0;
		}
		ANS = gen._security.a(1, reply);
		data ^= "" "\r";
*/
	} else if (mode.field(".", 1) == "GETNEXTTIMESHEET") {
		ID = mode.field(".", 2, 9999);
		call timesheetsubs("PREREAD");
		USER1 = ID;
		USER3 = "OK";

	} else if (mode == "PREREAD") {

		idate = ID.field("*", 2, 999).field("~", 1);
		if (idate) {

			//skip if known date - the following is to find the next date
			if (idate.isnum()) {
				return 0;
			}

			//cater for strange case of non-numeric date (commonly javascript "NaN")
			//force it to '' instead of returning an error
			call sysmsg(DQ ^ (ID ^ DQ) ^ " non-numeric date in ts key");
			ID = ID.field("*", 1);

		}

		//find next fillable timesheet date

		usercode = ID.field("*", 1);

		//cater for strange case of no user code
		if (not usercode) {
			usercode = USERNAME;
			ID = ID.fieldstore("*", 1, 1, usercode);
		}

		gosub getregisterparams();

		//find last ts
		for (idate = var().date(); idate >= var().date() - 31; --idate) {
			var timesheet;
			if (not(timesheet.read(win.srcfile, ID.field("*", 1) ^ "*" ^ idate))) {
				timesheet = "";
			}
			if (timesheet)
				break;

		};//idate;

		getholidaytypedate = idate;
		gosub getholidaytype();

		if (timesheet == "") {

			//no previous timesheet found
			// - enter today (even if it is a holiday)
			idate = var().date();

			goto L1007;
		}
		if (timesheet and not holidaytype and (timesheet.a(2)).sum() < win.registerx(1).a(2)) {

			//found incomplete timesheet
			// - enter it

		}else{

			//previous full working or holiday timesheet found
			// - enter next working day after that
			var nextworkingday = 1;

			call holiday("GETWORKDATE", idate, usercode, USER, marketcode, market, agy.agp, holidaytype, nextworkingday);

			//never go beyond today even if today is a holiday
			if (idate > var().date()) {
				idate = var().date();
			}

		}
L1007:

		ID = ID.fieldstore("*", 2, 1, idate);

		//case mode='DEF.PERSON'
		// if is then return 0
		// @ans=@username
		// if isadmin else data is

	} else if (mode == "VAL.PERSON") {
		if (win.is == "") {
			win.reset = 99;
			return 0;
		}
		if (win.is == "NEOSYS" and USERNAME == "NEOSYS") {
			return 0;
		}
		if (win.is == "---") {
			goto baduser;
		}
		if (not(gen._security.locate(win.is, personn, 1))) {
baduser:
			msg = DQ ^ (win.is ^ DQ) ^ " - person not authorised";
			return invalid(msg);
		}

		//prevent others
		if (not win.registerx(7) and win.is ne USERNAME) {
			win.is = USERNAME;
			call note(win.registerx(8));
		}

		usercode = win.is;
		cols = "CODE";
		xml = "";
		gosub getactivities();

		//some people cannot do timesheets
		if (win.registerx(1).a(1) and win.registerx(3) == "") {
			msg = DQ ^ (win.is ^ DQ) ^ " does not do timesheets|There are no activity codes|setup for dept. " ^ (DQ ^ (win.registerx(2) ^ DQ));
			return invalid(msg);
		}

	} else if (mode == "DEF.DATE") {
		if (win.is) {
			return 0;
		}
		ANS = var().date();
		if (not(win.registerx(7))) {
			data ^= ANS.oconv("[DATE,*]") ^ "\r";
		}

	} else if (mode == "DEF.JOB") {
		//data @username
		//data ''
		//data ''
		//call agency.subs('F2.JOBS')

	} else if (mode == "VAL.JOB") {
		if (win.is == win.isorig) {
			return 0;
		}

		var xx;
		call agencysubs("VAL.JOB.OPEN", xx);
		if (not win.valid) {
			return 0;
		}

		//jobno=is
		//gosub checkclosed
		//if msg then goto invalid

	} else if (mode == "VAL.HOURS") {

		if (win.is == "") {
			return 0;
		}

		//check numeric
		if (not win.is.isnum()) {
			msg = "Please enter a number of hours";
			return invalid(msg);
		}

		//check not negative
		if (win.is < 0) {
			msg = "Please enter number of hours greater than 0";
			return invalid(msg);
		}

		//check minimum time unit
		if (win.registerx(1).a(5)) {
			var tsminhours2 = win.registerx(1).a(5) / 60;
			var is2 = (win.is / tsminhours2).floor() * tsminhours2;
			if (is2 ne win.is) {
				msg = "Warning: The minimum time interval is " ^ win.registerx(1).a(5) ^ " minutes";
				gosub note(msg);
				win.is = is2;
			}
		}

		//warning if exceeded the max number of hours (will not be able to save)
		if (win.registerx(1).a(3)) {

			//determine the new total number of hours
			var tothours = RECORD.a(2);
			tothours.r(1, win.mvx, win.is);
			tothours = tothours.sum();

			if (win.is > win.registerx(1).a(3) or tothours > win.registerx(1).a(3)) {
				msg = "The total hours you have entered is " ^ tothours ^ " but|";
				msg.r(-1, "the maximum hours allowed is " ^ win.registerx(1).a(3) ^ "|");
				if (win.is > win.registerx(1).a(3)) {
					return invalid(msg);
				}
				gosub note(msg);
			}

		}

	} else if (mode == "F2.ACTIVITY") {
		var xx;
		call agencysubs("F2.PRODUCTION.TYPE," ^ win.registerx(2), xx);

	} else if (mode == "DEF.ACTIVITY") {

		if (win.is) {
			return 0;
		}

		//default to previous line
		if (MV > 1) {
			win.isdflt = RECORD.a(4, MV - 1);
		}

		//force required
		//if tsactivitycoderequired then
		// si<18>='R'
		// w(wi)<18>='R'
		// end

	} else if (mode == "READ") {

		if (RECORD.read(gen.timesheets, ID)) {
			return 0;
		}

		usercode = ID.field("*", 1);
		idate = ID.field("*", 2);

		gosub getprevioustimesheet();
		//orec=@record

	} else if (mode == "POSTREAD2") {

		usercode = ID.field("*", 1);
		idate = ID.field("*", 2);

		win.valid = 1;

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
		}

		gosub security(mode);

		//check if amending in authorised period
		if (win.wlocked) {
			call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
			if (msg) {
				//comment to client
				win.reset = -1;
				gosub note(msg);
				unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}
		}

		gosub getregisterparams();

		//restrictions for standard users ie not-timesheet admins
		if (not win.registerx(7)) {
	//if @username='NEOSYS' or not(isadmin) then

			//check if allowed to enter or edit old timesheets
			if (win.wlocked and win.registerx(1).a(10)) {
				var daysdelay = var().date() - ID.field("*", 2);
				if (daysdelay > win.registerx(1).a(10)) {
					msg = "Sorry, you cannot enter or edit timesheets|older than " ^ win.registerx(1).a(10) ^ " days.";
					gosub invalid();
					win.wlocked = 0;
					if (RECORD) {
						win.reset = -1;
					}else{
						win.reset = 5;
					}
				}
			}

			//check if allowed to enter advance timesheets
			if (win.wlocked and RECORD == "" and win.registerx(1).a(7) ne "") {
				var daysadvance = ID.field("*", 2) - var().date();
				if (daysadvance > win.registerx(1).a(7)) {
					msg = "Sorry, you cannot enter or edit timesheets|more than " ^ (win.registerx(1).a(7) + 0) ^ " days in advance.";
					unlockrecord("TIMESHEETS", win.srcfile, ID);
					win.wlocked = 0;
					gosub invalid();
					win.reset = 5;
				}
			}

			//check if allowed to modify approved timesheets
			if (win.wlocked and RECORD.a(8) == "APPROVED") {
				call mssg("This timesheet is already approved and cannot be edited unless a timesheet administrator changes the status");
				//x=unlockrecord(datafile,src.file,@id)
				win.wlocked = 0;
				win.reset = -1;
			}

		}

		if (win.wlocked and RECORD == "") {

			gosub getprevioustimesheet();

			//only needed until old ie5 clients (brandcom) are ungradeds to w3c UI
			RECORD.r(100, "");
			RECORD.r(101, "");

			//get any new jobs
			if (win.registerx(1).a(18)) {
				var ndays = win.registerx(1).a(18);
				if (not ndays.isnum()) {
					ndays = 7;
				}

//				call pushselect(0, v69, v70, v71);
				var cmd = "SSELECT JOBS";
				var minjobcreationdate = var().date() - (ndays - 1);
				cmd ^= " WITH DATE_CREATED GE " ^ (DQ ^ (minjobcreationdate.oconv("[DATE]") ^ DQ));
				cmd ^= " AND WITH CLOSED NE \"Y\" AND WITH AUTHORISED";
//				call safeselect(cmd);
				var().select(cmd);
				var namvs = agy.tsmvfns.dcount(VM);
nextjob:
				var jobno;
				if (var().readnext(jobno)) {
					if (not(RECORD.locate(jobno, ln, 1))) {
						//insert a new blank line at the top
						for (var amvn = 1; amvn <= namvs; ++amvn) {
							var fn = agy.tsmvfns.a(1, amvn);
							var temp = RECORD.a(fn);
							if (temp) {
								RECORD.inserter(fn, 1, "");
							}
						};//amvn;
					}
					//set the job no
					RECORD.r(1, 1, jobno);
					RECORD.r(3, 1, "New");
					goto nextjob;
				}
//				call popselect(0, v69, v70, v71);

			}

		}

		//delete this once no more ie5 old style clients (brandcom/venture)
		//get lookup fields (upto 48Kb max) for old mac ie5 user interface
		//if index(system<40>,'VERSION',1) else
		//moved up so new jobs can have *NEW* prefixed
		//@record<100>='';*job description
		//@record<101>='';*brand name
		if (agy.brands.open("BRANDS", "")) {
			var jobnos = RECORD.a(1);
			var njobs = jobnos.count(VM) + 1;
			for (var jobn = 1; jobn <= njobs; ++jobn) {
			///BREAK;
			if (RECORD.length() > 48000) break;;
				var job;
				if (job.read(agy.jobs, jobnos.a(1, jobn))) {
					RECORD.r(100, jobn, RECORD.a(100, jobn) ^ " " ^ trimb(job.a(9, 1)));
					var brand;
					if (brand.read(agy.brands, job.a(2))) {
						RECORD.r(101, jobn, brand.a(2));
					}
				}
			};//jobn;
		}
		// end

	} else if (mode == "POSTREAD") {
		gosub security(mode);

		//standard users may not be able to enter or edit old timesheets
		if (not win.registerx(7) and win.registerx(1).a(10)) {
			var daysdelay = var().date() - ID.field("*", 2);
			if (daysdelay > win.registerx(1).a(10)) {
				var xx = unlockrecord("TIMESHEETS", win.srcfile, ID);
				win.wlocked = 0;
				msg = "Sorry, you cannot enter or edit timesheets|older than " ^ win.registerx(1).a(10) ^ " days.";
				gosub invalid();
				win.reset = 5;
			}
		}

	} else if (mode.field(".", 1) == "PREWRITE") {

		//prevent creating new records if no lic
//		call chklic(mode, msg);
		if (msg) {
			return invalid(msg);
		}

		usercode = ID.field("*", 1);
		idate = ID.field("*", 2);

		//(double) check if authorised to approve
		var approving = mode.field(".", 2) == "APPROVING";
		if (approving) {
			if (not(authorised("TIMESHEET APPROVAL", msg, "TAP"))) {
				return invalid(msg);
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//determine if it timesheet of an administrator (allowed to go under hours etc)
		if (usercode == USERNAME) {
			userisadmin = win.registerx(7);
		}else{
			userisadmin = authorised("TIMESHEET ADMINISTRATION", msg, "TA", usercode);
		}

		gosub getregisterparams();

		var tothours = (RECORD.a(2)).sum();

		//always require some hours
		if (not tothours) {
			msg = "The total hours:minutes cannot be zero";
			return invalid(msg);
		}

		//check max hours (if any) always
		if (win.registerx(1).a(3)) {
			if (tothours > win.registerx(1).a(3)) {
				msg = "The total hours entered is " ^ tothours ^ " but";
				msg = " the maximum allowed is " ^ win.registerx(1).a(3);
				return invalid(msg);
			}
		}

		//check min hours (of non-timesheet admin timesheets)
		if (win.registerx(1).a(2) and tothours < win.registerx(1).a(2) and not userisadmin) {

			//anybody can enter less for the current date but it cannot be approved
			if (idate ne var().date() or approving) {

				getholidaytypedate = idate;
				gosub getholidaytype();
				if (not win.valid) {
					return 0;
				}

				//any type of holidays can have less than min hours
				//should be determined per user
				//TODO work out first company of each user
				//in TIMESHEET.SUBS and ANALTIME2
				if (not holidaytype) {
					msg = "The total hours entered is " ^ tothours ^ " but ";
					msg ^= "the minimum allowed is " ^ (win.registerx(1).a(2) + 0);
					return invalid(msg);
				}

			}

		}

		//check details entered for sundry clients
		if (agy.clients.open("CLIENTS", "")) {
			var clientcodes = calculate("CLIENT_CODE");
			var nlns = clientcodes.count(VM) + 1;
			for (var ln = 1; ln <= nlns; ++ln) {
				if (not RECORD.a(3, ln) and RECORD.a(2, ln)) {
					var clientcode = clientcodes.a(1, ln);
					var client;
					if (not(client.read(agy.clients, clientcode))) {
						msg = DQ ^ (clientcode ^ DQ) ^ " is missing from the client file";
						return invalid(msg);
					}
					if (client.a(38)) {
						//msg='Details are missing for ':client<1>
						msg = client.a(1) ^ " is a \"Sundry Client\" (See Client/Brand File)";
						msg.r(-1, FM ^ "Please enter the client name/details in line " ^ ln);
						return invalid(msg);
					}
				}
			};//ln;
		}

		//check no missing activity codes
		if (win.registerx(1).a(1)) {
			var nlines = (RECORD.a(1)).count(VM) + (RECORD.a(1) ne "");
			for (var linen = 1; linen <= nlines; ++linen) {
				if (RECORD.a(1, linen) and RECORD.a(4, linen) == "") {
					msg = "Activity code is required but missing on line " ^ linen;
					return invalid(msg);
				}
			};//linen;
		}

		//remove joined job/brand data (provided to old mac ie5 clients)
		//@record<100>=''
		//@record<101>=''

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTWRITE") {

//		call flushindex("TIMESHEETS");

		//flag to init.general that the timesheet has been entered
		//register(6) is pseudo
		if (ID == win.registerx(6).a(1)) {
			win.registerx(6).r(2, ID);
		}

	} else if (mode == "POSTDELETE") {

//		call flushindex("TIMESHEETS");

	} else if (mode == "PREDELETE") {
		gosub security(mode);

	} else if (mode == "POSTAPP") {
		mv.PSEUDO = win.registerx(6);

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - unknown mode skipped in TIMESHEET.SUBS";
		return invalid(msg);
	}

	log2("-----timesheetsubs," ^ mode0^" exit", logtime);
	return 1;

}

function checkclosed() {
	msg = "";
	//check job has not been closed
	var job;
	if (job.read(agy.jobs, jobno)) {
		if (job.a(7) == "Y") {
			msg = DQ ^ (win.is ^ DQ) ^ " Job is closed";
		}
	}
	return 0;

}

subroutine getactivities() {
	//get dept
	call generalsubs("GETUSERDEPT," ^ usercode);
	win.registerx(2) = ANS.trim();
	win.registerx(2).converter("0123456789", "");

	//select
	//perform 'SSELECT JOB_TYPES WITH DEPARTMENT ':quote(deptcode)
	call select2("JOB_TYPES", "", "WITH DEPARTMENT " ^ (DQ ^ (win.registerx(2) ^ DQ)), cols, xml, data, USER3);
	win.registerx(3) = data;

	//convert fm to vm in activitycodes

	return;

}

subroutine getregisterparams() {
	//open 'JOBS' to jobs else
	// call fsmsg()
	// stop
	// end
	if (not((win.registerx(1)).read(gen._definitions, "TIMESHEET.PARAMS"))) {
		win.registerx(1) = "";
		//if index(ucase(company<1>),'IMPACT',1) then
		// register(1)<1>=1
		// register(1)<2>=8
		// register(1)<3>=8
		// register(1)<4>=3;maxdaysdelay
		// register(1)<5>=15
		// end
	}
	if (win.registerx(1).a(1) == "N" or not win.registerx(1).a(1)) {
		win.registerx(1).r(1, "");
	}
	//pad
	win.registerx(1).r(10, win.registerx(1).a(10));

	if (win.registerx(1).a(7) == "") {
		win.registerx(1).r(7, 7);
	}

	return;

}

subroutine getholidaytype() {

	holidaytype = "";

	//get user/check exists
	var users;
	if (not(users.open("USERS", ""))) {
		call fsmsg();
		win.valid = 0;
		return;
	}
	if (not(USER.read(users, usercode))) {
		msg = DQ ^ (usercode ^ DQ) ^ " user does not exist";
		gosub invalid(msg);
		return;
	}

	//should be determined per user
	//TODO work out first company of each user
	//in TIMESHEET.SUBS and ANALTIME2
	marketcode = gen.company.a(30, 1);
	if (not(market.read(agy.markets, marketcode))) {
		msg = DQ ^ (marketcode ^ DQ) ^ " Market is missing";
		gosub invalid(msg);
		return;
	}

	var xx;
	call holiday("GETTYPE", getholidaytypedate, usercode, USER, marketcode, market, agy.agp, holidaytype, xx);

	return;

}

subroutine getprevioustimesheet() {

	//get jobs from prior date up to 31 days ago
	var priordate;
	for (priordate = idate - 1; priordate >= idate - 31; --priordate) {
		if (not(timesheet.read(gen.timesheets, usercode ^ "*" ^ priordate))) {
			timesheet = "";
		}
		if (timesheet)
			break;
	};//priordate;

	//check if previous timesheet has sufficient hours
	if (timesheet and not win.registerx(7)) {

		var tothours = (timesheet.a(2)).sum();
		if (tothours < win.registerx(1).a(2)) {

			getholidaytypedate = priordate;
			gosub getholidaytype();
			if (not win.valid) {
				return;
			}

			if (not holidaytype) {
				msg = "Your previous timesheet ("
					^ idate.oconv("[DATE,4*]")
					^ ")|has only "
					^ tothours.oconv("[MT2]")
					^ " hours on it but the|minimum allowed is "
					^ (win.registerx(1).a(2)).oconv("[MT2]")
					^ ".||Please complete your|previous timesheet first.";
				gosub invalid();
				win.reset = 5;
			}

		}
	}

	//copy jobs from previous timesheet except closed jobs
	if (timesheet) {
		var dictjobs;
		if (not(dictjobs.open("DICT", "JOBS"))) {
			dictjobs = "";
		}
		RECORD.r(1, timesheet.a(1));
		//dont copy hours
		//@record<2>=timesheet<2>
		RECORD.r(3, timesheet.a(3));
		RECORD.r(4, timesheet.a(4));
		var nlines = (RECORD.a(1)).count(VM) + (RECORD.a(1) ne "");
		for (var linen = nlines; linen >= 1; --linen) {
			jobno = RECORD.a(1, linen);
			gosub checkclosed();
			if (jobno == "" or msg) {
				RECORD.eraser(1, linen);
				RECORD.eraser(3, linen);
				RECORD.eraser(4, linen);
			}else{
				//dont carry forward details on sundry clients
				//to ensure they enter details every time
				if (dictjobs) {
					var job;
					if (job.read(agy.jobs, jobno)) {
						var sundryclient = calculate("SUNDRY", dictjobs, jobno, job, 0);
						if (sundryclient) {
							RECORD.r(3, linen, "");
						}
					}
				}

			}
		};//linen;
	}

	return;

}

libraryexit()
