#include <exodus/library.h>
libraryinit()

//#include <ubname.h>
#include <daybooksubs3.h>
#include <plansubs5.h>
#include <jobsubs.h>
#include <timesheetsubs.h>
#include <prodordersubs.h>
#include <prodinvsubs.h>
#include <getsubs.h>
#include <nextkey.h>
#include <singular.h>

#include <win.h>
#include <gen.h>
#include <fin.h>

#include <window.hpp>

var datasetparams;
var msg;
var datasetcode;
var datasetcodes;
var xx;
var usercode;
//var usern;//num
var depts;
var reply;
var tt;
var deptn;
var directory;
var lastbackup;

function main(in mode) {

	var thisprogram="generalsubs";

	if (mode == "GETDATASETS") {
		gosub getdatasets();
		ANS = datasetparams;

/*
	} else if (mode == "DEF.DATASET") {
		if (not win.is) {
			goto f2dataset;
		}

	} else if (mode == "F2.DATASET") {
f2dataset:
		gosub getdatasets();

		//users selects a dataset
		var qq = "Which dataset do you want?";
		var datasetn = pop_up(0, 1, "", datasetparams, "1:39\\2:10\\3:10:::Users\\4:12:::Last Backup", "F", "", qq, "", "", 1, "P");

		if (datasetn) {
			datasetcode = datasetcodes.a(1, datasetn);
			var datasetname = datasetparams.a(1, datasetn, 1);
		}else{
			datasetcode = "";
			var datasetname = "";
		}
		ANS = datasetcode;
		if (ANS) {
			mv.DATA ^= ANS ^ "\r";
		}

	} else if (mode == "VAL.DATASET") {

		if (win.is == win.isorig) {
			return 0;
		}

		gosub getdatasets();

		if (not(datasetcodes.locate(win.is, xx, 1))) {
			msg = DQ ^ (win.is ^ DQ) ^ " is not a dataset";
			return invalid();
		}

	} else if (mode.field(",", 1) == "GETUSERDEPTX") {
		//does not popup any errormessage
		gosub getuserdept();
		if (ANS == "") {
			ANS = "Deleted";
		}
*/
	} else if (mode.field(",", 1) == "GETUSERDEPT") {
		gosub getuserdept(mode);
		if (ANS == "") {
			msg = DQ ^ (usercode ^ DQ) ^ " - USER DOES NOT EXIST";
			return invalid();
		}

	} else if (mode == "GETDEPTS") {
		gosub getdepts();
		ANS = depts;
/*
	} else if (mode == "F2.DEPARTMENT") {
		gosub getdepts();
		if (not(decide("Which department do you want?", depts ^ "", reply))) {
			return;
		}
		ANS = depts.a(reply);
		mv.DATA ^= ANS ^ "\r";

	} else if (mode == "VAL.DEPARTMENT") {
		if (win.is == "") {
			return;
		}
		gosub getdepts();
		if (not(depts.locate(win.is, xx))) {
			msg = DQ ^ (win.is ^ DQ) ^ " IS NOT A VALID DEPARTMENT";
			return invalid();
		}

	} else if (mode == "VAL.EXCH.RATE") {
		if (win.is.substr(1, 2) == "1/" or win.is[1] == "/") {
			tt = win.is.field("/", 2);
			if (tt and tt.isnum()) {
				win.is = ((1 / win.is.substr(3, 99)).oconv("MD90P")) + 0;
			}else{
badexchrate:
				msg = DQ ^ (win.is ^ DQ) ^ " - EXCHANGE RATE MUST BE NUMERIC";
				return invalid();
			}
		}else{
			if (not win.is.isnum()) {
				goto badexchrate;
			}
		}
*/
	} else if (mode.field(".", 1, 2) == "DEF.SK" or mode.field(".", 1, 2) == "DEF.SK2") {

		if (not(win.wlocked or RECORD)) {
			//is.dflt=nextkey(':%SK%:':datafile,'')

			//special defaults for special files
			if (mode ne "DEF.SK2") {

				//batches
				if (win.datafile == "BATCHES") {
					call daybooksubs3("DEF.BATCH.REF");
					win.isdflt = ID;
					return 1;
				}

				//schedules
				if (win.datafile == "PLANS" or win.datafile == "SCHEDULES") {
					call plansubs5("DEF.REF." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 1;
				}

				//jobs
				if (win.datafile == "JOBS") {
					call jobsubs("DEF.JOB.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 1;
				}

				//timesheets
				if (win.datafile == "TIMESHEETS") {
					call timesheetsubs("DEF.KEY." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 1;
				}

				//production orders
				if (win.datafile == "PRODUCTION.ORDERS") {
					call prodordersubs("DEF.ORDER.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 1;
				}

				//production estimates
				if (win.datafile == "PRODUCTION.INVOICES") {
					call prodinvsubs("DEF.QUOTE.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 1;
				}

				//documents
				if (win.datafile == "DOCUMENTS") {
					call getsubs("DEF.DOCUMENT.NO");
					win.isdflt = ID;
					return 1;
				}

			}

			//convert SK in datafile to SK in definitions
			var no;
			if (not(no.readv(gen._definitions, win.datafile ^ ".SK", 1))) {
				if (no.readv(win.srcfile, "%SK%", 1)) {
					win.srcfile.deleterecord("%SK%");
				}else{
					no = 1;
				}
				no.writev(gen._definitions, win.datafile ^ ".SK", 1);
			}


			do {
				var params = ":" ^ win.datafile ^ ".SK:DEFINITIONS";

				win.isdflt = nextkey(params, "");
				if (xx.read(win.srcfile, win.isdflt)) {
					if (win.isdflt.isnum()) {
						win.isdflt += 1;
						win.isdflt.writev(gen._definitions, win.datafile ^ ".SK", 1);
						continue;
					}
				}
				ANS = "";
				break;
			} while (true);
		}
/*
	} else if (mode == "DEF.CURRENCY") {
		if (win.is == "") {
			win.is = gen.company.a(3);

			if ((gen.company.a(1).ucase()).index("Promopub", 1)) {
				win.is = "AED";
			}
		}

	} else if (mode == "F2.CURRENCY") {
		call catalyst("P", "POPUPS*CURRENCY");
		if (ANS) {
			mv.DATA ^= "" "\r";
		}

	} else if (mode == "F2.CURRENCIES") {
		call catalyst("P", "POPUPS*CURRENCIES");
		if (ANS) {
			mv.DATA ^= "" "\r";
		}

	} else if (mode == "VAL.CURRENCY") {
		if (not win.is) {
			return 1;
		}
		if (not(xx.read(gen.currencies, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - currency code not on file";
			return invalid();
		}
*/
	} else if (mode == "DEF.COMPANY") {
		if (win.is == "") {
			win.isdflt = fin.currcompany;
		}
/*
	} else if (mode == "F2.COMPANY" or mode == "F2.COMPANIES") {
		var storedict = DICT;
//		call safeselect("SELECT COMPANIES WITH AUTHORISED (S)");
		var().select("SELECT COMPANIES WITH AUTHORISED (S)");
		if (mode == "F2.COMPANY") {
			tt = "COMPANIES";
		}else{
			tt = "MCOMPANIES";
		}
		call catalyst("P", "POPUPS*" ^ tt);
		if (ANS) {
			mv.DATA ^= ANS ^ "\r";
		}
		DICT = storedict;

	} else if (mode == "VAL.COMPANY") {
		if (not win.is) {
			return 1;
		}

		if (not(authorised("COMPANY FILE ACCESS " ^ (DQ ^ (win.is ^ DQ)), msg, ""))) {
			return invalid();
		}

		if (not(xx.read(gen.companies, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - company code not on file";
			return invalid();
		}

	} else if (mode.field(".", 1, 2) == "VAL.ALPHANUMERIC") {
		if (win.is ne win.isorig) {

			//do not validate if already on file
			if (win.ww[win.wi].a(4) == 0) {
				if (not win.wlocked and RECORD == "" and ID == "") {
					var RECORD;
					if (RECORD.read(win.srcfile, win.is)) {
						return;
					}
				}
			}

			tt = win.is;
			tt.converter("0123456789" ^ LOWERCASE ^ UPPERCASE, "");
			//remove allowable non-alphanumeric characters
			tt.converter(mode.field(".", 3), "");
			if (tt ne "") {
				msg = "PLEASE USE ALPHANUMERIC CHARACTERS ONLY|THE " ^ (DQ ^ (tt ^ DQ)) ^ " CHARACTERS HAVE BEEN IGNORED";
				win.is.converter(tt, "");
				gosub EOF_429();
			}
		}
*/
	} else {
		var().chr(7).output();
		call mssg(DQ ^ (mode ^ DQ) ^ " - invalid mode ignored in " ^ thisprogram);
		return invalid();
	}

	return 1;

}

subroutine getuserdept(in mode) {
	//locate the user in the list of users
	usercode = mode.field(",", 2);
	var usern;
	if (not(gen._security.locate(usercode, usern, 1))) {
		if (usercode == "NEOSYS") {
			ANS = "NEOSYS";
			return;
		}else{
			ANS = "";
			return;
		}
	}

	//locate divider, or usern+1
	var nusers = (gen._security.a(1)).count(VM) + 1;
	for (usern = 1; usern <= nusers; ++usern) {
		///BREAK;
		if (gen._security.a(1, usern) == "---")
			break;
	};//usern;

	//get the department code
	ANS = gen._security.a(1, usern - 1);
	return;
}

subroutine getdepts() {
	depts = "";
	var nusers = (gen._security.a(1)).count(VM) + 1;
	for (var usern = 2; usern <= nusers + 1; ++usern) {
		var text = gen._security.a(1, usern);
		if (text == "---" or text == "") {
			text = gen._security.a(1, usern - 1);
			text.converter("0123456789", "");
			text.trimmer();
			if (text and text ne "---") {
				if (not(depts.locate(text, deptn))) {
					depts.r(-1, text);
				}
			}
		}
	};//usern;

	return;

}

subroutine getdatasets() {
	var dospath = (var().osdir()).substr(1, 2) ^ "..\\data\\";

	var dosfilename = ACCOUNT ^ ".vol";
	if (not directory.osread(dosfilename)) {
		if (not(directory.osread(ACCOUNT ^ ".vox"))) {
			call mssg(ACCOUNT ^ ".vol is missing");
			var().stop();
		}
	}

	var nodata = (directory.field("\r", 1))[-1] == " ";
	//call msg(nodata:' ')
	//convert dos text to revelation format and standardise
	directory.ucaser();
	directory = directory.substr(1, 0x1A).trim();
	var dosformat = directory.index(var().chr(13), 1);
	directory.converter(" " ^ FM ^ "\r\n", FM ^ " " " " " ");
	directory.trimmer();
	directory.converter(" " ^ FM, FM ^ " ");
	var nvols = directory.count(FM) + 1;

	datasetparams = directory.a(1);
	if (not(datasetparams.index(",", 1))) {
		//CALL MSG('LINE 1 OF THE DOS FILE ':DOS.FILENAME:' IS INVALID')
		//STOP
	}

	datasetparams.converter(",*", SVM ^ VM);
	var subst = datasetparams.field(" ", 1);
	datasetparams = datasetparams.field(" ", 2, 9999);
	var ndatasets = datasetparams.count(VM) + 1;

	datasetcodes = "";
	var datasetnames = "";

	for (var datasetn = 1; datasetn <= ndatasets; ++datasetn) {
		var temp = datasetparams.a(1, datasetn);
		datasetcodes.r(1, datasetn, temp.a(1, 1, 2));
		datasetnames.r(1, datasetn, temp.a(1, 1, 1));
		if (not(lastbackup.osread("..\\data\\" ^ temp.a(1, 1, 2) ^ "\\params2"))) {
			lastbackup = "";
		}
		lastbackup = lastbackup.a(2);
		datasetparams.r(1, datasetn, 4, lastbackup.oconv("D"));
	};//datasetn;

	return;

}


libraryexit()
