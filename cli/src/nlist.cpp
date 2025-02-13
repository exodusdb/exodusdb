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

	Perhaps the most readable syntax

    printl(xx.f(6).field("/",2).oconv("D2"));

Comments about style:

	1. Should use int instead of var for "for" loop index variables despite the fact that int doesnt always behave
		precisely like pick integers, speed is important in this program and C++ allows us access to raw power when we need it.

	2. Many for loops should be converted to use range()

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

	Notably in Exodus libraries, which are separately edited and compiled callable functions,
	class global variables are only initialised on the first call to the function.
	If the function is performed/executed then class member variable are initialised on each perform/execute.

Example command line:

		list ads brand_code brand_name with brand_code 'XYZ' (N)

	If using a shell like bash then quotes must be escaped or wrapped
	and () should be done with {}

		list ads brand_code brand_name with brand_code "'XYZ'" {N}

	or

		list ads brand_code brand_name with brand_code '"XYZ"' {N}

	Type just list by itself to get a summary of its syntax

*/

#include <exodus/library.h>
libraryinit()

#include <exodus/printtx.hpp>

//bool showborder;//num
//int headtabperpage;//num
//var colorprefix;
var nblocks;//num
var blockn;//num
var fromtimestamp;
bool usecols;//num
//var decimalchar;
//bool html;//num
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
//var printptr;//num
//var cssver;//num
var sentencex;
int maxncols;//num
int maxnrecs;
var preselect;//num
var keylist;//num
var crtx;//num
var idsupp;//num
var dblspc;//num
var detsupp;//num
bool gtotsupp;//num
bool gtotreq;//num
var nobase;//num
var rawtable;//num
var silent;//num
var dictvoc;
dim colname;
dim coldict;
dim coldict2;
dim mcol;
dim pcol;
dim ccol;
dim scol;
dim icol;
var bheadfn;//num
//int coln;//num
//var head;
var foot2;
int nbreaks;//num
var breakcolns;
var breakopts;
var pagebreaks;
var headtab;
var hrown;//num
var hcoln;//num
var replacements;
int nreplacements;//num
//var topmargin;//num
var totalflag;//num
var breakonflag;//num
var anytotals;//num
var multirowcolhdg;//num
var limits;
int nlimits;//num
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
var sortselect;
int wordn;//num
var onlyauthorised;//num
var word;
var filename;
var dictfilename;
var xx;
//var printtxmark;
var nextword;
var dictrec;
var limitx;
var lastword;
var title;
var value;
var rec;
int tcoln;//num
//int nn;
//int ii;//num
var tt2;
var charx;
int charn;//num
int ncols;//num
//int breakn;//num
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
//var style;
var coln2;//num
int vmcount;//num
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
int limitn;//num
int recn;//num
//bool ok;//num
var readerr;//num
//var fns;
//int nfns;
//int nmvs;//num
//int fnn;//num
//int fn;
var limitvals;
//var mvx;
var newhead;
//var printfilename;
int breakleveln;//num
int leveln;//num
int previousmarklevel;//num
int newmarklevel;//num
var str1;
var str2;
var str3;
var tx1;
var oconvx;
//int bodyln;//num
//var bottomline;
var lastrecord;//num
//var printfile;
int storewordn;
int storecharn;
var storeword;
var storedictrec;
int startcharn;//num
var searchchar;
//var letterhead;
var wordexpanded;//num
int breakcoln;
var storetx;
//var anycell;//num
var lastblockn;
var underline2;
var cell;//num
//var colbreakn;
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

//int pagelns;
//int realpagen;//num
//int pagen;//num
//var newpage;//num
//var rfmt;
//var foot;
//int ntxlns;//num
//int nbodylns;//num
//var ptx_temp;
//var headx;
//var newpagetag;
//var ptx_css;
//var stylennx;//num
//var htmltitle;
//var head_or_foot;
//var footx;
//var head1;
//var newoptions;
//int printtx_ii;//num
//var spaceoptionsize;

let ulchar = "-";

