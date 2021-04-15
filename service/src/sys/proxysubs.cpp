#include <exodus/library.h>
libraryinit()

#include <convpdf.h>
#include <sysmsg.h>

#include <gen_common.h>

var outfile;
var errors;

function main(in module, in mode, in stationery) {
	//c sys

	//checks there is some output and converts htm to pdf if necessary

	//given stationery
	//
	//   1 Preprinted letterhead
	//   2 Blank paper/PDF/Email
	//   P PDF - Portrait
	//   L PDF - Landscape

	//perform
	//
	//0. convert the file if necessary
	//1. filename(s) in system<2> may be converted from .htm to .pdf
	//2. put converted filename(s) into DATA - to web ui with ../../data/
	//3. error response set if file(s) do not exist or < 5 bytes

	#include <general_common.h>

	#define request USER0
	#define data USER1
	#define response USER3
	#define msg USER4

	var outfiles = SYSTEM.a(2);
	outfiles.converter(";", VM);
	USER1 = outfiles;
	var nfiles = outfiles.count(VM) + (outfiles ne "");
	for (var filen = 1; filen <= nfiles; ++filen) {

		outfile = outfiles.a(1, filen);

		gosub check_exists_and_maybe_convert(module, mode, stationery, outfile);

		//save back into list in case htm file converted to pdf and changed name
		outfiles.r(1, filen, outfile);

		//save the web ui "data" in ../../data/ format
		//TODO web gui to be smarter in accepting files like */data/xxxx/yyyy.htm
		var t2 = OSSLASH;
		var tt = outfile.index(t2 ^ "data" ^ t2);
		if (tt) {
			USER1.r(1, filen, ".." ^ t2 ^ ".." ^ outfile.substr(tt, 999999));
		} else {
			USER1.r(1, filen, outfile);
		}

	} //filen;

	USER1.converter(VM, ";");
	//convert vm to ';' in outfiles
	SYSTEM.r(2, outfiles);

	//if returning many files then always return ok, with any messages as warnings
	if (nfiles gt 1) {
		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}

	return 0;
}

subroutine check_exists_and_maybe_convert(in module, in mode, in stationery, io outfile) {

	var diroutfile = outfile.osfile();
	if (diroutfile.a(1) gt 5) {
fileok:

		//convert to pdf
		if (stationery gt 2) {
			call convpdf(outfile, stationery, errors);
			if (errors) {
				USER4.r(-1, errors);
			}
		}

		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";

	} else {

		//check for long filenames too (DOS limit was 8.3)
		if (not(diroutfile)) {
			if (oslistf(outfile)) {
				goto fileok;
			}
		}

		USER3 = USER4;
		if (USER3 eq "") {
			USER3 = "Error: No output file in " ^ module ^ "PROXY " ^ mode;
			call sysmsg(USER3);
		}

		//force error
		if (USER3.substr(1, 6) ne "Error:") {
			USER3.splicer(1, 0, "Error:");
		}

	}

	return;
}

libraryexit()
