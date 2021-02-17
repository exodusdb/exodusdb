#include <exodus/program.h>
programinit()

#define EXO_NOHTML
#ifdef EXO_NOHTML
	#define xselect select
	#define timedate2 timedate
#else
#include <colrowspan.h>
#include <getsortjs.h>
#include <getmark.h>
#include <convcss.h>
#include <htmllib2.h>
#include <xselect.h>
#include <addunits.h>
#include <elapsedtimetext.h>
#include <sendmail.h>
#include <gethtml.h>
#include <getcss.h>
#include <docmods.h>
#include <timedate2.h>
#endif

#include "printtx.hpp"

//#include <gen_common.h>

var copyright;
var showborder;//num
var headtabperpage;//num
var ulchar;
var nblocks;//num
var blockn;//num
var fromdate;
var fromtime;
var usecols;//num
var decimalchar;
var html;//num
var td0;
var nbsp;
var td;
var tdx;
var th;
var thx;
var tr;
var trx;
var tt;//num
var tdz;
var thcolor;
var tdcolor;
var reportfont;
var printptr;//num
var cssver;//num
var sentencex;
var maxncols;//num
var maxnrecs;
var preselect;//num
var keylist;//num
var crtx;//num
var idsupp;//num
var dblspc;//num
var detsupp;//num
var gtotsupp;//num
var gtotreq;//num
var nobase;//num
var rawtable;//num
var silent;//num
var dictvoc;
dim colname;
dim coldict;
dim mcol;
dim pcol;
dim ccol;
dim scol;
dim icol;
var bheadfn;//num
var coln;//num
var head;
var foot2;
var nbreaks;//num
var breakcolns;
var breakopts;
var pagebreaks;
var headtab;
var hrown;//num
var hcoln;//num
var replacements;
var nreplacements;//num
var topmargin;//num
var totalflag;//num
var breakonflag;//num
var anytotals;//num
var multirowcolhdg;//num
var limits;
var nlimits;//num
var ignorewords;
var emailtoid;
var emailccid;
var emailsubjectid;
var emailto;
var emailcc;
var emailsubject;
var nextemailto;
var nextemailcc;
var nextemailsubject;
var ss;
var wordn;//num
var word;
var filename;
var dictfilename;
var xx;
var printtxmark;
var nextword;
var dictrec;
var limitx;
var lastword;
var title;
var value;
var tcoln;//num
var nn;
var ii;//num
var tt2;
var charx;
var charn;//num
var ncols;//num
var breakn;//num
var underline;
var colul;
var bar;
var tx;
var srcfile;
dim breakcount;
dim breaktotal;
dim breakvalue;
dim oldbreakvalue;
var colhdg;
var coltags;
var style;
var coln2;//num
var vmcount;//num
var thproperties;
var align;
var clientmark;
var posttheadmark;
var headtabcols;
var mode;
var headtabstyle;
var orighead;
var v69;
var v70;
var v71;
var dictid;
var limitn;//num
var recn;//num
var lastid;
var readerr;//num
var fns;
var nfns;
var nmvs;//num
var fnn;//num
var fn;
var limitvals;
var mvx;
var newhead;
var printfilename;
var breakleveln;//num
var leveln;//num
var previousmarklevel;//num
var newmarklevel;//num
var str1;
var str2;
var str3;
var tx1;
var oconvx;
var bodyln;//num
var bottomline;
var lastrecord;//num
var printfile;
var storewordn;
var storecharn;
var storeword;
var storedictrec;
var startcharn;//num
var searchchar;
var letterhead;
var wordexpanded;//num
var breakcoln;
var storetx;
var anycell;//num
var lastblockn;
var underline2;
var cell;//num
var colbreakn;
var bottomline2;
var char1;//num
var char2;//num
var char3;
var body;
var tt3;
var xxerrmsg;
var ownprintfile;//num
var ptx_filenamelen;
var ptx_random;
var pagelns;
var realpagen;//num
var pagen;//num
var newpage;//num
var rfmt;
var foot;
var ntxlns;//num
var nbodylns;//num
var ptx_temp;
var headx;
var newpagetag;
var ptx_css;
var stylennx;//num
var htmltitle;
var head_or_foot;
var footx;
var head1;
var optioncharn;//num
var optionchars;
var optionchar;
var newoptions;
var printtx_ii;//num
var spaceoptionsize;