function main() {

	bool showborder = false;
	bool headtabperpage = 1;
	const char* colorprefix = "\x19";

	// Compile list with optimisation. Its inner loops benefit from efficiency

	if (USERNAME == "EXODUS") {
		var(SENTENCE).oswrite("nlist") or lasterror().errputl("nlist:");
		logputl();
		logputl(SENTENCE);
	}

	//for printtx

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
		[	ENDING, ENDS	;
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

	nblocks = 0;
	blockn = 0;

	// Ensure default db connection works
	if (not connect()) {
		loglasterror();
		abort("");
	}

	if (LISTACTIVE) {
		fromtimestamp = date().timestamp(SYSTEM.f(25));
	} else {
		fromtimestamp = timestamp();
	}

	// Use <COL> for hiding non-totalled cols in det-supp (slow)
	usecols = 0;

//	if (BASEFMT[2] == "C") {
//		decimalchar = ",";
//	} else {
//		decimalchar = ".";
//	}

	html = SYSTEM.f(2).lcase().ends("htm") or OPTIONS.contains("h");

	// Extract pdf options. Portrait and Landscape
	stationery = OPTIONS.replace("[^12PL]"_rex, "").first();

	if (html) {

		// Prevent printtx from creating a random .htm file for output
		// and make it output to stdout
		if (not SYSTEM.f(2))
			SYSTEM(2) = "-";

		tt = SYSTEM.f(2);
		tt.replacer(".txt", ".htm");
		SYSTEM(2) = tt;

		td0 = "";
		nbsp = "&nbsp;";
		td = td0 ^ "<td>";
		tdx = "</td>";
		th = td0 ^ "<th>";
		thx = "</th>";
		tr = "<tr>";
		trx = "</tr>";

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

////////
//init1:
////////

	thcolor = SYSTEM.f(46, 1);
	tdcolor = SYSTEM.f(46, 2);
	reportfont = SYSTEM.f(46, 3);

	//@sentence='LIST 10 SCHEDULES BY VEHICLE_CODE with vehicle_code "kjh kjh" VEHICLE_NAME BREAK-ON VEHICLE_CODE BREAK-ON VEHICLE_CODE TOTAL PRICE (SX)'
	//@sentence='list markets code name'

	printptr = 0;
	cssver = 2;

	sentencex = SENTENCE;
	DICT = "";
	maxncols = 256;

	maxnrecs = 0;
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
	gtotsupp = false;
	gtotreq = false;
	nobase = "";
	rawtable = 0;
	silent = 0;

	if (not dictvoc.open("DICT.voc")) {
		//createfile("DICT.voc");
		if (not createfile("DICT.voc")) {
			abort(lasterror());
		}
		if (not dictvoc.open("DICT.voc")) {
			abort(lasterror());
		}
	}

	if (not(tt.read(dictvoc, "@ID"))) {
		"F^0^TABLE_NAME^S^1^^^^L^20^^VARCHAR"_var.write(dictvoc, "TABLE_NAME");
		"F^0^COLUMN_NAME^S^2^^^^L^20"_var.write(dictvoc, "COLUMN_NAME");
		"G^^TYPE FMC PART HEADING SM CONV JUST LEN MASTER_FLAG^^^^^^^^^^^^^^^^^^^^^^^^^0"_var.write(dictvoc, "@CRT");
		"F^0^Ref.^S^0^^^^T^20"_var.write(dictvoc, "@ID");
		"F^1^TYPE^S^0^^^^L^4"_var.write(dictvoc, "TYPE");
		"F^2^FMC^S^0^^^^R^3"_var.write(dictvoc, "FMC");
		"F^5^PART^S^0^^^^R^2"_var.write(dictvoc, "PART");
		"F^3^HEADING^S^0^^^^T^20"_var.write(dictvoc, "HEADING");
		"F^4^SM^S^0^^^^L^4"_var.write(dictvoc, "SM");
		"F^7^CONV^S^0^^^^L^9"_var.write(dictvoc, "CONV");
		"F^9^JST^S^0^^^^L^3"_var.write(dictvoc, "JUST");
		"F^10^LEN^S^0^^^^R^3^^LENGTH"_var.write(dictvoc, "LEN");
		"F^28^MST^S^^^BYes,^^L^4"_var.write(dictvoc, "MASTER_FLAG");
	}

	colname.redim(maxncols);
	coldict.redim(maxncols);
	mcol.redim(maxncols);
	pcol.redim(maxncols);
	ccol.redim(maxncols);
	scol.redim(maxncols);
	icol.redim(maxncols);
	bheadfn = 15;
	int coln = 0;
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

	// Remove AUTHORISED since it can be filtered locally
	// and avoid triggering a 2 stage select
	tt = " AND WITH AUTHORISED AND ";
	onlyauthorised = sentencex.contains(tt);
	if (onlyauthorised) {
		sentencex.replacer(tt, " AND ");
	}

//initphrase:
/////////////

	// First phrase must be LIST|SORT [SORT] [NNNN] filename [itemid ...]

	wordn = 0;
	sortselect = "SELECT";

	// Note that getword gets 'word' and 'nextword' to allow look ahead
	gosub getword();

	if (word != "LIST" and word != "SORT" and word != "NLIST" and word != "OLIST") {
		call mssg(word.quote() ^ " first word must be LIST or SORT");
		abort();
	}

	// Decide initial ordering SELECT (No order) or SSELECT (by key)
	if (word == "SORT")
		sortselect = "SSELECT";
	else
		sortselect = "SELECT";

	// 2nd word is SORT or number of records or filename
	gosub getword();

	// SORT changes SELECT to SSELECT which sorts
	// finally (i.e. after any other BY phrases) by ID
	if (word == "SORT") {
		if (sortselect.field(" ", 1) == "SELECT")
			// Prefix 'S' to change 'SELECT' to 'SSELECT'
			sortselect.prefixer("S");
		gosub getword();
	}

	// Limit number of records if 2nd word is digits like 9999
	if (word.match("^\\d+$")) {
		maxnrecs = word;
		sortselect ^= " ";
		sortselect ^= maxnrecs;
		gosub getword();
	}

	if (not word) {
		call mssg("FILE NAME IS REQUIRED");
		abort();
	}

	// Filename
	filename = word;
	if (word == "DICT") {
		gosub getword();
		filename = "DICT." ^ word;
	}
	if (filename.starts("DICT.")) {
		dictfilename = "VOC";
	} else {
		dictfilename = filename;
	}
	if (not(DICT.open("DICT." ^ dictfilename))) {
		//commented so that list dict_clients gives dict.voc columns
		//crtx=1
		DICT = dictvoc;
	}
	sortselect ^= " " ^ word;

	// Decide if we wil filter by AUTHORISED
	if (xx.read(DICT, "AUTHORISED")) {
		onlyauthorised = 1;
	}

	// NEOSYS custom code TODO remove to SYSTEM
	if (html) {

		if (var("PLANS,SCHEDULES,ADS,BOOKING_ORDERS,VEHICLES,RATECARDS").locateusing(",", filename, xx)) {
			printtxmark = "Media Management";
		}

		if (var("JOBS,JOB_ORDERS,JOB_INVOICES").locateusing(",", filename, xx)) {
			printtxmark = "Job Management";
		}

		if (var("CHARTS,RECURRING").locateusing(",", filename, xx)) {
			printtxmark = "Financial";
		}

	}

	// Get any specfic keys.
	// which are value words starting with 0-9, " or '
	while (nextword.len() and (nextword.isnum() or nextword.starts("'") or nextword.starts(DQ))) {
		keylist = 1;
		sortselect ^= " " ^ nextword;
		gosub getword();
	}

	//TRACE(sortselect)

nextphrase:
///////////

	gosub getword();
	if (word == "") {
		goto x1exit;
	}

phraseinit:
///////////

	if (word == "GETLIST") {
		gosub getword();
		getlist("" ^ word ^ " (S)");

	} else if (word == "AND" or word == "OR") {
		sortselect ^= " " ^ word;

	} else if (word == "(" or word == ")") {
		sortselect ^= " " ^ word;

	} else if (word == "BY" or word == "BY-DSND") {
		sortselect ^= " " ^ word;
		gosub getword();
		sortselect ^= " " ^ word;

		// Determine if limited nrecs sorted by mv field (which needs preselect)
		if ((maxnrecs and not(preselect)) and DICT) {
			if (dictrec.readc(DICT, word)) {
				preselect = dictrec.f(4).starts("M");
			}
		}

	} else if (((word == "WITH NOT" or word == "WITH") or word == "WITHOUT") or word == "LIMIT") {

		if (nextword == "AUTHORISED") {
			onlyauthorised = 1;
			gosub getword();
			if (sortselect.ends(" AND")) {
				sortselect.cutter(-4);
			}
			if (nextword == "AND") {
				gosub getword();
			}
			goto nextphrase;
		}

		sortselect ^= " " ^ word;

		limitx = word == "LIMIT";
		if (limitx) {
			nlimits += 1;
		}

		gosub getword();

		// NO/EVERY
		if ((word == "NOT" or word == "NO") or word == "EVERY") {
			sortselect ^= " " ^ word;
			gosub getword();
		}

		// Field or NO
		sortselect ^= " " ^ word;
		if (limitx) {
			limits(1, nlimits) = word;
			if (not(dictrec.readc(DICT, word))) {
				call mssg(word.quote() ^ " is not a valid dictionary item (1)");
				abort();
			}
			tt = dictrec.f(4).field(".", 1);
			if (not tt.starts("M")) {
				call mssg(word.quote() ^ " limit must be a multivalued dict item");
				abort();
			}
			limits(4, nlimits) = tt;
		}

		// Negate next comparision
		if (var("NOT,NE,<>").locateusing(",", nextword, xx)) {
			nextword = "NOT";
			gosub getword();
			sortselect ^= " " ^ word;
		}

		// Comparision
		if (var("MATCH,EQ,,NE,GT,LT,GE,LE,<,>,<=,>=,=,[],[,]").locateusing(",", nextword, xx)) {
			//only EQ works at the moment
			gosub getword();
			sortselect ^= " " ^ word;
			if (limitx) {
				limits(2, nlimits) = word;
			}
		}

		// with xx between y and z
		// with xx from y to z
		if (nextword == "BETWEEN" or nextword == "FROM") {
			gosub getword();
			sortselect ^= " " ^ word;
			gosub getword();
			sortselect ^= " " ^ word;
			gosub getword();
			sortselect ^= " " ^ word;
			gosub getword();
			sortselect ^= " " ^ word;

		} else {

			// parameters
			while (nextword != "" and (nextword.isnum() or nextword.starts(DQ) or nextword.starts(SQ))) {

				gosub getword();
				sortselect ^= " " ^ word;
				if (limitx) {
					if (word.starts(DQ) or word.starts(SQ)) {
						if (word.starts(word.last())) {
							word.cutter(1).popper();
						}
					}
					if (word == "") {
						word = "\"\"";
					}
					limits(3, nlimits, -1) = word;
				}
			}//loop;

		}

	} else if (word == "BREAK-ON") {
		tt = coln + 1;
		breakcolns.prefixer(tt ^ _FM);
		breakopts.prefixer(_FM);
		nbreaks += 1;
		breakonflag = 1;

	} else if (word == "GRAND-TOTAL") {
		//zzz Throw away the grand total options for the time being
		gosub getword();
		gtotreq = true;

	} else if (word == "NO-BASE") {
		nobase = 1;

	//"DET-SUPP"
	} else if (word == "DS" or word == "DET-SUPP") {
		detsupp = 1;

	//"DET-SUPP"
	} else if (word == "DS2" or word == "DET-SUPP2") {
		detsupp = 2;

	//"GTOT-SUPP"
	} else if (word == "GTS" or word == "GTOT-SUPP") {
		gtotsupp = true;

	//"MULTIROW-COLHDG"
	} else if (word == "DS" or word == "MULTIROW-COLHDG") {
		multirowcolhdg = 1;

	} else if (word == "TOTAL") {
		totalflag = 1;

	} else if (word == "USING") {
		gosub getword();
		dictfilename = word;
		if (not DICT.open("DICT." ^ dictfilename)) {
			abort(lasterror());
		}
		sortselect ^= " USING " ^ dictfilename;

	} else if (word == "HEADINGTABLE") {

		lastword = word;

		// Layout in html table in pairs of cells like <td>title:</td><td>value</td>
		// each column number in the command corresponds to a PAIR of columns in html

		// Examples:
		//
		// in next row of whatever the last column is (1 to start)
		// HEADINGTABLE "Client:" "XYZ"
		//
		// in next row of column 2
		// HEADINGTABLE 2 "Client:" "XYZ"
		//
		// in column 2 row 3
		// HEADINGTABLE 2 3 "Client:" "XYZ"
		//
		// titled from dictionary, value from runtime break
		// like break-on with B1,1 option but doesnt get tested everytime so is faster
		// but assumes that the field is dependent on a field that does have break-on
		// HEADINGTABLE "{CLIENT_NAME}"
		//
		// overidding the dictionary title
		// HEADINGTABLE "Client" "{CLIENT_NAME}"

		if (nextword.match("^\\d+$")) {
			gosub getword();

			//reset to row 1 if column changed
			if (hcoln != word) {
				hrown = 1;
			}

			hcoln = word;
		}

		if (nextword.match("^\\d+$")) {
			gosub getword();
			hrown = word;
		}

		gosub getquotedword2();

		if ((DQ ^ "'").contains(nextword.first())) {
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

		// Automatic labelling with dictionary title
		if (word.starts("{")) {
			tt = word.cut(1).pop();
			replacements(-1) = tt;
			nreplacements += 1;
			if (not(rec.readc(DICT, tt))) {
				if (not(rec.readc(dictvoc, tt))) {
					call mssg(tt.quote() ^ " is not a valid dictionary item (2)");
					abort();
				}
			}
			if (title == "") {
				title = rec.f(3) ^ nbsp ^ ":";
			}
		}

		tcoln = (hcoln - 1) * 2 + 1;
		// Find the next empty row in the heading
		while (headtab.f(hrown, tcoln) or headtab.f(hrown, tcoln + 1)) {
			hrown += 1;
		}//loop;

		// Prevent trailing colon folding onto following line
		if (title.ends(" :")) {
			title.paster(-2, 2, "&nbsp;:");
		}

		// Any existing doubled single quotes are removed to avoid double doublimg
		title.replacer("''", "'");
		// Double any single quotes to avoid them being understood as options
		title.replacer("'", "''");

		headtab(hrown, tcoln) = title.convert(_FM _VM, "  ");
		headtab(hrown, tcoln + 1) = value;
		hrown += 1;

	} else if (word == "HEADING" and dictrec.f(3) != "HEADING") {

		gosub getquotedword();
		head ^= word;

		// Remove page numbering options from headings
		if (html) {
			head.replacer("Page 'P'", "");
			head.replacer("Page 'P", "'");
		}

	} else if (word == "FOOTING") {
		gosub getquotedword();
		foot2 ^= word;

	//justlen
	} else if (word == "JL" or word == "JUSTLEN") {
		if (not coln) {
			call mssg("JUSTLEN/JL must follow a column name");
			abort();
		}
		// Skip if detsupp2 and column is being skipped
		if (not(coldict[coln].unassigned())) {
			gosub getquotedword();
			coldict[coln](9) = word.first();
			coldict[coln](10) = word.cut(2);
			coldict[coln](11) = word;
		}

	//colhead
	} else if (word == "CH" or word == "COLHEAD") {
		if (not coln) {
			call mssg("COLHEAD/CH must follow a column name");
			abort();
		}
		gosub getquotedword();
		// Skip if detsupp2 and column is being skipped
		if (not(coldict[coln].unassigned())) {
			word.converter("|", _VM);
			coldict[coln](3) = word;
		}

	} else if (word == "OC" or word == "OCONV") {
		if (not coln) {
			call mssg("OCONV/OC must follow a column name");
			abort();
		}
		gosub getquotedword();
		// Skip if detsupp2 and column is being skipped
		if (not(coldict[coln].unassigned())) {
			if (html) {
				word.replacer("[DATE]", "[DATE,*]");
			}
			coldict[coln](7) = word;
		}

	} else if (word == "ID-SUPP" or word == "IS") {
		idsupp = 1;

	} else if (word == "DBL-SPC" or word == "DB") {
		dblspc = 1;

	} else if (word == "EMAIL_TO") {
		gosub getword();
		emailtoid = word;

	} else if (word == "EMAIL_CC") {
		gosub getword();
		if ((DQ ^ "'").contains(word.first())) {
			emailcc = word.cut(1).pop();
			nextemailcc = emailcc;
		} else {
			emailccid = word;
		}

	} else if (word == "EMAIL_SUBJECT") {
		gosub getword();
		if ((DQ ^ "'").contains(word.first())) {
			emailsubject = word.cut(1).pop();
			nextemailsubject = emailsubject;
		} else {
			emailsubjectid = word;
		}

	} else if (dictrec) {

		if (var("FSDIA").contains(dictrec.f(1))) {

			// PickOS format dictionary
			//if index('DI',dictrec<1>,1) then
			// call dicti2a(dictrec)
			// end
			//if dictrec<1>='A' then dictrec<1>='F'

			// Suppress untotalled columns if doing detsupp2
			if (detsupp == 2 and (not(totalflag or breakonflag))) {
				if (var("JL,JUSTLEN,CH,COLHEAD,OC,OCONV").locateusing(",", nextword, xx)) {
					gosub getword();
					gosub getword();
				}
				goto dictrecexit;
			}

			coln += 1;
			colname[coln] = word;

			// Increase column width if column title needs it
			{
				int nn = dictrec.f(3).fcount(_VM);
				for (int ii = 1; ii <= nn; ++ii) {
					tt = dictrec.f(3, ii);
					if (dictrec.f(10) and tt.len() > dictrec.f(10)) {
						dictrec(10) = tt.len();
					}
				} //ii;
			}

			dictrec(bheadfn) = "";

			if (detsupp < 2) {
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

			// Total required ?
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
				if (tt == "[DATE,4]") {
					tt = "[DATE,4*]";
				}
				dictrec(7) = tt;
				if (tt == "[DATE,*]") {
					dictrec(9) = "R";
				}
			}
			coldict[coln] = dictrec;

			// Store the format in a convenient place
			tt = "";
			if (not html) {
				tt = coldict[coln].f(9) ^ "#" ^ coldict[coln].f(10);
			}
			coldict[coln](11) = tt;

			// This could be a break-on column and have break-on options
			if (breakonflag) {
				coldict[coln](13) = 1;
				breakonflag = 0;

				if (nextword.starts(DQ)) {
					gosub getword();

					//zzz break  options
					int tt = word.index("B");
					if (tt) {

						// Suppress columns that appear in the heading
						coldict[coln](10) = 0;

						// Determine B99.99 format for row and col
						tt2 = "";
						int wordlen = word.len();
						for (tt += 1; tt <= wordlen; ++tt) {
							charx = word.at(tt);

							if (not var("0123456789,").contains(charx))
								break;

							tt2 ^= charx;
						} //tt;

						// Build table heading table if rown/column givem
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
							headtab(hrown, tcoln) = coldict[coln].f(3).convert(_VM, " ") ^ nbsp ^ ":";
							headtab(hrown, tcoln + 1) = "'B" ^ tt2 ^ "'";
							hrown += 1;
						}

						pagebreaks(coln) = "'" "B" ^ tt2 ^ "'";

					}
					breakopts(1) = word;
				}
			}

		}
dictrecexit:
	{}
	}
	else if (word == "IGNOREWORD") {
		gosub getword();
		ignorewords(1, -1) = word;

	} else {
		tt = word.quote() ^ " is an unrecognized word.";
		if (isterminal()) {
			tt ^= " Replace with? (Enter to cancel):";
		}
		let oldword = word;
		call mssg(tt, "RCE", word, "");
		if (word == oldword or word == "\x1B") {
			abort();
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

	// If no columns selected then try to use default @crt or @lptr group item
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

	// Insert the @id column
	if (not(idsupp) or not(ncols)) {

		ncols += 1;

		// Move the columns up by one to make space for a new column 1
		for (coln = ncols; coln >= 2; --coln) {
			coldict[coln] = coldict[coln - 1];
			colname[coln] = colname[coln - 1];
		}

		// Set column 1
		colname[1] = "@ID";
		if (not(coldict[1].readc(DICT, "@ID"))) {
			if (not(coldict[1].readc(dictvoc, "@ID"))) {
				coldict[1] = "F^^Ref^^^^^^L^15"_var;
			}
		}
		if (html) {
			tt = "";
		} else {
			tt = coldict[1].f(9) ^ "#" ^ coldict[1].f(10);
		}
		coldict[1](11) = tt;

		// Increment the list of breaking columns by one as well
		for (int breakn : range(1, nbreaks)) {
			breakcolns(breakn) = breakcolns.f(breakn) + 1;
		}

		// and the page break colns
		if (pagebreaks) {
			pagebreaks = _FM ^ pagebreaks;
		}

	}

	breakcolns.trimmerlast(_FM);
	breakopts.trimmerlast(_FM);

	// Split coldict into two dimensional array for speed
	const int maxdictfn = 20;
	coldict2.redim(ncols, maxdictfn);
	for (auto coln : range(1, ncols)) {
		var& dictrec = coldict[coln];
		for (auto fn : range(1, maxdictfn)) {
			coldict2(coln, fn) = dictrec.f(fn);
		}
	}

	// Make underline and column title underline
	if (not html) {
		underline = "";
		colul = "";
		for (int coln : range(1, ncols)) {
			if (coldict2(coln, 10)) {
				if (coldict2(coln, 12)) {
					tt = ulchar;
				} else {
					tt = " ";
				}
				underline ^= tt.str(coldict2(coln, 10)) ^ " ";
				colul ^= ulchar.str(coldict2(coln, 10)) ^ " ";
			}
		} //coln;
		bar = ulchar.str(colul.len() - 1);
	}

////////
//init2:
////////

	tx = "";

	if (not srcfile.open(filename, "")) {
		abort(lasterror());
	}

	breakcount.redim(nbreaks + 1);
	breaktotal.redim(maxncols, nbreaks + 1);
	breakvalue.redim(maxncols);
	oldbreakvalue.redim(maxncols);

	breakvalue = "";//dim
	oldbreakvalue = "";//dim
	breakcount = "";//dim
	breaktotal = "";//dim

	// Build the column headings
	colhdg = "";
	coltags = "";
	style = "";
	coln2 = 0;
	vmcount = 0;
	thproperties = "";

	for (int coln : range(1, ncols)) {

		// Suppress drilldown if no totals or breakdown
		if (not(anytotals) or not(nbreaks)) {
			coldict2(coln, bheadfn) = "";
		}

		if (coldict2(coln, 10)) {
			if (html) {
				coln2 += 1;

				// Suppressing non-totalled columns may not work well with multi-row colhdg
				tt = coldict2(coln, bheadfn);

				thproperties(coln2) = tt;

				// Without the MULTIROW_COLHDG keyword,
				// vm indicates folding, \\ indicates rows in column headings
				tt = coldict2(coln, 3);
				if (not multirowcolhdg) {
					tt.replacer(_VM, "<br />");
				}
				tt.replacer("\\\\", _VM);

				colhdg(coln2) = tt;
				if (tt.count(_VM) > vmcount) {
					vmcount = tt.count(_VM);
				}

				coltags(-1) = " <col";
				align = coldict2(coln, 9);
				if (align == "R") {
					align = "right";
					coltags ^= " style=\"text-align:right\"";
				} else if (align == "T") {
					align = "left";
					coltags ^= " style=\"text-align:left\"";
				} else {
					align = "center";
				}
				if (usecols) {
					coltags ^= coldict2(coln, bheadfn);
				}
				coltags ^= " />";

				// nth child style column justification in case <col> doesnt work like on FF
				if (align) {
					// works per table if the table is assigned a class (.maintable) here
					style ^= "table.exodustable td:nth-child(" ^ coln2 ^ "){text-align:" ^ align ^ "}" _EOL;
				}

			} else {
				// L# R# C# T#
				for (int ii = 1; ii <= 9; ++ii) {
					colhdg(ii) = colhdg.f(ii) ^ oconv(coldict2(coln, 3).f(1, ii), coldict2(coln, 11)) ^ " ";
				}
			}
		}
	} //coln;

	if (style) {
		style = "<style type=\"text/css\">" _EOL ^ style ^ "</style>" _EOL;
	}

	// Convert to html with colspan/rowspan where necessary and (Base) as currcode
	if (html) {
		//var basecurrcode = sys.company.f(3)
		//var basecurrcode = "";
		// basecurrencycode stored in SYSTEM in initcompany
		let basecurrcode = SYSTEM.f(134);
		call htmllib2("COLROWSPAN", colhdg, thproperties, nobase, basecurrcode);
	}

	// Trim off blank lines (due to the 9 above)
	if (html) {

		var htmlcode;
		call htmllib2("GETSORTJS", htmlcode);

		if (not rawtable) {
			call getmark("CLIENT", html, clientmark);
			htmlcode ^= clientmark ^ _EOL;
		}

		htmlcode ^= "<table class=\"exodustable\"";
		htmlcode ^= " cellspacing=\"0\"";
		htmlcode ^= " style=\"font-size:66%";
		htmlcode ^= ";page-break-after:avoid";
		htmlcode ^= "\">";
		htmlcode ^= _FM "<colgroup>" ^ coltags ^ "</colgroup>";
		// <thead> may be hardcoded elsewhere for page heading
		//!!!if you change it here, search and change it there too
		htmlcode ^= _FM "<thead style=\"cursor:pointer\" onclick=\"sorttable(event)\">";
		posttheadmark = "<postthead/>";
		if (headtab) {
			htmlcode ^= posttheadmark;
		}

		htmlcode ^= colhdg ^ _FM "</thead>";
		htmlcode ^= _FM "<tbody>";
		htmlcode.move(colhdg);

		// Allow for single quotes
		colhdg.replacer("'", "''");

	} else {
		colhdg.trimmerlast(" " _FM);
	}

	// Heading options

	if (head == "" and not(rawtable)) {
		head = filename ^ var(10).space() ^ " 'T'";
	}

	if (html) {
		head.replacer(_FM, "<br />");
		// Div to make header as wide as body
		// the report title
		if (head) {
			head = "<h2{%20}style=\"margin:0px;text-align:center;padding-top:0px\">" ^ head ^ "</h2>";
		}

		// Supertable to ensure any heading is as wide as the body of the report
		if (showborder) {
			head(-1) = "<table{%20}border=1{%20}style=\"border-width:3px\"{%20}cellpadding=0{%20}cellspacing=0{%20}align=center><tr><td>";
		} else {
			head(-1) = "<table{%20}align=center><tr><td>";
		}
	}

	// Footing options

	if (headtab) {
		tt = "<table id=\"headtab0\" width=100% align=center cellpadding=3>";

		// Older browser <col> styling
		headtabcols = " <col style=\"text-align:left\"/>" _VM "<col style=\"text-align: left;font-weight:bold\"/>";
		// Allow 8 max pair of headtab columns
		headtabcols = (headtabcols ^ _VM).str(8);
		headtabcols.popper();

		tt ^= "<colgroup>" _EOL ^ headtabcols.replace(_VM, _EOL) ^ "</colgroup>";

		// Style columns where '<col>' not supported.
		call htmllib2("CONVCSS", headtabstyle, "headtab0", headtabcols);
		style ^= _EOL ^ headtabstyle;

		tt ^= "<colgroup>" _EOL ^ headtabcols.replace(_VM, _EOL) ^ "</colgroup>";

		tt ^= _EOL "<TBODY>";
		call htmllib2("TABLE.MAKE", headtab, tt, "");
		headtab.replacer("</TR>", "</TR>" _EOL);
		if (headtabperpage) {
			colhdg.replacer(posttheadmark, tr ^ td0 ^ "<th style=\"background-color:white\" colspan=" ^ ncols ^ ">" ^ headtab ^ thx ^ trx);
		} else {
			headtab.replacer(posttheadmark, "");
			head ^= _FM ^ headtab ^ _FM;
		}
	}

	if (dblspc) {
		head ^= _FM;
	}
	if (not html) {
		head ^= _FM ^ colul;
	}
	head ^= _FM ^ colhdg;
	if (not html) {
		head ^= _FM ^ colul;
	}
	if (dblspc) {
		head ^= _FM;
	}

	head.prefixer(FM);

	orighead = head;

	// Work out assoc mv fns for limits
	if (nlimits) {
		call pushselect(0, v69, v70, v71);
		select(DICT);
nextdict:
		if (readnext(dictid)) {
			if (not(dictrec.readc(DICT, dictid))) {
				goto nextdict;
			}
			if (dictrec.f(1) != "F") {
				goto nextdict;
			}
			for (int limitn : range(1, nlimits)) {
				if (dictrec.f(4).field(".", 1) == limits.f(4, limitn)) {
					tt = dictrec.f(2);
					if (tt) {
						if (not(limits.f(5, limitn).locateusing(_SM, tt, xx))) {
							limits(5, limitn, -1) = dictrec.f(2);
						}
					}
				}
			} //limitn;
			goto nextdict;
		}
		call popselect(0, v69, v70, v71);
	}

//////////
//initrec:
//////////
	if (sortselect.count(" ") > 2 or keylist) {

		// Preselect if sselect is by any mv fields since that ignores maxnrecs
		if (not(LISTACTIVE)) {
			if (preselect) {
				call xselect(sortselect.field(" ", 1, 3) ^ " (SR)");
			}
			maxnrecs = 0;
		}

		call xselect(sortselect ^ " (SR)");

		if (not LISTACTIVE) {
			// The words "No record" is hardcoded in autorun and maybe elsewhere
			// so if you change it here, CHANGE IT THERE TOO
			call mssg("No records found");
			abort();
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
			call xselect("SELECT " ^ filename ^ " (SR)");
		}
	}
	recn = 0;
	RECCOUNT = 0;

////////
nextrec:
////////

	//TRACE(__PRETTY_FUNCTION__)
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

	// Limit number of records
	if (maxnrecs) {
		if (recn >= maxnrecs) {
			clearselect();
		}
	}

	// Readnext record, key and mv

	FILEERRORMODE = 1;
	FILEERROR = "";

	RECORD = "";
	//TRACE(__PRETTY_FUNCTION__)
	if (not readnext(RECORD, ID, MV)) {
		FILEERRORMODE = 0;
		if (STATUS) {
			tx = "*** Fatal Error " ^ FILEERROR.f(1) ^ " reading record " ^ ID ^ " ***";
			gosub printtx();
			abort();
		}
		if (FILEERROR.f(1) == 421) {
			tx = "Operation aborted by user.";
			gosub printtx();
			abort();
		}
		if (FILEERROR and FILEERROR.f(1) != 111) {
			tx = "*** Error " ^ FILEERROR.f(1) ^ " reading record " ^ ID ^ " ***";
			gosub printtx();
			readerr += 1;
			abort();
		}
		goto x2exit;
	}

	// Skip side channel data which has keys starting '%'. and always ending '%'?
	if (ID.starts("%")) {
		goto nextrec;
	}

	// The select (R) option should have provided RECORD already
	if (not RECORD.len() and not RECORD.readc(srcfile, ID)) {
		goto nextrec;
	}

	// Skip records not AUTHORISED
	if (onlyauthorised) {
		if (not(calculate("AUTHORISED"))) {
			goto nextrec;
		}
	}

	// Designed to filter multivalues which are not selected properly
	// unless sorted "by"
	if (limits) {

		// For each limit pass through record deleting all unwanted multivalues
		for (int limitn : range(1, nlimits)) {

			// Find maximum mv number for the associated group of fns
			let fns = limits.f(5, limitn).convert(_SM, _FM);
			int nmvs = 0;
			for (const int fn : fns) {
				tt = RECORD.f(fn);
				if (tt.len()) {
					tt = tt.fcount(_VM);
					if (tt > nmvs) {
						nmvs = tt;
					}
				}
			} //fn;

			limitvals = calculate(limits.f(1, limitn));
			for (int mvx = nmvs; mvx >= 1; --mvx) {
				tt = limitvals.f(1, mvx);
				if (tt == "") {
					tt = "\"\"";
				}
				if (not(limits.f(3, limitn).locateusing(_SM, tt, xx))) {
					for (int fn : fns)
						RECORD.remover(fn, mvx);
				}
			} //mvx;
		} //limitn;

	}

	recn += 1;
	RECCOUNT += 1;

//////////
//recinit:
//////////

	if (recn == 1) {
		head.move(newhead);
		gosub newheadreplacements();
		newhead.move(head);
	}

	if (not(silent) and not(printfilename.unassigned())) {
		if (recn <= 2) {
			logputl();
		}
		if (printfilename and not(mod(recn, 10))) {
			//first recn will be 2
			//similar in recinit and x2exit
			if (TERMINAL) {
				logput(AT(-40), recn, ". ", ID, " ", MV);
				osflush();
			}
		}
	}

	//if det-supp2 then zero width initialiser columns" will not be called
	//unless you put TOTAL dictid
	//get the data from the record into an array of columns

	for (int coln : range(1, ncols)) {

		if (coldict2(coln, 9) == "T" and not(html)) {

			// T#
			mcol[coln] = oconv(calculate(colname[coln]), coldict2(coln, 11));

		} else {

			mcol[coln] = calculate(colname[coln]);

			if (html) {
				mcol[coln].replacer(TM, "<br />");
			}
		}

		pcol[coln] = 1;
		ccol[coln] = 7;
		scol[coln] = mcol[coln];
	} //coln;

	// Break subtotals
	// Detect most major level to break
	if (recn == 1) {
		// Print breaks will not actually print before the first record
		// but it needs to set the various break values
		breakleveln = nbreaks;
	} else {
		for (leveln = nbreaks; leveln >= 1; --leveln) {
			coln = breakcolns.f(leveln);

			if (scol[coln] != breakvalue[coln])
				break;

		} //leveln;
		breakleveln = leveln;
	}

	gosub printbreaks();

	oldbreakvalue = breakvalue;//dim

	previousmarklevel = 0;

////////
recexit:
////////

	// Remove appropriate value from multi-valued column(s)
	newmarklevel = 0;
	for (int coln : range(1, ncols)) {
		if (ccol[coln] >= previousmarklevel) {
			icol[coln] = mcol[coln].substr2(pcol[coln], ccol[coln]);
			scol[coln] = icol[coln];
		}
		if (ccol[coln] > newmarklevel) {
			newmarklevel = ccol[coln];
		}
	} //coln;

	// Break totals - add at the bottom level (1)
	for (int coln : range(1, ncols)) {
		// Only totalled columns
		if (coldict2(coln, 12)) {
			if (icol[coln]) {
				if (html) {
					//breaktotal(coln,1)+=i.col(coln)
					//call addunits(icol[coln], breaktotal(coln, 1), VM);
					// breaktotal <- icol
					call htmllib2("ADDUNITS", breaktotal(coln, 1), icol[coln], _VM);
				} else {
					if (breaktotal(coln, 1).isnum() and icol[coln].isnum()) {
						breaktotal(coln, 1) += icol[coln];
					} else {
						if (colname[coln] == "DATEGRID") {
							str1 = icol[coln];
							str2 = breaktotal(coln, 1);
							gosub addstr();
							breaktotal(coln, 1) = str3;
						}
					}
				}
			}
			breakcount[1] += 1;
			icol[coln] = "";
		}
	} //coln;

	if (detsupp < 2) {

		if (anytotals and not(blockn)) {
			nblocks += 1;
			blockn = nblocks;
			tx ^= _FM;
		}

		// Output one row of text
		tx1 = "";
		if (html) {

			tdz = td;

			tx1 ^= "<tr";
			if (blockn) {
				tx1 ^= " id=\"B" ^ blockn ^ DQ;
				tx1 ^= " class=\"B" ^ blockn ^ DQ;
				if (detsupp) {
					tx1 ^= " style=\"cursor:pointer";
					if (detsupp == 1) {
						tx1 ^= ";display:none";
					}
					tx1 ^= DQ;
					tx1 ^= " onclick=\"toggle(" "'" "B" ^ blockn ^ "'" ")\"";
				}
			}
			tx1 ^= ">";
		}
		for (int coln : range(1, ncols)) {
			tt = scol[coln];
			oconvx = coldict2(coln, 7);
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
			if (coldict2(coln, 10)) {
				if (not html) {
					// L# R# C# T#
					tt = oconv(tt, coldict2(coln, 11));
				}
				if (tt == "") {
					//tt=nbsp
				} else {
					tt.replacer(_EOL, "<br />");
				}

				// Colored cells starting with colorprefix
				if (tt.starts(colorprefix)) {
					if (tt.at(2) == colorprefix) {
						tt = tt.field(" ", 2, 999999);
						if (tt.len()) {
							tx1 ^= td ^ "<nobr>" ^ tt ^ "</nobr>" ^ tdx;
						} else {
							tx1 ^= td ^ tdx;
						}
					} else {
						//TODO do with class? to save document space?
						tx1 ^= td0 ^ "<td style=\"background-color:" ^ tt.field(" ", 1).cut(1) ^ "\">";
						tt = tt.field(" ", 2, 999999);
						if (tt.len()) {
							tx1 ^= tt ^ tdx;
						} else {
							tx1 ^= tdx;
						}
					}

				} else {
					if (tt.len()) {
						tx1 ^= tdz ^ tt ^ tdx;
					} else {
						tx1 ^= tdz ^ tdx;
					}
				}

			}
		} //coln;

		tx ^= tx1;

		if (html) {
			tx ^= "</tr>";
		}

		gosub printtx();

		// Folding text or multivalued lines
		if (newmarklevel) {
			for (int coln : range(1, ncols)) {
				scol[coln] = "";
			}
			previousmarklevel = newmarklevel;
			goto recexit;
		}

		// Double space
		if (dblspc) {
			gosub printtx();
		}

	}

	goto nextrec;

///////
x2exit:
///////

	// Similar in recinit and x2exit
	if (TERMINAL) {
		logput(AT(-40), recn, ". ");
		osflush();
	}

	// Print the closing subtotals
	breakleveln = nbreaks;
	if (not(gtotsupp) and ((not(pagebreaks) or gtotreq))) {
		breakleveln += 1;
	}
	gosub printbreaks();

	bodyln = 1;

	if (html and not(bottomline.unassigned())) {
		tx(-1) = _FM ^ bottomline ^ _FM;
	}

	// Print number of records and elapsed time
	if (not rawtable) {

		// This results in duplication and is page-wide instead of report-wide so remove
		// but will this cause problems in some reports?

		if (pagebreaks == "" and headtab == "") {
			if (html) {
				tx ^= "<p style=\"text-align:center\">";
			}

			// records
			tt = recn + 0;
			tx ^= tt ^ " record";
			if (recn != 1) {
				tx ^= "s";
			}

			if (fromtimestamp) {
				tx ^= ", " ^ elapsedtimetext(fromtimestamp, timestamp());
			}
			if (html) {
				tx ^= "</p>";
			}
		}

	}

	if (html) {

		if (not detsupp) {
			tx(-1) = "<script type=\"text/javascript\">" _FM " togglendisplayed=" ^ nblocks ^ _FM "</script>";
		}

		tx(-1) = "<script type=\"text/javascript\">" _FM;
		tx ^= "function nwin(key,url,readonly) {";
		tx ^= _FM " gwindowopenparameters={};";
		tx ^= _FM " if (readonly) gwindowopenparameters.readonlymode=true;";
		tx ^= _FM " gwindowopenparameters.key=key;";
		tx ^= _FM " glogincode=\"" ^ SYSTEM.f(17) ^ "*" ^ USERNAME ^ "*\";";
		// Similar code in NLIST and LEDGER2
		tx ^= _FM " var vhtm=window.opener.location.toString().split(\"/\");";
		tx ^= _FM " vhtm[vhtm.length-1]=url;";
		tx ^= _FM " window.open(vhtm.join(\"/\"));";
		tx ^= "}";
		tx ^= _FM "</script>";
	}

x2bexit:

	// Close supertable div and html
	if (html) {
		tx(-1) = "</td></tr></table>";
		tx(-1) = "</div></body></html>";
	}

	gosub printtx();

	lastrecord = 1;
	gosub emailing();

	printfile.osclose();

	return "";
}

subroutine getquotedword() {
	lastword = word;
	gosub getquotedword2();
	return;
}

subroutine getquotedword2() {
	gosub getword();
	if (((DQ ^ "'").contains(word.first())) and (word.starts(word.last()))) {
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

	if (word == "") {
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

	return;
}

subroutine getword2() {

getword2b:
//////////

	word = "";
	wordn += 1;

	// Initialise pointer to zero (pointing before the first char which is 1)
	// NB 1 based character indexing! not zero based!
	if (wordn == 1) {
		charn = 0;
	}

	// Charn is always left pointing to the character BEFORE the next word
	charn += 1;

	// Skip spaces
	while (sentencex.at(charn) == " ") {
		charn += 1;
		if (charn > sentencex.len()) {
			return;
		}
	}

	// If next word starts with " or ' then scan for the same closing
	// otherwise scan up to the next space char
	startcharn = charn;
	charx = sentencex.at(charn);
	if (charx == _DQ or charx == _SQ) {
		searchchar = charx;
	} else {
		searchchar = " ";
	}
	word ^= charx;

	// Build up the word character by character until the closing char is found
	// closing character (" ' or space)
	while (charn < sentencex.len()) {
		charn += 1;
		charx = sentencex.at(charn);

		if (charx == searchchar)
			break;

		word ^= charx;
	}

	// If scanned for " or ' then add it to the word
	if (searchchar != " ") {
		word ^= searchchar;
		charn += 1;

	// Otherwise
	} else {
		word.ucaser();
		word.trimmerboth();
	}

	// Get options and skip to next word
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

	// Standardise using dict.voc to translate input words
	if (DICT == "") {
		goto dictvoc;
	}
	if (dictrec.readc(DICT, word)) {
maindict:
		if (dictrec.f(1) == "G") {
			tt = dictrec.f(3);
			tt.converter(_VM, " ");
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
			if (dictrec.f(1) == "RLIST") {
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
	dictrec.converter("|", _VM);

	if (word == "=") {
		word = "EQ";
	}
	if (word == "EQUAL") {
		word = "EQ";
	}
	if (word == "<>") {
		word = "NE";
	}
	if (word == ">") {
		word = "GT";
	}
	if (word == "<") {
		word = "LT";
	}
	if (word == ">=") {
		word = "GE";
	}
	if (word == "<=") {
		word = "LE";
	}
	if (word == "CONTAINING") {
		word = "[]";
	}
	if (word == "ENDING") {
		word = "[";
	}
	if (word == "STARTING") {
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

	// Print breaks from minor level (1) up to required level
	// Required level can be nbreaks+1 (ie the grand total)

	for (int leveln : range(1, breakleveln)) {
		breakcoln = breakcolns.f(leveln);

		storetx = tx;
		bool anycell = false;

		lastblockn = blockn;
		blockn = 0;
		if (detsupp) {
			if (tx and not tx.ends(_FM)) {
				tx ^= _FM;
			}
			if (leveln > 1 and not(html)) {
				tx ^= underline ^ _FM;
			}
		} else {
			if (not html) {
				underline2 = leveln == 1 ? underline : bar;
				if (not((tx.last(2)).contains(ulchar))) {
					if (not tx.ends(_FM)) {
						tx ^= _FM;
					}
					tx ^= underline2;
				}
			}
			tx ^= _FM;
		}

		// Print one row of totals
		if (html) {
			if (gtotreq) {
				if (leveln > nbreaks) {
					tx ^= "</tbody><tbody>";
				}
			}
			tx ^= "<tr";
			if (lastblockn) {
				tx ^= " style=\"cursor:pointer\" onclick=\"toggle(" "'" "B" ^ lastblockn ^ "'" ")\"";
			}
			tx ^= ">";
		}
		for (int coln : range(1, ncols)) {

			// Total column
			if (coldict2(coln, 12)) {
				cell = breaktotal(coln, leveln);

				// Add into the higher level
				if (leveln <= nbreaks) {

					if (cell) {
						if (html) {
							call htmllib2("ADDUNITS", breaktotal(coln, leveln + 1), cell, _VM);
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

					breakcount[leveln + 1] += breakcount[leveln];
				}

				// Format it
				oconvx = coldict2(coln, 7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				if (html) {
					cell.replacer(_VM, "<br />");
				}

				// and clear it
				breaktotal(coln, leveln) = "";

			// Break column
			} else if (coln == breakcoln) {

				// Print the old break value
				cell = breakvalue[coln];
				oconvx = coldict2(coln, 7);
				if (oconvx) {
					cell = oconv(cell, oconvx);
				}

				// Store the new break value
				breakvalue[coln] = scol[coln];

				if (pagebreaks.f(coln)) {

					// Rebuild the heading
					if (newhead == "") {
						newhead = orighead;
						gosub newheadreplacements();
					}

					// Get the page break data
					tt = scol[coln];

					if (oconvx) {
						tt = oconv(tt, oconvx);
					}
					// Ensure single quotes in data dont screw up the html
					tt.replacer("'", "''");

					// Insert the page break data
					// swap "'B'" with tt in newhead
					newhead.replacer(pagebreaks.f(coln), tt);

				}

				if (detsupp < 2 and not(anytotals)) {
					//cell=nbsp
					cell = "";
				}

			// Other non-total columns are blank
			} else {
				cell = "";

				cell = oconv(oldbreakvalue[coln], coldict2(coln, 7));
				if (var colbreakn; breakcolns.locateusing(_FM, coln, colbreakn)) {
					if (colbreakn < leveln) {
						cell = "Total";
					}
				}

			}

			if (coldict2(coln, 10)) {

				if (html) {
					tx ^= td0 ^ "<th";
				}
				if (not usecols) {
					tx ^= coldict2(coln, bheadfn);
				}
				if (html) {
					if (coldict2(coln, 9) == "R") {
						tx ^= " style=\"text-align:right\"";
					}
					tx ^= ">";
				}
				// L# R# C# T#
				if (not html) {
					cell = oconv(cell, coldict2(coln, 11));
				}
				tx ^= cell;
				if (html) {
					tx ^= thx;
				} else {
					tx ^= " ";
				}
				if (cell.len()) {
					anycell = true;
				}
			}

		} //coln;

//breakrowexit:

		if (html) {
			tx ^= "</tr>";
		}

		if (detsupp < 2) {
			//if leveln>1 and not(html) then tx:=fm:underline
		} else {
			if (not html) {
				tx ^= _FM ^ underline2;
			}
		}

		// Option to suppress the current level
		// or if this is the first record cannot be any totals before it.
		if ((not(anycell) or breakopts.f(leveln).contains("X")) or recn == 1) {
			tx = storetx;
		}

	} //leveln;

	// If only one record found then avoid skipping printing totals for det-supp
	if (detsupp < 2 or recn > 1) {

		if (not html) {
			if (not anytotals) {
				tx = bar;
			}
		}

		// Ensure </td></tr></table> gets printed instead endless nesting every table
		// resulting in only first part of any long report being shown in browser
		if ((newhead and html) and printptr) {

			gosub newheadreplacements();

			// Take over the bottomline so that we can print footer after it
			if (bottomline.unassigned()) {
				bottomline = "";
			}
			bottomline.move(bottomline2);

			tx(-1) = bottomline2;

			if (foot2) {
				tx(-1) = foot2;
			}

			// Close supertable
			tx(-1) = "</td></tr></table>";

		}
		if (tx) {

			gosub printtx();

		}
	} else {
		tx = "";
	}

	// Force new page and new heading
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
	if (str3.len() < str1.len()) {
		str3 ^= (str1.len() - str2.len()).space();
	}
	for (int ii = 1; ii <= str1.len(); ++ii) {
		char1 = (str1.at(ii)).trim();
		if (char1 != "") {
			char2 = str3.at(ii);
			if (char2 == " ") {
				str3.paster(ii, 1, char1);
			} else {
				if (char1.isnum() and char2.isnum()) {
					char3 = char1 + char2;
					if (char3 > 9) {
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
	for (int ii = 1; ii <= nreplacements; ++ii) {
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

	if ((DQ ^ "'").contains(emailtoid.first())) {
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

	// Dont email if no change
	if (lastrecord) {
	} else if (nextemailto != emailto) {
	} else if (nextemailcc != emailcc) {
	} else if (nextemailsubject != emailsubject) {
	} else {
		return;
	}

	if (printptr) {

		// Attachfilename
		body = "@" ^ SYSTEM.f(2);

		tt = emailto;
		tt2 = emailcc;
		if (not tt) {
			tt2.move(tt);
		}

		if (emailsubject) {
			tt3 = emailsubject;
		} else {
			tt3 = head.f(1, 1, 1).field("'", 1);
			if (tt3.contains(">")) {
				tt3 = field2(tt3, ">", -1);
			}
			tt3 = "EXODUS: " ^ tt3;
		}

		// Dont email stuff which has no email address
		if (tt) {
			call sendmail(tt, tt2, tt3, body, "", "", xxerrmsg);
		}

		// Reset the output file to start another email
		printptr = 0;
		var("").oswrite(SYSTEM.f(2)) or lasterror().errputl("nlist:");

	}

	emailto = nextemailto;
	emailcc = nextemailcc;
	emailsubject = nextemailsubject;

	return;
}

libraryexit()
