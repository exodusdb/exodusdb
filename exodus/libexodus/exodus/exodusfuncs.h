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

#include <exodus/mv.h>

#ifndef DLL_PUBLIC
#define DLL_PUBLIC
#endif

#ifdef SWIG
#define DEFAULTNULL
#define DEFAULTDOT
#define DEFAULTSPACE
#define DEFAULTVM
#else
#define DEFAULTNULL = ""
#define DEFAULTDOT = "."
#define DEFAULTSPACE = " "
#define DEFAULTVM = VM_
#endif

// add global function type syntax to the exodus users
namespace exodus {

// Removed to reduce compile time of exodus programs. Use .output() .errput() and .logput() for threadsafe output.
// print() errput() logput() to output all arguments together in a thread-safe manner
//static std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM

DLL_PUBLIC int exodus_main(int exodus__argc, const char* exodus__argv[], MvEnvironment& mv, int environmentno);

DLL_PUBLIC ND var osgetenv(CVR name = "");
DLL_PUBLIC bool osgetenv(CVR name, VARREF value);
DLL_PUBLIC bool ossetenv(CVR name, CVR value);
DLL_PUBLIC ND var ostempdirname();
DLL_PUBLIC ND var ostempfilename();
DLL_PUBLIC ND bool assigned(CVR var2);
DLL_PUBLIC ND bool unassigned(CVR var2);
DLL_PUBLIC VARREF transfer(VARREF fromvar, VARREF tovar);
DLL_PUBLIC VARREF exchange(VARREF var1, VARREF var2);
DLL_PUBLIC ND var date();
DLL_PUBLIC ND var time();
DLL_PUBLIC ND var timedate();
DLL_PUBLIC void ossleep(const int milliseconds);
DLL_PUBLIC ND var ostime();

DLL_PUBLIC void breakon();
DLL_PUBLIC void breakoff();

DLL_PUBLIC bool osopen(CVR osfilename, VARREF osfilevar, CVR locale DEFAULTNULL);
DLL_PUBLIC void osclose(CVR osfilevar);

//DLL_PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length,
//			const bool adjust = true);
//DLL_PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length,
//			const bool adjust = true);

//DLL_PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset,
//			 const bool adjust = true);
//DLL_PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset,
//			 const bool adjust = true);
DLL_PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length);
DLL_PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length);
DLL_PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset);
DLL_PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset);

DLL_PUBLIC bool oswrite(CVR data, CVR osfilename, CVR codepage DEFAULTNULL);
DLL_PUBLIC bool osread(VARREF data, CVR osfilename, CVR codepage DEFAULTNULL);
DLL_PUBLIC ND var osread(CVR osfilename);

DLL_PUBLIC bool osdelete(CVR osfilename);
DLL_PUBLIC bool osrename(CVR oldosdir_or_filename, CVR newosdir_or_filename);
DLL_PUBLIC bool oscopy(CVR fromosdir_or_filename, CVR newosdir_or_filename);
DLL_PUBLIC bool osmove(CVR fromosdir_or_filename, CVR newosdir_or_filename);

DLL_PUBLIC ND var oslist(CVR path DEFAULTDOT, CVR wildcard DEFAULTNULL, const int mode = 0);
DLL_PUBLIC ND var oslistf(CVR path DEFAULTDOT, CVR wildcard DEFAULTNULL);
DLL_PUBLIC ND var oslistd(CVR path DEFAULTDOT, CVR wildcard DEFAULTNULL);
DLL_PUBLIC ND var osfile(CVR filename);
DLL_PUBLIC ND var osdir(CVR filename);
DLL_PUBLIC bool osmkdir(CVR dirname);
DLL_PUBLIC bool osrmdir(CVR dirname, const bool evenifnotempty = false);
DLL_PUBLIC ND var oscwd();
DLL_PUBLIC var oscwd(CVR dirname);
DLL_PUBLIC void osflush();
DLL_PUBLIC ND var ospid();
DLL_PUBLIC var suspend(CVR command);
DLL_PUBLIC bool osshell(CVR command);
DLL_PUBLIC var osshellread(CVR command);
DLL_PUBLIC bool osshellread(VARREF readstr, CVR command);
DLL_PUBLIC bool osshellwrite(CVR writestr, CVR command);
DLL_PUBLIC void stop(CVR text DEFAULTNULL);
DLL_PUBLIC void abort(CVR text DEFAULTNULL);	 // dont confuse with abort() which is standard c/c++
DLL_PUBLIC void abortall(CVR text DEFAULTNULL);
DLL_PUBLIC var execute(CVR command);
// DLL_PUBLIC var chain(CVR command);
DLL_PUBLIC var logoff();
DLL_PUBLIC void debug();