function main() {
	//
	//c sys
	copyright = "Copyright (C) NEOSYS All Rights Reserved";
	copyright = "";
	showborder = 0;
	headtabperpage = 1;

	if (USERNAME eq "EXODUS") {
		var(SENTENCE).oswrite("NLIST");
	}

//	#include <general_common.h>
	//global all
	//for printtx
	//global html,head,foot,cssver,htmltitle,topmargin,bottomline,tx

	#define interactive not(SYSTEM.a(33))
	ulchar = "-";

	//NLIST keywords and aliases. blank keyword means throwaway word
	/* LIST dict_voc WITH F1 'RLIST' BY F4 BY ID ID-SUPP F4 ID;
	==== LOGIC GROUPING ====;

		(	{
		)	}
		AND;
		OR;

	==== DATA FILTERS ====;

		MATCH	MATCHES, MATCHING;
		NOT	#, <>, ARENT, DOESNT, ISNT, NE, NO, NOT;
		WITH	BECAUSE, IF, INCLUDE, INCLUDING, THAT, WHEN, WHENEVER, WHICH;
		WITH NOT	EXCEPT, EXCUDE, EXCLUDING, UNLESS, WITHOUT,;
		ONLY;
		LIMIT	;

	==== VALUE TESTS ====;
		;
		[	ENDING	;
		[]	CONTAINING, CONTAINS;
		]	STARTING, STARTS;
		GE	>=;
		GT	>, AFTER, GREATER, LATER, OVER	;
		EVERY	EACH;
		EQ	 =, EQUAL, LIKE;
		LE	<=;
		LT	<, BEFORE, LESS, UNDER;
		BETWEEN	;
		FROM	;
		TO;

	==== SEQUENCE/ORDER ====;

	Example:

		BY MARKET_NAME BY CLIENT_NAME;

		BY	BY-EXP;
		BY-DSND	BY-EXP-DSND;

	==== COLUMN MODIFIERS ====;

		CH	COLHEAD;
		JL	JUSTLEN;
		OC	OCONV;
		CS	COL-HDR-SUPP;
		DB	DBL-SPC;
		HS	HDR-SUPP, SUPP;
		IS	ID-SUPP;

	==== TOTALLING ====;

		BREAK-ON	;
		GRAND-TOTAL;
		GTS	GTOT-SUPP;
		AVERAGE	;
		TOTAL;
		DS	DET-SUPP;

	==== TITLING ====;

		FOOTING	;
		HEADING	;

	==== DICTIONARY OVERRIDE ====;

		USING;
		DICT		;

	=== Throwaway words ===;

		A AN ANY ARE DOES FILE FOR HAS IN IS IT ITEMS ITS OF OR PAGE PG SHOW SHOWING THAN THE;

	*/

	//PRINTER OFF;
	//print @AW<30>
	//get.cursor(cursor)

	nblocks = 0;
	blockn = 0;

	if (LISTACTIVE) {
		fromdate = "";
		fromtime = "";
	}else{
		fromdate = var().date();
		fromtime = ostime();
	}

	//use <COL> for hiding non-totalled cols in det-supp (slow)
	usecols = 0;

	if (BASEFMT[2] eq "C") {
		decimalchar = ",";
	}else{
		decimalchar = ".";
	}

	if (VOLUMES) {
		if (SYSTEM.a(2) eq "") {
			perform("GET NEW " ^ SENTENCE);
			stop();
		}
		html = 1;

	//if not arev then dont force html or force output to file
	}else{
		html = SYSTEM.a(2).lcase().substr(-3,3) eq "htm";
	}

	if (html) {
		//td0=crlf:' '
		td0 = "";
		nbsp = "&nbsp;";
		td = td0 ^ "<td>";
		tdx = "</td>";
		th = td0 ^ "<th>";
		thx = "</th>";
		tr = "<tr>";
		trx = "</tr>";
		tt = SYSTEM.a(2);
		tt.swapper(".txt", ".htm");
		SYSTEM.r(2, tt);
	}else{
		td0 = "";
		td = "";
		tdx = " ";
		tdz = "";
		th = "";
		thx = " ";
		tr = "";
		trx = " ";
	}

	//////
	//init1:
	//////

	thcolor = SYSTEM.a(46, 1);
	tdcolor = SYSTEM.a(46, 2);
	reportfont = SYSTEM.a(46, 3);
	//if tdcolor else tdcolor='#FFFFC0'
	//if thcolor else thcolor='#FFFF80'

	//@sentence='LIST 10 SCHEDULES BY VEHICLE_CODE with vehicle_code "kjh kjh" VEHICLE_NAME BREAK-ON VEHICLE_CODE BREAK-ON VEHICLE_CODE TOTAL PRICE (SX)'
	//@sentence='list markets code name'

	printptr = 0;
	cssver = 2;

	sentencex = SENTENCE;
	DICT = "";
	maxncols = 128;
	maxnrecs = "";
	preselect = 0;
	keylist = "";

	crtx = "";
	idsupp = "";
	dblspc = "";
	detsupp = 0;
	if (sentencex.index(" DET-SUPP")) {
		detsupp = 1;
	}
	if (sentencex.index(" DET-SUPP2")) {
		detsupp = 2;
	}
	gtotsupp = "";
	gtotreq = "";
	nobase = "";
	rawtable = 0;
	silent = 0;

	if (not(dictvoc.open("dict_voc", ""))) {
		createfile("dict_voc");
		if (not(dictvoc.open("dict_voc", ""))) {
			call fsmsg();
			stop();
		}
	}

	if (not(tt.read(dictvoc,"@ID"))) {
		var("F|0|TABLE_NAME|S|1||||L|20||VARCHAR").converter("|",FM).write(dictvoc,"TABLE_NAME");
		var("F|0|COLUMN_NAME|S|2||||L|20").converter("|",FM).write(dictvoc,"COLUMN_NAME");
		var("G||TYPE FMC PART HEADING SM CONV JUST LEN MASTER_FLAG|||||||||||||||||||||||||0").converter("|",FM).write(dictvoc,"@CRT");
		var("F|0|Ref.|S|0||||T|20").converter("|",FM).write(dictvoc,"@ID");
		var("F|1|TYPE|S|0||||L|4").converter("|",FM).write(dictvoc,"TYPE");
		var("F|2|FMC|S|0||||R|3").converter("|",FM).write(dictvoc,"FMC");
		var("F|5|PART|S|0||||R|2").converter("|",FM).write(dictvoc,"PART");
		var("F|3|HEADING|S|0||||T|20").converter("|",FM).write(dictvoc,"HEADING");
		var("F|4|SM|S|0||||L|4").converter("|",FM).write(dictvoc,"SM");
		var("F|7|CONV|S|0||||L|9").converter("|",FM).write(dictvoc,"CONV");
		var("F|9|JST|S|0||||L|3").converter("|",FM).write(dictvoc,"JUST");
		var("F|10|LEN|S|0||||R|3||LENGTH").converter("|",FM).write(dictvoc,"LEN");
		var("F|28|MST|S|||BYes,||L|4").converter("|",FM).write(dictvoc,"MASTER_FLAG");
	}

	colname.redim(maxncols);
	coldict.redim(maxncols);
	mcol.redim(maxncols);
	pcol.redim(maxncols);
	ccol.redim(maxncols);
	scol.redim(maxncols);
	icol.redim(maxncols);
	bheadfn = 15;
	coln = 0;
	head = "";
	foot2 = "";
	nbreaks = 0;
	breakcolns = "";
	breakopts = "";
	//pagebreakcolns=''
	pagebreaks = "";
	headtab = "";
	hrown = 1;
	hcoln = 1;
	replacements = "";
	nreplacements = 0;

	topmargin = 0;
	totalflag = 0;
	breakonflag = 0;
	anytotals = 0;
	multirowcolhdg = 0;

	limits = "";
	nlimits = 0;

	ignorewords = "";

	emailtoid = "";
	emailccid = "";
	emailsubjectid = "";
	emailto = "";
	emailcc = "";
	emailsubject = "";
	nextemailto = "";
	nextemailcc = "";
	nextemailsubject = "";

	//initphrase:
	///////////
	ss = "";
	wordn = 0;

///////////
nextphrase:
///////////

	gosub getword();
	if (word eq "") {
		goto x1exit;
	}

phraseinit:
///////////
	if (word.substr(1,4) eq "SORT" or word.substr(1,5) eq "NSORT") {
		ss ^= "SSELECT";

filename:
		gosub getword();
		if (not word) {
			call mssg("FILE NAME IS REQUIRED");
			stop();
		}

		//limit number of records
		if (word.match("^\\d+$")) {
			maxnrecs = word;
			ss ^= " " ^ maxnrecs;
			gosub getword();
		}

		//get the file name
		filename = word;
		if (word eq "DICT") {
			gosub getword();
			filename = "DICT." ^ word;
		}
		if (filename.substr(1,5) eq "DICT.") {
			dictfilename = "VOC";
		}else{
			dictfilename = filename;
		}
		if (not(DICT.open("dict_" ^ dictfilename))) {
			//commented so that list dict_clients gives dict.voc columns
			//crtx = 1;
			DICT = dictvoc;
		}
		ss ^= " " ^ word;

		//exodus custom

		if (html) {

			if (var("PLANS,SCHEDULES,ADS,BOOKING_ORDERS,VEHICLES,RATECARDS").locateusing(",",filename,xx)) {
				printtxmark = "Media Management";
			}

			if (var("JOBS,PRODUCTION_ORDERS,PRODUCTION_INVOICES").locateusing(",",filename,xx)) {
				printtxmark = "Production Management";
			}

			if (var("CHARTS,RECURRING").locateusing(",",filename,xx)) {
				printtxmark = "Financial";
			}

		}
	/*;
			//get any specfic keys
			loop;
			while num(nextword) or nextword[1,1]="'" or nextword[1,1]='"';
				keylist=1;
				gosub getword;
			if word='' then goto exitloop;
				ss:=' ':word;
				repeat;
	exitloop:
	*/

		//get any specfic keys
nextkey:
		if ((nextword.isnum() or (nextword[1] eq "'")) or (nextword[1] eq DQ)) {
			keylist = 1;
			ss ^= " " ^ nextword;
			gosub getword();
			if (nextword.length()) {
				goto nextkey;
			}
		}

	} else if ((word.substr(1,4) eq "LIST" or word.substr(1,5) eq "NLIST") or word eq "XLIST") {
		ss ^= "SELECT";
		goto filename;
		{}

	} else if (word eq "GETLIST") {
		gosub getword();
		getlist(word ^ " (S)");

	} else if (word eq "AND" or word eq "OR") {
		ss ^= " " ^ word;

	} else if (word eq "(" or word eq ")") {
		ss ^= " " ^ word;

	} else if (word eq "BY" or word eq "BY-DSND") {
		ss ^= " " ^ word;
		gosub getword();
		ss ^= " " ^ word;

		//determine if limited nrecs sorted by mv field (which needs preselect)
		if ((maxnrecs and not(preselect)) and DICT) {
			if (dictrec.reado(DICT, word)) {
				preselect = dictrec.a(4)[1] eq "M";
			}
		}

	} else if ((((word eq "WITH NOT") or (word eq "WITH")) or (word eq "WITHOUT")) or (word eq "LIMIT")) {
		ss ^= " " ^ word;

		limitx = word eq "LIMIT";
		if (limitx) {
			nlimits += 1;
		}

		gosub getword();

		//NO/EVERY
		if ((word eq "NOT" or word eq "NO") or word eq "EVERY") {
			ss ^= " " ^ word;
			gosub getword();
		}

		//field or NO
		ss ^= " " ^ word;
		if (limitx) {
			limits.r(1, nlimits, word);
			if (not(dictrec.reado(DICT, word))) {
				call mssg(word ^ " is not a valid dictionary item");
				stop();
			}
			tt = dictrec.a(4).field(".", 1);
			if (tt[1] ne "M") {
				call mssg(word ^ " limit must be a multivalued dict item");
				stop();
			}
			limits.r(4, nlimits, tt);
		}

		//negate next comparision
		if (var("NOT,NE,<>").locateusing(",",nextword,xx)) {
			nextword = "NOT";
			gosub getword();
			ss ^= " " ^ word;
		}

		//comparision
		if (var("MATCH,EQ,,NE,GT,LT,GE,LE,<,>,<=,>=,=,[],[,]").locateusing(",",nextword,xx)) {
	//only EQ works at the moment
			gosub getword();
			ss ^= " " ^ word;
			if (limitx) {
				limits.r(2, nlimits, word);
			}
		}

		//with xx between y and z
		//with xx from y to z
		if (nextword eq "BETWEEN" or nextword eq "FROM") {
			gosub getword();
			ss ^= " " ^ word;
			gosub getword();
			ss ^= " " ^ word;
			gosub getword();
			ss ^= " " ^ word;
			gosub getword();
			ss ^= " " ^ word;

		}else{

			//parameters
			while (true) {
				///BREAK;
				if (not(nextword ne "" and (((nextword.isnum() or (nextword[1] eq DQ)) or (nextword[1] eq "'"))))) break;
				gosub getword();
				ss ^= " " ^ word;
				if (limitx) {
					if ((DQ ^ "'").index(word[1])) {
						if (word[1] eq word[-1]) {
							word = word.substr(2,word.length() - 2);
						}
					}
					if (word eq "") {
						word = "\"\"";
					}
					limits.r(3, nlimits, -1, word);
				}
				{}
			}//loop;

		}

	} else if (word eq "BREAK-ON") {
		tt = coln + 1;
		breakcolns.splicer(1, 0, tt ^ FM);
		breakopts.splicer(1, 0, FM);
		nbreaks += 1;
		breakonflag = 1;

	} else if (word eq "GRAND-TOTAL") {
		//zzz throw away the grand total options for the time being
		gosub getword();
		gtotreq = 1;

	} else if (word eq "NO-BASE") {
		nobase = 1;

	//"DET-SUPP"
	} else if (word eq "DS" or word eq "DET-SUPP") {
		detsupp = 1;

	//"DET-SUPP"
	} else if (word eq "DS2" or word eq "DET-SUPP2") {
		detsupp = 2;

	//"GTOT-SUPP"
	} else if (word eq "GTS" or word eq "GTOT-SUPP") {
		gtotsupp = 1;

	//"MULTIROW-COLHDG"
	} else if (word eq "DS" or word eq "MULTIROW-COLHDG") {
		multirowcolhdg = 1;

	//case dictrec

	} else if (word eq "TOTAL") {
		totalflag = 1;

	} else if (word eq "USING") {
		gosub getword();
		dictfilename = word;
		if (not(DICT.open("dict_" ^ dictfilename))) {
			call fsmsg();
			stop();
		}

	} else if (word eq "HEADINGTABLE") {

		lastword = word;

		//layout in html table in pairs of cells like <td>title:</td><td>value</td>
		//each column number in the command corresponds to a PAIR of columns in html

		//examples
		//in next row of whatever the last column is (1 to start)
		//HEADINGTABLE "Client:" "XYZ"
		//in next row of column 2
		//HEADINGTABLE 2 "Client:" "XYZ"
		//in column 2 row 3
		//HEADINGTABLE 2 3 "Client:" "XYZ"
		//titled from dictionary, value from runtime break
		//like break-on with B1,1 option but doesnt get tested everytime so is faster
		//but assumes that the field is dependent on a field that does have break-on
		//HEADINGTABLE "{CLIENT_NAME}"
		//overidding the dictionary title
		//HEADINGTABLE "Client" "{CLIENT_NAME}"

		if (nextword.match("^\\d+$")) {
			gosub getword();

			//reset to row 1 if column changed
			if (hcoln ne word) {
				hrown = 1;
			}

			hcoln = word;
		}

		if (nextword.match("^\\d+$")) {
			gosub getword();
			hrown = word;
		}

		gosub getquotedword2();

		if ((DQ ^ "'").index(nextword[1])) {
			title = word;
			gosub getquotedword2();
			value = word;
		} else if (word[1] eq "{") {
			title = "";
			value = word;
		} else {
			title = word;
			value = "";
		}

		//automatic labelling with dictionary title
		if (word[1] eq "{") {
			tt = word.substr(2,word.length() - 2);
			replacements.r(-1, tt);
			nreplacements += 1;
			if (not(tt.reado(DICT, tt))) {
				if (not(tt.reado(dictvoc, tt))) {
					call mssg(tt ^ " is not a valid dictionary item");
					stop();
				}
			}
			if (title eq "") {
				title = tt.a(3) ^ nbsp ^ ":";
			}
		}

		tcoln = (hcoln - 1) * 2 + 1;
		//find the next empty row
		while (true) {
			///BREAK;
			if (not(headtab.a(hrown, tcoln) or (headtab.a(hrown, tcoln + 1)))) break;
			hrown += 1;
		}//loop;

		//prevent trailing colon folding onto following line
		if (title.substr(-2,2) eq " :") {
			title.splicer(-2, 2, "&nbsp;:");
		}

		//any existing doubled single quotes are removed to avoid double doublimg
		title.swapper("''", "'");
		//double any single quotes to avoid them being understood as options
		title.swapper("'", "''");

		headtab.r(hrown, tcoln, title.convert(FM ^ VM, "  "));
		headtab.r(hrown, tcoln + 1, value);
				hrown += 1;

	} else if (word eq "HEADING" and dictrec.a(3) ne "HEADING") {

		gosub getquotedword();
		head ^= word;

		//remove page numbering options from headings
		if (html) {
			head.swapper("Page 'P'", "");
			head.swapper("Page 'P", "'");
		}

	} else if (word eq "FOOTING") {
		gosub getquotedword();
		foot2 ^= word;

	//justlen
	} else if (word eq "JL" or word eq "JUSTLEN") {
		if (not coln) {
			call mssg("JUSTLEN/JL must follow a column name");
			stop();
		}
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			gosub getquotedword();
			coldict(coln).r(9, word[1]);
			coldict(coln).r(10, word.substr(3,9999));
			coldict(coln).r(11, word);
		}

	//colhead
	} else if (word eq "CH" or word eq "COLHEAD") {
		if (not coln) {
			call mssg("COLHEAD/CH must follow a column name");
			stop();
		}
		gosub getquotedword();
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			word.converter("|", VM);
			coldict(coln).r(3, word);
		}

	} else if (word eq "OC" or word eq "OCONV") {
		if (not coln) {
			call mssg("OCONV/OC must follow a column name");
			stop();
		}
		gosub getquotedword();
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			if (html) {
				word.swapper("[DATE]", "[DATE,*]");
			}
			coldict(coln).r(7, word);
		}

	} else if (word eq "ID-SUPP" or word eq "IS") {
		idsupp = 1;

	} else if (word eq "DBL-SPC" or word eq "DB") {
		dblspc = 1;

	} else if (word eq "EMAIL_TO") {
		gosub getword();
		emailtoid = word;

	} else if (word eq "EMAIL_CC") {
		gosub getword();
		if ((DQ ^ "'").index(word[1])) {
			emailcc = word.substr(2,word.length() - 2);
			nextemailcc = emailcc;
		}else{
			emailccid = word;
		}

	} else if (word eq "EMAIL_SUBJECT") {
		gosub getword();
		if ((DQ ^ "'").index(word[1])) {
			emailsubject = word.substr(2,word.length() - 2);
			nextemailsubject = emailsubject;
		}else{
			emailsubjectid = word;
		}

	} else if (dictrec) {

		if (var("FSDIA").index(dictrec.a(1))) {

			//pick format dictionary
			//if index('DI',dictrec<1>,1) then
			// call dicti2a(dictrec)
			// end
			//if dictrec<1>='A' then dictrec<1>='F'

			//suppress untotalled columns if doing detsupp2
			if (detsupp eq 2 and (not(totalflag or breakonflag))) {
				if (var("JL,JUSTLEN,CH,COLHEAD,OC,OCONV").locateusing(",",nextword,xx)) {
					gosub getword();
					gosub getword();
				}
				goto dictrecexit;
			}

			coln += 1;
			colname(coln) = word;

			//increase column width if column title needs it
			nn = dictrec.a(3).count(VM) + 1;
			for (ii = 1; ii <= nn; ++ii) {
				tt = dictrec.a(3, ii);
				if (dictrec.a(10) and tt.length() gt dictrec.a(10)) {
					dictrec.r(10, tt.length());
				}
			};//ii;

			dictrec.r(bheadfn, "");

			if (detsupp lt 2) {
				if (not(totalflag or breakonflag)) {
					//tt=' id="BHEAD"'
					//if detsupp then tt:=' style="display:none"'
					if (detsupp) {
						tt = " class=\"BHEAD\"";
					}else{
						tt = " class=\"BHEAD2\"";
					}
					dictrec.r(bheadfn, tt);
				}
			}

			//total required ?
			if (totalflag) {
				totalflag = 0;
				dictrec.r(12, 1);
				anytotals = 1;
			}else{
				dictrec.r(12, 0);
			}

			if (html) {
				tt = dictrec.a(7);
				tt.swapper("[DATE]", "[DATE,*]");
				if (tt eq "[DATE,4]") {
					tt = "[DATE,4*]";
				}
				dictrec.r(7, tt);
				if (tt eq "[DATE,*]") {
					dictrec.r(9, "R");
				}
			}
			coldict(coln) = dictrec;

			//store the format in a convenient place
			tt = "";
			if (not html) {
				tt = coldict(coln).a(9) ^ "#" ^ coldict(coln).a(10);
			}
			coldict(coln).r(11, tt);

			//this could be a break-on column and have break-on options
			//if coln=breakcolns<1> then
			if (breakonflag) {
				coldict(coln).r(13, 1);
				breakonflag = 0;

				if (nextword[1] eq DQ) {
					gosub getword();

					//zzz break  options
					tt = word.index("B");
					if (tt) {

						//suppress columns that appear in the heading
						coldict(coln).r(10, 0);

						//pagebreakcolns<1,-1>=coln

						//determine B99.99 format for row and col
						tt2 = "";
						for (tt = 1; tt <= 9999; ++tt) {
							charx = word[tt];
							///BREAK;
							if (not(charx.length() and var("0123456789,").index(charx))) break;
							tt2 ^= charx;
						};//tt;

						//build table heading table if rown/column givem
						if (tt2) {
							hrown = tt2.field(",", 1);
							if (not(hrown.isnum())) {
								hrown = 1;
							}
							hcoln = tt2.field(",", 2);
							if (not(hcoln.isnum())) {
								hcoln = 1;
							}
							if (not hcoln) {
								hcoln = 1;
							}
							tcoln = (hcoln - 1) * 2 + 1;
							headtab.r(hrown, tcoln, coldict(coln).a(3).convert(VM, " ") ^ nbsp ^ ":");
							headtab.r(hrown, tcoln + 1, "'B" ^ tt2 ^ "'");
							hrown += 1;
						}

						pagebreaks.r(coln, "'" "B" ^ tt2 ^ "'");

					}
					breakopts.r(1, word);
				}
			}

		}

dictrecexit:;

	} else if (word eq "IGNOREWORD") {
		gosub getword();
		ignorewords.r(1, -1, word);

	} else {
		//sys.messages W156
		tt = "|\"%1%\" is an unrecognized word.";
		tt.swapper("%1%", word);
		if (interactive) {
			if (VOLUMES) {
				tt ^= "|Please correct the word by retyping it|or pressing [F4] to edit it.|Press [Esc] to re-start.|";
			}else{
				tt ^= " Replace with? (Enter to cancel):";
			}
		}
		call mssg(tt, "RCE", word, word);
		if (word eq "\x1B") {
			stop();
		}
		gosub getwordexit();
		goto phraseinit;

	}

	///////////
	//phraseexit:
	///////////
	goto nextphrase;

	//////
