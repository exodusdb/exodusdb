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

#ifndef EXODUSFUNCS_H
#define EXODUSFUNCS_H 1

#include <exodus/var.h>

#ifndef PUBLIC
#	define PUBLIC
#endif

// add global function type syntax to the exodus users
// SIMILAR code in exofuncs.h and varimpl.h
namespace exodus {

	// clang-format off

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__

	inline const var       EOL      = "\r\n";
#	define                _EOL        "\r\n"

	inline const var       OSSLASH  = "\\";
#	define                _OSSLASH    "\\"
	constexpr char         OSSLASH_ = '\\';
	constexpr bool         OSSLASH_IS_BACKSLASH = true;

#else

	inline const var       EOL      = "\n";
#	define                _EOL        "\n"

	inline const var       OSSLASH  = "/";
#	define                _OSSLASH    "/"
	constexpr char         OSSLASH_ = '/';
	constexpr bool         OSSLASH_IS_BACKSLASH = false;

#endif

#if defined(_WIN64) or defined(_LP64)
	inline const var       PLATFORM = "x64";
#	define                _PLATFORM   "x84"
#else
	inline const var       PLATFORM = "x86";
#	define                _PLATFORM   "x86"
#endif

	// clang-format on

// Removed to reduce compile time of exodus programs.
// Use .output() .errput() and .logput() for threadsafe output.
// print() errput() logput() to output all arguments together in a thread-safe manner
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM

PUBLIC int exodus_main(int exodus__argc, const char* exodus__argv[], ExoEnv& mv, int threadno);

PUBLIC ND var osgetenv(CVR envcode = "") {
    var envvalue = "";
    envvalue.osgetenv(envcode);
    return envvalue;
}
PUBLIC bool osgetenv(CVR code, VARREF value) {return value.osgetenv(code);}
PUBLIC bool ossetenv(CVR code, CVR value) {return value.ossetenv(code);}

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
PUBLIC var oswait(const int milliseconds, SV dirpath) {return var().oswait(milliseconds, dirpath);}

// Read/write osfile at specified offset. Must open/close.
PUBLIC bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale DEFAULT_EMPTY) {return osfilevar.osopen(osfilepath, locale);}
PUBLIC void osclose(CVR osfilevar) {osfilevar.osclose();}
// Versions where offset is input and output
PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length) {return data.osbread(osfilevar, offset, length);}
PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset) {return data.osbwrite(osfilevar, offset);}
// Allow calling with const offset e.g. numeric ints
PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length) {return data.osbread(osfilevar, const_cast<VARREF>(offset), length);}
PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset) {return data.osbwrite(osfilevar, const_cast<VARREF>(offset));}

// Read/Write whole osfile
PUBLIC bool oswrite(CVR data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.oswrite(osfilepath, codepage);}
PUBLIC bool osread(VARREF data, CVR osfilepath, const char* codepage DEFAULT_EMPTY) {return data.osread(osfilepath, codepage);}
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
PUBLIC ND var osread(CVR osfilepath) {
    var data;
    if (data.osread(osfilepath))
        return data;
    else
        return "";
}

PUBLIC bool osremove(CVR ospath) {return ospath.osremove();}
PUBLIC bool osrename(CVR old_ospath, CVR new_ospath) {return old_ospath.osrename(new_ospath);}
PUBLIC bool oscopy(CVR from_ospath, CVR to_ospath) {return from_ospath.oscopy(to_ospath);}
PUBLIC bool osmove(CVR from_ospath, CVR to_ospath) {return from_ospath.osmove(to_ospath);}

PUBLIC ND var oslist(CVR path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode = 0) {return path.oslist(globpattern, mode);}
PUBLIC ND var oslistf(CVR filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return filepath.oslistf(globpattern);}
PUBLIC ND var oslistd(CVR dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return dirpath.oslistd(globpattern);}

PUBLIC ND var osinfo(CVR path, const int mode = 0) {return path.osinfo(mode);}
PUBLIC ND var osfile(CVR filepath) {return filepath.osfile();}
PUBLIC ND var osdir(CVR dirpath) {return dirpath.osdir();}

PUBLIC bool osmkdir(CVR dirpath) {return dirpath.osmkdir();}
PUBLIC bool osrmdir(CVR dirpath, const bool evenifnotempty = false) {return dirpath.osrmdir(evenifnotempty);}

PUBLIC ND var oscwd() {return var().oscwd();}
PUBLIC var oscwd(CVR dirpath) {return dirpath.oscwd(dirpath);}

PUBLIC void osflush() {return var().osflush();}
PUBLIC ND var ospid() {return var().ospid();}

