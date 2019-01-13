#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <openfile.h>
#include <select2.h>
#include <timesheetsubs.h>
#include <sysmsg.h>
#include <holiday.h>
#include <agencysubs.h>
#include <generalsubs2.h>
#include <safeselect.h>
#include <chklic.h>
#include <flushindex.h>
#include <generalsubs.h>
#include <quote2.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var msg;
var usercode;
var xml;
var cols;
var datax;
var idate;//num
var timesheet;
var getholidaytypedate;
var holidaytype;
var USER;
var marketcode;
var market;
var personn;
var xx;
var tt;
var v69;
var v70;
var v71;
var jobno;
var ln;//num
var job;
var userisadmin;
var client;
var priordate;
var wsmsg;

function main(in mode) {
	//c tim
	//global priordate,job

	//confuses with existing msg
	//equ msg to @user4

	//why was this changed? <10> is supposed to be days grace before ts reminders
	//equ maxdaysdelay to register(1)<4>
	//equ jobs to register(5)
	//equ security.net to register(9)

	var interactive = not SYSTEM.a(33);

	win.registerx(7) = authorised("TIMESHEET ADMINISTRATION", msg, "TA");
	//register(8)=msg
	var notadminmsg = msg;

	//if jobs='' then if openfile('JOBS',jobs) else jobs=''

	if (mode == "OPTIONSETUP") {
		//used to setup dos mode window

		if (not(authorised("TIMESHEET ADMINISTRATION", msg, ""))) {
			gosub invalid(msg);
			var().stop();
		}

		gosub getregisterparams();

		PSEUDO = "";
		PSEUDO.r(1, win.registerx(1).a(1));
		PSEUDO.r(2, win.registerx(1).a(2));
		PSEUDO.r(3, win.registerx(1).a(3));
		PSEUDO.r(4, win.registerx(1).a(4));
		PSEUDO.r(5, win.registerx(1).a(5));

	} else if (mode == "OPTIONSAVE") {
		//used to update from dos mode window

		RECORD.write(DEFINITIONS, "TIMESHEET.PARAMS");

	} else if (mode.substr(1,8) == "POSTINIT") {
		win.valid = 1;
		gosub security(mode);
		if (not win.valid) {
			return 0;
		}
		if (not win.valid) {
			var().stop();
		}

		gosub getregisterparams();

		if (not(openfile("JOB_TYPES", win.registerx(4)))) {
			win.registerx(4) = "";
		}
		//if openfile('JOBS',jobs) else jobs=''

		//flag to init.general that the timesheet is not required
		win.registerx(6) = PSEUDO;
		if (win.registerx(1).a(2) == "") {
			win.registerx(6) = "";
		}

		if (mode == "POSTINIT2") {

			usercode = USERNAME;
			xml = "";
			cols = "CODE NAME";
			gosub getactivities();

			win.registerx(4) = "";
			//jobs=''

			if (win.registerx(7)) {
				//equ response to @user3
				//call select2('USERS','','BY DEPT_AND_USER_NAME WITH DEPARTMENT_CODE NE @ID','USER_CODE USER_AND_DEPT_NAME','',datax,response)
				call select2("USERS", "", "BY RANK WITH DEPARTMENT_CODE NE @ID", "USER_CODE USER_AND_DEPT_NAME", "", datax, USER3, "", "", "");
				datax.transfer(win.registerx(8));
			}

			//security.net=security
			//!except passwords
			//register(8)<4>=''

		}

	} else if (mode.field(".", 1) == "GETNEXTTIMESHEET") {
		ID = mode.field(".", 2, 9999);
		call timesheetsubs("PREREAD");
		USER1 = ID;
		USER3 = "OK";

	} else if (mode == "PREREAD") {

		idate = (ID.field("*", 2, 999)).field("~", 1);
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
		///BREAK;
		if (timesheet) break;;
		};//idate;

		getholidaytypedate = idate;
		gosub getholidaytype();

		if (timesheet == "") {

			//no previous timesheet found
			// - enter today (even if it is a holiday)
			idate = var().date();

		} else if (timesheet and not holidaytype and (timesheet.a(2)).sum() < win.registerx(1).a(2)) {

			//found incomplete timesheet
			// - enter it
			{}

		} else {

			//previous full working or holiday timesheet found
			// - enter next working day after that
			var nextworkingday = 1;

			call holiday("GETWORKDATE", idate, usercode, USER, marketcode, market, agy.agp, holidaytype, nextworkingday);

			//never go beyond today even if today is a holiday
			if (idate > var().date()) {
				idate = var().date();
			}

		}
//L941:
		ID = ID.fieldstore("*", 2, 1, idate);

		//case mode='DEF.PERSON'
		// if is then return
		// @ans=@username
		// if isadmin else datax is

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
		if (not SECURITY.a(1).locateusing(win.is, VM, personn)) {
baduser:
			msg = DQ ^ (win.is ^ DQ) ^ " - person not authorised";
			gosub invalid(msg);
			return 0;
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
			gosub invalid(msg);
			return 0;
		}

	} else if (mode == "VAL.JOB") {
		if (win.is == win.isorig) {
			return 0;
		}

		call agencysubs("VAL.JOB.OPEN", xx);
		if (not win.valid) {
			return 0;
		}

	} else if (mode == "VAL.HOURS") {

		if (win.is == "") {
			return 0;
		}

		//check numeric
		if (not win.is.isnum()) {
			msg = "Please enter a number of hours";
			gosub invalid(msg);
			return 0;
		}

		//check not negative
		if (win.is < 0) {
			msg = "Please enter number of hours greater than 0";
			gosub invalid(msg);
			return 0;
		}

		//check minimum time unit
		if (win.registerx(1).a(5)) {
			var tsminhours2 = win.registerx(1).a(5) / 60;
			var is2 = (win.is / tsminhours2).floor() * tsminhours2;
			if (is2 ne win.is) {
				msg = "Warning: The minimum time interval is " ^ win.registerx(1).a(5) ^ " minutes";
				call note(msg);
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
					gosub invalid(msg);
					return 0;
				}
				call note(msg);
			}

		}

	} else if (mode == "F2.ACTIVITY") {
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
	/*;
		case mode='VAL.ACTIVITY';

			//check required
			if is='' and tsactivitycoderequired and wi.next>=wi then;
				msg='activity code is required';
				gosub invalid;
				return 0;
				end;

			if is=is.orig then return 0;

			//check is on file
	getactivity:
			read activity from activities,is else;

				//search activity names
				nactivities=count(activitycodes,vm)+1;
				activitycodes2='';
				for activityn=1 to nactivities;
					activitycode=activitycodes<1,activityn>;
					reado activity from activities,activitycode else activity='';
					if index(ucase(activity<1>:' ':activitycode),is,1) then;
						activitycodes2<1,-1>=activitycode;
						end;
					next activityn;

				if activitycodes2='' then;
					msg=quote(is):' - is not an activity code';
					gosub invalid;
					return 0;
					end;

				//user selects or cancels
				q='Which do you want ?';
				if count(activitycodes2,vm) then;
					ans=pop.up(0,2,activities,activitycodes2:'','1:30:::Activity\0:4:::Code','T','',q,'','','','K');
					if ans else valid=0;return 0;
					is=ans;
				end else;
					is=activitycodes2;
					@ans=is;
					end;

				goto getactivity;

				end;

			//check activity code is valid for this person
			locate is in activitycodes<1> setting activityn else;
				msg=quote(is):' - This activity code is|not allowed for this person';
				gosub invalid;
				return 0;
				end;

			//set/update the details
			details=@record<3,@mv>;
			if is.orig then;
				read oldactivity from activities,is.orig else oldactivity='';
			end else;
				oldactivity='';
				end;
			if oldactivity and index(details,oldactivity<1>,1) then;
				swap oldactivity<1> with activity<1> in details;
			end else;
				details=trim(details:' ':activity<1>);
				end;
			@record<3,@mv>=details;
			t=3;gosub redisplay;
	*/
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
		if (not win.valid) {
			return 0;
		}

		//check if amending in authorised period
		if (win.wlocked) {
			call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
			if (msg) {
				//comment to client
				win.reset = -1;
				call note(msg);
				gosub unlockrec();
			}
		}

		gosub getregisterparams();

		//restrictions for standard users ie not-timesheet admins
		if (not win.registerx(7)) {
	//if @username='NEOSYS' or not(isadmin) then

			//check if allowed to enter or edit old timesheets
			if (win.wlocked and win.registerx(1).a(10)) {

				var fromdate = ID.field("*", 2);
				var uptodate = var().date();
				var daysdelay = uptodate - fromdate;
				//exclude non-working days of any type (personal/weekend/general etc)
				for (getholidaytypedate = fromdate; getholidaytypedate <= uptodate; ++getholidaytypedate) {
					gosub getholidaytype();
					if (holidaytype) {
						daysdelay -= 1;
					}
				};//getholidaytypedate;
				if (daysdelay > win.registerx(1).a(10)) {
					msg = "Sorry, you cannot enter or edit timesheets|older than " ^ win.registerx(1).a(10) ^ " working days.|Your timesheet administrator can.";
					gosub invalid(msg);
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
					//msg='Sorry, you cannot enter or edit timesheets|more than ':tsmaxdaysadvance+0:' days in advance.'
					tt = win.registerx(1).a(7) + 0;
					msg = "Sorry, you cannot enter or edit timesheets|more than " ^ tt ^ " days in advance.";
					xx = unlockrecord("TIMESHEETS", win.srcfile, ID);
					win.wlocked = 0;
					gosub invalid(msg);
					win.reset = 5;
				}
			}

			//check if allowed to modify approved timesheets
			if (win.wlocked and RECORD.a(8) == "APPROVED") {
				call mssg("This timesheet is already approved and cannot be edited unless a timesheet administrator changes the status");
				//xx=unlockrecord(datafile,src.file,@id)
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

				call pushselect(0, v69, v70, v71);
				var cmd = "SSELECT JOBS";
				var minjobcreationdate = var().date() - (ndays - 1);
				cmd ^= " WITH DATE_CREATED GE " ^ (DQ ^ (minjobcreationdate.oconv("[DATE]") ^ DQ));
				cmd ^= " AND WITH CLOSED NE \"Y\" AND WITH AUTHORISED";
				call safeselect(cmd);
				var tamvfns = "1" _VM_ "100" _VM_ "101" _VM_ "3" _VM_ "4" _VM_ "2";
				var namvs = tt.count(VM) + 1;
nextjob:
				if (readnext(jobno)) {
					if (not RECORD.a(1).locateusing(jobno, VM, ln)) {
						//insert a new blank line at the top
						for (var amvn = 1; amvn <= namvs; ++amvn) {
							//fn=amv.fns<1,amvn>
							var fn = tamvfns.a(1, amvn);
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
				call popselect(0, v69, v70, v71);

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
				if (job.read(agy.jobs, jobnos.a(1, jobn))) {
					RECORD.r(100, jobn, (RECORD.a(100, jobn) ^ " ").trimb() ^ job.a(9, 1));
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
		if (not win.valid) {
			return 0;
		}

		//standard users may not be able to enter or edit old timesheets
		if (not win.registerx(7) and win.registerx(1).a(10)) {
			var daysdelay = var().date() - ID.field("*", 2);
			if (daysdelay > win.registerx(1).a(10)) {
				xx = unlockrecord("TIMESHEETS", win.srcfile, ID);
				win.wlocked = 0;
				msg = "Sorry, you cannot enter or edit timesheets|older than " ^ win.registerx(1).a(10) ^ " days.";
				gosub invalid(msg);
				win.reset = 5;
			}
		}

	} else if (mode.field(".", 1) == "PREWRITE") {

		//prevent creating new records if no lic
		call chklic(mode, msg);
		if (msg) {
			gosub invalid(msg);
			return 0;
		}

		usercode = ID.field("*", 1);
		idate = ID.field("*", 2);

		//(double) check if authorised to approve
		var approving = mode.field(".", 2) == "APPROVING";
		if (approving) {
			if (not(authorised("TIMESHEET APPROVAL", msg, "TAP"))) {
				gosub invalid(msg);
				return 0;
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			gosub invalid(msg);
			return 0;
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
		//removed to allow saving on timer start
		//if tothours else
		// msg='The total hours:minutes cannot be zero'
		// gosub invalid
		// return
		// end

		//check max hours (if any) always
		if (win.registerx(1).a(3)) {
			if (tothours > win.registerx(1).a(3)) {
				msg = "The total hours entered is " ^ tothours ^ " but";
				msg = " the maximum allowed is " ^ win.registerx(1).a(3);
				gosub invalid(msg);
				return 0;
			}
		}

		//autosave if done for timer timesheet (timer is only available for today)
		//but anybody can save less time for the current date

		//check min hours (of non-timesheet admin timesheets)
		if (not calculate("AUTOSAVED") and win.registerx(1).a(2) and tothours < win.registerx(1).a(2) and not userisadmin) {

			//anybody can enter less for the current and future dates
			//but it cannot be approved
			//TODO idate is users local tz date so shouldnt be checked v. server date()
			if (idate < var().date() or approving) {

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
					tt = win.registerx(1).a(2) + 0;
					msg ^= "the minimum allowed is " ^ tt;
					gosub invalid(msg);
					return 0;
				}

			}

		}

		//check details entered for sundry clients
		MV = 0;
		if (agy.clients.open("CLIENTS", "")) {
			var clientcodes = calculate("CLIENT_CODE");
			var nlns = clientcodes.count(VM) + 1;
			for (ln = 1; ln <= nlns; ++ln) {
				if (not RECORD.a(3, ln) and RECORD.a(2, ln)) {
					var clientcode = clientcodes.a(1, ln);
					if (not client.read(agy.clients, clientcode)) {
						client = "";
					}
					//msg=quote(clientcode):' is missing from the client file'
					//gosub invalid
					//return
					//end
					if (client.a(38)) {
						//msg='Details are missing for ':client<1>
						msg = client.a(1) ^ " is a \"Sundry Client\" (See Client/Brand File)";
						msg.r(-1, FM ^ "Please enter the client name/details in line " ^ ln);
						gosub invalid(msg);
						return 0;
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
					gosub invalid(msg);
					return 0;
				}
			};//linen;
		}

		//remove joined job/brand data (provided to old mac ie5 clients)
		//@record<100>=''
		//@record<101>=''

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTWRITE") {

		call flushindex("TIMESHEETS");

		//flag to init.general that the timesheet has been entered
		//register(6) is pseudo
		if (ID == win.registerx(6).a(1)) {
			win.registerx(6).r(2, ID);
		}

	} else if (mode == "POSTDELETE") {

		call flushindex("TIMESHEETS");

	} else if (mode == "PREDELETE") {
		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTAPP") {
		PSEUDO = win.registerx(6);

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - unknown mode skipped in TIMESHEET.SUBS";
		gosub invalid(msg);
		return 0;
	}
//L3524:
	return 0;

}

subroutine getactivities() {
	//VERY similar code in timesheet.subs and jobproxy GETACTIVITIES
	//TODO deduplicate

	//get dept
	if (usercode) {
		call generalsubs("GETUSERDEPT," ^ usercode);
		win.registerx(2) = ANS.trim();
	}else{
		win.registerx(2) = "";
	}

	//departments with numbers on the end are all the same
	//department for the sake of activities
	win.registerx(2).converter("0123456789", "");

	//select
	call select2("JOB_TYPES", "", "WITH DEPARTMENT " ^ quote2(win.registerx(2)), cols, xml, datax, USER3);
	win.registerx(3) = datax;

	//convert fm to vm in activitycodes

	return;

}

subroutine getregisterparams() {
	//open 'JOBS' to jobs else
	// call fsmsg()
	// stop
	// end
	if (not((win.registerx(1)).read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
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
	if (not USER.read(users, usercode)) {
		msg = DQ ^ (usercode ^ DQ) ^ " user does not exist";
		gosub invalid(msg);
		return;
	}

	//should be determined per user
	//TODO work out first company of each user
	//in TIMESHEET.SUBS and ANALTIME2
	marketcode = gen.company.a(30, 1);
	if (not market.read(agy.markets, marketcode)) {
		market = "";
		msg = DQ ^ (marketcode ^ DQ) ^ " Market is missing";
		gosub invalid(msg);
		return;
	}

	call holiday("GETTYPE", getholidaytypedate, usercode, USER, marketcode, market, agy.agp, holidaytype, xx);

	return;

}

subroutine getprevioustimesheet() {

	//get jobs from prior date up to 31 days ago
	for (priordate = idate - 1; priordate >= idate - 31; --priordate) {
		if (not(timesheet.read(gen.timesheets, usercode ^ "*" ^ priordate))) {
			timesheet = "";
		}
	///BREAK;
	if (timesheet) break;;
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
				//MT2 is a GBP PROGRAM!
				msg = "Your previous timesheet (" ^ idate.oconv("[DATE,4*]") ^ ")|has only " ^ tothours.oconv("MT2") ^ " hours on it but the|minimum allowed is " ^ (win.registerx(1).a(2)).oconv("MT2") ^ ".||Please complete your|previous timesheet first.";
				gosub invalid(msg);
				win.reset = 5;
			}

		}
	}

	//copy jobs from previous timesheet except closed jobs
	if (timesheet) {
		var dictjobs;
		if (not(dictjobs.open("dict_JOBS"))) {
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

	//deduplicate same job/activity/details
	//now that timer can cause multiple entries for one job/activity
	var nfns = var("1,2,3,4,10,11").count(",") + 1;
	var nlines = (RECORD.a(1)).count(VM) + 1;
	for (ln = nlines; ln >= 2; --ln) {
		//if job and activity are the same then delete the lower line
		//different details are ignored so only the first details are retained
		if (RECORD.a(1, ln) == RECORD.a(1, ln - 1)) {
			if (RECORD.a(4, ln) == RECORD.a(4, ln - 1)) {
				for (var fnn = 1; fnn <= nfns; ++fnn) {
					var fn = var("1,2,3,4,10,11").field(",", fnn);
					RECORD.eraser(fn, ln);
				};//fnn;
			}
		}
	};//ln;
	return;

}

subroutine checkclosed() {
	msg = "";
	//check job has not been closed
	if (job.read(agy.jobs, jobno)) {
		if (job.a(7) == "Y") {
			msg = DQ ^ (win.is ^ DQ) ^ " Job is closed";
		}
	}
	return;

}

subroutine unlockrec() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}


libraryexit()
