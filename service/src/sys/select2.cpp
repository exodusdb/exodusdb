#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <listen3.h>
#include <safeselect.h>
#include <systemsubs.h>

#include <gen_common.h>
#include <win_common.h>

var dictids;
var options;
var maxnrecs;
var nlimitfields;//num
dim hexx;
var useactivelist;//num
var v69;
var v70;
var v71;
var storer;
var storeid;
var storedict;
var storemv;
var commonsaved;//num
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

function main(in filenamex, in linkfilename2, in sortselect0, in dictids0, in options0, io datax, io response, in limitfields="", in limitchecks="", in limitvalues="", in maxnrecs0=0) {
	//c sys ,,,,,io,io,"","","",0

	//NB add %SELECTLIST% to sortselect to use active select list
	//or provide an FM terminated list of keys

	//given a sort/select statement
	//returns a dynamic array or file of xml data

	//unless sortselect is prefixed with %RAW% then
	//!!!needs something like the following in the calling program
	//since it calls LISTEN3 and filename.subs('POSTREAD') to get accurate data
	//$insert gbp,arev.common
	//clearcommon

	#include <general_common.h>
	//global maxnrecs,useactivelist

	var filename = filenamex;

	var sortselect = sortselect0;
	if (dictids0.unassigned()) {
		dictids = "";
	}else{
		dictids = dictids0;
	}
	if (options0.unassigned()) {
		options = "";
	}else{
		options = options0;
	}
	if (maxnrecs0.unassigned()) {
		maxnrecs = "";
	}else{
		maxnrecs = maxnrecs0;
	}
	if (limitfields.unassigned()) {
		nlimitfields = 0;
	}else{
		nlimitfields = limitfields.count(VM) + (limitfields ne "");
	}

	var xml = options.index("XML");
	var rawread = options.index("RAW");

	datax = "";

	if (linkfilename2) {
		hexx.redim(8);
		//changed to allow language characters to pass through x80-xF9
		for (var ii = 249; ii <= 255; ++ii) {
			hexx(ii - 249) = "%" ^ ii.oconv("MX").oconv("R(0)#2");
		};//ii;
	}

	#define msg USER4

	useactivelist = sortselect.index("%SELECTLIST%");
	sortselect.swapper("%SELECTLIST%", "");
	if (not(LISTACTIVE)) {
		useactivelist = 0;
	}

	//fm termination indicates list of keys and optionally mvnos
	var givenkeys = sortselect[-1] eq FM;
	var givenkeyn = 0;

	if (not useactivelist) {
		call pushselect(0, v69, v70, v71);
	}

	RECORD.transfer(storer);
	ID.transfer(storeid);
	DICT.transfer(storedict);
	MV.transfer(storemv);

	//detect if called without common
	//NB cant run without common in caller since perform gives some RTP20 error
	if (rawread) {
		goto nocommon;
	}
	if (win.srcfile.unassigned()) {
nocommon:
		commonsaved = 0;
	}else{
		commonsaved = 1;
		win.srcfile.transfer(savesrcfile);
		win.datafile.transfer(savedatafile);
		win.wlocked.transfer(savewlocked);
		USER4.transfer(savemsg);
		win.reset.transfer(savereset);
		win.valid.transfer(savevalid);
	}

	//if unassigned(limitfields) then limitfields=''

	//filename can be 'filename USING dictfilename'

	var filename0 = filename;
	if (filename.field(" ", 2) eq "USING") {
		dictfilename = filename.field(" ", 3);
		filename = filename.field(" ", 1);
	}else{
		dictfilename = filename;
	}

	response = "OK";
	var file;
	if (not(file.open(filename, ""))) {
		response = "Error: " ^ (filename.quote()) ^ " file is not available";

		//abort
		gosub exit();
		return 0;

	}

	if (linkfilename2) {
		call oswrite("", linkfilename2);
		if (not(linkfile2.osopen(linkfilename2))) {
			response = "Error: " ^ (linkfilename2.quote()) ^ " cannot open output file";

			//abort
			gosub exit();
			return 0;

		}
	}else{
		datax = "";
	}
	var dataptr = 0;

	var cmd = "SELECT";
	if (maxnrecs) {
		cmd ^= " " ^ maxnrecs;
	}
	cmd ^= " " ^ filename0;
	//if trim(@station)='SBCP1800' then cmd='SELECT 10 ':filename

	//check no @ in xml dict ids because cannot return xml tag with @
	if (xml and dictids.index("@")) {
		response = "Error: XML dictids cannot contain @ characters in SELECT2";

		//abort
		gosub exit();
		return 0;

	}

	var oconvsx = "";
	if (dictids.unassigned()) {
		dictids = "";
	}
	if (dictids eq "") {
		dictids = "ID";
	}
	dictids.trimmer();
	dictids.converter(" ", FM);
	var dictrecs = "";

	if (not(DICT.open("dict_" ^ dictfilename))) {
		response = "Error: " ^ (("DICT." ^ filename).quote()) ^ " file is not available";

		//abort
		gosub exit();
		return 0;

	}

	if (not(openfile("dict_voc", dictvoc))) {
		dictvoc = "";
	}

	if (rawread) {
		haspostread = "";
	}else{
		call listen3(filename, "READ", realfilename, triggers);
		//postread=triggers<3>
		haspostread = triggers.a(3) ne "";
		systemsubs = triggers.a(3);
		postreadmode = triggers.a(4);
	}

	//check/get dict recs

	if (dictids ne "RECORD") {
		while (true) {
			///BREAK;
			if (not(dictids[-1] eq FM)) break;
			dictids.splicer(-1, 1, "");
		}//loop;
		if (dictids eq "") {
			dictids = "ID";
		}
		ndictids = dictids.count(FM) + 1;
		for (var dictidn = 1; dictidn <= ndictids; ++dictidn) {
			var dictid = dictids.a(dictidn);
			var dictrec;
			if (not(dictrec.read(DICT, dictid))) {
				if (not(dictrec.read(dictvoc, dictid))) {
					if (dictid eq "ID") {
						dictrec = "F,0,No,,,,,,L,15,";
						dictrec.converter(",", FM);
					}else{
						response = "Error: " ^ (dictid.quote()) ^ " IS MISSING FROM DICT." ^ filename;

						//abort
						gosub exit();
						return 0;

					}
				}
			}

			//pick items
			//if index('DI',dictrec<1>,1) then call dicti2a(dictrec)

			//pick A is revelation F
			if (dictrec.a(1) eq "A") {
				dictrec.r(1, "F");
				}

			dictrec = lower(dictrec);
			dictrecs.r(dictidn, dictrec);
			oconvsx.r(dictidn, dictrec.a(1, 7));
		};//dictidn;
	}

	var tx = "";
	if (xml and linkfilename2) {
		//tx:='<XML id=':quote(lcase(filename)):'>':crlf
		tx ^= "<records>" "\r\n";
		call osbwrite(tx, linkfile2,  dataptr);
	}

	//read xx from @dict,'AUTHORISED' then
	// if index(sortselect,' WITH AUTHORISED',1) else
	//  if index(' ':sortselect,' WITH ',1) then sortselect:=' AND'
	//  sortselect:=' WITH AUTHORISED'
	//  end
	// end
	var chk_authorised;
	if (not(chk_authorised.read(DICT, "AUTHORISED"))) {
		chk_authorised = 0;
	}

	if (not givenkeys) {

		//zzz should for validity of select parameters first
		//otherwise in server mode it loops with a very long error message

	//perform 'SELECT XXXXXXX WITH BRAND_CODE "LU" BY YEAR_PERIOD AND WITH ID NE "[*I" BY ID'

		//if filename='JOBS' or filename='COMPANIES' then

		//do after readnext to avoid two stage sort/select in c++
		//read xx from @dict,'AUTHORISED' then
		// if index(sortselect,' WITH AUTHORISED',1) else
		//  if index(' ':sortselect,' WITH ',1) then sortselect:=' AND'
		//  sortselect:=' WITH AUTHORISED'
		//  end
		// end

		//if not sorted then try use %RECORDS% if present and <200 chars
		var records = "";
		//if @list.active or index(' ':sortselect,' BY ',1) or index(sortselect,'WITH AUTHORISED',1) else
		if (not(LISTACTIVE or ((" " ^ sortselect).index(" BY ")))) {
			//only look in selected files otherwise c++ takes too long on some files
			if (var("COMPANIES,CURRENCIES,UNITS,LEDGERS,JOB_TYPES").locateusing(",",filenamex,xx)) {
				if (records.read(file, "%RECORDS%")) {
					if (records.length() lt 200) {
						records.swapper(FM, "\" \"");
						sortselect.splicer(1, 0, records.quote() ^ " ");
					}
				}
			}
		}

		//if @list.active else call safeselect(cmd:' ':sortselect:' (S)')
		if (not(LISTACTIVE) or sortselect) {
			call safeselect(cmd ^ " " ^ sortselect ^ " (S)");
		}

		//handle invalid cmd
		//R18.1 is normal 'No records found' message
		if (USER4 and not(USER4.index("R18.1"))) {
			if (USER4.field(" ", 1) eq "W156") {
				USER4 = USER4.field(" ", 2).quote() ^ " is not in the dictionary.||" ^ cmd ^ " " ^ sortselect;
			}
			response = USER4;

			//abort
			gosub exit();
			return 0;

		}

	}

	//return empty results even if no records selected
	var selectresult = "";

	var recn = "";

	if (not rawread) {
		win.datafile = filename;
		win.srcfile = file;
	}

	//read each record and add the required columns to the selectresult

////////
nextrec:
////////
	//if recn then if recn>=maxnrecs then goto nomore

	if (givenkeys) {

		givenkeyn += 1;
		ID = sortselect.a(givenkeyn);
		MV = ID.a(1, 2);
		if (MV.length()) {
			ID = ID.a(1, 1);
		}

	}else{
		if (not(readnext(ID, MV))) {
			ID = "";
		}
	}

	//if no more
	if (ID eq "") {

		if (xml and linkfilename2) {
			var tt = "</records>";
			call osbwrite(tt, linkfile2,  dataptr);
		}

		if (linkfilename2) {
			linkfile2.osclose();
			var().osflush();
		}

		//finished
		gosub exit();
		return 0;
	}

	if (ID[1] eq "%") {
		goto nextrec;
	}
	if (not(RECORD.read(file, ID))) {
		goto nextrec;
	}

	//filter out unauthorised
	if (chk_authorised) {
		if (not(calculate("AUTHORISED"))) {
			goto nextrec;
		}
	}

	//filter out unwanted multivalues that the stupid rev sortselect leaves in
	for (var limitfieldn = 1; limitfieldn <= nlimitfields; ++limitfieldn) {
		var value = calculate(limitfields.a(1, limitfieldn));
		var reqvalue = limitvalues.a(1, limitfieldn);
		var limitcheck = limitchecks.a(1, limitfieldn);

		if (limitcheck eq "EQ") {
			if (value ne reqvalue) {
				goto nextrec;
			}

		} else if (limitcheck eq "NE") {
			if (value eq reqvalue) {
				goto nextrec;
			}

		} else {
			call mssg(limitcheck.quote() ^ " invalid limitcheck in select2");

			//abort
			gosub exit();
			stop();
			return 0;

		}
	};//limitfieldn;

	recn += 1;

	if (dictids eq "RECORD") {

		//postread (something similar also in listen/READ)
		if (haspostread) {

			//simulate window environment for POSTREAD
			win.orec = RECORD;
			win.wlocked = 1;
			USER4 = "";
			win.reset = 0;

			call systemsubs(postreadmode);
			DATA = "";

			//call trimexcessmarks(iodat)

			//postread can request abort by setting msg or reset>=5
			if (win.reset ge 5 or USER4) {
				goto nextrec;
			}

		}

		//prevent reading passwords postread and postwrite
		if (filename eq "DEFINITIONS" and ID eq "SECURITY") {
			RECORD.r(4, "");
		}

		///

		RECORD.transfer(row);

		var prefix = ID ^ FM;

		if (dataptr) {
			prefix.splicer(1, 0, RM);
		}
		row.splicer(1, 0, prefix);

	//dictids ne RECORD
	}else{
		row = "";

		for (var dictidn = 1; dictidn <= ndictids; ++dictidn) {
			var dictid = dictids.a(dictidn);
			var dictid2 = dictid;
			dictid2.converter("@", "");
			var cell = calculate(dictid);
			if (oconvsx.a(dictidn)) {
				cell = oconv(cell, oconvsx.a(dictidn));
			}
			if (xml) {
				//cell='X'
				//convert "'":'".+/,()&%:-1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz' to '' in cell
				cell.swapper("%", "%25");
				cell.swapper("&", "&amp;");
				cell.swapper("<", "&lt;");
				cell.swapper(">", "&gt;");
				//if cell then deb ug
				//cell=quote(str(cell,10))
				row ^= "<" ^ dictid2 ^ ">" ^ cell ^ "</" ^ dictid2 ^ ">" "\r\n";
			}else{
				row.r(1, dictidn, cell);
			}
		};//dictidn;

		if (xml) {
			row = "<RECORD>" "\r\n" ^ row ^ "</RECORD>" "\r\n";
			//move up
			//swap '&' with '&amp;' in row
			//swap "'" with "" in row
		}
		if (dataptr) {
			if (not xml) {
				row.splicer(1, 0, FM);
			}
		}else{
			if (xml) {
				row.splicer(1, 0, "<records>" "\r\n");
				//row[1,0]='<XML id=':quote(lcase(filename)):'>':crlf
			}
		}

	}

	if (linkfilename2) {

		//in LISTEN and SELECT2 for direct output

		var blockn = 0;
		while (true) {
			var rowpart = row.substr(blockn * 16384 + 1,16384);
			///BREAK;
			if (not rowpart) break;

			rowpart.swapper("%", "%25");
			//changed to allow language characters to pass through x80-xF9
			for (var ii = 249; ii <= 255; ++ii) {
				rowpart.swapper(var().chr(ii), hexx(ii - 249));
			};//ii;

			//output converted row part
			call osbwrite(rowpart, linkfile2,  dataptr);

			blockn += 1;
		}//loop;

	}else{
		datax ^= row;
		dataptr += row.length();
	}

	//get next if output to file or space for more data
	//goto nextrec
	//if xml or len(DATAX)<64000 then goto nextrec
	if (xml or (datax.length() lt maxstrsize - 1530)) {
		goto nextrec;
	}

	gosub exit();
	return 0;
}

subroutine exit() {
	if (commonsaved) {
		savesrcfile.transfer(win.srcfile);
		savedatafile.transfer(win.datafile);
		savewlocked.transfer(win.wlocked);
		savemsg.transfer(USER4);
		savereset.transfer(win.reset);
		savevalid.transfer(win.valid);
	}

	storer.transfer(RECORD);
	storeid.transfer(ID);
	storedict.transfer(DICT);
	storemv.transfer(MV);

	if (not useactivelist) {
		call popselect(0, v69, v70, v71);
	}

	return;
}

libraryexit()
