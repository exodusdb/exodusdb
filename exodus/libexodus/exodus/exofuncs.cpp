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

#undef DEFAULT_EMPTY
#undef DEFAULT_DOT
#undef DEFAULT_SPACE
#undef DEFAULT_CSPACE
#undef DEFAULT__FM
#undef DEFAULT_0
#undef DEFAULT_1
#undef DEFAULT_M1
#undef DEFAULT_FALSE

#define DEFAULT_EMPTY
#define DEFAULT_DOT
#define DEFAULT_SPACE
#define DEFAULT_CSPACE
#define DEFAULT__FM
#define DEFAULT_0
#define DEFAULT_1
#define DEFAULT_M1
#define DEFAULT_FALSE

namespace exodus {

PUBLIC ND var osgetenv(CVR envcode DEFAULT_EMPTY) {var envvalue = ""; envvalue.osgetenv(envcode); return envvalue;}
PUBLIC ND bool osgetenv(CVR code, VARREF value) {return value.osgetenv(code);}
PUBLIC void ossetenv(CVR code, CVR value) {return value.ossetenv(code);}

PUBLIC ND var ostempdirpath() {return var().ostempdirpath();}
PUBLIC ND var ostempfilename() {return var().ostempfilename();}

PUBLIC ND bool assigned(CVR var1) {return var1.assigned();}
PUBLIC ND bool unassigned(CVR var1) {return !var1.assigned();}

PUBLIC void move(VARREF fromvar, VARREF tovar) {fromvar.move(tovar);}
PUBLIC void swap(VARREF var1, VARREF var2) {var1.swap(var2);}

// OS

PUBLIC ND var date() {return var().date();}
PUBLIC ND var time() {return var().time();}
PUBLIC ND var ostime() {return var().ostime();}
PUBLIC ND var timestamp() {return var().timestamp();}
//PUBLIC ND var timedate();

PUBLIC void ossleep(const int milliseconds) {var().ossleep(milliseconds);}
PUBLIC ND var oswait(const int milliseconds, SV dirpath) {return var().oswait(milliseconds, dirpath);}

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
PUBLIC ND bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale DEFAULT_EMPTY) {return osfilevar.osopen(osfilepath, locale);}
PUBLIC void osclose(CVR osfilevar) {osfilevar.osclose();}
// Versions where offset is input and output
PUBLIC ND bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length) {return data.osbread(osfilevar, offset, length);}
PUBLIC ND bool osbwrite(CVR data, CVR osfilevar, VARREF offset) {return data.osbwrite(osfilevar, offset);}
// Versions where offset is const offset e.g. numeric ints
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
PUBLIC ND bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length) {return data.osbread(osfilevar, const_cast<VARREF>(offset), length);}
PUBLIC ND bool osbwrite(CVR data, CVR osfilevar, CVR offset) {return data.osbwrite(osfilevar, const_cast<VARREF>(offset));}
#endif

// Read/Write whole osfile
PUBLIC ND bool oswrite(CVR data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.oswrite(osfilepath, codepage);}
PUBLIC ND bool osread(VARREF data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.osread(osfilepath, codepage);}
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
PUBLIC ND var osread(CVR osfilepath) {var data; if (data.osread(osfilepath)) return data; else return "";}

PUBLIC ND bool osremove(CVR ospath) {return ospath.osremove();}
PUBLIC ND bool osrename(CVR old_ospath, CVR new_ospath) {return old_ospath.osrename(new_ospath);}
PUBLIC ND bool oscopy(CVR from_ospath, CVR to_ospath) {return from_ospath.oscopy(to_ospath);}
PUBLIC ND bool osmove(CVR from_ospath, CVR to_ospath) {return from_ospath.osmove(to_ospath);}

PUBLIC ND var oslist(CVR path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode DEFAULT_0) {return path.oslist(globpattern, mode);}
PUBLIC ND var oslistf(CVR filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return filepath.oslistf(globpattern);}
PUBLIC ND var oslistd(CVR dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return dirpath.oslistd(globpattern);}

PUBLIC ND var osinfo(CVR path, const int mode DEFAULT_0) {return path.osinfo(mode);}
PUBLIC ND var osfile(CVR filepath) {return filepath.osfile();}
PUBLIC ND var osdir(CVR dirpath) {return dirpath.osdir();}


PUBLIC ND bool osmkdir(CVR dirpath) {return dirpath.osmkdir();}
PUBLIC ND bool osrmdir(CVR dirpath, const bool evenifnotempty DEFAULT_FALSE) {return dirpath.osrmdir(evenifnotempty);}

