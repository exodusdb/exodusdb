#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <holiday.h>
#include <timedate2.h>
#include <getmark.h>
#include <printtx.h>
#include <sendmail.h>
#include <gethtml.h>
#include <readcss.h>
#include <docmods.h>

#include <gen.h>
#include <agy.h>

var printptr;//num
var tt;//num
var testtoaddress;
var neosystesting;//num
var testccaddress;
var sendmailx;//num
var reminderapproval;//num
var msg;
var nemails;//num
var nerrors;//num
var alltoaddresses;
var link;
var fmx;
var vmx;
var fromdate;//num
var uptodate;//num
var idate;//num
var period;
var isntadmin;
var usercode;
var xx;
var marketcode;
var market;
var timesheet;
var groupapprovers;
var lasttimesheetadmin;
var userx;
var istimesheetadmin;
var istimesheetapprover;
var today;
var usermarketcode;
var usermarket;
var userholidaytypetoday;
var emailaddress;
var latestrequireddate;
var holidaytype;
var usermissingdates;
var unapprovedday;//num
var jobno;
var linen;//num
var insufficient;
var toaddress;
var ccaddress;
var subject;
var body;
var attachfilename;
var deletex;//num
var newpage;//num
var useremail;
var clientmark;
var tags;
var userholidaytypes;
var userholidaytype;
var linetothours;
var printfilename;
var errormsg;
var printfile;
var letterhead;
var realpagen;//num
var bottomline;
var printtxmark;
var rfmt;
var foot;
var nbodylns;//num
var headx;
var newpagetag;
var css;
var cssver;
var style;
var htmltitle;

