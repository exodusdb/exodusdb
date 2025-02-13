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

#if EXO_MODULE
	import std;
	import var;
#else
#	include <mutex>
#endif

//#include <exodus/var.h>
#include <exodus/vardefs.h>
#include <exodus/timebank.h>

#include <exodus/exoimpl.h>

// clang-format off

//#include <exodus/format.h>

// add global function type syntax to the exodus users
// SIMILAR code in exofuncs.h and varimpl.h
namespace exo {
//
//
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wexit-time-destructors"
//#pragma clang diagnostic ignored "-Wglobal-constructors"
//#pragma clang diagnostic ignored "-Wreserved-identifier"
//
//#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
//
//#	define                         _EOL        "\r\n"
////	extern const var         EOL      = _EOL;
//
//#	define                         _OSSLASH    "\\"
////	extern const var         OSSLASH  = _OSSLASH;
//
////	CONSTINIT_OR_CONSTEXPR auto     OSSLASH_ = '\\';
//
////	CONSTINIT_OR_CONSTEXPR auto     OSSLASH_IS_BACKSLASH = true;
//
//#else
//
//#	define                          _EOL       "\n"
////	extern const var          EOL     = _EOL;
//
//#	define                         _OSSLASH    "/"
////	extern const var         OSSLASH  = _OSSLASH;
//
////	CONSTINIT_OR_CONSTEXPR auto     OSSLASH_ = '/';
////	CONSTINIT_OR_CONSTEXPR auto     OSSLASH_IS_BACKSLASH = false;
//
//#endif
//
//	// _cplusplus is in format YYYYMM e.g. 202002, 202102, 202302 etc.
//	// We will extract the two digit year only - using integer division and integer remainder ops.
//	// Years e.g. 21 which are in between the actual standards like c++20, c++23, c++26 etc.
//	// indicate partial informal support for some features of the next standard
//	// 20 c++20
//	// 21 some c++23
//	// 23 c++23
//	// 24 some c++26
////	CONSTINIT_OR_CONSTEXPR auto _CPP_STANDARD=__cplusplus / 100 % 1000;
//
//#if defined(_WIN64) or defined(_LP64)
//#	define                    _PLATFORM   "x64"
//#else
//#	define                    _PLATFORM   "x86"
//#endif
//	extern const var    PLATFORM = _PLATFORM;
//
//#ifdef __clang__
//	//__clang_major__
//	//__clang_minor__
//	//__clang_patchlevel__
//	//__clang_version__
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER =  "clang";
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION =  __clang_major__;
//#elif defined(__GNUC__)
//	//__GNUC__
//	//__GNUC_MINOR__
//	//__GNUC_PATCHLEVEL__
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER =  "gcc";
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION =  __GNUC__;
//#else
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER =  "unknown";
//	inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION = 0;
//#endif
//
//#pragma clang diagnostic pop
//
//// clang-format on

#ifndef EXO_FUNCS_CPP
	extern
#endif
PUBLIC std::mutex global_mutex_threadstream;

//
// SLOW = threadsafe. With locking.  Output all arguments together.
//      = printx/printl/printt, errput/errputl
//#define LOCKIOSTREAM_YES std::lock_guard guard(global_mutex_threadstream);
//
// FAST = not threadsafe. No locking. As fast as possible. Intermingled output.
//      = output/outputl/outputt, logput/logputl
//#define LOCKIOSTREAM_NO

int exodus_main(int exodus_argc, const char* exodus_argv[], ExoEnv& mv, int threadno);

ND var  hash(in instring, const std::uint64_t modulus = 0);

ND var  osgetenv(SV envcode = "" );
ND bool osgetenv(SV code, io value);
   void ossetenv(SV code, in value);

ND var ostempdirpath(void);
ND var ostempfilename(void);

ND bool assigned(in var1);
ND bool unassigned(in var1);

   void move(io fromvar, io tovar);
   void swap(io var1, io var2);

// OS

ND var date(void);
ND var time(void);
ND var ostime(void);
ND var timestamp(void);
ND var timestamp(in date, in time);
//ND var timedate(void);

   void ossleep(const int milliseconds);
ND var  oswait(SV file_dir_list, const int milliseconds);

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
ND bool osopen(in osfilepath, out osfilevar, const bool utf8 = true );
   void osclose(in osfilevar);
// Versions where offset is input and output
ND bool osbread(io data, in osfilevar, io offset, const int length);
ND bool osbwrite(in data, in osfilevar, io offset);
// Versions where offset is const offset e.g. numeric ints
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
ND bool osbread(io data, in osfilevar, in offset, const int length);
ND bool osbwrite(in data, in osfilevar, in offset);
#endif

// Read/Write whole osfile
ND bool oswrite(in data, in osfilepath, const char* codepage = "" );
ND bool osread(io data, in osfilepath, const char* codepage = "" );
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
ND var  osread(in osfilepath);

ND bool osremove(in ospath);
ND bool osrename(in old_ospath, in new_ospath);
ND bool oscopy(in from_ospath, in to_ospath);
ND bool osmove(in from_ospath, in to_ospath);

ND var  oslist(in path = "." , SV globpattern = "" , const int mode = 0 );
ND var  oslistf(in filepath = "." , SV globpattern = "" );
ND var  oslistd(in dirpath = "." , SV globpattern = "" );

ND var  osinfo(in path, const int mode = 0 );
ND var  osfile(in filepath);
ND var  osdir(in dirpath);

ND bool osmkdir(in dirpath);
ND bool osrmdir(in dirpath, const bool evenifnotempty = false );

ND var  oscwd(void);
ND var  oscwd(in dirpath);

