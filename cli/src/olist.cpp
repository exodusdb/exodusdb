#include <exodus/program.h>
programinit()

/*
//Copyright (c) 2009 Stephen Bush
//Licence http://www.opensource.org/licenses/mit-license.php

This is a complete ACCESS/ENGLISH/LIST replacement that can output in DOS or HTML format.

It shows a substantial program written in Exodus's reincarnation of Multivalue Basic.
The original Pick Basic version was in heavy production usage for many years.

It is written using Exodus OO flavour syntax not traditional flavour.

Procedural flavour: Function names on the left and parameters on the right.

	printl(oconv(field(extract(xx,6),"/",2),"D2"));

OO flavour: Does exactly the same thing but is perhaps easier to read from left to right in one pass:

    xx.f(6).field("/",2).oconv("D2").outputl();

xx.f(6) stands for "xx<6>" in mv/pick. <> brackets cannot be used for functions in c++. "a" stands for "attribute".

Hybrid:

    printl(xx.f(6).field("/",2).oconv("D2"));

Comments about style:

	1. Should use int instead of var for "for" loop index variables despite the fact that int doesnt always behave
		precisely like pick integers, speed is important in this program and C++ allows us access to raw power when we need it.

	2. Should convert to using range() in "for" loops

	3. Note the familiar "goto" statement which considerably eases porting of mv basic to Exodus.
		This can be a major point scorer for Exodus because "goto" is banished from almost all other languages.
		Exodus's "goto" cannot jump over "var" statements or into/out of subroutines some code refactoring
		may be still be required during porting.

	For normal programs, global variables that should be generally available to all subroutines
	should be declared, and optionally *simply* initialised, before or after the programinit() line.
	"simply initialised" means set to expressons using raw numbers and strings.
	eg 'var xx="xyz";' or 'var xx=100.5;' etc.

	Before the programinit statement, variables are true free global and are not threadsafe unless
	marked as thread_local. They also do not have access to all the mv global variable like ID, DICT etc.

	After the programinit statement, variables are member variables of the program class.

	Notably in libraries (separately edited and compiled callable functions) class global variables
	are only initialised on the first call to the function.

Example command line:

		list ads brand_code brand_name with brand_code 'XYZ' (N)

	If using a shell like bash then quotes must be escaped or wrapped
	and () should be done with {}

		list ads brand_code brand_name with brand_code "'XYZ'" {N}

	or

		list ads brand_code brand_name with brand_code '"XYZ"' {N}

	Type just list by itself to get a summary of its syntax

*/

#include <exodus/printtx.hpp>

//#include <gen_common.h>

	var copyright;
	var showborder;		 //num
	var headtabperpage;	 //num
	var ulchar;
	var nblocks;  //num
	var blockn;	  //num
	var fromtimestamp;
	var usecols;  //num
	var decimalchar;
	var html;  //num
	var td0;
	var nbsp;
	var td;
	var tdx;
	var th;
	var thx;
	var tr;
	var trx;
	var tt;	 //num
	var tdz;
	var thcolor;
	var tdcolor;
	var reportfont;
	var printptr;  //num
	var cssver;	   //num
	var sentencex;
	var maxncols;  //num
	var maxnrecs;
	var preselect;	//num
	var keylist;	//num
	var crtx;		//num
	var idsupp;		//num
	var dblspc;		//num
	var detsupp;	//num
	var gtotsupp;	//num
	var gtotreq;	//num
	var nobase;		//num
	var rawtable;	//num
	var silent;		//num
	var dictvoc;
	dim colname;
	dim coldict;
	dim mcol;
	dim pcol;
	dim ccol;
	dim scol;
	dim icol;
	var bheadfn;  //num
	var coln;	  //num
	var head;
	var foot2;
	var nbreaks;  //num
	var breakcolns;
	var breakopts;
	var pagebreaks;
	var headtab;
	var hrown;	//num
	var hcoln;	//num
	var replacements;
	var nreplacements;	 //num
	var topmargin;		 //num
	var totalflag;		 //num
	var breakonflag;	 //num
	var anytotals;		 //num
	var multirowcolhdg;	 //num
	var limits;
	var nlimits;  //num
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
	var wordn;	//num
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
	var tcoln;	//num
	//var nn;
	var ii;	 //num
	var tt2;
	var charx;
	var charn;	 //num
	var ncols;	 //num
	var breakn;	 //num
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
	var coln2;	  //num
	var vmcount;  //num
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
	var limitn;	 //num
	var recn;	 //num
	var lastid;
	var readerr;  //num
	var fns;
	var nfns;
	var nmvs;  //num
	var fnn;   //num
	var fn;
	var limitvals;
	var mvx;
	var newhead;
	var printfilename;
	var breakleveln;		//num
	var leveln;				//num
	var previousmarklevel;	//num
	var newmarklevel;		//num
	var str1;
	var str2;
	var str3;
	var tx1;
	var oconvx;
	var bodyln;	 //num
	var bottomline;
	var lastrecord;	 //num
	var printfile;
	var storewordn;
	var storecharn;
	var storeword;
	var storedictrec;
	var startcharn;	 //num
	var searchchar;
	var letterhead;
	var wordexpanded;  //num
	var breakcoln;
	var storetx;
	var anycell;  //num
	var lastblockn;
	var underline2;
	var cell;  //num
	var colbreakn;
	var bottomline2;
	var char1;	//num
	var char2;	//num
	var char3;
	var body;
	var tt3;
	var xxerrmsg;
	var ownprintfile;  //num
	var ptx_filenamelen;
	var ptx_random;
	var pagelns;
	var realpagen;	//num
	var pagen;		//num
	var newpage;	//num
	var rfmt;
	var foot;
	var ntxlns;	   //num
	var nbodylns;  //num
	var ptx_temp;
	var headx;
	var newpagetag;
	var ptx_css;
	var stylennx;  //num
	var htmltitle;
	var head_or_foot;
	var footx;
	var head1;
	var optioncharn;  //num
	var optionchars;
	var optionchar;
	var newoptions;
	var printtx_ii;	 //num
	var spaceoptionsize;

