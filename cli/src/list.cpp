/*
//Exodus Copyright (c) 2009 Stephen Bush
//Licence http://www.opensource.org/licenses/mit-license.php

this is written using Exodus but although extensive is NOT a good example of exodus programming style.
It was the first attempt to implement a substantial classic pick program using a c++ class. It was mostly automatically transliterated from it's mv parent program and therefore uses OO flavour method calling.

Currently it uses some C++ style programming that will later be hidden to minimise the knowledge required by exodus application programmers

The entry point is short program at the bottom of the code which creates an instance of the class/program and calls its entry point. the environment setup will be done automatically by exodus soon

From the point of view of a pick programmer a c++ class can be viewed as a classic pick program with program wide accessible variables being (the class data) and internal subroutines (the class methods).

A c++ class consists of a set of class variables and a set of methods (subroutines). all the subroutines have access to all the class variables.

Unlike pick basic, c++ methods (subroutines) can have their own private variables, be called with a list of parameters, and return values in

So, in summary, c++ methods can be used like internal and external subroutines with common variables.

If all the variables are defined as class variables then a c++ class is essentially identical to a class pick program with subroutines

One way to think of it is as if a c++ class allows common variables and external subroutines but all can be witten in a single file.

*/

/*
usage

list ads brand_code brand_name with brand_code \"XYZ\"

NB if using from shells like bash then quotes must be prefixed by \

Type just list by itself to get a summary of its syntax

*/

#include <exodus/program.h>

//For normal programs, global variables that should be generally available to all subroutines
//should be declared, and optionally *simply* initialised, before the programinit() line.
//"simply initialised" means set to raw numbers and strings.
//eg 'var xx="xyz";' or 'var xx=100.5;' etc.
//
//Multithreaded programs should declare globals after the programinit() line
//and initialise them within any function.

//TODO implement printtext as normal exodus internal or external subroutine
#include "printtext.h"
printtext printer1;

//NB FOLLOWING SECTION is only declaration and optional *simple* initialisation of global variables.
//All programming proper is to be done inside the main or other functions and subroutines
//AFTER the programinit() line

var datafile;

var quotes;
var company;

var wordn;
var sentencex;
var ignorewords;
var html;
var tx;
var ncols;
var breakleveln;
var breakcolns;
var blockn;
var nblocks;
var orighead;
var detsupp;
var nbreaks;
var anytotals;
var recn;
var bodyln;
var head;
var foot;
var breakoptions;
var pagebreakcoln;
var usecols;
var startcharn;
var newmarklevel;
var previousmarklevel;
var lastid;
var crtx;
var idsupp;
var dblspc;
var maxnrecs;
var keylist;
var gtotsupp;
var gtotreq;
var nobase;
var limits;
var nlimits;
var crlf;
int coln;

var tr;
var trx;
var td;
var tdx;
var tt;
var nbsp;
var dictvoc;
var decimalchar;

//if exodus's redim becomes (in effect) non-destructive
//(possible because c++ strings are generally implemented as reference counted, copy on write)
//then this abitrary restriction could be eased
#define maxncols 1000
dim colname(maxncols);
dim coldict(maxncols);
dim mcol(maxncols);
dim pcol(maxncols);
dim ccol(maxncols);
dim scol(maxncols);
dim icol(maxncols);
dim wcol(maxncols);

//the following will be redim'ed  to the required size later
//once the number of BREAK-ON statements is determined
dim breakcount;
dim breaktotal;
dim breakvalue;
dim oldbreakvalue;

var word;
var filename;
var dictfilename;
var xx;
var printtxmark;
var nextword;
var dictrec;
var charn;//num
var underline;
var colunderline;
var bar;
var srcfile;
var clientmark;
var readerr;//num
var temp;
var cursor;
var str1;
var str2;
var str3;
var searchchar;
var underline2;
var cell;//num
var colbreakn;
var char3;

//programinit() is required AFTER program globals and BEFORE all local functions and subroutines including main()
//in MV terms, "programinit" indicates the start of a set of functions and variables that form a single "program"
//in OO terms, "programinit()" indicates the start of a "class"
programinit()

