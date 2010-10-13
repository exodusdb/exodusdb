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

//add global function type syntax to the exodus users
namespace exodus
{

DLL_PUBLIC int exodus_main(int exodus__argc, char* exodus__argv[]);

DLL_PUBLIC var osgetenv(const var& name);
DLL_PUBLIC bool osgetenv(const var& name, var& value);
DLL_PUBLIC bool ossetenv(const var& name, const var& value);
DLL_PUBLIC bool assigned(const var& mv1);
DLL_PUBLIC bool unassigned(const var& mv1);
DLL_PUBLIC var date();
DLL_PUBLIC var time();
DLL_PUBLIC var timedate();
DLL_PUBLIC void ossleep(const int milliseconds);
DLL_PUBLIC var ostime();
DLL_PUBLIC bool osopen(const var& filename, var& filehandle);
DLL_PUBLIC void osclose(const var& filehandle);
DLL_PUBLIC var osbread(const var& filehandle, const int startoffset, const int length);
DLL_PUBLIC void osbread(var& data, const var& filehandle, const int startoffset, const int length);
DLL_PUBLIC void osbwrite(const var& data, const var& filehandle, const int startoffset);
DLL_PUBLIC bool osread(var& data, const var& osfilename);
DLL_PUBLIC var osread(const var& osfilename);
DLL_PUBLIC bool oswrite(const var& data,const var& osfilename);
DLL_PUBLIC bool osdelete(const var& osfilename);
DLL_PUBLIC bool osrename(const var& oldosfilename, const var& newosfilename);
DLL_PUBLIC bool oscopy(const var& fromosdir_or_filename, const var& newosdir_or_filename);
DLL_PUBLIC var oslist(const var& path=L".", const var& wildcard=L"", const int mode=0);
DLL_PUBLIC var oslistf(const var& path=L".", const var& wildcard=L"");
DLL_PUBLIC var oslistd(const var& path=L".", const var& wildcard=L"");
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
DLL_PUBLIC void stop(const var& text=L"");
DLL_PUBLIC void abort(const var& text);//dont confuse with abort() which is standard c/c++
DLL_PUBLIC var perform(const var& command);
DLL_PUBLIC var execute(const var& command);
DLL_PUBLIC var chain(const var& command);
DLL_PUBLIC var logoff();
DLL_PUBLIC void debug();

void print(const var& mv1);
void printl(const var& mv1=L"");
void printt(const var& mv1=L"");

DLL_PUBLIC var at(const int columnorcode);
DLL_PUBLIC var at(const var& column, const var& row);
DLL_PUBLIC var getcursor();
DLL_PUBLIC void setcursor(const var& cursor);
DLL_PUBLIC var getprompt();
DLL_PUBLIC void setprompt(const var& prompt);

DLL_PUBLIC var input();
DLL_PUBLIC void input(var& instr);
DLL_PUBLIC void input(const var& prompt, var& instr);
DLL_PUBLIC void inputn(var& instr, const var& n);

DLL_PUBLIC var len(const var& mv1);
DLL_PUBLIC var length(const var& mv1);
DLL_PUBLIC var& converter(var& instring, const var& fromchars, const var& tochars);
DLL_PUBLIC var convert(const var& instring, const var& fromchars, const var& tochars);
DLL_PUBLIC var& swapper(var& instring, const var& fromchars, const var& tochars, const var& options=L"");
DLL_PUBLIC var swap(const var& instring, const var& fromchars, const var& tochars, const var& options=L"");
DLL_PUBLIC var& ucaser(var& instring);
DLL_PUBLIC var ucase(const var& instring);
DLL_PUBLIC var& lcaser(var& instring);
DLL_PUBLIC var lcase(const var& instring);
DLL_PUBLIC var& inverter(var& instring);
DLL_PUBLIC var invert(const var& instring);
DLL_PUBLIC var& lowerer(var& instring);
DLL_PUBLIC var lower(const var& instring);
DLL_PUBLIC var& raiser(var& instring);
DLL_PUBLIC var raise(const var& instring);
DLL_PUBLIC var& splicer(var& instring, const int start1,const int length,const var& str);
DLL_PUBLIC var splice(const var& instring, const int start1,const int length,const var& str);
DLL_PUBLIC var& quoter(var& instring);
DLL_PUBLIC var quote(const var& instring);
DLL_PUBLIC var& fieldstorer(var& instring, const var& sepchar,const int fieldn, const int nfields,const var& replacement);
DLL_PUBLIC var fieldstore(const var& instring, const var& sepchar,const int fieldn, const int nfields,const var& replacement);
DLL_PUBLIC var& trimmer(var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var trim(const var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var& trimmerf(var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var trimf(const var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var& trimmerb(var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var trimb(const var& instring, const wchar_t* trimchar=L" ");
DLL_PUBLIC var& trimmer(var& instring, const var trimchar);
DLL_PUBLIC var trim(const var& instring, const var trimchar);
DLL_PUBLIC var& trimmerf(var& instring, const var trimchar);
DLL_PUBLIC var trimf(const var& instring, const var trimchar);
DLL_PUBLIC var& trimmerb(var& instring, const var trimchar);
DLL_PUBLIC var trimb(const var& instring, const var trimchar);
DLL_PUBLIC var crop(const var& instring);
DLL_PUBLIC var cropper(var& instring);
DLL_PUBLIC var chr(const var& integer);
DLL_PUBLIC var chr(const int integer);
DLL_PUBLIC bool match(const var& instring, const var& matchstr, const var& options=L"");
DLL_PUBLIC var seq(const var& char1);
DLL_PUBLIC var str(const var& instring, const int number);
DLL_PUBLIC var rnd(const int number);
DLL_PUBLIC void initrnd(const int seednumber);
DLL_PUBLIC var mod(const var& dividend, const int divisor);
DLL_PUBLIC var space(const int number);
DLL_PUBLIC var dcount(const var& instring, const var& substrx);
DLL_PUBLIC var count(const var& instring, const var& substrx);
DLL_PUBLIC var substr(const var& instring, const int startx);
DLL_PUBLIC var substr(const var& instring, const int startx,const int length);
DLL_PUBLIC var substrer(var& instring, const int startx);
DLL_PUBLIC var substrer(var& instring, const int startx,const int length);
DLL_PUBLIC var index(const var& instring, const var& substr,const int startchar1=1);
DLL_PUBLIC var field(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx=1);
DLL_PUBLIC var field2(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx=1);
DLL_PUBLIC var oconv(const var& instring, const wchar_t* conversion);
DLL_PUBLIC var oconv(const var& instring, const var& conversion);
DLL_PUBLIC var iconv(const var& instring, const wchar_t* conversion);
DLL_PUBLIC var iconv(const var& instring, const var& conversion);
DLL_PUBLIC bool connect(const var& connectionstring=L"");
DLL_PUBLIC bool disconnect();
DLL_PUBLIC bool createfile(const var& filename,const var& options=L"");
DLL_PUBLIC bool deletefile(const var& filename);
DLL_PUBLIC bool clearfile(const var& filename);
DLL_PUBLIC var listfiles();
DLL_PUBLIC bool createindex(const var& filename, const var& fieldname, const var& dictfilename=L"");
DLL_PUBLIC bool deleteindex(const var& filename, const var& fieldname);
DLL_PUBLIC var listindexes(const var& filename);
DLL_PUBLIC bool begin();
DLL_PUBLIC bool rollback();
DLL_PUBLIC bool end();
DLL_PUBLIC bool lock(const var& filehandle, const var& key);
DLL_PUBLIC void unlock(const var& filehandle, const var& key);
DLL_PUBLIC void unlockall();
DLL_PUBLIC bool open(const var& filename, var& filehandle);
DLL_PUBLIC bool open(const var& dictdata, const var& filename, var& filehandle);
DLL_PUBLIC bool read(var& record, const var& filehandle, const var& key);
DLL_PUBLIC bool readv(var& record, const var& filehandle, const var& key, const var& fieldnumber);
DLL_PUBLIC bool write(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool writev(const var& record, const var& filehandle,const var& key,const int fieldn);
DLL_PUBLIC bool deleterecord(const var& filehandle, const var& key);
DLL_PUBLIC bool updaterecord(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool insertrecord(const var& record, const var& filehandle,const var& key);
DLL_PUBLIC bool select(const var& sortselectclause = L"");
DLL_PUBLIC void clearselect();
DLL_PUBLIC bool readnext(var& key);
DLL_PUBLIC bool readnext(var& key, var& valueno);
DLL_PUBLIC bool selectrecord(const var& sortselectclause = L"");
DLL_PUBLIC bool readnextrecord(var& key, var& record);
DLL_PUBLIC var calculate(const var& fieldname);
DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const wchar_t* mode);
DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const var& mode);
DLL_PUBLIC var remove(const var& fromstr, var& startx, var& delimiterno);
DLL_PUBLIC var replace(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& replacement);
DLL_PUBLIC var extract(const var& instring, const var& fieldno=0, const var& valueno=0, const var& subvalueno=0);
DLL_PUBLIC var insert(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& insertion);
DLL_PUBLIC var erase(const var& instring, const var& fieldno, const var& valueno=0, const var& subvalueno=0);
DLL_PUBLIC var& replacer(var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& replacement);
DLL_PUBLIC var& inserter(var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& insertion);
DLL_PUBLIC var& eraser(var& instring, const var& fieldno, const var& valueno=0, const var& subvalueno=0);
DLL_PUBLIC bool locate(const var& instring, const var& target, var& setting, const int fieldn=0,const int valuen=0);
DLL_PUBLIC bool locateby(const var& instring, const var& target, const wchar_t* ordercode, var& setting, const int fieldn=0,const int valuen=0);
DLL_PUBLIC bool locateby(const var& instring, const var& target, const var& ordercode, var& setting, const int fieldn=0,const int valuen=0);
DLL_PUBLIC bool locateusing(const var& instring, const var& target, const var& usingchar, var& setting, const int fieldn=0, const int valuen=0, const int subvaluen=0);
DLL_PUBLIC bool locateusing(const var& instring, const var& target, const var& usingchar);
DLL_PUBLIC var sum(const var& instring, const var& sepchar=VM_);

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
{ print(arg1); printt(arg2);}
template<class T1, class T2, class T3>
void printt(T1 arg1, T2 arg2, T3 arg3)
{ print(arg1); print(arg2); printt(arg3);}
template<class T1, class T2, class T3, class T4>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{ print(arg1); print(arg2); print(arg3); printt(arg4);}
template<class T1, class T2, class T3, class T4, class T5>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{ print(arg1); print(arg2); print(arg3); print(arg4); printt(arg5);}
template<class T1, class T2, class T3, class T4, class T5, class T6>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); printt(arg6);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); printt(arg7);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); printt(arg8);}
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); printt(arg9); }
template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
void printt(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
{ print(arg1); print(arg2); print(arg3); print(arg4); print(arg5); print(arg6); print(arg7); print(arg8); print(arg9); printt(arg10);}

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
