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
#include <mutex>

// add global function type syntax to the exodus users
// SIMILAR code in exofuncs.h and varimpl.h
namespace exodus {

	// clang-format off

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__

	const var              EOL      = "\r\n";
#	define                _EOL        "\r\n"

	const var              OSSLASH  = "\\";
#	define                _OSSLASH    "\\"
	constexpr char         OSSLASH_ = '\\';
	constexpr bool         OSSLASH_IS_BACKSLASH = true;

#else

	const var              EOL      = "\n";
#	define                _EOL        "\n"

	const var              OSSLASH  = "/";
#	define                _OSSLASH    "/"
	constexpr char         OSSLASH_ = '/';
	constexpr bool         OSSLASH_IS_BACKSLASH = false;

#endif

#if defined(_WIN64) or defined(_LP64)
	const var              PLATFORM = "x64";
#	define                _PLATFORM   "x84"
#else
	const var              PLATFORM = "x86";
#	define                _PLATFORM   "x86"
#endif

// clang-format on

inline std::mutex global_mutex_threadstream;
//
// SLOW = threadsafe. With locking.  Output all arguments together.
//      = print/printl/printt, errput/errputl
#define LOCKIOSTREAM_SLOW std::lock_guard guard(global_mutex_threadstream);
//
// FAST = not threadsafe. No locking. As fast as possible. Intermingled output.
//      = output/outputl/outputt, logput/logputl
#define LOCKIOSTREAM_FAST

#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_CSPACE = ' '
#define DEFAULT__FM = _FM
#define DEFAULT_0 = 0
#define DEFAULT_1  = 1
#define DEFAULT_M1 = -1
#define DEFAULT_FALSE = false

int exodus_main(int exodus__argc, const char* exodus__argv[], ExoEnv& mv, int threadno);

ND var  osgetenv(CVR envcode DEFAULT_EMPTY);
ND bool osgetenv(CVR code, VARREF value);
   void ossetenv(CVR code, CVR value);

ND var ostempdirpath();
ND var ostempfilename();

ND bool assigned(CVR var1);
ND bool unassigned(CVR var1);

   void move(VARREF fromvar, VARREF tovar);
   void swap(VARREF var1, VARREF var2);

// OS

ND var date();
ND var time();
ND var ostime();
ND var timestamp();
ND var timestamp(CVR date, CVR time);
//ND var timedate();

   void ossleep(const int milliseconds);
ND var  oswait(const int milliseconds, SV dirpath);

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
ND bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale DEFAULT_EMPTY);
   void osclose(CVR osfilevar);
// Versions where offset is input and output
ND bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length);
ND bool osbwrite(CVR data, CVR osfilevar, VARREF offset);
// Versions where offset is const offset e.g. numeric ints
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
ND bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length);
ND bool osbwrite(CVR data, CVR osfilevar, CVR offset);
#endif

// Read/Write whole osfile
ND bool oswrite(CVR data, CVR osfilepath, const char* codepage DEFAULT_EMPTY);
ND bool osread(VARREF data, CVR osfilepath, const char* codepage DEFAULT_EMPTY);
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
ND var  osread(CVR osfilepath);

ND bool osremove(CVR ospath);
ND bool osrename(CVR old_ospath, CVR new_ospath);
ND bool oscopy(CVR from_ospath, CVR to_ospath);
ND bool osmove(CVR from_ospath, CVR to_ospath);

ND var  oslist(CVR path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode DEFAULT_0);
ND var  oslistf(CVR filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY);
ND var  oslistd(CVR dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY);

ND var  osinfo(CVR path, const int mode DEFAULT_0);
ND var  osfile(CVR filepath);
ND var  osdir(CVR dirpath);

ND bool osmkdir(CVR dirpath);
ND bool osrmdir(CVR dirpath, const bool evenifnotempty DEFAULT_FALSE);

ND var  oscwd();
ND var  oscwd(CVR dirpath);

   void osflush();
ND var  ospid();

ND bool osshell(CVR command);
ND bool osshellwrite(CVR writestr, CVR command);
ND bool osshellread(VARREF readstr, CVR command);
ND var  osshellread(CVR command);

