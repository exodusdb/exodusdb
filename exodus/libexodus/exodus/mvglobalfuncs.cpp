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

//#include <cstdlib> //for atexit()

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <exodus/mv.h>
#include <exodus/mvenvironment.h>

namespace exodus {

DLL_PUBLIC bool setxlocale(const var& locale) {
	return locale.setxlocale();
}

DLL_PUBLIC var getxlocale() {
	return var().getxlocale();
}

DLL_PUBLIC bool assigned(const var& var1) {
	return var1.assigned();
}

DLL_PUBLIC bool unassigned(const var& var1) {
	return !var1.assigned();
}

DLL_PUBLIC var& transfer(var& fromvar, var& tovar) {
	return fromvar.transfer(tovar);
}

DLL_PUBLIC const var& exchange(const var& var1, const var& var2) {
	return var1.exchange(var2);
}

DLL_PUBLIC var date() {
	return var().date();
}

DLL_PUBLIC var time() {
	return var().time();
}

DLL_PUBLIC var timedate() {
	return var().timedate();
}

DLL_PUBLIC void ossleep(const int milliseconds) {
	var().ossleep(milliseconds);
}

DLL_PUBLIC var ostime() {
	return var().ostime();
}

DLL_PUBLIC void breakon() {
	return var().breakon();
}
DLL_PUBLIC void breakoff() {
	return var().breakoff();
}

// osopen x to y else
DLL_PUBLIC bool osopen(const var& osfilename, var& osfilevar, const var& locale) {
	return osfilevar.osopen(osfilename, locale);
}

// osclose x
DLL_PUBLIC void osclose(const var& osfilevar) {
	osfilevar.osclose();
}

// 4 argument version for statement format
// osbread(data from x at y length z)
// DLL_PUBLIC var& osbread(var& data, const var& filehandle, const int offset, const int
// length)
//var& osbread(var& data, const var& filehandle, var& offset, const int length,
//	     const bool adjust)
DLL_PUBLIC bool osbread(var& data, const var& filehandle, var& offset, const int length) {
	return data.osbread(filehandle, offset, length);
}

// 4 argument version for statement format BUT ALLOWING offset TO BE A CONSTANT ie output
// ignored osbread(data from x at y length z) DLL_PUBLIC var& osbread(var& data, const var&
// filehandle, const int offset, const int length)
//var& osbread(var& data, const var& filehandle, const var& offset, const int length,
//	     const bool adjust)
DLL_PUBLIC bool osbread(var& data, const var& filehandle, const var& offset, const int length) {
	return data.osbread(filehandle, const_cast<var&>(offset), length);
}

//DLL_PUBLIC bool osbwrite(const var& data, const var& filehandle, var& offset,
//			 const bool adjust = true)
DLL_PUBLIC bool osbwrite(const var& data, const var& filehandle, var& offset) {
	return data.osbwrite(filehandle, offset);
}

//DLL_PUBLIC bool osbwrite(const var& data, const var& filehandle, const var& offset,
//			 const bool adjust)
DLL_PUBLIC bool osbwrite(const var& data, const var& filehandle, const var& offset) {
	return data.osbwrite(filehandle, const_cast<var&>(offset));
}

// two argument version returns success/failure to be used in if statement
// target variable first to be like "osread x from y else" and "read x from y else"
// unfortunately different from osgetenv which is the reverse
DLL_PUBLIC bool osread(var& data, const var& osfilename, const var& codepage) {
	return data.osread(osfilename, codepage);
}

// one argument returns the contents directly to be used in assignments
DLL_PUBLIC var osread(const var& osfilename) {
	var data;
	if (data.osread(osfilename))
		return data;
	else
		return "";
}

// oswrite x on y else
DLL_PUBLIC bool oswrite(const var& data, const var& osfilename, const var& codepage = "") {
	return data.oswrite(osfilename, codepage);
}

// if osdelete x else
DLL_PUBLIC bool osdelete(const var& osfilename) {
	return osfilename.osdelete();
}

// if osrename x to y else
DLL_PUBLIC bool osrename(const var& oldosdir_or_filename, const var& newosdir_or_filename) {
	return oldosdir_or_filename.osrename(newosdir_or_filename);
}

// oscopy x to y
DLL_PUBLIC bool oscopy(const var& fromosfilename, const var& to_osfilename) {
	return fromosfilename.oscopy(to_osfilename);
}

DLL_PUBLIC var oslist(const var& path, const var& wildcard, const int mode) {
	return var().oslist(path, wildcard, mode);
}

DLL_PUBLIC var oslistf(const var& path, const var& wildcard) {
	return var().oslistf(path, wildcard);
}

DLL_PUBLIC var oslistd(const var& path, const var& wildcard) {
	return var().oslistd(path, wildcard);
}

DLL_PUBLIC var osfile(const var& filename) {
	return filename.osfile();
}

DLL_PUBLIC var osdir(const var& dirname) {
	return dirname.osdir();
}

DLL_PUBLIC bool osmkdir(const var& dirname) {
	return dirname.osmkdir();
}

DLL_PUBLIC bool osrmdir(const var& dirname, const bool evenifnotempty) {
	return dirname.osrmdir(evenifnotempty);
}

DLL_PUBLIC var ospid() {
	return var().ospid();
}

DLL_PUBLIC var oscwd() {
	return var().oscwd();
}

DLL_PUBLIC var oscwd(const var& dirname) {
	return dirname.oscwd(dirname);
}

DLL_PUBLIC void osflush() {
	return var().osflush();
}

DLL_PUBLIC var suspend(const var& command) {
	return command.suspend();
}

// version to get return int (conventionally 0 means success, otherwise error)
// example: if (osshell(somecommand)) { ...
DLL_PUBLIC bool osshell(const var& command) {
	return command.osshell();
}

// simple version to write one liner read (declare and assign in one line)
// example: var xx=osshellread("somecommand");
DLL_PUBLIC var osshellread(const var& command) {
	var result;
	result.osshellread(command);
	return result;
}

// versions to be written like a command

// example: osshellread(command,outputresult);
// for now returns nothing since popen cannot write and tell success
DLL_PUBLIC bool osshellread(var& readstr, const var& command) {
	return readstr.osshellread(command);
}

// example: osshellwrite(command,inputforcommand);
// for now returns nothing since popen cannot write and tell success
DLL_PUBLIC bool osshellwrite(const var& writestr, const var& command) {
	return writestr.osshellwrite(command);
}

DLL_PUBLIC void stop(const var& text) {
	text.stop(text);
}

DLL_PUBLIC void abort(const var& text) {
	var().abort(text);
}
DLL_PUBLIC void abortall(const var& text) {
	var().abortall(text);
}

// DLL_PUBLIC var perform(const var& command)
//{
//	return command.perform();
//}

// DLL_PUBLIC var execute(const var& command)
//{
//	return command.execute();
//}

// DLL_PUBLIC var chain(const var& command)
//{
//	return command.chain();
//}

DLL_PUBLIC var logoff() {
	return var().logoff();
}

DLL_PUBLIC void debug() {
	var().debug();
}

DLL_PUBLIC var at(const int columnorcode) {
	return var().at(columnorcode);
}

DLL_PUBLIC var at(const var& column, const var& row) {
	return var().at(column, row);
}

DLL_PUBLIC var getcursor() {
	var cursor;
	cursor.getcursor();
	return cursor;
}

DLL_PUBLIC void setcursor(const var& cursor) {
	cursor.setcursor();
}

DLL_PUBLIC var getprompt() {
	return var().getprompt();
}

DLL_PUBLIC void setprompt(const var& prompt) {
	prompt.setprompt();
}

DLL_PUBLIC bool echo(const int on_off) {
	return var().echo(on_off);
}

DLL_PUBLIC var input() {
	var v;
	v.input();
	return v;
}

DLL_PUBLIC var input(const var& prompt) {
	var v;
	v.input(prompt);
	return v;
}

DLL_PUBLIC var inputn(const int nchars) {
	var v;
	v.inputn(nchars);
	return v;
}

DLL_PUBLIC var len(const var& var1) {
	return var1.len();
}

DLL_PUBLIC var length(const var& var1) {
	return var1.length();
}

DLL_PUBLIC var& converter(var& instring, const var& oldchars, const var& newchars) {
	return instring.converter(oldchars, newchars);
}

DLL_PUBLIC var convert(const var& instring, const var& oldchars, const var& newchars) {
	return instring.convert(oldchars, newchars);
}

DLL_PUBLIC var& textconverter(var& instring, const var& oldchars, const var& newchars) {
	return instring.textconverter(oldchars, newchars);
}

DLL_PUBLIC var textconvert(const var& instring, const var& oldchars, const var& newchars) {
	return instring.textconvert(oldchars, newchars);
}

DLL_PUBLIC var& swapper(var& instring, const var& from, const var& to) {
	return instring.swapper(from, to);
}

DLL_PUBLIC var swap(const var& instring, const var& from, const var& to) {
	var newstring = instring;
	return newstring.swap(from, to);
}

DLL_PUBLIC var& replacer(var& instring, const var& regexstr, const var& replacementstr, const var& options = "") {
	return instring.replacer(regexstr, replacementstr, options);
}

DLL_PUBLIC var replace(const var& instring, const var& regexstr, const var& replacementstr, const var& options = "") {
	var newstring = instring;
	return newstring.replacer(regexstr, replacementstr, options);
}

DLL_PUBLIC var& ucaser(var& instring) {
	return instring.ucaser();
}

DLL_PUBLIC var ucase(const var& instring) {
	return instring.ucase();
}

DLL_PUBLIC var& lcaser(var& instring) {
	return instring.lcaser();
}

DLL_PUBLIC var lcase(const var& instring) {
	return instring.lcase();
}

DLL_PUBLIC var& tcaser(var& instring) {
	return instring.tcaser();
}

DLL_PUBLIC var tcase(const var& instring) {
	return instring.tcase();
}

DLL_PUBLIC var& fcaser(var& instring) {
	return instring.fcaser();
}

DLL_PUBLIC var fcase(const var& instring) {
	return instring.fcase();
}

DLL_PUBLIC var& normalizer(var& instring) {
	return instring.normalizer();
}

DLL_PUBLIC var normalize(const var& instring) {
	return instring.normalize();
}

DLL_PUBLIC var unique(const var& instring) {
	return instring.unique();
}

DLL_PUBLIC var& inverter(var& instring) {
	return instring.inverter();
}

DLL_PUBLIC var invert(const var& instring) {
	return instring.invert();
}

DLL_PUBLIC var& lowerer(var& instring) {
	return instring.lowerer();
}

DLL_PUBLIC var lower(const var& instring) {
	return instring.lower();
}

DLL_PUBLIC var& raiser(var& instring) {
	return instring.raiser();
}

DLL_PUBLIC var raise(const var& instring) {
	return instring.raise();
}

DLL_PUBLIC var& splicer(var& instring, const int start1, const int length, const var& str) {
	return instring.splicer(start1, length, str);
}

DLL_PUBLIC var splice(const var& instring, const int start1, const int length, const var& str) {
	return instring.splice(start1, length, str);
}

DLL_PUBLIC var& quoter(var& instring) {
	return instring.quoter();
}

DLL_PUBLIC var quote(const var& instring) {
	return instring.quote();
}

DLL_PUBLIC var& unquoter(var& instring) {
	return instring.unquoter();
}

DLL_PUBLIC var unquote(const var& instring) {
	return instring.unquote();
}

DLL_PUBLIC var& fieldstorer(var& instring, const var& sepchar, const int fieldno, const int nfields, const var& replacement) {
	return instring.fieldstorer(sepchar, fieldno, nfields, replacement);
}

DLL_PUBLIC var fieldstore(const var& instring, const var& sepchar, const int fieldno, const int nfields, const var& replacement) {
	return instring.fieldstore(sepchar, fieldno, nfields, replacement);
}

DLL_PUBLIC var crop(const var& instring) {
	return instring.crop();
}

DLL_PUBLIC var cropper(var& instring) {
	return instring.cropper();
}

DLL_PUBLIC var& trimmer(var& instring, const char* trimchars) {
	return instring.trimmer(trimchars);
}

DLL_PUBLIC var trim(const var& instring, const char* trimchars) {
	return instring.trim(trimchars);
}

DLL_PUBLIC var& trimmerf(var& instring, const char* trimchars) {
	return instring.trimmerf(trimchars);
}

DLL_PUBLIC var trimf(const var& instring, const char* trimchars) {
	return instring.trimf(trimchars);
}

DLL_PUBLIC var& trimmerb(var& instring, const char* trimchars) {
	return instring.trimmerb(trimchars);
}

DLL_PUBLIC var trimb(const var& instring, const char* trimchars) {
	return instring.trimb(trimchars);
}

DLL_PUBLIC var& trimmer(var& instring, const var& trimchars) {
	return instring.trimmer(trimchars);
}

DLL_PUBLIC var& trimmer(var& instring, const var& trimchars, const var& options) {
	return instring.trimmer(trimchars, options);
}

DLL_PUBLIC var trim(const var& instring, const var& trimchars) {
	return instring.trim(trimchars);
}

DLL_PUBLIC var trim(const var& instring, const var& trimchars, const var& options) {
	return instring.trim(trimchars, options);
}

DLL_PUBLIC var& trimmerf(var& instring, const var& trimchars) {
	return instring.trimmerf(trimchars);
}

DLL_PUBLIC var trimf(const var& instring, const var& trimchars) {
	return instring.trimf(trimchars);
}

DLL_PUBLIC var& trimmerb(var& instring, const var& trimchars) {
	return instring.trimmerb(trimchars);
}

DLL_PUBLIC var trimb(const var& instring, const var& trimchars) {
	return instring.trimb(trimchars);
}

DLL_PUBLIC bool matread(dim& dimrecord, const var& filehandle, const var& key) {
	return dimrecord.read(filehandle, key);
}

DLL_PUBLIC bool matwrite(const dim& dimrecord, const var& filehandle, const var& key) {
	return dimrecord.write(filehandle, key);
}

DLL_PUBLIC var split(const var& sourcevar, dim& destinationdim) {
	return destinationdim.split(sourcevar);
}

DLL_PUBLIC dim split(const var& sourcevar) {
	return sourcevar.split();
}

DLL_PUBLIC var join(const dim& sourcedim) {
	return sourcedim.join();
}

DLL_PUBLIC var chr(const var& integer) {
	return var().chr(integer);
}

DLL_PUBLIC var chr(const int integer) {
	return var().chr(integer);
}

DLL_PUBLIC var textchr(const var& integer) {
	return var().textchr(integer);
}

DLL_PUBLIC var textchr(const int integer) {
	return var().textchr(integer);
}

DLL_PUBLIC var match(const var& instring, const var& matchstr, const var& options) {
	return instring.match(matchstr, options);
}

DLL_PUBLIC var seq(const var& char1) {
	return char1.seq();
}

DLL_PUBLIC var textseq(const var& char1) {
	return char1.textseq();
}

DLL_PUBLIC var str(const var& instring, const int number) {
	return instring.str(number);
}

// MATH/BOOLEAN
DLL_PUBLIC var abs(const var& num1) {
	return num1.abs();
}

DLL_PUBLIC var pwr(const var& base, const var& exponent) {
	return base.pwr(exponent);
}

DLL_PUBLIC var exp(const var& exponent) {
	return exponent.exp();
}

DLL_PUBLIC var sqrt(const var& num1) {
	return num1.sqrt();
}

DLL_PUBLIC var sin(const var& degrees) {
	return degrees.sin();
}

DLL_PUBLIC var cos(const var& degrees) {
	return degrees.cos();
}

DLL_PUBLIC var tan(const var& degrees) {
	return degrees.tan();
}

DLL_PUBLIC var atan(const var& degrees) {
	return degrees.atan();
}

DLL_PUBLIC var loge(const var& num1) {
	return num1.loge();
}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
DLL_PUBLIC var integer(const var& num1) {
	return num1.integer();
}

DLL_PUBLIC var floor(const var& num1) {
	return num1.floor();
}

DLL_PUBLIC var round(const var& num1, const int ndecimals = 0) {
	return num1.round(ndecimals);
}

DLL_PUBLIC var rnd(const int number) {
	return var(number).rnd();
}

DLL_PUBLIC void initrnd(const var& seed) {
	var(seed).initrnd();
}

DLL_PUBLIC var mod(const var& dividend, const var& divisor) {
	return dividend.mod(divisor);
}

DLL_PUBLIC var mod(const var& dividend, const int divisor) {
	return dividend.mod(divisor);
}

DLL_PUBLIC var space(const int number) {
	return var(number).space();
}

DLL_PUBLIC var dcount(const var& instring, const var& substrx) {
	return instring.dcount(substrx);
}

DLL_PUBLIC var count(const var& instring, const var& substrx) {
	return instring.count(substrx);
}

DLL_PUBLIC var substr(const var& instring, const int startx) {
	return instring.substr(startx);
}

DLL_PUBLIC var substr(const var& instring, const int startx, const int length) {
	return instring.substr(startx, length);
}

DLL_PUBLIC var substrer(var& instring, const int startx) {
	return instring.substrer(startx);
}

DLL_PUBLIC var substrer(var& instring, const int startx, const int length) {
	return instring.substrer(startx, length);
}

DLL_PUBLIC var index(const var& instring, const var& substr, const int occurrenceno) {
	return instring.index(substr, occurrenceno);
}

DLL_PUBLIC var index2(const var& instring, const var& substr, const int startcharno) {
	return instring.index2(substr, startcharno);
}

DLL_PUBLIC var field(const var& instring, const var& substrx, const int fieldnx, const int nfieldsx) {
	return instring.field(substrx, fieldnx, nfieldsx);
}

DLL_PUBLIC var field2(const var& instring, const var& substrx, const int fieldnx, const int nfieldsx) {
	return instring.field2(substrx, fieldnx, nfieldsx);
}

/* moved to mvprogram to allow custom conversions like "[DATE]"
DLL_PUBLIC var oconv(const var& instring, const char* conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var oconv(const var& instring, const var& conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var iconv(const var& instring, const char* conversion)
{
	return instring.iconv(conversion);
}

DLL_PUBLIC var iconv(const var& instring, const var& conversion)
{
	return instring.iconv(conversion);
}
*/

DLL_PUBLIC bool connect(const var& connectioninfo) {
	var conn1;
	return conn1.connect(connectioninfo);
}

DLL_PUBLIC void disconnect() {
	var().disconnect();
}

DLL_PUBLIC void disconnectall() {
	var().disconnectall();
}

DLL_PUBLIC bool createdb(const var& dbname) {
	return dbname.createdb(dbname);
}

DLL_PUBLIC bool copydb(const var& from_dbname, const var& to_dbname) {
	return var().copydb(from_dbname, to_dbname);
}

DLL_PUBLIC bool deletedb(const var& dbname) {
	return var().deletedb(dbname);
}

DLL_PUBLIC bool createfile(const var& filename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var filename2 = filename.field("(", 1).trim();

	//remove pickos volume locations
	filename2.swapper("DATA ", "").swapper("REVBOOT ", "").swapper("DATAVOL ", "").trimmer();

	return filename2.createfile(filename2);
}

DLL_PUBLIC bool deletefile(const var& filename_or_handle) {
	//remove options like (S)
	var filename2 = filename_or_handle.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	filename2.swapper("DATA ", "").trimmer();

	return filename_or_handle.deletefile(filename2);
}

DLL_PUBLIC bool clearfile(const var& filename_or_handle) {
	//remove options like (S)
	var filename2 = filename_or_handle.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	filename2.swapper("DATA ", "").trimmer();

	return filename_or_handle.clearfile(filename2);
}

DLL_PUBLIC bool renamefile(const var& filename_or_handle, const var& newfilename) {
	return filename_or_handle.renamefile(filename_or_handle.a(1), newfilename);
}

DLL_PUBLIC bool createindex(const var& filename_or_handle, const var& fieldname, const var& dictfilename) {

	//virtually identical code in createindex and deleteindex
	if (filename_or_handle.index(" ")) {
		var filename2 = filename_or_handle.field(" ", 1);
		var fieldname2 = filename_or_handle.field(" ", 2);
		var indextype2 = filename_or_handle.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return filename2.createindex(fieldname2, dictfilename);
	}

	return filename_or_handle.createindex(fieldname, dictfilename);
}

DLL_PUBLIC bool deleteindex(const var& filename_or_handle, const var& fieldname) {
	//virtually identical code in createindex and deleteindex
	if (filename_or_handle.index(" ")) {
		var filename2 = filename_or_handle.field(" ", 1);
		var fieldname2 = filename_or_handle.field(" ", 2);
		var indextype2 = filename_or_handle.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return filename2.deleteindex(fieldname2);
	}

	return filename_or_handle.deleteindex(fieldname);
}

DLL_PUBLIC bool begintrans() {
	return var().begintrans();
}

DLL_PUBLIC bool rollbacktrans() {
	return var().rollbacktrans();
}

DLL_PUBLIC bool committrans() {
	return var().committrans();
}

DLL_PUBLIC bool lock(const var& filehandle, const var& key) {
	return (bool)filehandle.lock(key);
}

DLL_PUBLIC void unlock(const var& filehandle, const var& key) {
	filehandle.unlock(key);
}

DLL_PUBLIC void unlockall() {
	var().unlockall();
}

DLL_PUBLIC bool open(const var& filename, var& filehandle) {
	return filehandle.open(filename);
}

DLL_PUBLIC bool open(const var& filename) {
	var filehandle;
	return filehandle.open(filename);
}

DLL_PUBLIC bool read(var& record, const var& filehandle, const var& key) {
	return record.read(filehandle, key);
}

DLL_PUBLIC bool reado(var& record, const var& filehandle, const var& key) {
	return record.reado(filehandle, key);
}

DLL_PUBLIC bool readv(var& record, const var& filehandle, const var& key, const var& fieldnumber) {
	return record.readv(filehandle, key, fieldnumber);
}

DLL_PUBLIC bool write(const var& record, const var& filehandle, const var& key) {
	return record.write(filehandle, key);
}

DLL_PUBLIC bool writev(const var& record, const var& filehandle, const var& key, const int fieldno) {
	return record.writev(filehandle, key, fieldno);
}

DLL_PUBLIC bool updaterecord(const var& record, const var& filehandle, const var& key) {
	return record.updaterecord(filehandle, key);
}

DLL_PUBLIC bool insertrecord(const var& record, const var& filehandle, const var& key) {
	return record.insertrecord(filehandle, key);
}

/*
DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const char* mode)
{
	return key.xlate(filename, fieldno, mode);
}

DLL_PUBLIC var xlate(const var& filename, const var& key, const var& fieldno, const var& mode)
{
	return key.xlate(filename, fieldno, mode);
}
*/

// DLL_PUBLIC var remove(const var& fromstr, var& startx, var& delimiterno)
DLL_PUBLIC var substr2(const var& fromstr, var& startx, var& delimiterno) {
	// return fromstr.remove(startx,delimiterno);
	return fromstr.substr2(startx, delimiterno);
}

DLL_PUBLIC var pickreplace(const var& instring, const int fieldno, const int valueno, const int subvalueno, const var& replacement) {
	return instring.pickreplace(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC var pickreplace(const var& instring, const int fieldno, const int valueno, const var& replacement) {
	return instring.pickreplace(fieldno, valueno, 0, replacement);
}

DLL_PUBLIC var pickreplace(const var& instring, const int fieldno, const var& replacement) {
	return instring.pickreplace(fieldno, 0, 0, replacement);
}

DLL_PUBLIC var extract(const var& instring, const int fieldno, const int valueno, const int subvalueno) {
	return instring.a(fieldno, valueno, subvalueno);
}

DLL_PUBLIC var insert(const var& instring, const int fieldno, const int valueno, const int subvalueno, const var& insertion) {
	return instring.insert(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC var insert(const var& instring, const int fieldno, const int valueno, const var& insertion) {
	return instring.insert(fieldno, valueno, 0, insertion);
}

DLL_PUBLIC var insert(const var& instring, const int fieldno, const var& insertion) {
	return instring.insert(fieldno, 0, 0, insertion);
}

// DLL_PUBLIC var erase(const var& instring, const int fieldno, const int valueno, const int
// subvalueno)
DLL_PUBLIC var remove(const var& instring, const int fieldno, const int valueno, const int subvalueno) {
	// return instring.erase(fieldno, valueno, subvalueno);
	return instring.remove(fieldno, valueno, subvalueno);
}

DLL_PUBLIC var& pickreplacer(var& instring, const int fieldno, const int valueno, const int subvalueno, const var& replacement) {
	return instring.r(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC var& pickreplacer(var& instring, const int fieldno, const int valueno, const var& replacement) {
	return instring.r(fieldno, valueno, 0, replacement);
}

DLL_PUBLIC var& pickreplacer(var& instring, const int fieldno, const var& replacement) {
	return instring.r(fieldno, 0, 0, replacement);
}

DLL_PUBLIC var& inserter(var& instring, const int fieldno, const int valueno, const int subvalueno, const var& insertion) {
	return instring.inserter(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC var& inserter(var& instring, const int fieldno, const int valueno, const var& insertion) {
	return instring.inserter(fieldno, valueno, 0, insertion);
}

DLL_PUBLIC var& inserter(var& instring, const int fieldno, const var& insertion) {
	return instring.inserter(fieldno, 0, 0, insertion);
}

// DLL_PUBLIC var& eraser(var& instring, const int fieldno, const int valueno, const int subvalueno)
DLL_PUBLIC var& remover(var& instring, const int fieldno, const int valueno, const int subvalueno) {
	// return instring.eraser(fieldno, valueno, subvalueno);
	return instring.remover(fieldno, valueno, subvalueno);
}

DLL_PUBLIC bool locate(const var& target, const var& instring) {
	return instring.locate(target);
}

DLL_PUBLIC bool locate(const var& target, const var& instring, var& setting) {
	return instring.locate(target, setting);
}

DLL_PUBLIC bool locate(const var& target, const var& instring, var& setting, const int fieldno, const int valueno) {
	return instring.locate(target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateby(const char* ordercode, const var& target, const var& instring, var& setting) {
	return instring.locateby(ordercode, target, setting);
}

DLL_PUBLIC bool locateby(const char* ordercode, const var& target, const var& instring, var& setting, const int fieldno, const int valueno) {
	return instring.locateby(ordercode, target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateby(const var& ordercode, const var& target, const var& instring, var& setting) {
	return instring.locateby(ordercode, target, setting);
}

DLL_PUBLIC bool locateby(const var& ordercode, const var& target, const var& instring, var& setting, const int fieldno, const int valueno) {
	return instring.locateby(ordercode, target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateusing(const var& usingchar, const var& target, const var& instring) {
	return instring.locateusing(usingchar, target);
}

DLL_PUBLIC bool locateusing(const var& usingchar, const var& target, const var& instring, var& setting) {
	return instring.locateusing(usingchar, target, setting);
}

DLL_PUBLIC bool locateusing(const var& usingchar, const var& target, const var& instring, var& setting, const int fieldno, const int valueno, const int subvalueno) {
	return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);
}

DLL_PUBLIC var sum(const var& instring, const var& sepchar) {
	return instring.sum(sepchar);
}

DLL_PUBLIC var sum(const var& instring) {
	return instring.sum();
}

DLL_PUBLIC var listfiles() {
	return var().listfiles();
}

DLL_PUBLIC var reccount(const var& filename_or_handle) {
	return filename_or_handle.reccount();
}

DLL_PUBLIC var listindexes(const var& filename, const var& fieldname) {
	return var().listindexes(filename, fieldname);
}

// one argument returns the contents of an envvar (empty name returns the whole environment)
DLL_PUBLIC var osgetenv(const var& name = "") {
	var temp = "";
	temp.osgetenv(name);
	return temp;
}

// two argument returns the success/failure to be used in an if statement
// target variable last to be like ossetenv and traditional getenv name, value
// unfortunately different from osread which is the reverse
DLL_PUBLIC bool osgetenv(const var& name, var& value) {
	return value.osgetenv(name);
}

// like "if set xxx=yyy"
DLL_PUBLIC bool ossetenv(const var& name, const var& value) {
	return value.ossetenv(name);
}

DLL_PUBLIC
int exodus_main(int exodus__argc, const char* exodus__argv[], MvEnvironment& mv, int environmentno) {

	// signal/interrupt handlers
	// install_signals();
	var().breakon();

	//	tss_environmentns.reset(new int(0));
	//	global_environments.resize(6);
	//int environmentn = 0;
	mv.init(environmentno);
	// mv.DICT.outputl("DICT=");
	//	global_environments[environmentn] = &mv;

	mv.EXECPATH = getexecpath();
	if (not mv.EXECPATH) {
		if (exodus__argc)
			mv.EXECPATH = var(exodus__argv[0]);
		if (not mv.EXECPATH.index(OSSLASH))
			mv.EXECPATH.splicer(1, 0, oscwd() ^ OSSLASH);
	}
	//"see getting path to current executable" above
	// or use "which EXECPATH somehow like in mvdebug.cpp
	// if (not EXECPATH.index(OSSLASH) && not EXECPATH.index(":"))
	//{
	//	EXECPATH.splicer(0,0,oscwd()^OSSLASH);
	//	if (OSSLASH=="\\")
	//		EXECPATH.converter("/","\\");
	//}
	// leave a global copy where backtrace can get at it
	EXECPATH2 = mv.EXECPATH;

	mv.SENTENCE = "";  // ALN:TODO: hm, again, char->var-> op=(var)

	mv.COMMAND = "";
	mv.OPTIONS = "";

	// reconstructs complete original sentence unfortunately quote marks will have been lost
	// unless escaped needs to go after various exodus definitions how the MSVCRT tokenizes
	// arguments http://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	// http://stackoverflow.com/questions/4094699/how-does-the-windows-command-interpreter-cmd-exe-parse-scripts
	for (int ii = 0; ii < exodus__argc; ++ii) {
		var word = var(exodus__argv[ii]);
		if (ii == 0) {
			word = word.field2(OSSLASH, -1);
			// remove trailing ".exe"
			if (word.lcase().substr(-4) == ".exe")
				word.splicer(-4, 4, "");
		} else
			mv.SENTENCE ^= " ";

		//dont do this because easier to understand bash expansion without it
		// put back quotes if any spaces
		//if (word.index(" "))
		//	word.quoter();

		mv.SENTENCE ^= word;
		mv.COMMAND ^= FM ^ word;
	}
	mv.COMMAND.splicer(1, 1, "");

	// options are in either (XXX) or {XXX} at the end of the command.
	// similar code in exodus_main() and mvprogram.cpp:perform()
	var lastchar = mv.COMMAND[-1];
	// if (lastchar==")")
	//	mv.OPTIONS=mv.COMMAND.field2("(",-1);
	// else if (lastchar=="}")
	//	mv.OPTIONS=mv.COMMAND.field2("{",-1);
	// if (mv.OPTIONS)
	//	mv.COMMAND.splicer(-(len(mv.OPTIONS)+2),len(mv.OPTIONS)+2, "");
	// var lastchar=mv.COMMAND[-1];
	if (lastchar == ")") {
		mv.OPTIONS = "(" ^ mv.COMMAND.field2("(", -1);
	} else if (lastchar == "}")
		mv.OPTIONS = "{" ^ mv.COMMAND.field2("{", -1);
	if (mv.OPTIONS)
		mv.COMMAND.splicer(-(mv.OPTIONS.length()), mv.OPTIONS.length(), "");

	var temp;
	// DBTRACE=osgetenv("EXO_DBTRACE",temp)?1:-1;

	// would have to passed in as a function pointer
	// main2(exodus__argc, exodus__argv);

	//atexit(exodus_atexit);

	return 0;
}

//DLL_PUBLIC
//void exodus_atexit()
//{
//	// done in handle cache destructor now
//	//	var().disconnect();
//}

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

}  // namespace exodus