   void osflush(void);
ND var  ospid(void);
ND var  ostid(void);

ND bool osshell(in command);
ND bool osshellwrite(in writestr, in command);
ND bool osshellread(io readstr, in command);
ND var  osshellread(in command);

//var execute(in command);

//void debug(in = "" );
ND var  backtrace(void);

   bool setxlocale(SV locale);
ND var  getxlocale(void);

ND var  from_codepage(in instring, const char* codepage);
ND var  to_codepage(in instring, const char* codepage);

// MATH

ND bool isnum(in num1);
ND var  num(in num1);

ND var  abs(in num1);
ND var  pwr(in base, in exponent);
ND var  exp(in power);
ND var  sqrt(in num1);
ND var  sin(in degrees);
ND var  cos(in degrees);
ND var  tan(in degrees);
ND var  atan(in degrees);
ND var  loge(in num1);
ND var  mod(in dividend, in divisor);
ND var  mod(in dividend, const double divisor);
ND var  mod(in dividend, const int divisor);

// integer(void) represents pick int(void) because int(void) is reserved word in c/c++
// Note that integer like pick int(void) is the same as floor(void)
// whereas the usual c/c++ int(void) simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt(void) (although toInt(void) returns an int
// instead of a var like normal exodus functions)
ND var  integer(in num1);
ND var  floor(in num1);
ND var  round(in num1, const int ndecimals = 0 );

ND var  rnd(const int number);
   void initrnd(in seed = 0 );

// INPUT

ND var  getprompt(void);
   void setprompt(in prompt);

   var  input(void);

   var  input(in prompt);

   var  inputn(const int nchars);

ND bool isterminal(void);
ND bool hasinput(const int millisecs = 0 );
ND bool eof(void);
   bool echo(const int on_off);

   void breakon(void);
   void breakoff(void);

// SIMPLE STRINGS

ND var len(in var1);
ND var textlen(in var1);
ND var textwidth(in var1);

ND var  convert(in instring, SV fromchars, SV tochars);
   IO   converter(io iostring, SV fromchars, SV tochars);

ND var  textconvert(in instring, SV fromchars, SV tochars);
   IO   textconverter(io iostring, SV fromchars, SV tochars);

ND var  replace(in instring, SV fromstr, SV tostr);
   IO   replacer(io iostring, SV fromstr, SV tostr);

ND var  replace(in instring, const rex& regex, SV replacement);
   IO   replacer(io iostring, const rex& regex, SV replacement);

ND var  ucase(in instring);
   IO   ucaser(io iostring);

ND var  lcase(in instring);
   IO   lcaser(io iostring);

ND var  tcase(in instring);
   IO   tcaser(io iostring);

ND var  fcase(in instring);
   IO   fcaser(io iostring);

ND var  normalize(in instring);
   IO   normalizer(io iostring);

   IO   uniquer(io iostring);
ND var  unique(in instring);

ND var  invert(in instring);
   IO   inverter(io iostring);
ND var  invert(var&& instring);
//IO inverter(io iostring);;
//ND var invert(in instring);;
//ND var invert(var&& instring);;
//template<typename T> T invert(var && instring);

ND var  lower(in instring);
   IO   lowerer(io iostring);

ND var  raise(in instring);
   IO   raiser(io iostring);

// CUT

ND var  cut(in instring, const int length);
   IO   cutter(io instring, const int length);

// PASTER

// 1. paste replace
ND var  paste(in instring, const int pos1, const int length, in str);
   IO   paster(io iostring, const int pos1, const int length, in str);

//// 2. paste over to end
//IO pasterall(io iostring, const int pos1, in str);
//ND var pasteall(in instring, const int pos1, in str);

// 3. paste insert at pos1
ND var  paste(in instring, const int pos1, in str);
   IO   paster(io iostring, const int pos1, in str);

// PREFIX
ND var  prefix(in instring, in str);
   IO   prefixer(io iostring, in str);

template<typename... ARGS>
ND var  append(var instring, const ARGS&... appendable) {
	instring.appender(appendable...);
	return instring;
}

template<typename... ARGS>
   IO   appender(io iostring, const ARGS&... appendable) {
	(iostring ^= ... ^= appendable);
	//return void;
}

ND var  pop(in instring);
   IO   popper(io iostring);


ND var  quote(in instring);
   IO   quoter(io iostring);

ND var  squote(in instring);
   IO   squoter(io iostring);

ND var  unquote(in instring);
   IO   unquoter(io iostring);


ND var  fieldstore(in instring, SV sepchar, const int fieldno, const int nfields, in replacement);
   IO   fieldstorer(io iostring, SV sepchar, const int fieldno, const int nfields, in replacement);


ND var  trim(in instring, SV trimchars = " ");
ND var  trimfirst(in instring, SV trimchars = " ");
ND var  trimlast(in instring, SV trimchars = " ");
ND var  trimboth(in instring, SV trimchars = " ");

