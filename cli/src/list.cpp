/*
//Exodus Copyright (c) 2009 Stephen Bush
//Licence http://www.opensource.org/licenses/mit-license.php

This is a complete ACCESS/ENGLISH/LIST replacement that can output in DOS or HTML format.

It is an attempt to show a substantial program written in Exodus's reincarnation of Multivalue Basic.
The original Pick Basic version was in heavy production usage for many years.

It is written using Exodus OO flavour syntax not traditional flavour.

Traditional - honed by usage over the decades. function names on the left and parameters on the right.

	printl(oconv(field(extract(xx,6),"/",2),"D2"));

OO flavour - does exactly the same thing but is perhaps easier to read from left to right in one pass:

    xx.a(6).field(L"/",2).oconv(L"D2").outputl();

xx.a(6) stands for "xx<6>" in mv/pick. <> brackets cannot be used for functions in c++. "a" stands for "attribute".

Hybrid:

    printl(xx.a(6).field(L"/",2).oconv(L"D2"));

Comments about style:

	1. Decided to stick with wordier "xx.a(99)" instead exodus's "xx(99)" for now because
		a. Exodus lack of xx<99> syntax makes it harder to understand what xx(99) means
		*especially* because list also makes liberal use of dim arrays which have the same round brackets
		b. It makes LIST marginally more portable into languages like java which are slightly less flexible syntaxwise
	2. Using "int" instead of "var" for "for loop" index variables despite the fact that int doesnt always behave
		precisely like pick integers, speed is important in this program and C++ allows us access to raw power when we need it.
	3. Note the familiar "goto" statement which considerably eases porting of mv basic to Exodus.
		This can be a major point scorer for Exodus because "goto" is banished from almost all other languages.
		Exodus's "goto" cannot jump over "var" statements or into/out of subroutines some code refactoring
		may be still be required during porting.
	4. L prefix to all strings is not required but faster for utility programs *unbenchmarked

usage

list ads brand_code brand_name with brand_code 'XYZ'

NB if using from shells like bash then double quotes must be prefixed by \ and () should be done with {}

Type just list by itself to get a summary of its syntax

*/

#include <exodus/program.h>

//For normal programs, global variables that should be generally available to all subroutines
//should be declared, and optionally *simply* initialised, before the programinit() line.
//"simply initialised" means set to expressons using raw numbers and strings.
//eg 'var xx="xyz";' or 'var xx=100.5;' etc.

//NB FOLLOWING SECTION is only declaration and optional *simple* initialisation of global variables.
//In Exodus, all programming proper is to be done inside the main or other functions and subroutines
//AFTER the "programinit()" line which can be found below.

//these global variables are not thread safe
//unless they are put as MEMBER variables
//ie after the programinit() and before the programexit line
//and outside the main and other functions (which are c++ inline member functions)
//but then they cannot be simply initialised
//and would have to be initialised in the main() or other function

var datafile;

var quotes=DQ^SQ;
//var crlf="\r\n";
//put() on standard out (on windows at least) converts \n to 0d 0a automatically
//and manually outputing 0d 0a results in 0d 0d 0a in the file
var crlf="\n";

//some identifying name (from environment?)
var company="";

var wordn;
//var sentencex=COMMAND;//only const exodus system variables can be used before programinit()
var sentencex="";
var ignorewords="";
var html;
var printing;//only prefers @lptr to @crt if no columns specified
var tx;
var ncols;
var breakleveln;
var breakcolns="";
var blockn=0;
var nblocks=0;
var orighead;
var detsupp="";
var nbreaks=0;
var anytotals=0;
var recn;
var bodyln;

var breakoptions="";
var pagebreakcoln="";
var topmargin = 0;
var totalflag = 0;
var breakonflag = 0;

//use <COL> for hiding non-totalled cols in det-supp (slow)
var	usecols = 0;

var startcharn;
var newmarklevel;
var previousmarklevel;
var lastid;
var crtx="";

var idsupp="";
var dblspc="";
var maxnrecs=0;
var keylist="";
var gtotsupp="";
var gtotreq="";
var nobase="";

var limits="";
var nlimits=0;

int coln=0;

var head = L"";
var foot = L"";

var tr;
var trx;
var td;
var tdx;
var tt;
var nbsp;
var dictmd;
var decimalchar;

//when exodus gets a non-destructive redim then this max columns restriction will be removed
#define maxncols 1000
dim colname(maxncols);
//dim colfile(maxncols);
dim coldict(maxncols);
dim mcol(maxncols);
dim pcol(maxncols);
dim ccol(maxncols);
dim scol(maxncols);
dim icol(maxncols);
dim wcol(maxncols);

//the following will be redim'ed  to the required size further down
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

//TODO maybe implement printtext as normal exodus internal or external subroutine
//although it is a good example how you can use objects in Exodus.
#include "printtext.h"

//programinit() is required AFTER program globals and BEFORE all local functions and subroutines including main()
//in MV terms, "programinit" indicates the start of a set of functions and variables that form a single "program"
//in OO terms, "programinit()" indicates the start of a "class"
programinit()

//all programs must have a "function main()" that returns an integer - usually 0 to indicate success.

