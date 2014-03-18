#include <exodus/library.h>
libraryinit()

#include <getenv.h>
#include <raise.h>

#include <g.h>
#include <f.h>

var reply;
var langkey;
var codepage;

function main() {
	//jbase linemark
	var lang = "";
	if (languagefile.unassigned()) {
		languagefile = "";
	}
	if (languagefile == "") {
		if (not(languagefile.open("ALANGUAGE", ""))) {
			return 0;
		}
	}

	var origlanguagecode = languagecode0;
	if (not origlanguagecode) {
		origlanguagecode = g.company.a(14);
		if (origlanguagecode == "ENGLISH") {
			origlanguagecode = "";
		}
		if (not origlanguagecode) {
			origlanguagecode = f.currcompany;
		}
	}

	var progname = origprogname;
	var languagecode = origlanguagecode;
	var datatype = origdatatype;

	lang = "";

	//try with data type if present
	if (datatype) {
		gosub getlang2();
		if (lang) {
			goto exit;
		}
	}

	//try without data type
	languagecode = origlanguagecode;
	datatype = "";
	gosub getlang2();
	if (lang) {
		goto exit;
	}

	//check if user wants to continue without text
	cout << var().chr(7);
	var temp = progname;
	temp.r(-1, languagecode);
	temp.r(-1, datatype);
	var question = temp ^ "|TEXT IS MISSING";
	question ^= "||DO YOU WANT TO CONTINUE ?";
	if (not(decide(question, "", reply))) {
		var().abort();
	}
	if (reply == 2) {
		var().abort();
	}

exit:
	/////

	var custlang;
	if (custlang.read(g._definitions, "LANGUAGE*" ^ langkey)) {
		var nn = custlang.count(FM) + 1;
		for (var fn = 1; fn <= nn; ++fn) {
			var tt = custlang.a(fn);
			if (tt.length()) {
				if (tt == "\"\"") {
					tt = "";
				}
				lang.r(fn, custlang.a(fn));
			}
		};//fn;
	}

	//force 737 greek codepage characters so indexing is ok etc
	if (origprogname == "GENERAL") {
		call getenv("CODEPAGE", codepage);

		//greek
		if (codepage == "737") {
			if (not(codepage.read(languagefile, "GENERAL*GREEK"))) {
				codepage = "";
			}
getupperlower:
			lang.r(9, codepage.a(1, 9));
			lang.r(10, codepage.a(1, 10));

			//central european including poland
			goto 451;
		}
		if (codepage == "852") {
			if (not(codepage.read(languagefile, "GENERAL*POLISH"))) {
				codepage = "";
			}
			goto getupperlower;
		}
	}

	return 0;

}

subroutine getlang2() {

	//try with language if present
	if (languagecode) {
		gosub getlang3();
		if (lang) {
			return;
		}
	}

	//try without language
	languagecode = "";
	gosub getlang3();

	return;

}

subroutine getlang3() {

	langkey = progname;
	if (languagecode) {
		langkey = langkey.fieldstore("*", 2, 1, languagecode);
	}
	if (datatype) {
		langkey = langkey.fieldstore("*", 3, 1, datatype);
	}
	//CALL MSG(T)
	if (lang.read(languagefile, langkey)) {
		//convert to FM if not a format record
		//IF orig.progname<>'SORTORDER' and LANG<8>='' then
		if (origprogname ne "SORTORDER" and (lang.a(1)).count(VM)) {
			lang = raise(lang.a(1));

			//strip out English pretext
			if (lang.index("ª", 1)) {
				var nn = lang.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var tt = lang.a(ii).field("ª", 2);
					if (tt) {
						lang.r(ii, tt);
					}
				};//ii;
			}

		}
	}
	return;

}


libraryexit()