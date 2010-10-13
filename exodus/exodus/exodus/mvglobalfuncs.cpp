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

#include <cstdlib>//for atexit()

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mv.h>

namespace exodus {

DLL_PUBLIC bool assigned(const var& var1)
{
	return var1.assigned();
}

DLL_PUBLIC bool unassigned(const var& var1)
{
	return !var1.assigned();
}

DLL_PUBLIC var date()
{
	return var().date();
}

DLL_PUBLIC var time()
{
	return var().time();
}

DLL_PUBLIC var timedate()
{
	return var().timedate();
}

void DLL_PUBLIC ossleep(const int milliseconds)
{
	var().ossleep(milliseconds);
}

DLL_PUBLIC var ostime()
{
	return var().ostime();
}

//osopen x to y else
DLL_PUBLIC bool osopen(const var& filename, var& filehandle)
{
	return filehandle.osopen(filename);
}

//osclose x
void DLL_PUBLIC osclose(const var& filehandle)
{
	filehandle.osclose();
}

//3 argument version assignment statement format
//x=osbread(file,postition,length)
DLL_PUBLIC var osbread(const var& filehandle, const int startoffset, const int length)
{
	var data;
	data.osbread(filehandle, startoffset, length);
	return data;
}

//4 argument version for statement format
//osbread(data from x at y length z)
DLL_PUBLIC
var& osbread(var& data, const var& filehandle, const int startoffset, const int length)
{
	//perhaps we can return book for success/failure despite the fact that it is a filehandle supposedly ok
	data.osbread(filehandle, startoffset, length);
	return data;
}

void DLL_PUBLIC osbwrite(const var& data, const var& filehandle, const int startoffset)
{
	data.osbwrite(filehandle, startoffset);
}

//two argument version returns success/failure to be used in if statement
//target variable first to be like "osread x from y else" and "read x from y else"
//unfortunately different from osgetenv which is the reverse
DLL_PUBLIC bool osread(var& data, const var& osfilename)
{
	return data.osread(osfilename);
}

//one argument returns the contents directly to be used in assignments
DLL_PUBLIC var osread(const var& osfilename)
{
	var data;
	if (data.osread(osfilename))
		return data;
	else
		return L"";
}

//oswrite x on y else
DLL_PUBLIC bool oswrite(const var& data,const var& osfilename)
{
	return data.oswrite(osfilename);
}

//if osdelete x else
DLL_PUBLIC bool osdelete(const var& osfilename)
{
	return osfilename.osdelete();
}

//if osrename x to y else
DLL_PUBLIC bool osrename(const var& oldosfilename, const var& newosfilename)
{
	return oldosfilename.osrename(newosfilename);
}

//oscopy x to y
DLL_PUBLIC bool oscopy(const var& fromosfilename, const var& to_osfilename)
{
	return fromosfilename.oscopy(to_osfilename);
}

DLL_PUBLIC var oslist(const var& path, const var& wildcard, const int mode)
{
	return var().oslist(path, wildcard, mode);
}

DLL_PUBLIC var oslistf(const var& path, const var& wildcard)
{
	return var().oslistf(path, wildcard);
}

DLL_PUBLIC var oslistd(const var& path, const var& wildcard)
{
	return var().oslistd(path, wildcard);
}

DLL_PUBLIC var osfile(const var& filename)
{
	return filename.osfile();
}

DLL_PUBLIC var osdir(const var& dirname)
{
	return dirname.osdir();
}

DLL_PUBLIC bool osmkdir(const var& dirname)
{
	return dirname.osmkdir();
}

DLL_PUBLIC bool osrmdir(const var& dirname, const bool evenifnotempty)
{
	return dirname.osrmdir();
}

DLL_PUBLIC var oscwd()
{
	return var().oscwd();
}

DLL_PUBLIC var oscwd(const var& dirname)
{
	return dirname.oscwd(dirname);
}

void DLL_PUBLIC osflush()
{
	return var().osflush();
}

DLL_PUBLIC var suspend(const var& command)
{
	return command.suspend();
}

DLL_PUBLIC var osshell(const var& command)
{
	return command.osshell();
}

void DLL_PUBLIC stop(const var& text)
{
	text.stop(text);
}

void DLL_PUBLIC abort(const var& text)
{
	var().abort(text);
}

DLL_PUBLIC var perform(const var& command)
{
	return command.perform();
}

DLL_PUBLIC var execute(const var& command)
{
	return command.execute();
}

DLL_PUBLIC var chain(const var& command)
{
	return command.chain();
}

DLL_PUBLIC var logoff()
{
	return var().logoff();
}

void DLL_PUBLIC debug()
{
	var().debug();
}


void DLL_PUBLIC print(const var& var1)
{
	var1.output();
}

void DLL_PUBLIC printl(const var& var1)
{
	var1.outputl();
}

void DLL_PUBLIC printt(const var& var1)
{
	var1.outputt();
}


void DLL_PUBLIC output(const var& var1)
{
	var1.output();
}

void DLL_PUBLIC outputl(const var& var1)
{
	var1.outputl();
}

void DLL_PUBLIC outputt(const var& var1)
{
	var1.outputt();
}


void DLL_PUBLIC errput(const var& var1)
{
	var1.errput();
}

void DLL_PUBLIC errputl(const var& var1)
{
	var1.errputl();
}


void DLL_PUBLIC logput(const var& var1)
{
	var1.logput();
}

void DLL_PUBLIC logputl(const var& var1)
{
	var1.logputl();
}


DLL_PUBLIC var at(const int columnorcode)
{
	return var().at(columnorcode);
}

DLL_PUBLIC var at(const var& column, const var& row)
{
	return var().at(column,row);
}

DLL_PUBLIC var getcursor()
{
	return var().getcursor();
}

void DLL_PUBLIC setcursor(const var& cursor)
{
	cursor.setcursor();
}

DLL_PUBLIC var getprompt()
{
	return var().getprompt();
}

void DLL_PUBLIC setprompt(const var& prompt)
{
	prompt.setprompt();
}

DLL_PUBLIC var input()
{
	var temp;
	temp.input();
	return temp;
}

DLL_PUBLIC void input(var& inputfield)
{
	inputfield.input();
}

DLL_PUBLIC void input(const var& prompt, var& inputfield)
{
	inputfield.input(prompt);
}

DLL_PUBLIC void inputn(var& inputfield, const int nchars)
{
	inputfield.input(L"", nchars);
}

DLL_PUBLIC var len(const var& var1)
{
	return var1.len();
}

DLL_PUBLIC var length(const var& var1)
{
	return var1.length();
}

DLL_PUBLIC var& converter(var& instring, const var& fromchars, const var& tochars)
{
	return instring.converter(fromchars,tochars);
}

DLL_PUBLIC var convert(const var& instring, const var& fromchars, const var& tochars)
{
	return instring.convert(fromchars,tochars);
}

DLL_PUBLIC var& swapper(var& instring, const var& from, const var& to, const var& options=L"")
{
	return instring.swapper(from, to, options);
}

DLL_PUBLIC var swap(const var& instring, const var& from, const var& to, const var& options=L"")
{
	return instring.swap(from,to,options);
}

DLL_PUBLIC var& ucaser(var& instring)
{
	return instring.ucaser();
}

DLL_PUBLIC var ucase(const var& instring)
{
	return instring.ucase();
}

DLL_PUBLIC var& lcaser(var& instring)
{
	return instring.lcaser();
}

DLL_PUBLIC var lcase(const var& instring)
{
	return instring.lcase();
}

DLL_PUBLIC var& inverter(var& instring)
{
	return instring.inverter();
}

DLL_PUBLIC var invert(const var& instring)
{
	return instring.invert();
}

DLL_PUBLIC var& lowerer(var& instring)
{
	return instring.lowerer();
}

DLL_PUBLIC var lower(const var& instring)
{
	return instring.lower();
}

DLL_PUBLIC var& raiser(var& instring)
{
	return instring.raiser();
}

DLL_PUBLIC var raise(const var& instring)
{
	return instring.raise();
}

DLL_PUBLIC var& splicer(var& instring, const int start1,const int length,const var& str)
{
	return instring.splicer(start1, length, str);
}

DLL_PUBLIC var splice(const var& instring, const int start1,const int length,const var& str)
{
	return instring.splice(start1, length, str);
}

DLL_PUBLIC var& quoter(var& instring)
{
	return instring.quoter();
}

DLL_PUBLIC var quote(const var& instring)
{
	return instring.quote();
}

DLL_PUBLIC var& fieldstorer(var& instring, const var& sepchar,const int fieldn, const int nfields,const var& replacement)
{
	return instring.fieldstorer(sepchar, fieldn, nfields, replacement);
}

DLL_PUBLIC var fieldstore(const var& instring, const var& sepchar,const int fieldn, const int nfields,const var& replacement)
{
	return instring.fieldstore(sepchar, fieldn, nfields, replacement);
}

DLL_PUBLIC var crop(const var& instring)
{
	return instring.crop();
}

DLL_PUBLIC var cropper(var& instring)
{
	return instring.cropper();
}

DLL_PUBLIC var& trimmer(var& instring, const wchar_t* trimchar)
{
	return instring.trimmer(trimchar);
}

DLL_PUBLIC var trim(const var& instring, const wchar_t* trimchar)
{
	return instring.trim(trimchar);
}

DLL_PUBLIC var& trimmerf(var& instring, const wchar_t* trimchar)
{
	return instring.trimmerf(trimchar);
}

DLL_PUBLIC var trimf(const var& instring, const wchar_t* trimchar)
{
	return instring.trimf(trimchar);
}

DLL_PUBLIC var& trimmerb(var& instring, const wchar_t* trimchar)
{
	return instring.trimmerb(trimchar);
}

DLL_PUBLIC var trimb(const var& instring, const wchar_t* trimchar)
{
	return instring.trimb(trimchar);
}

DLL_PUBLIC var& trimmer(var& instring, const var trimchar)
{
	return instring.trimmer(trimchar);
}

DLL_PUBLIC var trim(const var& instring, const var trimchar)
{
	return instring.trim(trimchar);
}

DLL_PUBLIC var& trimmerf(var& instring, const var trimchar)
{
	return instring.trimmerf(trimchar);
}

DLL_PUBLIC var trimf(const var& instring, const var trimchar)
{
	return instring.trimf(trimchar);
}

DLL_PUBLIC var& trimmerb(var& instring, const var trimchar)
{
	return instring.trimmerb(trimchar);
}

DLL_PUBLIC var trimb(const var& instring, const var trimchar)
{
	return instring.trimb(trimchar);
}

DLL_PUBLIC var chr(const var& integer)
{
	return var().chr(integer);
}

DLL_PUBLIC var chr(const int integer)
{
	return var().chr(integer);
}

DLL_PUBLIC bool match(const var& instring, const var& matchstr)
{
	return instring.match(matchstr);
}

DLL_PUBLIC var seq(const var& char1)
{
	return char1.seq();
}

DLL_PUBLIC var str(const var& instring, const int number)
{
	return instring.str(number);
}

DLL_PUBLIC var rnd(const int number)
{
	return var(number).rnd();
}

void DLL_PUBLIC initrnd(const int seednumber)
{
	var(seednumber).initrnd();
}

DLL_PUBLIC var mod(const var& dividend, const int divisor)
{
	return dividend % divisor;
}

DLL_PUBLIC var space(const int number)
{
	return var(number).space();
}

DLL_PUBLIC var dcount(const var& instring, const var& substrx)
{
	return instring.dcount(substrx);
}

DLL_PUBLIC var count(const var& instring, const var& substrx)
{
	return instring.count(substrx);
}

DLL_PUBLIC var substr(const var& instring, const int startx)
{
	return instring.substr(startx);
}

DLL_PUBLIC var substr(const var& instring, const int startx,const int length)
{
	return instring.substr(startx, length);
}

DLL_PUBLIC var substrer(var& instring, const int startx)
{
	return instring.substrer(startx);
}

DLL_PUBLIC var substrer(var& instring, const int startx,const int length)
{
	return instring.substrer(startx, length);
}

DLL_PUBLIC var index(const var& instring, const var& substr,const int startchar1)
{
	return instring.index(substr, startchar1);
}

DLL_PUBLIC var field(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx)
{
	return instring.field(substrx, fieldnx,nfieldsx);
}

DLL_PUBLIC var field2(const var& instring, const var& substrx,const int fieldnx,const int nfieldsx)
{
	return instring.field2(substrx, fieldnx,nfieldsx);
}

DLL_PUBLIC var oconv(const var& instring, const wchar_t* conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var oconv(const var& instring, const var& conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var iconv(const var& instring, const wchar_t* conversion)
{
	return instring.iconv(conversion);
}

DLL_PUBLIC var iconv(const var& instring, const var& conversion)
{
	return instring.iconv(conversion);
}


DLL_PUBLIC bool connect(const var& connectioninfo)
{
	return var().connect(connectioninfo);
}

DLL_PUBLIC bool disconnect()
{
	return var().disconnect();
}


DLL_PUBLIC bool createfile(const var& filename,const var& options)
{
	return var().createfile(filename, options);
}

DLL_PUBLIC bool deletefile(const var& filename)
{
	return filename.deletefile();
}

DLL_PUBLIC bool createindex(const var& filename, const var& fieldname, const var& dictfilename)
{
	return filename.createindex(fieldname, dictfilename);
}

DLL_PUBLIC bool deleteindex(const var& filename, const var& fieldname)
{
	return filename.deleteindex(fieldname);
}

DLL_PUBLIC bool clearfile(const var& filename)
{
	return filename.clearfile();
}


DLL_PUBLIC bool begin()
{
	return var().begin();
}

DLL_PUBLIC bool rollback()
{
	return var().rollback();
}

DLL_PUBLIC bool end()
{
	return var().end();
}


DLL_PUBLIC bool lock(const var& filehandle, const var& key)
{
	return filehandle.lock(key);
}

void DLL_PUBLIC unlock(const var& filehandle, const var& key)
{
	filehandle.unlock(key);
}

void DLL_PUBLIC unlockall()
{
	var().unlockall();
}


DLL_PUBLIC bool open(const var& filename, var& filehandle)
{
	return filehandle.open(filename);
}

DLL_PUBLIC bool open(const var& dictdata, const var& filename, var& filehandle)
{
	if (dictdata==L"DICT")
		return filehandle.open(dictdata,filename);
	else
		return filehandle.open(filename);
}

DLL_PUBLIC bool read(var& record, const var& filehandle, const var& key)
{
	return record.read(filehandle,key);
}

DLL_PUBLIC bool readv(var& record, const var& filehandle, const var& key, const var& fieldnumber)
{
	return record.readv(filehandle,key,fieldnumber);
}

DLL_PUBLIC bool write(const var& record, const var& filehandle,const var& key)
{
	return record.write(filehandle, key);
}

DLL_PUBLIC bool writev(const var& record, const var& filehandle,const var& key,const int fieldn)
{
	return record.writev(filehandle, key, fieldn);
}

DLL_PUBLIC bool deleterecord(const var& filehandle, const var& key)
{
	return filehandle.deleterecord(key);
}


DLL_PUBLIC bool updaterecord(const var& record, const var& filehandle,const var& key)
{
	return record.updaterecord(filehandle, key);
}

DLL_PUBLIC bool insertrecord(const var& record, const var& filehandle,const var& key)
{
	return record.insertrecord(filehandle, key);
}

DLL_PUBLIC bool select(const var& sortselectclause)
{
	return var("default").select(sortselectclause);
}

void DLL_PUBLIC clearselect()
{
	var("default").clearselect();
}

DLL_PUBLIC bool readnext(var& key)
{
	return var("default").readnext(key);
}

DLL_PUBLIC bool readnext(var& key, var& valueno)
{
	return var("default").readnext(key,valueno);
}

DLL_PUBLIC bool selectrecord(const var& sortselectclause)
{
	return var("default").selectrecord(sortselectclause);
}

DLL_PUBLIC bool readnextrecord(var& key, var& record)
{
	return var("default").readnextrecord(key, record);
}


DLL_PUBLIC var calculate(const var& fieldname)
{
	return fieldname.calculate();
}

DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const wchar_t* mode)
{
	return key.xlate(filename, fieldno, mode);
}

DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const var& mode)
{
	return key.xlate(filename, fieldno, mode);
}

DLL_PUBLIC var remove(const var& fromstr, var& startx, var& delimiterno)
{
	return fromstr.remove(startx,delimiterno);
}

//TODO (sb) provide versions of the following that accept ints+vars in all the various combinations or a template
//to optimise away the unnecessary int->var conversion
DLL_PUBLIC var replace(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& replacement)
{
	return instring.replace(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC var extract(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno)
{
	return instring.extract(fieldno, valueno, subvalueno);
}

DLL_PUBLIC var insert(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& insertion)
{
	return instring.insert(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC var erase(const var& instring, const var& fieldno, const var& valueno, const var& subvalueno)
{
	return instring.erase(fieldno, valueno, subvalueno);
}

DLL_PUBLIC var& replacer(var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& replacement)
{
	return instring.replacer(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC var& inserter(var& instring, const var& fieldno, const var& valueno, const var& subvalueno, const var& insertion)
{
	return instring.inserter(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC var& eraser(var& instring, const var& fieldno, const var& valueno, const var& subvalueno)
{
	return instring.eraser(fieldno, valueno, subvalueno);
}

DLL_PUBLIC bool locate(const var& target, const var& instring, var& setting, const int fieldn,const int valuen)
{
	return instring.locate(target,setting,fieldn,valuen);
}

DLL_PUBLIC bool locateby(const var& target, const var& instring, const wchar_t* ordercode, var& setting, const int fieldn,const int 
valuen)
{
	return instring.locateby(target,ordercode,setting,fieldn,valuen);
}

DLL_PUBLIC bool locateby(const var& target, const var& instring, const var& ordercode, var& setting, const int fieldn,const int valuen)
{
	return instring.locateby(target,ordercode,setting,fieldn,valuen);
}

DLL_PUBLIC bool locateusing(const var& target, const var& instring, const var& usingchar, var& setting, const int fieldn, const int 
valuen, const int subvaluen)
{
	return instring.locateusing(target, usingchar, setting, fieldn, valuen, subvaluen);
}

DLL_PUBLIC bool locateusing(const var& target, const var& instring, const var& usingchar)
{
	return instring.locateusing(target, usingchar);
}

DLL_PUBLIC var sum(const var& instring, const var& sepchar)
{
	return instring.sum(sepchar);
}

DLL_PUBLIC var listfiles()
{
	return var().listfiles();
}

DLL_PUBLIC var listindexes(const var& filename)
{
	return var().listindexes(filename);
}

//one argument returns the contents
DLL_PUBLIC var osgetenv(const var& name)
{
	var temp="";
	temp.osgetenv(name);
	return temp;
}

//two argument returns the success/failure to be used in an if statement
//target variable last to be like ossetenv and traditional getenv name, value
//unfortunately different from osread which is the reverse
DLL_PUBLIC bool osgetenv(const var& name, var& value)
{
	return value.osgetenv(name);
}

//like "if set xxx=yyy"
DLL_PUBLIC bool ossetenv(const var& name, const var& value)
{
	return value.ossetenv(name);
}

DLL_PUBLIC
void exodus_atexit()
{
	var().disconnect();
}

/* getting path to current executable

from http://stackoverflow.com/questions/143174/c-c-how-to-obtain-the-full-path-of-current-directory

Windows:

int bytes = GetModuleFileName(NULL, pBuf, len);
if(bytes == 0)
        return -1;
else
        return bytes;

Linux but not macosx apparently:

char szTmp[32];
sprintf(szTmp, "/proc/%d/exe", getpid());
int bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
if(bytes >= 0)
        pBuf[bytes] = '\0';
return bytes;
*/

DLL_PUBLIC
int exodus_main(int exodus__argc, char *exodus__argv[])
{

	//signal/interrupt handlers
	//install_signals();
	var().breakon();

	_EXECPATH=exodus__argv[0];
	//"see getting path to current executable" above
	//or use "which _EXECPATH somehow like in mvdebug.cpp
	//if (not _EXECPATH.index(_SLASH) && not _EXECPATH.index(":"))
	//{
	//	_EXECPATH.splicer(0,0,oscwd()^_SLASH);
	//	if (_SLASH==L"\\")
	//		_EXECPATH.converter(L"/",L"\\");
	//}

	_SENTENCE="";
	_COMMAND="";
	_OPTIONS="";

	//reconstructs complete original sentence unfortunately quote marks will have been lost unless escaped
	//needs to go after various exodus definitions
	for (int ii=0; ii<exodus__argc; ++ii)
	{
		var word=exodus__argv[ii];
		if (ii == 0)
		{
			word=word.field2(_SLASH,-1);
			//remove trailing ".exe"
			if (word.lcase().substr(-4) == ".exe")
				word.splicer(-4,4,"");
		} else
			_SENTENCE^=" ";

		//put back quotes if any spaces
		if (word.index(" "))
			word.quoter();

		_SENTENCE^=word;
		_COMMAND^=FM^word;
	}
	_COMMAND.splicer(1,1,"");

	//options are in either (XXX) or {XXX} at the end of the command.
	var lastchar=_COMMAND.substr(-1);
	if (lastchar==")")
		_OPTIONS=_COMMAND.field2("(",-1);
	else if (lastchar=="}")
		_OPTIONS=_COMMAND.field2("{",-1);
	if (_OPTIONS)
		_COMMAND.splicer(-(len(_OPTIONS)+2),len(_OPTIONS)+2, "");

	//would have to passed in as a function pointer
	//main2(exodus__argc, exodus__argv);

	atexit(exodus_atexit);

	return 0;
}


}// of namespace exodus

