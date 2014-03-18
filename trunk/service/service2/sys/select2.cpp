#include <exodus/library.h>
libraryinit()

#include "win.h"

function main(in filenamex, in linkfilename2, in sortselect0, in dictids0, in options, io datax, io response, in limitfields0="", in limitchecks="", in limitvalues="", int maxnrecs=0)
{

	var v69;
	var v70;
	var v71;
	var storer;
	var storeid;
	var storedict;
	var storemv;
	var savesrcfile;
	var savedatafile;
	var savewlocked;
	var savemsg;
	var reset;//num
	var savereset;
	var savevalid;
	var dictfilename;
	var dictmd;
	var realfilename;
	var triggers;
	var ndictids;
	var row;
	//jbase
	//nb add %selectlist% to sortselect to use active select list

	//given a sort/select statement
	//returns a dynamic array or file of xml data
	//needs something like the following in the calling program
	//$insert gbp,arev.common
	//clearcommon

	//declared high up outside range of goto exit statement
	var selectresult;
	var dataptr;
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
	var useactivelist = sortselect.index(L"%SELECTLIST%", 1);
	if (not LISTACTIVE)
		useactivelist = 0;

	if (!useactivelist)
//TODO:		pushselect(0, v69, v70, v71);

	RECORD.transfer(storer);
	ID.transfer(storeid);
	DICT.transfer(storedict);
	MV.transfer(storemv);

	win.srcfile.transfer(savesrcfile);
	win.datafile.transfer(savedatafile);
	win.wlocked.transfer(savewlocked);
	USER4.transfer(savemsg);
	win.reset.transfer(savereset);
	win.valid.transfer(savevalid);

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
		response = L"Error: " ^ (filename.quote()) ^ L" file is not available";
		goto exit;
	}

	if (linkfilename2) {
			var(L"").oswrite(linkfilename2);
		if (!(linkfilename2.osopen())) {
			response = L"Error: " ^ (linkfilename2.quote()) ^ L" cannot open output file";
			goto exit;
		}
	}else{
		datax = L"";
	}
	dataptr = 0;

	cmd = L"SELECT " ^ filename0;
	if (maxnrecs)
		cmd ^= maxnrecs;

	//if trim(@station)='sbcp1800' then cmd='select 10 ':filename

	xml = options.index(L"XML", 1);

	oconvsx = L"";

	//check no @ in xml dict ids because cannot return xml tag with @
	if (xml and dictids.index(L"@", 1)) {
		response = L"Error: XML dictids cannot contain @ characters in SELECT2";
		goto exit;
	}

	if (!DICT.open(L"dict_"^dictfilename)) {
		response = L"Error: " ^ ((L"DICT." ^ filename).quote()) ^ L" file is not available";
		goto exit;
	}

	//if (!mv.openfile(L"DICT.MD", dictmd))
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
		while (dictids.substr(-1, 1) == FM) {
			dictids.splicer(-1, 1, L"");
		}
		if (dictids == L"")
			dictids = L"ID";
		ndictids = dictids.count(FM) + 1;
		for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
			var dictid = dictids.a(dictidn);
			var dictrec;
			if (!dictrec.read(DICT, dictid)) {
				if (!dictmd||!dictrec.read(dictmd, dictid)) {
					if (dictid == L"ID") {
						dictrec=var(L"F" _VM_ L"0" _VM_ L"No" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"L" _VM_ L"15" _VM_ L"").raise();
					}else{
						response = L"Error: " ^ (dictid.quote()) ^ L" IS MISSING FROM DICT." ^ filename;
						goto exit;
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
		mv.osbwritex(tx, linkfilename2, linkfilename2, dataptr);
	}
	dataptr += tx.length();

	//zzz should for validity of select parameters first
	//otherwise in server mode it loops with a very long error message

	//perform 'select schedules with brand_code "lu" by year_period and with id ne "[*i" by id'

	//if filename='jobs' or filename='companies' then

	if (xx.read(DICT, L"mv.authorised")) {
		if (!(sortselect.index(L" WITH mv.authorised", 1))) {
			if (var(L" " ^ sortselect).index(L" WITH ", 1))
				sortselect ^= L" AND";
			sortselect ^= L" WITH mv.authorised";
		}
	}

	//if not sorted then try use %records% if present and <200 chars
	records = L"";
	//if @list.active or index(' ':sortselect,' by ',1) or index(sortselect,'with mv.authorised',1) else
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
		response = USER4;
		goto exit;
	}

	//return empty results even if no records selected
	//if @list.active then

	//moved up var selectresult = L"";

	recn = L"";

	win.datafile = filename;
	win.srcfile = file;

	//read each record and add the required columns to the selectresult
selectnext:

	if (var(L"").readnext(ID,MV)) {

		if (RECORD.read(file, ID)) {

			//filter out unwanted multivalues that the stupid rev sortselect leaves in
			if (limitfields) {
				var nlimitfields = limitfields.count(VM) + 1;
				var value, reqvalue, limitcheck;
				for (int limitfieldn = 1; limitfieldn <= nlimitfields; limitfieldn++) {
					value = calculate(var(limitfields.a(1, limitfieldn)));
					reqvalue = limitvalues.a(1, limitfieldn);
					limitcheck = limitchecks.a(1, limitfieldn);
					if (limitcheck == L"EQ") {
						if (value not_eq reqvalue)
							goto selectnext;

					}else if (limitcheck == L"NE") {
						if (value == reqvalue)
							goto selectnext;

					}else if (1) {
						mssg(limitcheck.quote() ^ L" invalid limitcheck in select2");
						goto exit;
					}
				};//limitfieldn;
			}

			recn += 1;

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
					mv.DATA = L"";

					//call trimexcessmarks(iodat)

					//postread can request abort by setting msg or reset>=5
					if (win.reset >= 5 or USER4)
						goto selectnext;

				}

				//prevent reading passwords postread and postwrite
				if (filename == L"DEFINITIONS" and ID == L"SECURITY")
					RECORD.r(4, L"");

				RECORD.transfer(row);

				var prefix = ID ^ FM;

				if (dataptr)
					prefix.splicer(1, 0, RM);
				row.splicer(1, 0, prefix);

			}else{
				row = L"";

				for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
					var dictid = dictids.a(dictidn);
					var dictid2 = dictid;
					dictid2.converter(L"@", L"");
					var cell = L"";
//TODO:calculate					var cell = var(dictid).calculate();
                    if (dictid==L"ID")
id:
                        cell=ID;
					else if (dictrecs.a(dictidn,1)=="F")
					{
						var fn=dictrecs.a(dictidn,2);
						if (!fn) goto id;
						var vn=dictrecs.a(dictidn,4).substr(1,1)==L"M";
						if (vn)
							vn=MV;
						else
							vn=0;
						cell=RECORD.a(fn,vn);
					}
					else
					{
						//cell = dictlib(dictid);
/* reimplement as external function
cell=library.call(dictid);
*/						//cell=ANS;
					}
					if (oconvsx.a(dictidn))
						cell = cell.oconv(oconvsx.a(dictidn));
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
				if (dataptr) {
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
				mv.osbwritex(row, linkfilename2, linkfilename2, dataptr);

			}else{
				datax ^= row;
			}
			dataptr += row.length();

		}
		if (xml or datax.length() < 64000)
			goto selectnext;
	}

	// end

	if (xml and linkfilename2) {
		var tt = L"</records>";
		mv.osbwritex(tt, linkfilename2, linkfilename2, dataptr);
	}

	if (linkfilename2) {
		linkfilename2.osclose();
		var().osflush();
	}

exit:

	savesrcfile.transfer(win.srcfile);
	savedatafile.transfer(win.datafile);
	savewlocked.transfer(win.wlocked);
	savemsg.transfer(USER4);
	savereset.transfer(win.reset);
	savevalid.transfer(win.valid);

	storer.transfer(RECORD);
	storeid.transfer(ID);
	storedict.transfer(DICT);
	storemv.transfer(MV);

//TODO:	if (!useactivelist)
//		popselect(0, v69, v70, v71);

	return 1;

}

libraryexit()
