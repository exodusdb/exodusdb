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

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
	inline const var OSSLASH = "\\";
	constexpr char OSSLASH_ = '\\';
	constexpr bool SLASH_IS_BACKSLASH = true;
	constexpr const char* EOL = "\r\n";
#else
	inline const var OSSLASH = "/";
	constexpr char OSSLASH_ = '/';
	constexpr bool SLASH_IS_BACKSLASH = false;
	constexpr const char* EOL = "\n";
#endif

#if defined(_WIN64) or defined(_LP64)
	inline const var PLATFORM_ = "x64";
#else
	inline const var PLATFORM_ = "x86";
#endif

// Removed to reduce compile time of exodus programs.
// Use .output() .errput() and .logput() for threadsafe output.
// print() errput() logput() to output all arguments together in a thread-safe manner
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM

PUBLIC int exodus_main(int exodus__argc, const char* exodus__argv[], ExoEnv& mv, int threadno);

PUBLIC ND var osgetenv(CVR code = "");
PUBLIC bool osgetenv(CVR code, VARREF value);
PUBLIC bool ossetenv(CVR code, CVR value);

PUBLIC ND var ostempdirpath();
PUBLIC ND var ostempfilename();

PUBLIC ND bool assigned(CVR var2);
PUBLIC ND bool unassigned(CVR var2);

PUBLIC void move(VARREF fromvar, VARREF tovar);
PUBLIC void swap(VARREF var1, VARREF var2);

// OS

PUBLIC ND var date();
PUBLIC ND var time();
PUBLIC ND var ostime();
PUBLIC ND var timestamp();
//PUBLIC ND var timedate();

PUBLIC void ossleep(const int milliseconds);
PUBLIC var oswait(const int milliseconds, SV dirpath);

// Read/write osfile at specified offset. Must open/close.
PUBLIC bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale DEFAULT_EMPTY);
PUBLIC void osclose(CVR osfilevar);
// Versions where offset is input and output
PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset);
// Allow calling with const offset e.g. numeric ints
PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset);

// Read/Write whole osfile
PUBLIC bool oswrite(CVR data, CVR osfilepath, const char* codepage DEFAULT_EMPTY);
PUBLIC bool osread(VARREF data, CVR osfilepath, const char* codepage DEFAULT_EMPTY);
// Simple version without codepage returns the contents or "" if file cannot be read
PUBLIC ND var osread(CVR osfilepath);

PUBLIC bool osremove(CVR ospath);
PUBLIC bool osrename(CVR old_ospath, CVR new_ospath);
PUBLIC bool oscopy(CVR from_ospath, CVR to_ospath);
PUBLIC bool osmove(CVR from_ospath, CVR to_ospath);

PUBLIC ND var oslist(CVR path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode = 0);
PUBLIC ND var oslistf(CVR filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY);
PUBLIC ND var oslistd(CVR dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY);

PUBLIC ND var osinfo(CVR path, const int mode = 0);
PUBLIC ND var osfile(CVR filepath);
PUBLIC ND var osdir(CVR dirpath);

PUBLIC bool osmkdir(CVR dirpath);
PUBLIC bool osrmdir(CVR dirpath, const bool evenifnotempty = false);

PUBLIC ND var oscwd();
PUBLIC var oscwd(CVR dirpath);

PUBLIC void osflush();
PUBLIC ND var ospid();

PUBLIC bool osshell(CVR command);
PUBLIC var osshellread(CVR command);
PUBLIC bool osshellread(VARREF readstr, CVR command);
PUBLIC bool osshellwrite(CVR writestr, CVR command);

// Moved to exoprog
//PUBLIC void stop(CVR text DEFAULT_EMPTY);
//PUBLIC void abort(CVR text DEFAULT_EMPTY);	 // dont confuse with abort() which is standard c/c++
//PUBLIC void abortall(CVR text DEFAULT_EMPTY);
//PUBLIC var logoff();

PUBLIC var execute(CVR command);
// PUBLIC var chain(CVR command);

//PUBLIC void debug(CVR DEFAULT_EMPTY);
PUBLIC ND var backtrace();

PUBLIC bool setxlocale(const char* locale);
PUBLIC ND var getxlocale();

//replace by templates with variable number of arguments
//void print(CVR var2);
//void printl(CVR var2 DEFAULT_EMPTY);
//void printt(CVR var2 DEFAULT_EMPTY);

// MATH

PUBLIC ND var abs(CVR num1);
PUBLIC ND var pwr(CVR base, CVR exponent);
PUBLIC ND var exp(CVR power);
PUBLIC ND var sqrt(CVR num1);
PUBLIC ND var sin(CVR degrees);
PUBLIC ND var cos(CVR degrees);
PUBLIC ND var tan(CVR degrees);
PUBLIC ND var atan(CVR degrees);
PUBLIC ND var loge(CVR num1);
PUBLIC ND var mod(CVR dividend, CVR divisor);
PUBLIC ND var mod(CVR dividend, const double divisor);
PUBLIC ND var mod(CVR dividend, const int divisor);

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
PUBLIC ND var integer(CVR num1);
PUBLIC ND var floor(CVR num1);
PUBLIC ND var round(CVR num1, const int ndecimals = 0);

