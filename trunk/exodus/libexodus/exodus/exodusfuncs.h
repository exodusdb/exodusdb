/*
Copyright (c) 2009 Stephen John Bush

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
#       define DEFAULTNULL
#       define DEFAULTDOT
#       define DEFAULTSPACE
#       define DEFAULTVM
#else
#       define DEFAULTNULL =L""
#       define DEFAULTDOT =L"."
#       define DEFAULTSPACE =L" "
#       define DEFAULTVM =VM_
#endif

//add global function type syntax to the exodus users
namespace exodus
{

DLL_PUBLIC int exodus_main(int exodus__argc, char* exodus__argv[], MvEnvironment& mv);

DLL_PUBLIC var osgetenv(const var& name);
DLL_PUBLIC bool osgetenv(const var& name, var& value);
DLL_PUBLIC bool ossetenv(const var& name, const var& value);
DLL_PUBLIC bool assigned(const var& var2);
DLL_PUBLIC bool unassigned(const var& var2);
DLL_PUBLIC var& transfer(var& fromvar, var& tovar);
DLL_PUBLIC var& exchange(var& var1, var& var2);
DLL_PUBLIC var date();
DLL_PUBLIC var time();
DLL_PUBLIC var timedate();
DLL_PUBLIC void ossleep(const int milliseconds);
DLL_PUBLIC var ostime();
DLL_PUBLIC bool osopen(const var& osfilename, var& osfilevar, const var& locale DEFAULTNULL);
DLL_PUBLIC void osclose(const var& osfilevar);
DLL_PUBLIC var osbread(const var& osfilevar, var& startoffset, const int length);
DLL_PUBLIC bool osbread(var& data, const var& osfilevar, var& startoffset, const int length);
DLL_PUBLIC bool osbwrite(const var& data, const var& osfilevar, var& startoffset);

DLL_PUBLIC bool osread(var& data, const var& osfilename, const var& locale DEFAULTNULL);
DLL_PUBLIC var osread(const var& osfilename);
DLL_PUBLIC bool oswrite(const var& data,const var& osfilename, const var& locale DEFAULTNULL);

DLL_PUBLIC bool osdelete(const var& osfilename);
DLL_PUBLIC bool osrename(const var& oldosdir_or_filename, const var& newosdir_or_filename);
DLL_PUBLIC bool oscopy(const var& fromosdir_or_filename, const var& newosdir_or_filename);
DLL_PUBLIC var oslist(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL, const int mode=0);
DLL_PUBLIC var oslistf(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL);
DLL_PUBLIC var oslistd(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL);
DLL_PUBLIC var osfile(const var& filename);
DLL_PUBLIC var osdir(const var& filename);
DLL_PUBLIC bool osmkdir(const var& dirname);
DLL_PUBLIC bool osrmdir(const var& dirname, const bool evenifnotempty=false);
DLL_PUBLIC var oscwd();
DLL_PUBLIC var oscwd(const var& dirname);
DLL_PUBLIC void osflush();
DLL_PUBLIC var suspend(const var& command);
DLL_PUBLIC var osshell(const var& command);
DLL_PUBLIC var osshellread(const var& command);
DLL_PUBLIC void osshellread(var& readstr, const var& command);
DLL_PUBLIC void osshellwrite(const var& writestr, const var& command);
DLL_PUBLIC void stop(const var& text DEFAULTNULL);
DLL_PUBLIC void abort(const var& text);//dont confuse with abort() which is standard c/c++
DLL_PUBLIC var perform(const var& command);
DLL_PUBLIC var execute(const var& command);
DLL_PUBLIC var chain(const var& command);
DLL_PUBLIC var logoff();
DLL_PUBLIC void debug();

DLL_PUBLIC bool setxlocale(const var& locale);
DLL_PUBLIC var getxlocale();

void print(const var& var2);
void printl(const var& var2 DEFAULTNULL);
void printt(const var& var2 DEFAULTNULL);

	//MATH/BOOLEAN
DLL_PUBLIC var abs(const var& num1);
DLL_PUBLIC var pwr(const var& base, const var& exponent);
DLL_PUBLIC var exp(const var& power);
DLL_PUBLIC var sqrt(const var& num1);
DLL_PUBLIC var sin(const var& degrees);
DLL_PUBLIC var cos(const var& degrees);
DLL_PUBLIC var tan(const var& degrees);
DLL_PUBLIC var atan(const var& degrees);
DLL_PUBLIC var loge(const var& num1);
//integer() represents pick int() because int() is reserved word in c/c++
//Note that integer like pick int() is the same as floor()
//whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional decimal places)
//to get the usual c/c++ effect use toInt() (although toInt() returns an int instead of a var like normal exodus functions)
DLL_PUBLIC var integer(const var& num1);
DLL_PUBLIC var floor(const var& num1);
DLL_PUBLIC var round(const var& num1, const int ndecimals=0);

DLL_PUBLIC var rnd(const int number);
DLL_PUBLIC void initrnd(const int seednumber);
DLL_PUBLIC var mod(const var& dividend, const int divisor);
DLL_PUBLIC var mod(const var& dividend, const var& divisor);

DLL_PUBLIC var at(const int columnorcode);
DLL_PUBLIC var at(const var& column, const var& row);
DLL_PUBLIC var getcursor();
DLL_PUBLIC void setcursor(const var& cursor);
DLL_PUBLIC var getprompt();
DLL_PUBLIC void setprompt(const var& prompt);

DLL_PUBLIC var input();
DLL_PUBLIC void input(var& intostr);
DLL_PUBLIC void input(const var& prompt, var& intostr);
DLL_PUBLIC void inputn(var& intostr, const int nchars);

DLL_PUBLIC var len(const var& var2);
DLL_PUBLIC var length(const var& var2);
DLL_PUBLIC var& converter(var& instring, const var& oldchars, const var& newchars);
DLL_PUBLIC var convert(const var& instring, const var& oldchars, const var& newchars);
DLL_PUBLIC var& swapper(var& instring, const var& oldstr, const var& newstr, const var& options DEFAULTNULL);
DLL_PUBLIC var swap(const var& instring, const var& oldstr, const var& newstr, const var& options DEFAULTNULL);
DLL_PUBLIC var& ucaser(var& instring);
DLL_PUBLIC var ucase(const var& instring);
DLL_PUBLIC var& lcaser(var& instring);
DLL_PUBLIC var lcase(const var& instring);
//arev character bit swapper
//changing all the bits of exodus' 2 or 4 byte characters might make illegal unicode characters
//could be implemented as xor xff ie bottom eight bits only and leave top bits unchanged
//this would shuffle every unicode page but only *within* the page
//so producing undefined but not illegal unicode bytes
//DLL_PUBLIC var& inverter(var& instring);
//DLL_PUBLIC var invert(const var& instring);
DLL_PUBLIC var& lowerer(var& instring);
DLL_PUBLIC var lower(const var& instring);
DLL_PUBLIC var& raiser(var& instring);
DLL_PUBLIC var raise(const var& instring);
DLL_PUBLIC var& splicer(var& instring, const int start1,const int length,const var& str);
DLL_PUBLIC var splice(const var& instring, const int start1,const int length,const var& str);
DLL_PUBLIC var& quoter(var& instring);
DLL_PUBLIC var quote(const var& instring);
DLL_PUBLIC var& unquoter(var& instring);
DLL_PUBLIC var unquote(const var& instring);
DLL_PUBLIC var& fieldstorer(var& instring, const var& sepchar,const int fieldno, const int nfields,const var& replacement);
DLL_PUBLIC var fieldstore(const var& instring, const var& sepchar,const int fieldno, const int nfields,const var& replacement);
DLL_PUBLIC var& trimmer(var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var trim(const var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var& trimmerf(var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var trimf(const var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var& trimmerb(var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var trimb(const var& instring, const wchar_t* trimchars DEFAULTSPACE);
DLL_PUBLIC var& trimmer(var& instring, const var trimchars);
DLL_PUBLIC var trim(const var& instring, const var trimchars);
DLL_PUBLIC var& trimmerf(var& instring, const var trimchars);
DLL_PUBLIC var trimf(const var& instring, const var trimchars);
DLL_PUBLIC var& trimmerb(var& instring, const var trimchars);
DLL_PUBLIC var trimb(const var& instring, const var trimchars);
DLL_PUBLIC var crop(const var& instring);
DLL_PUBLIC var cropper(var& instring);
DLL_PUBLIC var chr(const var& integer);
DLL_PUBLIC var chr(const int integer);
DLL_PUBLIC bool match(const var& instring, const var& matchstr, const var& options DEFAULTNULL);
DLL_PUBLIC var seq(const var& char1);
DLL_PUBLIC var str(const var& instring, const int number);
DLL_PUBLIC var space(const int number);
DLL_PUBLIC var dcount(const var& instring, const var& substrx);
DLL_PUBLIC var count(const var& instring, const var& substrx);
DLL_PUBLIC var substr(const var& instring, const int startx);
DLL_PUBLIC var substr(const var& instring, const int startx,const int length);
DLL_PUBLIC var substrer(var& instring, const int startx);
DLL_PUBLIC var substrer(var& instring, const int startx,const int length);
DLL_PUBLIC var index(const var& instring, const var& substr,const int occurrenceno=1);
DLL_PUBLIC var index2(const var& instring, const var& substr,const int startcharno=1);
DLL_PUBLIC var field(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx=1);
DLL_PUBLIC var field2(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx=1);
DLL_PUBLIC var oconv(const var& instring, const wchar_t* conversion);
DLL_PUBLIC var oconv(const var& instring, const var& conversion);
DLL_PUBLIC var iconv(const var& instring, const wchar_t* conversion);
DLL_PUBLIC var iconv(const var& instring, const var& conversion);
DLL_PUBLIC bool connect(const var& connectionstring DEFAULTNULL);
DLL_PUBLIC bool disconnect();
DLL_PUBLIC bool createdb(const var& dbname);
DLL_PUBLIC bool deletedb(const var& dbname);
DLL_PUBLIC bool createdb(const var& dbname, var& errmsg);
DLL_PUBLIC bool deletedb(const var& dbname, var& errmsg);
DLL_PUBLIC bool createfile(const var& filename);
DLL_PUBLIC bool deletefile(const var& filename);
DLL_PUBLIC bool clearfile(const var& filename);
DLL_PUBLIC var listfiles();
DLL_PUBLIC bool createindex(const var& filename, const var& fieldname, const var& dictfilename DEFAULTNULL);
DLL_PUBLIC bool deleteindex(const var& filename, const var& fieldname);
DLL_PUBLIC var listindexes(const var& filename DEFAULTNULL);
DLL_PUBLIC bool begintrans();
DLL_PUBLIC bool rollbacktrans();
DLL_PUBLIC bool committrans();
DLL_PUBLIC bool lock(const var& filehandle, const var& key);
DLL_PUBLIC void unlock(const var& filehandle, const var& key);
DLL_PUBLIC void unlockall();
DLL_PUBLIC bool open(const var& filename, var& filehandle);
DLL_PUBLIC bool open(const var& dictdata, const var& filename, var& filehandle);
DLL_PUBLIC bool read(var& record, const var& filehandle, const var& key);
DLL_PUBLIC bool matread(dim& dimrecord, const var& filehandle, const var& key);
DLL_PUBLIC bool readv(var& record, const var& filehandle, const var& key, const var& fieldnumber);
DLL_PUBLIC bool write(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool matwrite(const dim& dimrecord, const var& filehandle,const var& key);
DLL_PUBLIC bool writev(const var& record, const var& filehandle,const var& key,const int fieldno);
DLL_PUBLIC bool deleterecord(const var& filehandle, const var& key);
DLL_PUBLIC bool updaterecord(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool insertrecord(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool select(const var& sortselectclause DEFAULTNULL);
DLL_PUBLIC void clearselect();
DLL_PUBLIC bool readnext(var& key);
DLL_PUBLIC bool readnext(var& key, var& valueno);
DLL_PUBLIC bool selectrecord(const var& sortselectclause DEFAULTNULL);
DLL_PUBLIC bool readnextrecord(var& record, var& key);
/* done in ExodusProgramBase and MvEnvironment now
DLL_PUBLIC var calculate(const var& fieldname);
*/
DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const wchar_t* mode);
DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const var& mode);
DLL_PUBLIC var remove(const var& fromstr, var& startx, var& delimiterno);