//all programs must have a "function main()" that returns an integer - usually 0 to indicate success.
function main() {

	quotes=DQ^SQ;

	//some identifying name (from environment?)
	company="";

	crlf="\r\n";

	printer1.init(&mv);

	//@sentence='LIST 10 SCHEDULES BY VEHICLE_CODE with vehicle_code "kjh kjh" VEHICLE_NAME BREAK-ON VEHICLE_CODE BREAK-ON VEHICLE_CODE TOTAL PRICE (SX)'
	//@sentence='list markets code name'

	sentencex = _COMMAND.convert(FM," ");

	if (dcount(_COMMAND,FM)==1)
		abort(
		"Syntax is :"
		"\nlist {n} (filename) {fieldnames} {selectionclause} {orderclause} {modifiers} (options)"
		"\n"
		"\nlist {n} (filename)"
		"\n {using (filename) }"
		"\n"
		"\n { {total|break-on} (fieldname)"
		"\n   {justlen \"(L|R|Tnnn)\" }"
		"\n   {colhead \"(title)\" }"
		"\n   {oconv \"(oconvcode)\" }"
		"\n } ..."
		"\n"
		"\n { {by|by-dsnd} (fieldname) } ..."
		"\n"
		"\n { {with|without|limit} {not|no|every} (fieldname)"
		"\n     | {eq|ne|gt|lt|ge||le|starting|ending|containing}"
		"\n         [value] ...}"
		"\n     | {between"
		"\n         [value] and [value]}"
		"\n } {and|or|(|) with|without|limit etc...}"
		"\n"
		"\n { \"(keyvalue)\" } ..."
		"\n"
		"\n {heading     {\"(headingtext&options)\"} }"
		"\n {footing     {\"(footingtext&options)\"} }"
		"\n {grand-total {\"(titletext&options)\"} }"
		"\n"
		"\n {det-supp|det-supp2}"
		"\n {gtot-supp}"
		"\n {id-supp}"
		"\n"
		"\n {(options)}"
		"\n"
		"\nOptions: (H)tml"
		"\n"
		"\nOn Unix prefix special characters like \" ' etc. with \\. Use {} instead of ()."
		"\nOn Windows use ' instead of \". Use the word GT instead of the > character."
		);
	/*
	BREAK-ON fieldname/no
	GRAND-TOTAL grandtotaltext
	TOTAL fieldname/no
	HEADING headingtext
	FOOTING footingtext
	JUSTLEN justificationlengthcode
	COLHEAD columntitletext
	OCONV oconvcode
	DET-SUPP
	DET-SUPP2
	GTOT-SUPP
	DBL-SPC
	IGNOREWORD
	NO-BASE

	{{WITH|WITHOUT|LIMIT} {NOT|NO|EVERY} fieldname|fieldno {NOT} {{EQUAL|EQ|NE|GT|LT|GE|LE|=|<>|>|<|>=|<=|STARTING|ENDING|CONTAINING|MATCHING} value ...|BETWEEN value AND value}

	*/

	var options=_OPTIONS.ucase();

	//declare function get.cursor,put.cursor

	nblocks = 0;
	blockn = 0;

	//use <COL> for hiding non-totalled cols in det-supp (slow)
	usecols = 0;

	if (mv.USER2.substr(2, 1) eq "C") {
		decimalchar = ",";
	} else {
		decimalchar = ".";
	}

	html = index(options,"H");
	if (html) {
		tr = "<tr>";
		trx = "</tr>" ^ crlf;
		td = "<td>";
		tdx = "</td>";
		nbsp = "&nbsp;";
		tt = mv.SYSTEM.extract(2);
		tt.swapper(".txt", ".htm");
		mv.SYSTEM.replacer(2, 0, 0, tt);
	} else {
		tr = "";
		trx = crlf;
		td = "";
		tdx = " ";
		nbsp = "";
	}

	//automatically create dict_voc so you can list dictionaries
	if (not open("DICT_VOC",dictvoc)) {
		createfile("DICT_VOC");
		if (open("DICT_VOC",dictvoc)) {

			//prepare some dictionary records
			var dictrecs = "";
			dictrecs  =      "@ID   |F|0 |Id     |S|||||L|10";
			dictrecs ^= FM ^ "TYPE  |F|1 |Type   |S|||||L|4";
			dictrecs ^= FM ^ "FMC   |F|2 |Field  |S|||||R|3";
			dictrecs ^= FM ^ "PART  |F|5 |Part   |S|||||R|2";
			dictrecs ^= FM ^ "TITLE |F|3 |Title  |M|||||T|20";
			dictrecs ^= FM ^ "SM    |F|4 |SM     |S|||||L|1";
			dictrecs ^= FM ^ "CONV  |F|7 |Convert|S|||||T|20";
			dictrecs ^= FM ^ "JUST  |F|9 |Justify|S|||||L|3";
			dictrecs ^= FM ^ "LENGTH|F|10|Length |S|||||R|10";
			dictrecs ^= FM ^ "MASTER|F|28|Master |S|||||L|1";
			dictrecs ^= FM ^ "@CRT  |G|  |TYPE FMC PART TITLE SM CONV JUST LENGTH MASTER";

			//write the dictionary records to the dictionary
			var nrecs=count(dictrecs,FM)+1;
			for (var recn = 1; recn <= nrecs; recn++) {
				var dictrec=dictrecs.extract(recn);
				while (dictrec.index(" |"))
					dictrec.swapper(" |","|");
				var key=field(dictrec,"|",1);
				var rec=field(dictrec,"|",2,9999);
				//printl(key ^ ": " ^ rec);
				write(rec.convert("|",FM), dictvoc, key);
			}
		}
	}

////////
//init1:
////////

	var thcolor = mv.SYSTEM.extract(46, 1);
	var tdcolor = mv.SYSTEM.extract(46, 2);
	var reportfont = mv.SYSTEM.extract(46, 3);
	if (!tdcolor)
		tdcolor = "#FFFFC0";
	if (!thcolor)
		thcolor = "#FFFF80";

	mv.DICT = "";
	maxnrecs = "";
	keylist = "";

	crtx = "";
	idsupp = "";
	dblspc = "";
	detsupp = 0;
	if (sentencex.index(" DET-SUPP", 1))
		detsupp = 1;
	if (sentencex.index(" DET-SUPP2", 1))
		detsupp = 2;
	maxnrecs = "";
	gtotsupp = "";
	gtotreq = "";
	nobase = "";

	if (!(open("DICT_VOC", dictvoc)))
		//stop("Cannot open DICT.VOC");
		dictvoc="";

	coln = 0;
	head = "";
	foot = "";
	nbreaks = 0;
	breakcolns = "";
	breakoptions = "";
	pagebreakcoln = "";

	var topmargin = 0;
	var totalflag = 0;
	var breakonflag = 0;
	anytotals = 0;

	limits = "";
	nlimits = 0;

	ignorewords = "";

//initphrase:
/////////////
	var ss = "";
	wordn = 0;

///////////
nextphrase:
///////////

	gosub getword();
	if (word eq "")
		goto x1exit;

phraseinit:
///////////

	if (word.substr(1, 4) eq "SORT" or word.substr(1, 4) eq "LIST") {

		if (word.index("SORT"))
			ss ^= "S";
		ss ^= "SELECT";
		//filename:
		gosub getword();
		if (!word)
			abort("FILE NAME IS REQUIRED");

		//limit number of records
		if (word.match("\\d+","r")) {
			maxnrecs = word;
			ss ^= " " ^ maxnrecs;
			gosub getword();
		}

		//get the filename
		// deleted
		if (word eq "DICT") {
			gosub getword();
			word = "dict_" ^ word;    // changed
		}
		filename = word;               // new

		if (filename.substr(1, 5).lcase() eq "dict_") {
			dictfilename = "VOC";
		} else {
			dictfilename = filename;
		}
		if (not mv.DICT.open("dict_"^dictfilename))
			mv.DICT = dictvoc;
		ss ^= " " ^ word;

		//get any specific keys
		while (true) {

			///BREAK;
			if (!(nextword.isnum() or nextword.substr(1, 1) eq SQ or nextword.substr(1, 1) eq DQ))
				break;

			keylist = 1;
			gosub getword();
			if (word eq "")
				//goto exitloop;
				break;
			ss ^= " " ^ word;
		}//loop;
		//exitloop:

	} else if (word eq "GETLIST") {
		gosub getword();
		var("GETLIST " ^ word).perform();

	} else if (word eq "AND" or word eq "OR") {
		ss ^= " " ^ word;

	} else if (word eq "(" or word eq ")") {
		ss ^= " " ^ word;

	} else if (word eq "BY" or word eq "BY-DSND") {
		ss ^= " " ^ word;
		gosub getword();
		ss ^= " " ^ word;

	} else if (word eq "WITH NOT" or word eq "WITH" or word eq "WITHOUT" or word eq "LIMIT") {
		ss ^= " " ^ word;

		var limit = word eq "LIMIT";
		if (limit)
			nlimits += 1;

		gosub getword();

		//NO/EVERY
		if (word eq "NOT" or word eq "NO" or word eq "EVERY") {
			ss ^= " " ^ word;
			gosub getword();
		}

		//field or NO
		ss ^= " " ^ word;
		if (limit)
			limits.replacer(1, nlimits, 0, word);

		//negate next comparision
		if (var("NOT,NE,<>").extract(1).locateusing(nextword, ",", xx)) {
			nextword = "NOT";
			gosub getword();
			ss ^= " " ^ word;
		}

		//comparision
		if (var("MATCH,EQ,NE,GT,LT,GE,LE,[,],[]").locateusing(nextword, ",", xx)) {
			gosub getword();
			ss ^= " " ^ word;
			if (limit)
				limits.replacer(2, nlimits, 0, word);
		}

		//with x between y and z
		//with x from y to z
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
				if (!(nextword not_eq "" and (nextword.isnum() or nextword.substr(1, 1) eq DQ or nextword.substr(1, 1) eq SQ)))
					break;

				gosub getword();
				ss ^= " " ^ word;
				if (limit) {
					if ((DQ ^ SQ).index(word.substr(1, 1), 1)) {
						if (word.substr(1, 1) eq word.substr(-1, 1))
							word = word.substr(2, word.length() - 2);
					}
					if (word eq "")
						word = "\"\"";
					limits.replacer(3, nlimits, -1, word);
				}
			}//loop;

		}

	} else if (word eq "BREAK-ON") {
		breakcolns.splicer(1, 0, coln + 1 ^ FM);
		breakoptions.splicer(1, 0, FM);
		nbreaks += 1;
		breakonflag = 1;

	} else if (word eq "GRAND-TOTAL") {
		//zzz throw away the grand total options for the time being
		gosub getword();
		gtotreq = 1;

	} else if (word eq "NO-BASE") {
		nobase = 1;

	//"DET-SUPP"
	} else if (word eq "DET-SUPP") {
		detsupp = 1;

	//"DET-SUPP"
	} else if (word eq "DET-SUPP2") {
		detsupp = 2;

	//"GTOT-SUPP"
	} else if (word eq "GTOT-SUPP") {
		gtotsupp = 1;

	//case dictrec
	} else if (word eq "TOTAL") {
		totalflag = 1;

	} else if (word eq "USING") {
		gosub getword();
		dictfilename = word;
		if (!(mv.DICT.open("dict_"^dictfilename))) {
			fsmsg();
			abort("");
		}

	} else if (word eq "HEADING") {

		gosub getword();
		head = word;
		if (html) {
			head.swapper("Page \'P\'", "");
			head.swapper("Page \'P", SQ);
		}
		head.splicer(1, 1, "");
		head.splicer(-1, 1, "");

	} else if (word eq "FOOTING") {
		gosub getword();
		foot = word;
		foot.splicer(1, 1, "");
		foot.splicer(-1, 1, "");

	//justlen
	} else if (word eq "JUSTLEN") {
		if (!coln) {
			mssg("JUSTLEN/JL must follow a column name");
			abort("");
		}
		gosub getword();
		word.splicer(1, 1, "");
		word.splicer(-1, 1, "");
		coldict(int(coln)).replacer(9, 0, 0, word.substr(1, 1));
		coldict(coln).replacer(10, 0, 0, word.substr(3, 9999));
		coldict(coln).replacer(11, 0, 0, word);

	//colhead
	} else if (word eq "COLHEAD") {
		gosub getword();
		//skip if detsupp2 and column is being skipped
		if (!(coldict(coln).unassigned())) {
			word.splicer(1, 1, "");
			word.splicer(-1, 1, "");
			word.converter("|", VM);
			coldict(coln).replacer(3, 0, 0, word);
		}

	} else if (word eq "OCONV") {
		gosub getword();
		word.splicer(1, 1, "");
		word.splicer(-1, 1, "");
		if (html)
			word.swapper("[DATE]", "[DATE,*]");
		coldict(coln).replacer(7, 0, 0, word);

	} else if (word eq "ID-SUPP") {
		idsupp = 1;

	} else if (word eq "DBL-SPC") {
		dblspc = 1;

	} else if (dictrec) {
		if (var("FSDIA").index(dictrec.extract(1), 1)) {

			var nn;

			//pick items
			if (var("DI").index(dictrec.extract(1), 1))
				dicti2a(dictrec);

			//pick A equ F
			if (dictrec.extract(1) eq "A")
				dictrec.replacer(1, 0, 0, "F");

			//suppress untotalled columns if doing detsupp2
			if (detsupp eq 2) {
				//if (var("JL,JUSTLEN,CH,COL,HEAD,OC,OCONV").locateusing(nextword, ",", xx)) {
				if (var("JUSTLEN,COLHEAD,OCONV").locateusing(nextword, ",", xx)) {
					gosub getword();
					gosub getword();
				}
				if (!(totalflag or breakonflag))
					goto dictrecexit;
			}

			coln += 1;
			colname(coln) = word;

			//increase column width if column title needs it
			nn = (dictrec.extract(3)).count(VM) + 1;
			for (var ii = 1; ii <= nn; ii++) {
				tt = dictrec.extract(3, ii);
				var f10=dictrec.extract(10);
				if (f10=="" or f10 and tt.length() > f10)
					dictrec.replacer(10, 0, 0, tt.length());
			};//ii;

			if (detsupp < 2) {
				if (!(totalflag or breakonflag)) {
					tt = " id=\"BHEAD\"";
					if (detsupp)
						tt ^= " style=\"display:none\"";
					dictrec.replacer(14, 0, 0, tt);
				}
			}

			//total required ?
			if (totalflag) {
				totalflag = 0;
				dictrec.replacer(12, 0, 0, 1);
				anytotals = 1;
			} else {
				dictrec.replacer(12, 0, 0, 0);
			}

			if (html) {
				tt = dictrec.extract(7);
				tt.swapper("[DATE]", "[DATE,*]");
				if (tt eq "[DATE,4]")
					tt = "[DATE,4*]";
				dictrec.replacer(7, 0, 0, tt);
				if (tt eq "[DATE,*]")
					dictrec.replacer(9, 0, 0, "R");
			}
			coldict(coln) = dictrec;

			//store the format in a convenient place
			if (html) {
				tt = "";
			} else {
				tt = coldict(coln).extract(9);
				if (tt)
					tt ^= "#" ^ coldict(coln).extract(10);
			}
			coldict(coln).replacer(11, 0, 0, tt);

			//this could be a break-on column and have break-on options
			//if coln=breakcolns<1> then
			if (breakonflag) {
				coldict(coln).replacer(13, 0, 0, 1);
				breakonflag = 0;
				if (nextword.substr(1, 1) eq DQ) {
					gosub getword();
					//zzz break options
					if (word.index("B", 1))
						pagebreakcoln = coln;
					breakoptions.replacer(1, 0, 0, word);
				}
			}
		}

	} else if (word eq "IGNOREWORD") {
		gosub getword();
		ignorewords.replacer(1, -1, 0, word);

		//@LPTR word is skipped if not located in VOC/DICT.VOC
	} else if (word eq "@LPTR") {

	} else {
		//sys.messages W156
		//tt = "\"%1%\" is an unrecognized word.Please correct the word by retyping it|or pressing (F4] to edit it.Press [Esc) to re-start.";
		tt = "\"%1%\" is an unrecognized word.";
		tt.swapper("%1%", word);
		msg2(tt, "RCE", word, word);
		if (word eq var().chr(27))
			abort("");
		gosub getwordexit();
		goto phraseinit;

	}