PUBLIC ND var rnd(const int number);
PUBLIC void initrnd(CVR seed = 0);

// INPUT

PUBLIC ND var getprompt();
PUBLIC void setprompt(CVR prompt);

PUBLIC var input();
PUBLIC var input(CVR prompt);
PUBLIC var inputn(const int nchars);

PUBLIC ND bool isterminal();
PUBLIC ND bool hasinput(const int millisecs = 0);
PUBLIC ND bool eof();
PUBLIC bool echo(const int on_off);

PUBLIC void breakon();
PUBLIC void breakoff();

// SIMPLE STRINGS

PUBLIC ND var len(CVR var2);
PUBLIC ND var textlen(CVR var2);

PUBLIC ND var convert(CVR instring, SV fromchars, SV tochars);
PUBLIC VARREF converter(VARREF iostring, SV fromchars, SV tochars);

PUBLIC ND var textconvert(CVR instring, SV fromchars, SV tochars);
PUBLIC VARREF textconverter(VARREF iostring, SV fromchars, SV tochars);

PUBLIC ND var replace(CVR instring, SV fromstr, SV tostr);
PUBLIC VARREF replacer(VARREF iostring, SV fromstr, SV tostr);

PUBLIC ND var regex_replace(CVR instring, SV regex, SV replacement, SV options DEFAULT_EMPTY);
PUBLIC VARREF regex_replacer(VARREF iostring, SV regex, SV replacement, SV options DEFAULT_EMPTY);

PUBLIC ND var ucase(CVR instring);
PUBLIC VARREF ucaser(VARREF iostring);

PUBLIC ND var lcase(CVR instring);
PUBLIC VARREF lcaser(VARREF iostring);

PUBLIC ND var tcase(CVR instring);
PUBLIC VARREF tcaser(VARREF iostring);

PUBLIC ND var fcase(CVR instring);
PUBLIC VARREF fcaser(VARREF iostring);

PUBLIC ND var normalize(CVR instring);
PUBLIC VARREF normalizer(VARREF iostring);

PUBLIC VARREF uniquer(VARREF iostring);
PUBLIC ND var unique(CVR instring);

PUBLIC ND var invert(CVR instring);
PUBLIC VARREF inverter(VARREF iostring);
PUBLIC ND var invert(var&& instring);
//PUBLIC VARREF inverter(VARREF iostring) {return instring.inverter();};
//PUBLIC ND var invert(CVR instring) {return var(instring).inverter();};
//PUBLIC ND var invert(var&& instring) {return instring.inverter();};
//template<typename T> PUBLIC T invert(var && instring) {return T (std::forward<var>(instring));}

PUBLIC ND var lower(CVR instring);
PUBLIC VARREF lowerer(VARREF iostring);

PUBLIC ND var raise(CVR instring);
PUBLIC VARREF raiser(VARREF iostring);

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

PUBLIC ND var pop(CVR instring);
PUBLIC VARREF popper(VARREF iostring);


PUBLIC ND var quote(CVR instring);
PUBLIC VARREF quoter(VARREF iostring);

PUBLIC ND var squote(CVR instring);
PUBLIC VARREF squoter(VARREF iostring);

PUBLIC ND var unquote(CVR instring);
PUBLIC VARREF unquoter(VARREF iostring);


PUBLIC ND var fieldstore(CVR instring, SV sepchar, const int fieldno, const int nfields, CVR replacement);
PUBLIC VARREF fieldstorer(VARREF iostring, SV sepchar, const int fieldno, const int nfields, CVR replacement);


PUBLIC ND var trim(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trim(trimchars);}
PUBLIC ND var trimfirst(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimfirst(trimchars);}
PUBLIC ND var trimlast(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimlast(trimchars);}
PUBLIC ND var trimboth(CVR instring, SV trimchars DEFAULT_SPACE) {return instring.trimboth(trimchars);}

PUBLIC VARREF trimmer(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmer(trimchars);}
PUBLIC VARREF trimmerfirst(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerfirst(trimchars);}
PUBLIC VARREF trimmerlast(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerlast(trimchars);}
PUBLIC VARREF trimmerboth(VARREF iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerboth(trimchars);}


PUBLIC ND var chr(const int integer);
PUBLIC ND var textchr(const int integer);
PUBLIC ND var match(CVR instring, CVR matchstr, CVR options DEFAULT_EMPTY);
PUBLIC ND var seq(CVR char1);
PUBLIC ND var textseq(CVR char1);
PUBLIC ND var str(CVR instring, const int number);
PUBLIC ND var space(const int number);
PUBLIC ND var fcount(CVR instring, SV substr);
PUBLIC ND var count(CVR instring, SV substr);

PUBLIC ND var substr(CVR instring, const int startindex);
PUBLIC ND var substr(CVR instring, const int startindex, const int length);
PUBLIC VARREF substrer(VARREF iostring, const int startindex);
PUBLIC VARREF substrer(VARREF iostring, const int startindex, const int length);