DLL_PUBLIC bool setxlocale(CVR locale);
DLL_PUBLIC ND var getxlocale();

//replace by templates with variable number of arguments
//void print(CVR var2);
//void printl(CVR var2 DEFAULTNULL);
//void printt(CVR var2 DEFAULTNULL);

// MATH/BOOLEAN
DLL_PUBLIC ND var abs(CVR num1);
DLL_PUBLIC ND var pwr(CVR base, CVR exponent);
DLL_PUBLIC ND var exp(CVR power);
DLL_PUBLIC ND var sqrt(CVR num1);
DLL_PUBLIC ND var sin(CVR degrees);
DLL_PUBLIC ND var cos(CVR degrees);
DLL_PUBLIC ND var tan(CVR degrees);
DLL_PUBLIC ND var atan(CVR degrees);
DLL_PUBLIC ND var loge(CVR num1);
// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
DLL_PUBLIC ND var integer(CVR num1);
DLL_PUBLIC ND var floor(CVR num1);
DLL_PUBLIC ND var round(CVR num1, const int ndecimals = 0);

DLL_PUBLIC ND var rnd(const int number);
DLL_PUBLIC void initrnd(CVR seed);
DLL_PUBLIC ND var mod(CVR dividend, const int divisor);
DLL_PUBLIC ND var mod(CVR dividend, CVR divisor);

DLL_PUBLIC ND var at(const int columnorcode);
DLL_PUBLIC ND var at(CVR column, CVR row);
DLL_PUBLIC ND var getcursor();
DLL_PUBLIC void setcursor(CVR cursor);
DLL_PUBLIC ND var getprompt();
DLL_PUBLIC void setprompt(CVR prompt);
DLL_PUBLIC bool echo(const int on_off);

//DLL_PUBLIC VARREF input();
//DLL_PUBLIC VARREF input(VARREF intostr);
//DLL_PUBLIC VARREF input(CVR prompt, VARREF intostr);
//DLL_PUBLIC VARREF inputn(VARREF intostr, const int nchars);
DLL_PUBLIC var input();
DLL_PUBLIC var input(CVR prompt);
DLL_PUBLIC var inputn(const int nchars);

