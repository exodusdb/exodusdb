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
namespace exodus {

// Removed to reduce compile time of exodus programs.
// Use .output() .errput() and .logput() for threadsafe output.
// print() errput() logput() to output all arguments together in a thread-safe manner
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM

PUBLIC int exodus_main(int exodus__argc, const char* exodus__argv[], MvEnvironment& mv, int threadno);

PUBLIC ND var osgetenv(CVR name = "");
PUBLIC bool osgetenv(CVR name, VARREF value);
PUBLIC bool ossetenv(CVR name, CVR value);

PUBLIC ND var ostempdirpath();
PUBLIC ND var ostempfilename();

PUBLIC ND bool assigned(CVR var2);
PUBLIC ND bool unassigned(CVR var2);

PUBLIC void transfer(VARREF fromvar, VARREF tovar);
PUBLIC void exchange(VARREF var1, VARREF var2);

PUBLIC ND var date();
PUBLIC ND var time();
PUBLIC ND var timedate();

PUBLIC void ossleep(const int milliseconds);
PUBLIC var oswait(const int milliseconds, CVR dirpath);

PUBLIC ND var ostime();

PUBLIC void breakon();
PUBLIC void breakoff();

// Read/write osfile at specified offset. Must open/close.
PUBLIC bool osopen(CVR osfilename, VARREF osfilevar, const char* locale DEFAULT_EMPTY);
PUBLIC void osclose(CVR osfilevar);
PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset);
PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length);
PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset);

// Read/Write whole osfile
PUBLIC bool oswrite(CVR data, CVR osfilename, const char* codepage DEFAULT_EMPTY);
PUBLIC bool osread(VARREF data, CVR osfilename, const char* codepage DEFAULT_EMPTY);
// Simple version without codepage returns the contents or "" if file cannot be read
PUBLIC ND var osread(CVR osfilename);

PUBLIC bool osremove(CVR osfilename);
PUBLIC bool osrename(CVR oldosdir_or_filename, CVR newosdir_or_filename);
PUBLIC bool oscopy(CVR fromosdir_or_filename, CVR newosdir_or_filename);
PUBLIC bool osmove(CVR fromosdir_or_filename, CVR newosdir_or_filename);

PUBLIC ND var oslist(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY, const int mode = 0);
PUBLIC ND var oslistf(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY);
PUBLIC ND var oslistd(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY);

PUBLIC ND var osfile(CVR filename);
PUBLIC ND var osdir(CVR filename);

PUBLIC bool osmkdir(CVR dirname);
PUBLIC bool osrmdir(CVR dirname, const bool evenifnotempty = false);

PUBLIC ND var oscwd();
PUBLIC var oscwd(CVR dirname);

PUBLIC void osflush();
PUBLIC ND var ospid();

PUBLIC bool osshell(CVR command);
PUBLIC var osshellread(CVR command);
PUBLIC bool osshellread(VARREF readstr, CVR command);
PUBLIC bool osshellwrite(CVR writestr, CVR command);

PUBLIC void stop(CVR text DEFAULT_EMPTY);
PUBLIC void abort(CVR text DEFAULT_EMPTY);	 // dont confuse with abort() which is standard c/c++
PUBLIC void abortall(CVR text DEFAULT_EMPTY);
PUBLIC var logoff();

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
PUBLIC void initrnd(CVR seed = 0);

PUBLIC ND var mod(CVR dividend, CVR divisor);
PUBLIC ND var mod(CVR dividend, const double divisor);
PUBLIC ND var mod(CVR dividend, const int divisor);

PUBLIC ND var at(const int columnorcode);
PUBLIC ND var at(CVR column, CVR row);

// Moved to mvprogram
//PUBLIC ND var getcursor();
//PUBLIC void setcursor(CVR cursor);

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

PUBLIC ND bool isterminal();
PUBLIC ND bool hasinput(const int millisecs = 0);
PUBLIC ND bool eof();
PUBLIC bool echo(const int on_off);

PUBLIC ND var len(CVR var2);
PUBLIC ND var length(CVR var2);

PUBLIC VARREF converter(VARREF iostring, CVR oldchars, CVR newchars);
PUBLIC ND var convert(CVR instring, CVR oldchars, CVR newchars);

