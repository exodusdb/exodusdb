#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <getlang.h>

#include <service_common.h>


// var nn;
var progname;
var languagecode;
var langkey;
var codepage;

func main(in origprogname, in languagecode0, in origdatatype, io languagefile, io lang) {

	lang = "";
//	if (languagefile.unassigned()) {
//		languagefile = "";
//	}
	languagefile.defaulter("");
	if (languagefile == "") {
		if (not languagefile.open("ALANGUAGE", "")) {
			return 0;
		}
	}

	var origlanguagecode = languagecode0;
	if (not origlanguagecode) {
		origlanguagecode = srv.company.f(14);
		if (origlanguagecode == "ENGLISH") {
			origlanguagecode = "";
		}
		if (not origlanguagecode) {
			origlanguagecode = srv.gcurrcompcode;
		}
	}

	if (origlanguagecode.contains("*")) {

		var	 langcode1 = origlanguagecode.field("*", 1);
		call getlang(origprogname, langcode1, origdatatype, languagefile, lang);
		var	 lang1 = lang;

		var	 langcode2 = origlanguagecode.field("*", 2);
		call getlang(origprogname, langcode2, origdatatype, languagefile, lang);
		var	 lang2 = lang;

		// bilingual
		if (lang2 and lang2 != lang1) {
			lang   = "";
			let n1 = lang1.fcount(FM);
			let n2 = lang2.fcount(FM);
			// 			if (n1 < n2) {
			// 				nn = n2;
			// 			} else {
			// 				nn = n1;
			// 			}
			let nn = (n1 < n2) ? n2 : n1;
			for (const var fn : range(1, nn)) {
				// lang(fn) = (lang1.f(fn) ^ " " ^ lang2.f(fn)).trim();
				let lang1line = lang1.f(fn);
				let lang2line = lang2.f(fn);
				let nparts	  = fcount(lang1line, "|");
				if (nparts == 1) {
					// eg English Arabic
					lang(fn) = (lang1line ^ " " ^ lang2line).trim();
				} else {
					// eg January Janvier|February Fevruar| etc.
					var bilingual = "";
					for (const var partn : range(1, nparts)) {
						bilingual ^= trim(field(lang1line, "|", partn) ^ " " ^ field(lang2line, "|", partn)) ^ "|";
					}
					// bilingual[-1,1]="";
					bilingual.popper();
					lang(fn) = bilingual;
				}
			}  // fn;
		} else {
			lang = lang1;
		}

		return 0;
	}

	progname	 = origprogname;
	languagecode = origlanguagecode;
	var datatype = origdatatype;

	lang = "";

	// try with data type if present
	if (datatype) {
		gosub getlang2(origprogname, datatype, languagefile, lang);
		if (lang) {
			goto exit;
		}
	}

	// try without data type
	languagecode = origlanguagecode;
	datatype	 = "";
	gosub getlang2(origprogname, datatype, languagefile, lang);
	if (lang) {
		goto exit;
	}

	// check if user wants to continue without text
	// PRINT CHAR(7):
	// TEMP=PROGNAME
	// TEMP<-1>=LANGUAGECODE
	// TEMP<-1>=DATATYPE
	// QUESTION=TEMP:'|TEXT IS MISSING'
	// QUESTION:='||DO YOU WANT TO CONTINUE ?'
	// IF DECIDE(QUESTION,'',REPLY) ELSE ABORT
	// IF REPLY=2 THEN ABORT

exit:
	// ///

	var custlang;
	if (custlang.read(DEFINITIONS, "LANGUAGE*" ^ langkey)) {
		let nn = custlang.fcount(FM);
		for (const var fn : range(1, nn)) {
			var tt = custlang.f(fn);
			if (tt.len()) {
				if (tt == "\"\"") {
					tt = "";
				}
				lang(fn) = custlang.f(fn);
			}
		}  // fn;
	}

	// force 737 greek codepage characters so indexing is ok etc
	if (origprogname == "GENERAL") {

		//call osgetenv("CODEPAGE", codepage);
		if (not osgetenv("CODEPAGE", codepage)) {
			//null
		}

		// greek
		if (codepage == "737") {
			if (not codepage.read(languagefile, "GENERAL*GREEK")) {
				codepage = "";
			}
getupperlower:
			lang(9)	 = codepage.f(1, 9);
			lang(10) = codepage.f(1, 10);

			// central european including poland
		} else if (codepage == "852") {
			if (not codepage.read(languagefile, "GENERAL*POLISH")) {
				codepage = "";
			}
			goto getupperlower;
		}
	}

	// swap '(Base)' with '(':base.currency:')' in lang
	lang.replacer("(Base)", "(" ^ SYSTEM.f(134) ^ ")");

	lang(100) = languagecode;
	return 0;
}

subr getlang2(in origprogname, in datatype, in languagefile, io lang) {

	// try with language if present
	if (languagecode) {
		gosub getlang3(origprogname, datatype, languagefile, lang);
		if (lang) {
			return;
		}
	}

	// try without language
	languagecode = "";
	gosub getlang3(origprogname, datatype, languagefile, lang);

	return;
}

subr getlang3(in origprogname, in datatype, in languagefile, io lang) {

	langkey = progname;
	if (languagecode) {
		langkey = langkey.fieldstore("*", 2, 1, languagecode);
	}
	if (datatype) {
		langkey = langkey.fieldstore("*", 3, 1, datatype);
	}
	// CALL MSG(T)
	if (not lang.read(languagefile, langkey)) {
		lang = "";
		return;
	}

	// convert to FM if not a format record
	// IF orig.progname<>'SORTORDER' and LANG<8>='' then
	if (origprogname != "SORTORDER" and lang.f(1).count(VM)) {
		lang = raise(lang.f(1));

		// strip out English pretext
		if (lang.contains(chr(170))) {
			let nn = lang.fcount(FM);
			for (const var ii : range(1, nn)) {
				let tt = lang.f(ii).field(chr(170), 2);
				if (tt) {
					lang(ii) = tt;
				}
			}  // ii;
		}
	}

	return;
}

}; // libraryexit()
