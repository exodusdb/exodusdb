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

#ifdef EXO_FORMAT
// Including the large fmt library header here so exodus::format can precompile strings using fmt::vformat
//
// Sadly that implies that it will be waste time in endless recompilations until it becomes
// a c++ module.
//
// Always use fmt library for now because of various bugs causing inconsistency
// in early versions of stdlibc++ implementation of std::format
// As of 2024
// 1. Width calculation doesn’t use grapheme clusterization. The latter has been implemented in a separate branch but hasn’t been integrated yet.
// 2. Most C++20 chrono types are not supported yet.
//
//#include <version>
//#ifdef __cpp_lib_format
//#if __has_include(<formatx>)
//#	warning has <format>
//#	define EXO_FORMAT 1
//#	include <format>
//	namespace fmt = std;

#undef EXO_FORMAT // only to avoid warning about redefinition below

//#elif __has_include(<fmt/core.h>)
#if __has_include(<fmt/core.h>)
//#	warning Using fmt library instead std::format
#	define EXO_FORMAT 2
#	pragma GCC diagnostic push
#	pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#	pragma GCC diagnostic ignored "-Winline"
#	pragma GCC diagnostic ignored "-Wswitch-default"
//#	include <fmt/core.h>
#	include <fmt/format.h> // for fmt::formatter<std::string_view> etc.
#	include <fmt/args.h> // for fmt::dynamic_format_arg_store
#	include <variant>
#	pragma GCC diagnostic pop
#	if __GNUC__ >= 11
///root/exodus/fmt/include/fmt/core.h: In member function ‘constexpr auto fmt::v10::formatter<exodus::var>::parse(ParseContext&) [with ParseContext =
// fmt::v10::basic_format_parse_context<char>]’:
///root/exodus/fmt/include/fmt/core.h:2712:22: warning: inlining failed in call to ‘constexpr const Char* fmt::v10::formatter<T, Char, typename std::enable_if<(fmt::v10::detail::type_constant<T, Char>::value != fmt::v10::detail::type::custom_type), void>::type>::parse(ParseContext&) [with ParseContext = fmt::v10::basic_format_parse_context<char>; T = fmt::v10::basic_string_view<char>; Char = char]’: --param max-inline-insns-single limit reached [-Winline]
// 2712 |   FMT_CONSTEXPR auto parse(ParseContext& ctx) -> const Char* {
//      |                      ^~~~~
///root/exodus/test/src/../../exodus/libexodus/exodus/exofuncs.h:850:58: note: called from here
//  850 |                 return formatter<std::string_view>::parse(ctx);
//      |                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
#		pragma GCC diagnostic ignored "-Winline"
#	endif
#endif
#endif // EXO_FORMAT

#include <exodus/var.h>
#include <mutex>

// add global function type syntax to the exodus users
// SIMILAR code in exofuncs.h and varimpl.h
namespace exodus {

	// clang-format off

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wreserved-identifier"

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

	// _cplusplus is in format YYYYMM e.g. 202002, 202102, 202302 etc.
	// We will extract the two digit year only - using integer division and integer remainder ops.
	// Years e.g. 21 which are in between the actual standards like c++20, c++23, c++26 etc.
	// indicate partial informal support for some features of the next standard
	const constexpr auto _CPP_STANDARD=__cplusplus / 100 % 1000;

#if defined(_WIN64) or defined(_LP64)
#	define                _PLATFORM   "x64"
	const var              PLATFORM = _PLATFORM;
#else
#	define                _PLATFORM   "x86"
	const var              PLATFORM = _PLATFORM;
#endif

#ifdef __clang__
	//__clang_major__
	//__clang_minor__
	//__clang_patchlevel__
	//__clang_version__
	inline const constexpr auto _COMPILER =  "clang";
	inline const constexpr auto _COMPILER_VERSION =  __clang_major__;
#elif defined(__GNUC__)
	//__GNUC__
	//__GNUC_MINOR__
	//__GNUC_PATCHLEVEL__
	inline const constexpr auto _COMPILER =  "gcc";
	inline const constexpr auto _COMPILER_VERSION =  __GNUC__;
#else
	inline const constexpr auto _COMPILER =  "unknown";
	inline const constexpr auto _COMPILER_VERSION = 0;
#endif

#pragma clang diagnostic pop

// clang-format on

#ifndef EXO_FUNCS_CPP
extern
#endif
PUBLIC std::mutex global_mutex_threadstream;

//
// SLOW = threadsafe. With locking.  Output all arguments together.
//      = printx/printl/printt, errput/errputl
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

int exodus_main(int exodus_argc, const char* exodus_argv[], ExoEnv& mv, int threadno);

ND var  osgetenv(CVR envcode DEFAULT_EMPTY);
ND bool osgetenv(CVR code, VARREF value);
   void ossetenv(CVR code, CVR value);

ND var ostempdirpath(void);
ND var ostempfilename(void);

ND bool assigned(CVR var1);
ND bool unassigned(CVR var1);

