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

#ifndef PUBLIC
#define PUBLIC
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

PUBLIC int exodus_main(int exodus__argc, const char* exodus__argv[], MvEnvironment& mv, int environmentno);

PUBLIC ND var osgetenv(CVR name = "");
PUBLIC bool osgetenv(CVR name, VARREF value);
PUBLIC bool ossetenv(CVR name, CVR value);
PUBLIC ND var ostempdirname();
PUBLIC ND var ostempfilename();
PUBLIC ND bool assigned(CVR var2);
PUBLIC ND bool unassigned(CVR var2);
PUBLIC VARREF transfer(VARREF fromvar, VARREF tovar);
PUBLIC VARREF exchange(VARREF var1, VARREF var2);
PUBLIC ND var date();
PUBLIC ND var time();
PUBLIC ND var timedate();
PUBLIC void ossleep(const int milliseconds);
PUBLIC ND var ostime();

PUBLIC void breakon();
PUBLIC void breakoff();

PUBLIC bool osopen(CVR osfilename, VARREF osfilevar, const char* locale DEFAULTNULL);
PUBLIC void osclose(CVR osfilevar);

//PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length,
//			const bool adjust = true);
//PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length,
//			const bool adjust = true);

//PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset,
//			 const bool adjust = true);
//PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset,
//			 const bool adjust = true);
PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset);
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset);
#endif
PUBLIC bool oswrite(CVR data, CVR osfilename, const char* codepage DEFAULTNULL);
PUBLIC bool osread(VARREF data, CVR osfilename, const char* codepage DEFAULTNULL);
PUBLIC ND var osread(CVR osfilename);

PUBLIC bool osremove(CVR osfilename);
PUBLIC bool osrename(CVR oldosdir_or_filename, CVR newosdir_or_filename);
PUBLIC bool oscopy(CVR fromosdir_or_filename, CVR newosdir_or_filename);
PUBLIC bool osmove(CVR fromosdir_or_filename, CVR newosdir_or_filename);

PUBLIC ND var oslist(CVR path DEFAULTDOT, CVR globpattern DEFAULTNULL, const int mode = 0);
PUBLIC ND var oslistf(CVR path DEFAULTDOT, CVR globpattern DEFAULTNULL);
PUBLIC ND var oslistd(CVR path DEFAULTDOT, CVR globpattern DEFAULTNULL);
PUBLIC ND var osfile(CVR filename);
PUBLIC ND var osdir(CVR filename);
PUBLIC bool osmkdir(CVR dirname);
PUBLIC bool osrmdir(CVR dirname, const bool evenifnotempty = false);
PUBLIC ND var oscwd();
PUBLIC var oscwd(CVR dirname);
PUBLIC void osflush();
PUBLIC ND var ospid();
PUBLIC var suspend(CVR command);
PUBLIC bool osshell(CVR command);
PUBLIC var osshellread(CVR command);
PUBLIC bool osshellread(VARREF readstr, CVR command);
PUBLIC bool osshellwrite(CVR writestr, CVR command);
PUBLIC void stop(CVR text DEFAULTNULL);
PUBLIC void abort(CVR text DEFAULTNULL);	 // dont confuse with abort() which is standard c/c++
PUBLIC void abortall(CVR text DEFAULTNULL);
PUBLIC var execute(CVR command);
// PUBLIC var chain(CVR command);
PUBLIC var logoff();
PUBLIC void debug();

PUBLIC bool setxlocale(const char* locale);
PUBLIC ND var getxlocale();

//replace by templates with variable number of arguments
//void print(CVR var2);
//void printl(CVR var2 DEFAULTNULL);
//void printt(CVR var2 DEFAULTNULL);

// MATH/BOOLEAN
PUBLIC ND var abs(CVR num1);
PUBLIC ND var pwr(CVR base, CVR exponent);
PUBLIC ND var exp(CVR power);
PUBLIC ND var sqrt(CVR num1);
PUBLIC ND var sin(CVR degrees);
PUBLIC ND var cos(CVR degrees);
PUBLIC ND var tan(CVR degrees);
PUBLIC ND var atan(CVR degrees);
PUBLIC ND var loge(CVR num1);
// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
PUBLIC ND var integer(CVR num1);
PUBLIC ND var floor(CVR num1);
PUBLIC ND var round(CVR num1, const int ndecimals = 0);