x1exit:
	//////
	//if no columns selected then try to use default @crt or @lptr group item
	if (not(coln or crtx)) {
		word = "@LPTR";
		if (not(xx.reado(DICT, word))) {
			word = "@CRT";
			if (not(xx.reado(DICT, word))) {
				word = "";
			}
		}
		if (word) {
			crtx = 1;
			sentencex ^= " " ^ word;
			//charn-=2
			charn = sentencex.length() - word.length();
			goto nextphrase;
		}
	}

	ncols = coln;

	//insert the @id column
	if (not idsupp) {

		ncols += 1;

		//move the columns up by one to make space for a new column 1
		for (coln = ncols; coln >= 2; --coln) {
			coldict(coln) = coldict(coln - 1);
			colname(coln) = colname(coln - 1);
		};//coln;

		//set column 1
		colname(1) = "@" "ID";
		if (not(coldict(1).reado(DICT, "@" "ID"))) {
			if (not(coldict(1).reado(dictvoc, "@" "ID"))) {
				coldict(1) = "F" ^ FM ^ FM ^ "Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "L" ^ FM ^ 15;
			}
		}
		if (html) {
			tt = "";
		}else{
			tt = coldict(1).a(9) ^ "#" ^ coldict(1).a(10);
		}
		coldict(1).r(11, tt);

		//increment the list of breaking columns by one as well
		for (breakn = 1; breakn <= nbreaks; ++breakn) {
			breakcolns.r(breakn, breakcolns.a(breakn) + 1);
		};//breakn;

		//and the page break colns
		//for ii=1 to 9999
		// tt=pagebreakcolns<1,ii>
		//while tt
		// pagebreakcolns<1,ii>=tt+1
		// next ii
		if (pagebreaks) {
			pagebreaks = "" ^ FM ^ pagebreaks;
		}

	}

	if (breakcolns[-1] eq FM) {
		breakcolns.splicer(-1, 1, "");
	}
	if (breakopts[-1] eq FM) {
		breakopts.splicer(-1, 1, "");
	}

	//make underline and column title underline
	if (not html) {
		underline = "";
		colul = "";
		for (coln = 1; coln <= ncols; ++coln) {
			if (coldict(coln).a(10)) {
				if (coldict(coln).a(12)) {
					tt = ulchar;
				}else{
					tt = " ";
				}
				underline ^= tt.str(coldict(coln).a(10)) ^ " ";
				colul ^= ulchar.str(coldict(coln).a(10)) ^ " ";
			}
		};//coln;
		bar = ulchar.str(colul.length() - 1);
	}

	//////
	//init2:
	//////

	tx = "";

	if (not(srcfile.open(filename, ""))) {
		call fsmsg();
		stop();
	}

	breakcount.redim(nbreaks + 1);
	breaktotal.redim(maxncols, nbreaks + 1);
	breakvalue.redim(maxncols);
	oldbreakvalue.redim(maxncols);

	breakvalue="";
	oldbreakvalue="";
	breakcount="";
	breaktotal="";

	//build the column headings
	colhdg = "";
	coltags = "";
	style = "";
	coln2 = 0;
	vmcount = 0;
	thproperties = "";

	for (coln = 1; coln <= ncols; ++coln) {

		//suppress drilldown if no totals or breakdown
		if (not(anytotals) or not(nbreaks)) {
			coldict(coln).r(bheadfn, "");
		}

		if (coldict(coln).a(10)) {
			if (html) {
				coln2 += 1;

				//suppressing non-totalled columns may not work well with multi-row colhdg
				tt = coldict(coln).a(bheadfn);

				//tt2=index(tt,'style="',1)
				//tt3='background-color:':thcolor
				//if tt2 then
				// tt[tt2+7,0]=tt3:';'
				//end else
				// tt:=' style=':quote(tt3)
				// end

				thproperties.r(coln2, tt);

				//without the MULTIROW_COLHDG keyword,
				//vm indicates folding, \\ indicates rows in column headings
				tt = coldict(coln).a(3);
				if (not multirowcolhdg) {
					tt.swapper(VM, "<br />");
				}
				tt.swapper("\\\\", VM);

				colhdg.r(coln2, tt);
				if (tt.count(VM) gt vmcount) {
					vmcount = tt.count(VM);
				}

				//swap vm with '<br />' in colhdg

				coltags.r(-1, " <col");
				align = coldict(coln).a(9);
				if (align eq "R") {
					//if index(coldict(coln)<7>,'[NUMBER',1) then
					// *http://www.w3.org/TR/html401/struct/tables.html#adef-align-TD
					// coltags:=' align="char" char="':decimalchar:'"'
					//end else
						//coltags:=' align="right"'
					align = "right";
					coltags ^= " style=\"text-align:right\"";
					// end
				} else if (align eq "T") {
					//coltags:=' align="left"'
					align = "left";
					coltags ^= " style=\"text-align:left\"";
				} else {
					align = "center";
				}
				if (usecols) {
					coltags ^= coldict(coln).a(bheadfn);
				}
				coltags ^= " />";

				//nth child style column justification in case <col> doesnt work like on FF
				if (align) {
					//works per table if the table is assigned a class (.maintable) here
					style ^= "table.exodustable td:nth-child(" ^ coln2 ^ "){text-align:" ^ align ^ "}" "\r\n";
				}

			}else{
				for (ii = 1; ii <= 9; ++ii) {
					colhdg.r(ii, colhdg.a(ii) ^ oconv(coldict(coln).a(3, ii), coldict(coln).a(11)) ^ " ");
				};//ii;
			}
		}
	};//coln;

	if (style) {
		style = "<style type=\"text/css\">" "\r\n" ^ style ^ "</style>" "\r\n";
	}

	//convert to html with colspan/rowspan where necessary and (Base) as currcode
	//thproperties='style="background-color:':thcolor:'"'