   IO   trimmer(io iostring, SV trimchars = " ");
   IO   trimmerfirst(io iostring, SV trimchars = " ");
   IO   trimmerlast(io iostring, SV trimchars = " ");
   IO   trimmerboth(io iostring, SV trimchars = " ");

ND var  first(in instring);
ND var  last(in instring);
ND var  first(in instring, const int nbytes);
ND var  last(in instring, const int nbytes);

ND var  chr(const int integer);
ND var  textchr(const int integer);

// Match
ND var  match(in instring, SV regex_str, SV options = "" );
ND var  match(in instring, const rex& regex);

// Search
ND var  search(in instring, SV regex_str, io startchar1, SV regex_options = "");
// Ditto starting from first char
ND var  search(in instring, SV regex_str);
// Ditto given a rex
ND var  search(in instring, const rex& regex, io startchar1);
// Ditto starting from first char.
ND var  search(in instring, const rex& regex);

ND var  seq(in char1);
ND var  textseq(in char1);
ND var  str(in instring, const int number);
ND var  space(const int number);
ND var  fcount(in instring, SV substr);
ND var  count(in instring, SV substr);

ND var  substr(in instring, const int startindex);
ND var  substr(in instring, const int startindex, const int length);
   IO   substrer(io iostring, const int startindex);
   IO   substrer(io iostring, const int startindex, const int length);

ND var  substr(in instring, const int startindex, SV delimiterchars, io pos2);
//IO substrer(io iostring, const int startindex, SV delimiterchars, io pos2);

ND bool starts(in instring, SV substr);
ND bool ends(in instring, SV substr);
ND bool contains(in instring, SV substr);

ND var  index(in instring, SV substr, const int startindex = 1 );
ND var  indexn(in instring, SV substr, int occurrence);
ND var  indexr(in instring, SV substr, const int startindex = -1 );

ND var  field(in instring, SV substr, const int fieldno, const int nfields = 1 );
ND var  field2(in instring, SV substr, const int fieldno, const int nfields = 1 );

// STRINGS WITH FIELD MARKS

ND var  substr2(in fromstr, io startindex, io delimiterno);

ND dim  split(in sourcevar, SV sepchar = _FM );
ND var  join(const dim& sourcedim, SV sepchar = _FM );

ND var  pickreplace(in instring, const int fieldno, const int valueno, const int subvalueno, in replacement);
ND var  pickreplace(in instring, const int fieldno, const int valueno, in replacement);
ND var  pickreplace(in instring, const int fieldno, in replacement);

ND var  extract(in instring, const int fieldno = 0 , const int valueno = 0 , const int subvalueno = 0 );

ND var  insert(in instring, const int fieldno, const int valueno, const int subvalueno, in insertion);
ND var  insert(in instring, const int fieldno, const int valueno, in insertion);
ND var  insert(in instring, const int fieldno, in insertion);

// var erase(in instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
ND var  remove(in instring, const int fieldno, const int valueno = 0 , const int subvalueno = 0 );

   IO   pickreplacer(io iostring, const int fieldno, const int valueno, const int subvalueno, in replacement);
   IO   pickreplacer(io iostring, const int fieldno, const int valueno, in replacement);
   IO   pickreplacer(io iostring, const int fieldno, in replacement);

   IO   inserter(io iostring, const int fieldno, const int valueno, const int subvalueno, in insertion);
   IO   inserter(io iostring, const int fieldno, const int valueno, in insertion);
   IO   inserter(io iostring, const int fieldno, in insertion);

// IO eraser(io iostring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
   IO   remover(io iostring, const int fieldno, const int valueno = 0 , const int subvalueno = 0 );

ND var  locate(in target, in instring);
ND bool locate(in target, in instring, io setting);
ND bool locate(in target, in instring, io setting, const int fieldno, const int valueno = 0 );

ND bool locateby(const char* ordercode, in target, in instring, io setting);
ND bool locateby(const char* ordercode, in target, in instring, io setting, const int fieldno, const int valueno = 0 );

ND bool locateby(in ordercode, in target, in instring, io setting);
ND bool locateby(in ordercode, in target, in instring, io setting, const int fieldno, const int valueno = 0 );

ND bool locateusing(in usingchar, in target, in instring);
ND bool locateusing(in usingchar, in target, in instring, io setting);
ND bool locateusing(in usingchar, in target, in instring, io setting, const int fieldno, const int valueno = 0 , const int subvalueno = 0 );

ND var  sum(in instring, SV sepchar);
ND var  sum(in instring);
ND var  sumall(in instring);

ND var  crop(in instring);
   IO   cropper(io iostring);

ND var  sort(in instring, SV sepchar = _FM );
   IO   sorter(io iostring, SV sepchar = _FM );

ND var  reverse(in instring, SV sepchar = _FM );
   IO   reverser(io iostring, SV sepchar = _FM );

ND var  shuffle(in instring, SV sepchar = _FM );
   IO   shuffler(io iostring, SV sepchar = _FM );

ND var  parse(in instring, char sepchar = ' ');
   IO   parser(io instring, char sepchar = ' ');

// DATABASE

ND bool connect(in connectioninfo = "" );
   void disconnect(void);
   void disconnectall(void);

ND bool sqlexec(in sqlcmd);
ND bool sqlexec(in sqlcmd, out response);

ND bool attach(in filenames);

ND bool dbcreate(in dbname, in from_dbname = "" );
ND var  dblist(void);
ND bool dbcopy(in from_dbname, in to_dbname);
ND bool dbdelete(in dbname);

ND bool createfile(in dbfilename);
ND bool deletefile(in dbfilename_or_var);
ND bool clearfile(in dbfilename_or_var);
ND bool renamefile(in old_dbfilename, in new_dbfilename);
ND var  listfiles(void);

ND var  reccount(in dbfilename_or_var);

ND bool createindex(in dbfilename_or_var, in fieldname = "" , in dictfilename = "" );
ND bool deleteindex(in dbfilename_or_var, in fieldname = "" );
ND var  listindex(in dbfilename = "" , in fieldname = "" );

ND bool begintrans(void);
ND bool statustrans(void);
ND bool rollbacktrans(void);
ND bool committrans(void);
   void cleardbcache(void);

ND bool lock(in dbfile, in key);
   bool unlock(in dbfile, in key);
   bool unlockall(in conn = "" );