PUBLIC ND var oscwd() {return var().oscwd();}
PUBLIC ND var oscwd(CVR dirpath) {return dirpath.oscwd(dirpath);}

PUBLIC void osflush() {return var().osflush();}
PUBLIC ND var ospid() {return var().ospid();}

PUBLIC ND bool osshell(CVR command) {return command.osshell();}
PUBLIC ND bool osshellwrite(CVR writestr, CVR command) {return writestr.osshellwrite(command);}
PUBLIC ND bool osshellread(VARREF readstr, CVR command) {return readstr.osshellread(command);}
PUBLIC ND var osshellread(CVR command) {var result; result.osshellread(command); return result;}

PUBLIC ND var backtrace();

PUBLIC bool setxlocale(const char* locale) {return var(locale).setxlocale();}
PUBLIC ND var getxlocale() {return var().getxlocale();}

// MATH

PUBLIC ND var abs(CVR num1) {return num1.abs();}
PUBLIC ND var pwr(CVR base, CVR exponent) {return base.pwr(exponent);}
PUBLIC ND var exp(CVR power) {return power.exp();}
PUBLIC ND var sqrt(CVR num1) {return num1.sqrt();}
PUBLIC ND var sin(CVR degrees) {return degrees.sin();}
PUBLIC ND var cos(CVR degrees) {return degrees.cos();}
PUBLIC ND var tan(CVR degrees) {return degrees.tan();}
PUBLIC ND var atan(CVR degrees) {return degrees.atan();}
PUBLIC ND var loge(CVR num1) {return num1.loge();}
PUBLIC ND var mod(CVR dividend, CVR divisor) {return dividend.mod(divisor);}
PUBLIC ND var mod(CVR dividend, const double divisor) {return dividend.mod(divisor);}
PUBLIC ND var mod(CVR dividend, const int divisor) {return dividend.mod(divisor);}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
PUBLIC ND var integer(CVR num1) {return num1.integer();}
PUBLIC ND var floor(CVR num1) {return num1.floor();}
PUBLIC ND var round(CVR num1, const int ndecimals DEFAULT_0) {return num1.round(ndecimals);}

PUBLIC ND var rnd(const int number) {return var(number).rnd();}
PUBLIC void initrnd(CVR seed DEFAULT_0) {seed.initrnd();}

// INPUT

PUBLIC ND var getprompt();
PUBLIC void setprompt(CVR prompt);

PUBLIC var input() {var v; v.input(); return v;}

PUBLIC var input(CVR prompt) {var v; v.input(prompt); return v;}

PUBLIC var inputn(const int nchars) {var v; v.inputn(nchars); return v;}

PUBLIC ND bool isterminal() {return var().isterminal();}
PUBLIC ND bool hasinput(const int millisecs DEFAULT_0) {return var().hasinput(millisecs);}
PUBLIC ND bool eof() {return var().eof();}
PUBLIC bool echo(const int on_off) {return var().echo(on_off);}

PUBLIC void breakon();
PUBLIC void breakoff();

// SIMPLE STRINGS

PUBLIC ND var len(CVR var1) {return var1.len();}
PUBLIC ND var textlen(CVR var1) {return var1.textlen();}

PUBLIC ND var convert(CVR instring, SV fromchars, SV tochars) {return instring.convert(fromchars, tochars);}
PUBLIC VARREF converter(VARREF iostring, SV fromchars, SV tochars) {return iostring.converter(fromchars, tochars);}

PUBLIC ND var textconvert(CVR instring, SV fromchars, SV tochars) {return instring.textconvert(fromchars, tochars);}
PUBLIC VARREF textconverter(VARREF iostring, SV fromchars, SV tochars) {return iostring.textconverter(fromchars, tochars);}

PUBLIC ND var replace(CVR instring, SV fromstr, SV tostr) {var newstring = instring; return newstring.replace(fromstr, tostr);}
PUBLIC VARREF replacer(VARREF iostring, SV fromstr, SV tostr) {return iostring.replacer(fromstr, tostr);}

PUBLIC ND var regex_replace(CVR instring, SV regex, SV replacement, SV options DEFAULT_EMPTY) {var newstring = instring; return newstring.regex_replacer(regex, replacement, options);}
PUBLIC VARREF regex_replacer(VARREF iostring, SV regex, SV replacement, SV options DEFAULT_EMPTY) {return iostring.regex_replacer(regex, replacement, options);}