#ifndef EXO_NOHTML
	if (html) {
		call colrowspan(colhdg, thproperties, nobase);
	}
#endif
	//trim off blank lines (due to the 9 above)
	if (html) {
#ifndef EXO_NOHTML
		call getsortjs(tt);

		if (not rawtable) {
			call getmark("CLIENT", html, clientmark);
			tt ^= clientmark ^ "\r\n";
			//tt:='<span style="font-size:66%"><small>':clientmark:'</small></span>':crlf
		}

		//tt:='<table border="1" cellspacing="0" cellpadding="2"'
		//tt:=' align="center" '
		//tt:=' class="maintable"'

		tt ^= "<table class=\"exodustable\"";
		//cellspacing is only required up to IE7 (or border-collapse)
		tt ^= " cellspacing=\"0\"";
		tt ^= " style=\"font-size:66%";
		tt ^= ";page-break-after:avoid";
		//tt:=';background-color:':tdcolor
		tt ^= "\">";
		tt ^= FM ^ "<colgroup>" ^ coltags ^ "</colgroup>";
		//<thead> may be hardcoded elsewhere for page heading
		//!!!if you change it here, search and change it there too

		tt ^= FM ^ "<thead style=\"cursor:pointer\" onclick=\"sorttable(event)\">";
		posttheadmark = "<postthead/>";
		if (headtab) {
			tt ^= posttheadmark;
		}
		//tt:=fm:coltags

		tt ^= colhdg ^ FM ^ "</thead>";
		tt ^= FM ^ "<tbody>";
		tt.transfer(colhdg);

		//allow for single quotes
		colhdg.swapper("'", "''");
#endif
	}else{
		while (true) {
			///BREAK;
			if (not(colhdg and ((" " ^ FM).index(colhdg[-1])))) break;
			colhdg.splicer(-1, 1, "");
		}//loop;
	}

	//heading options

	//if head='' then head="Page 'P' ":space(50):" 'T'"
	if (head eq "" and not(rawtable)) {
		head = filename ^ var(10).space() ^ " 'T'";
	}

	if (html) {
		head.swapper(FM, "<br />");
		//div to make header as wide as body
		//the report title
		if (head) {
			head = "<h2{%20}style=\"margin:0px;text-align:center;padding-top:0px\">" ^ head ^ "</h2>";
		}

		//supertable to ensure any heading is as wide as the body of the report
		if (showborder) {
			head.r(-1, "<table{%20}border=1{%20}style=\"border-width:3px\"{%20}cellpadding=0{%20}cellspacing=0{%20}align=center><tr><td>");
		}else{
			//head:='<table{%20}cellpadding=0{%20}cellspacing=0{%20}align=center><tr><td>'
			head.r(-1, "<table{%20}align=center><tr><td>");
		}
	}

	//footing options

	if (headtab) {
		tt = "<table id=\"headtab0\" width=100% align=center cellpadding=3>";

		//older MSIE <col> styling
		headtabcols = " <col style=\"text-align:left\"/>" ^ VM ^ "<col style=\"text-align: left;font-weight:bold\"/>";
		//allow 8 max pair of headtab columns
		headtabcols = (headtabcols ^ VM).str(8);
		headtabcols.splicer(-1, 1, "");

		tt ^= "<colgroup>" "\r\n" ^ headtabcols.swap(VM, "\r\n") ^ "</colgroup>";
#ifndef EXO_NOHTML
		//style columns where '<col>' not supported.
		call convcss(mode, "headtab0", headtabcols, headtabstyle);
		style ^= "\r\n" ^ headtabstyle;
		//tt[1,0]=headtabstyle:crlf

		tt ^= "<colgroup>" "\r\n" ^ headtabcols.swap(VM, "\r\n") ^ "</colgroup>";

		//tt:=crlf:'</THEAD>':crlf:'<TBODY>'
		tt ^= "\r\n" "<TBODY>";
		call htmllib2("TABLE.MAKE", headtab, tt, "");
#endif
		headtab.swapper("</TR>", "</TR>" "\r\n");
		if (headtabperpage) {
			colhdg.swapper(posttheadmark, tr ^ td0 ^ "<th style=\"background-color:white\" colspan=" ^ ncols ^ ">" ^ headtab ^ thx ^ trx);
		}else{
			headtab.swapper(posttheadmark, "");
			head ^= FM ^ headtab ^ FM;
		}
	}

	if (dblspc) {
		head ^= FM;
	}
	if (not html) {
		head ^= FM ^ colul;
	}
	head ^= FM ^ colhdg;
	if (not html) {
		head ^= FM ^ colul;
	}
	if (dblspc) {
		head ^= FM;
	}

	head.splicer(1, 0, FM);

	orighead = head;

	//work out assoc mv fns for limits
	if (nlimits) {
		call pushselect(0, v69, v70, v71);
		select(DICT);
nextdict:
		if (readnext(dictid)) {
			if (not(dictrec.reado(DICT, dictid))) {
				goto nextdict;
			}
			if (dictrec.a(1) ne "F") {
				goto nextdict;
			}
			for (limitn = 1; limitn <= nlimits; ++limitn) {
				if (dictrec.a(4).field(".", 1) eq limits.a(4, limitn)) {
					tt = dictrec.a(2);
					if (tt) {
						if (not(limits.a(5, limitn).locateusing(SVM,tt,xx))) {
							limits.r(5, limitn, -1, dictrec.a(2));
						}
					}
				}
			};//limitn;
			goto nextdict;
		}
		call popselect(0, v69, v70, v71);
	}

	////////
	//initrec:
	////////
	if (ss.count(" ") gt 2 or keylist) {

		//preselect if sselect is by any mv fields since that ignores maxnrecs
		if (not(LISTACTIVE)) {
			if (preselect) {
				call xselect(ss.field(" ", 1, 3) ^ " (SR)");
			}
			maxnrecs = "";
		}

		//call mssg('Selecting records, please wait.||(Press Esc or F10 to interrupt)','UB',buffer,'')
		call xselect(ss ^ " (SR)");
		//call mssg('','DB',buffer,'')

		if (not LISTACTIVE) {
			//the words "No record" is hardcoded in autorun and maybe elsewhere
			call mssg("No records found");
			stop();
		}

		if (not LISTACTIVE) {
			if (html) {
				tx ^= "</tbody></table>";
			}
			tx ^= "No records listed";
			gosub printtx();
			goto x2bexit;
		}

	}else{
		if (not LISTACTIVE) {
			//select(srcfile);
			select("SELECT " ^ srcfile ^ " (SR)");
		}
	}
	recn = "";
	RECCOUNT = 0;

	//cmd='LIST ':filename:' ':matunparse(colnames)
	//convert fm to ' ' in cmd
	//call perf(cmd)

	lastid = "%NONE%";

