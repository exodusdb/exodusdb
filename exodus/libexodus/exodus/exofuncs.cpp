/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//#include <cstdlib> //for atexit()

#include <exodus/var.h>
#include <exodus/exoenv.h>
#include <exodus/exofuncs.h>

namespace exodus {


//PUBLIC bool assigned(CVR var1) {
//	return var1.assigned();
//}
//
//PUBLIC bool unassigned(CVR var1) {
//	return !var1.assigned();
//}
//
//PUBLIC void move(VARREF fromvar, VARREF tovar) {
//	fromvar.move(tovar);
//}
//
//PUBLIC void swap(VARREF var1, VARREF var2) {
//	var1.swap(var2);
//}


PUBLIC var input() {
	var v;
	v.input();
	return v;
}

PUBLIC var input(CVR prompt) {
	var v;
	v.input(prompt);
	return v;
}

PUBLIC var inputn(const int nchars) {
	var v;
	v.inputn(nchars);
	return v;
}


//PUBLIC ND bool isterminal() {
//	return var().isterminal();
//}
//
//PUBLIC ND bool hasinput(const int millisecs) {
//	return var().hasinput(millisecs);
//}
//
//PUBLIC ND bool eof() {
//	return var().eof();
//}
//
//PUBLIC bool echo(const int on_off) {
//	return var().echo(on_off);
//}
//
//
//PUBLIC var len(CVR var1) {
//	return var1.len();
//}
//
//PUBLIC var textlen(CVR var1) {
//	return var1.textlen();
//}
//
//
//PUBLIC var convert(CVR instring, SV fromchars, SV tochars) {
//	return instring.convert(fromchars, tochars);
//}
//PUBLIC VARREF converter(VARREF iostring, SV fromchars, SV tochars) {
//	return iostring.converter(fromchars, tochars);
//}
//
//PUBLIC var textconvert(CVR instring, SV fromchars, SV tochars) {
//	return instring.textconvert(fromchars, tochars);
//}
//PUBLIC VARREF textconverter(VARREF iostring, SV fromchars, SV tochars) {
//	return iostring.textconverter(fromchars, tochars);
//}
//
//
PUBLIC var replace(CVR instring, SV fromstr, SV tostr) {
	var newstring = instring; //GB ??? why new var unlike others e.g textconvert( or convert(
	return newstring.replace(fromstr, tostr);
}
//PUBLIC VARREF replacer(VARREF iostring, SV fromstr, SV tostr) {
//	return iostring.replacer(fromstr, tostr);
//}
//
//
//PUBLIC VARREF regex_replacer(VARREF iostring, SV regex, SV replacement, SV options) {
//	return iostring.regex_replacer(regex, replacement, options);
//}
PUBLIC var regex_replace(CVR instring, SV regex, SV replacement, SV options) {
	var newstring = instring;
	return newstring.regex_replacer(regex, replacement, options);
}
//
//
//PUBLIC VARREF ucaser(VARREF iostring) {
//	return iostring.ucaser();
//}
//PUBLIC var ucase(CVR instring) {
//	return instring.ucase();
//}
//
//
//PUBLIC VARREF lcaser(VARREF iostring) {
//	return iostring.lcaser();
//}
//PUBLIC var lcase(CVR instring) {
//	return instring.lcase();
//}
//
//
//PUBLIC VARREF tcaser(VARREF iostring) {
//	return iostring.tcaser();
//}
//PUBLIC var tcase(CVR instring) {
//	return instring.tcase();
//}
//
//
//PUBLIC VARREF fcaser(VARREF iostring) {
//	return iostring.fcaser();
//}
//PUBLIC var fcase(CVR instring) {
//	return instring.fcase();
//}
//
//
//PUBLIC VARREF normalizer(VARREF iostring) {
//	return iostring.normalizer();
//}
//PUBLIC var normalize(CVR instring) {
//	return instring.normalize();
//}
//
//
//PUBLIC VARREF uniquer(VARREF iostring) {
//	return iostring.uniquer();
//}
//PUBLIC var unique(CVR instring) {
//	return instring.unique();
//}
//
//
//PUBLIC VARREF inverter(VARREF iostring) {
//	return iostring.inverter();
//}
//PUBLIC ND var invert(CVR instring) {
//	return var(instring).invert();
//}
//PUBLIC var invert(var&& instring) {
//	return instring.inverter();
//}
//
//
//PUBLIC VARREF lowerer(VARREF iostring) {
//	return iostring.lowerer();
//}
//PUBLIC var lower(CVR instring) {
//	return instring.lower();
//}
//
//
//PUBLIC VARREF raiser(VARREF iostring) {
//	return iostring.raiser();
//}
//PUBLIC var raise(CVR instring) {
//	return instring.raise();
//}


// POP

//PUBLIC VARREF popper(VARREF iostring) {
//	return iostring.popper();
//}
//PUBLIC var pop(CVR instring) {
//	return instring.pop();
//}
//
//
//PUBLIC VARREF quoter(VARREF iostring) {
//	return iostring.quoter();
//}
//PUBLIC var quote(CVR instring) {
//	return instring.quote();
//}
//
//PUBLIC VARREF squoter(VARREF iostring) {
//	return iostring.squoter();
//}
//PUBLIC var squote(CVR instring) {
//	return instring.squote();
//}
//
//
//PUBLIC VARREF unquoter(VARREF iostring) {
//	return iostring.unquoter();
//}
//PUBLIC var unquote(CVR instring) {
//	return instring.unquote();
//}
//
//
//PUBLIC VARREF fieldstorer(VARREF iostring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {
//	return iostring.fieldstorer(sepchar, fieldno, nfields, replacement);
//}
//PUBLIC var fieldstore(CVR instring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {
//	return instring.fieldstore(sepchar, fieldno, nfields, replacement);
//}
//
//
//PUBLIC VARREF cropper(VARREF iostring) {
//	return iostring.cropper();
//}
//PUBLIC var crop(CVR instring) {
//	return instring.crop();
//}
////
//PUBLIC bool dimread(dim& dimrecord, CVR dbfilevar, CVR key) {
//	return dimrecord.read(dbfilevar, key);
//}
//
//PUBLIC bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key) {
//	return dimrecord.write(dbfilevar, key);
//}
//
//
//PUBLIC var chr(const int integer) {
//	return var().chr(integer);
//}
//
//PUBLIC var textchr(const int integer) {
//	return var().textchr(integer);
//}
//
//PUBLIC var match(CVR instring, CVR matchstr, CVR options) {
//	return instring.match(matchstr, options);
//}
//
//PUBLIC var seq(CVR char1) {
//	return char1.seq();
//}
//
//PUBLIC var textseq(CVR char1) {
//	return char1.textseq();
//}
//
//PUBLIC var str(CVR instring, const int number) {
//	return instring.str(number);
//}
//
// MATH/BOOLEAN
//PUBLIC var abs(CVR num1) {
//	return num1.abs();
//}
//
//PUBLIC var pwr(CVR base, CVR exponent) {
//	return base.pwr(exponent);
//}
//
//PUBLIC var exp(CVR exponent) {
//	return exponent.exp();
//}
//
//PUBLIC var sqrt(CVR num1) {
//	return num1.sqrt();
//}
//
//PUBLIC var sin(CVR degrees) {
//	return degrees.sin();
//}
//
//PUBLIC var cos(CVR degrees) {
//	return degrees.cos();
//}
//
//PUBLIC var tan(CVR degrees) {
//	return degrees.tan();
//}
//
//PUBLIC var atan(CVR degrees) {
//	return degrees.atan();
//}
//
//PUBLIC var loge(CVR num1) {
//	return num1.loge();
//}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
//PUBLIC var integer(CVR num1) {
//	return num1.integer();
//}
//
//PUBLIC var floor(CVR num1) {
//	return num1.floor();
//}
//
//PUBLIC var round(CVR num1, const int ndecimals) {
//	return num1.round(ndecimals);
//}
//
//PUBLIC var rnd(const int number) {
//	return var(number).rnd();
//}
//
//PUBLIC void initrnd(CVR seed) {
//	seed.initrnd();
//}
//
//PUBLIC var mod(CVR dividend, CVR limit) {
//	return dividend.mod(limit);
//}
//
//PUBLIC var mod(CVR dividend, const double limit) {
//	return dividend.mod(limit);
//}
//
//PUBLIC var mod(CVR dividend, const int limit) {
//	return dividend.mod(limit);
//}
//
//PUBLIC var space(const int number) {
//	return var(number).space();
//}
//
//
//PUBLIC var fcount(CVR instring, SV substr) {
//	return instring.fcount(substr);
//}
//
//PUBLIC var count(CVR instring, SV substr) {
//	return instring.count(substr);
//}
//
//
//PUBLIC var substr(CVR instring, const int startindex) {
//	return instring.b(startindex);
//}
//
//PUBLIC var substr(CVR instring, const int startindex, const int length) {
//	return instring.b(startindex, length);
//}
//
//PUBLIC VARREF substrer(VARREF iostring, const int startindex) {
//	return iostring.substrer(startindex);
//}
//
//PUBLIC VARREF substrer(VARREF iostring, const int startindex, const int length) {
//	return iostring.substrer(startindex, length);
//}
//
//
//PUBLIC bool starts(CVR instring, SV substr) {
//	return instring.starts(substr);
//}
//
//PUBLIC bool ends(CVR instring, SV substr) {
//	return instring.ends(substr);
//}
//
//PUBLIC bool contains(CVR instring, SV substr) {
//	return instring.contains(substr);
//}
//
//
//PUBLIC var index(CVR instring, SV substr) {
//	return instring.index(substr);
//}
//
//PUBLIC var index(CVR instring, SV substr, const int startindex) {
//	return instring.index(substr, startindex);
//}
//
//PUBLIC var indexn(CVR instring, SV substr, const int occurrenceno) {
//	return instring.indexn(substr, occurrenceno);
//}
//
//PUBLIC var indexr(CVR instring, SV substr, const int startindex) {
//	return instring.indexr(substr, startindex);
//}
//
//
//PUBLIC var field(CVR instring, SV substr, const int fieldno, const int nfields) {
//	return instring.field(substr, fieldno, nfields);
//}
//
//PUBLIC var field2(CVR instring, SV substr, const int fieldno, const int nfields) {
//	return instring.field2(substr, fieldno, nfields);
//}
//
PUBLIC bool connect(CVR connectioninfo) {
	var conn1;
	return conn1.connect(connectioninfo);
}
//
//PUBLIC void disconnect() {
//	var().disconnect();
//}
//
//PUBLIC void disconnectall() {
//	var().disconnectall();
//}
//
//PUBLIC var lasterror() {
//	return var().lasterror();
//}
//
//PUBLIC bool dbcreate(CVR dbname) {
//	return dbname.dbcreate(dbname);
//}
//
//PUBLIC var dblist() {
//	return var().dblist();
//}
//
//PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname) {
//	return var().dbcopy(from_dbname, to_dbname);
//}
//
//PUBLIC bool dbdelete(CVR dbname) {
//	return var().dbdelete(dbname);
//}
//
PUBLIC bool createfile(CVR dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "").replacer("REVBOOT ", "").replacer("DATAVOL ", "").trimmer();

