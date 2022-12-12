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

ND PUBLIC var osgetenv(CVR envcode DEFAULT_EMPTY) {var envvalue = ""; if (not envvalue.osgetenv(envcode)) {}; return envvalue;}
ND PUBLIC bool osgetenv(CVR code, VARREF value) {return value.osgetenv(code);}
PUBLIC void ossetenv(CVR code, CVR value) {return value.ossetenv(code);}

ND PUBLIC var ostempdirpath() {return var().ostempdirpath();}
ND PUBLIC var ostempfilename() {return var().ostempfilename();}

ND PUBLIC bool assigned(CVR var1) {return var1.assigned();}
ND PUBLIC bool unassigned(CVR var1) {return !var1.assigned();}

PUBLIC void move(VARREF fromvar, VARREF tovar) {fromvar.move(tovar);}
PUBLIC void swap(VARREF var1, VARREF var2) {var1.swap(var2);}

// OS

ND PUBLIC var date() {return var().date();}
ND PUBLIC var time() {return var().time();}
ND PUBLIC var ostime() {return var().ostime();}
ND PUBLIC var timestamp() {return var().timestamp();}
ND PUBLIC var timestamp(CVR date, CVR time) {return date.timestamp(time);}
//ND PUBLIC var timedate();

PUBLIC void ossleep(const int milliseconds) {var().ossleep(milliseconds);}
ND PUBLIC var oswait(const int milliseconds, SV dirpath) {return var().oswait(milliseconds, dirpath);}

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
ND PUBLIC bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale DEFAULT_EMPTY) {return osfilevar.osopen(osfilepath, locale);}
PUBLIC void osclose(CVR osfilevar) {osfilevar.osclose();}
// Versions where offset is input and output
ND PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length) {return data.osbread(osfilevar, offset, length);}
ND PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset) {return data.osbwrite(osfilevar, offset);}
// Versions where offset is const offset e.g. numeric ints
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
ND PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length) {return data.osbread(osfilevar, const_cast<VARREF>(offset), length);}
ND PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset) {return data.osbwrite(osfilevar, const_cast<VARREF>(offset));}
#endif

// Read/Write whole osfile
ND PUBLIC bool oswrite(CVR data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.oswrite(osfilepath, codepage);}
ND PUBLIC bool osread(VARREF data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.osread(osfilepath, codepage);}
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
ND PUBLIC var osread(CVR osfilepath) {var data; if (data.osread(osfilepath)) return data; else return "";}

ND PUBLIC bool osremove(CVR ospath) {return ospath.osremove();}
ND PUBLIC bool osrename(CVR old_ospath, CVR new_ospath) {return old_ospath.osrename(new_ospath);}
ND PUBLIC bool oscopy(CVR from_ospath, CVR to_ospath) {return from_ospath.oscopy(to_ospath);}
ND PUBLIC bool osmove(CVR from_ospath, CVR to_ospath) {return from_ospath.osmove(to_ospath);}

ND PUBLIC var oslist(CVR path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode DEFAULT_0) {return path.oslist(globpattern, mode);}
ND PUBLIC var oslistf(CVR filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return filepath.oslistf(globpattern);}
ND PUBLIC var oslistd(CVR dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return dirpath.oslistd(globpattern);}

ND PUBLIC var osinfo(CVR path, const int mode DEFAULT_0) {return path.osinfo(mode);}
ND PUBLIC var osfile(CVR filepath) {return filepath.osfile();}
ND PUBLIC var osdir(CVR dirpath) {return dirpath.osdir();}


ND PUBLIC bool osmkdir(CVR dirpath) {return dirpath.osmkdir();}
ND PUBLIC bool osrmdir(CVR dirpath, const bool evenifnotempty DEFAULT_FALSE) {return dirpath.osrmdir(evenifnotempty);}

ND PUBLIC var oscwd() {return var().oscwd();}
ND PUBLIC var oscwd(CVR dirpath) {return dirpath.oscwd(dirpath);}

PUBLIC void osflush() {return var().osflush();}
ND PUBLIC var ospid() {return var().ospid();}
ND PUBLIC var ostid() {return var().ostid();}

ND PUBLIC bool osshell(CVR command) {return command.osshell();}
ND PUBLIC bool osshellwrite(CVR writestr, CVR command) {return writestr.osshellwrite(command);}
ND PUBLIC bool osshellread(VARREF readstr, CVR command) {return readstr.osshellread(command);}
ND PUBLIC var osshellread(CVR command) {var result = ""; if (not result.osshellread(command)) {}; return result;}