PUBLIC bool osshell(CVR command) {return command.osshell();}
PUBLIC bool osshellwrite(CVR writestr, CVR command) {return writestr.osshellwrite(command);}
PUBLIC bool osshellread(VARREF readstr, CVR command) {return readstr.osshellread(command);}
PUBLIC var osshellread(CVR command) {
    var result;
    result.osshellread(command);
    return result;
}

// Moved to exoprog
//PUBLIC void stop(CVR text DEFAULT_EMPTY);
//PUBLIC void abort(CVR text DEFAULT_EMPTY);	 // dont confuse with abort() which is standard c/c++
//PUBLIC void abortall(CVR text DEFAULT_EMPTY);
//PUBLIC var logoff();

PUBLIC var execute(CVR command);
// PUBLIC var chain(CVR command);

//PUBLIC void debug(CVR DEFAULT_EMPTY);
PUBLIC ND var backtrace();

PUBLIC bool setxlocale(const char* locale) {return var(locale).setxlocale();}
PUBLIC ND var getxlocale() {return var().getxlocale();}

//replace by templates with variable number of arguments
//void print(CVR var2);
//void printl(CVR var2 DEFAULT_EMPTY);
//void printt(CVR var2 DEFAULT_EMPTY);

// MATH

PUBLIC ND var abs(CVR num1) {return num1.abs();}
PUBLIC ND var pwr(CVR base, CVR exponent) {return base.pwr(exponent);}
PUBLIC ND var exp(CVR power) {return power.exp();}
PUBLIC ND var sqrt(CVR num1) {return num1.sqrt();}
PUBLIC ND var sin(CVR degrees) {return degrees.cos();}
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
PUBLIC ND var round(CVR num1, const int ndecimals = 0) {return num1.round(ndecimals);}

PUBLIC ND var rnd(const int number) {return var(number).rnd();}
PUBLIC void initrnd(CVR seed = 0) {seed.initrnd();}

// INPUT

PUBLIC ND var getprompt();
PUBLIC void setprompt(CVR prompt);

PUBLIC var input();
PUBLIC var input(CVR prompt);
PUBLIC var inputn(const int nchars);

PUBLIC ND bool isterminal() {return var().isterminal();}
PUBLIC ND bool hasinput(const int millisecs = 0) {return var().hasinput(millisecs);}
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

PUBLIC ND var replace(CVR instring, SV fromstr, SV tostr);
PUBLIC VARREF replacer(VARREF iostring, SV fromstr, SV tostr) {return iostring.replacer(fromstr, tostr);}

PUBLIC ND var regex_replace(CVR instring, SV regex, SV replacement, SV options DEFAULT_EMPTY);
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
//PUBLIC VARREF inverter(VARREF iostring) {return instring.inverter();};
//PUBLIC ND var invert(CVR instring) {return var(instring).inverter();};
//PUBLIC ND var invert(var&& instring) {return instring.inverter();};
//template<typename T> PUBLIC T invert(var && instring) {return T (std::forward<var>(instring));}

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

PUBLIC bool starts(CVR instring, SV substr) {return instring.starts(substr);}
PUBLIC bool end(CVR instring, SV substr) {return instring.ends(substr);}
PUBLIC bool contains(CVR instring, SV substr) {return instring.contains(substr);}

PUBLIC ND var index(CVR instring, SV substr, const int startindex = 1) {return instring.index(substr, startindex);}
PUBLIC ND var indexn(CVR instring, SV substr, int occurrence) {return instring.indexn(substr, occurrence);}
PUBLIC ND var indexr(CVR instring, SV substr, const int startindex = -1) {return instring.indexr(substr, startindex);}

PUBLIC ND var field(CVR instring, SV substr, const int fieldno, const int nfields = 1) {return instring.field(substr, fieldno, nfields);}
PUBLIC ND var field2(CVR instring, SV substr, const int fieldno, const int nfields = 1) {return instring.field2(substr, fieldno, nfields);}

// STRINGS WITH FIELD MARKS

PUBLIC var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno) {return fromstr.substr2(startindex, delimiterno);}

PUBLIC ND dim split(CVR sourcevar, SV sepchar = _FM) {return sourcevar.split(sepchar);}
PUBLIC ND var join(const dim& sourcedim, SV sepchar = _FM) {return sourcedim.join(sepchar);}

PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, subvalueno, replacement);}
PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement) {return instring.pickreplace(fieldno, valueno, replacement);}
PUBLIC ND var pickreplace(CVR instring, const int fieldno, CVR replacement) {return instring.pickreplace(fieldno, replacement);}

PUBLIC ND var extract(CVR instring, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) {return instring.f(fieldno, valueno, subvalueno);}

PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return instring.insert(fieldno, valueno, subvalueno, insertion);}
PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, CVR insertion) {return instring.insert(fieldno, valueno, insertion);}
PUBLIC ND var insert(CVR instring, const int fieldno, CVR insertion) {return instring.insert(fieldno, insertion);}

