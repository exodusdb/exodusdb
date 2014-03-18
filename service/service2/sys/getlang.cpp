#include <exodus/library.h>
libraryinit()

#include <log2.h>

#include <gen.h>

var reply;
var langkey;
var codepage;
var languagecode;
var progname;
var datatype;
var origprogname;
var languagefile2;

var temp;
var question;
var logtime;

function main(in origprogname0, in languagecode0, in origdatatype, io languagefile, io lang) {
	lang = "";

	log2("getlang", logtime);

	origprogname=origprogname0;
	if (languagefile.unassigned()) {
		languagefile = "";
	}
	if (languagefile == "") {
		if (not(languagefile.open("ALANGUAGE", ""))) {
			return 0;
		}
	}
	languagefile2=languagefile;
	log2("origlanguagecode", logtime);
	var origlanguagecode;
	if (languagecode0.assigned())
		origlanguagecode = languagecode0;
	else
		origlanguagecode = "";
	if (not origlanguagecode) {
		origlanguagecode = gen.company.a(14);
		if (origlanguagecode == "ENGLISH") {
			origlanguagecode = "";
		}
		if (not origlanguagecode) {
			origlanguagecode = gen.gcurrcompany;
		}
	}

	log2("progname", logtime);
	if (origprogname0.assigned())
		progname = origprogname0;
	else
		progname = "";
	languagecode = origlanguagecode;
	datatype = origdatatype;

	lang = "";

	log2("try with data type if present", logtime);
	if (datatype) {
		gosub getlang2(lang);
		if (lang) {
			goto exit;
		}
	}

	log2("try without data type", logtime);
	languagecode = origlanguagecode;
	datatype = "";
	gosub getlang2(lang);
	if (lang) {
		goto exit;
	}

/*
	log2("check if user wants to continue without text", logtime);
	var().chr(7).output();
	temp = progname;
	temp.r(-1, languagecode);
	temp.r(-1, datatype);
	question = temp;
	question ^= "|TEXT IS MISSING";
	question ^= "||DO YOU WANT TO CONTINUE ?";
	if (not(decide(question, "", reply))) {
		var().abort();
	}
	if (reply == 2) {
		var().abort();
	}
*/

exit:
	var custlang;
	if (custlang.read(DEFINITIONS, "LANGUAGE*" ^ langkey)) {
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
	if (origprogname0 == "GENERAL") {
		codepage.osgetenv("CODEPAGE");

		//greek
		if (codepage == "737") {
			if (not(codepage.read(languagefile, "GENERAL*GREEK"))) {
				codepage = "";
			}
getupperlower:
			lang.r(9, codepage.a(1, 9));
			lang.r(10, codepage.a(1, 10));

			//central european including poland
		} if (codepage == "852") {
			if (not(codepage.read(languagefile, "GENERAL*POLISH"))) {
				codepage = "";
			}
			goto getupperlower;
		}
	}

	return 0;

}

subroutine getlang2(io lang) {

	//try with language if present
	if (languagecode) {
		gosub getlang3(lang);
		if (lang) {
			return;
		}
	}

	//try without language
	languagecode = "";
	gosub getlang3(lang);

	return;

}

subroutine getlang3(io lang) {

	langkey = progname;
	if (languagecode) {
		langkey = langkey.fieldstore("*", 2, 1, languagecode);
	}
	if (datatype) {
		langkey = langkey.fieldstore("*", 3, 1, datatype);
	}
	//CALL MSG(T)
	if (lang.read(languagefile2, langkey)) {
		//convert to FM if not a format record
		//IF orig.progname<>'SORTORDER' and LANG<8>='' then
		if (origprogname ne "SORTORDER" and (lang.a(1)).count(VM)) {
			lang = lang.a(1).raise();

			//strip out English pretext
			var chr170=chr(170);
			if (lang.index(chr170, 1)) {
				var nn = lang.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var tt = lang.a(ii).field(chr170, 2);
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