function main() {
	//
	//c sys
	copyright = "Copyright (c) 2009 Stephen Bush";
	showborder = 0;
	headtabperpage = 1;

	//TRACE(SENTENCE)
	//TRACE(COMMAND)

	if (TERMINAL and not OPTIONS.contains("N")) {
		// Switch off TERMINAL
		var oscmd = "TERM=";

		// Reconstruct the list command into a format suitable for processing by bash
		// If word has any bash special characters or spaces
		// then escape any single quotes and wrap in single quotes
		for (var word : COMMAND) {

			if (word[1] eq DQ and word[-1] eq DQ) {
				word.squoter();
			}
			else if (word[1] eq SQ and word[-1] eq SQ) {
				word.quoter();
			}
			else {
				var word2 = word.convert(R"( `~!@#$%^&*(){}[]:;'"|<>?\)" to "");
				if (word2.len() < word.len()) {

					// Single quotes inside single quotes can be represented as '"'"' for bash
					// The leading and trailng aingle quotes in '"'"' terminate and resume the single quoted string
					// so 'abc'"'"'def' is actually abc'def
					word.replacer("'", R"('"'"')");

					word.squoter();
				}
			}

			// Escape any ( )
			if (var("()").index(word))
				word = BS ^ word;

			oscmd ^= " " ^ word;
		}

		// Add any options
		if (OPTIONS)
			oscmd ^= " {"^ OPTIONS ^ "}";

		// Pipe into pager
		oscmd ^= " | pager --chop-long-lines --quit-if-one-screen";

		//TRACE(oscmd)
		return osshell(oscmd);
	}

//	if (USERNAME eq "EXODUS") {
//		var(SENTENCE).oswrite("NLIST");
//	}
//
	//	#include <general_common.h>
	//global all
	//for printtx
	//global html,head,foot,cssver,htmltitle,topmargin,bottomline,tx

	#define interactive not(SYSTEM.f(33))

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
		fromtimestamp = "";
	} else {
		fromtimestamp = timestamp();
	}

	//use <COL> for hiding non-totalled cols in det-supp (slow)
	usecols = 0;

	if (BASEFMT[2] eq "C") {
		decimalchar = ",";
	} else {
		decimalchar = ".";
	}

	html = SYSTEM.f(2).lcase().ends("htm") or OPTIONS.contains("h");

	if (html) {
		td0 = "";
		nbsp = "&nbsp;";
		td = td0 ^ "<td>";
		tdx = "</td>";
		th = td0 ^ "<th>";
		thx = "</th>";
		tr = "<tr>";
		trx = "</tr>";
//		tt = SYSTEM.f(2);
//		tt.replacer(".txt", ".htm");
//		SYSTEM(2) = tt;
		SYSTEM(2) = "-";
	} else {
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

	thcolor = SYSTEM.f(46, 1);
	tdcolor = SYSTEM.f(46, 2);
	reportfont = SYSTEM.f(46, 3);
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
	if (sentencex.contains(" DET-SUPP")) {
		detsupp = 1;
	}
	if (sentencex.contains(" DET-SUPP2")) {
		detsupp = 2;
	}
	gtotsupp = "";
	gtotreq = "";
	nobase = "";
	rawtable = 0;
	silent = 0;

	if (not(dictvoc.open("dict.voc", ""))) {
		createfile("dict.voc");
		if (not(dictvoc.open("dict.voc", ""))) {
			abort(lasterror());
		}
	}

	if (not(tt.read(dictvoc, "@ID"))) {
		write("F^0^TABLE_NAME^S^1^^^^L^20^^VARCHAR"_var on dictvoc, "TABLE_NAME");
		write("F^0^COLUMN_NAME^S^2^^^^L^20"_var on dictvoc, "COLUMN_NAME");
		write("G^^TYPE FMC PART HEADING SM CONV JUST LEN MASTER_FLAG^^^^^^^^^^^^^^^^^^^^^^^^^0"_var on dictvoc, "@CRT");
		write("F^0^Ref.^S^0^^^^T^20"_var on dictvoc, "@ID");
		write("F^1^TYPE^S^0^^^^L^4"_var on dictvoc, "TYPE");
		write("F^2^FMC^S^0^^^^R^3"_var on dictvoc, "FMC");
		write("F^5^PART^S^0^^^^R^2"_var on dictvoc, "PART");
		write("F^3^HEADING^S^0^^^^T^20"_var on dictvoc, "HEADING");
		write("F^4^SM^S^0^^^^L^4"_var on dictvoc, "SM");
		write("F^7^CONV^S^0^^^^L^9"_var on dictvoc, "CONV");
		write("F^9^JST^S^0^^^^L^3"_var on dictvoc, "JUST");
		write("F^10^LEN^S^0^^^^R^3^^LENGTH"_var on dictvoc, "LEN");
		write("F^28^MST^S^^^BYes,^^L^4"_var on dictvoc, "MASTER_FLAG");
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

	//TRACE(sentencex)
	//TRACE(OPTIONS)

///////////
nextphrase:
	///////////

	gosub getword();
	if (word eq "") {
		goto x1exit;
	}

phraseinit:
	///////////
	if (word.starts("SORT") or word.starts("NSORT")) {
		ss ^= "SSELECT";

filename:
		gosub getword();
		if (not word) {
			call mssg("FILE NAME IS REQUIRED");
			abort("");
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
		if (filename.starts("DICT.")) {
			dictfilename = "VOC";
		} else {
			dictfilename = filename;
		}
		if (not(DICT.open("dict." ^ dictfilename))) {
			//commented so that list dict_clients gives dict.voc columns
			//crtx = 1;
			DICT = dictvoc;
		}
		ss ^= " " ^ word;

		//exodus custom

		if (html) {

			if (var("PLANS,SCHEDULES,ADS,BOOKING_ORDERS,VEHICLES,RATECARDS").locateusing(",", filename, xx)) {
				printtxmark = "Media Management";
			}

			if (var("JOBS,PRODUCTION_ORDERS,PRODUCTION_INVOICES").locateusing(",", filename, xx)) {
				printtxmark = "Production Management";
			}

			if (var("CHARTS,RECURRING").locateusing(",", filename, xx)) {
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
		if ((nextword.isnum() or (nextword.starts("'"))) or (nextword.starts(DQ))) {
			keylist = 1;
			ss ^= " " ^ nextword;
			gosub getword();
			if (nextword.len()) {
				goto nextkey;
			}
		}

	} else if ((word.starts("LIST") or word.starts("NLIST")) or word eq "OLIST") {
		ss ^= "SELECT";
		goto filename;

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
			if (dictrec.readc(DICT, word)) {
				preselect = dictrec.f(4).starts("M");
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
			limits(1, nlimits) = word;
			if (not(dictrec.readc(DICT, word))) {
				call mssg(word ^ " is not a valid dictionary item");
				abort("");
			}
			tt = dictrec.f(4).field(".", 1);
			if (tt[1] ne "M") {
				call mssg(word ^ " limit must be a multivalued dict item");
				abort("");
			}
			limits(4, nlimits) = tt;
		}

		//negate next comparision
		if (var("NOT,NE,<>").locateusing(",", nextword, xx)) {
			nextword = "NOT";
			gosub getword();
			ss ^= " " ^ word;
		}

		//comparision
		if (var("MATCH,EQ,,NE,GT,LT,GE,LE,<,>,<=,>=,=,[],[,]").locateusing(",", nextword, xx)) {
			//only EQ works at the moment
			gosub getword();
			ss ^= " " ^ word;
			if (limitx) {
				limits(2, nlimits) = word;
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

		} else {

			//parameters
			while (true) {
				///BREAK;
				if (not(nextword ne "" and (((nextword.isnum() or (nextword.starts(DQ))) or (nextword.starts("'"))))))
					break;
				gosub getword();
				ss ^= " " ^ word;
				if (limitx) {
//					if ((DQ ^ "'").contains(word[1])) {
//						if (word.starts(word[-1])) {
//							word.cutter(1).popper();
//						}
//					}
					word.unquoter();
					if (word eq "") {
						word = "\"\"";
					}
					limits(3, nlimits, -1) = word;
				}
			}  //loop;
		}

	} else if (word eq "BREAK-ON") {
		tt = coln + 1;
		breakcolns.prefixer(tt ^ FM);
		breakopts.prefixer(FM);
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
		if (not DICT.open("dict." ^ dictfilename)) {
			abort(lasterror());
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

		if ((DQ ^ "'").contains(nextword[1])) {
			title = word;
			gosub getquotedword2();
			value = word;
		} else if (word.starts("{")) {
			title = "";
			value = word;
		} else {
			title = word;
			value = "";
		}

		//automatic labelling with dictionary title
		if (word.starts("{")) {
			tt = word.cut(1).pop();
			replacements(-1) = tt;
			nreplacements += 1;
			if (not(tt.readc(DICT, tt))) {
				if (not(tt.readc(dictvoc, tt))) {
					call mssg(tt ^ " is not a valid dictionary item");
					abort("");
				}
			}
			if (title eq "") {
				title = tt.f(3) ^ nbsp ^ ":";
			}
		}

		tcoln = (hcoln - 1) * 2 + 1;
		//find the next empty row
		while (true) {
			///BREAK;
			if (not(headtab.f(hrown, tcoln) or (headtab.f(hrown, tcoln + 1))))
				break;
			hrown += 1;
		}  //loop;

		//prevent trailing colon folding onto following line
		if (title.ends(" :")) {
			title.paster(-2, 2, "&nbsp;:");
		}

		//any existing doubled single quotes are removed to avoid double doublimg
		title.replacer("''", "'");
		//double any single quotes to avoid them being understood as options
		title.replacer("'", "''");

		headtab(hrown, tcoln) = title.convert(FM ^ VM, "  ");
		headtab(hrown, tcoln + 1) = value;
		hrown += 1;

	} else if (word eq "HEADING" and dictrec.f(3) ne "HEADING") {

		gosub getquotedword();
		head ^= word;

		//remove page numbering options from headings
		if (html) {
			head.replacer("Page 'P'", "");
			head.replacer("Page 'P", "'");
		}

	} else if (word eq "FOOTING") {
		gosub getquotedword();
		foot2 ^= word;

		//justlen
	} else if (word eq "JL" or word eq "JUSTLEN") {
		if (not coln) {
			call mssg("JUSTLEN/JL must follow a column name");
			abort("");
		}
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			gosub getquotedword();
			coldict(coln)(9) = word[1];
			coldict(coln)(10) = word.b(3);
			coldict(coln)(11) = word;
		}

		//colhead
	} else if (word eq "CH" or word eq "COLHEAD") {
		if (not coln) {
			call mssg("COLHEAD/CH must follow a column name");
			abort("");
		}
		gosub getquotedword();
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			word.converter("|", VM);
			coldict(coln)(3) = word;
		}

	} else if (word eq "OC" or word eq "OCONV") {
		if (not coln) {
			call mssg("OCONV/OC must follow a column name");
			abort("");
		}
		gosub getquotedword();
		//skip if detsupp2 and column is being skipped
		if (not(coldict(coln).unassigned())) {
			if (html) {
				word.replacer("[DATE]", "[DATE,*]");
			}
			coldict(coln)(7) = word;
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
		if ((DQ ^ "'").contains(word[1])) {
			emailcc = word.cut(1).pop();
			nextemailcc = emailcc;
		} else {
			emailccid = word;
		}

	} else if (word eq "EMAIL_SUBJECT") {
		gosub getword();
		if ((DQ ^ "'").contains(word[1])) {
			emailsubject = word.cut(1).pop();
			nextemailsubject = emailsubject;
		} else {
			emailsubjectid = word;
		}

	} else if (dictrec) {

		if (var("FSDIA").contains(dictrec.f(1))) {

			//pick format dictionary
			//if index('DI',dictrec<1>,1) then
			// call dicti2a(dictrec)
			// end
			//if dictrec<1>='A' then dictrec<1>='F'

			//suppress untotalled columns if doing detsupp2
			if (detsupp eq 2 and (not(totalflag or breakonflag))) {
				if (var("JL,JUSTLEN,CH,COLHEAD,OC,OCONV").locateusing(",", nextword, xx)) {
					gosub getword();
					gosub getword();
				}
				goto dictrecexit;
			}

			coln += 1;
			colname(coln) = word;

			//increase column width if column title needs it
//			let nn = dictrec.f(3).count(VM) + 1;
//			for (ii = 1; ii <= nn; ++ii) {
//				tt = dictrec.f(3, ii);
			if (dictrec.f(10)) {
				let titles = dictrec.f(3).convert(VM, FM);
				for (let part : titles) {
					if (part.len() gt dictrec.f(10)) {
						dictrec(10) = part.len();
					}
				} //ii;
			}

			dictrec(bheadfn) = "";

			if (detsupp lt 2) {
				if (not(totalflag or breakonflag)) {
					//tt=' id="BHEAD"'
					//if detsupp then tt:=' style="display:none"'
					if (detsupp) {
						tt = " class=\"BHEAD\"";
					} else {
						tt = " class=\"BHEAD2\"";
					}
					dictrec(bheadfn) = tt;
				}
			}

			//total required ?
			if (totalflag) {
				totalflag = 0;
				dictrec(12) = 1;
				anytotals = 1;
			} else {
				dictrec(12) = 0;
			}

			if (html) {
				tt = dictrec.f(7);
				tt.replacer("[DATE]", "[DATE,*]");
				if (tt eq "[DATE,4]") {
					tt = "[DATE,4*]";
				}
				dictrec(7) = tt;
				if (tt eq "[DATE,*]") {
					dictrec(9) = "R";
				}
			}
			coldict(coln) = dictrec;

			//store the format in a convenient place
			tt = "";
			if (not html) {
				tt = coldict(coln).f(9) ^ "#" ^ coldict(coln).f(10);
			}
			coldict(coln)(11) = tt;

			//this could be a break-on column and have break-on options
			//if coln=breakcolns<1> then
			if (breakonflag) {
				coldict(coln)(13) = 1;
				breakonflag = 0;

				if (nextword.starts(DQ)) {
					gosub getword();

					//zzz break  options
					tt = word.index("B");
					if (tt) {

						//suppress columns that appear in the heading
						coldict(coln)(10) = 0;

						//pagebreakcolns<1,-1>=coln

						//determine B99.99 format for row and col
						tt2 = "";
						for (tt += 1; tt <= 9999; ++tt) {
							charx = word[tt];
							///BREAK;
							if (not(charx.len() and var("0123456789,").contains(charx)))
								break;
							tt2 ^= charx;
						} //tt;

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
							headtab(hrown, tcoln) = coldict(coln).f(3).convert(VM, " ") ^ nbsp ^ ":";
							headtab(hrown, tcoln + 1) = "'B" ^ tt2 ^ "'";
							hrown += 1;
						}

//						pagebreaks.r(coln,
//									 "'"
//									 "B" ^
//										 tt2 ^ "'");
						pagebreaks(coln) = squote("B" ^ tt2);
					}
					breakopts(1) = word;
				}
			}
		}

dictrecexit:
	{}

	} else if (word eq "IGNOREWORD") {
		gosub getword();
		ignorewords(1, -1) = word;

	} else {
		tt = word.quote() ^ " is an unrecognized word.";
		if (interactive) {
			tt ^= " Replace with? (Enter to cancel):";
		}
		var oldword = word;
		call mssg(tt, "RCE", word, "");
		if (word eq oldword or word eq "\x1B") {
			abort("");
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
		if (not(xx.readc(DICT, word))) {
			word = "@CRT";
			if (not(xx.readc(DICT, word))) {
				word = "";
			}
		}
		if (word) {
			crtx = 1;
			sentencex ^= " " ^ word;
			//charn-=2
			charn = sentencex.len() - word.len();
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
		} //coln;

		//set column 1
		colname(1) =
			"@"
			"ID";
		if (not(coldict(1).readc(DICT,
								 "@"
								 "ID"))) {
			if (not(coldict(1).readc(dictvoc,
									 "@"
									 "ID"))) {
				coldict(1) = "F" ^ FM ^ FM ^ "Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "L" ^ FM ^ 15;
			}
		}
		if (html) {
			tt = "";
		} else {
			tt = coldict(1).f(9) ^ "#" ^ coldict(1).f(10);
		}
		coldict(1)(11) = tt;

		//increment the list of breaking columns by one as well
		for (breakn = 1; breakn <= nbreaks; ++breakn) {
			breakcolns(breakn) = breakcolns.f(breakn) + 1;
		} //breakn;

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

	if (breakcolns.ends(FM)) {
		breakcolns.popper();
	}
	if (breakopts.ends(FM)) {
		breakopts.popper();
	}

	//make underline and column title underline
	if (not html) {
		underline = "";
		colul = "";
		for (coln = 1; coln <= ncols; ++coln) {
			if (coldict(coln).f(10)) {
				if (coldict(coln).f(12)) {
					tt = ulchar;
				} else {
					tt = " ";
				}
				underline ^= tt.str(coldict(coln).f(10)) ^ " ";
				colul ^= ulchar.str(coldict(coln).f(10)) ^ " ";
			}
		} //coln;
		bar = ulchar.str(colul.len() - 1);
	}

	//////
	//init2:
	//////

	tx = "";

	if (not srcfile.open(filename, "")) {
		abort(lasterror());
	}

	breakcount.redim(nbreaks + 1);
	breaktotal.redim(maxncols, nbreaks + 1);
	breakvalue.redim(maxncols);
	oldbreakvalue.redim(maxncols);

	breakvalue = "";
	oldbreakvalue = "";
	breakcount = "";
	breaktotal = "";

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
			coldict(coln)(bheadfn) = "";
		}

		if (coldict(coln).f(10)) {
			if (html) {
				coln2 += 1;

				//suppressing non-totalled columns may not work well with multi-row colhdg
				tt = coldict(coln).f(bheadfn);

				//tt2=index(tt,'style="',1)
				//tt3='background-color:':thcolor
				//if tt2 then
				// tt[tt2+7,0]=tt3:';'
				//end else
				// tt:=' style=':quote(tt3)
				// end

				thproperties(coln2) = tt;

				//without the MULTIROW_COLHDG keyword,
				//vm indicates folding, \\ indicates rows in column headings
				tt = coldict(coln).f(3);
				if (not multirowcolhdg) {
					tt.replacer(VM, "<br />");
				}
				tt.replacer("\\\\", VM);

				colhdg(coln2) = tt;
				if (tt.count(VM) gt vmcount) {
					vmcount = tt.count(VM);
				}

				//replace vm with '<br />' in colhdg

				coltags(-1) = " <col";
				align = coldict(coln).f(9);
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
					coltags ^= coldict(coln).f(bheadfn);
				}
				coltags ^= " />";

				//nth child style column justification in case <col> doesnt work like on FF
				if (align) {
					//works per table if the table is assigned a class (.maintable) here
					style ^= "table.exodustable td:nth-child(" ^ coln2 ^ "){text-align:" ^ align ^
							 "}"
							_EOL;
				}

			} else {
				for (ii = 1; ii <= 9; ++ii) {
					colhdg(ii) = colhdg.f(ii) ^ oconv(coldict(coln).f(3, ii), coldict(coln).f(11)) ^ " ";
				} //ii;
			}
		}
	} //coln;

	if (style) {
		style =
			"<style type=\"text/css\">"
			_EOL ^
			style ^
			"</style>"
			_EOL;
	}

	//convert to html with colspan/rowspan where necessary and (Base) as currcode
	//thproperties='style="background-color:':thcolor:'"'
//#ifndef EXO_NOHTML
	if (html) {
		//call colrowspan(colhdg, thproperties, nobase);
		call htmllib2("COLROWSPAN", colhdg, thproperties, nobase);
	}
//#endif
	//trim off blank lines (due to the 9 above)
	if (html) {
//#ifndef EXO_NOHTML
		//call getsortjs(tt);
		call htmllib2("GETSORTJS", tt);

		if (not rawtable) {
			call getmark("CLIENT", html, clientmark);
			tt ^= clientmark ^ _EOL;
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
		tt.move(colhdg);

		//allow for single quotes
		colhdg.replacer("'", "''");
//#endif
	} else {
		while (true) {
			///BREAK;
			if (not(colhdg and ((" " ^ FM).contains(colhdg[-1]))))
				break;
			colhdg.popper();
		}  //loop;
	}

	//heading options

	//if head='' then head="Page 'P' ":space(50):" 'T'"
	if (head eq "" and not(rawtable)) {
		head = filename ^ var(10).space() ^ " 'T'";
	}

	if (html) {
		head.replacer(FM, "<br />");
		//div to make header as wide as body
		//the report title
		if (head) {
			head = "<h2{%20}style=\"margin:0px;text-align:center;padding-top:0px\">" ^ head ^ "</h2>";
		}

		//supertable to ensure any heading is as wide as the body of the report
		if (showborder) {
			head(-1) = "<table{%20}border=1{%20}style=\"border-width:3px\"{%20}cellpadding=0{%20}cellspacing=0{%20}align=center><tr><td>";
		} else {
			//head:='<table{%20}cellpadding=0{%20}cellspacing=0{%20}align=center><tr><td>'
			head(-1) = "<table{%20}align=center><tr><td>";
		}
	}

	//footing options

	if (headtab) {
		tt = "<table id=\"headtab0\" width=100% align=center cellpadding=3>";

		//older MSIE <col> styling
		headtabcols = " <col style=\"text-align:left\"/>" ^ VM ^ "<col style=\"text-align: left;font-weight:bold\"/>";
		//allow 8 max pair of headtab columns
		headtabcols = (headtabcols ^ VM).str(8);
		headtabcols.popper();

		tt ^=
			"<colgroup>"
			_EOL ^
			headtabcols.replace(VM, _EOL) ^ "</colgroup>";
//#ifndef EXO_NOHTML
		//style columns where '<col>' not supported.
		//call convcss(mode, "headtab0", headtabcols, headtabstyle);
		call htmllib2("CONVCSS", headtabstyle, "headtab0", headtabcols);
		style ^= _EOL ^ headtabstyle;

		tt ^=
			"<colgroup>"
			_EOL ^
			headtabcols.replace(VM, _EOL) ^ "</colgroup>";

		tt ^=
			_EOL
			"<TBODY>";
		call htmllib2("TABLE.MAKE", headtab, tt, "");
//#endif
		headtab.replacer("</TR>",
						"</TR>"
						_EOL);
		if (headtabperpage) {
			colhdg.replacer(posttheadmark, tr ^ td0 ^ "<th style=\"background-color:white\" colspan=" ^ ncols ^ ">" ^ headtab ^ thx ^ trx);
		} else {
			headtab.replacer(posttheadmark, "");
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

	head.prefixer(FM);

	orighead = head;

	//work out assoc mv fns for limits
	if (nlimits) {
		call pushselect(0, v69, v70, v71);
		select(DICT);
nextdict:
		if (readnext(dictid)) {
			if (not(dictrec.readc(DICT, dictid))) {
				goto nextdict;
			}
			if (dictrec.f(1) ne "F") {
				goto nextdict;
			}
			for (limitn = 1; limitn <= nlimits; ++limitn) {
				if (dictrec.f(4).field(".", 1) eq limits.f(4, limitn)) {
					tt = dictrec.f(2);
					if (tt) {
						if (not(limits.f(5, limitn).locateusing(SM, tt, xx))) {
							limits(5, limitn, -1) = dictrec.f(2);
						}
					}
				}
			} //limitn;
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
			abort("");
		}

		if (not LISTACTIVE) {
			if (html) {
				tx ^= "</tbody></table>";
			}
			tx ^= "No records listed";
			gosub printtx();
			goto x2bexit;
		}

	} else {
		if (not LISTACTIVE) {
			//select(srcfile);
			select("SELECT " ^ filename ^ " (SR)");
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
	if (TERMINAL and esctoexit()) {
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

//	//readnext key
//	FILEERRORMODE = 1;
//	FILEERROR = "";

	//if (not(readnext(ID, MV))) {
	if (not(readnext(RECORD, ID, MV))) {
//		FILEERRORMODE = 0;
//		if (STATUS) {
//			tx = "*** Fatal Error " ^ FILEERROR.f(1) ^ " reading record " ^ ID ^ " ***";
//			gosub printtx();
//			abort("");
//		}
//		if (FILEERROR.f(1) eq 421) {
//			tx = "Operation aborted by user.";
//			gosub printtx();
//			abort("");
//		}
//		if (FILEERROR and FILEERROR.f(1) ne 111) {
//			tx = "*** Error " ^ FILEERROR.f(1) ^ " reading record " ^ ID ^ " ***";
//			gosub printtx();
//			readerr += 1;
//			abort("");
//		}
		goto x2exit;
	}

	if (ID.starts("%")) {
		goto nextrec;
	}

	//skip if record appears twice to fix a bug in indexing
	if (not(MV)) {
		//watch out that 200 is equal to 200.0 etc and would be skipped!!!
		if (ID eq lastid) {
			if (ID.len() eq lastid.len()) {
				goto nextrec;
			}
		}
	}
	lastid = ID;

	// The select (R) option should have provided RECORD already
	if (not RECORD.len() and not RECORD.read(srcfile, ID)) {
		goto nextrec;
	}

	//designed to filter multivalues which are not selected properly
	//unless sorted "by"
	if (limits) {

		//for each limit pass through record deleting all unwanted multivalues
		for (limitn = 1; limitn <= nlimits; ++limitn) {

			//find maximum mv number for the associated group of fns
			fns = limits.f(5, limitn);
			nfns = fns.count(SM) + (SM ne "");
			nmvs = 0;
			for (fnn = 1; fnn <= nfns; ++fnn) {
				fn = fns.f(1, 1, fnn);
				tt = RECORD.f(fn);
				if (tt.len()) {
					tt = tt.count(VM) + 1;
					if (tt gt nmvs) {
						nmvs = tt;
					}
				}
			} //fnn;

			limitvals = calculate(limits.f(1, limitn));
			for (mvx = nmvs; mvx >= 1; --mvx) {
				tt = limitvals.f(1, mvx);
				if (tt eq "") {
					tt = "\"\"";
				}
				//locate tt in (limits<3,limitn>)<1,1> using sm setting xx else
				if (not(limits.f(3, limitn).locateusing(SM, tt, xx))) {
					for (fnn = 1; fnn <= nfns; ++fnn) {
						RECORD.remover(fns.f(1, 1, fnn), mvx);
					} //fnn;
				}
			} //mvx;
		} //limitn;
	}

	recn += 1;
	RECCOUNT += 1;

	////////
	//recinit:
	////////

	if (recn eq 1) {
		head.move(newhead);
		gosub newheadreplacements();
		newhead.move(head);
	}

	//if interactive then print @AW<30>:@(36,@CRTHIGH/2):
	if (not(silent) and not(printfilename.unassigned())) {
		//put.cursor(cursor)
		if (printfilename and TERMINAL) {
			print(AT(-40), recn, ". ", ID, " ", MV);
		}
	}

	//if det-supp2 then zero width initialiser columns" will not be called
	//unless you put TOTAL dictid
	//get the data from the record into an array of columns

	for (coln = 1; coln <= ncols; ++coln) {
		//  @mv=0

		if (coldict(coln).f(9) eq "T" and not(html)) {

			mcol(coln) = oconv(calculate(colname(coln)), coldict(coln).f(11));

		} else {

			mcol(coln) = calculate(colname(coln));

			//replace '<' with '&lt;' in m.col(coln)
			//replace '>' with '&gt;' in m.col(coln)
			if (html) {
				mcol(coln).replacer(TM, "<br />");
			}
		}

		//call note(' @id/coln/m.col/coldictx<11>=':@id:'/':coln:'/':m.col(coln):'/':coldict(coln)<11>)
		//call note(' @id/@record<':(coldict(coln)<2>):'>=':@id:'/':(@record<coldict(coln)<2>))

		pcol(coln) = 1;
		ccol(coln) = 7;
		scol(coln) = mcol(coln);
	} //coln;

	//break subtotals
	//detect most major level to break
	if (recn eq 1) {
		//print breaks will not actually print before the first record
		// but it needs to set the various break values
		breakleveln = nbreaks;
	} else {
		for (leveln = nbreaks; leveln >= 1; --leveln) {
			coln = breakcolns.f(leveln);
			///BREAK;
			if (scol(coln) ne breakvalue(coln))
				break;
		} //leveln;
		breakleveln = leveln;
	}

	gosub printbreaks();

	oldbreakvalue = breakvalue;

	previousmarklevel = 0;

////////
recexit:
	////////

	//remove appropriate value from multi-valued column(s)
	newmarklevel = 0;
	for (coln = 1; coln <= ncols; ++coln) {
		if (ccol(coln) ge previousmarklevel) {
			icol(coln) = mcol(coln).substr2(pcol(coln), ccol(coln));
			scol(coln) = icol(coln);
		}
		if (ccol(coln) gt newmarklevel) {
			newmarklevel = ccol(coln);
		}
	} //coln;

	//break totals - add at the bottom level (1)
	for (coln = 1; coln <= ncols; ++coln) {
		//only totalled columns
		if (coldict(coln).f(12)) {
			if (icol(coln)) {
				if (html) {
//#ifndef EXO_NOHTML
					//breaktotal(coln,1)+=i.col(coln)
					//call addunits(icol(coln), breaktotal(coln, 1), VM);
					// breaktotal <- icol
					call htmllib2("ADDUNITS", breaktotal(coln, 1), icol(coln), VM);
//#endif
				} else {
					if (breaktotal(coln, 1).isnum() and icol(coln).isnum()) {
						breaktotal(coln, 1) += icol(coln);
					} else {
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
	} //coln;

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
					tx1 ^=
						" onclick=\"toggle("
						"'"
						"B" ^
						blockn ^
						"'"
						")\"";
				}
			}
			tx1 ^= ">";
		}
		for (coln = 1; coln <= ncols; ++coln) {
			tt = scol(coln);
			oconvx = coldict(coln).f(7);
			if (oconvx) {
				tt = oconv(tt, oconvx);
				if (html) {
					if (tt.starts("-")) {
						if (oconvx.starts("[NUMBER")) {
							tt = "<nobr>" ^ tt ^ "</nobr>";
						}
					}
				}
			}
			if (coldict(coln).f(10)) {
				if (not html) {
					tt = oconv(tt, coldict(coln).f(11));
				}
				if (tt eq "") {
					//tt=nbsp
				} else {
					tt.replacer(_EOL, "<br />");
				}

				//colored cells starting with ESC
				if (tt[1] eq "\x1B") {
					if (tt.starts("\x1B\x1B")) {
						tt = tt.field(" ", 2, 999999);
						if (tt.len()) {
							tx1 ^= td ^ "<nobr>" ^ tt ^ "</nobr>" ^ tdx;
						} else {
							//tx1:=td:nbsp:tdx
							tx1 ^= td ^ tdx;
						}
					} else {
						//tx1:=td0:'<td bgcolor=':field(tt,' ',1)[2,9999]:'>'
						//TODO do with class? to save document space?
						tx1 ^= td0 ^ "<td style=\"background-color:" ^ tt.field(" ", 1).cut(1) ^ "\">";
						tt = tt.field(" ", 2, 999999);
						if (tt.len()) {
							tx1 ^= tt ^ tdx;
						} else {
							//tx1:=nbsp:tdx
							tx1 ^= tdx;
						}
					}

				} else {
					if (tt.len()) {
						tx1 ^= tdz ^ tt ^ tdx;
					} else {
						//tx1:=tdz:nbsp:tdx
						tx1 ^= tdz ^ tdx;
					}
				}
			}
		} //coln;

		//replace '<td' with '<th' in tx1
		//replace '<\td' with '<\th' in tx1
		tx ^= tx1;

		if (html) {
			//replace '<td></td>' with '<td>':nbsp:'</td>' in tx
			tx ^= "</tr>";
		}

		gosub printtx();

		//folding text or multivalued lines
		if (newmarklevel) {
			for (coln = 1; coln <= ncols; ++coln) {
				scol(coln) = "";
			} //coln;
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
		tx(-1) = FM ^ bottomline ^ FM;
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

			if (fromtimestamp) {
				tx ^= ", " ^ elapsedtimetext(fromtimestamp, timestamp()) ^ ".";
			}
			if (html) {
				tx ^= "</p>";
			}
		}
	}

	if (html) {

		if (not detsupp) {
			tx(-1) = "<script type=\"text/javascript\">" ^ FM ^ " togglendisplayed=" ^ nblocks ^ FM ^ "</script>";
		}

		tx(-1) = "<script type=\"text/javascript\">" ^ FM;
		tx ^= "function nwin(key,url,readonly) {";
		tx ^= "gwindowopenparameters={};";
		tx ^= "if (readonly) gwindowopenparameters.readonlymode=true;";
		tx ^= "gwindowopenparameters.key=key;";
		tx ^= "glogincode=\"" ^ SYSTEM.f(17) ^ "*" ^ USERNAME ^ "*\";";
		//tx:='window.open(url)}'
		//similar code in NLIST and LEDGER2
		tx ^= "var vhtm=window.opener.location.toString().split(\"/\");";
		tx ^=
			"vhtm[vhtm.length-1]=url;"
			_EOL;
		tx ^=
			"window.open(vhtm.join(\"/\"));"
			_EOL;
		tx ^= "}";
		tx ^= FM ^ "</script>";
	}

x2bexit:

	//if html then tx<-1>='</div></body></html>'
	//close supertable div and html
	if (html) {
		tx(-1) = "</td></tr></table>";
		tx(-1) = "</div></body></html>";
	}

	gosub printtx();

	lastrecord = 1;
	gosub emailing();

	printfile.osclose();

	//stop();

	return "";
}

subroutine getquotedword() {
	lastword = word;
	gosub getquotedword2();
	return;
}

subroutine getquotedword2() {
	gosub getword();
	if (((DQ ^ "'").contains(word[1])) and word.starts(word[-1])) {
		word.cutter(1);
		word.popper();
	} else {
		call mssg(lastword ^ " must be followed by a quoted phrase");
	}
	return;
}

subroutine getword() {

getnextword:
	////////////
	gosub getword2();

	if (word.len()) {
		if (ignorewords.f(1).locate(word, xx)) {
			goto getnextword;
		}
	}

	if (word eq "") {
		nextword = "";
	} else {
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
	//TRACE(charn ^ " " ^ word)
	//TRACE(nextword)
	return;
}

subroutine getword2() {

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
//	while (true) {
//		///BREAK;
//		if (not(sentencex[charn] eq " "))
//			break;
//		charn += 1;
//		if (charn gt sentencex.len()) {
//			return;
//		}
//	}  //loop;
	while (sentencex[charn] eq " ") {
		charn++;
	}
	if (charn gt sentencex.len())
		return;

	//if next word starts with " or ' then scan for the same closing
	//otherwise scan up to the next space char
	startcharn = charn;
	charx = sentencex[charn];
	if (charx and ("'" ^ DQ).contains(charx)) {
		searchchar = charx;
	} else {
		searchchar = " ";
	}
	word ^= charx;

	//build up the word character by character until the closing char is found
	//closing character (" ' or space)
	while (true) {
		charn += 1;
		charx = sentencex[charn];
		///BREAK;
		if (not(charx ne "" and charx ne searchchar))
			break;
		word ^= charx;
	}  //loop;

	//if scanned for " or ' then add it to the word
	if (searchchar ne " ") {
		word ^= searchchar;
		charn += 1;

		//otherwise
	} else {
		word.ucaser();
		word = word.trimlast().trimfirst();
	}

	//get options and skip to next word
	if (((word.starts("(")) and (word.ends(")"))) or (((word.starts("{")) and (word.ends("}"))))) {
		tt = word;
		//option (N) no letterhead
		if (tt.contains("N")) {
			letterhead = "";
		}
		if (tt.contains("NN")) {
			rawtable = 1;
			bottomline = "";
		}
		if (tt.contains("S")) {
			silent = 1;
		}
		if (tt.contains("T")) {
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
	if (not(html) and word.ends("_LINK"))
		word.cutter(-5);
	if (dictrec.readc(DICT, word)) {
maindict:
		if (dictrec.f(1) eq "G") {
			tt = dictrec.f(3);
			tt.converter(VM, " ");
			sentencex.paster(startcharn, word.len(), tt);
			charn = startcharn - 1;
			wordn -= 1;
			wordexpanded = 1;
			return;
		}
	} else {
		if (dictrec.readc(DICT, word.ucase())) {
			goto maindict;
		}
dictvoc:
		dictrec = "";
		if (dictrec.readc(dictvoc, word)) {
gotdictvoc:
			if (dictrec.f(1) eq "RLIST") {
				if (dictrec.f(4)) {
					word = dictrec.f(4);
				}
				dictrec = "";
			}
		} else {
			if (dictrec.readc(dictvoc, word.ucase())) {
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

	word.replacer("%DQUOTE%", DQ);
	word.replacer("%SQUOTE%", "'");

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
		breakcoln = breakcolns.f(leveln);

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
		} else {
			if (not html) {
				//underline2=if breakleveln>=nbreaks then bar else underline
				//WARNING TODO: check ternary op following;
				underline2 = leveln eq 1 ? underline : bar;
				if (not((tx.last(2)).contains(ulchar))) {
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
				tx ^=
					" style=\"cursor:pointer\" onclick=\"toggle("
					"'"
					"B" ^
					lastblockn ^
					"'"
					")\"";
			}
			//if detsupp<2 or (nbreaks>1 and leveln>1) then tx:=' style="font-weight:bold"'
			tx ^= ">";
		}
		for (coln = 1; coln <= ncols; ++coln) {

			//total column
			if (coldict(coln).f(12)) {
				cell = breaktotal(coln, leveln);

				//add into the higher level
				if (leveln le nbreaks) {

					if (cell) {
						if (html) {
//#ifndef EXO_NOHTML
							//breaktotal(coln,leveln+1)+=cell
							//call addunits(cell, breaktotal(coln, leveln + 1), VM);
							// breaktotal <- cell
							call htmllib2("ADDUNITS", breaktotal(coln, leveln + 1), cell, VM);
//#endif
						} else {
							if (((breaktotal(coln, leveln + 1)).isnum()) and cell.isnum()) {
								breaktotal(coln, leveln + 1) += cell;
							} else {
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
				oconvx = coldict(coln).f(7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				if (html) {
					cell.replacer(VM, "<br />");
				}

				//if html and cell='' then cell=nbsp

				//and clear it
				breaktotal(coln, leveln) = "";

				//break column
			} else if (coln eq breakcoln) {

				//print the old break value
				cell = breakvalue(coln);
				oconvx = coldict(coln).f(7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				//store the new break value
				breakvalue(coln) = scol(coln);

				//if coln=pagebreakcoln then
				if (pagebreaks.f(coln)) {

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
					tt.replacer("'", "''");
					//if tt='' and html then tt=nbsp

					//insert the page break data
					//replace "'B'" with tt in newhead
					newhead.replacer(pagebreaks.f(coln), tt);
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
				cell = oconv(oldbreakvalue(coln), coldict(coln).f(7));
				if (breakcolns.locateusing(FM, coln, colbreakn)) {
					if (colbreakn lt leveln) {
						cell = "Total";
					}
				}
				// end
			}

			if (coldict(coln).f(10)) {
				if (not(html)) {
					cell = cell.first(coldict(coln).f(10));
					cell = oconv(cell, coldict(coln).f(11));
				} else {
					tx ^= td0 ^ "<th";
					if (not usecols) {
						tx ^= coldict(coln).f(bheadfn);
					}
					if (coldict(coln).f(9) eq "R") {
						tx ^= " style=\"text-align:right\"";
					}
					tx ^= ">";
				}

				tx ^= cell ^ thx;
				//if len(cell) then if cell<>nbsp then anycell=1
				if (cell.len()) {
					anycell = 1;
				}
			}

		} //coln;

		//breakrowexit:
		if (html) {
			tx ^= "</tr>";
		}

		if (detsupp lt 2) {
			//if leveln>1 and not(html) then tx:=fm:underline
		} else {
			if (not html) {
				tx ^= FM ^ underline2;
			}
		}

		//option to suppress the current level
		//or if this is the first record cannot be any totals before it.
		if ((not(anycell) or breakopts.f(leveln).contains("X")) or recn eq 1) {
			tx = storetx;
		}

	} //leveln;

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
			bottomline.move(bottomline2);

			tx(-1) = bottomline2;

			if (foot2) {
				tx(-1) = foot2;
			}

			//close supertable
			tx(-1) = "</td></tr></table>";
		}
		if (tx) {

			gosub printtx();
		}
	} else {
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
	if (str3.len() lt str1.len()) {
		str3 ^= (str1.len() - str2.len()).space();
	}
	for (ii = 1; ii <= str1.len(); ++ii) {
		char1 = str1.at(ii).trim();
		if (char1 ne "") {
			char2 = str3[ii];
			if (char2 eq " ") {
				str3.paster(ii, 1, char1);
			} else {
				//if num(char1) else char1=1
				//if num(char2) else char2=1
				if (char1.isnum() and char2.isnum()) {
					char3 = char1 + char2;
					if (char3 gt 9) {
						char3 = "*";
					}
				} else {
					char3 = char1;
				}
				str3.paster(ii, 1, char3);
			}
		}
	} //ii;

	return;
}

subroutine newheadreplacements() {
	//if index(newhead,'{',1) else return
	//replace any {DICTID} in the heading
	for (ii = 1; ii <= nreplacements; ++ii) {
		dictid = replacements.f(ii);
		tt = "{" ^ dictid ^ "}";
		tt2 = calculate(dictid);
		tt2.replacer("'", "''");
		newhead.replacer(tt, tt2);
	} //ii;
	return;
}

subroutine emailing() {
	if (not emailtoid) {
		return;
	}

	if ((DQ ^ "'").contains(emailtoid[1])) {
		nextemailto = emailtoid.cut(1).pop();
	} else {
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
		body = "@" ^ SYSTEM.f(2);

		tt = emailto;
		tt2 = emailcc;
		if (not tt) {
			tt2.move(tt);
		}

		if (emailsubject) {
			tt3 = emailsubject;
		} else {
			//tt3='EXODUS: ':field(head<1,1,1>,"'",1)
			tt3 = head.f(1, 1, 1).field("'", 1);
			if (tt3.contains(">")) {
				tt3 = field2(tt3, ">", -1);
			}
			tt3 = "EXODUS: " ^ tt3;
		}

		//osclose printfile
		//dont email stuff which has no email address
		if (tt) {
			call sendmail(tt, tt2, tt3, body, "", "", xxerrmsg);
		}

		printptr = 0;
		var("").oswrite(SYSTEM.f(2)) or true;
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
