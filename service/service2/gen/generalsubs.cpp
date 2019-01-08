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
#include <authorised.h>
#include <singular.h>

#include <gen.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var mode;
var datasetparams;
var msg;
var datasetcodes;
var xx;
var usercode;
var usern0;//num
var usern;//num
var depts;
var reply;
var tt;
var nusers;//num
var deptn;
var directory;
var lastbackup;
var wsmsg;

function main(in mode0) {
	//c gen

	//global tt,nusers,usern,mode

	//nb general.subs can only be called from programs with AREV.COMMON2
	//because msg etc are common variables and must be defined in caller

	mode = mode0;

	//eg create all brands, jobs, accountindex for clients
	//WINDOWSTUB GENERAL.SUBS CALLSUBS,CLIENTS,CLIENT.SUBS,POSTWRITE
	if (mode.field(",", 1) == "CALLSUBS") {
		win.datafile = mode.field(",", 2);
		var subname = mode.field(",", 3);
		var submode = mode.field(",", 4);
		if (not(win.srcfile.open(win.datafile, ""))) {
			call fsmsg();
			return 0;
		}
		if (not(DICT.open("DICT", win.datafile))) {
			call fsmsg();
			return 0;
		}
		printl();
		printl("GENERAL.SUBS ", mode);
		if (not LISTACTIVE) {
			win.srcfile.select();
		}
		var recordn = 0;
nextrecord:
		if (not readnext(ID)) {
			return 0;
		}
		if (ID[1] == "%") {
			goto nextrecord;
		}
		recordn += 1;
		print(var().at(0), var().at(-4), recordn, ". ", ID);
		if (not(RECORD.read(win.srcfile, ID))) {
			goto nextrecord;
		}
		win.orec = "";
		//call client.subs('POSTWRITE')
		//call ubname(submode);
		goto nextrecord;

	} else if (mode == "GETDATASETS") {
		gosub getdatasets();
		ANS = datasetparams;

	} else if (mode == "PROTECT") {
		if (win.is ne win.isorig) {
			msg = "You cannot change this.";
			gosub invalid(msg);
			DATA ^= "" "\r";
			return 0;
		}

	} else if (mode == "VAL.DATASET") {

		if (win.is == win.isorig) {
			return 0;
		}

		gosub getdatasets();

		if (not(datasetcodes.a(1).locateusing(win.is, VM, xx))) {
			msg = DQ ^ (win.is ^ DQ) ^ " is not a dataset";
			return invalid(msg);
		}

	} else if (mode.field(",", 1) == "GETUSERDEPTX") {
		//does not popup any errormessage
		gosub getuserdept2();
		if (ANS == "") {
			ANS = "Deleted";
		}

	} else if (mode.field(",", 1) == "GETUSERDEPT") {
		gosub getuserdept2();
		if (ANS == "") {
			msg = DQ ^ (usercode ^ DQ) ^ " - USER DOES NOT EXIST";
			return invalid(msg);
		}
		return 0;

	} else if (mode == "GETGROUPUSERS") {

		var groupusers = "";
		if (SECURITY.a(1).locateusing(USERNAME, VM, usern0)) {
			//add lower users in group
			for (usern = usern0 + 1; usern <= 9999; ++usern) {
				usercode = SECURITY.a(1, usern);
			///BREAK;
			if (not(usercode and usercode ne "---")) break;;
				groupusers ^= VM ^ usercode;
			};//usern;
			//add higher users in group
			for (usern = usern0 - 1; usern >= 1; --usern) {
				usercode = SECURITY.a(1, usern);
			///BREAK;
			if (not(usercode ne "---")) break;;
				groupusers ^= VM ^ usercode;
			};//usern;
		}

		ANS = groupusers;
		ANS.splicer(1, 1, "");

	} else if (mode == "GETDEPTS") {
		gosub getdepts();
		ANS = depts;

	} else if (mode == "F2.DEPARTMENT") {
		gosub getdepts();
		if (not(decide("Which department do you want?", depts ^ "", reply))) {
			return 0;
		}
		ANS = depts.a(reply);
		DATA ^= ANS ^ "\r";

	} else if (mode == "VAL.DEPARTMENT") {
		if (win.is == "") {
			return 0;
		}
		gosub getdepts();
		if (not(depts.locateusing(win.is, FM, xx))) {
			msg = DQ ^ (win.is ^ DQ) ^ " IS NOT A VALID DEPARTMENT";
			return invalid(msg);
		}

	} else if (mode == "VAL.EXCH.RATE") {
		if (win.is.substr(1,2) == "1/" or win.is[1] == "/") {
			tt = win.is.field("/", 2);
			if (tt and tt.isnum()) {
				win.is = ((1 / win.is.substr(3,99)).oconv("MD90P")) + 0;
			}else{
badexchrate:
				msg = DQ ^ (win.is ^ DQ) ^ " - EXCHANGE RATE MUST BE NUMERIC";
				return invalid(msg);
			}
		}else{
			if (not win.is.isnum()) {
				goto badexchrate;
			}
		}

	} else if (mode.field(".", 1, 2) == "DEF.SK" or mode.field(".", 1, 2) == "DEF.SK2") {

		if (not(win.wlocked or RECORD)) {
			//is.dflt=nextkey(':%SK%:':datafile,'')

			//special defaults for special files
			if (mode ne "DEF.SK2") {

				//ratecards
				if (win.datafile == "RATECARDS") {
					if (not(ID.field("*", 2))) {
						ID = ID.fieldstore("*", 2, 1, "1/1/" ^ (var().date()).oconv("D2/E").field("/", 3)).iconv("D2/E");
						win.isdflt = ID;
						return 0;
					}
				}

				//batches
				if (win.datafile == "BATCHES") {
					call daybooksubs3("DEF.BATCH.REF");
					win.isdflt = ID;
					return 0;
				}

				//schedules
				if (win.datafile == "PLANS" or win.datafile == "SCHEDULES") {
					call plansubs5("DEF.REF." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 0;
				}

				//jobs
				if (win.datafile == "JOBS") {
					call jobsubs("DEF.JOB.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 0;
				}

				//timesheets
				if (win.datafile == "TIMESHEETS") {
					call timesheetsubs("DEF.KEY." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 0;
				}

				//production orders
				if (win.datafile == "PRODUCTION_ORDERS") {
					call prodordersubs("DEF.ORDER.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 0;
				}

				//production estimates
				if (win.datafile == "PRODUCTION_INVOICES") {
					call prodinvsubs("DEF.QUOTE.NO." ^ mode.field(".", 3, 999));
					win.isdflt = ID;
					return 0;
				}

				//documents
				if (win.datafile == "DOCUMENTS") {
					call getsubs("DEF.DOCUMENT.NO");
					win.isdflt = ID;
					return 0;
				}

			}

			//convert SK in datafile to SK in definitions
			var no;
			if (not(no.readv(DEFINITIONS, win.datafile ^ ".SK", 1))) {
				if (no.readv(win.srcfile, "%SK%", 1)) {
					win.srcfile.deleterecord("%SK%");
					
				}else{
					no = 1;
				}
				no.writev(DEFINITIONS, win.datafile ^ ".SK", 1);
				
			}

next:

			var params = ":" ^ win.datafile ^ ".SK:DEFINITIONS";

			win.isdflt = nextkey(params, "");
			if (xx.read(win.srcfile, win.isdflt)) {
				if (win.isdflt.isnum()) {
					win.isdflt += 1;
					win.isdflt.writev(DEFINITIONS, win.datafile ^ ".SK", 1);
					
					goto next;
				}
			}
			ANS = "";
		}

	} else if (mode == "DEF.CURRENCY") {
		if (win.is == "") {
			win.is = gen.company.a(3);
			if ((gen.company.a(1).ucase()).index("Promopub", 1)) {
				win.is = "AED";
			}
		}

	} else if (mode == "VAL.CURRENCY") {
		if (not win.is) {
			return 0;
		}
		if (not(xx.read(gen.currencies, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - currency code not on file";
			return invalid(msg);
		}

	} else if (mode == "DEF.COMPANY") {
		if (win.is == "") {
			win.isdflt = fin.currcompany;
		}

	} else if (mode == "VAL.COMPANY") {
		if (not win.is) {
			return 0;
		}

		if (not(authorised("COMPANY ACCESS " ^ (DQ ^ (win.is ^ DQ)), msg, ""))) {
			return invalid(msg);
		}

		if (not(xx.read(gen.companies, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - company code not on file";
			return invalid(msg);
		}

	} else if (mode.field(".", 1, 2) == "VAL.ALPHANUMERIC") {
		if (win.is ne win.isorig) {

			//do not validate if already on file
			if (win.ww[win.wi].a(4) == 0) {
				if (not win.wlocked and RECORD == "" and ID == "") {
					if (RECORD.read(win.srcfile, win.is)) {
						return 0;
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
				call note(msg);
			}
		}

	} else {
		var().chr(7).output();
		call mssg(DQ ^ (mode ^ DQ) ^ " - invalid mode ignored");
		//valid=0
	}
//L1837:
	return 0;

}

subroutine getdepts() {
	depts = "";
	nusers = (SECURITY.a(1)).count(VM) + 1;
	for (usern = 2; usern <= nusers + 1; ++usern) {
		var text = SECURITY.a(1, usern);
		if (text == "---" or text == "") {
			text = SECURITY.a(1, usern - 1);
			text.converter("0123456789", "");
			text.trimmer();
			if (text and text ne "---") {
				if (not(depts.locateusing(text, FM, deptn))) {
					depts.r(-1, text);
				}
			}
		}
	};//usern;

	return;

}

subroutine getdatasets() {
	var dospath = oscwd().substr(1,2) ^ "..\\data\\";

	var dosfilename = APPLICATION ^ ".vol";
	if (not directory.osread(dosfilename)) {
		if (not(directory.osread(APPLICATION ^ ".vox"))) {
			call mssg(APPLICATION ^ ".vol is missing");
			var().stop();
		}
	}

	var nodata = directory.field("\r", 1)[-1] == " ";
	//call msg(nodata:' ')
	//convert dos text to revelation format and standardise
	directory.ucaser();
	directory = directory.substr(1,0x1A).trim();
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
		var datasetcode = temp.a(1, 1, 2);
		var datasetname = temp.a(1, 1, 1);
		if (not(("..\\DATA\\" ^ datasetcode ^ "\\GENERAL\\REVMEDIA.LK").osfile())) {
			datasetname = "*" ^ datasetname;
			datasetparams.r(1, datasetn, 1, datasetname);
		}
		datasetcodes.r(1, datasetn, datasetcode);
		datasetnames.r(1, datasetn, datasetname);
		if (not(lastbackup.osread("..\\data\\" ^ temp.a(1, 1, 2) ^ "\\params2"))) {
			lastbackup = "";
		}
		lastbackup = lastbackup.a(2);
		datasetparams.r(1, datasetn, 4, lastbackup.oconv("D"));
	};//datasetn;

	return;

}

subroutine getuserdept2() {
	//locate the user in the table
	usercode = mode.field(",", 2);
	if (not(SECURITY.a(1).locateusing(usercode, VM, usern))) {
		if (usercode == "NEOSYS") {
			ANS = "NEOSYS";
			return;
		}else{
			ANS = "";
			return;
		}
	}

	//locate divider, or usern+1
	nusers = (SECURITY.a(1)).count(VM) + 1;
	for (usern = 1; usern <= nusers; ++usern) {
	///BREAK;
	if (SECURITY.a(1, usern) == "---") break;;
	};//usern;

	//get the department code
	ANS = SECURITY.a(1, usern - 1);
	return;

}


libraryexit()
