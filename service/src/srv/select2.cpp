#include <exodus/library.h>
libraryinit()

#include <listen3.h>
#include <openfile.h>
#include <safeselect.h>
#include <systemsubs.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

var dictids;
var options;
var maxnrecs;
//var nlimitfields; // num
dim hexx;
var useactivelist;	// num
var v69;
var v70;
var v71;
var storer;
var storeid;
var storedict;
var storemv;
var commonsaved;  // num
var savesrcfile;
var savedatafile;
var savewlocked;
var savemsg;
var savereset;
var savevalid;
var dictfilename;
var linkfile2;
var dictvoc;
var haspostread;
var realfilename;
var triggers;
var postreadmode;
var ndictids;
var xx;
var row;

function main(in filenamex, in linkfilename2, in sortselect0, in dictids0, in options0, io datax, io response, in limitfields = "", in limitchecks = "", in limitvalues = "", in maxnrecs0 = 0) {

	// NB add %SELECTLIST% to sortselect to use active select list
	// or provide an FM terminated list of keys

	// given a sort/select statement
	// returns a dynamic array or file of xml data

	// use app specific version of listen3
	if (APPLICATION != "EXODUS") {
		listen3 = "listen3_app";
	}

	var filename = filenamex;

	var sortselect = sortselect0;
	if (dictids0.unassigned()) {
		dictids = "";
	} else {
		dictids = dictids0;
	}
	if (options0.unassigned()) {
		options = "";
	} else {
		options = options0;
	}
	if (maxnrecs0.unassigned()) {
		maxnrecs = "";
	} else {
		maxnrecs = maxnrecs0;
	}
	// 	if (limitfields.unassigned()) {
	// 		nlimitfields = 0;
	// 	} else {
	// 		nlimitfields = limitfields.fcount(VM);
	// 	}
	let nlimitfields = limitfields.unassigned("").fcount(VM);

	let xml		= options.contains("XML");
	let rawread = options.contains("RAW");

	datax = "";

	if (linkfilename2) {
		hexx.redim(8);
		// changed to allow language characters to pass through x80-xF9
		for (const var ii : range(249, 255)) {
			hexx(ii - 249) = "%" ^ ii.oconv("MX").oconv("R(0)#2");
		}  // ii;
	}

	useactivelist = sortselect.contains("%SELECTLIST%");
	sortselect.replacer("%SELECTLIST%", "");
	if (not LISTACTIVE) {
		useactivelist = 0;
	}

	// fm termination indicates list of keys and optionally mvnos
	let givenkeys = sortselect.ends(FM);
	var givenkeyn = 0;

	if (not useactivelist) {
		call pushselect(0, v69, v70, v71);
	}

	RECORD.move(storer);
	ID.move(storeid);
	DICT.move(storedict);
	MV.move(storemv);

	// detect if called without common
	// NB cant run without common in caller since perform gives some RTP20 error
	if (rawread) {
		goto nocommon;
	}
	if (req.srcfile.unassigned()) {
nocommon:
		commonsaved = 0;
	} else {
		commonsaved = 1;
		req.srcfile.move(savesrcfile);
		req.datafile.move(savedatafile);
		req.wlocked.move(savewlocked);
		msg_.move(savemsg);
		req.reset.move(savereset);
		req.valid.move(savevalid);
	}

	// if unassigned(limitfields) then limitfields=''

	// filename can be 'filename USING dictfilename'

	let filename0 = filename;
	if (filename.field(" ", 2) == "USING") {
		dictfilename = filename.field(" ", 3);
		filename	 = filename.field(" ", 1);
	} else {
		dictfilename = filename;
	}

	response = "OK";
	var file;
	if (not file.open(filename, "")) {
		response = "Error: select2: " ^ (filename.quote()) ^ " file is not available";

		// abort
		gosub exit();
		return 0;
	}

	if (linkfilename2) {
		//call oswrite("", linkfilename2);
		if (not oswrite("", linkfilename2)) {
			//abort(lasterror());
			response = "Error: select2: Cannot write to output file" ^ (linkfilename2.quote());

			// abort
			gosub exit();
			return 0;
		}
		if (not linkfile2.osopen(linkfilename2)) {
			//response = "Error: select2: " ^ (linkfilename2.quote()) ^ " cannot open output file";
			response = "Error: select2: Cannot open output file" ^ (linkfilename2.quote());

			// abort
			gosub exit();
			return 0;
		}
	} else {
		datax = "";
	}
	var dataptr = 0;

	var cmd = "SELECT";
	if (maxnrecs) {
		cmd ^= " " ^ maxnrecs;
	}
	cmd ^= " " ^ filename0;
	// if trim(@station)='SBCP1800' then cmd='SELECT 10 ':filename

	// check no @ in xml dict ids because cannot return xml tag with @
	if (xml and dictids.contains("@")) {
		response = "Error: select2: XML dictids cannot contain @ characters";

		// abort
		gosub exit();
		return 0;
	}

	var oconvsx = "";
	if (dictids.unassigned()) {
		dictids = "";
	}
	if (dictids == "") {
		dictids = "ID";
	}
	dictids.trimmer();
	dictids.converter(" ", FM);
	var dictrecs = "";

	if (not DICT.open("DICT." ^ dictfilename)) {
		response = "Error: select2: " ^ (("DICT." ^ filename).quote()) ^ " file is not available";

		// abort
		gosub exit();
		return 0;
	}

	if (not openfile("DICT.voc", dictvoc)) {
		dictvoc = "";
	}

	if (rawread) {
		haspostread = "";
	} else {
		call listen3(filename, "READ", realfilename, triggers);
		// postread=triggers<3>
		haspostread	 = triggers.f(3) != "";
		systemsubs	 = triggers.f(3);
		postreadmode = triggers.f(4);
	}

	// check/get dict recs

	let ndictids = dictids.fcount(FM);
	if (dictids != "RECORD") {
		// 		while (true) {
		// 			// /BREAK;
		// 			if (not dictids.ends(FM)) break;
		// 			dictids.popper();
		// 		}// loop;
		dictids.trimmerlast(FM);
		if (dictids == "") {
			dictids = "ID";
		}
		for (const var dictidn : range(1, ndictids)) {
			let dictid = dictids.f(dictidn);
			var dictrec;
			if (not dictrec.read(DICT, dictid)) {
				if (not dictrec.read(dictvoc, dictid)) {
					if (dictid == "ID") {
						dictrec = "F,0,No,,,,,,L,15,";
						dictrec.converter(",", FM);
					} else {
						response = "Error: select2: " ^ (dictid.quote()) ^ " IS MISSING FROM DICT." ^ filename ^ " in SELECT2";

						// abort
						gosub exit();
						return 0;
					}
				}
			}

			// pick items
			// if index('DI',dictrec<1>,1) then call dicti2a(dictrec)

			// pick A is revelation F
			if (dictrec.f(1) == "A") {
				dictrec(1) = "F";
			}

			dictrec			  = lower(dictrec);
			dictrecs(dictidn) = dictrec;
			oconvsx(dictidn)  = dictrec.f(1, 7);
		}  // dictidn;
	}

	var tx = "";
	if (xml and linkfilename2) {
		tx ^=
			"<records>"
			"\n";
		//call osbwrite(tx, linkfile2, dataptr);
		if (not osbwrite(tx, linkfile2, dataptr)) {
			abort(lasterror());
		}
	}

	// read xx from @dict,'AUTHORISED' then
	// if index(sortselect,' WITH AUTHORISED',1) else
	//  if index(' ':sortselect,' WITH ',1) then sortselect:=' AND'
	//  sortselect:=' WITH AUTHORISED'
	//  end
	// end
	var chk_authorised;
	if (not chk_authorised.read(DICT, "AUTHORISED")) {
		chk_authorised = 0;
	}

	if (not givenkeys) {

		// zzz should for validity of select parameters first
		// otherwise in server mode it loops with a very long error message

		// perform 'SELECT XXXXXXX WITH BRAND_CODE "LU" BY YEAR_PERIOD AND WITH ID NE "[*I" BY ID'

		// if filename='JOBS' or filename='COMPANIES' then

		// do after readnext to avoid two stage sort/select in c++
		// read xx from @dict,'AUTHORISED' then
		// if index(sortselect,' WITH AUTHORISED',1) else
		//  if index(' ':sortselect,' WITH ',1) then sortselect:=' AND'
		//  sortselect:=' WITH AUTHORISED'
		//  end
		// end

		// if not sorted then try use %RECORDS% if present and <200 chars
		var records = "";
		// if @list.active or index(' ':sortselect,' BY ',1) or index(sortselect,'WITH AUTHORISED',1) else
		if (not(LISTACTIVE or ((" " ^ sortselect).contains(" BY ")))) {
			// only look in selected files otherwise c++ takes too long on some files
			if (var("COMPANIES,CURRENCIES,UNITS,LEDGERS,JOB_TYPES").locateusing(",", filenamex, xx)) {
				if (records.read(file, "%RECORDS%")) {
					if (records.len() < 200) {
						records.replacer(FM, "\" \"");
						sortselect.prefixer(records.quote() ^ " ");
					}
				}
			}
		}

		// if @list.active else call safeselect(cmd:' ':sortselect:' (S)')
		if (not(LISTACTIVE) or sortselect) {
			call safeselect(cmd ^ " " ^ sortselect ^ " (S)");
		}

		// handle invalid cmd
		// R18.1 is normal 'No records found' message
		if (msg_ and not(msg_.contains("R18.1"))) {
			if (msg_.field(" ", 1) == "W156") {
				msg_ = msg_.field(" ", 2).quote() ^ " is not in the dictionary.||" ^ cmd ^ " " ^ sortselect;
			}
			response = msg_;

			// abort
			gosub exit();
			return 0;
		}
	}

	// return empty results even if no records selected
	let selectresult = "";

	var recn = "";

	if (not rawread) {
		req.datafile = filename;
		req.srcfile	 = file;
	}

	// read each record and add the required columns to the selectresult

////////
nextrec:
	// //////
	// if recn then if recn>=maxnrecs then goto nomore

	if (givenkeys) {

		givenkeyn += 1;
		ID = sortselect.f(givenkeyn);
		MV = ID.f(1, 2);
		if (MV.len()) {
			ID = ID.f(1, 1);
		}

	} else {
		if (not readnext(ID, MV)) {
			ID = "";
		}
	}

	// if no more
	if (ID == "") {

		if (xml and linkfilename2) {
			var	 tt = "</records>";
			//call osbwrite(tt, linkfile2, dataptr);
			if (not osbwrite(tt, linkfile2, dataptr)) {
				abort(lasterror());
			}
		}

		if (linkfilename2) {
			linkfile2.osclose();
			var().osflush();
		}

		// finished
		gosub exit();
		return 0;
	}

	if (ID.starts("%")) {
		goto nextrec;
	}
	if (not RECORD.read(file, ID)) {
		goto nextrec;
	}

	// filter out unauthorised
	if (chk_authorised) {
		if (not calculate("AUTHORISED")) {
			goto nextrec;
		}
	}

	// filter out unwanted multivalues that the stupid rev sortselect leaves in
	for (const var limitfieldn : range(1, nlimitfields)) {
		let value	   = calculate(limitfields.f(1, limitfieldn));
		let reqvalue   = limitvalues.f(1, limitfieldn);
		let limitcheck = limitchecks.f(1, limitfieldn);

		if (limitcheck == "EQ") {
			if (value != reqvalue) {
				goto nextrec;
			}

		} else if (limitcheck == "NE") {
			if (value == reqvalue) {
				goto nextrec;
			}

		} else {
			call mssg(limitcheck.quote() ^ " invalid limitcheck in select2");

			// abort
			gosub exit();
			stop();
			return 0;
		}
	}  // limitfieldn;

	recn += 1;

	if (dictids == "RECORD") {

		// postread (something similar also in listen/READ)
		if (haspostread) {

			// simulate window environment for POSTREAD
			req.orec = RECORD;
			// wlocked=1
			// simulate unlocked read to avoid warning messages like job cannot be updated
			req.wlocked = 0;
			msg_		= "";
			req.reset	= 0;

			call systemsubs(postreadmode);
			DATA = "";

			// call trimexcessmarks(iodat)

			// postread can request abort by setting msg or reset>=5
			if (req.reset >= 5 or msg_) {
				goto nextrec;
			}
		}

		// prevent reading passwords postread and postwrite
		if (filename == "DEFINITIONS" and ID == "SECURITY") {
			RECORD(4) = "";
		}

		// /

		RECORD.move(row);

		var prefix = ID ^ FM;

		if (dataptr) {
			prefix.prefixer(RM);
		}
		row.prefixer(prefix);

		// dictids != RECORD
	} else {
		row = "";

		for (const var dictidn : range(1, ndictids)) {
			let dictid	= dictids.f(dictidn);
			var dictid2 = dictid;
			dictid2.converter("@", "");
			var cell = calculate(dictid);
			if (oconvsx.f(dictidn)) {
				cell = oconv(cell, oconvsx.f(dictidn));
			}
			if (xml) {
				// cell='X'
				// convert "'":'".+/,()&%:-1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz' to '' in cell
				cell.replacer("%", "%25");
				cell.replacer("&", "&amp;");
				cell.replacer("<", "&lt;");
				cell.replacer(">", "&gt;");
				// if cell then deb ug
				// cell=quote(str(cell,10))
				row ^= "<" ^ dictid2 ^ ">" ^ cell ^ "</" ^ dictid2 ^
					   ">"
					   "\n";
			} else {
				row(1, dictidn) = cell;
			}
		}  // dictidn;

		if (xml) {
			row =
				"<RECORD>"
				"\n" ^
				row ^
				"</RECORD>"
				"\n";
			// move up
			// swap '&' with '&amp;' in row
			// swap "'" with "" in row
		}
		if (dataptr) {
			if (not xml) {
				row.prefixer(FM);
			}
		} else {
			if (xml) {
				row.prefixer(
					"<records>"
					"\n");
			}
		}
	}

	if (linkfilename2) {

		// in LISTEN and SELECT2 for direct output

		var blockn = 0;
		while (true) {
			var rowpart = row.b(blockn * 16384 + 1, 16384);

			if (not rowpart)
				break;

			rowpart.replacer("%", "%25");
			// changed to allow language characters to pass through x80-xF9
			for (const var ii : range(249, 255)) {
				rowpart.replacer(chr(ii), hexx(ii - 249));
			}  // ii;

			// output converted row part
			//call osbwrite(rowpart, linkfile2, dataptr);
			if (not osbwrite(rowpart, linkfile2, dataptr)) {
				abort(lasterror());
			}

			blockn += 1;
		}  // loop;

	} else {
		datax ^= row;
		dataptr += row.len();
	}

	// get next if output to file or space for more data
	// goto nextrec
	// if xml or len(DATAX)<64000 then goto nextrec
	if (xml or (datax.len() < maxstrsize_ - 1530)) {
		goto nextrec;
	}

	gosub exit();
	return 0;
}

subroutine exit() {
	if (commonsaved) {
		savesrcfile.move(req.srcfile);
		savedatafile.move(req.datafile);
		savewlocked.move(req.wlocked);
		savemsg.move(msg_);
		savereset.move(req.reset);
		savevalid.move(req.valid);
	}

	storer.move(RECORD);
	storeid.move(ID);
	storedict.move(DICT);
	storemv.move(MV);

	if (not useactivelist) {
		call popselect(0, v69, v70, v71);
	}

	return;
}

libraryexit()