DLL_PUBLIC ND var len(CVR var2);
DLL_PUBLIC ND var length(CVR var2);
DLL_PUBLIC VARREF converter(VARREF instring, CVR oldchars, CVR newchars);
DLL_PUBLIC ND var convert(CVR instring, CVR oldchars, CVR newchars);
DLL_PUBLIC VARREF textconverter(VARREF instring, CVR oldchars, CVR newchars);
DLL_PUBLIC ND var textconvert(CVR instring, CVR oldchars, CVR newchars);
DLL_PUBLIC VARREF swapper(VARREF instring, CVR oldstr, CVR newstr);
DLL_PUBLIC ND var swap(CVR instring, CVR oldstr, CVR newstr);
DLL_PUBLIC VARREF replacer(VARREF instring, CVR oldstr, CVR newstr, CVR options DEFAULTNULL);
DLL_PUBLIC ND var replace(CVR instring, CVR oldstr, CVR newstr, CVR options DEFAULTNULL);
DLL_PUBLIC VARREF ucaser(VARREF instring);
DLL_PUBLIC ND var ucase(CVR instring);
DLL_PUBLIC VARREF lcaser(VARREF instring);
DLL_PUBLIC ND var lcase(CVR instring);
DLL_PUBLIC VARREF tcaser(VARREF instring);
DLL_PUBLIC ND var fcase(CVR instring);
DLL_PUBLIC VARREF tcaser(VARREF instring);
DLL_PUBLIC ND var fcase(CVR instring);
DLL_PUBLIC ND var unique(CVR instring);
DLL_PUBLIC VARREF inverter(VARREF instring);
DLL_PUBLIC ND var invert(CVR instring);
DLL_PUBLIC VARREF lowerer(VARREF instring);
DLL_PUBLIC ND var lower(CVR instring);
DLL_PUBLIC VARREF raiser(VARREF instring);
DLL_PUBLIC ND var raise(CVR instring);
DLL_PUBLIC VARREF splicer(VARREF instring, const int start1, const int length, CVR str);
DLL_PUBLIC ND var splice(CVR instring, const int start1, const int length, CVR str);
DLL_PUBLIC VARREF popper(VARREF instring);
DLL_PUBLIC ND var pop(CVR instring);
DLL_PUBLIC VARREF quoter(VARREF instring);
DLL_PUBLIC ND var quote(CVR instring);
DLL_PUBLIC VARREF unquoter(VARREF instring);
DLL_PUBLIC ND var unquote(CVR instring);
DLL_PUBLIC VARREF fieldstorer(VARREF instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);
DLL_PUBLIC ND var fieldstore(CVR instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);
DLL_PUBLIC VARREF trimmer(VARREF instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC VARREF trimmer(VARREF instring, CVR trimchars, CVR options);
DLL_PUBLIC ND var trim(CVR instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC VARREF trimmerf(VARREF instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC ND var trimf(CVR instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC VARREF trimmerb(VARREF instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC ND var trimb(CVR instring, const char* trimchars DEFAULTSPACE);
DLL_PUBLIC VARREF trimmer(VARREF instring, CVR trimchars);
DLL_PUBLIC ND var trim(CVR instring, CVR trimchars);
DLL_PUBLIC ND var trim(CVR instring, CVR trimchars, CVR options);
DLL_PUBLIC VARREF trimmerf(VARREF instring, CVR trimchars);
DLL_PUBLIC ND var trimf(CVR instring, CVR trimchars);
DLL_PUBLIC VARREF trimmerb(VARREF instring, CVR trimchars);
DLL_PUBLIC ND var trimb(CVR instring, CVR trimchars);
DLL_PUBLIC ND var crop(CVR instring);
DLL_PUBLIC var cropper(VARREF instring);
DLL_PUBLIC ND var chr(CVR integer);
DLL_PUBLIC ND var chr(const int integer);
DLL_PUBLIC ND var textchr(CVR integer);
DLL_PUBLIC ND var textchr(const int integer);
DLL_PUBLIC ND var match(CVR instring, CVR matchstr, CVR options DEFAULTNULL);
DLL_PUBLIC ND var seq(CVR char1);
DLL_PUBLIC ND var textseq(CVR char1);
DLL_PUBLIC ND var str(CVR instring, const int number);
DLL_PUBLIC ND var space(const int number);
DLL_PUBLIC ND var dcount(CVR instring, CVR substrx);
DLL_PUBLIC ND var count(CVR instring, CVR substrx);
DLL_PUBLIC ND var substr(CVR instring, const int startx);
DLL_PUBLIC ND var substr(CVR instring, const int startx, const int length);
DLL_PUBLIC var substrer(VARREF instring, const int startx);
DLL_PUBLIC var substrer(VARREF instring, const int startx, const int length);
DLL_PUBLIC ND var index(CVR instring, CVR substr, const int occurrenceno = 1);
DLL_PUBLIC ND var index2(CVR instring, CVR substr, const int startcharno = 1);
DLL_PUBLIC ND var field(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);
DLL_PUBLIC ND var field2(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);
// moved to mvprogram to allow custom conversions like "[DATE]"
// DLL_PUBLIC var oconv(CVR instring, const char* conversion);
// DLL_PUBLIC var oconv(CVR instring, CVR conversion);
// DLL_PUBLIC var iconv(CVR instring, const char* conversion);
// DLL_PUBLIC var iconv(CVR instring, CVR conversion);
DLL_PUBLIC bool connect(CVR connectionstring DEFAULTNULL);
DLL_PUBLIC void disconnect();
DLL_PUBLIC void disconnectall();
DLL_PUBLIC ND var lasterror();

DLL_PUBLIC bool dbcreate(CVR dbname);
DLL_PUBLIC ND var dblist();
DLL_PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname);
DLL_PUBLIC bool dbdelete(CVR dbname);

DLL_PUBLIC bool createfile(CVR filename);
DLL_PUBLIC bool deletefile(CVR filename_or_handle);
DLL_PUBLIC bool clearfile(CVR filename_or_handle);
DLL_PUBLIC bool renamefile(CVR filename, CVR newfilename);
DLL_PUBLIC ND var listfiles();
DLL_PUBLIC ND var reccount(CVR filename_or_handle);
DLL_PUBLIC bool createindex(CVR filename_or_handle, CVR fieldname DEFAULTNULL, CVR dictfilename DEFAULTNULL);
DLL_PUBLIC bool deleteindex(CVR filename_or_handle, CVR fieldname DEFAULTNULL);
DLL_PUBLIC ND var listindexes(CVR filename DEFAULTNULL, CVR fieldname DEFAULTNULL);
DLL_PUBLIC bool begintrans();
DLL_PUBLIC bool statustrans();
DLL_PUBLIC bool rollbacktrans();
DLL_PUBLIC bool committrans();
DLL_PUBLIC bool lock(CVR filehandle, CVR key);
DLL_PUBLIC void unlock(CVR filehandle, CVR key);
DLL_PUBLIC void unlockall();
DLL_PUBLIC bool open(CVR filename, VARREF filehandle);
DLL_PUBLIC bool open(CVR filename);
// DLL_PUBLIC bool open(CVR dictdata, CVR filename, VARREF filehandle);
DLL_PUBLIC bool read(VARREF record, CVR filehandle, CVR key);
DLL_PUBLIC bool reado(VARREF record, CVR filehandle, CVR key);
DLL_PUBLIC bool matread(dim& dimrecord, CVR filehandle, CVR key);
DLL_PUBLIC bool readv(VARREF record, CVR filehandle, CVR key, CVR fieldnumber);
DLL_PUBLIC bool write(CVR record, CVR filehandle, CVR key);
DLL_PUBLIC bool matwrite(const dim& dimrecord, CVR filehandle, CVR key);
DLL_PUBLIC bool writev(CVR record, CVR filehandle, CVR key, const int fieldno);
DLL_PUBLIC bool updaterecord(CVR record, CVR filehandle, CVR key);
DLL_PUBLIC bool insertrecord(CVR record, CVR filehandle, CVR key);

// moved to mvprogram so they have access to default cursor in mv.CURSOR
// DLL_PUBLIC bool select(CVR sortselectclause DEFAULTNULL);
// DLL_PUBLIC void clearselect();
// DLL_PUBLIC bool readnext(VARREF key);
// DLL_PUBLIC bool readnext(VARREF key, VARREF valueno);
// DLL_PUBLIC bool readnext(VARREF record, VARREF key, VARREF value);
// DLL_PUBLIC bool deleterecord(CVR filename_or_handle_or_command, CVR key DEFAULTNULL);

DLL_PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, const char* mode);
DLL_PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, CVR mode);
DLL_PUBLIC var substr2(CVR fromstr, VARREF startx, VARREF delimiterno);

DLL_PUBLIC var split(CVR sourcevar, dim& destinationdim);
DLL_PUBLIC ND dim split(CVR sourcevar);
DLL_PUBLIC ND var join(const dim& sourcedim);

DLL_PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
DLL_PUBLIC ND var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement);
DLL_PUBLIC ND var pickreplace(CVR instring, const int fieldno, CVR replacement);

DLL_PUBLIC ND var extract(CVR instring, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0);

DLL_PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
DLL_PUBLIC ND var insert(CVR instring, const int fieldno, const int valueno, CVR insertion);
DLL_PUBLIC ND var insert(CVR instring, const int fieldno, CVR insertion);

// DLL_PUBLIC var erase(CVR instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
DLL_PUBLIC ND var remove(CVR instring, const int fieldno, const int valueno = 0, const int subvalueno = 0);

DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, CVR replacement);
DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, CVR replacement);

DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, CVR insertion);
DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, CVR insertion);

// DLL_PUBLIC VARREF eraser(VARREF instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
DLL_PUBLIC VARREF remover(VARREF instring, const int fieldno, const int valueno = 0, const int subvalueno = 0);

// locate & locateby without fieldno or valueno arguments uses character VM as separator character
// locate & locateby with fieldno=0 uses character FM
// locate & locateby with fieldno>0 uses character VM
// locate & locateby with valueno>0 uses character SM

DLL_PUBLIC ND bool locate(CVR target, CVR instring);
DLL_PUBLIC bool locate(CVR target, CVR instring, VARREF setting);
DLL_PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

DLL_PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting);
DLL_PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

DLL_PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting);
DLL_PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0);

DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring);
DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting);
DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno = 0, const int subvalueno = 0);

DLL_PUBLIC ND var sum(CVR instring, CVR sepchar);
DLL_PUBLIC ND var sum(CVR instring);

// std::cout
////////////

//use like this
// 1. print("hi","ho")					// "hi ho"
// 2. static char const sep[] = ", ";	// "hi, ho"
//    print<sep>("hi","ho");

// print(...) default sep is " ". No flush
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void print(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
}

// printl() no arguments and prints end of line.Flush stdout
DLL_PUBLIC void printl() {
	LOCKIOSTREAM
	std::cout << std::endl;
}