//var execute(CVR command);

//void debug(CVR DEFAULT_EMPTY);
ND var backtrace();

   bool setxlocale(const char* locale);
ND var  getxlocale();

// MATH

ND var abs(CVR num1);
ND var pwr(CVR base, CVR exponent);
ND var exp(CVR power);
ND var sqrt(CVR num1);
ND var sin(CVR degrees);
ND var cos(CVR degrees);
ND var tan(CVR degrees);
ND var atan(CVR degrees);
ND var loge(CVR num1);
ND var mod(CVR dividend, CVR divisor);
ND var mod(CVR dividend, const double divisor);
ND var mod(CVR dividend, const int divisor);

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
ND var integer(CVR num1);
ND var floor(CVR num1);
ND var round(CVR num1, const int ndecimals DEFAULT_0);

ND var rnd(const int number);
   void initrnd(CVR seed DEFAULT_0);

// INPUT

ND var getprompt();
   void setprompt(CVR prompt);

var input();

var input(CVR prompt);

var inputn(const int nchars);

ND bool isterminal();
ND bool hasinput(const int millisecs DEFAULT_0);
ND bool eof();
   bool echo(const int on_off);

   void breakon();
   void breakoff();

// SIMPLE STRINGS

ND var len(CVR var1);
ND var textlen(CVR var1);

ND var convert(CVR instring, SV fromchars, SV tochars);
VARREF converter(VARREF iostring, SV fromchars, SV tochars);

ND var textconvert(CVR instring, SV fromchars, SV tochars);
VARREF textconverter(VARREF iostring, SV fromchars, SV tochars);

ND var replace(CVR instring, SV fromstr, SV tostr);
VARREF replacer(VARREF iostring, SV fromstr, SV tostr);

ND var regex_replace(CVR instring, SV regex, SV replacement, SV options DEFAULT_EMPTY);
VARREF regex_replacer(VARREF iostring, SV regex, SV replacement, SV options DEFAULT_EMPTY);

ND var ucase(CVR instring);
VARREF ucaser(VARREF iostring);

ND var lcase(CVR instring);
VARREF lcaser(VARREF iostring);

ND var tcase(CVR instring);
VARREF tcaser(VARREF iostring);

ND var fcase(CVR instring);
VARREF fcaser(VARREF iostring);

ND var normalize(CVR instring);
VARREF normalizer(VARREF iostring);

VARREF uniquer(VARREF iostring);
ND var unique(CVR instring);

ND var invert(CVR instring);
VARREF inverter(VARREF iostring);
ND var invert(var&& instring);
//VARREF inverter(VARREF iostring);;
//ND var invert(CVR instring);;
//ND var invert(var&& instring);;
//template<typename T> T invert(var && instring);

ND var lower(CVR instring);
VARREF lowerer(VARREF iostring);

ND var raise(CVR instring);
VARREF raiser(VARREF iostring);

// PASTER

// 1. paste replace
ND var paste(CVR instring, const int pos1, const int length, CVR str);
VARREF paster(VARREF iostring, const int pos1, const int length, CVR str);

//// 2. paste over to end
//VARREF pasterall(VARREF iostring, const int pos1, CVR str);
//ND var pasteall(CVR instring, const int pos1, CVR str);

// 3. paste insert at pos1
ND var paste(CVR instring, const int pos1, CVR str);
VARREF paster(VARREF iostring, const int pos1, CVR str);

// PREFIX
ND var prefix(CVR instring, CVR str);
VARREF prefixer(VARREF iostring, CVR str);

ND var pop(CVR instring);
VARREF popper(VARREF iostring);


ND var quote(CVR instring);
VARREF quoter(VARREF iostring);

ND var squote(CVR instring);
VARREF squoter(VARREF iostring);

ND var unquote(CVR instring);
VARREF unquoter(VARREF iostring);


ND var fieldstore(CVR instring, SV sepchar, const int fieldno, const int nfields, CVR replacement);
VARREF fieldstorer(VARREF iostring, SV sepchar, const int fieldno, const int nfields, CVR replacement);