////////
nextrec:
////////
	if (esctoexit()) {
		tx = "";
		if (html) {
			tx ^= "</tbody></table>";
		}
		tx ^= "*** incomplete - interrupted ***";
		gosub printtx();
		clearselect();
		goto x2bexit;
	}

	//limit number of records
	if (maxnrecs) {
		if (recn ge maxnrecs) {
			clearselect();
		}
	}

	//readnext key
	FILEERRORMODE = 1;
	FILEERROR = "";

	//if (not(readnext(ID, MV))) {
	if (not(readnext(RECORD, ID, MV))) {
    		FILEERRORMODE = 0;
		if (STATUS) {
			tx = "*** Fatal Error " ^ FILEERROR.a(1) ^ " reading record " ^ ID ^ " ***";
			gosub printtx();
			stop();
		}
		if (FILEERROR.a(1) eq 421) {
			tx = "Operation aborted by user.";
			gosub printtx();
			stop();
		}
		if (FILEERROR and FILEERROR.a(1) ne 111) {
			tx = "*** Error " ^ FILEERROR.a(1) ^ " reading record " ^ ID ^ " ***";
			gosub printtx();
			readerr += 1;
			stop();
		}
		goto x2exit;
	}

	if (ID[1] eq "%") {
		goto nextrec;
	}

	//skip if record appears twice to fix a bug in indexing
	if (not(MV)) {
		//watch out that 200 is equal to 200.0 etc and would be skipped!!!
		if (ID eq lastid) {
			if (ID.length() eq lastid.length()) {
				goto nextrec;
			}
		}
	}
	lastid = ID;

	//if (not(RECORD.reado(srcfile, ID))) {
	//	goto nextrec;
	//}
	//RECORD.errputl("RECORD=");
    if (not(RECORD.length()) and not(RECORD.read(srcfile, ID))) {
		goto nextrec;
	}

	//designed to filter multivalues which are not selected properly
	//unless sorted "by"
	if (limits) {

		//for each limit pass through record deleting all unwanted multivalues
		for (limitn = 1; limitn <= nlimits; ++limitn) {

			//find maximum mv number for the associated group of fns
			fns = limits.a(5, limitn);
			nfns = fns.count(SVM) + (SVM ne "");
			nmvs = 0;
			for (fnn = 1; fnn <= nfns; ++fnn) {
				fn = fns.a(1, 1, fnn);
				tt = RECORD.a(fn);
				if (tt.length()) {
					tt = tt.count(VM) + 1;
					if (tt gt nmvs) {
						nmvs = tt;
					}
				}
			};//fnn;

			limitvals = calculate(limits.a(1, limitn));
			for (mvx = nmvs; mvx >= 1; --mvx) {
				tt = limitvals.a(1, mvx);
				if (tt eq "") {
					tt = "\"\"";
				}
				//locate tt in (limits<3,limitn>)<1,1> using sm setting xx else
				if (not(limits.a(3, limitn).locateusing(SVM,tt,xx))) {
					for (fnn = 1; fnn <= nfns; ++fnn) {
						RECORD.remover(fns.a(1, 1, fnn), mvx);
					};//fnn;
				}
			};//mvx;
		};//limitn;

	}

	recn += 1;
	RECCOUNT += 1;

	////////
	//recinit:
	////////

	if (recn eq 1) {
		head.transfer(newhead);
		gosub newheadreplacements();
		newhead.transfer(head);
	}

	//if interactive then print @AW<30>:@(36,@CRTHIGH/2):
	if (not(silent) and not(printfilename.unassigned())) {
		//put.cursor(cursor)
		if (printfilename) {
			print(var().at(0), var().at(-4), recn, ". ", ID, " ", MV);
		}
	}

	//if det-supp2 then zero width initialiser columns" will not be called
	//unless you put TOTAL dictid
	//get the data from the record into an array of columns

	for (coln = 1; coln <= ncols; ++coln) {
	//  @mv=0

		if (coldict(coln).a(9) eq "T" and not(html)) {

			mcol(coln) = oconv(calculate(colname(coln)), coldict(coln).a(11));

		}else{

			mcol(coln) = calculate(colname(coln));

			//swap '<' with '&lt;' in m.col(coln)
			//swap '>' with '&gt;' in m.col(coln)
			if (html) {
				mcol(coln).swapper(TM, "<br />");
			}
		}

		//call note(' @id/coln/m.col/coldictx<11>=':@id:'/':coln:'/':m.col(coln):'/':coldict(coln)<11>)
		//call note(' @id/@record<':(coldict(coln)<2>):'>=':@id:'/':(@record<coldict(coln)<2>))

		pcol(coln) = 1;
		ccol(coln) = 7;
		scol(coln) = mcol(coln);
	};//coln;

	//break subtotals
	//detect most major level to break
	if (recn eq 1) {
		//print breaks will not actually print before the first record
		// but it needs to set the various break values
		breakleveln = nbreaks;
	}else{
		for (leveln = nbreaks; leveln >= 1; --leveln) {
			coln = breakcolns.a(leveln);
			///BREAK;
			if (scol(coln) ne breakvalue(coln)) break;
		};//leveln;
		breakleveln = leveln;
	}

	gosub printbreaks();

	oldbreakvalue=breakvalue;

	previousmarklevel = 0;

