#include <exodus/library.h>
libraryinit()

#include <getlang.h>

#include <sys_common.h>

var nn;
var progname;
var languagecode;
var langkey;
var codepage;

function main(in origprogname, in languagecode0, in origdatatype, io languagefile, io lang) {
	//c sys in,in,in,io,io

	#include <system_common.h>
	//global question,temp,languagecode,progname

	lang = "";
	if (languagefile.unassigned()) {
		languagefile = "";
	}
	if (languagefile eq "") {
		if (not(languagefile.open("ALANGUAGE", ""))) {
			return 0;
		}
	}

	var origlanguagecode = languagecode0;
	if (not origlanguagecode) {
		origlanguagecode = sys.company.a(14);
		if (origlanguagecode eq "ENGLISH") {
			origlanguagecode = "";
		}
		if (not origlanguagecode) {
			origlanguagecode = sys.gcurrcompany;
		}
	}

	if (origlanguagecode.index("*")) {

		var langcode1 = origlanguagecode.field("*", 1);
		call getlang(origprogname, langcode1, origdatatype, languagefile, lang);
		var lang1 = lang;

		var langcode2 = origlanguagecode.field("*", 2);
		call getlang(origprogname, langcode2, origdatatype, languagefile, lang);
		var lang2 = lang;

		//bilingual
		if (lang2 and lang2 ne lang1) {
			lang = "";
			var n1 = lang1.count(FM) + 1;
			var n2 = lang2.count(FM) + 1;
			if (n1 lt n2) {
				nn = n2;
			} else {
				nn = n1;
			}
			for (var fn = 1; fn <= nn; ++fn) {
				//lang.r(fn, (lang1.a(fn) ^ " " ^ lang2.a(fn)).trim());
				var lang1line = lang1.a(fn);
				var lang2line = lang2.a(fn);
				var nparts = dcount(lang1line, "|");
				if (nparts eq 1) {
					//eg English Arabic
					lang.r(fn, (lang1line ^ " " ^ lang2line).trim());
				} else {
					//eg January Janvier|February Fevruar| etc.
					var bilingual = "";
					for (var partn = 1; partn <= nparts; partn++) {
						bilingual ^= trim(field(lang1line, "|", partn) ^  " " ^ field(lang2line, "|", partn)) ^ "|";
					}
					//bilingual[-1,1]="";
					bilingual.splicer(-1, 1, "");
					lang.r(fn, bilingual);
				}
			} //fn;
		} else {
			lang = lang1;
		}

		return 0;
	}

	progname = origprogname;
	languagecode = origlanguagecode;
	var datatype = origdatatype;

	lang = "";

	//try with data type if present
	if (datatype) {
		gosub getlang2(origprogname, datatype, languagefile, lang);
		if (lang) {
			goto exit;
		}
	}

	//try without data type
	languagecode = origlanguagecode;
	datatype = "";
	gosub getlang2(origprogname, datatype, languagefile, lang);
	if (lang) {
		goto exit;
	}

	//check if user wants to continue without text
	//PRINT CHAR(7):
	//TEMP=PROGNAME
	//TEMP<-1>=LANGUAGECODE
	//TEMP<-1>=DATATYPE
	//QUESTION=TEMP:'|TEXT IS MISSING'
	//QUESTION:='||DO YOU WANT TO CONTINUE ?'
	//IF DECIDE(QUESTION,'',REPLY) ELSE ABORT
	//IF REPLY=2 THEN ABORT

exit:
/////

	var custlang;
	if (custlang.read(DEFINITIONS, "LANGUAGE*" ^ langkey)) {
		nn = custlang.count(FM) + 1;
		for (var fn = 1; fn <= nn; ++fn) {
			var tt = custlang.a(fn);
			if (tt.length()) {
				if (tt eq "\"\"") {
					tt = "";
				}
				lang.r(fn, custlang.a(fn));
			}
		} //fn;
	}

	//force 737 greek codepage characters so indexing is ok etc
	if (origprogname eq "GENERAL") {
		call osgetenv("CODEPAGE", codepage);

		//greek
		if (codepage eq "737") {
			if (not(codepage.read(languagefile, "GENERAL*GREEK"))) {
				codepage = "";
			}
getupperlower:
			lang.r(9, codepage.a(1, 9));
			lang.r(10, codepage.a(1, 10));

		//central european including poland
		} else if (codepage eq "852") {
			if (not(codepage.read(languagefile, "GENERAL*POLISH"))) {
				codepage = "";
			}
			goto getupperlower;
			{}
		}

	}

	//swap '(Base)' with '(':base.currency:')' in lang
	lang.swapper("(Base)", "(" ^ SYSTEM.a(134) ^ ")");

	lang.r(100, languagecode);
	return 0;
}

subroutine getlang2(in origprogname, in datatype, in languagefile, io lang) {

	//try with language if present
	if (languagecode) {
		gosub getlang3(origprogname, datatype, languagefile, lang);
		if (lang) {
			return;
		}
	}

	//try without language
	languagecode = "";
	gosub getlang3(origprogname, datatype, languagefile, lang);

	return;
}

subroutine getlang3(in origprogname, in datatype, in languagefile, io lang) {

	langkey = progname;
	if (languagecode) {
		langkey = langkey.fieldstore("*", 2, 1, languagecode);
	}
	if (datatype) {
		langkey = langkey.fieldstore("*", 3, 1, datatype);
	}
	//CALL MSG(T)
	if (not(lang.read(languagefile, langkey))) {
		lang = "";
		return;
	}

	//convert to FM if not a format record
	//IF orig.progname<>'SORTORDER' and LANG<8>='' then
	if (origprogname ne "SORTORDER" and lang.a(1).count(VM)) {
		lang = raise(lang.a(1));

		//strip out English pretext
		if (lang.index(var().chr(170))) {
			nn = lang.count(FM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				var tt = lang.a(ii).field(var().chr(170), 2);
				if (tt) {
					lang.r(ii, tt);
				}
			} //ii;
		}

	}

	return;
}

libraryexit()
