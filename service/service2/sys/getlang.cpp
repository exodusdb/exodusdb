#include <exodus/library.h>
libraryinit()


#include <gen.h>
#include <fin.h>

var progname;
var languagecode;
var temp;
var question;
var reply;
var langkey;
var codepage;

function main(in origprogname, in languagecode0, in origdatatype, io languagefile, io lang) {
	//c sys in,in,in,io,io
	//global question,temp,languagecode,progname

	lang = "";
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
		origlanguagecode = gen.company.a(14);
		if (origlanguagecode == "ENGLISH") {
			origlanguagecode = "";
		}
		if (not origlanguagecode) {
			origlanguagecode = fin.currcompany;
		}
	}

	progname = origprogname;
	languagecode = origlanguagecode;
	var datatype = origdatatype;

	lang = "";

	//try with data type if present
	if (datatype) {
		gosub getlang2( origprogname,  datatype,  languagefile,  lang);
		if (lang) {
			goto exit;
		}
	}

	//try without data type
	languagecode = origlanguagecode;
	datatype = "";
	gosub getlang2( origprogname,  datatype,  languagefile,  lang);
	if (lang) {
		goto exit;
	}

	//check if user wants to continue without text
	var().chr(7).output();
	temp = progname;
	temp.r(-1, languagecode);
	temp.r(-1, datatype);
	question = temp ^ "|TEXT IS MISSING";
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
	if (origprogname == "GENERAL") {
		call osgetenv("CODEPAGE", codepage);

		//greek
		if (codepage == "737") {
			if (not(codepage.read(languagefile, "GENERAL*GREEK"))) {
				codepage = "";
			}
getupperlower:
			lang.r(9, codepage.a(1, 9));
			lang.r(10, codepage.a(1, 10));

		//central european including poland
		} else if (codepage == "852") {
			if (not(codepage.read(languagefile, "GENERAL*POLISH"))) {
				codepage = "";
			}
			goto getupperlower;
			{}
		}
//L460:
	}

	return 0;

}

subroutine getlang2(in origprogname, in datatype, in languagefile, io lang) {
	//getlang2(in origprogname, in datatype, in languagefile, io lang)
	//try with language if present
	if (languagecode) {
		gosub getlang3( origprogname,  datatype,  languagefile,  lang);
		if (lang) {
			return;
		}
	}

	//try without language
	languagecode = "";
	gosub getlang3( origprogname,  datatype,  languagefile,  lang);

	return;

}

subroutine getlang3(in origprogname, in datatype, in languagefile, io lang) {
	//getlang3(in origprogname, in datatype, in languagefile, io lang)

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
		if (origprogname ne "SORTORDER" and lang.a(1).count(VM)) {
			lang = raise(lang.a(1));

			//strip out English pretext
			if (lang.index(var().chr(170), 1)) {
				var nn = lang.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var tt = lang.a(ii).field(var().chr(170), 2);
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