PUBLIC ND var rnd(const int number);
PUBLIC void initrnd(CVR seed);
PUBLIC ND var mod(CVR dividend, const int divisor);
PUBLIC ND var mod(CVR dividend, CVR divisor);

PUBLIC ND var at(const int columnorcode);
PUBLIC ND var at(CVR column, CVR row);
PUBLIC ND var getcursor();
PUBLIC void setcursor(CVR cursor);
PUBLIC ND var getprompt();
PUBLIC void setprompt(CVR prompt);
PUBLIC bool echo(const int on_off);

//PUBLIC VARREF input();
//PUBLIC VARREF input(VARREF intostr);
//PUBLIC VARREF input(CVR prompt, VARREF intostr);
//PUBLIC VARREF inputn(VARREF intostr, const int nchars);
PUBLIC var input();
PUBLIC var input(CVR prompt);
PUBLIC var inputn(const int nchars);

PUBLIC ND var len(CVR var2);
PUBLIC ND var length(CVR var2);
PUBLIC VARREF converter(VARREF instring, CVR oldchars, CVR newchars);
PUBLIC ND var convert(CVR instring, CVR oldchars, CVR newchars);
PUBLIC VARREF textconverter(VARREF instring, CVR oldchars, CVR newchars);
PUBLIC ND var textconvert(CVR instring, CVR oldchars, CVR newchars);
PUBLIC VARREF swapper(VARREF instring, CVR oldstr, CVR newstr);
PUBLIC ND var swap(CVR instring, CVR oldstr, CVR newstr);
PUBLIC VARREF regex_replacer(VARREF instring, CVR oldstr, CVR newstr, CVR options DEFAULTNULL);
PUBLIC ND var regex_replace(CVR instring, CVR oldstr, CVR newstr, CVR options DEFAULTNULL);
PUBLIC VARREF ucaser(VARREF instring);
PUBLIC ND var ucase(CVR instring);
PUBLIC VARREF lcaser(VARREF instring);
PUBLIC ND var lcase(CVR instring);
PUBLIC VARREF tcaser(VARREF instring);
PUBLIC ND var fcase(CVR instring);
PUBLIC VARREF tcaser(VARREF instring);
PUBLIC ND var fcase(CVR instring);
PUBLIC ND var unique(CVR instring);
PUBLIC VARREF inverter(VARREF instring);
PUBLIC ND var invert(CVR instring);
PUBLIC VARREF lowerer(VARREF instring);
PUBLIC ND var lower(CVR instring);
PUBLIC VARREF raiser(VARREF instring);
PUBLIC ND var raise(CVR instring);
PUBLIC VARREF splicer(VARREF instring, const int start1, const int length, CVR str);
PUBLIC ND var splice(CVR instring, const int start1, const int length, CVR str);
PUBLIC VARREF popper(VARREF instring);
PUBLIC ND var pop(CVR instring);
PUBLIC VARREF quoter(VARREF instring);
PUBLIC ND var quote(CVR instring);
PUBLIC VARREF unquoter(VARREF instring);
PUBLIC ND var unquote(CVR instring);
PUBLIC VARREF fieldstorer(VARREF instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);
PUBLIC ND var fieldstore(CVR instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);
PUBLIC VARREF trimmer(VARREF instring, const char* trimchars DEFAULTSPACE);
PUBLIC VARREF trimmer(VARREF instring, CVR trimchars, CVR options);
PUBLIC ND var trim(CVR instring, const char* trimchars DEFAULTSPACE);
PUBLIC VARREF trimmerf(VARREF instring, const char* trimchars DEFAULTSPACE);
PUBLIC ND var trimf(CVR instring, const char* trimchars DEFAULTSPACE);
PUBLIC VARREF trimmerb(VARREF instring, const char* trimchars DEFAULTSPACE);
PUBLIC ND var trimb(CVR instring, const char* trimchars DEFAULTSPACE);
PUBLIC VARREF trimmer(VARREF instring, CVR trimchars);
PUBLIC ND var trim(CVR instring, CVR trimchars);
PUBLIC ND var trim(CVR instring, CVR trimchars, CVR options);
PUBLIC VARREF trimmerf(VARREF instring, CVR trimchars);
PUBLIC ND var trimf(CVR instring, CVR trimchars);
PUBLIC VARREF trimmerb(VARREF instring, CVR trimchars);
PUBLIC ND var trimb(CVR instring, CVR trimchars);
PUBLIC ND var crop(CVR instring);
PUBLIC var cropper(VARREF instring);
PUBLIC ND var chr(CVR integer);
PUBLIC ND var chr(const int integer);
PUBLIC ND var textchr(CVR integer);
PUBLIC ND var textchr(const int integer);
PUBLIC ND var match(CVR instring, CVR matchstr, CVR options DEFAULTNULL);
PUBLIC ND var seq(CVR char1);
PUBLIC ND var textseq(CVR char1);
PUBLIC ND var str(CVR instring, const int number);
PUBLIC ND var space(const int number);
PUBLIC ND var dcount(CVR instring, CVR substrx);
PUBLIC ND var count(CVR instring, CVR substrx);
PUBLIC ND var substr(CVR instring, const int startx);
PUBLIC ND var substr(CVR instring, const int startx, const int length);
PUBLIC var substrer(VARREF instring, const int startx);
PUBLIC var substrer(VARREF instring, const int startx, const int length);
PUBLIC ND var index(CVR instring, CVR substr, const int occurrenceno = 1);
PUBLIC ND var index2(CVR instring, CVR substr, const int startcharno = 1);
PUBLIC ND var field(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);
PUBLIC ND var field2(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);
// moved to mvprogram to allow custom conversions like "[DATE]"
// PUBLIC var oconv(CVR instring, const char* conversion);
// PUBLIC var oconv(CVR instring, CVR conversion);
// PUBLIC var iconv(CVR instring, const char* conversion);
// PUBLIC var iconv(CVR instring, CVR conversion);
PUBLIC bool connect(CVR connectionstring DEFAULTNULL);
PUBLIC void disconnect();
PUBLIC void disconnectall();
PUBLIC ND var lasterror();

