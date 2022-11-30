#include <exodus/library.h>
libraryinit()

var docrep;
var scrn;
var temp;

function main() {
	//

	// a much simpler non-interactive version of GET

	let where = "";
	let html  = 1;
	var cmd	  = SENTENCE;

	while (cmd) {
		let word1 = field(cmd, " ", 1);
		if (word1 == "GET" or word1 == "NEW")
			cmd = field(cmd, " ", 2, 99999).trimlast();
		else
			break;
	}

	// ///
	// MAKE:
	// ///

	// make up a dos print file name
	var dfs = where ^ var(99999999).rnd().str(8).first(8);
	if (html) {
		dfs ^= ".htm";
	} else {
		dfs ^= ".txt";
	}
	var prnfile = dfs;
	SYSTEM(2)	= prnfile;

	// redirect printed output to a dos file
	//call oswrite("", prnfile);
	if (not oswrite("", prnfile)) {
		abort(lasterror());
	}

	// check file has been prepared
	if (not prnfile.osfile()) {
		var msg = "OUTPUT FILE FOR PRINTOUT CANNOT BE CREATED.";
		msg(-1) = "|" ^ (DQ ^ (oscwd() ^ prnfile ^ DQ)) ^ "|";
		call mssg(msg);
		stop();
	}

	// equ getflag to system<3>;*1=printing program has reset setptr to prn and done its own printing
	// called program can set it to 1 or 0 or leave as ''
	SYSTEM(3) = "";

	// put up a message
	if (not OPTIONS.contains("S"))
		printl("Making a new document/report " ^ SYSTEM.f(2) ^ ":");

	let timestarted = time();

	// execute the cmd to produce the report
	perform(cmd);

	printl("");

	// redirect printed output back to the printer
	// program may change the output file
	if (SYSTEM.f(2) != prnfile) {
		prnfile = SYSTEM.f(2);
		dfs		= prnfile;
	}

	SYSTEM(2)	= "";
	let filelen = prnfile.osfile().f(1);

	// fail if print file less than 2 characters long
	if (filelen < 2) {
nooutput:
		//dfs.osremove();
		if (dfs.osfile() and not dfs.osremove()) {
			abort(lasterror());
		}
		stop();
	}

	// fail if only spaces output
	if (not SYSTEM.f(3)) {
		var	 offset = 0;
		//call osbread(temp, prnfile, offset, 1024);
		if (not osbread(temp, prnfile, offset, 1024)) {
			abort(lasterror());
		}
		// convert \200d0a0c1a\ to '' in temp
		temp.converter(var("200D0A0C1A").iconv("HEX2"), "");
		if (temp == "") {
			goto nooutput;
		}
	}

	// copy print file to public documents if necessary
	// if prnfile != dfs then
	// call shell('COPY /b ':PRNFILE:' ':dfS)
	// osremove prnfile
	// end

	// convert text to html
	// if not(html) then
	// call convfiletype(dfs,dfs2,'HTM')
	// osremove dfs
	// dfs=dfs2
	// end

	stop();
	// for c++
	return 0;
}

libraryexit()