////////
recexit:
////////

	//remove appropriate value from multi-valued column(s)
	newmarklevel = 0;
	for (coln = 1; coln <= ncols; ++coln) {
		if (ccol(coln) ge previousmarklevel) {
			icol(coln)=mcol(coln).substr2(pcol(coln), ccol(coln));
			scol(coln) = icol(coln);
		}
		if (ccol(coln) gt newmarklevel) {
			newmarklevel = ccol(coln);
		}
	};//coln;

	//break totals - add at the bottom level (1)
	for (coln = 1; coln <= ncols; ++coln) {
		//only totalled columns
		if (coldict(coln).a(12)) {
			if (icol(coln)) {
				if (html) {
#ifndef EXO_NOHTML
					//breaktotal(coln,1)+=i.col(coln)
					call addunits(icol(coln), breaktotal(coln, 1), VM);
#endif
				}else{
					if (breaktotal(coln, 1).isnum() and icol(coln).isnum()) {
						breaktotal(coln, 1) += icol(coln);
					}else{
						if (colname(coln) eq "DATEGRID") {
							str1 = icol(coln);
							str2 = breaktotal(coln, 1);
							gosub addstr();
							breaktotal(coln, 1) = str3;
						}
					}
				}
			}
			breakcount(1) += 1;
			icol(coln) = "";
		}
	};//coln;

	if (detsupp lt 2) {

		if (anytotals and not(blockn)) {
			nblocks += 1;
			blockn = nblocks;
			//tx:='<span style="display:none" id="B':blockn:'">'
			tx ^= FM;
		}

		//print one row of text
		tx1 = "";
		if (html) {

				/*;
				//sadly cannot indicate border on tr element
				begin case;
				case new.mark.level;
					if previous.mark.level then;
						//no top or bottom border
						tdz=td0:'<td class="nx">';
					end else;
						//no bottom border
						tdz=td0:'<td class="nb">';
						end;
				case if previous.mark.level;
					//no top border
					tdz=td0:'<td class="nt">';
				case 1;
					//normal top and bottom border
					tdz=td;
					end case;
				*/
			tdz = td;

			tx1 ^= "<tr";
			if (blockn) {
				tx1 ^= " id=\"B" ^ blockn ^ DQ;
				tx1 ^= " class=\"B" ^ blockn ^ DQ;
				//tx1:=' name="B':blockn:'"'
				//clicking expanded det-supped details collapses it
				if (detsupp) {
					tx1 ^= " style=\"cursor:pointer";
					if (detsupp eq 1) {
						tx1 ^= ";display:none";
					}
					tx1 ^= DQ;
					tx1 ^= " onclick=\"toggle(" "'" "B" ^ blockn ^ "'" ")\"";
				}
			}
			tx1 ^= ">";
		}
		for (coln = 1; coln <= ncols; ++coln) {
			tt = scol(coln);
			oconvx = coldict(coln).a(7);
			if (oconvx) {
				tt = oconv(tt, oconvx);
				if (html) {
					if (tt[1] eq "-") {
						if (oconvx.substr(1,7) eq "[NUMBER") {
							tt = "<nobr>" ^ tt ^ "</nobr>";
						}
					}
				}
			}
			if (coldict(coln).a(10)) {
				if (not html) {
					tt = oconv(tt, coldict(coln).a(11));
				}
				if (tt eq "") {
					//tt=nbsp
				}else{
					tt.swapper("\r\n", "<br />");
				}

				//colored cells starting with ESC
				if (tt[1] eq "\x1B") {
					if (tt.substr(1,2) eq ("\x1B" "\x1B")) {
						tt = tt.field(" ", 2, 999999);
						if (tt.length()) {
							tx1 ^= td ^ "<nobr>" ^ tt ^ "</nobr>" ^ tdx;
						}else{
							//tx1:=td:nbsp:tdx
							tx1 ^= td ^ tdx;
						}
					}else{
						//tx1:=td0:'<td bgcolor=':field(tt,' ',1)[2,9999]:'>'
						//TODO do with class? to save document space?
						tx1 ^= td0 ^ "<td style=\"background-color:" ^ tt.field(" ", 1).substr(2,9999) ^ "\">";
						tt = tt.field(" ", 2, 999999);
						if (tt.length()) {
							tx1 ^= tt ^ tdx;
						}else{
							//tx1:=nbsp:tdx
							tx1 ^= tdx;
						}
					}

				}else{
					if (tt.length()) {
						tx1 ^= tdz ^ tt ^ tdx;
					}else{
						//tx1:=tdz:nbsp:tdx
						tx1 ^= tdz ^ tdx;
					}
				}

			}
		};//coln;

		//swap '<td' with '<th' in tx1
		//swap '<\td' with '<\th' in tx1
		tx ^= tx1;

		if (html) {
			//swap '<td></td>' with '<td>':nbsp:'</td>' in tx
			tx ^= "</tr>";
		}

		gosub printtx();

		//folding text or multivalued lines
		if (newmarklevel) {
			for (coln = 1; coln <= ncols; ++coln) {
				scol(coln) = "";
			};//coln;
			previousmarklevel = newmarklevel;
			goto recexit;
		}

		//double space
		if (dblspc) {
			gosub printtx();
		}

	}

	goto nextrec;

	//////