   void move(VARREF fromvar, VARREF tovar);
   void swap(VARREF var1, VARREF var2);

// OS

ND var date(void);
ND var time(void);
ND var ostime(void);
ND var timestamp(void);
ND var timestamp(CVR date, CVR time);
//ND var timedate(void);

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

ND var  oscwd(void);
ND var  oscwd(CVR dirpath);

   void osflush(void);
ND var  ospid(void);
ND var  ostid(void);

ND bool osshell(CVR command);
ND bool osshellwrite(CVR writestr, CVR command);
ND bool osshellread(VARREF readstr, CVR command);
ND var  osshellread(CVR command);

//var execute(CVR command);

//void debug(CVR DEFAULT_EMPTY);
ND var backtrace(void);

   bool setxlocale(const char* locale);
ND var  getxlocale(void);

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

// integer(void) represents pick int(void) because int(void) is reserved word in c/c++
// Note that integer like pick int(void) is the same as floor(void)
// whereas the usual c/c++ int(void) simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt(void) (although toInt(void) returns an int
// instead of a var like normal exodus functions)
ND var integer(CVR num1);
ND var floor(CVR num1);
ND var round(CVR num1, const int ndecimals DEFAULT_0);

ND var rnd(const int number);
   void initrnd(CVR seed DEFAULT_0);

// INPUT

ND var getprompt(void);
   void setprompt(CVR prompt);

var input(void);

var input(CVR prompt);

var inputn(const int nchars);

ND bool isterminal(void);
ND bool hasinput(const int millisecs DEFAULT_0);
ND bool eof(void);
   bool echo(const int on_off);

