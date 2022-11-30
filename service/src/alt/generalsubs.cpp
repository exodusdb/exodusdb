#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <generalsubs.h>
#include <getsubs.h>
#include <nextkey.h>

#include <req_common.h>

#include <request.hpp>

var mode;
var datasetparams;
//var msg;
var datasetcodes;
//var xx;
var usercode;
//var usern0;	 // num
//var usern;	 // num
var depts;
//var reply;
//var tt;
//var no;		 // num
//var nusers;	 // num
//var deptn;
//var directory;
//var lastbackup;
//var op;
//var op2;
//var wspos;
//var wsmsg;

function main(in mode0) {

	// nb general.subs can only be called from programs with req_common.h
	// because msg etc are common variables and must be defined in caller

	mode = mode0;

	// use app specific version of generalsubs
	if (APPLICATION != "EXODUS") {
		generalsubs = "generalsubs_app";
	}

	if (mode == "GETDATASETS") {
		gosub getdatasets();
		ANS = datasetparams;

	} else if (mode == "VAL.DATASET") {

		if (req.is == req.isorig) {
			return 0;
		}

		gosub getdatasets();

		if (not datasetcodes.f(1).locate(req.is)) {
			let msg = req.is.quote() ^ " is not a dataset";
			return invalid(msg);
		}

	} else if (mode.field(",", 1) == "GETUSERDEPTX") {
		// does not popup any errormessage
		gosub getuserdept2();
		if (ANS == "") {
			ANS = "Deleted";
		}

	} else if (mode.field(",", 1) == "GETUSERDEPT") {
		gosub getuserdept2();
		if (ANS == "") {
			let msg = usercode.quote() ^ " - USER DOES NOT EXIST";
			return invalid(msg);
		}
		return 0;

	} else if (mode == "GETGROUPUSERS") {

		// Gets other users above and below the user, in the same group,
		// excluding the specified user.

		var usern0;
		if (not SECURITY.f(1).locate(USERNAME, usern0)) {
			ANS = "";
			return 0;
		}

		var groupusers = "";

		// Add lower users in group
		//for (usern = usern0 + 1; usern <= 9999; ++usern) {
		for (let usern : range(usern0 + 1, 999999)) {
			usercode = SECURITY.f(1, usern);
			// /BREAK;
			if (usercode == "" or usercode == "---")
				break;
			groupusers ^= usercode ^ VM;
		}  // usern;

		// Add higher users in group
		for (var usern = usern0 - 1; usern >= 1; --usern) {
			usercode = SECURITY.f(1, usern);
			// /BREAK;
			if (usercode == "---")
				break;
			groupusers ^= usercode ^ VM;
		}  // usern;

		ANS = groupusers.pop();

	} else if ((mode.field(".", 1, 2) == "DEF.SK") or (mode.field(".", 1, 2) == "DEF.SK2")) {

		if (not(req.wlocked or RECORD)) {
			// is.dflt=nextkey(':%SK%:':datafile,'')

			// special defaults for special files
			if (mode != "DEF.SK2") {

				// documents
				if (req.datafile == "DOCUMENTS") {
					call getsubs("DEF.DOCUMENT.NO");
					req.isdflt = ID;
					return 0;
				}
			}

			// convert SK in datafile to SK in definitions
			var sk;
			if (not sk.readf(DEFINITIONS, req.datafile ^ ".SK", 1)) {
				if (sk.readf(req.srcfile, "%SK%", 1)) {
					req.srcfile.deleterecord("%SK%");

				} else {
					sk = 1;
				}
				sk.writef(DEFINITIONS, req.datafile ^ ".SK", 1);
			}

next:

			let params = ":" ^ req.datafile ^ ".SK:DEFINITIONS";

			req.isdflt = nextkey(params, "");
			if (var().read(req.srcfile, req.isdflt)) {
				if (req.isdflt.isnum()) {
					req.isdflt += 1;
					req.isdflt.writef(DEFINITIONS, req.datafile ^ ".SK", 1);

					goto next;
				}
			}
			ANS = "";
		}

	} else {
		call mssg(mode.quote() ^ " - invalid mode ignored");
		// valid=0
	}

	return 0;
}

subroutine getdatasets() {

	var dospath = oscwd().first(2) ^ "../data/";
	dospath.converter("/", OSSLASH);

	let dosfilename = APPLICATION ^ ".vol";
	var directory;
	if (not directory.osread(dosfilename)) {
		if (not directory.osread(APPLICATION ^ ".vox")) {
			call mssg(APPLICATION ^ ".vol is missing");
			stop();
		}
	}

	let nodata = directory.field("\r", 1).ends(" ");
	// call msg(nodata:' ')
	// convert dos text to revelation format and standardise
	directory.ucaser();
	// DIRECTORY=TRIM(DIRECTORY[1,\1A\]);*DOS TEXT EOF IS CONTROL-Z
	let dosformat = directory.contains(chr(13));
	directory.converter(" " _FM "\r\n", _FM "   ");
	directory.trimmer();
	directory.converter(" " _FM, _FM " ");
	let nvols = directory.fcount(FM);

	datasetparams = directory.f(1);
	if (not datasetparams.contains(",")) {
		// CALL MSG('LINE 1 OF THE DOS FILE ':DOS.FILENAME:' IS INVALID')
		// STOP
	}

	datasetparams.converter(",*", SM ^ VM);
	let subst	  = datasetparams.field(" ", 1);
	datasetparams = datasetparams.field(" ", 2, 9999);
	let ndatasets = datasetparams.fcount(VM);

	datasetcodes	 = "";
	var datasetnames = "";

	for (const var datasetn : range(1, ndatasets)) {
		let temp		= datasetparams.f(1, datasetn);
		let datasetcode = temp.f(1, 1, 2);
		var datasetname = temp.f(1, 1, 1);

		//var tt			= "../data/" ^ datasetcode.lcase() ^ "/general/revmedia.lk";
		//tt.converter("/", OSSLASH);
		//if (not tt.osfile()) {
			datasetname					  = "*" ^ datasetname;
			datasetparams(1, datasetn, 1) = datasetname;
		//}
		datasetcodes(1, datasetn) = datasetcode;
		datasetnames(1, datasetn) = datasetname;
		var osfilename			  = "../data/" ^ temp.f(1, 1, 2).lcase() ^ "/params2";
		osfilename.converter("/", OSSLASH);
		var lastbackup;
		if (not lastbackup.osread(osfilename)) {
			lastbackup = "";
		}
		lastbackup					  = lastbackup.f(2);
		datasetparams(1, datasetn, 4) = lastbackup.oconv("D");
	}  // datasetn;

	return;
}

subroutine getuserdept2() {

	// Locate the user in the table otherwise department is ""
	let usercode = mode.field(",", 2);
	var usern0;
	if (not SECURITY.f(1).locate(usercode, usern0)) {

		// If EXODUS user isnt in the table then its dept is also EXODUS
		if (usercode == "EXODUS") {
			ANS = "EXODUS";
			return;
		}

		ANS = "";
		return;
	}

	// Locate the next "---" divider
	// and return the department code/name above it
	let nusers = SECURITY.f(1).fcount(VM);
	for (let usern : range(usern0 + 1, nusers)) {
		if (SECURITY.f(1, usern) == "---") {
			ANS = SECURITY.f(1, usern - 1);
			return;
		}
	}  // usern;

	// Otherwise get the last user as the department code
	// TODO Should this be ""
	ANS = SECURITY.f(1, nusers);
	return;
}

libraryexit()