x2exit:
	//////

	//print the closing subtotals
	breakleveln = nbreaks;
	//if not(gtotsupp) and (not(pagebreakcolns) or gtotreq) then
	if (not(gtotsupp) and ((not(pagebreaks) or gtotreq))) {
		breakleveln += 1;
	}
	gosub printbreaks();

	bodyln = 1;

	if (html and not(bottomline.unassigned())) {
		tx.r(-1, FM ^ bottomline ^ FM);
	}
	//tx:=bottomline:fm:fm

	//print number of records and elapsed time
	if (not rawtable) {

		//this results in duplication and is page-wide instead of report-wide so remove
		//but will this cause problems in some reports?
		//if recn and foot2 then tx:=fm:foot2

		if (pagebreaks eq "" and headtab eq "") {
			if (html) {
				tx ^= "<p style=\"text-align:center\">";
			}
	//records
			tt = recn + 0;
			tx ^= tt ^ " record";
			if (recn ne 1) {
				tx ^= "s";
			}

			if (fromdate) {
				tx ^= ", " ^ elapsedtimetext(fromdate, fromtime) ^ ".";
			}
			if (html) {
				tx ^= "</p>";
			}
		}

	}

	if (html) {

		if (not detsupp) {
			tx.r(-1, "<script type=\"text/javascript\">" ^ FM ^ " togglendisplayed=" ^ nblocks ^ FM ^ "</script>");
		}

		tx.r(-1, "<script type=\"text/javascript\">" ^ FM);
		tx ^= "function nwin(key,url,readonly) {";
		tx ^= "gwindowopenparameters={};";
		tx ^= "if (readonly) gwindowopenparameters.readonlymode=true;";
		tx ^= "gwindowopenparameters.key=key;";
		tx ^= "glogincode=\"" ^ SYSTEM.a(17) ^ "*" ^ USERNAME ^ "*\";";
		//tx:='window.open(url)}'
		//similar code in NLIST and LEDGER2
		tx ^= "var vhtm=window.opener.location.toString().split(\"/\");";
		tx ^= "vhtm[vhtm.length-1]=url;" "\r\n";
		//tx:='alert(vhtm.join("/"));':crlf
		tx ^= "window.open(vhtm.join(\"/\"));" "\r\n";
		tx ^= "}";
		tx ^= FM ^ "</script>";
	}

x2bexit:

	//if html then tx<-1>='</div></body></html>'
	//close supertable div and html
	if (html) {
		tx.r(-1, "</td></tr></table>");
		tx.r(-1, "</div></body></html>");
	}

	gosub printtx();

	lastrecord = 1;
	gosub emailing();

	printfile.osclose();

	stop();

	return "";
}

subroutine getquotedword() {
	lastword = word;
	gosub getquotedword2();
	return;
}

subroutine getquotedword2() {
	gosub getword();
	if (((DQ ^ "'").index(word[1])) and (word[1] eq word[-1])) {
		word.splicer(1, 1, "");
		word.splicer(-1, 1, "");
	}else{
		call mssg(lastword ^ " must be followed by a quoted phrase");
	}
	return;
}

subroutine getword() {
	{}

getnextword:
////////////
	gosub getword2();

	if (word.length()) {
		if (ignorewords.a(1).locate(word,xx)) {
			goto getnextword;
		}
	}

	if (word eq "") {
		nextword = "";
	}else{
		storewordn = wordn;
		storecharn = charn;
		storeword = word;
		storedictrec = dictrec;
		gosub getword2();
		nextword = word;
		word = storeword;
		dictrec = storedictrec;
		wordn = storewordn;
		charn = storecharn;
	}

	//!** call note(quote(word):' ':quote(nextword))

	return;
}

subroutine getword2() {
	{}

getword2b:
	//////////

	word = "";
	wordn += 1;

	//initialise pointer to zero (pointing before the first char which is 1)
	//nb 1 based character indexing! not zero based!
	if (wordn eq 1) {
		charn = 0;
	}

	//charn is always left pointing to the character BEFORE the next word
	charn += 1;

	//skip spaces
	while (true) {
		///BREAK;
		if (not(sentencex[charn] eq " ")) break;
		charn += 1;
		if (charn gt sentencex.length()) {
			return;
		}
	}//loop;

	//if next word starts with " or ' then scan for the same closing
	//otherwise scan up to the next space char
	startcharn = charn;
	charx = sentencex[charn];
	if (("'" ^ DQ).index(charx)) {
		searchchar = charx;
	}else{
		searchchar = " ";
	}
	word ^= charx;

	//build up the word character by character until the closing char is found
	//closing character (" ' or space)
	while (true) {
		charn += 1;
		charx = sentencex[charn];
		///BREAK;
		if (not(charx ne "" and charx ne searchchar)) break;
		word ^= charx;
	}//loop;

	//if scanned for " or ' then add it to the word
	if (searchchar ne " ") {
		word ^= searchchar;
		charn += 1;

	//otherwise
	}else{
		word.ucaser();
		word = word.trimb().trimf();
	}

	//get options and skip to next word
	if (((word[1] eq "(") and (word[-1] eq ")")) or (((word[1] eq "{") and (word[-1] eq "}")))) {
		tt = word;
		//option (N) no letterhead
		if (tt.index("N")) {
			letterhead = "";
		}
		if (tt.index("NN")) {
			rawtable = 1;
			bottomline = "";
		}
		if (tt.index("S")) {
			silent = 1;
		}
		if (tt.index("T")) {
			html = 0;
		}
		goto getword2b;
	}

	gosub getwordexit();
	if (wordexpanded) {
		goto getword2b;
	}

	return;
}

subroutine getwordexit() {

	wordexpanded = 0;

	//standardise
	if (DICT eq "") {
		goto dictvoc;
	}
	if (not(html) and word.substr(-5,5) == "_LINK")
		word.splicer(-5,5,"");
	if (dictrec.reado(DICT, word)) {
maindict:
		if (dictrec.a(1) eq "G") {
			tt = dictrec.a(3);
			tt.converter(VM, " ");
			sentencex.splicer(startcharn, word.length(), tt);
			charn = startcharn - 1;
			wordn -= 1;
			wordexpanded = 1;
			return;
		}
	}else{
		if (dictrec.reado(DICT, word.ucase())) {
			goto maindict;
		}
dictvoc:
		dictrec = "";
		if (dictrec.reado(dictvoc, word)) {
gotdictvoc:
			if (dictrec.a(1) eq "RLIST") {
				if (dictrec.a(4)) {
					word = dictrec.a(4);
				}
				dictrec = "";
			}
		}else{
			if (dictrec.reado(dictvoc, word.ucase())) {
				goto gotdictvoc;
			}
			dictrec = "";
		}
	}
	dictrec.converter("|", VM);

	if (word eq "=") {
		word = "EQ";
	}
	if (word eq "EQUAL") {
		word = "EQ";
	}
	if (word eq "<>") {
		word = "NE";
	}
	if (word eq ">") {
		word = "GT";
	}
	if (word eq "<") {
		word = "LT";
	}
	if (word eq ">=") {
		word = "GE";
	}
	if (word eq "<=") {
		word = "LE";
	}
	if (word eq "CONTAINING") {
		word = "[]";
	}
	if (word eq "ENDING") {
		word = "[";
	}
	if (word eq "STARTING") {
		word = "]";
	}

	word.swapper("%DQUOTE%", DQ);
	word.swapper("%SQUOTE%", "'");

	return;
}