   void breakon(void);
   void breakoff(void);

// SIMPLE STRINGS

ND var len(CVR var1);
ND var textlen(CVR var1);
ND var textwidth(CVR var1);

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

ND var shuffle(CVR instring, SV sepchar DEFAULT__FM);
VARREF shuffler(VARREF iostring, SV sepchar DEFAULT__FM);

ND var parser(CVR instring, char sepchar DEFAULT_CSPACE);
VARREF parser(VARREF iostring, char sepchar DEFAULT_CSPACE);

// DATABASE

ND bool connect(CVR connectioninfo DEFAULT_EMPTY);
   void disconnect(void);
   void disconnectall(void);

ND bool dbcreate(CVR dbname);
ND var  dblist(void);
ND bool dbcopy(CVR from_dbname, CVR to_dbname);
ND bool dbdelete(CVR dbname);

ND bool createfile(CVR dbfilename);
ND bool deletefile(CVR dbfilename_or_var);
ND bool clearfile(CVR dbfilename_or_var);
ND bool renamefile(CVR old_dbfilename, CVR new_dbfilename);
ND var  listfiles(void);

ND var  reccount(CVR dbfilename_or_var);

ND bool createindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
ND bool deleteindex(CVR dbfilename_or_var, CVR fieldname DEFAULT_EMPTY);
ND var  listindex(CVR dbfilename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY);

ND bool begintrans(void);
ND bool statustrans(void);
ND bool rollbacktrans(void);
ND bool committrans(void);
   void cleardbcache(void);

ND bool lock(CVR dbfilevar, CVR key);
   void unlock(CVR dbfilevar, CVR key);
   void unlockall(void);

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
// void clearselect(void);
// bool readnext(VARREF key);
// bool readnext(VARREF key, VARREF valueno);
// bool readnext(VARREF record, VARREF key, VARREF value);
// bool deleterecord(CVR dbfilename_or_var_or_command, CVR key DEFAULT_EMPTY);

ND var xlate(CVR dbfilename, CVR key, CVR fieldno, const char* mode);
ND var xlate(CVR dbfilename, CVR key, CVR fieldno, CVR mode);

ND var lasterror(void);
   var loglasterror(CVR source DEFAULT_SPACE);

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

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void printl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

//// errputl(args) to cerr
//
//template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
//void errputl(const Printable& value, const Additional&... values) {
//	LOCKIOSTREAM_SLOW
//	std::cerr << value;
//	((std::cerr << sep << values), ...);
//	std::cerr << std::endl;
//}
//
//// logputl(args) to clog
//
//template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
//void logputl(const Printable& value, const Additional&... values) {
//	LOCKIOSTREAM_FAST
//	std::clog << value;
//	((std::clog << sep << values), ...);
//	std::clog << std::endl;
//}

// errputl(args) to cerr

template <typename... Printable>
void errputl(const Printable&... values) {
	LOCKIOSTREAM_SLOW
	((std::cerr << values), ...);
	std::cerr << std::endl;
}

// logputl(args) to clog

template <typename... Printable>
void logputl(const Printable&... values) {
	LOCKIOSTREAM_SLOW
	((std::clog << values), ...);
	std::clog << std::endl;
}

//////////////////////////////////////////
// printx(args), errput(args), logput(args)
//////////////////////////////////////////

// Does *not* append \n
// Multi-argument
// Default sep is ' ' for printx
// Does *not* flush output

// printx(args) to cout

template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
void printx(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM_SLOW
	std::cout << value;
	((std::cout << sep << values), ...);
}

//// errput(args) to cerr
//
//template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
//void errput(const Printable& value, const Additional&... values) {
//	LOCKIOSTREAM_SLOW
//	std::cerr << value;
//	((std::cerr << sep << values), ...);
//}
//
//// logput(args) to clog
//
//template <auto sep DEFAULT_CSPACE, typename Printable, typename... Additional>
//void logput(const Printable& value, const Additional&... values) {
//	LOCKIOSTREAM_FAST
//	std::clog << value;
//	((std::clog << sep << values), ...);
//}

// errput(args) to cerr

template <typename... Printable>
void errput(const Printable&... values) {
	LOCKIOSTREAM_SLOW
	((std::cerr << values), ...);
}

// logput(args) to clog

template <typename... Printable>
void logput(const Printable&... values) {
	LOCKIOSTREAM_SLOW
	((std::clog << values), ...);
}

/////////////////////////////////
// printl(void), errputl(void), logputl(void)
/////////////////////////////////

// Always outputs just \n
// Flushes output

// printl(void) to cout

void printl(void) {
	LOCKIOSTREAM_SLOW
	std::cout << std::endl;
}

// errputl(void) to cerr

void errputl(void) {
	LOCKIOSTREAM_SLOW
	std::cerr << std::endl;
}

// logputl(void) to clog

void logputl(void) {
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
// printt(void)
///////////

// Zero arguments
// Default sep is tab
// Just appends a sep (tab)
// Does *not* flush output

// printt(void) to cout

template <auto sep = '\t'>
void printt(void) {
	LOCKIOSTREAM_SLOW
	std::cout << sep;
}

#ifdef EXO_FORMAT

/////////////////
// print, println - requires compile time format string
/////////////////

//#ifdef __cpp_lib_format

using fmt::print;
using fmt::println;

//void println() {
//	LOCKIOSTREAM_SLOW
//	std::cout << std::endl;
//}

///////////////////
// vprint, vprintln - can use a variable run time format string
///////////////////

template <typename... Args>
void vprint(std::string_view fmt_sv, Args&&... args) {
	LOCKIOSTREAM_SLOW
	std::cout << fmt::vformat(fmt_sv, fmt::make_format_args(args...));
}

template <typename... Args>
void vprintln(std::string_view fmt_sv, Args&&... args) {
	LOCKIOSTREAM_SLOW
	std::cout << fmt::vformat(fmt_sv, fmt::make_format_args(args...)) << std::endl;
}

//////////
// vformat - can use a variable run time format string
//////////

// not using fmt/std::vformat because we want to return a var
template <typename... Args>
ND var xvformat(std::string_view fmt_sv, Args&&... args) {
	return fmt::vformat(fmt_sv, fmt::make_format_args(args...));
}

// not using fmt/std::vformat because we want to return a var
template <typename... Args>
ND var xformat(std::string_view fmt_sv, Args&&... args) {
	return fmt::vformat(fmt_sv, fmt::make_format_args(args...));
}

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
////ND var xformat(std::string_view fmt_sv, Args&&... args) {
////	return xvformat(fmt_sv, args...);
////}
//#endif

#endif //EXO_FORMAT

}  // namespace exodus

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
struct fmt::formatter<exodus::var> : formatter<std::string_view>, formatter<double>, formatter<int> {

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
//	struct formatter<exodus::var> : formatter<std::string_view> {
//		auto format(const exodus::var& var1, format_context& ctx) {
//  	return formatter<std::string_view>::format(var1.toString(), ctx);
//	};
//
//
template<typename ParseContext>
constexpr auto parse(ParseContext& ctx) {

	//std::cerr << " \n>>> exofuncs.h parse  '" << std::string(ctx.begin(), ctx.end()) << "'" << std::endl;

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
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
					throw_format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'").c_str());
				}
			}

