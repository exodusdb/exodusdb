#include <exodus/library.h>
libraryinit()

#include "win.h"

	//uses RECORD/ID/DICT/MV to pass info to dictionary subroutine so protect the current values of these
	var storerecord;
	var storeid;
	var storedict;
	var storemv;

	//these are used to call POSTREAD routines in case returning whole RECORD so protect the current values
	var savewinsrcfile;
	var savewindatafile;
	var savewinorec;
	var savewinwlocked;
	var savewinmsg;
	var savewinreset;
	var savewinvalid;

	//maybe creating own default select list so perhaps should protect these
	//TODO consider creating a random new cursor on each call
	var useactivelist;
	var v69;
	var v70;
	var v71;

function main(in filenamex, in linkfilename2, in sortselect0, in dictids0, in options, io datax, io response, in limitfields0="", in limitchecks="", in limitvalues="", int maxnrecs=0)
{

	if (not win_isdefined) {
		stop();
		throw MVException("win common is missing in select2");
	}

	var dictfilename;
	var dictmd;
	var realfilename;
	var triggers;
	var ndictids;
	var row;

	//given a sort/select statement
	//returns a dynamic array or file of xml data
	//nb add %selectlist% to sortselect to use active select list

	//needs something like the following in the calling program?
	//$include <win.h>
	//clearcommon

	//declared high up outside range of return exit(); statement
	var selectresult;
	var offset;
	var cmd;
	var xml;
	var oconvsx;
	var tx;
	var xx;
	var records;
	var recn;
	var postread;
	var postreadmode;
	var dictrecs;
	var library;

	var filename = filenamex;
	datax = L"";

	var crlf2 = var().chr(13) ^ var().chr(10);
	//crlf2=''

	var sortselect=sortselect0.swap(L"%SELECTLIST%", L"");
	useactivelist = sortselect.index(L"%SELECTLIST%", 1);
	if (not LISTACTIVE)
		useactivelist = 0;

	gosub saveenv();

	var limitfields=limitfields0.unassigned()?L"":limitfields0;

	var dictids=dictids0.unassigned()?L"":dictids0;
	if (dictids == L"")
		dictids = L"ID";
	dictids.trimmer();
	dictids.converter(L" ", FM);
	dictrecs = L"";

	//filename can be 'filename using dictfilename'

	var filename0 = filename;
	if (filename.field(L" ", 2, 1) == L"USING") {
		dictfilename = filename.field(L" ", 3, 1);
		filename = filename.field(L" ", 1, 1);
	}else{
		dictfilename = filename;
	}

	response = L"OK";
	var file;
	if (!file.open(filename)) {
		return exit(response, filename.quote() ^ L" file is not available");
	}

	if (linkfilename2) {
			var(L"").oswrite(linkfilename2);
		if (!(linkfilename2.osopen())) {
			return exit(response, linkfilename2.quote() ^ L" cannot open output file");
		}
	}else{
		datax = L"";
	}
	offset = 0;

	cmd = L"SELECT " ^ filename0;
	if (maxnrecs)
		cmd ^= maxnrecs;

	//if trim(@station)='sbcp1800' then cmd='select 10 ':filename

	xml = options.index(L"XML", 1);

	oconvsx = L"";

	if (!DICT.open(L"dict_"^dictfilename)) {
		return exit(response, L"DICT." ^ filename.quote() ^ L" file is not available");
	}

	//if (!openfile(L"DICT.MD", dictmd))
		dictmd = L"";

	library="";
/* reimplement as external function
	if (!library.load(filename))
	{
		//throw L"MvWindow::select2() " ^ filename ^ L" unknown filename";
	}
*/

	//check/get dict recs

	if (dictids not_eq L"RECORD") {
		while (dictids[-1] == FM) {
			dictids.splicer(-1, 1, L"");
		}
		if (dictids == L"")
			dictids = L"id";
		ndictids = dictids.count(FM) + 1;
		for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
			var dictid = dictids.a(dictidn);
			var dictrec;
			if (!dictrec.read(DICT, dictid)) {
				if (!dictrec.read(DICT, dictid.lcase())) {
					if (!dictmd||!dictrec.read(dictmd, dictid)) {
						if (dictid.lcase() == L"id") {
							dictrec=var(L"F" _VM_ L"0" _VM_ L"No" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"L" _VM_ L"15" _VM_ L"").raise();
						}else{
							return exit(response, dictid.quote() ^ L" IS MISSING FROM DICT." ^ filename);
						}
					}
				}
			}

			//pick items
//TODO:			if (var(L"DI").index(dictrec.a(1), 1))
//				dicti2a(dictrec);

			//pick a is revelation f
			if (dictrec.a(1) == L"A")
				dictrec.r(1, L"F");

			dictrec.lowerer();
			dictrecs.r(dictidn, dictrec);
			oconvsx.r(dictidn, dictrec.a(1, 7));
		};//dictidn;
	}

	tx = L"";
	if (xml and linkfilename2) {
		//tx:='<xml id=':quote(lcase(filename)):'>':crlf
		tx ^= L"<records>" ^ crlf2;
		osbwritex(encode(tx), linkfilename2, linkfilename2, offset);
	}