PUBLIC VARREF textconverter(VARREF iostring, CVR oldchars, CVR newchars);
PUBLIC ND var textconvert(CVR instring, CVR oldchars, CVR newchars);

PUBLIC VARREF swapper(VARREF iostring, CVR oldstr, CVR newstr);
PUBLIC ND var swap(CVR instring, CVR oldstr, CVR newstr);

PUBLIC VARREF regex_replacer(VARREF iostring, CVR oldstr, CVR newstr, CVR options DEFAULT_EMPTY);
PUBLIC ND var regex_replace(CVR instring, CVR oldstr, CVR newstr, CVR options DEFAULT_EMPTY);

PUBLIC VARREF ucaser(VARREF iostring);
PUBLIC ND var ucase(CVR instring);

PUBLIC VARREF lcaser(VARREF iostring);
PUBLIC ND var lcase(CVR instring);

PUBLIC VARREF tcaser(VARREF iostring);
PUBLIC ND var tcase(CVR instring);

PUBLIC VARREF fcaser(VARREF iostring);
PUBLIC ND var fcase(CVR instring);

PUBLIC VARREF normalizer(VARREF iostring);
PUBLIC ND var normalize(CVR instring);

PUBLIC VARREF uniquer(VARREF iostring);
PUBLIC ND var unique(CVR instring);

PUBLIC VARREF inverter(VARREF iostring);
PUBLIC ND var invert(CVR instring);
PUBLIC ND var invert(var&& instring);
//PUBLIC VARREF inverter(VARREF iostring) {return instring.inverter();};
//PUBLIC ND var invert(CVR instring) {return var(instring).inverter();};
//PUBLIC ND var invert(var&& instring) {return instring.inverter();};
//template<typename T> PUBLIC T invert(var && instring) {return T (std::forward<var>(instring));}

PUBLIC VARREF lowerer(VARREF iostring);
PUBLIC ND var lower(CVR instring);

PUBLIC VARREF raiser(VARREF iostring);
PUBLIC ND var raise(CVR instring);

PUBLIC VARREF splicer(VARREF iostring, const int start1, const int length, CVR str);
PUBLIC ND var splice(CVR instring, const int start1, const int length, CVR str);

//length omitted
PUBLIC VARREF splicer(VARREF iostring, const int start1, CVR str);
PUBLIC ND var splice(CVR instring, const int start1, CVR str);

PUBLIC VARREF popper(VARREF iostring);
PUBLIC ND var pop(CVR instring);

PUBLIC VARREF quoter(VARREF iostring);
PUBLIC ND var quote(CVR instring);

PUBLIC VARREF squoter(VARREF iostring);
PUBLIC ND var squote(CVR instring);

PUBLIC VARREF unquoter(VARREF iostring);
PUBLIC ND var unquote(CVR instring);

PUBLIC VARREF fieldstorer(VARREF iostring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);
PUBLIC ND var fieldstore(CVR instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement);


PUBLIC VARREF trimmer(VARREF iostring, CVR trimchars, CVR options);
PUBLIC ND var trim(CVR instring, CVR trimchars, CVR options);


PUBLIC VARREF trimmer(VARREF iostring, const char* trimchars DEFAULT_SPACE);
PUBLIC ND var trim(CVR instring, const char* trimchars DEFAULT_SPACE);

PUBLIC VARREF trimmerf(VARREF iostring, const char* trimchars DEFAULT_SPACE);
PUBLIC ND var trimf(CVR instring, const char* trimchars DEFAULT_SPACE);

PUBLIC VARREF trimmerb(VARREF iostring, const char* trimchars DEFAULT_SPACE);
PUBLIC ND var trimb(CVR instring, const char* trimchars DEFAULT_SPACE);


//PUBLIC VARREF trimmer(VARREF iostring, CVR trimchars);
//PUBLIC ND var trim(CVR instring, CVR trimchars);
//
//PUBLIC VARREF trimmerf(VARREF iostring, CVR trimchars);
//PUBLIC ND var trimf(CVR instring, CVR trimchars);
//
//PUBLIC VARREF trimmerb(VARREF iostring, CVR trimchars);
//PUBLIC ND var trimb(CVR instring, CVR trimchars);


