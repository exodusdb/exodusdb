#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <convpdf.h>
#include <sysmsg.h>

#include <service_common.h>


//var outfile;
var errors;

func main(in module, in mode, in stationery) {

	// checks there is some output and converts htm to pdf if necessary

	// given stationery
	//
	//   1 Preprinted letterhead
	//   2 Blank paper/PDF/Email
	//   P PDF - Portrait
	//   L PDF - Landscape

	// perform
	//
	// 0. convert the file if necessary
	// 1. filename(s) in system<2> may be converted from .htm to .pdf
	// 2. put converted filename(s) into DATA - to web ui with ../../data/
	// 3. error response set if file(s) do not exist or < 5 bytes

	var outfiles = SYSTEM.f(2);
	outfiles.converter(";", VM);
	data_	   = outfiles;
	let nfiles = outfiles.fcount(VM);
	for (const var filen : range(1, nfiles)) {

		var outfile = outfiles.f(1, filen);

		gosub check_exists_and_maybe_convert(module, mode, stationery, outfile);

		// save back into list in case htm file converted to pdf and changed name
		outfiles(1, filen) = outfile;

		// save the web ui "data" in ../../data/ format
		// TODO web gui to be smarter in accepting files like */data/xxxx/yyyy.htm
		let t2 = OSSLASH;
		let tt = outfile.index(t2 ^ "data" ^ t2);
		if (tt) {
			data_(1, filen) = ".." ^ t2 ^ ".." ^ outfile.cut(tt - 1);
		} else {
			data_(1, filen) = outfile;
		}

	}  // filen;

	data_.converter(VM, ";");
	// convert vm to ';' in outfiles
	SYSTEM(2) = outfiles;

	// if returning many files then always return ok, with any messages as warnings
	if (nfiles > 1) {
		response_ = "OK";
		if (msg_) {
			response_ ^= " " ^ msg_;
		}
		msg_ = "";
	}

	return 0;
}

subr check_exists_and_maybe_convert(in module, in mode, in stationery, io outfile) {

	let diroutfile = outfile.osfile();
	if (diroutfile.f(1) > 5) {
fileok:

		// convert to pdf
		if (stationery > 2) {
			call convpdf(outfile, stationery, errors);
			if (errors) {
				msg_(-1) = errors;
			}
		}

		response_ = "OK";
		if (msg_) {
			response_ ^= " " ^ msg_;
		}
		msg_ = "";

	} else {

		// check for long filenames too (DOS limit was 8.3)
		if (not diroutfile) {
			if (oslistf(outfile)) {
				goto fileok;
			}
		}

		response_ = msg_;
		if (response_ == "") {
			response_ = "Error: No output file in " ^ module ^ "PROXY " ^ mode;
			call sysmsg(response_);
		}

		// force error
//		if (response_.first(6) != "Error:") {
		if (not response_.starts("Error:")) {
			response_.prefixer("Error:");
		}
	}

	return;
}

libraryexit()
