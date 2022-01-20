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

#include <exodus/mv.h>
#include <exodus/mvenvironment.h>

namespace exodus {

DLL_PUBLIC bool setxlocale(CVR locale) {
	return locale.setxlocale();
}

DLL_PUBLIC var getxlocale() {
	return var().getxlocale();
}

DLL_PUBLIC bool assigned(CVR var1) {
	return var1.assigned();
}

DLL_PUBLIC bool unassigned(CVR var1) {
	return !var1.assigned();
}

DLL_PUBLIC VARREF transfer(VARREF fromvar, VARREF tovar) {
	return fromvar.transfer(tovar);
}

DLL_PUBLIC CVR exchange(CVR var1, CVR var2) {
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
DLL_PUBLIC bool osopen(CVR osfilename, VARREF osfilevar, CVR locale) {
	return osfilevar.osopen(osfilename, locale);
}

// osclose x
DLL_PUBLIC void osclose(CVR osfilevar) {
	osfilevar.osclose();
}

// 4 argument version for statement format
// osbread(data from x at y length z)
// DLL_PUBLIC VARREF osbread(VARREF data, CVR filehandle, const int offset, const int
// length)
//VARREF osbread(VARREF data, CVR filehandle, VARREF offset, const int length,
//	     const bool adjust)
DLL_PUBLIC bool osbread(VARREF data, CVR filehandle, VARREF offset, const int length) {
	return data.osbread(filehandle, offset, length);
}

// 4 argument version for statement format BUT ALLOWING offset TO BE A CONSTANT ie output
// ignored osbread(data from x at y length z) DLL_PUBLIC VARREF osbread(VARREF data, CVR
// filehandle, const int offset, const int length)
//VARREF osbread(VARREF data, CVR filehandle, CVR offset, const int length,
//	     const bool adjust)
DLL_PUBLIC bool osbread(VARREF data, CVR filehandle, CVR offset, const int length) {
	return data.osbread(filehandle, const_cast<VARREF>(offset), length);
}

//DLL_PUBLIC bool osbwrite(CVR data, CVR filehandle, VARREF offset,
//			 const bool adjust = true)
DLL_PUBLIC bool osbwrite(CVR data, CVR filehandle, VARREF offset) {
	return data.osbwrite(filehandle, offset);
}

//DLL_PUBLIC bool osbwrite(CVR data, CVR filehandle, CVR offset,
//			 const bool adjust)
DLL_PUBLIC bool osbwrite(CVR data, CVR filehandle, CVR offset) {
	return data.osbwrite(filehandle, const_cast<VARREF>(offset));
}

// two argument version returns success/failure to be used in if statement
// target variable first to be like "osread x from y else" and "read x from y else"
// unfortunately different from osgetenv which is the reverse
DLL_PUBLIC bool osread(VARREF data, CVR osfilename, CVR codepage) {
	return data.osread(osfilename, codepage);
}

// one argument returns the contents directly to be used in assignments
DLL_PUBLIC var osread(CVR osfilename) {
	var data;
	if (data.osread(osfilename))
		return data;
	else
		return "";
}

// oswrite x on y else
DLL_PUBLIC bool oswrite(CVR data, CVR osfilename, CVR codepage = "") {
	return data.oswrite(osfilename, codepage);
}

// if osdelete x else
DLL_PUBLIC bool osdelete(CVR osfilename) {
	return osfilename.osdelete();
}

// if osrename x to y else
DLL_PUBLIC bool osrename(CVR oldosdir_or_filename, CVR newosdir_or_filename) {
	return oldosdir_or_filename.osrename(newosdir_or_filename);
}

// oscopy x to y
DLL_PUBLIC bool oscopy(CVR fromosfilename, CVR to_osfilename) {
	return fromosfilename.oscopy(to_osfilename);
}

// osmove x to y
DLL_PUBLIC bool osmove(CVR fromosfilename, CVR to_osfilename) {
	return fromosfilename.osmove(to_osfilename);
}

DLL_PUBLIC var oslist(CVR path, CVR wildcard, const int mode) {
	return var().oslist(path, wildcard, mode);
}

DLL_PUBLIC var oslistf(CVR path, CVR wildcard) {
	return var().oslistf(path, wildcard);
}

DLL_PUBLIC var oslistd(CVR path, CVR wildcard) {
	return var().oslistd(path, wildcard);
}

DLL_PUBLIC var osfile(CVR filename) {
	return filename.osfile();
}

DLL_PUBLIC var osdir(CVR dirname) {
	return dirname.osdir();
}

DLL_PUBLIC bool osmkdir(CVR dirname) {
	return dirname.osmkdir();
}

DLL_PUBLIC bool osrmdir(CVR dirname, const bool evenifnotempty) {
	return dirname.osrmdir(evenifnotempty);
}

DLL_PUBLIC var ospid() {
	return var().ospid();
}

DLL_PUBLIC var oscwd() {
	return var().oscwd();
}

DLL_PUBLIC var oscwd(CVR dirname) {
	return dirname.oscwd(dirname);
}

DLL_PUBLIC void osflush() {
	return var().osflush();
}

DLL_PUBLIC var suspend(CVR command) {
	return command.suspend();
}

// version to get return int (conventionally 0 means success, otherwise error)
// example: if (osshell(somecommand)) { ...
DLL_PUBLIC bool osshell(CVR command) {
	return command.osshell();
}

// simple version to write one liner read (declare and assign in one line)
// example: var xx=osshellread("somecommand");
DLL_PUBLIC var osshellread(CVR command) {
	var result;
	result.osshellread(command);
	return result;
}

// versions to be written like a command

// example: osshellread(command,outputresult);
// for now returns nothing since popen cannot write and tell success
DLL_PUBLIC bool osshellread(VARREF readstr, CVR command) {
	return readstr.osshellread(command);
}

// example: osshellwrite(command,inputforcommand);
// for now returns nothing since popen cannot write and tell success
DLL_PUBLIC bool osshellwrite(CVR writestr, CVR command) {
	return writestr.osshellwrite(command);
}

DLL_PUBLIC void stop(CVR text) {
	text.stop(text);
}

DLL_PUBLIC void abort(CVR text) {
	var().abort(text);
}
DLL_PUBLIC void abortall(CVR text) {
	var().abortall(text);
}

// DLL_PUBLIC var perform(CVR command)
//{
//	return command.perform();
//}

// DLL_PUBLIC var execute(CVR command)
//{
//	return command.execute();
//}

// DLL_PUBLIC var chain(CVR command)
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

DLL_PUBLIC var at(CVR column, CVR row) {
	return var().at(column, row);
}

DLL_PUBLIC var getcursor() {
	var cursor;
	cursor.getcursor();
	return cursor;
}

DLL_PUBLIC void setcursor(CVR cursor) {
	cursor.setcursor();
}

DLL_PUBLIC var getprompt() {
	return var().getprompt();
}

DLL_PUBLIC void setprompt(CVR prompt) {
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

DLL_PUBLIC var input(CVR prompt) {
	var v;
	v.input(prompt);
	return v;
}

DLL_PUBLIC var inputn(const int nchars) {
	var v;
	v.inputn(nchars);
	return v;
}

DLL_PUBLIC var len(CVR var1) {
	return var1.len();
}

DLL_PUBLIC var length(CVR var1) {
	return var1.length();
}

DLL_PUBLIC VARREF converter(VARREF instring, CVR oldchars, CVR newchars) {
	return instring.converter(oldchars, newchars);
}

DLL_PUBLIC var convert(CVR instring, CVR oldchars, CVR newchars) {
	return instring.convert(oldchars, newchars);
}

DLL_PUBLIC VARREF textconverter(VARREF instring, CVR oldchars, CVR newchars) {
	return instring.textconverter(oldchars, newchars);
}

DLL_PUBLIC var textconvert(CVR instring, CVR oldchars, CVR newchars) {
	return instring.textconvert(oldchars, newchars);
}

DLL_PUBLIC VARREF swapper(VARREF instring, CVR from, CVR to) {
	return instring.swapper(from, to);
}

DLL_PUBLIC var swap(CVR instring, CVR from, CVR to) {
	var newstring = instring;
	return newstring.swap(from, to);
}

DLL_PUBLIC VARREF replacer(VARREF instring, CVR regexstr, CVR replacementstr, CVR options = "") {
	return instring.replacer(regexstr, replacementstr, options);
}

DLL_PUBLIC var replace(CVR instring, CVR regexstr, CVR replacementstr, CVR options = "") {
	var newstring = instring;
	return newstring.replacer(regexstr, replacementstr, options);
}

DLL_PUBLIC VARREF ucaser(VARREF instring) {
	return instring.ucaser();
}

DLL_PUBLIC var ucase(CVR instring) {
	return instring.ucase();
}

DLL_PUBLIC VARREF lcaser(VARREF instring) {
	return instring.lcaser();
}

DLL_PUBLIC var lcase(CVR instring) {
	return instring.lcase();
}

DLL_PUBLIC VARREF tcaser(VARREF instring) {
	return instring.tcaser();
}

DLL_PUBLIC var tcase(CVR instring) {
	return instring.tcase();
}

DLL_PUBLIC VARREF fcaser(VARREF instring) {
	return instring.fcaser();
}

DLL_PUBLIC var fcase(CVR instring) {
	return instring.fcase();
}

DLL_PUBLIC VARREF normalizer(VARREF instring) {
	return instring.normalizer();
}

DLL_PUBLIC var normalize(CVR instring) {
	return instring.normalize();
}

DLL_PUBLIC var unique(CVR instring) {
	return instring.unique();
}

DLL_PUBLIC VARREF inverter(VARREF instring) {
	return instring.inverter();
}

DLL_PUBLIC var invert(CVR instring) {
	return instring.invert();
}

DLL_PUBLIC VARREF lowerer(VARREF instring) {
	return instring.lowerer();
}

DLL_PUBLIC var lower(CVR instring) {
	return instring.lower();
}

DLL_PUBLIC VARREF raiser(VARREF instring) {
	return instring.raiser();
}

DLL_PUBLIC var raise(CVR instring) {
	return instring.raise();
}

DLL_PUBLIC VARREF splicer(VARREF instring, const int start1, const int length, CVR str) {
	return instring.splicer(start1, length, str);
}

DLL_PUBLIC var splice(CVR instring, const int start1, const int length, CVR str) {
	return instring.splice(start1, length, str);
}

DLL_PUBLIC VARREF popper(VARREF instring) {
	return instring.popper();
}

DLL_PUBLIC var pop(CVR instring) {
	return instring.pop();
}

DLL_PUBLIC VARREF quoter(VARREF instring) {
	return instring.quoter();
}

DLL_PUBLIC var quote(CVR instring) {
	return instring.quote();
}

DLL_PUBLIC VARREF unquoter(VARREF instring) {
	return instring.unquoter();
}

DLL_PUBLIC var unquote(CVR instring) {
	return instring.unquote();
}

DLL_PUBLIC VARREF fieldstorer(VARREF instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement) {
	return instring.fieldstorer(sepchar, fieldno, nfields, replacement);
}

DLL_PUBLIC var fieldstore(CVR instring, CVR sepchar, const int fieldno, const int nfields, CVR replacement) {
	return instring.fieldstore(sepchar, fieldno, nfields, replacement);
}

DLL_PUBLIC var crop(CVR instring) {
	return instring.crop();
}

DLL_PUBLIC var cropper(VARREF instring) {
	return instring.cropper();
}

DLL_PUBLIC VARREF trimmer(VARREF instring, const char* trimchars) {
	return instring.trimmer(trimchars);
}

DLL_PUBLIC var trim(CVR instring, const char* trimchars) {
	return instring.trim(trimchars);
}

DLL_PUBLIC VARREF trimmerf(VARREF instring, const char* trimchars) {
	return instring.trimmerf(trimchars);
}

DLL_PUBLIC var trimf(CVR instring, const char* trimchars) {
	return instring.trimf(trimchars);
}

DLL_PUBLIC VARREF trimmerb(VARREF instring, const char* trimchars) {
	return instring.trimmerb(trimchars);
}

DLL_PUBLIC var trimb(CVR instring, const char* trimchars) {
	return instring.trimb(trimchars);
}

DLL_PUBLIC VARREF trimmer(VARREF instring, CVR trimchars) {
	return instring.trimmer(trimchars);
}

DLL_PUBLIC VARREF trimmer(VARREF instring, CVR trimchars, CVR options) {
	return instring.trimmer(trimchars, options);
}

DLL_PUBLIC var trim(CVR instring, CVR trimchars) {
	return instring.trim(trimchars);
}

DLL_PUBLIC var trim(CVR instring, CVR trimchars, CVR options) {
	return instring.trim(trimchars, options);
}

DLL_PUBLIC VARREF trimmerf(VARREF instring, CVR trimchars) {
	return instring.trimmerf(trimchars);
}

DLL_PUBLIC var trimf(CVR instring, CVR trimchars) {
	return instring.trimf(trimchars);
}

DLL_PUBLIC VARREF trimmerb(VARREF instring, CVR trimchars) {
	return instring.trimmerb(trimchars);
}

DLL_PUBLIC var trimb(CVR instring, CVR trimchars) {
	return instring.trimb(trimchars);
}

DLL_PUBLIC bool matread(dim& dimrecord, CVR filehandle, CVR key) {
	return dimrecord.read(filehandle, key);
}

DLL_PUBLIC bool matwrite(const dim& dimrecord, CVR filehandle, CVR key) {
	return dimrecord.write(filehandle, key);
}

DLL_PUBLIC var split(CVR sourcevar, dim& destinationdim) {
	return destinationdim.split(sourcevar);
}

DLL_PUBLIC dim split(CVR sourcevar) {
	return sourcevar.split();
}

DLL_PUBLIC var join(const dim& sourcedim) {
	return sourcedim.join();
}

DLL_PUBLIC var chr(CVR integer) {
	return var().chr(integer);
}

DLL_PUBLIC var chr(const int integer) {
	return var().chr(integer);
}

DLL_PUBLIC var textchr(CVR integer) {
	return var().textchr(integer);
}

DLL_PUBLIC var textchr(const int integer) {
	return var().textchr(integer);
}

DLL_PUBLIC var match(CVR instring, CVR matchstr, CVR options) {
	return instring.match(matchstr, options);
}

DLL_PUBLIC var seq(CVR char1) {
	return char1.seq();
}

DLL_PUBLIC var textseq(CVR char1) {
	return char1.textseq();
}

DLL_PUBLIC var str(CVR instring, const int number) {
	return instring.str(number);
}

// MATH/BOOLEAN
DLL_PUBLIC var abs(CVR num1) {
	return num1.abs();
}

DLL_PUBLIC var pwr(CVR base, CVR exponent) {
	return base.pwr(exponent);
}

DLL_PUBLIC var exp(CVR exponent) {
	return exponent.exp();
}

DLL_PUBLIC var sqrt(CVR num1) {
	return num1.sqrt();
}

DLL_PUBLIC var sin(CVR degrees) {
	return degrees.sin();
}

DLL_PUBLIC var cos(CVR degrees) {
	return degrees.cos();
}

DLL_PUBLIC var tan(CVR degrees) {
	return degrees.tan();
}

DLL_PUBLIC var atan(CVR degrees) {
	return degrees.atan();
}

DLL_PUBLIC var loge(CVR num1) {
	return num1.loge();
}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
DLL_PUBLIC var integer(CVR num1) {
	return num1.integer();
}

DLL_PUBLIC var floor(CVR num1) {
	return num1.floor();
}

DLL_PUBLIC var round(CVR num1, const int ndecimals = 0) {
	return num1.round(ndecimals);
}

DLL_PUBLIC var rnd(const int number) {
	return var(number).rnd();
}

DLL_PUBLIC void initrnd(CVR seed) {
	var(seed).initrnd();
}

DLL_PUBLIC var mod(CVR dividend, CVR divisor) {
	return dividend.mod(divisor);
}

DLL_PUBLIC var mod(CVR dividend, const int divisor) {
	return dividend.mod(divisor);
}

DLL_PUBLIC var space(const int number) {
	return var(number).space();
}

DLL_PUBLIC var dcount(CVR instring, CVR substrx) {
	return instring.dcount(substrx);
}

DLL_PUBLIC var count(CVR instring, CVR substrx) {
	return instring.count(substrx);
}

DLL_PUBLIC var substr(CVR instring, const int startx) {
	return instring.substr(startx);
}

DLL_PUBLIC var substr(CVR instring, const int startx, const int length) {
	return instring.substr(startx, length);
}

DLL_PUBLIC var substrer(VARREF instring, const int startx) {
	return instring.substrer(startx);
}

DLL_PUBLIC var substrer(VARREF instring, const int startx, const int length) {
	return instring.substrer(startx, length);
}

DLL_PUBLIC var index(CVR instring, CVR substr, const int occurrenceno) {
	return instring.index(substr, occurrenceno);
}

DLL_PUBLIC var index2(CVR instring, CVR substr, const int startcharno) {
	return instring.index2(substr, startcharno);
}

DLL_PUBLIC var field(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx) {
	return instring.field(substrx, fieldnx, nfieldsx);
}

DLL_PUBLIC var field2(CVR instring, CVR substrx, const int fieldnx, const int nfieldsx) {
	return instring.field2(substrx, fieldnx, nfieldsx);
}

/* moved to mvprogram to allow custom conversions like "[DATE]"
DLL_PUBLIC var oconv(CVR instring, const char* conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var oconv(CVR instring, CVR conversion)
{
	return instring.oconv(conversion);
}

DLL_PUBLIC var iconv(CVR instring, const char* conversion)
{
	return instring.iconv(conversion);
}

DLL_PUBLIC var iconv(CVR instring, CVR conversion)
{
	return instring.iconv(conversion);
}
*/

DLL_PUBLIC bool connect(CVR connectioninfo) {
	var conn1;
	return conn1.connect(connectioninfo);
}

DLL_PUBLIC void disconnect() {
	var().disconnect();
}

DLL_PUBLIC void disconnectall() {
	var().disconnectall();
}

DLL_PUBLIC var lasterror() {
	return var().lasterror();
}

DLL_PUBLIC bool dbcreate(CVR dbname) {
	return dbname.dbcreate(dbname);
}

DLL_PUBLIC var dblist() {
	return var().dblist();
}

DLL_PUBLIC bool dbcopy(CVR from_dbname, CVR to_dbname) {
	return var().dbcopy(from_dbname, to_dbname);
}

DLL_PUBLIC bool dbdelete(CVR dbname) {
	return var().dbdelete(dbname);
}

DLL_PUBLIC bool createfile(CVR filename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var filename2 = filename.field("(", 1).trim();

	//remove pickos volume locations
	filename2.swapper("DATA ", "").swapper("REVBOOT ", "").swapper("DATAVOL ", "").trimmer();

	return filename2.createfile(filename2);
}

DLL_PUBLIC bool deletefile(CVR filename_or_handle) {
	//remove options like (S)
	var filename2 = filename_or_handle.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	filename2.swapper("DATA ", "").trimmer();

	return filename_or_handle.deletefile(filename2);
}

DLL_PUBLIC bool clearfile(CVR filename_or_handle) {
	//remove options like (S)
	var filename2 = filename_or_handle.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	filename2.swapper("DATA ", "").trimmer();

	return filename_or_handle.clearfile(filename2);
}

DLL_PUBLIC bool renamefile(CVR filename_or_handle, CVR newfilename) {
	return filename_or_handle.renamefile(filename_or_handle.a(1), newfilename);
}

DLL_PUBLIC bool createindex(CVR filename_or_handle, CVR fieldname, CVR dictfilename) {

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

DLL_PUBLIC bool deleteindex(CVR filename_or_handle, CVR fieldname) {
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

DLL_PUBLIC bool statustrans() {
	return var().statustrans();
}

DLL_PUBLIC bool rollbacktrans() {
	return var().rollbacktrans();
}

DLL_PUBLIC bool committrans() {
	return var().committrans();
}

DLL_PUBLIC bool lock(CVR filehandle, CVR key) {
	return (bool)filehandle.lock(key);
}

DLL_PUBLIC void unlock(CVR filehandle, CVR key) {
	filehandle.unlock(key);
}

DLL_PUBLIC void unlockall() {
	var().unlockall();
}

DLL_PUBLIC bool open(CVR filename, VARREF filehandle) {
	return filehandle.open(filename);
}

DLL_PUBLIC bool open(CVR filename) {
	var filehandle;
	return filehandle.open(filename);
}

DLL_PUBLIC bool read(VARREF record, CVR filehandle, CVR key) {
	return record.read(filehandle, key);
}

DLL_PUBLIC bool reado(VARREF record, CVR filehandle, CVR key) {
	return record.reado(filehandle, key);
}

DLL_PUBLIC bool readv(VARREF record, CVR filehandle, CVR key, CVR fieldnumber) {
	return record.readv(filehandle, key, fieldnumber);
}

DLL_PUBLIC bool write(CVR record, CVR filehandle, CVR key) {
	return record.write(filehandle, key);
}

DLL_PUBLIC bool writev(CVR record, CVR filehandle, CVR key, const int fieldno) {
	return record.writev(filehandle, key, fieldno);
}

DLL_PUBLIC bool updaterecord(CVR record, CVR filehandle, CVR key) {
	return record.updaterecord(filehandle, key);
}

DLL_PUBLIC bool insertrecord(CVR record, CVR filehandle, CVR key) {
	return record.insertrecord(filehandle, key);
}

/*
DLL_PUBLIC var xlate(CVR filename, CVR key, CVR fieldno, const char* mode)
{
	return key.xlate(filename, fieldno, mode);
}

DLL_PUBLIC var xlate(CVR filename, CVR key, CVR fieldno, CVR mode)
{
	return key.xlate(filename, fieldno, mode);
}
*/

// DLL_PUBLIC var remove(CVR fromstr, VARREF startx, VARREF delimiterno)
DLL_PUBLIC var substr2(CVR fromstr, VARREF startx, VARREF delimiterno) {
	// return fromstr.remove(startx,delimiterno);
	return fromstr.substr2(startx, delimiterno);
}

DLL_PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {
	return instring.pickreplace(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC var pickreplace(CVR instring, const int fieldno, const int valueno, CVR replacement) {
	return instring.pickreplace(fieldno, valueno, 0, replacement);
}

DLL_PUBLIC var pickreplace(CVR instring, const int fieldno, CVR replacement) {
	return instring.pickreplace(fieldno, 0, 0, replacement);
}

DLL_PUBLIC var extract(CVR instring, const int fieldno, const int valueno, const int subvalueno) {
	return instring.a(fieldno, valueno, subvalueno);
}

DLL_PUBLIC var insert(CVR instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {
	return instring.insert(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC var insert(CVR instring, const int fieldno, const int valueno, CVR insertion) {
	return instring.insert(fieldno, valueno, 0, insertion);
}

DLL_PUBLIC var insert(CVR instring, const int fieldno, CVR insertion) {
	return instring.insert(fieldno, 0, 0, insertion);
}

// DLL_PUBLIC var erase(CVR instring, const int fieldno, const int valueno, const int
// subvalueno)
DLL_PUBLIC var remove(CVR instring, const int fieldno, const int valueno, const int subvalueno) {
	// return instring.erase(fieldno, valueno, subvalueno);
	return instring.remove(fieldno, valueno, subvalueno);
}

DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR replacement) {
	return instring.r(fieldno, valueno, subvalueno, replacement);
}

DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, const int valueno, CVR replacement) {
	return instring.r(fieldno, valueno, 0, replacement);
}

DLL_PUBLIC VARREF pickreplacer(VARREF instring, const int fieldno, CVR replacement) {
	return instring.r(fieldno, 0, 0, replacement);
}

DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, const int subvalueno, CVR insertion) {
	return instring.inserter(fieldno, valueno, subvalueno, insertion);
}

DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, const int valueno, CVR insertion) {
	return instring.inserter(fieldno, valueno, 0, insertion);
}

DLL_PUBLIC VARREF inserter(VARREF instring, const int fieldno, CVR insertion) {
	return instring.inserter(fieldno, 0, 0, insertion);
}

// DLL_PUBLIC VARREF eraser(VARREF instring, const int fieldno, const int valueno, const int subvalueno)
DLL_PUBLIC VARREF remover(VARREF instring, const int fieldno, const int valueno, const int subvalueno) {
	// return instring.eraser(fieldno, valueno, subvalueno);
	return instring.remover(fieldno, valueno, subvalueno);
}

DLL_PUBLIC bool locate(CVR target, CVR instring) {
	return instring.locate(target);
}

DLL_PUBLIC bool locate(CVR target, CVR instring, VARREF setting) {
	return instring.locate(target, setting);
}

DLL_PUBLIC bool locate(CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
	return instring.locate(target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting) {
	return instring.locateby(ordercode, target, setting);
}

DLL_PUBLIC bool locateby(const char* ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
	return instring.locateby(ordercode, target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting) {
	return instring.locateby(ordercode, target, setting);
}

DLL_PUBLIC bool locateby(CVR ordercode, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno) {
	return instring.locateby(ordercode, target, setting, fieldno, valueno);
}

DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring) {
	return instring.locateusing(usingchar, target);
}

DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting) {
	return instring.locateusing(usingchar, target, setting);
}

DLL_PUBLIC bool locateusing(CVR usingchar, CVR target, CVR instring, VARREF setting, const int fieldno, const int valueno, const int subvalueno) {
	return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);
}

DLL_PUBLIC var sum(CVR instring, CVR sepchar) {
	return instring.sum(sepchar);
}

DLL_PUBLIC var sum(CVR instring) {
	return instring.sum();
}

DLL_PUBLIC var listfiles() {
	return var().listfiles();
}

DLL_PUBLIC var reccount(CVR filename_or_handle) {
	return filename_or_handle.reccount();
}

DLL_PUBLIC var listindexes(CVR filename, CVR fieldname) {
	return var().listindexes(filename, fieldname);
}

// one argument returns the contents of an envvar (empty name returns the whole environment)
DLL_PUBLIC var osgetenv(CVR name = "") {
	var temp = "";
	temp.osgetenv(name);
	return temp;
}

// two argument returns the success/failure to be used in an if statement
// target variable last to be like ossetenv and traditional getenv name, value
// unfortunately different from osread which is the reverse
DLL_PUBLIC bool osgetenv(CVR name, VARREF value) {
	return value.osgetenv(name);
}

// like "if set xxx=yyy"
DLL_PUBLIC bool ossetenv(CVR name, CVR value) {
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
	mv.COMMAND.trimmerb(FM_);

	var temp;
	// DBTRACE=osgetenv("EXO_DBTRACE",temp)?1:-1;

	// would have to passed in as a function pointer
	// main2(exodus__argc, exodus__argv);

	//atexit(exodus_atexit);

	mv.TERMINAL = osgetenv("TERM");

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

int bytes = GetModuleFileName(nullptr, pBuf, len);
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