PUBLIC VARREF cropper(VARREF iostring);
PUBLIC ND var crop(CVR instring);

PUBLIC VARREF sorter(VARREF iostring, SV sepchar = _FM_);
PUBLIC ND var sort(CVR instring, SV sepchar = _FM_);

PUBLIC ND var chr(CVR integer);
PUBLIC ND var chr(const int integer);
PUBLIC ND var textchr(CVR integer);
PUBLIC ND var textchr(const int integer);
PUBLIC ND var match(CVR instring, CVR matchstr, CVR options DEFAULT_EMPTY);
PUBLIC ND var seq(CVR char1);
PUBLIC ND var textseq(CVR char1);
PUBLIC ND var str(CVR instring, const int number);
PUBLIC ND var space(const int number);
PUBLIC ND var dcount(CVR instring, CVR substrx);
PUBLIC ND var count(CVR instring, CVR substrx);

PUBLIC ND var substr(CVR instring, const int startx);
PUBLIC ND var substr(CVR instring, const int startx, const int length);
PUBLIC VARREF substrer(VARREF iostring, const int startx);
PUBLIC VARREF substrer(VARREF iostring, const int startx, const int length);

PUBLIC ND var index(CVR instring, CVR substr, const int occurrenceno = 1);
PUBLIC ND var index2(CVR instring, CVR substr, const int startcharno = 1);

PUBLIC ND var field(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);
PUBLIC ND var field2(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx = 1);

// moved to mvprogram to allow custom conversions like "[DATE]"
// PUBLIC var oconv(CVR instring, const char* conversion);
// PUBLIC var oconv(CVR instring, CVR conversion);
// PUBLIC var iconv(CVR instring, const char* conversion);
// PUBLIC var iconv(CVR instring, CVR conversion);

PUBLIC bool connect(CVR connectionstring DEFAULT_EMPTY);
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

PUBLIC bool createindex(CVR filename_or_handle, CVR fieldname DEFAULT_EMPTY, CVR dictfilename DEFAULT_EMPTY);
PUBLIC bool deleteindex(CVR filename_or_handle, CVR fieldname DEFAULT_EMPTY);
PUBLIC ND var listindexes(CVR filename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY);

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
PUBLIC bool readv(VARREF record, CVR filehandle, CVR key, CVR fieldnumber);

PUBLIC bool write(CVR record, CVR filehandle, CVR key);
PUBLIC bool writeo(CVR record, CVR filehandle, CVR key);
PUBLIC bool writev(CVR record, CVR filehandle, CVR key, const int fieldno);
PUBLIC bool updaterecord(CVR record, CVR filehandle, CVR key);
PUBLIC bool insertrecord(CVR record, CVR filehandle, CVR key);

PUBLIC bool dimread(dim& dimrecord, CVR filehandle, CVR key);
PUBLIC bool dimwrite(const dim& dimrecord, CVR filehandle, CVR key);

// moved to mvprogram so they have access to default cursor in mv.CURSOR
// PUBLIC bool select(CVR sortselectclause DEFAULT_EMPTY);
// PUBLIC void clearselect();
// PUBLIC bool readnext(VARREF key);
// PUBLIC bool readnext(VARREF key, VARREF valueno);
// PUBLIC bool readnext(VARREF record, VARREF key, VARREF value);
// PUBLIC bool deleterecord(CVR filename_or_handle_or_command, CVR key DEFAULT_EMPTY);

PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, const char* mode);
PUBLIC ND var xlate(CVR filename, CVR key, CVR fieldno, CVR mode);
PUBLIC var substr2(CVR fromstr, VARREF startx, VARREF delimiterno);

PUBLIC ND dim split(CVR sourcevar, SV sepchar = _FM_);
PUBLIC ND var join(const dim& sourcedim, SV sepchar = _FM_);

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

PUBLIC ND var sum(CVR instring, SV sepchar);
PUBLIC ND var sum(CVR instring);


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
	var(EXPRESSION).convert(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "==");
*/

}  // namespace exodus

#endif	// EXODUSFUNCS_H