//	offset += tx.length();

	//zzz should for validity of select parameters first
	//otherwise in server mode it loops with a very long error message

	//perform 'select schedules with brand_code "lu" by year_period and with id ne "[*i" by id'

	//if filename='jobs' or filename='companies' then

	if (xx.read(DICT, L"AUTHORISED")) {
		if (!(sortselect.index(L" WITH AUTHORISED", 1))) {
			if (var(L" " ^ sortselect).index(L" WITH ", 1))
				sortselect ^= L" AND";
			sortselect ^= L" WITH AUTHORISED";
		}
	}

	//if not sorted then try use %records% if present and <200 chars
	records = L"";
	//if @list.active or index(' ':sortselect,' by ',1) or index(sortselect,'with authorised',1) else
	if (!(LISTACTIVE or var(L" " ^ sortselect).index(L" BY ", 1))) {
		if (records.read(file, L"%RECORDS%")) {
			if (records.length() < 200) {
				records.swapper(FM, L"\" \"");
				sortselect.splicer(1, 0, records.quote() ^ L" ");
			}
		}
	}

	//if @list.active else call safeselect(cmd:' ':sortselect:' (s)')
	//oswrite cmd:' ':sortselect on 'x'
//TODO:	if (not LISTACTIVE or sortselect)
//		safeselect(cmd ^ L" " ^ sortselect ^ L" (S)");
	filename.select();

	//handle invalid cmd
	//r18.1 is normal 'no records found' message
	if (USER4 and not USER4.index(L"R18.1", 1)) {
		if (USER4.field(L" ", 1, 1) == L"W156")
			USER4 = (USER4.field(L" ", 2, 1)).quote() ^ L" is not in the dictionary.||" ^ cmd ^ L" " ^ sortselect;
		return exit(response, USER4);
	}

	//return empty results even if no records selected
	//if @list.active then

	//moved up var selectresult = L"";

	recn = L"";

	win.datafile = filename;
	win.srcfile = file;

	//read each record and add the required columns to the selectresult