/////////////////
function main() {
/////////////////
USER0="";
	printer1.init(mv);

	//@sentence='LIST 10 SCHEDULES BY VEHICLE_CODE with vehicle_code "kjh kjh" VEHICLE_NAME BREAK-ON VEHICLE_CODE BREAK-ON VEHICLE_CODE TOTAL PRICE (SX)'
	//@sentence='list markets code name'

	if (dcount(COMMAND,FM)==1)
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
		"\nOptions: (H)tml (P)rint"
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

	sentencex=COMMAND.convert(FM," ");
	var options=OPTIONS.ucase();

	//declare function get.cursor,put.cursor

	if (USER2[2] eq L"C")
		decimalchar = L",";
	else
		decimalchar = ".";

	DICT = L"";

	html = index(options,"H");
	printing = index(options,"P");

	if (html) {
		tr = L"<tr>";
		trx = L"</tr>" ^ crlf;
		td = L"<td>";
		tdx = L"</td>";
		nbsp = L"&nbsp;";
		tt = SYSTEM.a(2);
		tt.swapper(L".txt", L".htm");
		SYSTEM.r(2, tt);
		printer1.html=1;
	} else {
		tr = L"";
		trx = crlf;
		td = L"";
		tdx = L" ";
		nbsp = L"";
	}

	//automatically create dict_md if it is not present so you can list dictionaries
	if (not open(L"dict_md",dictmd)) {
		createfile(L"dict_md");
		if (open(L"dict_md",dictmd)) {

			//prepare some dictionary records
			var dictrecs = L"";
			dictrecs  =      L"@id   |F|0 |Id     |S|||||L|20";
			dictrecs ^= FM ^ L"type  |F|1 |Type   |S|||||L|4";
			dictrecs ^= FM ^ L"fmc   |F|2 |Field  |S|||||R|3";
			dictrecs ^= FM ^ L"title |F|3 |Title  |M|||||T|20";
			dictrecs ^= FM ^ L"sm    |F|4 |SM     |S|||||L|1";
			dictrecs ^= FM ^ L"part  |F|5 |Part   |S|||||R|2";
			dictrecs ^= FM ^ L"conv  |F|7 |Convert|S|||||T|20";
			dictrecs ^= FM ^ L"just  |F|9 |Justify|S|||||L|3";
			dictrecs ^= FM ^ L"length|F|10|Length |S|||||R|6";
			dictrecs ^= FM ^ L"master|F|28|Master |S|||||L|1";
			dictrecs ^= FM ^ L"@crt  |G|  |"
			"type fmc part title sm conv just length master"
			" by type by fmc by part by @id";

			//write the dictionary records to the dictionary
			var nrecs=dictrecs.dcount(FM);
			for (var recn = 1; recn <= nrecs; recn++) {
				var dictrec=dictrecs.a(recn);
				while (dictrec.index(L" |"))
					dictrec.swapper(L" |","|");
				var key=field(dictrec,"|",1);
				var rec=field(dictrec,"|",2,9999);
				if (key.a(1)=="F")
					rec.r(28,0,0,1);//master
				//printl(key ^ L": " ^ rec);
				write(rec.convert(L"|",FM), dictmd, key);
			}
		}
	}

////////
//init1:
////////

	var thcolor = SYSTEM.a(46, 1);
	var tdcolor = SYSTEM.a(46, 2);
	var reportfont = SYSTEM.a(46, 3);
	if (not tdcolor)
		tdcolor = L"#FFFFC0";
	if (not thcolor)
		thcolor = L"#FFFF80";
	if (sentencex.index(L" det-supp", 1))
		detsupp = 1;
	if (sentencex.index(L" det-supp2", 1))
		detsupp = 2;

	if (not open(L"dict_md", dictmd))
		//stop(L"Cannot open dict_md");
		dictmd="";


//initphrase:
/////////////
	var ss = L"";
	wordn = 0;

///////////
nextphrase:
///////////

	gosub getword();
	if (word eq L"")
		goto x1exit;

phraseinit:
///////////
	if (word.substr(1, 4) eq L"sort" or word.substr(1, 4) eq L"list") {

		if (word.index(L"sort"))
			ss ^= L"s";
		ss ^= L"select";
		//filename:
		gosub getword();
		if (not word)
			abort(L"FILE NAME IS REQUIRED");

		//limit number of records
		if (word.match(L"\\d+","r")) {
			maxnrecs = word;
			ss ^= L" " ^ maxnrecs;
			gosub getword();
		}

		//get the filename (or dict filename as dict_filename)
		if (word eq L"dict") {
			gosub getword();
			word = L"dict_" ^ word;
		}
		filename = word;
		if (not srcfile.open(filename))
			abort(filename^" file does not exist");

		if (filename.substr(1, 5).lcase() eq L"dict_")
			dictfilename = L"md";
		else
			dictfilename = filename;

		if (not DICT.open(L"dict_"^dictfilename)) {
			dictfilename = L"dict_md";
			DICT = dictmd;
		}

		//add the filename to the sort/select command
		ss ^= L" " ^ word;

		//get any specific keys (numbers or quoted words)
		while (nextword ne L"" and (nextword.isnum() or nextword[1] eq SQ or nextword[1] eq DQ)) {
			gosub getword();
			keylist = 1;
			ss ^= L" " ^ word;
		}

	} else if (word eq L"getlist") {
		gosub getword();		//var(L"GETLIST " ^ word).perform();
		perform(L"getlist "^word);

	} else if (word eq L"and" or word eq L"or") {
		ss ^= L" " ^ word;

	} else if (word eq L"(" or word eq L")") {
		ss ^= L" " ^ word;

	} else if (word eq L"by" or word eq L"by-dsnd") {
		ss ^= L" " ^ word;
		gosub getword();
		ss ^= L" " ^ word;

	} else if (word eq L"with not" or word eq L"with" or word eq L"without" or word eq L"limit") {
		ss ^= L" " ^ word;

		var limit = word eq L"limit";
		if (limit)
			nlimits += 1;

		gosub getword();

		//NO/EVERY
		if (word eq L"not" or word eq L"no" or word eq L"every") {
			ss ^= L" " ^ word;
			gosub getword();
		}

		//field or NO
		ss ^= L" " ^ word;
		if (limit)
			limits.r(1, nlimits, word);

		//negate next comparision
		if (var(L"not,ne,<>").locateusing(nextword, L",", xx)) {
			nextword = L"not";
			gosub getword();
			ss ^= L" " ^ word;
		}

		//comparision
		if (var(L"match,eq,ne,gt,lt,ge,le,[,],[]").locateusing(nextword, L",", xx)) {
			gosub getword();
			ss ^= L" " ^ word;
			if (limit)
				limits.r(2, nlimits, word);
		}

		//with x between y and z
		//with x from y to z
		if (nextword eq L"between" or nextword eq L"from") {
			gosub getword();
			ss ^= L" " ^ word;
			gosub getword();
			ss ^= L" " ^ word;
			gosub getword();
			ss ^= L" " ^ word;
			gosub getword();
			ss ^= L" " ^ word;

		} else {

			//parameters
			while (true) {

				///BREAK;
				if (not (nextword ne L"" and (nextword.isnum() or nextword[1] eq DQ or nextword[1] eq SQ)))
					break;

				gosub getword();
				ss ^= L" " ^ word;
				if (limit) {
					word.unquoter();
					if (word eq L"")
						word = L"\"\"";
					//append a subvalue
					limits.r(3, nlimits, -1, word);
				}
			}//loop;

		}

	} else if (word eq L"break-on") {
		breakcolns.splicer(1, 0, (coln + 1) ^ FM);
		breakoptions.splicer(1, 0, FM);
		nbreaks += 1;
		breakonflag = 1;

	} else if (word eq L"grand-total") {
		//zzz throw away the grand total options for the time being
		gosub getword();
		gtotreq = 1;

	} else if (word eq L"no-base") {
		nobase = 1;

	//"DET-SUPP"
	} else if (word eq L"det-supp") {
		detsupp = 1;

	//"DET-SUPP"
	} else if (word eq L"det-supp2") {
		detsupp = 2;

	//"GTOT-SUPP"
	} else if (word eq L"gtot-supp") {
		gtotsupp = 1;

	//case dictrec
	} else if (word eq L"total") {
		totalflag = 1;

	} else if (word eq L"using") {
		gosub getword();
		dictfilename = word;
		if (not DICT.open(L"dict_"^dictfilename)) {
			fsmsg();
			abort(L"");
		}

	} else if (word eq L"heading") {

		gosub getword();
		head = word;
		if (html) {
			head.swapper(L"Page \'P\'", L"");
			head.swapper(L"Page \'P", SQ);
		}
		head.splicer(1, 1, L"");
		head.splicer(-1, 1, L"");

	} else if (word eq L"footing") {
		gosub getword();
		foot = word;
		foot.splicer(1, 1, L"");
		foot.splicer(-1, 1, L"");

	//justlen
	} else if (word eq L"justlen") {
		if (not coln) {
			mssg(L"justlen/jl must follow a column name");
			abort(L"");
		}
		gosub getword();
		word.splicer(1, 1, L"");
		word.splicer(-1, 1, L"");
		coldict(int(coln)).r(9, word[1]);
		coldict(coln).r(10, word[3]);
		coldict(coln).r(11, word);

	//colhead
	} else if (word eq L"colhead") {
		gosub getword();
		//skip if detsupp2 and column is being skipped
		if (coldict(coln).assigned()) {
			word.unquoter();
			word.converter(L"|", VM);
			coldict(coln).r(3, word);
		}

	} else if (word eq L"oconv") {
		gosub getword();
		word.splicer(1, 1, L"");
		word.splicer(-1, 1, L"");
		if (html)
			word.swapper(L"[DATE]", L"[DATE,*]");
		coldict(coln).r(7, word);

	} else if (word eq L"id-supp") {
		idsupp = 1;

	} else if (word eq L"dbl-spc") {
		dblspc = 1;

	} else if (dictrec) {

		if (var(L"FSDIA").index(dictrec.a(1), 1)) {

			var nn;

			//pick items
			if (var(L"DI").index(dictrec.a(1), 1))
				dicti2a(dictrec);

			//pick A equ F
			if (dictrec.a(1) eq L"A")
				dictrec.r(1, L"F");

			//suppress untotalled columns if doing detsupp2
			if (detsupp eq 2) {
				//if (var(L"JL,JUSTLEN,CH,COL,HEAD,OC,OCONV").locateusing(nextword, L",", xx)) {
				if (var(L"justlen,colhead,oconv").locateusing(nextword, L",", xx)) {
					gosub getword();
					gosub getword();
				}
				if (not (totalflag or breakonflag))
					goto dictrecexit;
			}

			coln += 1;
			colname(coln) = word;

			//increase column width if column title needs it
			nn = (dictrec.a(3)).dcount(VM);
			for (var ii = 1; ii <= nn; ii++) {
				tt = dictrec.a(3, ii);
				var f10=dictrec.a(10);
				if (f10=="" or (f10 and tt.length() > f10))
					dictrec.r(10, tt.length());
			};//ii;

			if (detsupp < 2) {
				if (not (totalflag or breakonflag)) {
					tt = L" id=\"BHEAD\"";
					if (detsupp)
						tt ^= L" style=\"display:none\"";
					dictrec.r(14, tt);
				}
			}

			//total required ?
			if (totalflag) {
				totalflag = 0;
				dictrec.r(12, 1);
				anytotals = 1;
			} else
				dictrec.r(12, 0);

			if (html) {
				tt = dictrec.a(7);
				tt.swapper(L"[DATE]", L"[DATE,*]");
				if (tt eq L"[DATE,4]")
					tt = L"[DATE,4*]";
				dictrec.r(7, tt);
				if (tt eq L"[DATE,*]")
					dictrec.r(9, L"R");
			}
			coldict(coln) = dictrec;

			//store the format in a convenient place
			if (html)
				tt = L"";
			else {
				tt = coldict(coln).a(9);
				if (tt)
					tt ^= L"#" ^ coldict(coln).a(10);
			}
			coldict(coln).r(11, tt);

			//this could be a break-on column and have break-on options
			//if coln=breakcolns<1> then
			if (breakonflag) {
				coldict(coln).r(13, 1);
				breakonflag = 0;
				if (nextword[1] eq DQ) {
					gosub getword();
					//zzz break options
					if (word.index(L"B", 1))
						pagebreakcoln = coln;
					breakoptions.r(1, word);
				}
			}
		}

	} else if (word eq L"ignoreword") {
		gosub getword();
		ignorewords.r(1, -1, word);

	//@LPTR word is skipped if not located in MD/DICT.MD
	} else if (word eq L"@lptr" or word eq L"@crt") {

	} else {
		//sys.messages W156
		//tt = L"\"%1%\" is an unrecognized word.Please correct the word by retyping it|or pressing (F4] to edit it.Press [Esc) to re-start.";
		tt = L"\"%1%\" is an unrecognized word.";
		tt.swapper(L"%1%", word);
		mssg(tt, L"RCE", word, word);
		//if (word eq var().chr(27))
			abort(L"");
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
	//if (not (coln or crtx) and (DICT ne dictmd or datafile eq L"md" or datafile eq L"dict_md")) {
	if (not (coln or crtx) and DICT) {
	// and ((DICT.ucase() ne dictmd.ucase()) or (filename.ucase() eq L"MD") or (filename.ucase() eq L"DICT_MD"))) {

		var words=printing ? L"@lptr,@crt" : L"@crt,@lptr";
		for (int ii=1;ii<=2;++ii) {
			word = words.field(L",",ii);
			if (not xx.read(DICT, word)) {
				//word.ucaser().outputl();
				if (not xx.read(DICT, word)) {
					//word.outputl("word not found:");
					word = L"";
				}
			} //else word.outputl("word found:");

			if (word) {
				crtx = 1;
				sentencex ^= L" " ^ word;
				charn = sentencex.length() - word.length();
				goto nextphrase;
			}
		}
	}

	ncols = coln;

	//insert the @id column
	if (not idsupp) {

		ncols += 1;

		//move the columns up by one to make space for a new column 1
		for (int coln = ncols; coln >= 2; coln--) {
			coldict(coln) = coldict(coln - 1);
			colname(coln) = colname(coln - 1);
		};//coln;

		//set column 1 @ID
		colname(1) = L"@" L"id";
		if (not DICT or not coldict(1).read(DICT, L"@" L"id")) {
			if (not dictmd or not coldict(1).read(dictmd, L"@" L"ID"))
				coldict(1) = L"F" ^ FM ^ L"0" ^ FM ^ L"Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ L"L" ^ FM ^ 15;
		}
		if (html)
			tt = L"";
		else
			tt = coldict(1).a(9) ^ L"#" ^ coldict(1).a(10);

		coldict(1).r(11, tt);

		//increment the list of breaking columns by one as well
		for (var breakn = 1; breakn <= nbreaks; breakn++)
			breakcolns.r(breakn, breakcolns.a(breakn) + 1);
		pagebreakcoln += 1;

	}

	//work out column "widths". zero suppresses a column. blank or anything else allows it.
	for (int coln=1; coln<=ncols; ++coln) {
		var tt=coldict(coln).a(10);
		if (tt eq L"")
			tt="10";
		wcol(coln)=tt;
	}

	if (breakcolns[-1] eq FM)
		breakcolns.splicer(-1, 1, L"");

	if (breakoptions[-1] eq FM)
		breakoptions.splicer(-1, 1, L"");

	//make underline and column title underline
	if (not html) {
		underline = L"";
		colunderline = L"";
		for (int coln = 1; coln <= ncols; coln++) {
			if (wcol(coln)) {
				if (coldict(coln).a(12)) {
					tt = L"-";
				} else {
					tt = L" ";
				}
				underline ^= tt.str(wcol(coln)) ^ L" ";
				colunderline ^= var(L"-").str(wcol(coln)) ^ L" ";
			}
		};//coln;
		bar = var(L"-").str(colunderline.length() - 1);
	}

////////
//init2:
////////

	tx = "";

	if (filename.unassigned()||not filename)
		abort(L"Filename not specified");

	if (not open(filename, srcfile))
		abort(L"Cannot open file L" ^ filename);

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
	var colheading = L"";
	var coltags = L"";
	var coln2 = 0;
	for (int coln = 1; coln <= ncols; coln++) {

		//suppress drilldown if no totals or breakdown
		if (not anytotals or not nbreaks)
			coldict(coln).r(14, L"");

		if (wcol(coln)) {
			if (html) {
				coln2 += 1;

				//t='<th bgcolor=':thcolor
				tt = L"<th ";
				//if detsupp=1 then t:=coldict(coln)<14>
				if (not usecols)
					tt ^= coldict(coln).a(14);
				tt ^= L">" ^ coldict(coln).a(3) ^ L"</th>";
				colheading.r(coln2, tt);

				colheading.swapper(VM, L"<br />");
				coltags.r(-1, L"<col");
				var align = coldict(coln).a(9);
				if (align eq L"R") {
					//if index(coldict(coln)<7>,'[NUMBER',1) then
					// *http://www.w3.org/TR/html401/struct/tables.html#adef-align-TD
					// coltags:=' align="char" char="':decimalchar:'"'
					//end else
					//coltags:=' align="right"'
					coltags ^= L" style=\"text-align:right\"";
					// end
				} else if (align eq L"T")
					//coltags:=' align="left"'
					coltags ^= L" style=\"text-align:left\"";

				if (usecols)
					coltags ^= coldict(coln).a(14);
				coltags ^= L" />";
			} else {
				for (var ii = 1; ii <= 9; ++ii)
					colheading.r(ii, colheading.a(ii) ^ (coldict(coln).a(3, ii)).oconv(coldict(coln).a(11)) ^ L" ");
			}
		}
	}

	//change all "(Base)" in dictionary column headings to the base currency
	//unless the keyword NO-BASE is present in which case replace with blank
	//this is useful if all the columns are base and no need to see the currency

	//c language family conditional assign is fairly commonly understood
	//and arguably quite readable if the alternatives are simple
	//compare to (if (nobase) ... " a little below
	tt = html ? L"" : L"C#6";

	var t2 = company.a(3);
	if (t2) {
		if (nobase)
			t2 = L"";
		else
			t2 = L"(" ^ t2 ^ L")";
		colheading.swapper(L"(Base)", t2.oconv(tt));
		colheading.swapper(L"%BASE%", company.a(3));
	}

	//trim off blank lines (due to the 9 above)
	if (html) {

		tt = L"";

		gosub getmark(L"CLIENT", html, clientmark);
		tt ^= clientmark;

		//tt ^= L"<table class=\"maintable\" border=\"1\" cellspacing=\"0\" cellpadding=\"2\"";
		tt ^= L"<table border=\"1\" cellspacing=\"0\" cellpadding=\"2\"";
		tt ^= L" align=\"center\" ";
		tt ^= L" style=\"font-size:66%;font-family:arial,sans-serif";
		tt ^= L";background-color:" ^ tdcolor;
		tt ^= L";page-break-after:avoid";
		tt ^= L"\">";
		tt ^= FM ^ L"<colgroup>" ^ coltags ^ L"</colgroup>";
		tt ^= FM ^ L"<thead>";
		//t:=fm:coltags

		tt ^= tr ^ colheading ^ trx ^ FM ^ L"</thead>";
		tt ^= FM ^ L"<tbody>";
		tt.transfer(colheading);

		//allow for single quotes
		colheading.swapper(SQ, L"\'\'");

	} else
		colheading.trimmerb(L" "^FM);

	//heading options

	if (head eq L"")
			head = filename ^ space(10) ^ L" \'T\'";

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
		head.swapper(FM, L"<br />");
		head = L"<h2~style=\"text-align:center\">" ^ head ^ L"</h2>";
	}

	//footing options

	if (dblspc)
		head ^= FM;
	if (not html)
		head ^= FM ^ colunderline;
	head ^= FM ^ colheading;
	if (not html)
		head ^= FM ^ colunderline;
	if (dblspc)
		head ^= FM;
	orighead = head;
	printer1.setheadfoot(mv,head,foot);

	//required before select()
	begintrans();

//////////
//initrec:
//////////
	var selectedok;

	if (ss.count(L" ") > 2 or keylist) {
		//call mssg('Selecting records, please wait.||(Press Esc or F10 to interrupt)','UB',buffer,'')
		//perform ss:' (S)'
		//call perf(ss:' (S)')
		selectedok=filename.select(ss);

	} else {
		if (LISTACTIVE)
			selectedok=true;
		else
			selectedok=srcfile.select(ss);
	}

	if (not selectedok) {
		call mssg(L"No records found");

		abort(L"");

/*
		if (html)
			tx ^= L"</tbody></table>";

		tx ^= L"No records listed";
		printer1.printtx(mv,tx);
*/

	} else {
		recn = L"";
		lastid = L"%EXODUS_%%%%%_NONE%";
		gosub process_all_records();
	}

//x2bexit:
//////////

	//remove while no header being output
	if (html)
		tx ^= L"</tbody></table></div></body></html>";

	printer1.printtx(mv, tx);

	printer1.close(mv);

	return 0;
}