PUBLIC bool dbcreate(CVR dbname);
PUBLIC ND var dblist();
PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname);
PUBLIC bool dbdelete(CVR dbname);

PUBLIC bool createfile(CVR filename);
PUBLIC bool deletefile(CVR filename_or_handle);
PUBLIC bool clearfile(CVR filename_or_handle);
PUBLIC bool renamefile(CVR filename, CVR newfilename);
PUBLIC ND var listfiles();
PUBLIC ND var reccount(CVR filename_or_handle);
PUBLIC bool createindex(CVR filename_or_handle, CVR fieldname DEFAULTNULL, CVR dictfilename DEFAULTNULL);
PUBLIC bool deleteindex(CVR filename_or_handle, CVR fieldname DEFAULTNULL);
PUBLIC ND var listindexes(CVR filename DEFAULTNULL, CVR fieldname DEFAULTNULL);
PUBLIC bool begintrans();
PUBLIC bool statustrans();
PUBLIC bool rollbacktrans();
PUBLIC bool committrans();
PUBLIC bool lock(CVR filehandle, CVR key);
PUBLIC void unlock(CVR filehandle, CVR key);
PUBLIC void unlockall();
PUBLIC bool open(CVR filename, VARREF filehandle);
PUBLIC bool open(CVR filename);
// PUBLIC bool open(CVR dictdata, CVR filename, VARREF filehandle);
PUBLIC bool read(VARREF record, CVR filehandle, CVR key);
PUBLIC bool reado(VARREF record, CVR filehandle, CVR key);
PUBLIC bool matread(dim& dimrecord, CVR filehandle, CVR key);
PUBLIC bool readv(VARREF record, CVR filehandle, CVR key, CVR fieldnumber);
PUBLIC bool write(CVR record, CVR filehandle, CVR key);
PUBLIC bool matwrite(const dim& dimrecord, CVR filehandle, CVR key);
PUBLIC bool writev(CVR record, CVR filehandle, CVR key, const int fieldno);
PUBLIC bool updaterecord(CVR record, CVR filehandle, CVR key);
PUBLIC bool insertrecord(CVR record, CVR filehandle, CVR key);