			if (*it == '}') {
				it++;
			}

			continue;
		}

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

		fmt_code_ = *it;
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		it++;
#pragma GCC diagnostic pop
	}
	// If we dont find a closing "}" char

#if EXO_FORMAT == 1
	[[unlikely]]
	throw std::format_error("exofuncs.h: formatter_parse: format missing trailing }");
#else
	throw_format_error("exofuncs.h: formatter_parse: format missing trailing }");
//		fmt_str_ = "{}";
//		return it;
#endif
	//std::unreachable();

} // formatter::parse()

//////////////////////
// formatter::format() - run time
//////////////////////

// Good format code description although not from "fmt", not official c++
// https://fmt.dev/latest/syntax.html#formatspec

template <typename FormatContext>
auto format(const exodus::var& var1, FormatContext& ctx) const {

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
			exodus::var converted_var1 = var1.oconv(fmt_str_.c_str());
			auto sv1 = std::string_view(converted_var1);
			return vformat_to(ctx.out(), "{:}", fmt::make_format_args(sv1));

		}

		// 2. C++ style format codes

		// Standard floating point on var::toDouble()
		case 'F': {
			return formatter<double>::format(var1.toDouble(), ctx);
		}

		// Standard integer on var::toInt()
		case 'I': {
			return formatter<int>::format(var1.toInt(), ctx);
		}

		// Standard string on var::toString()
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
//struct formatter<exodus::var> : formatter<std::string_view> {
//
//auto format(const exodus::var& var1, format_context& ctx) {
//	// Sadly we dont have access to the formatstring that parse has access to
//	//auto s = std::string(ctx.begin(), ctx.end());
//	return formatter<std::string_view>::format(var1.toString(), ctx);
//}
//
//}; // formatter
//
//} // namespace fmt

#endif	// EXO_FORMAT

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

FOLLOWING SHOULD BE INSIDE class formatter<exodus::var>::format function

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

