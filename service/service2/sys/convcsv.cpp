#include <exodus/library.h>
libraryinit()

//#include <makelist.h>
//#include <dicti2a.h>
//#include <xselect.h>
//#include <uconvfile.h>

#include <gen.h>//only used for gen.company which should be moved to sys common

var temp;//num
var selectx;
var tt;
var dicthasauthorised;//num
var keyx;
var nkeys;//num
var outfilename;
var reply;
var buffer;
dim filenames;
dim oconvxs;
dim fmtxs;
dim dictrecs;
dim files;
dim dictids;
dim colgroups;
var v69;
var v70;
var v71;
var dict;
var outfile;
dim rec;
dim mvrec;
//var converterparams;
var result;
var errors;
var savelistactive;

function main(in mode, in select, in nfilters, dim& filters) {

	if (false && mode && select && nfilters && filters(0,0)){};

	var filename = SENTENCE.field(" ", 2);
	var file;
	if (not(file.open(filename))) {
		return fsmsg();
	}

	var sentencex = SENTENCE;
	sentencex.converter(" ", VM);
	if (sentencex.locate("SELECT", temp, 1)) {
		selectx = SENTENCE.field(" ", temp + 1, 9999);
		sentencex = SENTENCE.field(" ", 1, temp - 1);
	}else{
		sentencex = SENTENCE;
		selectx = "";
	}

	var normalise = sentencex.index(" NORMALISE", 1);
	if (normalise) {
		sentencex.swapper(" NORMALISE", "");
	}

	var raw = sentencex.index(" RAW", 1);
	if (raw) {
		sentencex.swapper(" RAW", "");
	}
	var mvgroupno = "";

	var colheaderrow = not sentencex.index(" NOCOLHEADER", 1);
	if (not colheaderrow) {
		sentencex.swapper(" NOCOLHEADER", "");
	}

	if (filename.substr(1,4) == "DICT") {
		tt = "VOC";
	}else{
		tt = filename;
	}
	if (not(DICT.open("dict_"^tt))) {
		return fsmsg();
	}

//	var converter = "";
//	if (not((tt!!!).read(DEFINITIONS, "CONVERTER*" ^ filename))) {
//		tt!!! = "";
//	}
//	if (tt!!!.a(1)) {
//		converter = "CONVERTER." ^ tt!!!.a(1);
//	}

	var xx;
	if (xx.read(DICT, "AUTHORISED")) {
		dicthasauthorised = 1;
	}else{
		dicthasauthorised = 0;
	}

	var notexportable = "";
	var exportable = sentencex.field(" ", 3, 9999);
	if (exportable) {
		exportable.converter(" ", FM);
		if (exportable.a(1) == "EXCEPT") {
			notexportable = exportable.field(FM, 2, 9999);
			exportable = "";
		}
	}

	//expand any group fields in notexportable
	if (notexportable) {
		for (var ii = 1; ii <= notexportable.count(FM) + 1; ++ii) {
			var dictrec;
			if (dictrec.read(DICT, notexportable.a(ii))) {
				if (dictrec.a(1) == "G") {
					temp = dictrec.a(3);
					temp.converter(VM ^ " ", FM ^ FM);
					notexportable.r(ii, temp);
				}
			}
		};//ii;
	}

	var listkey = var(1000000).rnd();

	if (not exportable) {

		if (exportable.read(DICT, "exportable")) {
			if (exportable.a(1) == "G") {
				exportable = exportable.a(3);
				exportable.converter(VM ^ " ", FM ^ FM);
			}
			keyx = exportable.substr(1,exportable.index(FM ^ FM, 1) - 1);
			nkeys = keyx.count(FM) + 1;
			if (nkeys > 2) {
				//call msg('Key field(s) should be followed by a blank line or space in EXPORTABLE')
				//stop
				nkeys = 0;
			}
		}else{
			exportable = "";
			keyx = "";
		}

	}

	var exportable2 = exportable;

//nextmvgroup:

	if (mvgroupno) {
		tt = keyx ^ FM ^ "LINE_NO" ^ FM;
	}else{
		tt = "";
	}

	exportable = tt ^ exportable2.field("%", 1);
	exportable2 = exportable2.field("%", 2, 9999);

	outfilename = SYSTEM.a(2);
	//zzz if mvgroupno then outfilename[8,1]=mvgroupno
	if (outfilename.lcase().substr(-4, 4) == ".htm") {
		outfilename.splicer(-3, 3, "xls");
		SYSTEM.r(2, outfilename);
	}

	var excel = outfilename.lcase().substr(-3, 3) == "xls";

//retry:
	outfilename.osdelete();
	if (outfilename.osfile()) {
		return exit2("CANNOT EXPORT BECAUSE " ^ outfilename ^ " IS ALREADY|OPEN IN ANOTHER PROGRAM, OR CANNOT BE ACCESSED");
	}

	//call note2("Exporting " ^ outfilename ^ "||Please wait ...", "UB", buffer, "");

	files.redim(255);
	filenames.redim(255);
	oconvxs.redim(255);
	fmtxs.redim(255);
	dictrecs.redim(255);
	filenames="";
	oconvxs="";
	fmtxs="";
	var nfields = 0;
	
	var selectlist = LISTACTIVE;
	if (selectlist) {
		savelistactive=LISTACTIVE;
	}

	if (exportable) {
		//call makelist("", exportable, "", "");
		//write exportable on lists,listkey
		//perform 'GET-LIST ':listkey:' (S)'
		//delete lists,listkey
		var tt="select dict_" ^ filename ^ " " ^ exportable.swap(FM, "\" \"").quote();
		DICT.select(tt);

	}else{
		if (filename.substr(1,4).lcase() == "dict") {
			dict = "";
		}else{
			dict = "dict_";
		}
		DICT.select("select " ^ dict ^ filename ^ " BY FMC WITH FMC BETWEEN 1 AND 999999 AND WITH @ID NOT STARTING \'%\' AND WITH MASTER.FLAG (S)");
		if (not LISTACTIVE) {
			return exit2(dict ^ filename ^ " has no exportable columns");
		}
	}

	dictids.redim(255);
	colgroups.redim(255);

	dictids = "";
	colgroups = "";
	var headingx = "";
	var coln = 0;

	var dictid;
	while (DICT.readnext(dictid, MV)) {

		if (notexportable.locateusing(dictid, FM, xx)) {
			continue;
		}

		if (dictid[1] == "%") {
			continue;
		}

		if (dictid == "LINE_NO") {
			coln += 1;
			dictids(coln) = dictid;
			headingx.r(coln, dictid);
			fmtxs(coln) = "R";
			dictrecs(coln) = "";
		}else{
			if (dict.read(DICT, dictid)) {
//				call dicti2a(dict);
				coln += 1;
				//if dict<2> matches '0N' then
				var fn = dict.a(2);
				if (fn > nfields) {
					nfields = fn;
				}

				fmtxs(coln) = dict.a(9)[1];

				if (raw) {
					headingx.r(coln, dictid);
				}else{

					//extract title
					var title = dict.a(3).trim();
					title.swapper("<WBR/>", "");
					title.swapper("<wbr/>", " ");
					title.converter(UPPERCASE ^ "|_" _VM_ "", LOWERCASE ^ "   ");

					//t=title[1,1]
					//convert @lower.case to @upper.case in t
					//title[1,1]=t
					title = capitalise(title);

					title.swapper("(Base)", "(" ^ gen.company.a(3) ^ ")");

					//swap ' code' with '' in title

					headingx.r(coln, title.trim());
				}

				//extract file
				if (dict.a(11)[1] == "<") {
					temp = dict.a(11).substr(2,9999).field(">", 1);
					filenames(coln) = temp;
					if (not(files(coln).open(temp, ""))) {
						return exit2(DQ ^ (temp ^ DQ) ^ " file cannot be found in dict " ^ (DQ ^ (dictid ^ DQ)));
					}
					var title = headingx.a(coln);
					if (title.ucase().substr(-5,5) == " CODE") {
						title.splicer(-5, 5, "");
						headingx.r(coln, title);
					}
				}

				//extract conversion
				if (dict.a(7)) {
					var oconvx = dict.a(7);

					//force long date format
					if (oconvx.index("DATE", 1) or oconvx[1] == "D") {
						//if raw then
						// oconvx='D4/J'
						//end else
						if (oconvx == "[SCH.DATES]") {
							oconvx = "";
						}else{
							oconvx = "D4/E";
						}
						//end
					}

					oconvxs(coln) = oconvx;
				}

				colgroups(coln) = dict.a(4)[1] == "M";
				dictids(coln) = dictid;
				dictrecs(coln) = dict;

			}
			//end
		}
	}//nextdict
	var ncols = coln;

	//if @username='NEOSYS' then oswrite matunparse(dictids) on 'csv'

	if (selectlist) {
		LISTACTIVE=savelistactive;
	}

	call oswrite("", outfilename);
	if (not outfile.osopen(outfilename)) {
		return exit2(outfilename.quote()^" file cannot be created");
	}
	var ptr = 0;

	//suppress headerrow if not required
	if (not colheaderrow) {
		headingx = "";
	}

	if (selectx) {
	//selectx:=' AND WITH PERSON_CODE "HARRIS"'
		//perform 'SELECT ':filename:' ':selectx
		tt = "SELECT " ^ filename ^ " " ^ selectx;
		//call xselect(tt);
		file.select(tt);
/*		if (not LISTACTIVE) {
			outfile.osclose();
			outfilename.osdelete();
			return exit2("No records found");
		}
*/
	}else{
		if (not LISTACTIVE) {
			file.select();
		}
	}

	var recn = 0;

	rec.redim(ncols);
	mvrec.redim(ncols);

/////
//nextrecord:
/////

	//get the next key and mv
	var mvx;
	while (file.readnext(ID, mvx)) {

		//user interrupt
		if (esctoexit()) {
			outfile.osclose();
			//osdelete outfilename
			return exit2("Interrupted by User");
		}

		//skip "" key
		if (ID == "") {
			continue;
		}

		recn += 1;

		//cout << AW.a(30)<< var().cursor(0)<< var().cursor(-4);
		//cout << var().cursor(39, _CRTHIGH / 2)<< recn<< ". ";

		//get the record
		if (not(RECORD.read(file, ID))) {
			continue;
		}

		//skip record if not authorised
		if (dicthasauthorised and not calculate("AUTHORISED")) {
			continue;
		}

		//skip zero hours in timesheets
		//TODO get this hack into a special dictionary item like LIMIT
		if (filename == "TIMESHEETS" and mvx and not RECORD.a(2, mvx)) {
			continue;
		}
		
		//get the data and work out maximum vn
		mvrec = "";
		var maxvn=0;
		for (var coln = 1; coln <= ncols; ++coln) {

			dictid = dictids(coln);
			if (dictid eq "LINE_NO")
				continue;

			MV = mvx;
			var cell = calculate(dictid);
			if (not cell.length())
				continue;
				
			if (oconvxs(coln)) {
				cell = cell.oconv(oconvxs(coln));
			}

			mvrec(coln) = cell;

			var temp = cell.dcount(VM);
			if (temp > maxvn)
				maxvn = temp;
				
		}

		//skip if no data
		if (not maxvn)
			continue;

		//normalise the data and output to csv file
		
		//output the lines
		for (var vn=1; vn <= maxvn; ++vn) {

			rec="";
					
			//conversions
			for (var coln = 1; coln <= ncols; ++coln) {

				var cell;
				
				if (dictids(coln) == "LINE_NO") {
					cell = vn;

				//select right multivalue if multivalued field/column
				//non-multivalued fields/columns are repeated on every line
				}else if (colgroups(coln)) {
						cell=mvrec(coln).a(1,vn);
						
				//not multivalued so repeat on every line
				} else {
						cell = mvrec(coln);
				}
								
				//skip empty cells
				if (not cell.length())
					continue;

				//conversions
				if (mvx or vn == 1) {

					//convert codes to names
					if (filenames(coln) and not raw) {
						var rec2;
						if (rec2.read(files(coln), cell)) {
							if (filenames(coln) == "BRANDS") {
								cell = rec2.a(2, 1);
							}else{
								cell = rec2.a(1);
							}
						}
					}

				}

				//remove any initial + sign - on numbers only
				if (cell[1] == "+") {
					if ((cell.substr(2,9999)).isnum()) {
						cell.splicer(1, 1, "");
					}
				}
				
				//a single double quote gets changed to ''
				if (cell == DQ) {
					cell = "\'\'";
				}
				
				//swap double quotes for '' unless already double quoted
				if (cell[1] ne DQ or cell[-1] ne DQ) {
					cell.swapper(DQ, "\'\'");
				}
				
				//WARNING prevent cell length more than 255
				if (cell.length() > 255) {
					cell = cell.substr(1,250) ^ " ...";
				}
				
				//double quote non-numerics
				if (fmtxs(coln) ne "R") {

					//make sure "1-12" is not interpreted as a formula (by prefixing a space?)
					if (1 or excel) {
						if (var(".-+0123456789").index(cell[1])) {
							if (not cell.isnum()) {
								cell.splicer(1, 0, " ");
							}
						}
					}

					//convert any existing double quotes to '' for T columns which are not already double quoted
					if (cell.index(DQ, 1)) {
						if (fmtxs(coln) == "T") {
							if (cell[1] ne DQ or cell[-1] ne DQ) {
								cell.swapper(DQ, "\'\'");
								cell = DQ ^ (cell ^ DQ);
							}
						}
					//otherwise just double quote stuff
					}else{
						cell = DQ ^ (cell ^ DQ);
					}

				}

				rec(coln) = cell;

			};//coln;

			var line = rec.unparse();
			
			//remove trailing or all tab chars
			line.trimmerb(FM);

			//suppress output of empty amv rows
			if (mvgroupno and nkeys) {
				if (line.field(FM, nkeys + 2, 9999) == "") {
					line = "";
				}
			}

			//output one line
			if (not line.length())
				continue;

			//remove leading equal signs in order not to confuse Excel
			line.swapper(FM ^ "=", FM);

			//output header row if first line and not suppressed

			if (headingx) {

//				if (converter) {
//					//headingx will come back converted and maybe as multiple lines
//					//converterparams initially contains first line so heading can put some columns into heading if required
//					//converterparams comes back with info to speed convertion of lines
//					converterparams = line;
//					call onverter("HEAD", headingx, converterparams, filename);
//				}else{
					headingx.converter(FM, var().chr(9));
					headingx ^= "\r\n";
//				}

				osbwrite(headingx, outfile, ptr);

				headingx = "";
			}

			//output line

//			if (converter) {
//				call @converter("LINE", line, converterparams, filename);
//			}else{
				line.swapper(FM, var().chr(9));
				line ^= "\r\n";
//			}

			osbwrite(line, outfile, ptr);

		}//next vn

	}//goto nextrecord
	
/////
//exit:
/////
	outfile.osclose();
//	call uconvfile(outfile, "CODEPAGE", "UTF16", result, errors);
	//general result code
	SYSTEM.r(34, 1);
	
//exit3:
//	if (raw and exportable2) {
//		mvgroupno += 1;
//		if (mvgroupno == 1) {
//			mvgroupno = 2;
//		}
//		goto nextmvgroup;
//	}
	return 1;
}

function exit2(in msg="") {
	call mssg(msg);
	SYSTEM.r(34, 0);
	return 0;
}

libraryexit()