PUBLIC ND var ucase(CVR instring) {return instring.ucase();}
PUBLIC VARREF ucaser(VARREF iostring) {return iostring.ucaser();}

PUBLIC ND var lcase(CVR instring) {return instring.lcase();}
PUBLIC VARREF lcaser(VARREF iostring) {return iostring.lcaser();}

PUBLIC ND var tcase(CVR instring) {return instring.tcase();}
PUBLIC VARREF tcaser(VARREF iostring) {return iostring.tcaser();}

PUBLIC ND var fcase(CVR instring) {return instring.fcase();}
PUBLIC VARREF fcaser(VARREF iostring) { return iostring.fcaser();}

PUBLIC ND var normalize(CVR instring) {return instring.normalize();}
PUBLIC VARREF normalizer(VARREF iostring) {return iostring.normalizer();}

PUBLIC VARREF uniquer(VARREF iostring) {return iostring.uniquer();}
PUBLIC ND var unique(CVR instring) {return instring.unique();}

PUBLIC ND var invert(CVR instring) {return var(instring).invert();}
PUBLIC VARREF inverter(VARREF iostring) {return iostring.inverter();}
PUBLIC ND var invert(var&& instring) {return instring.inverter();}

PUBLIC ND var lower(CVR instring) {return instring.lower();}
PUBLIC VARREF lowerer(VARREF iostring) {return iostring.lowerer();}

PUBLIC ND var raise(CVR instring) {return instring.raise();}
PUBLIC VARREF raiser(VARREF iostring) {return iostring.raiser();}

// PASTER

// 1. paste replace
PUBLIC ND var paste(CVR instring, const int pos1, const int length, CVR str) {return instring.paste(pos1, length, str);}
PUBLIC VARREF paster(VARREF iostring, const int pos1, const int length, CVR str) {return iostring.paster(pos1, length, str);}

//// 2. paste over to end
//PUBLIC VARREF pasterall(VARREF iostring, const int pos1, CVR str);
//PUBLIC ND var pasteall(CVR instring, const int pos1, CVR str);

// 3. paste insert at pos1
PUBLIC ND var paste(CVR instring, const int pos1, CVR str) {return instring.paste(pos1, str);}
PUBLIC VARREF paster(VARREF iostring, const int pos1, CVR str) {return iostring.paster(pos1, str);}

// PREFIX
PUBLIC ND var prefix(CVR instring, CVR str) {return instring.prefix(str);}
PUBLIC VARREF prefixer(VARREF iostring, CVR str) {return iostring.prefixer(str);}

PUBLIC ND var pop(CVR instring) {return instring.pop();}
PUBLIC VARREF popper(VARREF iostring) {return iostring.popper();}


PUBLIC ND var quote(CVR instring) {return instring.quote();}
PUBLIC VARREF quoter(VARREF iostring) {return iostring.quoter();}

PUBLIC ND var squote(CVR instring) {return instring.squote();}
PUBLIC VARREF squoter(VARREF iostring) {return iostring.squoter();}

PUBLIC ND var unquote(CVR instring) {return instring.unquote();}
PUBLIC VARREF unquoter(VARREF iostring) {return iostring.unquoter();}


