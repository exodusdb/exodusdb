#include <exodus/library.h>
libraryinit()

var logfilename;
var logfilelength;//num
var temposfilename83;
var logfile;
var datax;
var time2;//num

function main(in msg0, io time0) {

	var interactive = false; //not(SYSTEM.f(33));
	if (interactive) {
		return 0;
	}

	logfile = "";

	if (msg0 eq "GETLASTLOG") {

		if (not(VOLUMES)) {
			return "";
		}

		// Try multiple start points in case hit middle of multibyte character
		// space to defeat convsyntax
		for ( var ptr = logfilelength - 1024; ptr <= logfilelength - 1021; ptr++) {
		//for (const var ptr : range(logfilelength - 1024, logfilelength - 1021)) {
		//for (const var ptr : range(logfilelength - 1024, logfilelength - 1021)) {
			call osbread(datax, logfile, ptr, 1024);
			//if any data then break out of loop
			if (datax.len()) {
				ptr = logfilelength;
			}
		} //ptr;

		logfile.osclose();
		datax.converter("\r\n", FM ^ FM);
		time0 = logfilename ^ " " ^ field2(datax, FM, -3, 3);
		return 0;

	}

	time2 = ostime();
	if (time0.unassigned()) {
		time0 = time2;
	}

	var entry = "";
	if (TERMINAL) {
		//entry:=oconv(date(),'D2-J')
		entry = date().oconv("D2-E");
		entry = entry.last(2) ^ "-" ^ entry.first(5);
		entry ^= " " ^ time2.oconv("MTS") ^ " ";
		//entry:=field(time2,'.',2) 'MD20P'
	}
	//similar in listen and log2
	entry ^= SYSTEM.f(24) ^ ": " ^ (time2 - time0).oconv("MD20P");
	//entry:=sep:sep:sep:sep
	entry ^= " " ^ msg0;

	if (TERMINAL) {
		printl(entry);
	}

	entry ^= "\r\n";

	time0 = time2;

	if (logfile) {
		call osbwrite(entry, logfile, logfilelength);
		logfile.osclose();
	}

	return 0;
}

libraryexit()