// PUBLIC var erase(CVR instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
PUBLIC ND var remove(CVR instring, const int fieldno, const int valueno = 0, const int subvalueno = 0) {return instring.remove(fieldno, valueno, subvalueno);}

PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {return iostring.r(fieldno, valueno, subvalueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement) {return iostring.r(fieldno, valueno, replacement);}
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement) {return iostring.r(fieldno, replacement);}

PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {return iostring.inserter(fieldno, valueno, subvalueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion) {return iostring.inserter(fieldno, valueno, insertion);}
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, CVR insertion) {return iostring.inserter(fieldno, insertion);}

// PUBLIC VARREF eraser(VARREF iostring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
PUBLIC VARREF remover(VARREF iostring, const int fieldno, const int valueno = 0, const int subvalueno = 0) {return iostring.remover(fieldno, valueno, subvalueno);}

PUBLIC ND bool locate(CVR target, CVR instring) {return instring.locate(target);}
PUBLIC bool locate(CVR target, CVR instring, VARREF setting) {return instring.locate(target, setting);}
PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0) {return instring.locate(target, setting, fieldno, valueno);}

PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting) {return instring.locateby(ordercode, target, setting);}
PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring) {return instring.locateusing(usingchar, target);}
PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting) {return instring.locateusing(usingchar, target, setting);}
PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0, const int subvalueno = 0) {return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);}

PUBLIC ND var sum(CVR instring, SV sepchar) {return instring.sum(sepchar);}
PUBLIC ND var sum(CVR instring) {return instring.sum();}

PUBLIC ND var crop(CVR instring) {return instring.crop();}
PUBLIC VARREF cropper(VARREF iostring) {return iostring.cropper();}

PUBLIC ND var sort(CVR instring, SV sepchar = _FM) {return instring.sort(sepchar);}
PUBLIC VARREF sorter(VARREF iostring, SV sepchar = _FM) {return iostring.sorter(sepchar);}

PUBLIC ND var reverse(CVR instring, SV sepchar = _FM) {return instring.reverse(sepchar);}
PUBLIC VARREF reverser(VARREF iostring, SV sepchar = _FM) {return iostring.reverser(sepchar);}

PUBLIC ND var parser(CVR instring, char sepchar = ' ') {return instring.parse(sepchar);}
PUBLIC VARREF parser(VARREF iostring, char sepchar = ' ') {return iostring.parser(sepchar);}

// DATABASE

PUBLIC bool connect(CVR connectionstring DEFAULT_EMPTY);
PUBLIC void disconnect() {var().disconnect();}
PUBLIC void disconnectall() {var().disconnectall();}

PUBLIC ND var lasterror();

PUBLIC bool dbcreate(CVR dbname) {return dbname.dbcreate(dbname);}
PUBLIC ND var dblist() {return var().dblist();}
PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname) {return var().dbcopy(from_dbname, to_dbname);}
PUBLIC bool dbdelete(CVR dbname) {return var().dbdelete(dbname);}

PUBLIC bool createfile(CVR dbfilename);
PUBLIC bool deletefile(CVR dbfilename_or_var);
PUBLIC bool clearfile(CVR dbfilename_or_var);
PUBLIC bool renamefile(CVR old_dbfilename, CVR new_dbfilename) {return old_dbfilename.renamefile(old_dbfilename.f(1), new_dbfilename);}
PUBLIC ND var listfiles() {return var().listfiles();}

PUBLIC ND var reccount(CVR dbfilename_or_var) {return dbfilename_or_var.reccount();}

PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
PUBLIC ND var listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) {return var().listindex(dbfilename, fieldname);}

PUBLIC bool begintrans() {return var().begintrans();}
PUBLIC bool statustrans() {return var().statustrans();}
PUBLIC bool rollbacktrans() {return var().rollbacktrans();}
PUBLIC bool committrans() {return var().committrans();}
PUBLIC void cleardbcache() {var().cleardbcache();}

PUBLIC bool lock(CVR dbfilevar, CVR key) {return (bool)dbfilevar.lock(key);}
PUBLIC void unlock(CVR dbfilevar, CVR key) {dbfilevar.unlock(key);}
PUBLIC void unlockall() {var().unlockall();}

PUBLIC bool open(CVR dbfilename, VARREF dbfilevar) {return dbfilevar.open(dbfilename);}
PUBLIC bool open(CVR dbfilename);
// PUBLIC bool open(CVR dictdata, CVR dbfilename, VARREF dbfilevar);