PUBLIC ND var fieldstore(CVR instring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {return instring.fieldstore(sepchar, fieldno, nfields, replacement);}
PUBLIC VARREF fieldstorer(VARREF iostring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {return iostring.fieldstorer(sepchar, fieldno, nfields, replacement);}


PUBLIC ND var trim(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trim(trimchars);}
PUBLIC ND var trimfirst(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimfirst(trimchars);}
PUBLIC ND var trimlast(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimlast(trimchars);}
PUBLIC ND var trimboth(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimboth(trimchars);}

PUBLIC VARREF trimmer(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmer(trimchars);}
PUBLIC VARREF trimmerfirst(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerfirst(trimchars);}
PUBLIC VARREF trimmerlast(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerlast(trimchars);}
PUBLIC VARREF trimmerboth(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerboth(trimchars);}


PUBLIC ND var chr(const int integer) {return var().chr(integer);}
PUBLIC ND var textchr(const int integer) {return var().textchr(integer);}
PUBLIC ND var match(CVR instring, CVR matchstr, CVR options DEFAULT_EMPTY) {return instring.match(matchstr, options);}
PUBLIC ND var seq(CVR char1) {return char1.seq();}
PUBLIC ND var textseq(CVR char1) {return char1.textseq();}
PUBLIC ND var str(CVR instring, const int number) {return instring.str(number);}
PUBLIC ND var space(const int number) {return var(number).space();}
PUBLIC ND var fcount(CVR instring, SV substr) {return instring.fcount(substr);}
PUBLIC ND var count(CVR instring, SV substr) {return instring.count(substr);}

PUBLIC ND var substr(CVR instring, const int startindex) {return instring.b(startindex);}
PUBLIC ND var substr(CVR instring, const int startindex, const int length) {return instring.b(startindex, length);}
PUBLIC VARREF substrer(VARREF iostring, const int startindex) {return iostring.substrer(startindex);}
PUBLIC VARREF substrer(VARREF iostring, const int startindex, const int length) {return iostring.substrer(startindex, length);}

PUBLIC ND bool starts(CVR instring, SV substr) {return instring.starts(substr);}
PUBLIC ND bool end(CVR instring, SV substr) {return instring.ends(substr);}
PUBLIC ND bool contains(CVR instring, SV substr) {return instring.contains(substr);}

PUBLIC ND var index(CVR instring, SV substr, const int startindex DEFAULT_1) {return instring.index(substr, startindex);}
PUBLIC ND var indexn(CVR instring, SV substr, int occurrence) {return instring.indexn(substr, occurrence);}
PUBLIC ND var indexr(CVR instring, SV substr, const int startindex DEFAULT_M1) {return instring.indexr(substr, startindex);}

PUBLIC ND var field(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field(substr, fieldno, nfields);}
PUBLIC ND var field2(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field2(substr, fieldno, nfields);}

// STRINGS WITH FIELD MARKS

PUBLIC ND var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno) {return fromstr.substr2(startindex, delimiterno);}

PUBLIC ND dim split(CVR sourcevar, SV sepchar DEFAULT__FM) {return sourcevar.split(sepchar);}
PUBLIC ND var join(const dim& sourcedim, SV sepchar DEFAULT__FM) {return sourcedim.join(sepchar);}

PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, subvalueno, replacement);}
PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, replacement);}
PUBLIC ND var pickreplace(CVR instring, const int fieldno, CVR replacement) {return instring.pickreplace(fieldno, replacement);}

PUBLIC ND var extract(CVR instring, const int fieldno DEFAULT_0, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.f(fieldno, valueno, subvalueno);}

PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return instring.insert(fieldno, valueno, subvalueno, insertion);}
PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, CVR insertion) {return instring.insert(fieldno, valueno, insertion);}
PUBLIC ND var insert(CVR instring, const int fieldno, CVR insertion) {return instring.insert(fieldno, insertion);}

PUBLIC ND var remove(CVR instring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.remove(fieldno, valueno, subvalueno);}

PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return iostring.r(fieldno, valueno, subvalueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement) {return iostring.r(fieldno, valueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement) {return iostring.r(fieldno, replacement);}

PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return iostring.inserter(fieldno, valueno, subvalueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion) {return iostring.inserter(fieldno, valueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, CVR insertion) {return iostring.inserter(fieldno, insertion);}

PUBLIC VARREF remover(VARREF iostring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return iostring.remover(fieldno, valueno, subvalueno);}

PUBLIC ND bool locate(CVR target, CVR instring) {return instring.locate(target);}
PUBLIC ND bool locate(CVR target, CVR instring, VARREF setting) {return instring.locate(target, setting);}
PUBLIC ND bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locate(target, setting, fieldno, valueno);}

PUBLIC ND bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
PUBLIC ND bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

PUBLIC ND bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
PUBLIC ND bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

PUBLIC ND bool locateusing(CVR usingchar, CVR target, CVR instring) {return instring.locateusing(usingchar, target);}
PUBLIC ND bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting) {return instring.locateusing(usingchar, target, setting);}
PUBLIC ND bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);}

PUBLIC ND var sum(CVR instring, SV sepchar) {return instring.sum(sepchar);}
PUBLIC ND var sum(CVR instring) {return instring.sum();}

PUBLIC ND var crop(CVR instring) {return instring.crop();}
PUBLIC VARREF cropper(VARREF iostring) {return iostring.cropper();}

PUBLIC ND var sort(CVR instring, SV sepchar DEFAULT__FM) {return instring.sort(sepchar);}
PUBLIC VARREF sorter(VARREF iostring, SV sepchar DEFAULT__FM) {return iostring.sorter(sepchar);}

PUBLIC ND var reverse(CVR instring, SV sepchar DEFAULT__FM) {return instring.reverse(sepchar);}
PUBLIC VARREF reverser(VARREF iostring, SV sepchar DEFAULT__FM) {return iostring.reverser(sepchar);}