ND var trim(CVR instring, SV trimchars DEFAULT_SPACE);
ND var trimfirst(CVR instring, SV trimchars DEFAULT_SPACE);
ND var trimlast(CVR instring, SV trimchars DEFAULT_SPACE);
ND var trimboth(CVR instring, SV trimchars DEFAULT_SPACE);

VARREF trimmer(VARREF iostring, SV trimchars DEFAULT_SPACE);
VARREF trimmerfirst(VARREF iostring, SV trimchars DEFAULT_SPACE);
VARREF trimmerlast(VARREF iostring, SV trimchars DEFAULT_SPACE);
VARREF trimmerboth(VARREF iostring, SV trimchars DEFAULT_SPACE);


ND var chr(const int integer);
ND var textchr(const int integer);
ND var match(CVR instring, CVR matchstr, CVR options DEFAULT_EMPTY);
ND var seq(CVR char1);
ND var textseq(CVR char1);
ND var str(CVR instring, const int number);
ND var space(const int number);
ND var fcount(CVR instring, SV substr);
ND var count(CVR instring, SV substr);

ND var substr(CVR instring, const int startindex);
ND var substr(CVR instring, const int startindex, const int length);
VARREF substrer(VARREF iostring, const int startindex);
VARREF substrer(VARREF iostring, const int startindex, const int length);

ND bool starts(CVR instring, SV substr);
ND bool end(CVR instring, SV substr);
ND bool contains(CVR instring, SV substr);

ND var index(CVR instring, SV substr, const int startindex DEFAULT_1);
ND var indexn(CVR instring, SV substr, int occurrence);
ND var indexr(CVR instring, SV substr, const int startindex DEFAULT_M1);

ND var field(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1);
ND var field2(CVR instring, SV substr, const int fieldno, const int nfields DEFAULT_1);

// STRINGS WITH FIELD MARKS

ND var substr2(CVR fromstr, VARREF startindex, VARREF delimiterno);

ND dim split(CVR sourcevar, SV sepchar DEFAULT__FM);
ND var join(const dim& sourcedim, SV sepchar DEFAULT__FM);

ND var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
ND var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement);
ND var pickreplace(CVR instring, const int fieldno, CVR replacement);

ND var extract(CVR instring, const int fieldno DEFAULT_0, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0);

ND var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
ND var insert(CVR instring, const int fieldno, const int valueno, CVR insertion);
ND var insert(CVR instring, const int fieldno, CVR insertion);

// var erase(CVR instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
ND var remove(CVR instring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0);

VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
VARREF pickreplacer(VARREF iostring, const int fieldno, const int valueno, CVR replacement);
VARREF pickreplacer(VARREF iostring, const int fieldno, CVR replacement);

VARREF inserter(VARREF iostring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
VARREF inserter(VARREF iostring, const int fieldno, const int valueno, CVR insertion);
VARREF inserter(VARREF iostring, const int fieldno, CVR insertion);

// VARREF eraser(VARREF iostring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
VARREF remover(VARREF iostring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0);

ND bool locate(CVR target, CVR instring);
ND bool locate(CVR target, CVR instring, VARREF setting);
ND bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0);

ND bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting);
ND bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0);

ND bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting);
ND bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0);

ND bool locateusing(CVR usingchar, CVR target, CVR instring);
ND bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting);
ND bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0);

ND var sum(CVR instring, SV sepchar);
ND var sum(CVR instring);

ND var crop(CVR instring);
VARREF cropper(VARREF iostring);

ND var sort(CVR instring, SV sepchar DEFAULT__FM);
VARREF sorter(VARREF iostring, SV sepchar DEFAULT__FM);

ND var reverse(CVR instring, SV sepchar DEFAULT__FM);
VARREF reverser(VARREF iostring, SV sepchar DEFAULT__FM);

ND var parser(CVR instring, char sepchar DEFAULT_CSPACE);
VARREF parser(VARREF iostring, char sepchar DEFAULT_CSPACE);

// DATABASE

ND bool connect(CVR connectioninfo DEFAULT_EMPTY);
   void disconnect();
   void disconnectall();