// moved to mvprogram so they have access to default cursor in mv.CURSOR
// PUBLIC bool select(CVR sortselectclause DEFAULTNULL);
// PUBLIC void clearselect();
// PUBLIC bool readnext(VARREF key);
// PUBLIC bool readnext(VARREF key, VARREF valueno);
// PUBLIC bool readnext(VARREF record, VARREF key, VARREF value);
// PUBLIC bool deleterecord(CVR filename_or_handle_or_command, CVR key DEFAULTNULL);

PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, const char* mode);
PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, CVR mode);
PUBLIC var substr2(CVR fromstr, VARREF startx, VARREF delimiterno);

PUBLIC var split(CVR sourcevar, dim& destinationdim);
PUBLIC ND dim split(CVR sourcevar);
PUBLIC ND var join(const dim& sourcedim);

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

PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement);
PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, CVR replacement);
PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, CVR replacement);

PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion);
PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, CVR insertion);
PUBLIC VARREF inserter(VARREF instring, const int fieldno, CVR insertion);

// PUBLIC VARREF eraser(VARREF instring, const int fieldno, const int valueno=0, const int
// subvalueno=0);
PUBLIC VARREF remover(VARREF instring, const int fieldno, const int valueno = 0, const int subvalueno = 0);

// locate & locateby without fieldno or valueno arguments uses character VM as separator character
// locate & locateby with fieldno=0 uses character FM
// locate & locateby with fieldno>0 uses character VM
// locate & locateby with valueno>0 uses character SM

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

PUBLIC ND var sum(CVR instring, CVR sepchar);
PUBLIC ND var sum(CVR instring);

// std::cout
////////////

//use like this
// 1. print("hi","ho")					// "hi ho"
// 2. static char const sep[] = ", ";	// "hi, ho"
//    print<sep>("hi","ho");

// print(...) default sep is " ". No flush
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void print(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
}

// printl() no arguments and prints end of line.Flush stdout
PUBLIC void printl() {
	LOCKIOSTREAM
	std::cout << std::endl;
}

// printl(...) default sep is " ". always prints end of line. Flush
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void printl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cout << value;
	((std::cout << sep << values), ...);
	std::cout << std::endl;
}

// printt() no arguments just prints a sep (default tab). No flush
template <auto sep = ' '>
PUBLIC void printt() {
	LOCKIOSTREAM
	std::cout << sep;
}

// printt(...) default sep is tab. always adds a sep (tab) on the end. No flush
template <auto sep = '\t', typename... Printable>
PUBLIC void printt(const Printable&... values) {
	LOCKIOSTREAM
	((std::cout << values << sep), ...);
}

//output
////////

// output() BINARY TRANSPARENT version of print(). No automatic separators. No flush
template <typename... Printable>
PUBLIC void output(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
}

// outputl() BINARY TRANSPARENT version of printl(). No automatic separators. No flush
template <typename... Printable>
PUBLIC void outputl(const Printable&... value) {
	LOCKIOSTREAM
	(var(value).output(), ...);
	var("").outputl();
}

// std::cerr
////////////

// errput(...)
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void errput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
}

// errputl()
PUBLIC void errputl() {
	LOCKIOSTREAM
	std::cerr << std::endl;
}

// errputl(...)
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void errputl(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::cerr << value;
	((std::cerr << sep << values), ...);
	std::cerr << std::endl;
}

// std::clog
////////////

// logput(...)
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void logput(const Printable& value, const Additional&... values) {
	LOCKIOSTREAM
	std::clog << value;
	((std::clog << sep << values), ...);
}

// logputl()
PUBLIC void logputl() {
	LOCKIOSTREAM
	std::clog << std::endl;
}

// logputl(...)
template <auto sep = ' ', typename Printable, typename... Additional>
PUBLIC void logputl(const Printable& value, const Additional&... values) {
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