PUBLIC ND var parser(CVR instring, char sepchar DEFAULT_CSPACE) {return instring.parse(sepchar);}
PUBLIC VARREF parser(VARREF iostring, char sepchar DEFAULT_CSPACE) {return iostring.parser(sepchar);}

// DATABASE

PUBLIC ND bool connect(CVR connectioninfo DEFAULT_EMPTY) {var conn1; return conn1.connect(connectioninfo);}
PUBLIC void disconnect() {var().disconnect();}
PUBLIC void disconnectall() {var().disconnectall();}

PUBLIC ND bool dbcreate(CVR dbname) {return dbname.dbcreate(dbname);}
PUBLIC ND var dblist() {return var().dblist();}
PUBLIC ND bool dbcopy(CVR from_dbname, CVR to_dbname) {return var().dbcopy(from_dbname, to_dbname);}
PUBLIC ND bool dbdelete(CVR dbname) {return var().dbdelete(dbname);}

PUBLIC ND bool createfile(CVR dbfilename);
PUBLIC ND bool deletefile(CVR dbfilename_or_var);
PUBLIC ND bool clearfile(CVR dbfilename_or_var);
PUBLIC ND bool renamefile(CVR old_dbfilename, CVR new_dbfilename) {return old_dbfilename.renamefile(old_dbfilename.f(1), new_dbfilename);}
PUBLIC ND var listfiles() {return var().listfiles();}

PUBLIC ND var reccount(CVR dbfilename_or_var) {return dbfilename_or_var.reccount();}

PUBLIC ND bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
PUBLIC ND bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
PUBLIC ND var listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) {return var().listindex(dbfilename, fieldname);}

PUBLIC ND bool begintrans() {return var().begintrans();}
PUBLIC ND bool statustrans() {return var().statustrans();}
PUBLIC ND bool rollbacktrans() {return var().rollbacktrans();}
PUBLIC ND bool committrans() {return var().committrans();}
PUBLIC void cleardbcache() {var().cleardbcache();}

PUBLIC ND bool lock(CVR dbfilevar, CVR key) {return (bool)dbfilevar.lock(key);}
PUBLIC void unlock(CVR dbfilevar, CVR key) {dbfilevar.unlock(key);}
PUBLIC void unlockall() {var().unlockall();}

PUBLIC ND bool open(CVR dbfilename, VARREF dbfilevar) {return dbfilevar.open(dbfilename);}
PUBLIC ND bool open(CVR dbfilename) {return var().open(dbfilename);}

PUBLIC ND bool read(VARREF record, CVR dbfilevar, CVR key) {return record.read(dbfilevar, key);}
PUBLIC ND bool reado(VARREF record, CVR dbfilevar, CVR key) {return record.reado(dbfilevar, key);}
PUBLIC ND bool readv(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber) {return record.readv(dbfilevar, key, fieldnumber);}

PUBLIC bool write(CVR record, CVR dbfilevar, CVR key) {return record.write(dbfilevar, key);}
PUBLIC bool writeo(CVR record, CVR dbfilevar, CVR key) {return record.writeo(dbfilevar, key);}
PUBLIC bool writev(CVR record, CVR dbfilevar, CVR key, const int fieldno) {return record.writev(dbfilevar, key, fieldno);}
PUBLIC ND bool updaterecord(CVR record, CVR dbfilevar, CVR key) {return record.updaterecord(dbfilevar, key);}
PUBLIC ND bool insertrecord(CVR record, CVR dbfilevar, CVR key) {return record.insertrecord(dbfilevar, key);}

PUBLIC ND bool dimread(dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.read(dbfilevar, key);}
PUBLIC ND bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.write(dbfilevar, key);}

PUBLIC ND var xlate(CVR dbfilename, CVR key, CVR fieldno, const char* mode);
PUBLIC ND var xlate(CVR dbfilename, CVR key, CVR fieldno, CVR mode);

PUBLIC ND const var lasterror() {return var().lasterror();}
PUBLIC const var loglasterror(CVR source DEFAULT_EMPTY) {return var().loglasterror(source);}



PUBLIC ND bool createfile(CVR dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "").replacer("REVBOOT ", "").replacer("DATAVOL ", "").trimmer();

	return dbfilename2.createfile(dbfilename2);
}

PUBLIC ND bool deletefile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

PUBLIC ND bool clearfile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

PUBLIC ND bool createindex(CVR dbfilename_or_var, CVR fieldname, CVR dictdbfilename) {

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

PUBLIC ND bool deleteindex(CVR dbfilename_or_var, CVR fieldname) {
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
