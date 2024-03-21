#include <exodus/library.h>
libraryinit()

#include <convcsv.h>
#include <convertercsv.h>
#include <uconvfile.h>
#include <xselect.h>

#include <service_common.h>

#include <srv_common.h>

var sentencex;
var selectx;
var nfilters0;
//var nfilters;  // num
//var filterfields;
//var filtervalues;
dim filters;
var temp;  // num
var ptr;   // num
var tt;
var dicthasauthorised;	// num
var keyx;
var nkeys;	// num
dim xfilenames;
dim oconvxs;
dim fmtxs;
dim dictrecs;
dim xfiles;
var v69;
var v70;
var v71;
var dict;
dim dictids;
dim colgroups;
var dictid;
var outfile;
dim rec;
dim mvrec;
var converterparams;
var result;
var errors;

function main(in sentence0, in select0 = "", in filters0 = "") {

	// called from
	// BP ANALTIME    call with filters
	// BP AUDITINVS   perform then self call
	// BP EXPORTADS   ditto
	// BP MEDIADIARY2 ditto
	// ABP JOURNALS   ditto

	if (sentence0.unassigned()) {
		sentencex = "";
	} else {
		sentencex = sentence0;
	}
	if (select0.unassigned()) {
		selectx = "";
	} else {
		selectx = select0;
	}

	let nfilters = nfilters0.default_from("").fcount(FM);
	//let nfilters = filters0.f(1).fcount(VM);
	if (nfilters) {
		let filterfields = filters0.f(1);
		let filtervalues = filters0.f(3);
		filters.redim(3, nfilters);
		for (const var filtern : range(1, nfilters)) {
			filters(1, filtern) = filterfields.f(1, filtern).convert(SM, VM);
			filters(3, filtern) = filtervalues.f(1, filtern).convert(SM, VM);
		}  // filtern;
	}

	// convert command line to subroutine call
	if (SENTENCE.field(" ", 1) == "CONVCSV" and sentence0.unassigned()) {

		SENTENCE.move(sentencex);

		// locate and remove SELECT statement from end of command
		if (sentencex.locateusing(" ", "SELECT", temp)) {
			selectx	  = sentencex.field(" ", temp + 1, 9999);
			sentencex = sentencex.field(" ", 1, temp - 1);
		} else {
			selectx = "";
		}

		call convcsv(sentencex, selectx);
		return 0;
	}

	ptr = 0;

	let filename = sentencex.field(" ", 2);
	var file;
	if (not file.open(filename, "")) {
		call mssg(lasterror());
		return 0;
	}

	let normalise = sentencex.contains(" NORMALISE");
	if (normalise) {
		sentencex.replacer(" NORMALISE", "");
	}

	let firstmvonly = sentencex.contains(" FIRSTMVONLY");
	if (firstmvonly) {
		sentencex.replacer(" FIRSTMVONLY", "");
	}

	let mv1only = sentencex.contains(" MV1ONLY");
	if (mv1only) {
		sentencex.replacer(" MV1ONLY", "");
	}

	let raw = sentencex.contains(" RAW");
	if (raw) {
		sentencex.replacer(" RAW", "");
	}
	let mvgroupno = "";

	let colheaderrow = not(sentencex.contains(" NOCOLHEADER"));
	if (not colheaderrow) {
		sentencex.replacer(" NOCOLHEADER", "");
	}

	if (filename.starts("DICT")) {
		tt = "VOC";
	} else {
		tt = filename;
	}
	if (not DICT.open("DICT." ^ tt)) {
		call mssg(lasterror());
		return 0;
	}

	// eg ABP converter.maconomy
	convertercsv = "";
	var hasconverter;
	if (not hasconverter.read(DEFINITIONS, "CONVERTER*" ^ filename)) {
		hasconverter = "";
	}
	if (hasconverter.f(1)) {
		tt = "CONVERTER." ^ hasconverter.f(1);
		// c++ variation
		if (not VOLUMES) {
			tt.lcaser();
			tt.converter(".", "");
		}
		convertercsv = tt;
	}

	if (var().read(DICT, "AUTHORISED")) {
		dicthasauthorised = 1;
	} else {
		dicthasauthorised = 0;
	}

	var notexportable = "";
	var exportable	  = sentencex.field(" ", 3, 9999);
	if (exportable) {
		exportable.converter(" ", FM);
		if (exportable.f(1) == "EXCEPT") {
			notexportable = exportable.field(FM, 2, 9999);
			exportable	  = "";
		}
	}

	// expand any group fields in notexportable
	if (notexportable) {
		for (const var ii : range(1, notexportable.fcount(FM))) {
			var dictrec;
			if (dictrec.read(DICT, notexportable.f(ii))) {
				if (dictrec.f(1) == "G") {
					temp = dictrec.f(3);
					temp.converter(VM ^ " ", FM ^ FM);
					notexportable(ii) = temp;
				}
			}
		}  // ii;
	}

	let listkey = var(1000000).rnd();

	if (not exportable) {

		if (exportable.read(DICT, "EXPORTABLE")) {
			if (exportable.f(1) == "G") {
				exportable = exportable.f(3);
				exportable.converter(VM ^ " ", FM ^ FM);
			}
			// Find first blank field
			if (var pos = exportable.index(FM ^ FM);pos) {
				keyx  = exportable.first(pos - 1);
				nkeys = keyx.fcount(FM);
				if (nkeys > 2) {
					// call msg('Key field(s) should be followed by a blank line or space in EXPORTABLE')
					// stop
					nkeys = 0;
				}
			}
		} else {
			exportable = "";
			keyx	   = "";
		}
	}

	var exportable2 = exportable;

	// nextmvgroup:

	if (mvgroupno) {
		tt = keyx ^ FM ^ "LINE_NO" ^ FM;
	} else {
		tt = "";
	}

	exportable	= tt ^ exportable2.field("%", 1);
	exportable2 = exportable2.field("%", 2, 9999);

	var outfilename = SYSTEM.f(2);
	// zzz if mvgroupno then outfilename[8,1]=mvgroupno
	if (outfilename.lcase().ends(".htm")) {
		outfilename.paster(-3, 3, "xls");
		SYSTEM(2) = outfilename;
	}
	let excel = outfilename.lcase().ends("xls");

	//outfilename.osremove();
	if (outfilename.osfile() and not outfilename.osremove())
		abort(lasterror());
	if (outfilename.osfile()) {
		var	  msg = "CANNOT EXPORT BECAUSE " ^ outfilename ^ " IS ALREADY|OPEN IN ANOTHER PROGRAM, OR CANNOT BE ACCESSED";
		call  mssg(msg);
		gosub exit2();
		return 0;
	}

	xfilenames.redim(255);
	xfilenames = "";  //dim
	oconvxs.redim(255);
	fmtxs.redim(255);
	dictrecs.redim(255);
	oconvxs = "";  //dim
	fmtxs	= "";  //dim
	xfiles.redim(255);
	var nfields = 0;

	let selectlist = LISTACTIVE;
	if (selectlist) {
		call pushselect(0, v69, v70, v71);
	}

	if (exportable) {
		makelist("", exportable);
		// write exportable on lists,listkey
		// perform 'GET-LIST ':listkey:' (S)'
		// delete lists,listkey

	} else {
		if (filename.starts("DICT")) {
			dict = "";
		} else {
			dict = "DICT ";
		}
		perform("SELECT " ^ dict ^ filename ^ " BY FMC WITH FMC BETWEEN 1 AND 9999 AND WITH @ID NOT STARTING '%' AND WITH MASTER.FLAG (S)");
		if (not LISTACTIVE) {
			gosub exit2();
			return 0;
		}
	}

	dictids.redim(255);
	colgroups.redim(255);

	dictids		 = "";	// dim
	colgroups	 = "";	// dim
	var headingx = "";
	var colnx	 = 0;

nextdict:
	if (readnext(dictid, MV)) {

		if (notexportable.locateusing(FM, dictid)) {
			goto nextdict;
		}

		if (dictid.starts("%")) {
			goto nextdict;
		}

		if (dictid == "LINE_NO") {
			colnx += 1;
			dictids(colnx)  = dictid;
			headingx(colnx) = "Line No.";
			fmtxs(colnx)	   = "R";
			dictrecs(colnx) = "";
		} else {
			if (dict.read(DICT, dictid)) {
				// TODO implement JBASE/PICK dict types I/A/D
				// call dicti2a(dict)
				colnx += 1;
				// if dict<2> matches '0N' then
				let fn = dict.f(2);
				if (fn > nfields) {
					nfields = fn;
				}

				fmtxs(colnx) = dict.f(9)[1];

				if (raw) {
					headingx(colnx) = dictid;
				} else {

					// extract title
					var title = dict.f(3).trim();
					title.replacer("<WBR/>", "");
					title.replacer("<wbr/>", " ");
					// title.converter("ABCDEFGHIJKLMNOPQRSTUVWXYZ" "|_" _VM, "abcdefghijklmnopqrstuvwxyz" "   ");
					title.lcaser();
					title.converter("|_" _VM, "   ");

					// t=title[1,1]
					// convert @lower.case to @upper.case in t
					// title[1,1]=t
					title = capitalise(title);

					title.replacer("(Base)", "(" ^ srv.company.f(3) ^ ")");

					// swap ' code' with '' in title

					headingx(colnx) = title.trim();
				}

				// extract file
				if (dict.f(11).starts("<")) {
					temp			 = dict.f(11).cut(1).field(">", 1);
					xfilenames(colnx) = temp;
					if (not xfiles(colnx).open(temp, "")) {
						call  mssg(temp.quote() ^ " file cannot be found in dict " ^ (dictid.quote()));
						gosub exit2();
						return 0;
					}
					var title = headingx.f(colnx);
					if (title.ucase().ends(" CODE")) {
						title.cutter(-5);
						headingx(colnx) = title;
					}
				}

				// extract conversion
				if (dict.f(7)) {
					var oconvx = dict.f(7);

					// force long date format
					if (oconvx.contains("DATE") or (oconvx.starts("D"))) {
						// if raw then
						// oconvx='D4/J'
						// end else
						if (oconvx == "[SCH.DATES]") {
							oconvx = "";
						} else {
							oconvx = DATEFMT;
							oconvx.converter("2", "4");
						}
						// end

						// no commas to be added to numbers
					} else if (oconvx.starts("[NUMBER")) {
						oconvx = "";
					}

					oconvxs(colnx) = oconvx;
				}

				colgroups(colnx) = dict.f(4).starts("M");
				dictids(colnx)	= dictid;
				dictrecs(colnx)	= dict;
			}
			// end
		}
		goto nextdict;
	}
	let ncols = colnx;

	// if @username='EXODUS' then oswrite matunparse(dictids) on 'csv'

	if (selectlist) {
		call popselect(0, v69, v70, v71);
	}

	//call oswrite("", outfilename);
	if (not oswrite("", outfilename))
		abort(lasterror());
	if (not outfile.osopen(outfilename)) {
		call  mssg(lasterror());
		gosub exit2();
		return 0;
	}
	// ptr=0

	// suppress headerrow if not required
	if (not colheaderrow) {
		headingx = "";
	}

	if (selectx) {
		// selectx:=' AND WITH PERSON_CODE "HARRIS"'
		// perform 'SELECT ':filename:' ':selectx
		tt = "SELECT " ^ filename ^ " " ^ selectx;
		call xselect(tt);
		if (not LISTACTIVE) {
			outfile.osclose();
			//outfilename.osremove();
			if (outfilename.osfile() and not outfilename.osremove())
				abort(lasterror());
			call mssg("No records found");
			return 0;
		}
	} else {
		if (not LISTACTIVE) {
			select(file);
		}
	}
	var recn = 0;

	rec.redim(ncols);
	mvrec.redim(ncols);

	// oswrite matunparse(dictids) on 'DICTIDS'

////////
nextrec:
	// //////

	if (esctoexit()) {
		gosub exit();
		return 0;
	}

	// get the next key
	var mvx = 0;
	if (not readnext(ID, mvx)) {
		gosub exit();
		return 0;
	}

	if (ID == "") {
		goto nextrec;
	}
	recn += 1;

	if (TERMINAL)
		output(AT(-40), recn, ". ");

	// get the record
	if (not RECORD.read(file, ID)) {
		goto nextrec;
	}

	if (mv1only) {
		mvx = 1;
	}

	MV = mvx;

	if (dicthasauthorised) {
		if (not calculate("AUTHORISED")) {
			goto nextrec;
		}
	}

	// find the maximum multivalue
	var maxvn = 1;

	// skip multivalues
	for (const var filtern : range(1, nfilters)) {
		let value = calculate(filters(1, filtern));
		if (filters(3, filtern).len()) {

			// if reqvalue then skip if not matching
			if (not filters(3, filtern).locate(value)) {
				goto nextrec;
			}

			// no reqvalues means skip if no value present
		} else if (not value) {
			goto nextrec;
		}

	}  // filtern;

	for (const var coln : range(1, ncols)) {
		MV	   = mvx;
		dictid = dictids(coln);
		temp   = "";
		if (dictid != "LINE_NO" and dictrecs(coln).f(4) != "S") {
			temp = calculate(dictid).fcount(VM);
			if (temp > maxvn) {
				maxvn = temp;
			}
		}
	}  // coln;
	// d ebug
	// get the data
	rec			= "";  //dim
	var anydata = 0;
	for (const var coln : range(1, ncols)) {
		MV	   = mvx;
		dictid = dictids(coln);
		if (dictid == "LINE_NO") {
		} else {
			let cell = calculate(dictid);
			if (cell != "") {
				rec(coln) = cell;
				anydata	  = 1;
			}
		}
	}  // coln;

	// normalise the data and output to csv file
	// if rec<>'' then
	if (anydata) {

		// mat mvrec=mat rec
		mvrec = rec;  //dim

		var vn = 0;
nextvn:
		vn += 1;

		if (esctoexit()) {
			outfile.osclose();
			gosub exit2();
			return 0;
		}

		// conversions
		for (const var coln : range(1, ncols)) {

			// choose the right mv
			if (dictids(coln) == "LINE_NO") {
				rec(coln) = vn;
			} else {
				mvx = colgroups(coln);
				if (mvx) {
					rec(coln) = mvrec(coln).f(1, vn);
				}
			}

			var cell = rec(coln);

			if (cell != "") {

				if (mvx or vn == 1) {

					// convert codes to names
					if (xfilenames(coln) and not(raw)) {
						var rec2;
						if (rec2.read(xfiles(coln), cell)) {
							if (xfilenames(coln) == "BRANDS") {
								cell = rec2.f(2, 1);
							} else {
								cell = rec2.f(1);
							}
						}
					}

					// other conversions
					if (oconvxs(coln)) {
						cell = oconv(cell, oconvxs(coln));
					}
				}

				if (cell.starts("+")) {
					if (cell.cut(1).isnum()) {
						cell.cutter(1);
					}
				}
				if (cell == DQ) {
					cell = "''";
				}
				if (not cell.starts(DQ) or not cell.ends(DQ)) {
					cell.replacer(DQ, "''");
				}
				if (cell.len() > 255) {
					cell.firster(200);
					cell ^= " ...";
				}
				if (fmtxs(coln) != "R" or not(cell.isnum())) {

					// make sure "1-12" is not interpreted as a formula
					if (var(1) or excel) {
						if (var(".-+0123456789").contains(cell[1])) {
							if (not cell.isnum()) {
								cell.prefixer(" ");
							}
						}
					}

					if (cell.contains(DQ)) {
						if (fmtxs(coln) == "T") {
							if (not cell.starts(DQ) or not cell.ends(DQ)) {
								cell.replacer(DQ, "''");
								cell.quoter();
							}
						}
					} else {
						cell.quoter();
					}
				}
				// gotcell:
				rec(coln) = cell;
			}

		}  // coln;

		var line = rec.join();

		// remove trailing or all tab chars
		// 		while (true) {
		// 			// /BREAK;
		// 			if (not line.ends(FM)) break;
		// 			line.popper();
		// 		}// loop;
		line.trimmerlast(FM);

		// suppress output of empty amv rows
		if (mvgroupno and nkeys) {
			if (line.field(FM, nkeys + 2, 9999) == "") {
				line = "";
			}
		}

		// skip zero hours in timesheets
		// already skipped above probably
		// if filename='TIMESHEETS' then if @record<2,vn> else line=''

		// remove leading equal signs in order not to confuse Excel
		line.replacer(FM ^ "=", FM);

		// output one line
		if (line != "") {

			// output header row if first line and not suppressed

			if (headingx) {

				if (hasconverter) {
					// headingx will come back converted and maybe as multiple lines
					// converterparams initially contains first line so heading can put some columns into heading if required
					// converterparams comes back with info to speed convertion of lines
					converterparams = line;
					call convertercsv("HEAD", headingx, converterparams, filename);
				} else {
					headingx.converter(_FM, chr(9));
					headingx ^= _EOL;
				}

				//call osbwrite(headingx, outfile, ptr);
				if (not osbwrite(headingx, outfile, ptr))
					abort(lasterror());

				headingx = "";
			}

			// output line

			if (hasconverter) {
				call convertercsv("LINE", line, converterparams, filename);
			} else {
				line.replacer(_FM, chr(9));
				line ^= _EOL;
			}

			//call osbwrite(line, outfile, ptr);
			if (not osbwrite(line, outfile, ptr))
				abort(lasterror());
		}

		if (not(firstmvonly) and vn < maxvn) {
			goto nextvn;
		}
	}

	goto nextrec;
}

subroutine exit() {
	outfile.osclose();
	// will not do anything unless uconv is available
	// call uconvfile(outfile,'CODEPAGE','UTF16',result,errors)
	// we need to convert to UTF-8 since csv files have no way to specify
	// what codepage the charset is and this can result in failure to open correctly
	call uconvfile(outfile, "CODEPAGE", "UTF8", result, errors);
	// ignore errors like "uconv cannot be found" for now
	// otherwise will have to install cygwin uconv everywhere and add to installations
	// call msg(errors)
	SYSTEM(34) = 1;
	gosub exit3();
	return;
}

subroutine exit2() {
	// general result code
	SYSTEM(34) = 0;
	gosub exit3();
	return;
}

subroutine exit3() {
	// TODO reimplement multiple mv group export
	// if raw and exportable2 then
	// mvgroupno+=1
	// if mvgroupno=1 then mvgroupno=2
	// goto nextmvgroup
	// end

	if (not ptr) {
		call mssg("No records found");
		stop();
	}

	return;
}

libraryexit()