dictrecexit:
////////////

/////////////
//phraseexit:
/////////////
	goto nextphrase;

///////
x1exit:
///////
	//if no columns selected then try to use default @crt or @lptr group item
	if (!(coln or crtx)) {
		word = "@LPTR";
		if (mv.DICT and not xx.read(mv.DICT, word)) {
			word = "@CRT";
			if (not xx.read(mv.DICT, word))
				word = "";
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
	if (!idsupp) {

		ncols += 1;

		//move the columns up by one to make space for a new column 1
		for (int coln = ncols; coln >= 2; coln--) {
			coldict(coln) = coldict(coln - 1);
			colname(coln) = colname(coln - 1);
		};//coln;

		//set column 1 @ID
		colname(1) = "@" "ID";
		if (not mv.DICT or not coldict(1).read(mv.DICT, "@" "ID")) {
			if (not dictvoc or not coldict(1).read(dictvoc, "@" "ID"))
				coldict(1) = "F" ^ FM ^ "0" ^ FM ^ "Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "L" ^ FM ^ 15;
		}
		if (html) {
			tt = "";
		} else {
			tt = coldict(1).extract(9) ^ "#" ^ coldict(1).extract(10);
		}
		coldict(1).replacer(11, 0, 0, tt);

		//increment the list of breaking columns by one as well
		for (var breakn = 1; breakn <= nbreaks; breakn++)
			breakcolns.replacer(breakn, 0, 0, breakcolns.extract(breakn) + 1);
		pagebreakcoln += 1;

	}

	//work out column "widths". zero suppresses a column. blank or anything else allows it.
	for (int coln=1; coln<=ncols; ++coln) {
		var tt=coldict(coln).extract(10);
		if (tt eq "")
			tt="10";
		wcol(coln)=tt;
	}

	if (breakcolns.substr(-1, 1) eq FM)
		breakcolns.splicer(-1, 1, "");
	if (breakoptions.substr(-1, 1) eq FM)
		breakoptions.splicer(-1, 1, "");

	//make underline and column title underline
	if (!html) {
		underline = "";
		colunderline = "";
		for (int coln = 1; coln <= ncols; coln++) {
			if (wcol(coln)) {
				if (coldict(coln).extract(12)) {
					tt = "-";
				} else {
					tt = " ";
				}
				underline ^= tt.str(wcol(coln)) ^ " ";
				colunderline ^= var("-").str(wcol(coln)) ^ " ";
			}
		};//coln;
		bar = var("-").str(colunderline.length() - 1);
	}

////////
//init2:
////////

	tx = "";

	if (filename.unassigned()||not filename)
		stop("Filename not specified");

	if (!(open(filename, srcfile)))
		stop("Cannot open file " ^ filename);

	//reserve breaks and set all to ""
	breakcount.redim(nbreaks + 1);
	breaktotal.redim(maxncols, nbreaks+1);
	breakvalue.redim(maxncols);
	oldbreakvalue.redim(maxncols);
	breakvalue="";
	oldbreakvalue="";
	breakcount="";
	breaktotal="";

	//build the column headings
	var colheading = "";
	var coltags = "";
	var coln2 = 0;
	for (int coln = 1; coln <= ncols; coln++) {

		//suppress drilldown if no totals or breakdown
		if (not anytotals or not nbreaks)
			coldict(coln).replacer(14, 0, 0, "");

		if (wcol(coln)) {
			if (html) {
				coln2 += 1;

				//t='<th bgcolor=':thcolor
				tt = "<th style=\"background-color:" ^ thcolor ^ DQ;
				//if detsupp=1 then t:=coldict(coln)<14>
				if (!usecols)
					tt ^= coldict(coln).extract(14);
				tt ^= ">" ^ coldict(coln).extract(3) ^ "</th>";
				colheading.replacer(coln2, 0, 0, tt);

				colheading.swapper(VM, "<br />");
				coltags.replacer(-1, 0, 0, "<col");
				var align = coldict(coln).extract(9);
				if (align eq "R") {
					//if index(coldict(coln)<7>,'[NUMBER',1) then
					// *http://www.w3.org/TR/html401/struct/tables.html#adef-align-TD
					// coltags:=' align="char" char="':decimalchar:'"'
					//end else
					//coltags:=' align="right"'
					coltags ^= " style=\"text-align:right\"";
					// end
				} else if (align eq "T") {
					//coltags:=' align="left"'
					coltags ^= " style=\"text-align:left\"";
				}
				if (usecols)
					coltags ^= coldict(coln).extract(14);
				coltags ^= " />";
			} else {
				for (var ii = 1; ii <= 9; ++ii)
					colheading.replacer(ii, 0, 0, colheading.extract(ii) ^ (coldict(coln).extract(3, ii)).oconv(coldict(coln).extract(11)) ^ " ");
			}
		}
	}

	//change all "(Base)" in dictionary column headings to the base currency
	//unless the keyword NO-BASE is present in which case replace with blank
	//this is useful if all the columns are base and no need to see the currency

	tt = html ? "" : "C#6";
	var t2 = company.extract(3);
	if (t2) {
		if (nobase) {
			t2 = "";
		} else {
			t2 = "(" ^ t2 ^ ")";
		}
		colheading.swapper("(Base)", t2.oconv(tt));
		colheading.swapper("%BASE%", company.extract(3));
	}

	//trim off blank lines (due to the 9 above)
	if (html) {

		tt = "";

		getmark("CLIENT", html, clientmark);
		tt ^= clientmark;

		tt ^= "<table border=\"1\" cellspacing=\"0\" cellpadding=\"2\"";
		//t:=' align="center" '
		tt ^= " style=\"font-size:66%";
		tt ^= ";background-color:" ^ tdcolor;
		tt ^= ";page-break-after:avoid";
		tt ^= "\">";
		tt ^= FM ^ "<colgroup>" ^ coltags;
		tt ^= FM ^ "<thead>";
		//t:=fm:coltags

		tt ^= tr ^ colheading ^ trx ^ FM ^ "</thead>";
		tt ^= FM ^ "<tbody>";
		tt.transfer(colheading);

		//allow for single quotes
		colheading.swapper(SQ, "\'\'");

	} else {
		while (true) {

			///BREAK;
			if (!(colheading and var(" " ^ FM).index(colheading.substr(-1, 1), 1)))
				break;
			colheading.splicer(-1, 1, "");
		}//loop;
	}

	//heading options

	if (head eq "")
		head = "Page \'P\' " ^ space(50) ^ " \'T\'";

	if (html) {
		//
		//			//get the letterhead
		//			newpagetag='<div style="page-break-before:always">';
		//			call gethtml('head',letterhead,'');

		//			//insert the letterhead
		//			swap newpagetag with newpagetag:letterhead in tx;

		//			call readcss(css);
		//			swap '</head>' with crlf:field(css,char(26),1):crlf:'</head>' in tx;

		//			//remove the first pagebreak
		//			stylecode='page-break-before';
		//			t=index(ucase(tx),stylecode,1);
		//			if t then tx(t,len(stylecode))='XXX';
		//
		head.swapper(FM, "<br />");
		head = "<h2~style=\"text-align:center\">" ^ head ^ "</h2>";
	}

	//footing options

	if (dblspc)
		head ^= FM;
	if (!html)
		head ^= FM ^ colunderline;
	head ^= FM ^ colheading;
	if (!html)
		head ^= FM ^ colunderline;
	if (dblspc)
		head ^= FM;
	orighead = head;
	printer1.setheadfoot(head,foot);

//////////
//initrec:
//////////
	var selectedok;
	if (ss.count(" ") > 2 or keylist) {
		//call msg2('Selecting records, please wait.||(Press Esc or F10 to interrupt)','UB',buffer,'')
		//perform ss:' (S)'
		//call perf(ss:' (S)')
		selectedok=filename.select(ss);

	} else {
		if (mv.LISTACTIVE)
			selectedok=true;
		else
			selectedok=srcfile.select(ss);
	}

	if (not selectedok) {
		//call msg2('','DB',buffer,'')
		//if interactive else
		mssg("No records found");

		abort("");

		if (html) {
			tx ^= "</tbody></table>";
		}
		tx ^= "No records listed";
		printer1.printtx(tx);

	} else {
		recn = "";
		lastid = "%EXODUS_%%%%%_NONE%";
		process_all_records();
	}

//x2bexit:
//////////

	//remove while no header being output
	//if (html)
	//	tx ^= "</div></body></html>";

	printer1.printtx(tx);

	printer1.close();

	return 0;
}

//////////////////////////////
function process_all_records()
//////////////////////////////
{

//returns true if completes or false if interrupted

	while (true) {

		if (esctoexit()) {
			tx = "";
			if (html)
				tx ^= "</tbody></table>";
			tx ^= "*** incomplete - interrupted ***";
			printer1.printtx(tx);
			var().clearselect();
			//goto x2bexit;
			return false;
		}

		//limit number of records (allow one more to avoid double closing since nrecs limited in select clause as well
		if (maxnrecs and recn >= (maxnrecs+1))
			filename.clearselect();

		//readnext key
		mv.FILEERRORMODE = 1;
		mv.FILEERROR = "";
		if (not filename.readnext(mv.ID, mv.MV)) {

			mv.FILEERRORMODE = 0;
			/* treat all errors as just "no more records"
			if (_STATUS) {
			tx = "*** Fatal Error " ^ mv.FILEERROR.extract(1) ^ " reading record " ^ mv.ID ^ " ***";
			printer1.printtx(tx);
			abort("");
			}
			if (mv.FILEERROR.extract(1) eq 421) {
			tx = "Operation aborted by user.";
			printer1.printtx(tx);
			abort("");
			}
			if (mv.FILEERROR and mv.FILEERROR.extract(1) not_eq 111) {
			tx = "*** Error " ^ mv.FILEERROR.extract(1) ^ " reading record " ^ mv.ID ^ " ***";
			printer1.printtx(tx);
			readerr += 1;
			abort("");
			}
			*/

			return true;
		}

		//skip record keys starting with "%" (control records)
		if (mv.ID.substr(1, 1) eq "%")
			continue;

		//read the actual record (TODO sb modify select statement to return the actual columns required)
		//or skip if disappeared
		if (!(mv.RECORD.read(srcfile, mv.ID)))
			continue;

		//designed to filter multivalues which are not selected properly
		//unless sorted "by"
		if (limits) {
			var nfns = mv.RECORD.count(FM) + 1;
			//find maximum var number
			var nvars = 1;
			for (var fn = 1; fn <= nfns; fn++) {
				temp = mv.RECORD.extract(fn);
				if (temp.length()) {
					temp = temp.count(VM) + 1;
					if (temp > nvars)
						nvars = temp;
				}
			};//fn;
			//for each limit pass through record deleting all unwanted multivalues
			for (var limitn = 1; limitn <= nlimits; limitn++) {
				var limitvals = var(limits.extract(1, limitn)).calculate();
				for (var varx = nvars; varx >= 1; varx--) {
					temp = limitvals.extract(1, varx);
					if (temp eq "")
						temp = "\"\"";
					//locate temp in (limits<3,limitn>)<1,1> using sm setting x else
					if (!(limits.extract(3, limitn).locateusing(temp, SVM, xx))) {
						for (var fn = 1; fn <= nfns; fn++) {
							var varalues = mv.RECORD.extract(fn);
							if (varalues.count(VM))
								mv.RECORD.eraser(fn, varx, 0);
						};//fn;
					}
				};//varx;
			};//limitn;
		}

		recn += 1;

		//process/output one record
		process_one_record();

	}//readnext


	//print the closing subtotals
	breakleveln = nbreaks;
	if (not gtotsupp and (not pagebreakcoln or gtotreq))
		breakleveln += 1;
	gosub printbreaks();

	bodyln = 1;

	if (html)
		tx ^= "<p style=\"text-align:center\">";
	tx ^= FM ^ recn + 0 ^ " record";
	if (recn not_eq 1)
		tx ^= "s";
	if (html)
		tx ^= "</p>";

	if (!detsupp)
		tx ^= "<script type=\"text/javascript\"><!--" ^ FM ^ " togglendisplayed+=" ^ nblocks ^ FM ^ "--></script>";

	return true;
}

///////////////////////////////
subroutine process_one_record()
///////////////////////////////
{

	//var().getcursor();
	//if (not mv.SYSTEM.extract(33)) {
	//	print(mv.AW.extract(30), var().cursor(36, mv.CRTHIGH / 2));
	//}else{
	//	print(var().cursor(0));
	//}
	//print(recn, " ");
	//cursor.setcursor();

	var fieldno;
	var keypart;
	var cell;

	//get the data from the record into an array of columns
	for (int coln = 1; coln <= ncols; coln++) {
		// @var=0
		//if (coldict(coln).extract(9) eq "T" and not html) {
		//	mcol(coln) = (var(colname(coln)).calculate()).oconv(coldict(coln).extract(11));
		//call note(coln:'/':m.col(coln):'/':coldict(coln)<11>)
		//call note(@record<coldict(coln)<2>)
		//}else{

		//calculate doesnt work (just yet)
		//mcol(coln) = var(colname(coln)).calculate();

		dictrec=coldict(coln);
		if (dictrec.extract(1) eq "F") {
			fieldno=dictrec.extract(2);
			if (not fieldno) {
				cell=mv.ID;
				keypart=dictrec.extract(6);
				if (keypart)
					cell=cell.field(L'.',keypart);
			} else
				cell=mv.RECORD.extract(fieldno);
		} else {
			cell="";
		}

		mcol(coln)=cell;

		//	if (html)
		mcol(coln).swapper(TM, "<br />");

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
	} else {
		int coln;
		var leveln;
		for (leveln = nbreaks; leveln >= 1; leveln--) {
			coln = breakcolns.extract(leveln);

			///BREAK;
			if (scol(coln) not_eq breakvalue(coln))
				break;

		};//leveln;
		breakleveln = leveln;
	}

	gosub printbreaks();

	//oldbreakvalue.init(breakvalue);
	oldbreakvalue=breakvalue;

	previousmarklevel = 0;

	while (true) {

		//remove appropriate value from multi-valued column(s)
		newmarklevel = 0;
		for (int coln = 1; coln <= ncols; coln++) {
			if (ccol(coln) >= previousmarklevel) {
				icol(coln)=mcol(coln).remove(pcol(coln), ccol(coln));
				scol(coln) = icol(coln);
			}
			if (ccol(coln) > newmarklevel)
				newmarklevel = ccol(coln);
		};//coln;

		//break totals - add at the bottom level (1)
		for (int coln = 1; coln <= ncols; coln++) {
			//only totalled columns
			if (coldict(coln).extract(12)) {
				if (icol(coln)) {
					if (html) {
						//breaktotal(coln,1)+=i.col(coln)
						addunits(icol(coln), breaktotal(coln,1));
					} else {
						if ((breaktotal(coln,1)).isnum() and (icol(coln)).isnum()) {
							breaktotal(coln,1) += icol(coln);
						} else {
							if (colname(coln) eq "DATEGRID") {
								str1 = icol(coln);
								str2 = breaktotal(coln,1);
								gosub addstr();
								breaktotal(coln,1) = str3;
							}
						}
					}
				}
				breakcount(1) += 1;
				icol(coln) = "";
			}
		};//coln;

		if (detsupp < 2) {

			if (anytotals and not blockn) {
				nblocks += 1;
				blockn = nblocks;
				//tx:='<span style="display:none" id="B':blockn:'">'
				tx ^= FM;
			}

			//print one row of text

			//html rows can be hidden until subtotal is is clicked
			if (html) {
				tx ^= "<tr";
				if (blockn) {
					if (detsupp eq 1)
						tx ^= " style=\"display:none\"";
					tx ^= " id=\"B" ^ blockn ^ DQ;
					//clicking expanded det-supped details collapses it
					if (detsupp)
						tx ^= " style=\"cursor:pointer\" onclick=\"toggle(B" ^ blockn ^ ")\"";
				}
				tx ^= ">";
			}

			//output the columns

			for (int coln = 1; coln <= ncols; coln++) {
				tt = scol(coln);

				//oconv
				var oconvx = coldict(coln).extract(7);
				if (oconvx) {
					tt = tt.oconv(oconvx);

					//prevent html folding of numbers on minus sign
					if (html) {
						if (tt.substr(1, 1) eq "-") {
							if (oconvx.substr(1, 7) eq "[NUMBER")
								tt = "<nobr>" ^ tt ^ "</nobr>";
						}
					}
				}

				//non-zero width columns
				if (wcol(coln)) {

					//non-html format to fixed with with spaces
					if (!html)
						tt = tt.oconv(coldict(coln).extract(11));

					//html blank is nbsp
					if (tt eq "")
						tt = nbsp;

					//add one column
					tx ^= td ^ tt ^ tdx;
				}
			};//coln;

			//terminate the row and output it
			tx ^= trx;
			printer1.printtx(tx);

			//loop back if any multivalued lines/folded text
			if (newmarklevel) {
				for (int coln = 1; coln <= ncols; coln++)
					scol(coln) = "";
				previousmarklevel = newmarklevel;
				continue;
			}

			//double space
			if (dblspc)
				printer1.printtx(tx);

		}//detsupp < 2


		break;

	}//loop while folding text or multivalued lines

	return;
}

////////////////////
subroutine getword()
////////////////////
{
	while (true) {

		gosub getword2();

		if (word.length()) {
			if (ignorewords.locateusing(word, VM, xx))
				continue;
		}
		break;
	}

	if (word eq "") {
		nextword = "";
	} else {
		var storewordn = wordn;
		var storecharn = charn;
		var storeword = word;
		var storedictrec = dictrec;
		gosub getword2();
		nextword = word;
		word = storeword;
		dictrec = storedictrec;
		wordn = storewordn;
		charn = storecharn;
	}

	// call note(quote(word):' ':quote(nextword))

	if (not quotes.index(word.substr(1,1)))
		word.ucaser();

	if (not quotes.index(nextword.substr(1,1)))
		nextword.ucaser();

	return;
}

/////////////////////
subroutine getword2()
/////////////////////
{

	while (true) {

		/////////
		wordn += 1;
		if (wordn eq 1)
			charn = 0;

		// call note(wordn:' ':charn)

		word = "";
		charn += 1;
		while (true) {

			///BREAK;
			if (!(sentencex.substr(charn, 1) eq " "))
				break;
			charn += 1;
			if (charn > sentencex.length())
				return;
		}//loop;

		startcharn = charn;
		var charx = sentencex.substr(charn, 1);

		if (var(SQ ^ DQ).index(charx, 1)) {
			searchchar = charx;
		} else {
			searchchar = " ";
		}
		word ^= charx;

		while (true) {
			charn += 1;
			charx = sentencex.substr(charn, 1);

			///BREAK;
			if (charx eq "" or charx eq searchchar)
				break;

			word ^= charx;
		}

		if (searchchar not_eq " ") {
			word ^= searchchar;
			charn += 1;
		} else {
			word.ucaser();
			word = word.trimb().trimf();
		}

		//options
		if (word.substr(1, 1) eq "(" and word.substr(-1, 1) eq ")") {
			var commandoptions = word;
			continue;
		}

		break;

	}

	gosub getwordexit();
}

////////////////////////
subroutine getwordexit()
////////////////////////
{

	//standardise
	if (mv.DICT ne "" and dictrec.read(mv.DICT, word)) {
		if (dictrec.extract(1) eq "G") {
			tt = dictrec.extract(3);
			tt.converter(VM, " ");
			sentencex.splicer(startcharn, word.length(), tt);
			charn = startcharn - 1;
			wordn -= 1;
			//goto getword2;
			gosub getword2();
			return;
		}
	} else {
		dictrec = "";
		if (dictvoc and dictrec.read(dictvoc, word)) {
			if (dictrec.extract(1) eq "RLIST") {
				if (dictrec.extract(4))
					word = dictrec.extract(4);
				dictrec = "";
			}
		}
	}
	dictrec.converter("|", VM);

	if (word eq "=")
		word = "EQ";
	else if (word eq "EQUAL")
		word = "EQ";
	else if (word eq "<>")
		word = "NE";
	else if (word eq ">")
		word = "GT";
	else if (word eq "<")
		word = "LT";
	else if (word eq ">=")
		word = "GE";
	else if (word eq "<=")
		word = "LE";
	/*
	if (word eq "CONTAINING")
	word = "()";
	if (word eq "ENDING")
	word = "[";
	if (word eq "STARTING")
	word = "]";
	*/

	return;
}

////////////////////////
subroutine printbreaks()
////////////////////////
{

	if (!breakleveln)
		return;

	var newhead = "";

	//print breaks from minor level (1) up to required level
	//required level can be nbreaks+1 (ie the grand total)
	//for leveln=1 to breakleveln
	//for leveln=breakleveln to 1 step -1

	for (var leveln = 1; leveln <= breakleveln; leveln++) {
		var breakcoln = breakcolns.extract(leveln);

		var storetx = tx;
		//output any prior tx is this level is suppressed
		//if index(breakoptions<leveln>,'X',1) and tx then
		// if recn>1 then gosub printtx
		// end

		var lastblockn = blockn;
		blockn = 0;
		if (detsupp) {
			if (tx and tx.substr(-1, 1) not_eq FM)
				tx ^= FM;
			if (leveln > 1 and not html)
				tx ^= underline ^ FM;
		} else {
			if (!html) {
				//underline2=if breakleveln>=nbreaks then bar else underline
				underline2 = (leveln eq 1) ? (underline) : (bar);
				if (!((tx.substr(-2, 2)).index("-", 1))) {
					if (tx.substr(-1, 1) not_eq FM)
						tx ^= FM;
					tx ^= underline2;
				}
			}
			tx ^= FM;
		}

		//print one row of totals
		if (html) {
			tx ^= tr;
			if (lastblockn)
				tx ^= " style=\"cursor:pointer\" onclick=\"toggle(B" ^ lastblockn ^ ")\"";
			if (detsupp < 2 or nbreaks > 1 and leveln > 1)
				tx ^= " style=\"font-weight:bold\"";
			tx ^= ">";
		}
		for (int coln = 1; coln <= ncols; coln++) {

			//total column
			if (coldict(coln).extract(12)) {
				cell = breaktotal(coln,leveln);

				//add into the higher level
				if (leveln <= nbreaks) {

					if (cell) {
						if (html) {
							//breaktotal(coln,leveln+1)+=cell
							addunits(cell, breaktotal(coln,leveln + 1));
						} else {
							if ((breaktotal(coln,leveln + 1)).isnum() and cell.isnum()) {
								breaktotal(coln,leveln + 1) += cell;
							} else {
								str1 = cell;
								str2 = breaktotal(coln,leveln + 1);
								gosub addstr();
								breaktotal(coln,leveln + 1) = str3;
							}
						}
					}

					breakcount(leveln + 1) += breakcount(leveln);
				}

				//format it
				var oconvx = coldict(coln).extract(7);
				if (oconvx)
					cell = cell.oconv(oconvx);

				if (html) {
					if (cell eq "")
						cell = nbsp;
					else
						cell.swapper(VM, "<br />");
				}

				//and clear it
				breaktotal(coln,leveln) = "";

				//break column
			} else 	if (coln eq breakcoln) {

				//print the old break value
				cell = breakvalue(coln);
				var oconvx = coldict(coln).extract(7);
				if (oconvx)
					cell = cell.oconv(oconvx);

				//store the new break value
				breakvalue(coln) = scol(coln);

				if (coln eq pagebreakcoln) {
					newhead = orighead;
					temp = scol(coln);
					if (oconvx)
						temp = temp.oconv(oconvx);
					temp.swapper(SQ, "\'\'");
					newhead.swapper("\'B\'", temp);
				}

				if (detsupp < 2 and not anytotals)
					cell = "&nbsp";

				//other columns are blank
			} else {
				cell = "";

				if (1 or detsupp < 2) {
					cell = oldbreakvalue(coln);
					if (breakcolns.locateusing(coln, FM, colbreakn)) {
						if (colbreakn < leveln)
							cell = "Total";
					}
				}

			}

			if (wcol(coln)) {
				if (!html) {
					cell = cell.substr(1, wcol(coln));
					cell = cell.oconv(coldict(coln).extract(11));
					tx ^= cell ^ tdx;
				} else {
					tx ^= "<td";
					if (!usecols)
						tx ^= coldict(coln).extract(14);
					if (coldict(coln).extract(9) eq "R")
						tx ^= " style=\"text-align:right\"";
					tx ^= ">";
					tx ^= cell ^ tdx;
				}
			}

		};//coln;

		//breakrowexit:
		if (html)
			tx ^= trx;

		if (detsupp < 2) {
			if (leveln > 1 and not html)
				tx ^= FM ^ underline;
		} else {
			if (!html)
				tx ^= FM ^ underline2;
		}

		//option to suppress the current level
		//or if this is the first record cannot be any totals before it.
		if ((breakoptions.extract(leveln)).index("X", 1) or recn eq 1)
			tx = storetx;

	};//leveln;

	//force new page and new heading
	//if newhead and detsupp<2 then
	//if newhead then
	// head=newhead
	// bodyln=999
	// end

	//if recn>1 then
	//if only one record found then avoid skipping printing totals for det-supp
	if (detsupp < 2 or recn > 1) {
		if (!html) {
			if (!anytotals)
				tx = bar;
		}

		if (newhead and html and tx)
			tx ^= "</tbody></table>";
		if (tx)
			printer1.printtx(tx);
	} else {
		tx = "";
	}

	//force new page and new heading
	//if newhead and detsupp<2 then
	if (newhead) {
		head = newhead;
		bodyln = 999999;
	}

	return;
}

///////////////////
subroutine addstr()
///////////////////
{

	str3 = str2;
	if (str3.length() < str1.length())
		str3 ^= space(str1.length() - str2.length());
	for (var ii = 1; ii <= str1.length(); ii++) {
		var char1 = str1.substr(ii, 1).trim();
		if (char1 not_eq "") {
			var char2 = str3.substr(ii, 1);
			if (char2 eq " ") {
				str3.splicer(ii, 1, char1);
			} else {
				//if num(char1) else char1=1
				//if num(char2) else char2=1
				if (char1.isnum() and char2.isnum()) {
					char3 = char1 + char2;
					if (char3 > 9)
						char3 = "*";
				} else {
					char3 = char1;
				}
				str3.splicer(ii, 1, char3);
			}
		}
	};//ii;

	return;
}

//following are stubs to old external functions that may or may not be reimplemented in exodus.

subroutine getmark(in mode, in html, out mark)
{
	mark="";
	return;
}

subroutine addunits(in newunits, out totunits)
{
	return;
}

subroutine mssg(in msg)
{
	printl(msg);
}

subroutine fsmsg()
{
	printl("fsmsg():");
}

subroutine msg2(in msg, in options, out buffer, in params)
{
	printl(msg, " ", options, " ", params);
	if (options.index("R"))
		stop("msg2()");
}

subroutine dicti2a(out dictrec)
{
	return;
}

function esctoexit()
{
	return false;
}

//all exodus programs need to finish up with "programexit"
//in MV terms, this simply corresponds to the final END statement of a program except that it is mandatory.
//in OO terms, this
//1. defines a contructor that sets up the "mv" exodus global variable that provides mv.RECORD etc.
//2. closes the class that programinit opened
//3. sets up the required global main() function that will instantiate the class run its main() function

programexit()