PUBLIC bool read(VARREF record, CVR dbfilevar, CVR key) {return record.read(dbfilevar, key);}
PUBLIC bool reado(VARREF record, CVR dbfilevar, CVR key) {return record.reado(dbfilevar, key);}
PUBLIC bool readv(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber) {return record.readv(dbfilevar, key, fieldnumber);}

PUBLIC bool write(CVR record, CVR dbfilevar, CVR key) {return record.write(dbfilevar, key);}
PUBLIC bool writeo(CVR record, CVR dbfilevar, CVR key) {return record.writeo(dbfilevar, key);}
PUBLIC bool writev(CVR record, CVR dbfilevar, CVR key, const int fieldno) {return record.writev(dbfilevar, key, fieldno);}
PUBLIC bool updaterecord(CVR record, CVR dbfilevar, CVR key) {return record.updaterecord(dbfilevar, key);}
PUBLIC bool insertrecord(CVR record, CVR dbfilevar, CVR key) {return record.insertrecord(dbfilevar, key);}

PUBLIC bool dimread(dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.read(dbfilevar, key);}
PUBLIC bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key) {return dimrecord.write(dbfilevar, key);}

// moved to exoprog so they have access to default cursor in mv.CURSOR
// PUBLIC bool select(CVR sortselectclause DEFAULT_EMPTY);
// PUBLIC void clearselect();
// PUBLIC bool readnext(VARREF key);
// PUBLIC bool readnext(VARREF key, VARREF valueno);
// PUBLIC bool readnext(VARREF record, VARREF key, VARREF value);
// PUBLIC bool deleterecord(CVR dbfilename_or_var_or_command, CVR key DEFAULT_EMPTY);

PUBLIC ND var xlate(CVR dbfilename, CVR key, CVR fieldno, const char* mode);
PUBLIC ND var xlate(CVR dbfilename, CVR key, CVR fieldno, CVR mode);

////////////////////////////////////////////
//output(args), outputl(args), outputt(args)
////////////////////////////////////////////

// BINARY TRANSPARENT version of print()
// No automatic separator
// Outputs to stdout/cout
// Multi-argument
// Only outputl(args) flushes output
// Calls .output member function instead of direct cout/cerr/clog <<

// output(args...)

template <typename... Printable>
PUBLIC void output(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
}

// outputl(args) appends \n and flushes output

template <typename... Printable>
PUBLIC void outputl(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
	var("").outputl();
}

// outputt(args)  \t separator appends nothing

template <typename... Printable>
PUBLIC void outputt(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).outputt(), ...);
	//var("").outputt();
}

/////////////////////////////////////////////
// printl(args), errputl(args), logputl(args)
/////////////////////////////////////////////

// Multi-argument
// Default sep is " "
// Always appends \n
// Flushes output

//Use like this:
//
// 1. print("hi","ho");                 // "hi ho"
//
// 2. print<','>("hi","ho");            // "hi,ho"
//
// 3. static char const sep[] = ", ";
//    print<sep>("hi","ho");            // "hi, ho"

// printl(args) to cout

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void printl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

// errputl(args) to cerr

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void errputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
	std::cerr << std::endl;
}

// logputl(args) to clog

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void logputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::clog << value;
	((std::clog << sep << values), ...);
	std::clog << std::endl;
}

//////////////////////////////////////////
// print(args), errput(args), logput(args)
//////////////////////////////////////////

// Does *not* append \n
// Multi-argument
// Default sep is " "
// Does *not* flush output

// print(args) to cout

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void print(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
}

// errput(args) to cerr

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void errput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
}

// logput(args) to clog

template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void logput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::clog << value;
	((std::clog << sep << values), ...);
}

/////////////////////////////////
// printl(), errputl(), logputl()
/////////////////////////////////

// Always outputs just \n
// Flushes output

// printl() to cout

PUBLIC void printl() {
	LOCKIOSTREAM
	std::cout << std::endl;
}

// errputl() to cerr

PUBLIC void errputl() {
	LOCKIOSTREAM
	std::cerr << std::endl;
}

// logputl() to clog

PUBLIC void logputl() {
	LOCKIOSTREAM
	std::clog << std::endl;
}

///////////////
// printt(args)
///////////////

// Default sep is tab.
// Multi-argument
// Always adds a sep (tab) on the end.
// Does *not* flush output

// printt(args) to cout

template <auto sep = '\t', typename... Printable>
PUBLIC void printt(const Printable&... values) {
	LOCKIOSTREAM
	((std::cout << values << sep), ...);
}

///////////
// printt()
///////////

// Zero arguments
// Default sep is tab
// Just appends a sep (tab)
// Does *not* flush output

// printt() to cout

template <auto sep = '\t'>
PUBLIC void printt() {
	LOCKIOSTREAM
	std::cout << sep;
}

}  // namespace exodus

#endif	// EXODUSFUNCS_H