ND PUBLIC var backtrace();

PUBLIC bool setxlocale(const char* locale) {return var(locale).setxlocale();}
ND PUBLIC var getxlocale() {return var().getxlocale();}

// MATH

ND PUBLIC var abs(CVR num1) {return num1.abs();}
ND PUBLIC var pwr(CVR base, CVR exponent) {return base.pwr(exponent);}
ND PUBLIC var exp(CVR power) {return power.exp();}
ND PUBLIC var sqrt(CVR num1) {return num1.sqrt();}
ND PUBLIC var sin(CVR degrees) {return degrees.sin();}
ND PUBLIC var cos(CVR degrees) {return degrees.cos();}
ND PUBLIC var tan(CVR degrees) {return degrees.tan();}
ND PUBLIC var atan(CVR degrees) {return degrees.atan();}
ND PUBLIC var loge(CVR num1) {return num1.loge();}
ND PUBLIC var mod(CVR dividend, CVR divisor) {return dividend.mod(divisor);}
ND PUBLIC var mod(CVR dividend, const double divisor) {return dividend.mod(divisor);}
ND PUBLIC var mod(CVR dividend, const int divisor) {return dividend.mod(divisor);}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
ND PUBLIC var integer(CVR num1) {return num1.integer();}
ND PUBLIC var floor(CVR num1) {return num1.floor();}
ND PUBLIC var round(CVR num1, const int ndecimals DEFAULT_0) {return num1.round(ndecimals);}

ND PUBLIC var rnd(const int number) {return var(number).rnd();}
PUBLIC void initrnd(CVR seed DEFAULT_0) {seed.initrnd();}

// INPUT

ND PUBLIC var getprompt();
PUBLIC void setprompt(CVR prompt);

PUBLIC var input() {var v; v.input(); return v;}

PUBLIC var input(CVR prompt) {var v; v.input(prompt); return v;}

PUBLIC var inputn(const int nchars) {var v; v.inputn(nchars); return v;}

ND PUBLIC bool isterminal() {return var().isterminal();}
ND PUBLIC bool hasinput(const int millisecs DEFAULT_0) {return var().hasinput(millisecs);}
ND PUBLIC bool eof() {return var().eof();}
PUBLIC bool echo(const int on_off) {return var().echo(on_off);}

PUBLIC void breakon();
PUBLIC void breakoff();

// SIMPLE STRINGS

ND PUBLIC var len(CVR var1) {return var1.len();}
ND PUBLIC var textlen(CVR var1) {return var1.textlen();}

ND PUBLIC var convert(CVR instring, SV fromchars, SV tochars) {return instring.convert(fromchars, tochars);}
PUBLIC VARREF converter(VARREF iostring, SV fromchars, SV tochars) {return iostring.converter(fromchars, tochars);}

ND PUBLIC var textconvert(CVR instring, SV fromchars, SV tochars) {return instring.textconvert(fromchars, tochars);}
PUBLIC VARREF textconverter(VARREF iostring, SV fromchars, SV tochars) {return iostring.textconverter(fromchars, tochars);}

ND PUBLIC var replace(CVR instring, SV fromstr, SV tostr) {var newstring = instring; return newstring.replace(fromstr, tostr);}
PUBLIC VARREF replacer(VARREF iostring, SV fromstr, SV tostr) {return iostring.replacer(fromstr, tostr);}

ND PUBLIC var regex_replace(CVR instring, SV regex, SV replacement, SV options DEFAULT_EMPTY) {var newstring = instring; return newstring.regex_replacer(regex, replacement, options);}
PUBLIC VARREF regex_replacer(VARREF iostring, SV regex, SV replacement, SV options DEFAULT_EMPTY) {return iostring.regex_replacer(regex, replacement, options);}

ND PUBLIC var ucase(CVR instring) {return instring.ucase();}
PUBLIC VARREF ucaser(VARREF iostring) {return iostring.ucaser();}

ND PUBLIC var lcase(CVR instring) {return instring.lcase();}
PUBLIC VARREF lcaser(VARREF iostring) {return iostring.lcaser();}

ND PUBLIC var tcase(CVR instring) {return instring.tcase();}
PUBLIC VARREF tcaser(VARREF iostring) {return iostring.tcaser();}