DLL_PUBLIC var matparse(const var& dynarray, dim& intodimarray);
DLL_PUBLIC var matunparse(const dim& dimarray);

DLL_PUBLIC var replace(const var& instring, const int fieldno, const int valueno, const int subvalueno, const var& replacement);
DLL_PUBLIC var replace(const var& instring, const int fieldno, const int valueno, const var& replacement);
DLL_PUBLIC var replace(const var& instring, const int fieldno, const var& replacement);

DLL_PUBLIC var extract(const var& instring, const int fieldno=0, const int valueno=0, const int subvalueno=0);

DLL_PUBLIC var insert(const var& instring, const int fieldno, const int valueno, const int subvalueno, const var& insertion);
DLL_PUBLIC var insert(const var& instring, const int fieldno, const int valueno, const var& insertion);
DLL_PUBLIC var insert(const var& instring, const int fieldno, const var& insertion);

DLL_PUBLIC var erase(const var& instring, const int fieldno, const int valueno=0, const int subvalueno=0);

DLL_PUBLIC var& replacer(var& instring, const int fieldno, const int valueno, const int subvalueno, const var& replacement);
DLL_PUBLIC var& replacer(var& instring, const int fieldno, const int valueno, const var& replacement);
DLL_PUBLIC var& replacer(var& instring, const int fieldno, const var& replacement);