	return dbfilename2.createfile(dbfilename2);
}

PUBLIC bool deletefile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

PUBLIC bool clearfile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

//PUBLIC bool renamefile(CVR dbfilename_or_var, CVR newdbfilename) {
//	return dbfilename_or_var.renamefile(dbfilename_or_var.f(1), newdbfilename);
//}

PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname, CVR dictdbfilename) {

	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.createindex(fieldname2, dictdbfilename);
	}

	return dbfilename_or_var.createindex(fieldname, dictdbfilename);
}

PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname) {
	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.deleteindex(fieldname2);
	}

	return dbfilename_or_var.deleteindex(fieldname);
}
//
//PUBLIC bool begintrans() {
//	return var().begintrans();
//}
//
//PUBLIC bool statustrans() {
//	return var().statustrans();
//}
//
//PUBLIC bool rollbacktrans() {
//	return var().rollbacktrans();
//}
//
//PUBLIC bool committrans() {
//	return var().committrans();
//}
//
//PUBLIC void cleardbcache() {
//	var().cleardbcache();
//}
//
//
//PUBLIC bool lock(CVR dbfilevar, CVR key) {
//	return (bool)dbfilevar.lock(key);
//}
//
//PUBLIC void unlock(CVR dbfilevar, CVR key) {
//	dbfilevar.unlock(key);
//}
//
//PUBLIC void unlockall() {
//	var().unlockall();
//}
//
//PUBLIC bool open(CVR dbfilename, VARREF dbfilevar) {
//	return dbfilevar.open(dbfilename);
//}
//
//PUBLIC bool open(CVR dbfilename) {
//	var dbfilevar;
//	return dbfilevar.open(dbfilename);
//}
//
//PUBLIC bool read(VARREF record, CVR dbfilevar, CVR key) {
//	return record.read(dbfilevar, key);
//}
//
//PUBLIC bool reado(VARREF record, CVR dbfilevar, CVR key) {
//	return record.reado(dbfilevar, key);
//}
//
//PUBLIC bool readv(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber) {
//	return record.readv(dbfilevar, key, fieldnumber);
//}
//
//PUBLIC bool write(CVR record, CVR dbfilevar, CVR key) {
//	return record.write(dbfilevar, key);
//}
//
//PUBLIC bool writeo(CVR record, CVR dbfilevar, CVR key) {
//	return record.writeo(dbfilevar, key);
//}
//
//PUBLIC bool writev(CVR record, CVR dbfilevar, CVR key, const int fieldno) {
//	return record.writev(dbfilevar, key, fieldno);
//}
//
//PUBLIC bool updaterecord(CVR record, CVR dbfilevar, CVR key) {
//	return record.updaterecord(dbfilevar, key);
//}
//
//PUBLIC bool insertrecord(CVR record, CVR dbfilevar, CVR key) {
//	return record.insertrecord(dbfilevar, key);
//}
//
// PUBLIC var remove(CVR fromstr, VARREF startindex, VARREF delimiterno)
//PUBLIC var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno) {
//	// return fromstr.remove(startindex,delimiterno);
//	return fromstr.substr2(startindex, delimiterno);
//}
//
//PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {
//	return instring.pickreplace(fieldno, valueno, subvalueno, replacement);
//}
//PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement) {
//	return instring.pickreplace(fieldno, valueno, replacement);
//}
//PUBLIC var pickreplace(CVR instring, const int fieldno, CVR replacement) {
//	return instring.pickreplace(fieldno, replacement);
//}
//
//PUBLIC var extract(CVR instring, const int fieldno, const int valueno, const int subvalueno) {
//	return instring.f(fieldno, valueno, subvalueno);
//}
//
//PUBLIC var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {
//	return instring.insert(fieldno, valueno, subvalueno, insertion);
//}
//PUBLIC var insert(CVR instring, const int fieldno, const int valueno, CVR insertion) {
//	return instring.insert(fieldno, valueno, insertion);
//}
//PUBLIC var insert(CVR instring, const int fieldno, CVR insertion) {
//	return instring.insert(fieldno, insertion);
//}
//
// PUBLIC var erase(CVR instring, const int fieldno, const int valueno, const int
// subvalueno)
//PUBLIC var remove(CVR instring, const int fieldno, const int valueno, const int subvalueno) {
//	// return instring.erase(fieldno, valueno, subvalueno);
//	return instring.remove(fieldno, valueno, subvalueno);
//}
//
//PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {
//	return iostring.r(fieldno, valueno, subvalueno, replacement);
//}
//PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement) {
//	return iostring.r(fieldno, valueno, replacement);
//}
//PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement) {
//	return iostring.r(fieldno, replacement);
//}
//
//PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {
//	return iostring.inserter(fieldno, valueno, subvalueno, insertion);
//}
//
//PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion) {
//	return iostring.inserter(fieldno, valueno, insertion);
//}
//
//PUBLIC VARREF inserter(VARREF iostring, const int fieldno, CVR insertion) {
//	return iostring.inserter(fieldno, insertion);
//}
//
//// PUBLIC VARREF eraser(VARREF iostring, const int fieldno, const int valueno, const int subvalueno)
//PUBLIC VARREF remover(VARREF iostring, const int fieldno, const int valueno, const int subvalueno) {
//	// return iostring.eraser(fieldno, valueno, subvalueno);
//	return iostring.remover(fieldno, valueno, subvalueno);
//}
//
//PUBLIC bool locate(CVR target, CVR instring) {
//	return instring.locate(target);
//}
//
//PUBLIC bool locate(CVR target, CVR instring, VARREF setting) {
//	return instring.locate(target, setting);
//}
//
//PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
//	return instring.locate(target, setting, fieldno, valueno);
//}
//
//PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting) {
//	return instring.locateby(ordercode, target, setting);
//}
//
//PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
//	return instring.locateby(ordercode, target, setting, fieldno, valueno);
//}
//
//PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting) {
//	return instring.locateby(ordercode, target, setting);
//}
//
//PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
//	return instring.locateby(ordercode, target, setting, fieldno, valueno);
//}

//PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring) {
//	return instring.locateusing(usingchar, target);
//}
//
//PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting) {
//	return instring.locateusing(usingchar, target, setting);
//}
//
//PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno, const int subvalueno) {
//	return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);
//}
//
//PUBLIC var sum(CVR instring, SV sepchar) {
//	return instring.sum(sepchar);
//}
//
//PUBLIC var sum(CVR instring) {
//	return instring.sum();
//}
//
//PUBLIC var listfiles() {
//	return var().listfiles();
////}
//
//PUBLIC var reccount(CVR dbfilename_or_var) {
//	return dbfilename_or_var.reccount();
//}
//
//PUBLIC var listindex(CVR dbfilename, CVR fieldname) {
//	return var().listindex(dbfilename, fieldname);
//}

PUBLIC
int exodus_main(int exodus__argc, const char* exodus__argv[], ExoEnv& mv, int threadno) {

	// signal/interrupt handlers
	// install_signals();
	//var().breakon();
	breakon();

	//	tss_environmentns.reset(new int(0));
	//	global_environments.resize(6);
	//int environmentn = 0;
	mv.init(threadno);
	// mv.DICT.outputl("DICT=");
	//	global_environments[environmentn] = &mv;

	mv.EXECPATH = getexecpath();
	if (not mv.EXECPATH) {
		if (exodus__argc)
			mv.EXECPATH = var(exodus__argv[0]);
		if (not mv.EXECPATH.contains(OSSLASH))
			mv.EXECPATH.prefixer(oscwd() ^ OSSLASH);
	}
	//"see getting path to current executable" above
	// or use "which EXECPATH somehow like in mvdebug.cpp
	// if (not EXECPATH.contains(OSSLASH) && not EXECPATH.contains(":"))
	//{
	//	EXECPATH.prefixer(oscwd()^OSSLASH);
	//	if (OSSLASH=="\\")
	//		EXECPATH.converter("/","\\");
	//}
	// leave a global copy where backtrace can get at it
	//EXECPATH2 = mv.EXECPATH;

	mv.SENTENCE = "";  // ALN:TODO: hm, again, char->var-> op=(var)

	mv.COMMAND = "";
	mv.OPTIONS = "";

	// reconstructs complete original sentence unfortunately quote marks will have been lost
	// unless escaped needs to go after various exodus definitions how the MSVCRT tokenizes
	// arguments http://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	// http://stackoverflow.com/questions/4094699/how-does-the-windows-command-interpreter-cmd-exe-parse-scripts
	for (int ii = 0; ii < exodus__argc; ++ii) {
		var word = var(exodus__argv[ii]);
		if (ii == 0) {
			word = word.field2(OSSLASH, -1);
			// remove trailing ".exe"
			if (word.lcase().ends(".exe"))
				word.cutter(-4);
		} else
			mv.SENTENCE ^= " ";

		//dont do this because easier to understand bash expansion without it
		// put back quotes if any spaces
		//if (word.contains(" "))
		//	word.quoter();

		mv.SENTENCE ^= word;
		mv.COMMAND ^= word ^ FM_;
	}
	mv.COMMAND.popper();

	// Cut off OPTIONS from end of COMMAND if present
	// *** SIMILAR code in
	// 1. exofuncs.cpp exodus_main()
	// 2. exoprog.cpp  perform()
	// OPTIONS are in either (AbC) or {AbC} in the last field of COMMAND
	mv.OPTIONS = mv.COMMAND.field2(_FM, -1);
	if ((mv.OPTIONS.starts("{") and mv.OPTIONS.ends("}")) or (mv.OPTIONS.starts("(") and mv.OPTIONS.ends(")"))) {
		// Remove last field of COMMAND TODO fpopper command?
		mv.COMMAND.cutter(-mv.OPTIONS.len() - 1);
		// Remove first { or ( and last ) } chars of OPTIONS
		mv.OPTIONS.cutter(1).popper();
	} else {
		mv.OPTIONS = "";
	}

	var temp;
	// DBTRACE=osgetenv("EXO_DBTRACE",temp)?1:-1;

	// would have to passed in as a function pointer
	// main2(exodus__argc, exodus__argv);

	//atexit(exodus_atexit);

	if (isterminal())
		mv.TERMINAL = osgetenv("TERM");

	return 0;
}

//PUBLIC
//void exodus_atexit()
//{
//	// done in handle cache destructor now
//	//	var().disconnect();
//}

/* getting path to current executable

from http://stackoverflow.com/questions/143174/c-c-how-to-obtain-the-full-path-of-current-directory

Windows:

int bytes = GetModuleFileName(nullptr, pBuf, len);
if(bytes == 0)
	return -1;
else
	return bytes;

Linux but not macosx apparently:

char szTmp[32];
sprintf(szTmp, "/proc/%d/exe", getpid());
int bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
if(bytes >= 0)
	pBuf[bytes] = '\0';
return bytes;
*/

}  // namespace exodus