ND PUBLIC var fcase(CVR instring) {return instring.fcase();}
PUBLIC VARREF fcaser(VARREF iostring) { return iostring.fcaser();}

ND PUBLIC var normalize(CVR instring) {return instring.normalize();}
PUBLIC VARREF normalizer(VARREF iostring) {return iostring.normalizer();}

PUBLIC VARREF uniquer(VARREF iostring) {return iostring.uniquer();}
ND PUBLIC var unique(CVR instring) {return instring.unique();}

ND PUBLIC var invert(CVR instring) {return var(instring).invert();}
PUBLIC VARREF inverter(VARREF iostring) {return iostring.inverter();}
ND PUBLIC var invert(var&& instring) {return instring.inverter();}

ND PUBLIC var lower(CVR instring) {return instring.lower();}
PUBLIC VARREF lowerer(VARREF iostring) {return iostring.lowerer();}

ND PUBLIC var raise(CVR instring) {return instring.raise();}
PUBLIC VARREF raiser(VARREF iostring) {return iostring.raiser();}

// PASTER

// 1. paste replace
ND PUBLIC var paste(CVR instring, const int pos1, const int length, CVR str) {return instring.paste(pos1, length, str);}
PUBLIC VARREF paster(VARREF iostring, const int pos1, const int length, CVR str) {return iostring.paster(pos1, length, str);}

//// 2. paste over to end
//PUBLIC VARREF pasterall(VARREF iostring, const int pos1, CVR str);
//ND PUBLIC var pasteall(CVR instring, const int pos1, CVR str);

// 3. paste insert at pos1
ND PUBLIC var paste(CVR instring, const int pos1, CVR str) {return instring.paste(pos1, str);}
PUBLIC VARREF paster(VARREF iostring, const int pos1, CVR str) {return iostring.paster(pos1, str);}

// PREFIX
ND PUBLIC var prefix(CVR instring, CVR str) {return instring.prefix(str);}
PUBLIC VARREF prefixer(VARREF iostring, CVR str) {return iostring.prefixer(str);}

ND PUBLIC var pop(CVR instring) {return instring.pop();}
PUBLIC VARREF popper(VARREF iostring) {return iostring.popper();}


ND PUBLIC var quote(CVR instring) {return instring.quote();}
PUBLIC VARREF quoter(VARREF iostring) {return iostring.quoter();}

ND PUBLIC var squote(CVR instring) {return instring.squote();}
PUBLIC VARREF squoter(VARREF iostring) {return iostring.squoter();}

ND PUBLIC var unquote(CVR instring) {return instring.unquote();}
PUBLIC VARREF unquoter(VARREF iostring) {return iostring.unquoter();}


ND PUBLIC var fieldstore(CVR instring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {return instring.fieldstore(sepchar, fieldno, nfields, replacement);}
PUBLIC VARREF fieldstorer(VARREF iostring, SV sepchar, const int fieldno, const int nfields, CVR replacement) {return iostring.fieldstorer(sepchar, fieldno, nfields, replacement);}


ND PUBLIC var trim(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trim(trimchars);}
ND PUBLIC var trimfirst(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimfirst(trimchars);}
ND PUBLIC var trimlast(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimlast(trimchars);}
ND PUBLIC var trimboth(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimboth(trimchars);}

PUBLIC VARREF trimmer(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmer(trimchars);}
PUBLIC VARREF trimmerfirst(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerfirst(trimchars);}
PUBLIC VARREF trimmerlast(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerlast(trimchars);}
PUBLIC VARREF trimmerboth(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerboth(trimchars);}


ND PUBLIC var chr(const int integer) {return var().chr(integer);}
ND PUBLIC var textchr(const int integer) {return var().textchr(integer);}
ND PUBLIC var match(CVR instring, CVR matchstr, CVR options DEFAULT_EMPTY) {return instring.match(matchstr, options);}
ND PUBLIC var seq(CVR char1) {return char1.seq();}
ND PUBLIC var textseq(CVR char1) {return char1.textseq();}
ND PUBLIC var str(CVR instring, const int number) {return instring.str(number);}
ND PUBLIC var space(const int number) {return var(number).space();}
ND PUBLIC var fcount(CVR instring, SV substr) {return instring.fcount(substr);}
ND PUBLIC var count(CVR instring, SV substr) {return instring.count(substr);}

ND PUBLIC var substr(CVR instring, const int startindex) {return instring.b(startindex);}
ND PUBLIC var substr(CVR instring, const int startindex, const int length) {return instring.b(startindex, length);}
PUBLIC VARREF substrer(VARREF iostring, const int startindex) {return iostring.substrer(startindex);}
PUBLIC VARREF substrer(VARREF iostring, const int startindex, const int length) {return iostring.substrer(startindex, length);}

ND PUBLIC bool starts(CVR instring, SV substr) {return instring.starts(substr);}
ND PUBLIC bool end(CVR instring, SV substr) {return instring.ends(substr);}
ND PUBLIC bool contains(CVR instring, SV substr) {return instring.contains(substr);}

ND PUBLIC var index(CVR instring, SV substr, const int startindex DEFAULT_1) {return instring.index(substr, startindex);}
ND PUBLIC var indexn(CVR instring, SV substr, int occurrence) {return instring.indexn(substr, occurrence);}
ND PUBLIC var indexr(CVR instring, SV substr, const int startindex DEFAULT_M1) {return instring.indexr(substr, startindex);}

ND PUBLIC var field(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field(substr, fieldno, nfields);}
ND PUBLIC var field2(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field2(substr, fieldno, nfields);}

// STRINGS WITH FIELD MARKS

ND PUBLIC var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno) {return fromstr.substr2(startindex, delimiterno);}