function main() {
	//
	//c tim
	// http://localhost/neosys/2/neosys/dblink.htm?EXECUTE\rPRODUCTION\rAPPROVETIMESHEET?DONATELLA?14968?14974.62844

	//global idate,insufficient,usermissingdates,latestrequireddate,istimesheetadmin
	//global usercode,linetothours,sendmailx,tt,marketcode,ccaddress,testtoaddress,testccaddress
	//global neosystesting,printptr,istimesheetapprover,groupapprovers,lasttimesheetadmin
	//global today,userx,market

	printptr = 0;
	var interactive = not SYSTEM.a(33);
	var approverreminders = 1;

	var timesheetparams;
	if (not(timesheetparams.read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
		timesheetparams = "";
	}

	var users;
	if (not(users.open("USERS", ""))) {
		call fsmsg();
		var().stop();
	}

	//pseudox<1> period (from sentence 2 actually)
	//pseudox<2> 1=byuserandjob 2=consolidated 3=reminders 4=approvals 5 rem&app
	//pseudox<3> usercodes (sentence 4 is blank)
	//pseudox<4> 31=last 31 days (eg for NEOS_TSSU timesheet summary alert)
	var pseudox = PSEUDO;

	var baselinks = SYSTEM.a(114);
	var baselinkdescs = SYSTEM.a(115);

	//following is not necessary when new init.general2 is installed
	//condition the links
	//remove any obvious page addresses
	//ensure ends in slash
	if (not baselinks) {
		var baselink = "Configuration File";
	}
	if (not baselinkdescs) {
		baselinkdescs = "Pending Configuration";
	}
	var nlinks = baselinks.count(VM) + (baselinks ne "");
	for (var linkn = 1; linkn <= nlinks; ++linkn) {
		tt = baselinks.a(1, linkn);
		if (tt) {
			var tt2 = (field2(tt, "/", -1)).ucase();
			if (tt2.index(".HTM", 1)) {
				tt.splicer(-tt2.length(), tt2.length(), "");
			}
			if (not(var("\\/").index(tt[-1], 1))) {
				tt ^= "/";
			}
			baselinks.r(1, linkn, tt);
		}
	};//linkn;

	//note this may be autorun as NEOSYS so cannot say if NEOSYS then testing
	//neosystesting=@username='NEOSYS'
	testtoaddress = timesheetparams.a(17);
	neosystesting = testtoaddress.length();
	//force testing if on development machine
	if (not neosystesting) {
		if (var("NEOSYS.ID").osfile()) {
			neosystesting = 1;
			testtoaddress = "sb2@neosys.com";
		}
	}
	if (not testtoaddress) {
		testtoaddress = "sb2@neosys.com";
	}
	testccaddress = "";

	sendmailx = 1;
	//sendmailx=0
	if (not neosystesting) {
		sendmailx = 1;
	}

	//mode is 1;Personal:2;Consolidated:3;Reminder Emails:4;Approval Emails
	//reminderapproval is 0=timesheets,1=reminder,2=approval,3=both
	var mode = pseudox.a(2);
	if (mode > 2) {
		reminderapproval = mode - 2;
		mode = 0;
	}else{
		reminderapproval = 0;
	}

	//default go live date before which all timesheets are test
	//if timesheet.params<8>='' then
	// timesheet.params<8>=iconv('1/1/2009','D/E')
	// end

	//time at which expected to have prior days timesheet in
	//the general idea is that timesheets for the previous working day are filled
	//sometime during the morning and approved in the afternoon
	//reminders go out at 12:00 for yesterdays timesheets
	//so people have time to fill them in before or during lunch
	//and approvals go out at 14:00
	if (timesheetparams.a(9) == "") {
		timesheetparams.r(9, var("12:00").iconv("MT"));
	}

	//days grace (not used for reminders anymore)
	//due to confusion with <4> for locking
	//if timesheet.params<10>='' then
	timesheetparams.r(10, 1);
	// end

	//above should all go into timesheet params sometime

	//scheduled runs of this program is probably after 12:00
	//but if run before x then allow an additional days grace
	//no user interface to configure this
	//localtime=mod(time()+@sw<1>,86400)
	//if localtime<tsmintime then
	//?! doesnt do anything because is equ to <10>
	// tsdaysgrace+=1
	// end

	var sortbyjobno = 1;
	//hoursfmt='MD20PZ'
	//MT2 is a GBP PROGRAM!
	var hoursfmt = "MT2,Z]";
	var nbsp = "&nbsp;";

	var topmargin = 0;
	var tx = "";
	//y2k
	if (not(authorised("TIMESHEET PRINTOUT", msg, ""))) {
		var().chr(7).output();
		call mssg(msg);
		var().stop();
	}

	if (not(DICT.open("dict_TIMESHEETS"))) {
		call fsmsg();
		var().stop();
	}

	var html = 1;
	var r5 = "[TAGHTML,td]";
	var r10 = "[TAGHTML,td]";
	var l20 = "[TAGHTML,td ALIGN=LEFT]";
	var sep = "";
	var tr = "<tr>";
	//trr='<tr align=right>'
	var trr = "<tr>";
	var trx = "</tr>";
	var th = "<th>";
	var thx = "</th>";
	var td = "<td>";
	var tdx = "</td>";

	//remove options
	var temp = SENTENCE.index(" (P", 1);
	if (temp) {
		SENTENCE = SENTENCE.substr(1,temp - 1);
	}

	if (reminderapproval) {
		nemails = 0;
		nerrors = 0;
		alltoaddresses = "";

		//link=baselink
		link = "2/neosys/dblink.htm?";
		link ^= "ADAGENCY?";
		link ^= SYSTEM.a(17) ^ "?";
		//skip user
		link ^= "?";
		//skip pass
		link ^= "?";
		link ^= "EXECUTE\\rPRODUCTION\\rAPPROVETIMESHEET?";
		fmx = "\\u255E";
		vmx = "\\u255D";
	}

	var firstdayn = 0;
	var lastdayn = 0;
	var daysback = pseudox.a(4);

	if (reminderapproval or daysback) {

		if (not daysback) {
			daysback = 31;
		}
		//look over a maximum of 31 days
		fromdate = var().date() - (daysback - 1);
		uptodate = var().date();

		//dont allow approval of todays timesheets
		if (reminderapproval == 2) {
			uptodate -= 1;
		}

		//skip all timesheets before this date (when system went live)
		if (fromdate < timesheetparams.a(8)) {
			fromdate = timesheetparams.a(8);
		}

	}else{

		//determine period
		fromdate = SENTENCE.field(" ", 2);
		//if fromdate else fromdate=date() 'D'
		if (fromdate) {
			idate = fromdate.iconv("D/E");
			if (not idate) {
				call mssg(DQ ^ (fromdate ^ DQ) ^ " is not a valid date");
				var().stop();
				}
			period = idate.oconv("D2/E").substr(-5,5);
		}else{
			period = "";
		}

		//input period
		if (not period) {
			period = var().date().oconv("D/E").field("/", 2, 2);
			if (period[1] == "0") {
				period.splicer(1, 1, "");
			}
			if (interactive) {
inpperiod:
				call note("Which month do you want ?", "RC", period, "");
				if (not period) {
					var().stop();
				}
			}
			period.converter(" ", "");
		}

		//convert fromdate to period
		fromdate = var("1/" ^ period).iconv("D/E");
		if (not fromdate) {
			var().chr(7).output();
			call mssg(DQ ^ (period ^ DQ) ^ " must be month/year");
			if (not interactive) {
				var().stop();
			}
			goto inpperiod;
		}

		//find end of month
		var month = period.field("/", 1);
		for (idate = fromdate; idate <= fromdate + 9999; ++idate) {
		///BREAK;
		if (idate.oconv("D/E").field("/", 2) ne month) break;;
		};//idate;
		uptodate = idate - 1;

	}

	//call msg(uptodate 'D/E')

	if (not(authorised("TIMESHEET ADMINISTRATION", isntadmin, ""))) {
		{}
	}

	//determine usercodes
	var usercodes = pseudox.a(3);
	if (not usercodes) {
		usercodes = SENTENCE.field(" ", 4, 999);
		if (usercodes == "PERSONAL") {
			usercodes = USERNAME;
		}
	}
	if (not usercodes) {
		if (isntadmin) {
			usercode = USERNAME;
		}else{
			usercodes = SECURITY.a(1);
			var usercodes2 = "";
			var nusers = usercodes.count(VM) + 1;
			for (var usern = nusers; usern >= 1; --usern) {
				usercode = usercodes.a(1, usern);

				//not reverse order
				if (reminderapproval) {
					if (usercode ne "---") {
						if (not(authorised("TIMESHEET ACCESS", xx, "", usercode))) {
							usercodes.eraser(1, usern);
						}
					}

					//reverse order
				}else{
					if (usercode == "---") {
						if (usercodes2 and (usercodes2.substr(-3,3) ne "---")) {
							usercodes2 ^= VM ^ "---";
						}
					}else{
						if (authorised("TIMESHEET ACCESS", xx, "", usercode)) {
							usercodes2 ^= VM ^ usercode;
						}
					}
				}

			};//usern;
			if (not reminderapproval) {
				usercodes2.transfer(usercodes);
				if (usercodes[1] == VM) {
					usercodes.splicer(1, 1, "");
				}
			}
		}
	}

	//validate usercodes
	if (usercodes ne USERNAME) {
		if (isntadmin) {
			call mssg(isntadmin);
			var().stop();
		}
	}

	//should be determined per user
	//TODO work out first company of each user
	//in TIMESHEET.SUBS and ANALTIME2
	marketcode = gen.company.a(30, 1);
	if (not(market.read(agy.markets, marketcode))) {
		call mssg(DQ ^ (marketcode ^ DQ) ^ " Market is missing");
		var().stop();
	}

	//which dow do admin approval emails go out to timesheet admins
	//fixed to first day after weekend or monday at the moment
	//last day of weekend
	tt = market.a(9);
	tt = tt.a(1, 1, tt.count(SVM) + 1);
	//last day of week
	if (tt == "") {
		tt = agy.agp.a(13);
	}
	//or sunday
	if (tt == "") {
		tt = 7;
	}
	//1 day AFTER last day of weekend/week,sunday
	var adminapprovaldows = tt % 7 + 1;

	//approval emails for timesheet admins only go out once a week
	//depends on first day of week - 2 means send on tuesday if start of week is monday

	/////////
	//inituser:
	/////////
	var byuserandjob = (mode < 2) or reminderapproval;

	//initialise summary by usercodes
	var unapproveddays = "";
	if (not byuserandjob) {
		//timesheet=userprivs<1>
		timesheet = usercodes.a(1);
		//list of selected users becomes the consolidated lines
		if (pseudox.a(3)) {
			timesheet = pseudox.a(3);
		}
	}

	groupapprovers = "";
	lasttimesheetadmin = "";
	//remindersummary=''

	//nusers=count(userprivs<1>,vm)+1
	var nusers = usercodes.count(VM) + 1;
	var usern = "";

	/////////
nextuser:
	/////////

	usern += 1;
	if (usern > nusers) {

		//email the last batch of approvals
		if (byuserandjob) {
			if (reminderapproval) {
				gosub sendreminderapproval();
			}
			gosub exit();
			var().stop();
		}

		goto userinit;
	}

	//pass through them all in order now
	//skipping the undesired ones
	//usercode=usercodes<1,usern>
	//usercode=userprivs<1,usern>
	usercode = usercodes.a(1, usern);
	if ((usercode == "") or (usercode == "---")) {

		//deal with any timesheet approvals for the previous group
		if (reminderapproval and printptr) {
			gosub sendreminderapproval();
		}

		//if remindersummary then
		// gosub remindersummary
		// end

		groupapprovers = "";
		//remindersummary=''

		goto nextuser;
	}

	if (not(userx.read(users, usercode))) {
		userx = usercode;
	}

	//determine users company and reget market if changed
	//TODO

	istimesheetadmin = authorised("TIMESHEET ADMINISTRATION", xx, "", usercode);
	istimesheetapprover = authorised("TIMESHEET APPROVAL", xx, "", usercode);

	gosub getusermarket();

	//determine if the user is on holiday today
	today = var().date();
	call holiday("GETTYPE", today, usercode, userx, usermarketcode, usermarket, agy.agp, userholidaytypetoday, xx);

	//allow testing on holidays (ie do not skip)
	if (neosystesting) {
		userholidaytypetoday = "";
	}

	if (byuserandjob) {

		if (userx.a(35) and (var().date() >= userx.a(35))) {
			emailaddress = "";
		}else{
			emailaddress = userx.a(7);
		}

		if (reminderapproval) {

			//record timesheet admins (depending on day of week)
			var currentdow = (var().date() - 1) % 7 + 1;
			if (adminapprovaldows.index(currentdow, 1)) {
				if (istimesheetadmin) {
					if (emailaddress) {
						lasttimesheetadmin = emailaddress;
					}
				}
			}

			//record group timesheet approvers email addresses for later use
			if (istimesheetapprover) {

				if (emailaddress) {
					groupapprovers.r(-1, emailaddress);
				}

				//option to skip timesheet admins/approvers
				//simplify to avoid c++ compiler warning
				//if istimesheetadmin or ( not(approverreminders) and istimesheetapprover) then
				tt = not approverreminders and istimesheetapprover;
				if (istimesheetadmin or tt) {
					//if istimesheetadmin else de bug
					goto nextuser;
				}
				//!# means default to NOT authorised (ie if no lock specified)
				//if security('#TIMESHEET AVOID REMINDER',xx) then goto nextuser

			}
			if (reminderapproval == 1) {
				//# means default to NOT authorised (ie if no lock specified)
				if (authorised("#TIMESHEET AVOID REMINDER", xx, "", usercode)) {
					goto nextuser;
				}
			}
		}

		timesheet = "";
	}

	//skip unselected users
	if (not(usercodes.a(1).locateusing(usercode, VM, xx))) {
		goto nextuser;
	}

	if (reminderapproval) {

		//skip users who have not logged in the last 31 days
		//arguably cant do this since ts may be entered by someone else
		//if user<13> lt (fromdate-30) then goto nextuser

		//skip users who are the bottom departmental user
		//cant do this because such ts would go to jobs too
		//if usercode=user<21> then goto nextuser

		//if nobody to approve then skip the user (only if doing approvals)
		if (reminderapproval == 2) {
			if (not groupapprovers and not lasttimesheetadmin) {
				goto nextuser;
			}
		}

	}

	//determine the latest required timesheet
	latestrequireddate = var().date();
	tt = -timesheetparams.a(10);
	call holiday("GETWORKDATE", latestrequireddate, usercode, userx, usermarketcode, usermarket, agy.agp, holidaytype, tt);
	//call msg(usercode:' latest required:':latestrequireddate 'D/E')

	if (byuserandjob) {
		firstdayn = 0;
		lastdayn = 0;
	}

	//get any timesheets
	//dayn=0
	unapproveddays = "";
	usermissingdates = "";
	for (idate = fromdate; idate <= uptodate; ++idate) {
		//dayn+=1
		var dayn = idate - fromdate + 1;
		ID = usercode ^ "*" ^ idate;

		if (idate < timesheetparams.a(8)) {
			goto nextdate;
		}

		if (RECORD.read(gen.timesheets, ID)) {

			var approved = RECORD.a(8) == "APPROVED";

			//need unapproveddays for approval emails and ordinary printouts
			//if reminderapproval then
			if (byuserandjob) {

				if (approved) {

					//if emailing for approval then skip approved ts older than minappdays days
					var minappdays = 7;
					minappdays = 0;
					if (reminderapproval >= 2) {
						if ((idate < var().date() - minappdays - 1) and not firstdayn) {
							goto nextdate;
						}
					}

					//"0" not "" to indicate ts present but approved already
					unapprovedday = 0;

				}else{

					//unapprovedday indictes it is pending approval
					unapprovedday = RECORD.a(6);

				}

				unapproveddays.r(dayn, unapprovedday);
			}

			//record first and last dates discovered
			if (not firstdayn or (dayn < firstdayn)) {
				firstdayn = dayn;
			}
			if (dayn > lastdayn) {
				lastdayn = dayn;
			}

			var nlines = RECORD.a(1).count(VM) + 1;
			for (MV = 1; MV <= nlines; ++MV) {
				var hours = RECORD.a(2, MV);
				var details = RECORD.a(3, MV);
				var activitycode = RECORD.a(4, MV);
				if (activitycode) {
					if (details) {
						details.splicer(1, 0, ": ");
					}
					details.splicer(1, 0, activitycode.xlate("JOB_TYPES", 1, "C"));
				}

				if (hours) {

					if (byuserandjob) {
						jobno = RECORD.a(1, MV);
					}else{
						jobno = ID.field("*", 1);
					}

					if (sortbyjobno) {
						if (not(timesheet.a(1).locateby(jobno, "AL", linen))) {
							timesheet.inserter(1, linen, jobno);
							timesheet.inserter(2, linen, "");
							timesheet.inserter(3, linen, "");
							timesheet.inserter(4, linen, "");
						}
					}else{
						if (not(timesheet.a(1).locateusing(jobno, VM, linen))) {
							timesheet.r(1, linen, jobno);
						}
					}

					timesheet.r(2, linen, dayn, timesheet.a(2, linen, dayn) + hours);

					if (not byuserandjob and approved) {
						timesheet.r(4, linen, dayn, 1);
					}

					if (reminderapproval) {

						if (unapprovedday) {
							tt = timesheet.a(3, linen, dayn);
							//garbagecollect;
							tt ^= "<br>" ^ hours.oconv(hoursfmt);

							if (details) {
								tt ^= "<br>" ^ details;
							}

							timesheet.r(3, linen, dayn, tt);
						}

					}

				}

			};//MV;

			//check min hours
			var tothours = RECORD.a(2).sum();
			if (timesheetparams.a(2) and (tothours < timesheetparams.a(2))) {
				insufficient = tothours;
				goto insufficienthours;
			}

			//nothing more here - put above min hours check above

		}else{
			insufficient = "";

insufficienthours:

			//insufficient (or just no timesheet)

			//handle missing timesheet
			if ((idate >= timesheetparams.a(8)) and (idate <= latestrequireddate)) {
				xx = "";
				call holiday("GETTYPE", idate, usercode, userx, usermarketcode, usermarket, agy.agp, holidaytype, xx);

				//handle not holiday
				if (not holidaytype) {
					usermissingdates.r(1, -1, idate ^ "*" ^ insufficient);
				}

			}
		}

nextdate:
		//null to help c++ decompiler
		{}
	};//idate;

	//ndays=dayn

	//skip user if nothing to be approved
	if (reminderapproval) {

		//send reminder email if user is missing anything and is not on holiday today
		if (usermissingdates and not userholidaytypetoday) {

			//toaddress=user<7>
			if (userx.a(35) and (var().date() >= userx.a(35))) {
				toaddress = "";
			}else{
				toaddress = userx.a(7);
			}
			ccaddress = "";

			//invoke approver for people without emails IF any timesheets entered
			if ((toaddress == "") and timesheet.a(1)) {
				//toaddress=ccaddress
				toaddress = groupapprovers;
				ccaddress = "";
			}

			//missing date list
			var missingdatelist = "";
			var nn = usermissingdates.a(1).count(VM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				var idatemissing = usermissingdates.a(1, ii).field("*", 1);
				missingdatelist ^= " " ^ idatemissing.oconv("[DATE,4*]");
			};//ii;
			missingdatelist.splicer(1, 1, "");

			//remindersummary<-1>=user<1>:' ':user<7>:' ':missingdatelist

			if (reminderapproval ne 2 and toaddress) {

				var usermissingdate = usermissingdates.a(1, 1).field("*", 1);
				var usermissinghours = usermissingdates.a(1, 1).field("*", 2);
				subject = "NEOSYS: Timesheet Reminder for " ^ usercode ^ " " ^ usermissingdate.oconv("[DATE,4*]");

				body = "";
				if (usermissinghours ne "") {
					usermissinghours = (usermissinghours.oconv("MD20P")) + 0;
					body.r(-1, "Kindly complete your timesheet for " ^ usermissingdate.oconv("[DATE,4*]"));
					body.r(-1, "(You entered " ^ usermissinghours ^ " hours but the minimum required is " ^ timesheetparams.a(2) ^ " hours");
				}else{
					body.r(-1, "Kindly enter your timesheet for " ^ missingdatelist);
				}
				for (var linkn = 1; linkn <= nlinks; ++linkn) {
					body.r(-1, FM ^ baselinkdescs.a(1, linkn));
					body.r(-1, baselinks.a(1, linkn) ^ "2/jobs/timesheets.htm");
				};//linkn;

				body.converter(FM, var().chr(13));
				//solve failure to line break in outlook
				body.swapper(var().chr(13) ^ "http", var().chr(13) ^ var().chr(13) ^ "http");

				attachfilename = "";
				deletex = 0;

				gosub sendmailxx();

			}

		}

		//if nothing to approve then skip the user
		//nothing has been output at this stage
		if (not(unapproveddays.sum())) {
			goto nextuser;
		}

	}

	//do all users
	if (byuserandjob) {
		if (timesheet == "") {
			goto nextuser;
		}

		//dont output for admins or approvers
		if (reminderapproval and ((istimesheetadmin or istimesheetapprover))) {

			if (istimesheetadmin) {
				goto nextuser;
			}

			var currentdow = (var().date() - 1) % 7 + 1;
			if (not(adminapprovaldows.index(currentdow, 1))) {
				goto nextuser;
			}

		}

	}else{
		goto nextuser;
	}

	/////////
userinit:
	/////////

	//page heading
	var head = "";

	head ^= "<H2~style=\"margin:0px;text-align:center\">";
	head ^= "TIMESHEET (Hours) ";
	head ^= "</H2>";

	head ^= FM;

	head ^= "<H3>";

	//read user from users,usercode else user=@username
	var username = usercode.xlate("USERS", 1, "C");
	if (not username) {
		username = usercode;
	}

	if (byuserandjob) {
		newpage = 1;
		head ^= FM ^ "Name : ";
		//useremail=user<7>
		if (userx.a(35) and (var().date() >= userx.a(35))) {
			useremail = "";
		}else{
			useremail = userx.a(7);
		}
		if (useremail) {
			head ^= "<a href=\"mailto:" ^ useremail ^ "\">";
		}
		head ^= swap("\'", "\'\'", username);
		if (useremail) {
			head ^= "</a>";
		}
		head ^= FM;
		head ^= nbsp.str(10);
	}

	//head:='Period :' 'L#15':' ':period
	head ^= nbsp.str(10);
	head ^= "As at : " ^ timedate2();
	head ^= FM;
	head ^= "</H3>";

	//col heading
	var colh = "";
	var totallink = "";

	//clientmark=system<14>
	call getmark("CLIENT", html, clientmark);

	//clientwebsite=system<8>
	//t2='<P STYLE="MARGIN-BOTTOM:0" ALIGN=CENTER><small>':clientmark:'</small>'
	//if clientwebsite then t2='<A HREF=':quote(clientwebsite):'>':t2:'</A>'
	//colh:=t2:'<BR>'
	//colh:=clientmark:'<BR>'
	colh ^= FM ^ "<table><tr><td>" ^ clientmark ^ "</td></tr></table>";

	colh ^= FM ^ "<TABLE BGCOLOR=" ^ SYSTEM.a(46, 2) ^ " BORDER=1 CELLSPACING=0 CELLPADDING=2";
	colh ^= " class=neosystable";
	colh ^= " ALIGN=CENTER";
	colh ^= " STYLE=\"{font-size:66%}\"";
	colh ^= "><THEAD>";

	if (byuserandjob) {
		colh ^= tr ^ var("Job No").oconv(l20) ^ sep;
	}else{
		//colh:=tr:'Name' l20:sep
		colh ^= tr ^ var("Name<br /><small>* Approver<br />**Administrator</small>").oconv(l20) ^ sep;
	}

	if (byuserandjob) {
		colh ^= var("Job Description").oconv("[TAGHTML,td ALIGN=LEFT]") ^ sep;
	}

	//for dayn=1 to ndays
	var mthname = "";
	for (var dayn = firstdayn; dayn <= lastdayn; ++dayn) {
		colh ^= th;
		idate = fromdate + dayn - 1;
		var mthname2 = gen.glang.a(2).field("|", idate.oconv("D2/E").field("/", 2));
		if (mthname2 ne mthname) {
			mthname = mthname2;
			colh ^= mthname ^ nbsp;
		}
		colh ^= idate.oconv("D2/E").field("/", 1) + 0;
		colh ^= "<br>" ^ (gen.glang.a(21).field("|", (idate - 1) % 7 + 1)).substr(1,2);

		//if reminderapproval then
		colh ^= "<br>";
		unapprovedday = unapproveddays.a(dayn);
		if (unapprovedday) {
			if (reminderapproval) {
				colh ^= "<a href=\"" ^ baselinks.a(1, 1) ^ link;
				colh ^= usercode ^ fmx ^ idate ^ fmx ^ unapprovedday;
				//totallink<1,-1>=usercode
				//totallink<2,-1>=idate
				//totallink<3,-1>=unapprovedday
				//combine into multipart code for compact URL
				//usercode prefixed before insertion
				totallink.r(1, -1, "*" ^ idate ^ "*" ^ unapprovedday);
				colh ^= "\">Approve</a>";
			}else{
				colh ^= nbsp;
			}
		}else{
			if (unapprovedday == "") {
				colh ^= nbsp;
			}else{
				if (reminderapproval) {
					colh ^= "Approved";
				}else{
					//shorter column headings
					colh ^= "OK";
				}
			}
		}
		// end

		colh ^= thx;
		//colh:=sep
	};//dayn;

	colh ^= th;
	colh ^= "Total";

	//provide a link to approve all if more than one
	if (reminderapproval) {
		colh ^= "<br><br>";
		if (totallink.a(1).count(VM)) {
			totallink.splicer(1, 0, usercode);
			colh ^= "<a href=\"" ^ baselinks.a(1, 1) ^ link;
			totallink.swapper(VM, vmx);
			totallink.swapper(FM, fmx);
			colh ^= totallink;
			colh ^= "\">Approve</a>";
		}else{
			colh ^= nbsp;
		}
	}

	colh ^= thx ^ trx;

	colh ^= "</THEAD><TBODY>";
	colh.swapper("<td", "<th");
	colh.swapper("</td", "</th");
	head ^= colh;
	head.swapper("<th", "\r\n" " <th");

	//rows
	var nlines = timesheet.a(1).count(VM) + 1;

	//precalculate daytots
	var daytots = "";
	for (linen = 1; linen <= nlines; ++linen) {
		var timesheetline = timesheet.a(2, linen);
		for (var dayn = firstdayn; dayn <= lastdayn; ++dayn) {
			var hours = timesheetline.a(1, 1, dayn);
			daytots.r(dayn, daytots.a(dayn) + hours);
		};//dayn;
	};//linen;

	var colors = "";

	for (linen = 1; linen <= nlines; ++linen) {

	// tx:=trr:(sep:timesheet<1,linen>) l20:sep
		var cell = timesheet.a(1, linen);
		if (not byuserandjob) {
			usercode = cell;
			if (authorised("TIMESHEET ADMINISTRATION", xx, "", usercode)) {
				tags = "**";
			} else if (authorised("TIMESHEET APPROVAL", xx, "", usercode)) {
				tags = "*";
			} else {
				tags = "";
			}
			if (userx.read(users, usercode)) {
				//useremail=user<7>
				//dont send email to expired logins
				if (userx.a(35) and (var().date() >= userx.a(35))) {

					//skip expired users ie dont print a line for them
					if (not(timesheet.a(2, linen).sum())) {
						goto nextlinen;
					}

					useremail = "";
				}else{
					useremail = userx.a(7);
				}
				if (useremail) {
					cell = "<a href=\"mailto:" ^ useremail ^ "\">";
					cell ^= userx.a(1);
					cell ^= "</a>";
				}
				cell ^= " " ^ tags;
			}
		}
		tx ^= trr ^ (sep ^ cell).oconv(l20) ^ sep;

		if (byuserandjob) {
			jobno = timesheet.a(1, linen);
			var job;
			if (not(job.read(agy.jobs, jobno))) {
				job = "";
			}
			tx ^= job.a(9, 1).oconv("[TAGHTML,td ALIGN=LEFT]");
		}

		//determine user holidays
		if (not byuserandjob or (linen == 1)) {

			//get the user per line
			if (not byuserandjob) {
				usercode = timesheet.a(1, linen);
			}
			if (not(userx.read(users, usercode))) {
				userx = usercode;
			}

			gosub getusermarket();

			userholidaytypes = "";
			for (var dayn = firstdayn; dayn <= lastdayn; ++dayn) {

				//determine if the user is on holiday today
				idate = fromdate + dayn - 1;
				xx = "";
				call holiday("GETTYPE", idate, usercode, userx, usermarketcode, usermarket, agy.agp, userholidaytype, xx);

				userholidaytypes.r(dayn, userholidaytype);
			};//dayn;

		}

		linetothours = timesheet.a(2, linen).sum();

		//for dayn=1 to ndays
		for (var dayn = firstdayn; dayn <= lastdayn; ++dayn) {

			var hours = timesheet.a(2, linen, dayn);
			var details = timesheet.a(3, linen, dayn);
			var approved = timesheet.a(4, linen, dayn);
			if (details) {
				details.splicer(1, 4, "");
				//tx:=td:details:tdx
				tt = td ^ details ^ tdx;
			}else{
				//garbagecollect;
				tt = hours.oconv(hoursfmt);
				if (byuserandjob) {
					tt = tt.oconv(r5) ^ sep;
				}else{

					//bold pending approval
					if (not approved and hours) {
						tt = "<B>" ^ tt ^ "</B>";
					}

					tt = tt.oconv(r5) ^ sep;

				}

			}

			//color holidays
			userholidaytype = userholidaytypes.a(dayn);
			if (userholidaytype) {
				if (userholidaytype == 4) {
					//expired login(/not employed?)
					tt.swapper("<td", "<td bgcolor=grey");
				} else if (userholidaytype == 3) {
					//personal
					tt.swapper("<td", "<td bgcolor=lightgrey");
				} else {
					//weekend and public
					tt.swapper("<td", "<td bgcolor=" ^ SYSTEM.a(46, 1));
				}
			}else{

				//none/insufficient hours
				var COLOR = "";
				if (byuserandjob) {
					if (daytots.a(dayn) < timesheetparams.a(2)) {
						COLOR = "white";
					}
				}else{
					if (linetothours and (hours < timesheetparams.a(2))) {
						COLOR = "white";
					}
				}
				if (COLOR) {
					tt.swapper("<td", "<td bgcolor=" ^ COLOR);
				}

			}

			tx ^= tt;
			//day totals in columns
			//if num(daytots<dayn>) and num(hours) else de bug

		};//dayn;

		//final column
		//garbagecollect;
		tx ^= linetothours.oconv(hoursfmt).oconv(r10) ^ trx;

		tx.swapper("<td", "\r\n" " <td");
		tx.swapper("<th", "\r\n" " <th");
		gosub printtx(tx);

nextlinen:
		//null to help c++ decompiler
		{}

	};//linen;

	//total row
	tx ^= var(" Total Hours:").oconv(l20) ^ sep;
	if (byuserandjob) {
		tx ^= var("").oconv(l20);
	}
	//for dayn=1 to ndays
	for (var dayn = firstdayn; dayn <= lastdayn; ++dayn) {
		//garbagecollect;
		var cell = daytots.a(dayn).oconv(hoursfmt).oconv(r5) ^ sep;

		var COLOR = "";
		if (byuserandjob) {
			userholidaytype = userholidaytypes.a(dayn);
			if (userholidaytype == 3) {
				COLOR = "lightgrey";
			} else if (not userholidaytype and (daytots.a(dayn) < timesheetparams.a(2))) {
				COLOR = "white";
			}
		}
		if (COLOR) {
			cell.swapper("<td", "<td style=background-color:" ^ COLOR);
		}

		tx ^= cell;
	};//dayn;
	//garbagecollect;
	tx ^= daytots.sum().oconv(hoursfmt).oconv(r10) ^ FM;
	tx.swapper("<td", "<th");
	tx.swapper("</td", "</th");
	tx ^= "</TBODY></TABLE>";

	var mark = "Timesheet";
	call getmark("OWN", html, mark);
	tx ^= "<table><tr><td>" ^ mark ^ "</td></tr></table>";

	tx.swapper("<td", "\r\n" " <td");
	tx.swapper("<th", "\r\n" " <th");
	gosub printtx(tx);

	/////////
	//userexit:
	/////////

	if (reminderapproval) {
		if (istimesheetapprover) {
			toaddress = lasttimesheetadmin;
			ccaddress = "";
			gosub sendreminderapproval2();
		}
	}else{
		gosub printtx(xx,"pagebreak");
	}

	if (byuserandjob) {
		goto nextuser;
	}

	gosub exit();
	var().stop();

	return 0;

	return "";
}

subroutine exit() {

	//if unassigned(print.ptr) then print.ptr=0
	if (reminderapproval) {
		if (nemails or nerrors) {
			if (reminderapproval == 1) {
				tt = "users";
			} else if (reminderapproval == 2) {
				tt = "approvers";
			} else {
				tt = "users/approvers";
			}
			tt.r(-1, alltoaddresses);
			if (nerrors) {
				tt.r(-1, "Error:");
				tt.r(-1, nerrors ^ " emails could not be sent");
			}
			call mssg(nemails ^ " emails sent to timesheet " ^ tt);
			//analtime returns response messages in response. real errors in msg/@user4
			USER3 = "OK " ^ USER4;
			USER4 = "";
		}
	} else if (not printptr) {
		call mssg("No timesheets have been entered yet|(For the selected users and/or dates)");
	}
//L6083:
	return;

	/*;
	////////////////
	remindersummary:
	////////////////
	return;
		toaddress=groupapprovers;
		ccaddress=topadmin;
		body='Timesheet reminders have been sent to:';
		body:='';
		body<-1>=remindersummary;
		attachfilename='';
		delete=0;

		gosub sendmailxx;

		return;
	*/

}

subroutine sendreminderapproval() {
	if (not printptr) {
		return;
	}

	toaddress = groupapprovers;
	ccaddress = "";

	//invoke timesheet admins
	if (not toaddress) {
		toaddress = lasttimesheetadmin;
	}
}

subroutine sendreminderapproval2() {
	if ((reminderapproval > 1) and toaddress) {

		toaddress.converter(FM, ";");
		ccaddress.converter(FM, ";");
		subject = "NEOSYS: Timesheet Approval";

		body = "@" ^ printfilename;

		attachfilename = "";
		deletex = 0;

		gosub sendmailxx();

	}

	call oswrite("", printfilename);
	printptr = 0;

	return;

}

subroutine sendmailxx() {

	//dont send to real email addresses if testing
	var origtoaddress = toaddress;
	if (neosystesting) {
		goto nottoreal;
	}
	if (var("NEOSYS.ID").osfile()) {

nottoreal:

		subject ^= " (testing for " ^ toaddress;
		if (ccaddress) {
			subject ^= " cc: " ^ ccaddress;
		}
		subject ^= ")";

		toaddress = testtoaddress;
		ccaddress = testccaddress;

	}

	if (not toaddress) {
		ccaddress.transfer(toaddress);
	}
	if (not toaddress) {
		return;
	}

	if (sendmailx) {
		call sendmail(toaddress, ccaddress, subject, body, attachfilename, deletex, errormsg);
	}else{
		call mssg("Sent to " ^ origtoaddress ^ FM ^ "Subject:" ^ FM ^ subject);
		errormsg = "";
	}

	if (errormsg) {
		call mssg("Error sending email to " ^ toaddress ^ FM ^ FM ^ errormsg);
		nerrors += 1;
	}else{
		nemails += 1;
		if (reminderapproval == 1) {
			tt = usercode ^ " ";
		}else{
			tt = "";
		}
		alltoaddresses.r(-1, tt ^ origtoaddress);
	}

	return;

}

subroutine getusermarket() {
	usermarketcode = marketcode;
	usermarket = market;
	tt = userx.a(25);
	if (tt) {
		usermarketcode = tt;
		if (usermarket.read(agy.markets, tt)) {
			usermarketcode = tt;
		}
	}
	return;

}


libraryexit()
