#include <exodus/library.h>
libraryinit()


#include <gen.h>

var docrep;
var scrn;
var temp;

function main() {
	//
	//c xxx

	// a much simpler non-interactive version of GET

	var where = "";
	var html = 1;
	var cmd = SENTENCE;

	while (cmd) {
		var word1=field(cmd," ",1);
		if (word1 eq "GET" or word1 eq "NEW")
			cmd = field(cmd," ",2,99999).trimb();
		else
			break;
	}

	/////
	//MAKE:
	/////

	//make up a dos print file name
	var dfs = where ^ var(10).pwr(15).rnd().substr(1,8);
	if (html) {
		dfs ^= ".htm";
	}else{
		dfs ^= ".txt";
		}
	var prnfile = dfs;
	SYSTEM.r(2, prnfile);

	//redirect printed output to a dos file
	call oswrite("", prnfile);

	//check file has been prepared
	if (not(prnfile.osfile())) {
		var msg = "OUTPUT FILE FOR PRINTOUT CANNOT BE CREATED.";
		msg.r(-1, "|" ^ (DQ ^ (oscwd() ^ prnfile ^ DQ)) ^ "|");
		call mssg(msg);
		var().stop();
	}

	//equ getflag to system<3>;*1=printing program has reset setptr to prn and done its own printing
	//called program can set it to 1 or 0 or leave as ''
	SYSTEM.r(3, "");

	//put up a message
	if (not OPTIONS.index("S"))
		print("Making a new document/report " ^ SYSTEM.a(2) ^ ":");

	var timestarted = var().time();

	//execute the cmd to produce the report
	perform(cmd);

	printl("");

	//redirect printed output back to the printer
	//program may change the output file
	if (SYSTEM.a(2) ne prnfile) {
		prnfile = SYSTEM.a(2);
		dfs = prnfile;
	}

	SYSTEM.r(2, "");
	var filelen = prnfile.osfile().a(1);

	//fail if print file less than 2 characters long
	if (filelen < 2) {
nooutput:
		dfs.osdelete();
		var().stop();
	}

	//fail if only spaces output
	if (not SYSTEM.a(3)) {
		var offset = 0;
		call osbread(temp, prnfile,  offset, 1024);
		//convert \200d0a0c1a\ to '' in temp
		temp.converter(var("200D0A0C1A").iconv("HEX"), "");
		if (temp == "") {
			goto nooutput;
		}
	}

	//copy print file to public documents if necessary
	//if prnfile ne dfs then
	// call shell('COPY /b ':PRNFILE:' ':dfS)
	// osdelete prnfile
	// end

	//convert text to html
	//if not(html) then
	// call convfiletype(dfs,dfs2,'HTM')
	// osdelete dfs
	// dfs=dfs2
	// end

	var().stop();
	//for c++
	return 0;

}


libraryexit()