ND PUBLIC dim split(CVR sourcevar, SV sepchar DEFAULT__FM) {return sourcevar.split(sepchar);}
ND PUBLIC var join(const dim& sourcedim, SV sepchar DEFAULT__FM) {return sourcedim.join(sepchar);}

ND PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, subvalueno, replacement);}
ND PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, replacement);}
ND PUBLIC var pickreplace(CVR instring, const int fieldno, CVR replacement) {return instring.pickreplace(fieldno, replacement);}

ND PUBLIC var extract(CVR instring, const int fieldno DEFAULT_0, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.f(fieldno, valueno, subvalueno);}

ND PUBLIC var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return instring.insert(fieldno, valueno, subvalueno, insertion);}
ND PUBLIC var insert(CVR instring, const int fieldno, const int valueno, CVR insertion) {return instring.insert(fieldno, valueno, insertion);}
ND PUBLIC var insert(CVR instring, const int fieldno, CVR insertion) {return instring.insert(fieldno, insertion);}

ND PUBLIC var remove(CVR instring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.remove(fieldno, valueno, subvalueno);}

PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return iostring.r(fieldno, valueno, subvalueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement) {return iostring.r(fieldno, valueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement) {return iostring.r(fieldno, replacement);}

PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return iostring.inserter(fieldno, valueno, subvalueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion) {return iostring.inserter(fieldno, valueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, CVR insertion) {return iostring.inserter(fieldno, insertion);}

PUBLIC VARREF remover(VARREF iostring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return iostring.remover(fieldno, valueno, subvalueno);}

ND PUBLIC bool locate(CVR target, CVR instring) {return instring.locate(target);}
ND PUBLIC bool locate(CVR target, CVR instring, VARREF setting) {return instring.locate(target, setting);}
ND PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locate(target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring) {return instring.locateusing(usingchar, target);}
ND PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting) {return instring.locateusing(usingchar, target, setting);}
ND PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);}

ND PUBLIC var sum(CVR instring, SV sepchar) {return instring.sum(sepchar);}
ND PUBLIC var sum(CVR instring) {return instring.sum();}

ND PUBLIC var crop(CVR instring) {return instring.crop();}
PUBLIC VARREF cropper(VARREF iostring) {return iostring.cropper();}

ND PUBLIC var sort(CVR instring, SV sepchar DEFAULT__FM) {return instring.sort(sepchar);}
PUBLIC VARREF sorter(VARREF iostring, SV sepchar DEFAULT__FM) {return iostring.sorter(sepchar);}

ND PUBLIC var reverse(CVR instring, SV sepchar DEFAULT__FM) {return instring.reverse(sepchar);}
PUBLIC VARREF reverser(VARREF iostring, SV sepchar DEFAULT__FM) {return iostring.reverser(sepchar);}

ND PUBLIC var parser(CVR instring, char sepchar DEFAULT_CSPACE) {return instring.parse(sepchar);}
PUBLIC VARREF parser(VARREF iostring, char sepchar DEFAULT_CSPACE) {return iostring.parser(sepchar);}

// DATABASE