   void cleardbcache(in conn = "" );

ND bool open(in dbfilename, io dbfile);
ND bool open(in dbfilename);
ND void close(in dbfile);
// bool open(in dictdata, in dbfilename, io dbfile);

ND bool read(io record, in dbfile, in key);
ND bool readc(io record, in dbfile, in key);
ND bool readf(io record, in dbfile, in key, in fieldnumber);

   void write(in record, in dbfile, in key);
   void writec(in record, in dbfile, in key);
   void writef(in record, in dbfile, in key, const int fieldno);
ND bool deletec(in dbfile, in key);
ND bool updaterecord(in record, in dbfile, in key);
ND bool insertrecord(in record, in dbfile, in key);

ND bool dimread(dim& dimrecord, in dbfile, in key);
   void dimwrite(const dim& dimrecord, in dbfile, in key);

// moved to exoprog so they have access to default cursor in mv.CURSOR
// bool select(in sortselectclause = "" );
// void clearselect(void);
// bool readnext(io key);
// bool readnext(io key, io valueno);
// bool readnext(io record, io key, io value);
// deleterecord should be made a free function that doesnt use CURSOR
// bool deleterecord(in dbfilename_or_var_or_command, in key = "" );

ND var xlate(in dbfilename, in key, in fieldno, const char* mode);
ND var xlate(in dbfilename, in key, in fieldno, in mode);

ND var lasterror(void);
   var loglasterror(in source = " ");

////////////////////////////////////////////
//output(args), outputl(args), outputt(args)
////////////////////////////////////////////

// BINARY TRANSPARENT version of printx(void)
// No automatic separator
// Outputs to stdout/cout
// Multi-argument
// Only outputl(args) flushes output
// Calls .output member function instead of direct cout/cerr/clog <<

// output(args...)

template <typename... Printable>
void output(const Printable&... value) {
	(var(value).output(), ...);
}

// outputl(args) appends \n and flushes output

template <typename... Printable>
void outputl(const Printable&... value) {
	(var(value).output(), ...);
	var("").outputl();
}

// outputt(args)  \t separator appends nothing

template <typename... Printable>
void outputt(const Printable&... value) {
	(var(value).outputt(), ...);
	//var("").outputt();
}

/////////////////////////////////////////////
// printl(args), errputl(args), logputl(args)
/////////////////////////////////////////////

// Multi-argument
// Default sep is " " for printl
// Always appends \n
// Flushes output

//Use like this:
//
// 1. printx("hi","ho");                 // "hi ho"
//
// 2. printx<','>("hi","ho");            // "hi,ho"
//
// 3. static char const sep[] = ", ";
//    printx<sep>("hi","ho");            // "hi, ho"

// printl(args) to cout

template <auto sep = ' ', typename Printable, typename... Additional>
void printl(const Printable& value, const Additional&... values) {
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

//// errputl(args) to cerr
//
//template <auto sep = ' ', typename Printable, typename... Additional>
//void errputl(const Printable& value, const Additional&... values) {
//	std::cerr << value;
//	((std::cerr << sep << values), ...);
//	std::cerr << std::endl;
//}
//
//// logputl(args) to clog
//
//template <auto sep = ' ', typename Printable, typename... Additional>
//void logputl(const Printable& value, const Additional&... values) {
//	std::clog << value;
//	((std::clog << sep << values), ...);
//	std::clog << std::endl;
//}

// errputl(args) to cerr

template <typename... Printable>
void errputl(const Printable&... values) {
	((std::cerr << var(values).convert(_ALL_FMS, _VISIBLE_FMS)), ...);
	std::cerr << std::endl;
}

// logputl(args) to clog

template <typename... Printable>
void logputl(const Printable&... values) {
	((std::clog << var(values).convert(_ALL_FMS, _VISIBLE_FMS)), ...);
//	std::clog << std::endl;
	std::clog << "\n";
}

//////////////////////////////////////////
// printx(args), errput(args), logput(args)
//////////////////////////////////////////

// Does *not* append \n
// Multi-argument
// Default sep is ' ' for printx
// Does *not* flush output

// printx(args) to cout

template <auto sep = ' ', typename Printable, typename... Additional>
void printx(const Printable& value, const Additional&... values) {
	std::cout << value;
	((std::cout << sep << values), ...);
}

//// errput(args) to cerr
//
//template <auto sep = ' ', typename Printable, typename... Additional>
//void errput(const Printable& value, const Additional&... values) {
//	std::cerr << value;
//	((std::cerr << sep << values), ...);
//}
//
//// logput(args) to clog
//
//template <auto sep = ' ', typename Printable, typename... Additional>
//void logput(const Printable& value, const Additional&... values) {
//	std::clog << value;
//	((std::clog << sep << values), ...);
//}

// errput(args) to cerr

template <typename... Printable>
void errput(const Printable&... values) {
	((std::cerr << var(values).convert(_ALL_FMS, _VISIBLE_FMS)), ...);
}

// logput(args) to clog

template <typename... Printable>
void logput(const Printable&... values) {
	((std::clog << var(values).convert(_ALL_FMS, _VISIBLE_FMS)), ...);
}

/////////////////////////////////
// printl(void), errputl(void), logputl(void)
/////////////////////////////////

// Always outputs just \n
// Flushes output

// printl(void) to cout

inline void printl(void) {
	std::cout << std::endl;
}

// errputl(void) to cerr

inline void errputl(void) {
	std::cerr << std::endl;
}

// logputl(void) to clog

inline void logputl(void) {
//	std::clog << std::endl;
	std::clog << "\n";
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
	((std::cout << values << sep), ...);
}

///////////
// printt(void)
///////////

// Zero arguments
// Default sep is tab
// Just appends a sep (tab)
// Does *not* flush output

// printt(void) to cout

template <auto sep = '\t'>
void printt(void) {
	std::cout << sep;
}

#ifdef EXO_FORMAT

/////////////////
// print, println - requires compile time format string
/////////////////

//#ifdef __cpp_lib_format

//using fmt::print;
//using fmt::println;
//using fmt::vprint;
//using fmt::vprintln;

//void println() {
//	std::cout << std::endl;
//}

/////////////////////
//// vprint, vprintln - can use a variable run time format string
/////////////////////
//
//template <typename... Args>
//void vprint(std::string_view fmt_str, Args&&... args) {
//	std::cout << fmt::vformat(fmt_str, fmt::make_format_args(args...));
////	fmt::vformat_to(std::cout, fmt_str, fmt::make_format_args(args...));
//}
//
//template <typename... Args>
//void vprintln(std::string_view fmt_str, Args&&... args) {
//	std::cout << fmt::vformat(fmt_str, fmt::make_format_args(args...)) << std::endl;
////	fmt::vformat_to(std::cout, fmt_str, fmt::make_format_args(args...));
////	std::cout << std::endl;
//}
//
////////////
//// vformat - can use a variable run time format string
////////////
//
//// not using fmt/std::vformat because we want to return a var
//template <typename... Args>
//ND var xvformat(SV fmt_str, Args&&... args) {
////	return fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
////error: cannot bind non-const lvalue reference of type ‘exo::var&’ to an rvalue of type ‘exo::var’
//	try {
//		return fmt::vformat(fmt_str, fmt::make_format_args(args...));
//	} catch (fmt::format_error e) {
////		throw exo::VarError("Format error: " ^ var(fmt_str).squote() ^ " " ^ e.what());
//		throw exo::VarError("Format error: " ^ var(fmt_str).squote() ^ " " ^ e.what());
//	}
//}
//
//// Replicated in var.h and exofuncs.h - KEEP IN SYNC
//#if __GNUC__ >= 7 || __clang_major__ > 15
//    // Works at compile time (only? or if possible)
//#   define EXO_FORMAT_STRING_TYPE1 fmt::format_string<var, Args...>
//#   define EXO_FORMAT_STRING_TYPE2 fmt::format_string<Args...>
//#else
//    // Always run time
//#   define EXO_FORMAT_STRING_TYPE1 SV
//#   define EXO_FORMAT_STRING_TYPE2 SV
//#endif
//
//// not using fmt/std::vformat because we want to return a var
//template <typename... Args>
//ND var xformat(EXO_FORMAT_STRING_TYPE2 fmt_str, Args&&... args) {
////	return fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
//// error: cannot bind non-const lvalue reference of type ‘exo::var&’ to an rvalue of type ‘exo::var’
//	return fmt::vformat(fmt_str, fmt::make_format_args(args...));
//}

//template<class... Args>
//ND var vformat(SV fmt_str, Args&&... args) {
//	return fmt::vformat(fmt_str, fmt::make_format_args(*this, args...) );
//}

///////////
//// format - requires a compile time format string
///////////
//
//// not using fmt/std::format because we want to return a var
//// TODO does this prevent "using std;" when using #include <format> (standard)
//// by introducting an overload of std::format that only differs in return type?
//#ifdef EXO_FORMAT_MF
//template<class... Args>
//ND var xformat(fmt::format_string<Args...> fmt_str, Args&&... args) {
////#if __cpp_if_consteval >= 202106L
////	if consteval {
////		return fmt::format(fmt_str, args... );
////	} else
////#endif
//	{
//		return fmt::vformat(fmt_str, fmt::make_format_args(args...) );
//	}
//}
////#else
////template<class... Args>
////ND var xformat(std::string_view fmt_str, Args&&... args) {
////	return xvformat(fmt_str, args...);
////}
//#endif

//using print = std::print;
//using format = fmt::format;

//// Takes and returns a var instead of a std::string
//template<class... Args>
//ND var format(var fmt_str, Args&&... args) {
//	return fmt::vformat(fmt_str, fmt::make_format_args(args...));
//}

// Returns a var instead of a std::string
// WARNING: In gcc 14 on ubuntu 24.04
// this REQUIRES A PATCHED VERSION OF /usr/include/c++/14/ostream
// See comment in vardefs.h about EXO_FORMAT
template<class... Args>
ND var  format(SV fmt_str, Args&&... args) {
	THISIS("var  format(SV fmt_str, Args&&... args)")
#if __clang_major__ and __clang_major__ <= 15
	// Dont forward if the format library cannot handle it
	return fmt::vformat(fmt_str, fmt::make_format_args(args...));
#else
	return fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
#endif
}

// print
template<class... Args>
   void print(SV fmt_str, Args&&... args) {
#if __clang_major__ and __clang_major__ <= 15
	// Dont forward if the format library cannot handle it
	fmt::vprint(fmt_str, fmt::make_format_args(args...));
#else
	fmt::vprint(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
#endif
}

// println
template<class... Args>
   void println(SV fmt_str, Args&&... args) {
//	fmt::vprint(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
	print(fmt_str, std::forward<Args>(args)...);
	printl();
}

//    template<class... Args>
//    ND var  format(in fmt_str, Args&&... args) const {
//        THISIS("var  format(SV fmt_str, Args&&... args) const")
//        assertString(function_sig);
//        return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(*this, args...) );
//    }


#endif //EXO_FORMAT

}  // namespace exo

#ifdef EXO_FORMAT

// formatter for var must be defined in global namespace

// All c++ format specifiers formatting is locale-independent by default.
// Use the 'L' format specifier to insert the appropriate number separator characters from the locale:
// Locale only affects arithmetic and bool types
// https://en.cppreference.com/w/cpp/utility/format/spec
//
// Exodus conversions likewise although note that some conversions by oconv member function of exoprog is dependent on exoenv globals

/////////////////
// fmt::formatter - for var.
/////////////////

// Needs to know how to delegate parse and format functions to standard string_view, double and int versions
// therefore multiple inheritance
template <>
struct fmt::formatter<exo::var> : formatter<std::string_view>, formatter<double>, formatter<int> {

	// Detect leading ':' -> exodus conversions/format specifiers
	// otherwise trailing characters -> standard fmt/std format specifiers
	//
	// parse determines which parse/format functions should be used
	//
	// ':' for exodus conversions which do not use standard parse/float functions
	// 'F' standard floating point f,F,e,E,g,G,a,A
	// 'I' standard intege b,B,d,o,x,X
	// 'S' standard string for s,c or not specified
	char fmt_code_ = ' ';

	// Need to pass exodus format strings to formatter (starting :) e.g. as in "abc{::MD20}def"
	// since all parsing is done at runtime for exodus conversions
	std::string fmt_str_;

	// TODO allow dynamic arguments for exodus conversions (MD R# etc.)
	// Future use for dynamic arguments in exodus conversions ("format specifiers")
	std::string arg_str; // only here because clang on Ubuntu 22.04 cannot declare str string in constexpr functions
	int dynargn1 = -1;
	// Will require storage of the argument number(s) (either automatic or manual)
	// and using those argument numbers to extract the right argument value in the format stage
	// argument values are available in ctx.arg(n) in the format stage
	// but as they are variants they require careful extraction using visit_format_arg(...) or basic_format_arg.visit(...)
	//
	// 1. For exodus conversions?
	//
	// MD{}{}P ?
	//
	// 2. standard c++ fmt/std::format specifiers already work
	//  because we are calling the standard parse and format functions
	//
	// std::format dynamic arguments
	// https://hackingcpp.com/cpp/libs/fmt.html
	//
	// strings        -> field-width, cut-width
	// chars          -> pad-width
	// integers       -> pad-width
	// floating point -> pad-width, precision
	//
	// Note: int/float are not cut down in size if they exceed the pad-width

//////////////////////////
// fmt::formatter::parse() - maybe at compile time
//////////////////////////
//
//	NOTE that providing a parse function means the context object passed into format
//  is no longer capable of being forwarded to the standard format functions as in ...
//
//	Works but only supports string format specifier
//
//	template <>
//	struct formatter<exo::var> : formatter<std::string_view> {
//		auto format(const exo::var& var1, format_context& ctx) {
//  	return formatter<std::string_view>::format(var1.toString(), ctx);
//	};
//
//
template<typename ParseContext>
constexpr auto parse(ParseContext& ctx) {

	// Cannot use static timebank in a constexpr function
	//THISIS("auto fmt::formatter::parse(...)");

	//std::cerr << " \n>>> exofuncs.h parse  '" << std::string(ctx.begin(), ctx.end()) << "'" << std::endl;

	auto it = ctx.begin();

	// We might have been given nothing if pattern was {}
	if (it == ctx.end()) {
		fmt_code_ = 'S';
		return formatter<std::string_view>::parse(ctx);
	}

	// Pick/Exodus conversion codes if starts with :
	const bool exodus_style_conversion = *it == ':';

	while (it != ctx.end()) {

		// Acquire (and skip over?) any dynamic argument
		// TODO handle multiple dynamic arguments?
		if (*it == '{') {
			//fmt_str_.push_back('{');
			//std::string arg_str;
			arg_str.clear();
			it++;

			while (*it != '}' && it != ctx.end()) {
			//	fmt_str_.push_back(*it);
				arg_str.push_back(*it);
				it++;
			}

			if (! arg_str.empty()) {
				try {
					dynargn1 = std::stoi(arg_str);
				} catch (...) {
					//throw std::format_error("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'");
					//throw_format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg ") + arg_str);
					//throw_format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'").c_str());
					// Cannot throw VarError in constexpr parse expression
					throw  fmt::format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'").c_str());
				}
			}

			if (*it == '}') {
				it++;
			}

			continue;
		}

		try {
			// Terminate parse if we reach } char
			if (*it == '}') {

				// 1. exodus style conversions/format specifiers
				// need the whole fmt string (e.g. "MD20P") in the format stage
				// so save it in a member data of the this formatter object
				if (exodus_style_conversion) {
					fmt_code_ = ':';
					fmt_str_ = std::string(ctx.begin() + 1, it);
					return it;
				}

				// 2. C++ style format codes need parsing
				switch (fmt_code_) {

					// Floating point
					case 'a':
					case 'A':
					case 'e':
					case 'E':
					case 'f':
					case 'F':
					case 'g':
					case 'G': {
						fmt_code_ = 'F';
						return formatter<double>::parse(ctx);
					}
					// Integer
					case 'd':
					case 'b':
					case 'B':
					case 'o':
					case 'X':
					case 'x': {
						fmt_code_ = 'I';
						return formatter<int>::parse(ctx);
					}
					// String
					case 'c':
					default:
						fmt_code_ = 'S';
						return formatter<std::string_view>::parse(ctx);
				}
			}
		} catch (fmt::format_error e) {
			throw exo::VarError("Format error: " ^ exo::var(std::string(ctx.begin(), ctx.end())).squote() + " " + e.what());
//			throw exo::VarError("Format error: '" + std::string(std::string(ctx.begin(), ctx.end())) + "' " + e.what());

		} catch (...) {
			throw exo::VarError("Format error: " ^ exo::var(std::string(ctx.begin(), ctx.end())).squote());
//			throw exo::VarError("Format error: '" + std::string(std::string(ctx.begin(), ctx.end())) + "'");
		}

		fmt_code_ = *it;
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		it++;
#pragma GCC diagnostic pop
	}
	// If we dont find a closing "}" char

//#if EXO_FORMAT == 1
	[[unlikely]]
	// Cannot throw VarError in constexpr parse
	throw fmt::format_error("exofuncs.h: formatter_parse: format missing trailing }");
//#else
//	throw_format_error("exofuncs.h: formatter_parse: format missing trailing }");
//		fmt_str_ = "{}";
//		return it;
//#endif
	//std::unreachable();

} // formatter::parse()

//////////////////////
// formatter::format() - run time
//////////////////////

// Good format code description although not from "fmt", not official c++
// https://fmt.dev/latest/syntax.html#formatspec

template <typename FormatContext>
auto format(const exo::var& var1, FormatContext& ctx) const {

	THISIS("auto fmt::formatter::format(var) const")
	//std::cerr << ">>> exofuncs.h format '" << fmt_str_ << "' '" << fmt_code_ << "' '" << var1 << "'\n";

	switch (fmt_code_) {

		// 1. EXODUS conversions

		case ':': {

			// Unfortunately without time zone or number format currently.
			// TODO allow thread_local global timezone, number format?
			// {::MD20PZ}
			// {::D2/E} etc.
			// {::MTHS} etc.
			//return formatter<std::string_view>::format(var1, ctx);
			exo::var converted_var1 = var1.oconv(fmt_str_.c_str());
			auto sv1 = std::string_view(converted_var1);
			return vformat_to(ctx.out(), "{:}", fmt::make_format_args(sv1));

		}

		// 2. C++ style format codes

		// Standard floating point on var toDouble()
		case 'F': {
			return formatter<double>::format(var1.toDouble(), ctx);
		}

		// Standard integer on var toInt()
		case 'I': {
			return formatter<int>::format(var1.toInt(), ctx);
		}

		// Standard string on var toString()
		case 'S':
		default:
			return formatter<std::string_view>::format(std::string_view(var1), ctx);

	}
} // formatter::format
}; //fmt::formatter

// How to format user defined types.
//
// 1. https://en.cppreference.com/w/cpp/utility/format/formatter
//
// 2. https://fmt.dev/latest/api.html
// Section: Formatting User-Defined Types
//
// *** NEEDS #include fmt/format.h not fmt/core.h
//
//template<class T>
//struct formatter {
//	constexpr auto parse(format_parse_context&);
//
//	typename format_context::iterator
//	format(const T&, format_context&);
//};
//


//namespace fmt {
//
//// Works fine but only supports string format specifiers
//template <>
//struct formatter<exo::var> : formatter<std::string_view> {
//
//auto format(const exo::var& var1, format_context& ctx) {
//	// Sadly we dont have access to the formatstring that parse has access to
//	//auto s = std::string(ctx.begin(), ctx.end());
//	return formatter<std::string_view>::format(var1.toString(), ctx);
//}
//
//}; // formatter
//
//} // namespace fmt

#endif	// EXO_FORMAT

// clang-format on

#endif	// EXODUSFUNCS_H

/*
FOLLOWING IS A FAILED ATTEMPT TO USE VISITOR PATTERN
TO PROVIDE DYNAMIC FORMAT ARGUMENTS FOR EXODUS FORMAT CONVERSIONS LIKE {:MD20P}

//////////////////
// Visitor pattern
//////////////////   

template<typename ... Ts>
struct Overload : Ts ... {
    using Ts::operator() ...;
};
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

FOLLOWING SHOULD BE INSIDE class formatter<exo::var>::format function

// ctx.arg(n) appear to be format_arguments which is a variant like object
// https://en.cppreference.com/w/cpp/utility/format/basic_format_context
std::cerr << " has dynargn0 " << bool(ctx.arg(0)) << std::endl;
std::cerr << " has dynargn1 " << bool(ctx.arg(1)) << std::endl;
std::cerr << " has dynargn2 " << bool(ctx.arg(2)) << std::endl;
std::cerr << " has dynargn3 " << bool(ctx.arg(3)) << std::endl;
std::cerr << " has dynargn4 " << bool(ctx.arg(4)) << std::endl;
////				auto dynarg1v = 13;
////				return vformat_to(ctx.out(), fmt_str_, make_format_args(var1.toString(), dynarg1v));
//
//					auto format_args = make_format_args(var1.toString(), 1, 2, 3, 4);
////				if (dynarg1) {
////					format_args.push_back(dynarg1));
////				}
////				return vformat_to(ctx.out(), fmt_str_, format_args);
////				return vformat_to(ctx.out(), fmt_str_, ctx.arg);
////				return vformat_to(ctx.out(), fmt_str_, ctx.arg[dynargn1]);
//
// requires #include <fmt/args.h>

		fmt::dynamic_format_arg_store<FormatContext> store;
		store.push_back(var1.toString());

//	https://en.cppreference.com/w/cpp/utility/format/basic_format_arg
//	std::basic_format_arg
//
//	A basic_format_arg object behaves as if it stores a std::variant of the following types:
//
//	std::monostate (only if the object was default-constructed)
//	bool
//	Context::char_type
//	int
//	unsigned int
//	long long int
//	unsigned long long int
//	float
//	double
//	long double
//	const Context::char_type*
//	std::basic_string_view<Context::char_type>
//	const void*
//	basic_format_arg::handle

// https://en.cppreference.com/w/cpp/utility/format/visit_format_arg
// std::visit(std::forward<Visitor>(vis), value)
// visit_format_arg( Visitor&& vis, std::basic_format_arg<Context> arg );

// Sadly int(vx) doesnt compile even when protected inside constexpr visitor lambda when type of vx is int!

    auto visitor1 = Overload{
        [](auto vx) -> int {
				std::cout << "Visitor type    " << typeid(vx).name() << '\n';
//				std::cout << "double        d " << typeid(double).name() << '\n';
//				std::cout << "long double   e " << typeid(long double).name() << '\n';
//				std::cout << "short int     s " << typeid(short int).name() << '\n';
//				std::cout << "int           i " << typeid(int).name() << '\n';
//				std::cout << "long int      l " << typeid(long int).name() << '\n';
//				std::cout << "long long int x " << typeid(long long int).name() << '\n';
//return int(-1);
				if (std::is_same<decltype(vx), monostate>::value)
					return int(0);// missing argument

				if (std::is_same<decltype(vx), double>::value) {
					std::cout << "double" << std::endl;
					//return int(vx);
					return int(sizeof vx);
				}
				if (std::is_integral<decltype(vx)>::value)
					return int(11);
				if (std::is_floating_point<decltype(vx)>::value)
//					return static_cast<const int>(vx);
					return int(vx); // WHY DOES THIS NOT COMPILE??
					return int(sizeof vx);//8

//				if (std::is_same<decltype(vx), const char*>::value)
//					return int(2);
				if (std::is_same<decltype(vx), const fmt::v10::monostate>::value)
					return int(5);
				if (std::is_same<decltype(vx), std::monostate>::value)
					return int(5);
				if (std::is_same<decltype(vx), bool>::value)
					return int(5);
//				if (std::is_same<decltype(vx), FormatContext::char_type>::value)
//					return int(5);
//				if (std::is_same<decltype(vx), FormatContext::char_type*>::value)
//					return int(5);
				if (std::is_same<decltype(vx), short int>::value)
					return int(5);
				if (std::is_same<decltype(vx), int>::value)
					return int(5);
				if (std::is_same<decltype(vx), unsigned int>::value)
					return int(5);
				if (std::is_same<decltype(vx), long long int>::value)
					return int(5);
				if (std::is_same<decltype(vx), unsigned long long int>::value)
					return int(5);
				if (std::is_same<decltype(vx), float>::value)
					return int(5);
				if (std::is_same<decltype(vx), double>::value)
					return int(5);
				if (std::is_same<decltype(vx), long double>::value)
					return int(5);
				if (std::is_same<decltype(vx), std::basic_string_view<typename FormatContext::char_type>>::value)
					return int(5);
				if (std::is_same<decltype(vx), const void*>::value)
					return int(5);

				if (std::is_same<decltype(vx), const char*>::value)
					return int(5);
				if (std::is_same<decltype(vx), basic_string_view<char>>::value)
					return int(5);
//				if (std::is_same<decltype(vx), basic_format_arg<FormatContext>::handle>::value)
//					return int(5);

//				if (std::is_floating_point<decltype(vx)>::value)
//					return static_cast<int>(vx);
//				if (std::is_arithmetic_v<decltype(vx)>)
//					return static_cast<int>(vx);
				//return uint(sizeof vx);
//				fmt::println("missing {}", typeid(vx));
				return int(6);
		},
	};
//	[](std::monostate)           {return int(-1);},
//	[](bool v)                   {return int(v);},
//	[](FormatContext::char_type) {return int(-1);},
//	[](int v)                    {return int(v);},
//	[](unsigned int v)           {return int(v);},
//	[](long long int v)          {return int(v);},
//	[](unsigned long long int v) {return int(v);},
//	[](float v)                  {return int(v);},
//	[](double v)                 {return int(v);},
//	[](long double v)            {return int(v);},
//	[](const FormatContext::char_type*) {return int(-1);},
//	[](std::basic_string_view<typename FormatContext::char_type>) {return int(-1);},
//	[](const void*)              {return int(-1);},
//	[](basic_format_arg<FormatContext>::handle) {return int(-1);},
		auto result = visit_format_arg(visitor1, ctx.arg(1));
		//auto result2 = visit_format_arg(visitor1, ctx.arg(2));
		store.push_back(result);
//std::string result = vformat_to(ctx.out(), fmt_str_, store);
*/