PUBLIC bool starts(CVR instring, SV substr);
PUBLIC bool end(CVR instring, SV substr);
PUBLIC bool contains(CVR instring, SV substr);

PUBLIC ND var index(CVR instring, SV substr, const int startindex = 1);
PUBLIC ND var indexn(CVR instring, SV substr);
PUBLIC ND var indexr(CVR instring, SV substr, const int startindex = -1);

PUBLIC ND var field(CVR instring, SV substr, const int fieldno, const int nfields = 1);
PUBLIC ND var field2(CVR instring, SV substr, const int fieldno, const int nfields = 1);

// STRINGS WITH FIELD MARKS

PUBLIC var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno);

PUBLIC ND dim split(CVR sourcevar, SV sepchar = _FM) {return sourcevar.split(sepchar);}
PUBLIC ND var join(const dim& sourcedim, SV sepchar = _FM) {return sourcedim.join(sepchar);}

PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement);
PUBLIC ND var pickreplace(CVR instring, const int fieldno, CVR replacement);

PUBLIC ND var extract(CVR instring, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0);

PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, CVR insertion);
PUBLIC ND var insert(CVR instring, const int fieldno, CVR insertion);

// PUBLIC var erase(CVR instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
PUBLIC ND var remove(CVR instring, const int fieldno, const int valueno = 0, const int subvalueno = 0);

PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement);
PUBLIC VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement);

PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion);
PUBLIC VARREF inserter(VARREF iostring, const int fieldno, CVR insertion);

// PUBLIC VARREF eraser(VARREF iostring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
PUBLIC VARREF remover(VARREF iostring, const int fieldno, const int valueno = 0, const int subvalueno = 0);

PUBLIC ND bool locate(CVR target, CVR instring);
PUBLIC bool locate(CVR target, CVR instring, VARREF setting);
PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting);
PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting);
PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring);
PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting);
PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0, const int subvalueno = 0);

PUBLIC ND var sum(CVR instring, SV sepchar);
PUBLIC ND var sum(CVR instring);

PUBLIC ND var crop(CVR instring);
PUBLIC VARREF cropper(VARREF iostring);

PUBLIC ND var sort(CVR instring, SV sepchar = _FM) {return instring.sort(sepchar);}
PUBLIC VARREF sorter(VARREF iostring, SV sepchar = _FM) {return iostring.sorter(sepchar);}

PUBLIC ND var reverse(CVR instring, SV sepchar = _FM) {return instring.reverse(sepchar);}
PUBLIC VARREF reverser(VARREF iostring, SV sepchar = _FM) {return iostring.reverser(sepchar);}

// DATABASE

PUBLIC bool connect(CVR connectionstring DEFAULT_EMPTY);
PUBLIC void disconnect();
PUBLIC void disconnectall();

PUBLIC ND var lasterror();

PUBLIC bool dbcreate(CVR dbname);
PUBLIC ND var dblist();
PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname);
PUBLIC bool dbdelete(CVR dbname);

PUBLIC bool createfile(CVR dbfilename);
PUBLIC bool deletefile(CVR dbfilename_or_var);
PUBLIC bool clearfile(CVR dbfilename_or_var);
PUBLIC bool renamefile(CVR old_dbfilename, CVR new_dbfilename);
PUBLIC ND var listfiles();

PUBLIC ND var reccount(CVR dbfilename_or_var);

PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
PUBLIC ND var listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY);

PUBLIC bool begintrans();
PUBLIC bool statustrans();
PUBLIC bool rollbacktrans();
PUBLIC bool committrans();
PUBLIC void cleardbcache();

PUBLIC bool lock(CVR dbfilevar, CVR key);
PUBLIC void unlock(CVR dbfilevar, CVR key);
PUBLIC void unlockall();

PUBLIC bool open(CVR dbfilename, VARREF dbfilevar);
PUBLIC bool open(CVR dbfilename);
// PUBLIC bool open(CVR dictdata, CVR dbfilename, VARREF dbfilevar);

PUBLIC bool read(VARREF record, CVR dbfilevar, CVR key);
PUBLIC bool reado(VARREF record, CVR dbfilevar, CVR key);
PUBLIC bool readv(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber);

PUBLIC bool write(CVR record, CVR dbfilevar, CVR key);
PUBLIC bool writeo(CVR record, CVR dbfilevar, CVR key);
PUBLIC bool writev(CVR record, CVR dbfilevar, CVR key, const int fieldno);
PUBLIC bool updaterecord(CVR record, CVR dbfilevar, CVR key);
PUBLIC bool insertrecord(CVR record, CVR dbfilevar, CVR key);

PUBLIC bool dimread(dim& dimrecord, CVR dbfilevar, CVR key);
PUBLIC bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key);

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

/*#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_RM _FM _VM _SM _TM _ST,VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "==");
*/

}  // namespace exodus

#endif	// EXODUSFUNCS_H