ND PUBLIC bool connect(CVR connectioninfo DEFAULT_EMPTY) {var conn1; return conn1.connect(connectioninfo);}
PUBLIC void disconnect() {var().disconnect();}
PUBLIC void disconnectall() {var().disconnectall();}

ND PUBLIC bool dbcreate(CVR dbname) {return dbname.dbcreate(dbname);}
ND PUBLIC var dblist() {return var().dblist();}
ND PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname) {return var().dbcopy(from_dbname, to_dbname);}
ND PUBLIC bool dbdelete(CVR dbname) {return var().dbdelete(dbname);}

ND PUBLIC bool createfile(CVR dbfilename);
ND PUBLIC bool deletefile(CVR dbfilename_or_var);
ND PUBLIC bool clearfile(CVR dbfilename_or_var);
ND PUBLIC bool renamefile(CVR old_dbfilename, CVR new_dbfilename) {return old_dbfilename.renamefile(old_dbfilename.f(1), new_dbfilename);}
ND PUBLIC var listfiles() {return var().listfiles();}

ND PUBLIC var reccount(CVR dbfilename_or_var) {return dbfilename_or_var.reccount();}

ND PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
ND PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
ND PUBLIC var listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) {return var().listindex(dbfilename, fieldname);}

ND PUBLIC bool begintrans() {return var().begintrans();}
ND PUBLIC bool statustrans() {return var().statustrans();}
ND PUBLIC bool rollbacktrans() {return var().rollbacktrans();}
ND PUBLIC bool committrans() {return var().committrans();}
PUBLIC void cleardbcache() {var().cleardbcache();}

ND PUBLIC bool lock(CVR dbfilevar, CVR key) {return (bool)dbfilevar.lock(key);}
PUBLIC void unlock(CVR dbfilevar, CVR key) {dbfilevar.unlock(key);}
PUBLIC void unlockall() {var().unlockall();}

ND PUBLIC bool open(CVR dbfilename, VARREF dbfilevar) {return dbfilevar.open(dbfilename);}
ND PUBLIC bool open(CVR dbfilename) {return var().open(dbfilename);}

ND PUBLIC bool read(VARREF record, CVR dbfilevar, CVR key) {return record.read(dbfilevar, key);}
ND PUBLIC bool readc(VARREF record, CVR dbfilevar, CVR key) {return record.readc(dbfilevar, key);}
ND PUBLIC bool readf(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber) {return record.readf(dbfilevar, key, fieldnumber);}

   PUBLIC bool write(CVR record, CVR dbfilevar, CVR key) {return record.write(dbfilevar, key);}
   PUBLIC bool writec(CVR record, CVR dbfilevar, CVR key) {return record.writec(dbfilevar, key);}
   PUBLIC bool writef(CVR record, CVR dbfilevar, CVR key, const int fieldno) {return record.writef(dbfilevar, key, fieldno);}
ND PUBLIC bool updaterecord(CVR record, CVR dbfilevar, CVR key) {return record.updaterecord(dbfilevar, key);}
ND PUBLIC bool insertrecord(CVR record, CVR dbfilevar, CVR key) {return record.insertrecord(dbfilevar, key);}

ND PUBLIC bool dimread(dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.read(dbfilevar, key);}
ND PUBLIC bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.write(dbfilevar, key);}

ND PUBLIC var xlate(CVR dbfilename, CVR key, CVR fieldno, const char* mode);
ND PUBLIC var xlate(CVR dbfilename, CVR key, CVR fieldno, CVR mode);

ND PUBLIC var lasterror() {return var().lasterror();}
   PUBLIC var loglasterror(CVR source DEFAULT_EMPTY) {return var().loglasterror(source);}



ND PUBLIC bool createfile(CVR dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "").replacer("REVBOOT ", "").replacer("DATAVOL ", "").trimmer();

	return dbfilename2.createfile(dbfilename2);
}

ND PUBLIC bool deletefile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

ND PUBLIC bool clearfile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

ND PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname, CVR dictdbfilename) {

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

ND PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname) {
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

	// Done in exoenv contructors now
	// mv.EXECPATH = getexecpath();

	// No longer done here; to allow EXECPATH to be const
	//if (not mv.EXECPATH) {
	//	if (exodus__argc)
	//		mv.EXECPATH = var(exodus__argv[0]);
	//	if (not mv.EXECPATH.contains(OSSLASH))
	//		mv.EXECPATH.prefixer(oscwd() ^ OSSLASH);
	//}

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