ND bool dbcreate(CVR dbname);
ND var  dblist();
ND bool dbcopy(CVR from_dbname, CVR to_dbname);
ND bool dbdelete(CVR dbname);

ND bool createfile(CVR dbfilename);
ND bool deletefile(CVR dbfilename_or_var);
ND bool clearfile(CVR dbfilename_or_var);
ND bool renamefile(CVR old_dbfilename, CVR new_dbfilename);
ND var  listfiles();

ND var  reccount(CVR dbfilename_or_var);

ND bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
ND bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
ND var  listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY);

ND bool begintrans();
ND bool statustrans();
ND bool rollbacktrans();
ND bool committrans();
   void cleardbcache();

ND bool lock(CVR dbfilevar, CVR key);
   void unlock(CVR dbfilevar, CVR key);
   void unlockall();

ND bool open(CVR dbfilename, VARREF dbfilevar);
ND bool open(CVR dbfilename);
// bool open(CVR dictdata, CVR dbfilename, VARREF dbfilevar);

ND bool read(VARREF record, CVR dbfilevar, CVR key);
ND bool readc(VARREF record, CVR dbfilevar, CVR key);
ND bool readf(VARREF record, CVR dbfilevar, CVR key, CVR fieldnumber);

   bool write(CVR record, CVR dbfilevar, CVR key);
   bool writec(CVR record, CVR dbfilevar, CVR key);
   bool writef(CVR record, CVR dbfilevar, CVR key, const int fieldno);
ND bool updaterecord(CVR record, CVR dbfilevar, CVR key);
ND bool insertrecord(CVR record, CVR dbfilevar, CVR key);

ND bool dimread(dim& dimrecord, CVR dbfilevar, CVR key);
ND bool dimwrite(const dim& dimrecord, CVR dbfilevar, CVR key);

// moved to exoprog so they have access to default cursor in mv.CURSOR
// bool select(CVR sortselectclause DEFAULT_EMPTY);
// void clearselect();
// bool readnext(VARREF key);
// bool readnext(VARREF key, VARREF valueno);
// bool readnext(VARREF record, VARREF key, VARREF value);
// bool deleterecord(CVR dbfilename_or_var_or_command, CVR key DEFAULT_EMPTY);

ND var xlate(CVR dbfilename, CVR key, CVR fieldno, const char* mode);
ND var xlate(CVR dbfilename, CVR key, CVR fieldno, CVR mode);

ND var lasterror();
   var loglasterror(CVR source DEFAULT_SPACE);

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
void output(const Printable&... value) {
	LOCKIOSTREAM_FAST
	(var(value).output(), ...);
}

// outputl(args) appends \n and flushes output

template <typename... Printable>
void outputl(const Printable&... value) {
	LOCKIOSTREAM_FAST
	(var(value).output(), ...);
	var("").outputl();
}

// outputt(args)  \t separator appends nothing

template <typename... Printable>
void outputt(const Printable&... value) {
	LOCKIOSTREAM_FAST
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

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void printl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

// errputl(args) to cerr

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void errputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cerr << value;
	((std::cerr << sep << values), ...);
	std::cerr << std::endl;
}

// logputl(args) to clog

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void logputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_FAST
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

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void print(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cout << value;
	((std::cout << sep << values), ...);
}

// errput(args) to cerr

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void errput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cerr << value;
	((std::cerr << sep << values), ...);
}

// logput(args) to clog

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void logput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_FAST
	std::clog << value;
	((std::clog << sep << values), ...);
}

/////////////////////////////////
// printl(), errputl(), logputl()
/////////////////////////////////

// Always outputs just \n
// Flushes output

// printl() to cout

void printl() {
	LOCKIOSTREAM_SLOW
	std::cout << std::endl;
}

// errputl() to cerr

void errputl() {
	LOCKIOSTREAM_SLOW
	std::cerr << std::endl;
}

// logputl() to clog

void logputl() {
	LOCKIOSTREAM_FAST
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
void printt(const Printable&... values) {
	LOCKIOSTREAM_SLOW
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
void printt() {
	LOCKIOSTREAM_SLOW
	std::cout << sep;
}

}  // namespace exodus

#endif	// EXODUSFUNCS_H