DLL_PUBLIC var& inserter(var& instring, const int fieldno, const int valueno, const int subvalueno, const var& insertion);
DLL_PUBLIC var& inserter(var& instring, const int fieldno, const int valueno, const var& insertion);
DLL_PUBLIC var& inserter(var& instring, const int fieldno, const var& insertion);

DLL_PUBLIC var& eraser(var& instring, const int fieldno, const int valueno=0, const int subvalueno=0);
DLL_PUBLIC bool locate(const var& instring, const var& target, var& setting, const int fieldno=0,const int valueno=0);
DLL_PUBLIC bool locateby(const var& instring, const var& target, const wchar_t* ordercode, var& setting, const int fieldno=0,const int valueno=0);
DLL_PUBLIC bool locateby(const var& instring, const var& target, const var& ordercode, var& setting, const int fieldno=0,const int valueno=0);
DLL_PUBLIC bool locateusing(const var& instring, const var& target, const var& usingchar, var& setting, const int fieldno=0, const int valueno=0, const int subvalueno=0);
DLL_PUBLIC bool locateusing(const var& instring, const var& target, const var& usingchar);
DLL_PUBLIC var sum(const var& instring, const var& sepchar);
DLL_PUBLIC var sum(const var& instring);

template<class T1, class T2>
void print(T1 arg1, T2 arg2)
{ print(arg1); print(arg2);}
template<class T1, class T2, class T3>
void print(T1 arg1, T2 arg2, T3 arg3)
{ print(arg1); print(arg2); print(arg3);}
template<class T1, class T2, class T3, class T4>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{ print(arg1); print(arg2); print(arg3); print(arg4);}
template<class T1, class T2, class T3, class T4, class T5>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5);}
template<class T1, class T2, class T3, class T4, class T5, class T6>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); print(arg9); }
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
void print(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); print(arg9); print(arg10);}