nextrecord:
	if (filename.readnext(ID,MV)) {

		if (ID[1] eq "%") {
			goto nextrecord;
		}
//printl("select2 "^ID);
		if (not RECORD.read(file, ID)) {
			goto nextrecord;
		}

//printl("select2 id,record ", ID, ",", RECORD.substr(1,40));
		//filter out unwanted multivalues that sortselect sometimes leaves in
		if (limitfields) {
			var nlimitfields = limitfields.count(VM) + 1;
			var value, reqvalue, limitcheck;
			for (int limitfieldn = 1; limitfieldn <= nlimitfields; limitfieldn++) {
				value = calculate(var(limitfields.a(1, limitfieldn)));
				reqvalue = limitvalues.a(1, limitfieldn);
				limitcheck = limitchecks.a(1, limitfieldn);
				if (limitcheck == L"EQ") {
					if (value not_eq reqvalue)
						goto nextrecord;

				}else if (limitcheck == L"NE") {
					if (value == reqvalue)
						goto nextrecord;

				}else if (1) {
					return exit(response, limitcheck.quote() ^ L" invalid limitcheck in select2");
				}
			};//limitfieldn;
		}

		recn += 1;
//asm("int $3");
		if (dictids == L"RECORD") {

			//postread (something similar also in listen/read)
			if (library) {

				//simulate window environment for postread
				win.orec = RECORD;
				win.wlocked = 1;
				USER4 = L"";
				win.reset = 0;

				//dictlib(L"POSTREAD");
/* reimplement as external function
				library.call(filename,"POSTREAD");
*/
				//could perform but perform calls func with no args!

				DATA = L"";

				//postread can request abort by setting msg or reset>=5
				if (win.reset >= 5 or USER4)
					goto nextrecord;

			}

			//prevent reading passwords postread and postwrite
			if (filename == L"DEFINITIONS" and ID == L"SECURITY")
				RECORD.r(4, L"");

			RECORD.transfer(row);

			var prefix = ID ^ FM;

			if (offset)
				prefix.splicer(1, 0, RM);
			row.splicer(1, 0, prefix);

		}else{
			row = L"";

			for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
				var dictid = dictids.a(dictidn);
//printl("select2 ", dictid);
				//@ not allowed in postgres or xml tag names
				//var dictid2 = dictid.convert(L"@", L"_");
				var dictid2 = dictid.convert(L"@", L"");

				var cell = L"";
//TODO:calculate					var cell = var(dictid).calculate();
				if (dictid==L"ID")
id:
					cell=ID;
				else if (dictrecs.a(dictidn,1)=="F") {

					var fn=dictrecs.a(dictidn,2);
					if (!fn) {
						goto id;
					}

					var vn=dictrecs.a(dictidn,4)[1]==L"M";
					if (vn) {
						vn=MV;
					} else {
						vn=0;
					}

//printl("select2 fn ", fn, " ", dictid);
					cell=RECORD.a(fn,vn);

				} else {
// asm(" int $03");
					cell=calculate(dictid);
					//call calculate(dictid);
					//ANS.transfer(cell);
				}
				if (oconvsx.a(dictidn)) {
					cell = cell.oconv(oconvsx.a(dictidn));
				}
				if (xml) {
					//cell='x'
					//convert "'":'".+/,()&%:-1234567890abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz' to '' in cell
					cell.swapper(L"%", L"%25");
					cell.swapper(L"<", L"&lt;");
					cell.swapper(L">", L"&rt;");
					//if cell then deb ug
					//cell=quote(str(cell,10))
					row ^= L"<" ^ dictid2 ^ L">" ^ cell ^ L"</" ^ dictid2 ^ L">" ^ crlf2;
				}else{
					row.r(1, dictidn, cell);
				}
			};//dictidn;

			if (xml) {
				row = L"<RECORD>" ^ crlf2 ^ row ^ L"</RECORD>" ^ crlf2;
				row.swapper(L"&", L"&amp;");
				//swap "'" with "" in row
			}
			if (offset) {
				if (xml) {
				}else{
					row.splicer(1, 0, FM);
				}
			}else{
				if (xml) {
					row.splicer(1, 0, L"<records>" ^ crlf2);
					//row[1,0]='<xml id=':quote(lcase(filename)):'>':crlf2
				}
			}

		}

		if (linkfilename2) {
			osbwritex(encode(row), linkfilename2, linkfilename2, offset);

		}else{
			datax ^= row;
		}
//		offset += row.length();

		if (xml or datax.length() < 64000)
			goto nextrecord;
	}

	// end

	if (xml and linkfilename2) {
		var tt = L"</records>";
		osbwritex(encode(tt), linkfilename2, linkfilename2, offset);
	}

	if (linkfilename2) {
		linkfilename2.osclose();
		//var().osflush();
	}

	return exit(response, "");

}

subroutine saveenv(){

	//save and setup a clean environment

	//save selectlist unless specifically told to use it
	if (!useactivelist) {
//TODO:		pushselect(0, v69, v70, v71);
	}

	RECORD.transfer(storerecord);
	ID.transfer(storeid);
	DICT.transfer(storedict);
	MV.transfer(storemv);

	win.srcfile.transfer(savewinsrcfile);
	win.datafile.transfer(savewindatafile);
	win.orec.transfer(savewinorec);
	win.wlocked.transfer(savewinwlocked);
	USER4.transfer(savewinmsg);
	win.reset.transfer(savewinreset);
	win.valid.transfer(savewinvalid);

}

subroutine restoreenv(){

	storerecord.transfer(RECORD);
	storeid.transfer(ID);
	storedict.transfer(DICT);
	storemv.transfer(MV);

	savewinsrcfile.transfer(win.srcfile);
	savewindatafile.transfer(win.datafile);
	savewinorec.transfer(win.orec);
	savewinwlocked.transfer(win.wlocked);
	savewinmsg.transfer(USER4);
	savewinreset.transfer(win.reset);
	savewinvalid.transfer(win.valid);

//TODO:	if (!useactivelist)
//		popselect(0, v69, v70, v71);

}

function exit(io response, in errmsg="") {

	gosub restoreenv();

	if (not errmsg) {
		response="OK";
		return 1;
	} else {
		response=L"Error: select2: " ^ errmsg;
		return 0;
	}
}

//in server and select2 for directoutput
function encode(in instr) {
	//do xml character encoding (do % FIRST!)
	return instr
	.swap(L"%",L"%25")
	.swap(L"<",L"%3C")
	.swap(L">",L"%3E")
	.swap(L"&",L"%26");
}

libraryexit()