//////////////////////////////
subroutine process_all_records()
//////////////////////////////
{

//returns true if completes or false if interrupted

	while (true) {

		if (esctoexit()) {
			tx = L"";
			if (html)
				tx ^= L"</tbody></table>";
			tx ^= L"*** incomplete - interrupted ***";
			printer1.printtx(mv,tx);
			clearselect();
			//goto x2bexit;
			return;
		}

		//limit number of records (allow one more to avoid double closing since nrecs limited in select clause as well
		if (maxnrecs and recn >= (maxnrecs+1))
			filename.clearselect();

		//readnext key
		FILEERRORMODE = 1;
		FILEERROR = L"";
		if (not filename.readnext(ID, MV)) {

			FILEERRORMODE = 0;
			/* treat all errors as just "no more records"
			if (_STATUS) {
			tx = L"*** Fatal Error " ^ FILEERROR.a(1) ^ L" reading record " ^ ID ^ L" ***";
			printer1.printtx(mv,tx);
			abort(L"");
			}
			if (FILEERROR.a(1) eq 421) {
			tx = L"Operation aborted by user.";
			printer1.printtx(mv,tx);
			abort(L"");
			}
			if (FILEERROR and FILEERROR.a(1) ne 111) {
			tx = L"*** Error " ^ FILEERROR.a(1) ^ L" reading record " ^ ID ^ L" ***";
			printer1.printtx(mv,tx);
			readerr += 1;
			abort(L"");
			}
			*/

			return;
		}

		//skip record keys starting with "%" (control records)
		if (ID[1] eq L"%")
			continue;

		//read the actual record (TODO sb modify select statement to return the actual columns required)
		//or skip if disappeared
		if (not RECORD.read(srcfile, ID))
			continue;

		//designed to filter multivalues which are not selected properly
		//unless sorted "by"
		if (limits) {
			var nfns = RECORD.dcount(FM);
			//find maximum var number
			var nvars = 1;
			for (var fn = 1; fn <= nfns; fn++) {
				temp = RECORD.a(fn);
				if (temp.length()) {
					temp = temp.dcount(VM);
					if (temp > nvars)
						nvars = temp;
				}
			};//fn;
			//for each limit pass through record deleting all unwanted multivalues
			for (var limitn = 1; limitn <= nlimits; limitn++) {

				//calculate() accesses data via dictionary keys
				var limitvals = calculate(var(limits.a(1, limitn)));

				for (var varx = nvars; varx >= 1; varx--) {
					temp = limitvals.a(1, varx);
					if (temp eq L"")
						temp = L"\"\"";
					//locate temp in (limits<3,limitn>)<1,1> using sm setting x else
					if (not limits.a(3, limitn).locateusing(temp, SVM, xx)) {
						for (var fn = 1; fn <= nfns; fn++) {
							var varalues = RECORD.a(fn);
							if (varalues.count(VM))
								RECORD.eraser(fn, varx, 0);
						};//fn;
					}
				};//varx;
			};//limitn;
		}

		recn += 1;

		//process/output one record
		gosub process_one_record();

	}//readnext


	//print the closing subtotals
	breakleveln = nbreaks;
	if (not gtotsupp and (not pagebreakcoln or gtotreq))
		breakleveln += 1;
	gosub printbreaks();

	bodyln = 1;

	if (html)
		tx ^= L"<p style=\"text-align:center\">";
	tx ^= FM ^ (recn + 0) ^ L" record";
	if (recn ne 1)
		tx ^= L"s";
	if (html)
		tx ^= L"</p>";

	if (not detsupp)
		tx ^= L"<script type=\"text/javascript\"><!--" ^ FM ^ L" togglendisplayed+=" ^ nblocks ^ FM ^ L"--></script>";

	return;
}