// printl(...) default sep is " ". always prints end of line. Flush
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void printl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

// printt() no arguments just prints a sep (default tab). No flush
template <auto sep = ' '>
DLL_PUBLIC void printt() {
	LOCKIOSTREAM
	std::cout << sep;
}

// printt(...) default sep is tab. always adds a sep (tab) on the end. No flush
template <auto sep = '\t', typename... Printable>
DLL_PUBLIC void printt(const Printable&... values) {
	LOCKIOSTREAM
	((std::cout << values << sep), ...);
}

//output
////////

// output() BINARY TRANSPARENT version of print(). No automatic separators. No flush
template <typename... Printable>
DLL_PUBLIC void output(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
}

// outputl() BINARY TRANSPARENT version of printl(). No automatic separators. No flush
template <typename... Printable>
DLL_PUBLIC void outputl(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
	var("").outputl();
}

// std::cerr
////////////

// errput(...)
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void errput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
}

// errputl()
DLL_PUBLIC void errputl() {
	LOCKIOSTREAM
	std::cerr << std::endl;
}

// errputl(...)
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void errputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
	std::cerr << std::endl;
}

// std::clog
////////////

// logput(...)
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void logput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::clog << value;
	((std::clog << sep << values), ...);
}

// logputl()
DLL_PUBLIC void logputl() {
	LOCKIOSTREAM
	std::clog << std::endl;
}

// logputl(...)
template <auto sep = ' ', typename Printable, typename... Additional>
DLL_PUBLIC void logputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::clog << value;
	((std::clog << sep << values), ...);
	std::clog << std::endl;
}

/*#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "==");
*/

}  // namespace exodus

#endif	// EXODUSFUNCS_H