template<class T1, class T2>
void printt(T1 arg1, T2 arg2)
{ printt(arg1); printt(arg2);}
template<class T1, class T2, class T3>
void printt(T1 arg1, T2 arg2, T3 arg3)
{ printt(arg1); printt(arg2); printt(arg3);}
template<class T1, class T2, class T3, class T4>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4);}
template<class T1, class T2, class T3, class T4, class T5>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5);}
template<class T1, class T2, class T3, class T4, class T5, class T6>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5); printt(arg6);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5); printt(arg6); printt(arg7);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5); printt(arg6); printt(arg7); printt(arg8);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5); printt(arg6); printt(arg7); printt(arg8); printt(arg9); }
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
{ printt(arg1); printt(arg2); printt(arg3); printt(arg4); printt(arg5); printt(arg6); printt(arg7); printt(arg8); printt(arg9); printt(arg10);}

template<class T1, class T2>
void printl(T1 arg1, T2 arg2)
{ print(arg1); printl(arg2);}
template<class T1, class T2, class T3>
void printl(T1 arg1, T2 arg2, T3 arg3)
{ print(arg1); print(arg2); printl(arg3);}
template<class T1, class T2, class T3, class T4>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{ print(arg1); print(arg2); print(arg3); printl(arg4);}
template<class T1, class T2, class T3, class T4, class T5>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{ print(arg1); print(arg2); print(arg3); print(arg4); printl(arg5);}
template<class T1, class T2, class T3, class T4, class T5, class T6>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); printl(arg6);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); printl(arg7);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); printl(arg8);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); printl(arg9); }
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
void printl(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); print(arg9); printl(arg10);}

}//namespace exodus

#endif //EXODUSFUNCS_H