subroutine printbreaks() {

	if (not breakleveln) {
		return;
	}

	newhead = "";

	//print breaks from minor level (1) up to required level
	//required level can be nbreaks+1 (ie the grand total)
	//for leveln=1 to breakleveln
	//for leveln=breakleveln to 1 step -1

	for (leveln = 1; leveln <= breakleveln; ++leveln) {
		breakcoln = breakcolns.a(leveln);

		storetx = tx;
		anycell = 0;
		//output any prior tx is this level is suppressed
		//if index(breakopts<leveln>,'X',1) and tx then
		// if recn>1 then
		//  gosub printtx
		//  end
		// end

		lastblockn = blockn;
		blockn = 0;
		if (detsupp) {
			if (tx and (tx[-1] ne FM)) {
				tx ^= FM;
			}
			if (leveln gt 1 and not(html)) {
				tx ^= underline ^ FM;
			}
		}else{
			if (not html) {
				//underline2=if breakleveln>=nbreaks then bar else underline
				//WARNING TODO: check ternary op following;
				underline2 = leveln eq 1 ? underline : bar;
				if (not((tx.substr(-2,2)).index(ulchar))) {
					if (tx[-1] ne FM) {
						tx ^= FM;
					}
					tx ^= underline2;
				}
			}
			tx ^= FM;
		}

		//print one row of totals
		if (html) {
			if (gtotreq) {
				if (leveln gt nbreaks) {
					tx ^= "</tbody><tbody>";
				}
			}
			tx ^= "<tr";
			if (lastblockn) {
				tx ^= " style=\"cursor:pointer\" onclick=\"toggle(" "'" "B" ^ lastblockn ^ "'" ")\"";
			}
			//if detsupp<2 or (nbreaks>1 and leveln>1) then tx:=' style="font-weight:bold"'
			tx ^= ">";
		}
		for (coln = 1; coln <= ncols; ++coln) {

			//total column
			if (coldict(coln).a(12)) {
				cell = breaktotal(coln, leveln);

				//add into the higher level
				if (leveln le nbreaks) {

					if (cell) {
						if (html) {
#ifndef EXO_NOHTML
							//breaktotal(coln,leveln+1)+=cell
							call addunits(cell, breaktotal(coln, leveln + 1), VM);
#endif
						}else{
							if (((breaktotal(coln, leveln + 1)).isnum()) and cell.isnum()) {
								breaktotal(coln, leveln + 1) += cell;
							}else{
								str1 = cell;
								str2 = breaktotal(coln, leveln + 1);
								gosub addstr();
								breaktotal(coln, leveln + 1) = str3;
							}
						}
					}

					breakcount(leveln + 1) += breakcount(leveln);
				}

				//format it
				oconvx = coldict(coln).a(7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				if (html) {
					cell.swapper(VM, "<br />");
				}

				//if html and cell='' then cell=nbsp

				//and clear it
				breaktotal(coln, leveln) = "";

			//break column
			} else if (coln eq breakcoln) {

				//print the old break value
				cell = breakvalue(coln);
				oconvx = coldict(coln).a(7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				//store the new break value
				breakvalue(coln) = scol(coln);

				//if coln=pagebreakcoln then
				if (pagebreaks.a(coln)) {

					//rebuild the heading
					if (newhead eq "") {
						newhead = orighead;
						gosub newheadreplacements();
					}

					//get the page break data
					tt = scol(coln);

					if (oconvx) {
						tt = oconv(tt, oconvx);
					}
					//ensure single quotes in data dont screw up the html
					tt.swapper("'", "''");
					//if tt='' and html then tt=nbsp

					//insert the page break data
					//swap "'B'" with tt in newhead
					newhead.swapper(pagebreaks.a(coln), tt);

				}

				if (detsupp lt 2 and not(anytotals)) {
					//cell=nbsp
					cell = "";
				}

			//other columns are blank
			} else {
				cell = "";

				//if 1 or detsupp<2 then
					//cell=oldbreakvalue(coln)
				cell = oconv(oldbreakvalue(coln), coldict(coln).a(7));
				if (breakcolns.locateusing(FM,coln,colbreakn)) {
					if (colbreakn lt leveln) {
						cell = "Total";
					}
				}
				// end

			}

			if (coldict(coln).a(10)) {
				if (not(html)) {
					cell=cell.substr(1,coldict(coln).a(10));
					cell=oconv(cell,coldict(coln).a(11));
				} else {
					tx ^= td0 ^ "<th";
					if (not usecols) {
						tx ^= coldict(coln).a(bheadfn);
					}
					if (coldict(coln).a(9) eq "R") {
						tx ^= " style=\"text-align:right\"";
					}
					tx ^= ">";
				}

				tx ^= cell ^ thx;
				//if len(cell) then if cell<>nbsp then anycell=1
				if (cell.length()) {
					anycell = 1;
				}
			}

		};//coln;

	//breakrowexit:
		if (html) {
			tx ^= "</tr>";
		}

		if (detsupp lt 2) {
			//if leveln>1 and not(html) then tx:=fm:underline
		}else{
			if (not html) {
				tx ^= FM ^ underline2;
			}
		}

		//option to suppress the current level
		//or if this is the first record cannot be any totals before it.
		if ((not(anycell) or breakopts.a(leveln).index("X")) or recn eq 1) {
			tx = storetx;
		}

	};//leveln;

	//force new page and new heading
	//if newhead and detsupp<2 then
	//if newhead then
	// head=newhead
	// bodyln=999
	// end

	//if recn>1 then
	//if only one record found then avoid skipping printing totals for det-supp
	if (detsupp lt 2 or recn gt 1) {

		if (not html) {
			if (not anytotals) {
				tx = bar;
			}
		}

		//ensure </td></tr></table> gets printed instead endless nesting every table
		//resulting in only first part of any long report being shown in browser
		//if newhead and html and tx then
		if ((newhead and html) and printptr) {

			gosub newheadreplacements();

			//tx:='</tbody></table>'

			//take over the bottomline so that we can print footer after it
			if (bottomline.unassigned()) {
				bottomline = "";
			}
			bottomline.transfer(bottomline2);

			tx.r(-1, bottomline2);

			if (foot2) {
				tx.r(-1, foot2);
			}

			//close supertable
			tx.r(-1, "</td></tr></table>");

		}
		if (tx) {

			gosub printtx();

		}
	}else{
		tx = "";
	}

	//force new page and new heading
	//if newhead and detsupp<2 then
	if (newhead) {

		lastrecord = 0;
		gosub emailing();

		head = newhead;
		bodyln = 999999;
	}

	return;
}

subroutine addstr() {

	str3 = str2;
	if (str3.length() lt str1.length()) {
		str3 ^= (str1.length() - str2.length()).space();
	}
	for (ii = 1; ii <= str1.length(); ++ii) {
		char1 = (str1[ii]).trim();
		if (char1 ne "") {
			char2 = str3[ii];
			if (char2 eq " ") {
				str3.splicer(ii, 1, char1);
			}else{
				//if num(char1) else char1=1
				//if num(char2) else char2=1
				if (char1.isnum() and char2.isnum()) {
					char3 = char1 + char2;
					if (char3 gt 9) {
						char3 = "*";
					}
				}else{
					char3 = char1;
				}
				str3.splicer(ii, 1, char3);
			}
		}
	};//ii;

	return;
}

subroutine newheadreplacements() {
	//if index(newhead,'{',1) else return
	//replace any {DICTID} in the heading
	for (ii = 1; ii <= nreplacements; ++ii) {
		dictid = replacements.a(ii);
		tt = "{" ^ dictid ^ "}";
		tt2 = calculate(dictid);
		tt2.swapper("'", "''");
		newhead.swapper(tt, tt2);
	};//ii;
	return;
}

subroutine emailing() {
	if (not emailtoid) {
		return;
	}

	if ((DQ ^ "'").index(emailtoid[1])) {
		nextemailto = emailtoid.substr(2,emailtoid.length() - 2);
	}else{
		nextemailto = calculate(emailtoid);
	}
	if (emailccid) {
		nextemailcc = calculate(emailccid);
	}
	if (emailsubjectid) {
		nextemailsubject = calculate(emailsubjectid);
	}

	//dont email if no change
	if (lastrecord) {
	} else if (nextemailto ne emailto) {
	} else if (nextemailcc ne emailcc) {
	} else if (nextemailsubject ne emailsubject) {
	} else {
		return;
	}

	if (printptr) {

		//body='NONE'
		//attachfilename=system<2>
		body = "@" ^ SYSTEM.a(2);

		tt = emailto;
		tt2 = emailcc;
		if (not tt) {
			tt2.transfer(tt);
		}

		if (emailsubject) {
			tt3 = emailsubject;
		}else{
			//tt3='EXODUS: ':field(head<1,1,1>,"'",1)
			tt3 = head.a(1, 1, 1).field("'", 1);
			if (tt3.index(">")) {
				tt3 = field2(tt3, ">", -1);
			}
			tt3 = "EXODUS: " ^ tt3;
		}

		//osclose printfile
		//dont email stuff which has no email address
		if (tt) {
#ifndef EXO_NOHTML
			call sendmail(tt, tt2, tt3, body, "", "", xxerrmsg);
#endif
		}

		printptr = 0;
		var("").oswrite(SYSTEM.a(2));
		//osopen system<2> to printfile else
		// call msg(quote(system<2>):' failed to reopen')
		// stop
		// end

	}

	emailto = nextemailto;
	emailcc = nextemailcc;
	emailsubject = nextemailsubject;

	return;
}

programexit()

