#include <exodus/library.h>
libraryinit()

#include <generalsubs.h>
#include <getsubs.h>
#include <nextkey.h>
#include <authorised.h>
#include <singular.h>

//#include <sys_common.h>
#include <win_common.h>

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
var no;//num
var nusers;//num
var deptn;
var directory;
var lastbackup;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode0) {
	//c gen
	//#include <sys_common.h>

	//global tt,nusers,usern,mode

	//nb general.subs can only be called from programs with win_common.h
	//because msg etc are common variables and must be defined in caller

	mode = mode0;

	if (mode == "GETDATASETS") {
		gosub getdatasets();
		ANS = datasetparams;

	} else if (mode == "VAL.DATASET") {

		if (win.is == win.isorig) {
			return 0;
		}

		gosub getdatasets();

		if (not(datasetcodes.a(1).locate(win.is,xx))) {
			msg = win.is.quote() ^ " is not a dataset";
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
			msg = usercode.quote() ^ " - USER DOES NOT EXIST";
			return invalid(msg);
		}
		return 0;

	} else if (mode == "GETGROUPUSERS") {

		var groupusers = "";
		if (SECURITY.a(1).locate(USERNAME,usern0)) {
			//add lower users in group
			for (usern = usern0 + 1; usern <= 9999; ++usern) {
				usercode = SECURITY.a(1, usern);
				///BREAK;
				if (not(usercode and usercode ne "---")) break;
				groupusers ^= VM ^ usercode;
			}//usern;
			//add higher users in group
			for (usern = usern0 - 1; usern >= 1; --usern) {
				usercode = SECURITY.a(1, usern);
				///BREAK;
				if (not(usercode ne "---")) break;
				groupusers ^= VM ^ usercode;
			}//usern;
		}

		ANS = groupusers;
		ANS.splicer(1, 1, "");

	} else if ((mode.field(".", 1, 2) == "DEF.SK") or (mode.field(".", 1, 2) == "DEF.SK2")) {

		if (not(win.wlocked or RECORD)) {
			//is.dflt=nextkey(':%SK%:':datafile,'')

			//special defaults for special files
			if (mode ne "DEF.SK2") {

				//documents
				if (win.datafile == "DOCUMENTS") {
					call getsubs("DEF.DOCUMENT.NO");
					win.isdflt = ID;
					return 0;
				}

			}

			//convert SK in datafile to SK in definitions
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

	} else {
		var().chr(7).output();
		call mssg(mode.quote() ^ " - invalid mode ignored");
		//valid=0
	}

	return 0;
}

subroutine getdatasets() {
	var dospath = oscwd().substr(1,2) ^ "../data/";
	dospath.converter("/", OSSLASH);

	var dosfilename = APPLICATION ^ ".vol";
	if (not(directory.osread(dosfilename))) {
		if (not(directory.osread(APPLICATION ^ ".vox"))) {
			call mssg(APPLICATION ^ ".vol is missing");
			stop();
		}
	}

	var nodata = directory.field("\r", 1)[-1] == " ";
	//call msg(nodata:' ')
	//convert dos text to revelation format and standardise
	directory.ucaser();
	//DIRECTORY=TRIM(DIRECTORY[1,\1A\]);*DOS TEXT EOF IS CONTROL-Z
	var dosformat = directory.index(var().chr(13));
	directory.converter(" " ^ FM ^ "\r\n", FM ^ " " " " " ");
	directory.trimmer();
	directory.converter(" " ^ FM, FM ^ " ");
	var nvols = directory.count(FM) + 1;

	datasetparams = directory.a(1);
	if (not(datasetparams.index(","))) {
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
		tt = "../data/" ^ datasetcode.lcase() ^ "/general/revmedia.lk";
		tt.converter("/", OSSLASH);
		if (not(tt.osfile())) {
			datasetname = "*" ^ datasetname;
			datasetparams.r(1, datasetn, 1, datasetname);
		}
		datasetcodes.r(1, datasetn, datasetcode);
		datasetnames.r(1, datasetn, datasetname);
		tt = "../data/" ^ temp.a(1, 1, 2).lcase() ^ "/params2";
		tt.converter("/", OSSLASH);
		if (not(lastbackup.osread(tt))) {
			lastbackup = "";
		}
		lastbackup = lastbackup.a(2);
		datasetparams.r(1, datasetn, 4, lastbackup.oconv("D"));
	}//datasetn;

	return;
}

subroutine getuserdept2() {
	//locate the user in the table
	usercode = mode.field(",", 2);
	if (not(SECURITY.a(1).locate(usercode,usern))) {
		if (usercode == "EXODUS") {
			ANS = "EXODUS";
			return;
		}else{
			ANS = "";
			return;
		}
	}

	//locate divider, or usern+1
	nusers = SECURITY.a(1).count(VM) + 1;
	for (usern = 1; usern <= nusers; ++usern) {
		///BREAK;
		if (SECURITY.a(1, usern) == "---") break;
	}//usern;

	//get the department code
	ANS = SECURITY.a(1, usern - 1);
	return;
}

libraryexit()