///////////////////////////////
subroutine process_one_record()
///////////////////////////////
{

	//visible progress indicator
	//var().getcursor();
	//if (not SYSTEM.a(33)) {
	//	print(AW.a(30), var().cursor(36, CRTHIGH / 2));
	//}else{
	//	print(var().cursor(0));
	//}
	//print(recn, L" ");
	//cursor.setcursor();

	var fieldno;
	var keypart;
	var cell;

	//get the data from the record into an array of columns
	for (int coln = 1; coln <= ncols; coln++) {

		//dont call calculate except for S items because some F items
		//are constructed and/or exist in dict_md
		dictrec=coldict(coln);
		if (dictrec.a(1) eq L"F") {
			fieldno=dictrec.a(2);
			if (not fieldno) {
				cell=ID;
				keypart=dictrec.a(5);
				if (keypart)
					cell=cell.field(L'*',keypart);
			} else {
				if (dictrec.a(4)[1] eq L"M")
					cell=RECORD.a(fieldno,MV);
				else
					cell=RECORD.a(fieldno);
			}
		//calculate() accesses data via dictionary keys
		} else 
			cell=calculate(colname(coln));

		if (not html and dictrec.a(9) == L"T")
			mcol(coln)=cell.oconv(dictrec.a(11));
		else
			mcol(coln)=cell;

		if (html)
			mcol(coln).swapper(TM, L"<br />");

		pcol(coln) = 1;
		ccol(coln) = 7;
		scol(coln) = mcol(coln);
	}//coln;

	//break subtotals

	//detect most major level to break
	if (recn eq 1) {
		//print breaks will not actually print before the first record
		// but it needs to set the various break values
		breakleveln = nbreaks;
	} else {
		int coln;
		int leveln;//used after loop so cant declare in the "for" statement
		for (leveln = nbreaks; leveln >= 1; leveln--) {
			coln = breakcolns.a(leveln);

			///BREAK;
			if (scol(coln) ne breakvalue(coln))
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

				//"remove" is a rarely used pick function that extracts text from a given character
				//number up to the next pick field, value, subvalue or other separator character.
				//It doesnt actually "remove" any text but it does also move the pointer up
				//ready for the next extraction and tells you what the next separator character is
				icol(coln)=mcol(coln).remove(pcol(coln), ccol(coln));

				scol(coln) = icol(coln);
			}
			if (ccol(coln) > newmarklevel)
				newmarklevel = ccol(coln);
		};//coln;

		//break totals - add at the bottom level (1)
		for (int coln = 1; coln <= ncols; coln++) {
			//only totalled columns
			if (coldict(coln).a(12)) {
				if (icol(coln)) {
					if (html) {
						//breaktotal(coln,1)+=i.col(coln)
						addunits(icol(coln), breaktotal(coln,1));
					} else {
						if ((breaktotal(coln,1)).isnum() and (icol(coln)).isnum()) {
							breaktotal(coln,1) += icol(coln);
						} else {
							if (colname(coln) eq L"DATEGRID") {
								str1 = icol(coln);
								str2 = breaktotal(coln,1);
								gosub addstr();
								breaktotal(coln,1) = str3;
							}
						}
					}
				}
				breakcount(1) += 1;
				icol(coln) = L"";
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
				tx ^= L"<tr";
				if (blockn) {
					if (detsupp eq 1)
						tx ^= L" style=\"display:none\"";
					tx ^= L" id=\"B" ^ blockn ^ DQ;
					//clicking expanded det-supped details collapses it
					if (detsupp)
						tx ^= L" style=\"cursor:pointer\" onclick=\"toggle(B" ^ blockn ^ L")\"";
				}
				tx ^= L">";
			}

			//output the columns

			for (int coln = 1; coln <= ncols; coln++) {
				tt = scol(coln);

				//oconv
				var oconvx = coldict(coln).a(7);
				if (oconvx) {
					tt = tt.oconv(oconvx);

					//prevent html folding of numbers on minus sign
					if (html) {
						if (tt[1] eq L"-") {
							if (oconvx.substr(1, 7) eq L"[NUMBER")
								tt = L"<nobr>" ^ tt ^ L"</nobr>";
						}
					}
				}

				//non-zero width columns
				if (wcol(coln)) {

					//non-html format to fixed with with spaces
					if (not html)
						tt = tt.oconv(coldict(coln).a(11));

					//html blank is nbsp
					if (tt eq L"")
						tt = nbsp;

					//add one column
					tx ^= td ^ tt ^ tdx;
				}
			};//coln;

			//terminate the row and output it
			//tx ^= trx;
			printer1.printtx(mv,tx);

			//loop back if any multivalued lines/folded text
			if (newmarklevel) {
				for (int coln = 1; coln <= ncols; coln++)
					scol(coln) = L"";
				previousmarklevel = newmarklevel;
				continue;
			}

			//double space
			if (dblspc)
				printer1.printtx(mv,tx);

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

		if (word.length() and ignorewords.locateusing(word, VM)) {
			continue;
		}
		break;
	}

	if (word eq L"") {
		nextword = L"";
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

	//if (not quotes.index(word[1]))
	//	word.ucaser();
	//if (not quotes.index(nextword[1]))
	//	nextword.ucaser();

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

		word = L"";
		charn += 1;
		while (true) {

			///BREAK;
			if (sentencex[charn] ne L" ")
				break;
			charn += 1;
			if (charn > sentencex.length())
				return;
		}//loop;

		startcharn = charn;
		var charx = sentencex[charn];

		if (quotes.index(charx, 1)) {
			searchchar = charx;
		} else {
			searchchar = L" ";
		}
		word ^= charx;

		while (true) {
			charn += 1;
			charx = sentencex[charn];

			///BREAK;
			if (charx eq L"" or charx eq searchchar)
				break;

			word ^= charx;
		}

		if (searchchar ne L" ") {
			word ^= searchchar;
			charn += 1;
		} else {
			//word.ucaser();
			word.trimmerf().trimmerb();
		}

		//options
		if (word[1] eq L"(" and word[-1] eq L")") {
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
	if (DICT ne L"" and dictrec.read(DICT, word)) {
		if (dictrec.a(1) eq L"G") {
			tt = dictrec.a(3);
			tt.converter(VM, L" ");
			sentencex.splicer(startcharn, word.length(), tt);
			charn = startcharn - 1;
			wordn -= 1;
			//goto getword2;
			gosub getword2();
			return;
		}
	} else {
		dictrec = L"";
		if (dictmd and dictrec.read(dictmd, word)) {
			if (dictrec.a(1) eq L"RLIST") {
				if (dictrec.a(4))
					word = dictrec.a(4);
				dictrec = L"";
			}
		}
	}
	dictrec.converter(L"|", VM);

	if (word eq L"=")
		word = L"EQ";
	else if (word eq L"EQUAL")
		word = L"EQ";
	else if (word eq L"<>")
		word = L"NE";
	else if (word eq L">")
		word = L"GT";
	else if (word eq L"<")
		word = L"LT";
	else if (word eq L">=")
		word = L"GE";
	else if (word eq L"<=")
		word = L"LE";
	/*
	if (word eq L"CONTAINING")
	word = L"()";
	if (word eq L"ENDING")
	word = L"[";
	if (word eq L"STARTING")
	word = L"]";
	*/

	return;
}

////////////////////////
subroutine printbreaks()
////////////////////////
{

	if (not breakleveln)
		return;

	var newhead = L"";

	//print breaks from minor level (1) up to required level
	//required level can be nbreaks+1 (ie the grand total)
	//for leveln=1 to breakleveln
	//for leveln=breakleveln to 1 step -1

	for (var leveln = 1; leveln <= breakleveln; leveln++) {
		var breakcoln = breakcolns.a(leveln);

		var storetx = tx;
		//output any prior tx is this level is suppressed
		//if index(breakoptions<leveln>,'X',1) and tx then
		// if recn>1 then gosub printtx
		// end

		var lastblockn = blockn;
		blockn = 0;
		if (detsupp) {
			if (tx and tx[-1] ne FM)
				tx ^= FM;
			if (leveln > 1 and not html)
				tx ^= underline ^ FM;
		} else {
			if (not html) {
				//underline2=if breakleveln>=nbreaks then bar else underline
				underline2 = (leveln eq 1) ? (underline) : (bar);
				if (not tx.substr(-2, 2).index(L"-", 1)) {
					if (tx[-1] ne FM)
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
				tx ^= L" style=\"cursor:pointer\" onclick=\"toggle(B" ^ lastblockn ^ L")\"";
			if (detsupp < 2 or (nbreaks > 1 and leveln > 1))
				tx ^= L" style=\"font-weight:bold\"";
			tx ^= L">";
		}
		for (int coln = 1; coln <= ncols; coln++) {

			//total column
			if (coldict(coln).a(12)) {
				cell = breaktotal(coln,leveln);

				//add into the higher level
				if (leveln <= nbreaks) {

					if (cell) {
						if (html) {
							//breaktotal(coln,leveln+1)+=cell
							gosub addunits(cell, breaktotal(coln,leveln + 1));
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
				var oconvx = coldict(coln).a(7);
				if (oconvx)
					cell = cell.oconv(oconvx);

				if (html) {
					if (cell eq L"")
						cell = nbsp;
					else
						cell.swapper(VM, L"<br />");
				}

				//and clear it
				breaktotal(coln,leveln) = L"";

				//break column
			} else 	if (coln eq breakcoln) {

				//print the old break value
				cell = breakvalue(coln);
				var oconvx = coldict(coln).a(7);
				if (oconvx)
					cell = cell.oconv(oconvx);

				//store the new break value
				breakvalue(coln) = scol(coln);

				if (coln eq pagebreakcoln) {
					newhead = orighead;
					temp = scol(coln);
					if (oconvx)
						temp = temp.oconv(oconvx);
					temp.swapper(SQ, L"\'\'");
					newhead.swapper(L"\'B\'", temp);
				}

				if (detsupp < 2 and not anytotals)
					cell = L"&nbsp";

				//other columns are blank
			} else {
				cell = L"";

				if (1 or detsupp < 2) {
					cell = oldbreakvalue(coln);
					if (breakcolns.locateusing(coln, FM, colbreakn)) {
						if (colbreakn < leveln)
							cell = L"Total";
					}
				}

			}

			if (wcol(coln)) {
				if (not html) {
					cell = cell.substr(1, wcol(coln));
					cell = cell.oconv(coldict(coln).a(11));
					tx ^= cell ^ tdx;
				} else {
					tx ^= L"<td";
					if (not usecols)
						tx ^= coldict(coln).a(14);
					if (coldict(coln).a(9) eq L"R")
						tx ^= L" style=\"text-align:right\"";
					tx ^= L">";
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
			if (not html)
				tx ^= FM ^ underline2;
		}

		//option to suppress the current level
		//or if this is the first record cannot be any totals before it.
		if ((breakoptions.a(leveln)).index(L"X", 1) or recn eq 1)
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
		if (not html) {
			if (not anytotals)
				tx = bar;
		}

		if (newhead and html and tx)
			tx ^= L"</tbody></table>";
		if (tx)
			printer1.printtx(mv,tx);
	} else {
		tx = L"";
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
		var char1 = str1[ii].trim();
		if (char1 ne L"") {
			var char2 = str3[ii];
			if (char2 eq L" ") {
				str3.splicer(ii, 1, char1);
			} else {
				//if num(char1) else char1=1
				//if num(char2) else char2=1
				if (char1.isnum() and char2.isnum()) {
					char3 = char1 + char2;
					if (char3 > 9)
						char3 = L"*";
				} else
					char3 = char1;
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
	//prevent "warning unused" until properly implemented
	if (mode or html){};
}

subroutine addunits(in newunits, out totunits)
{
	return;
	//prevent "warning unused" until properly implemented
	if (newunits or totunits){};

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
//1. defines a contructor that sets up the "mv" exodus global variable that provides RECORD etc.
//2. closes the class that programinit opened
//3. sets up the required global main() function that will instantiate the class run its main() function

//debugprogramexit()
programexit()
