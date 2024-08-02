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

// clang-format off

#if EXO_MODULE
	import var;
#else
#	include <cstdlib> //for atexit()
#	include <exodus/var.h>
#	include <exodus/dim.h>
#	include <exodus/rex.h>
#endif
#include <exodus/vardefs.h>

#include <exodus/format.h>

#include <exodus/exoenv.h>

#define EXO_FUNCS_CPP
#include <exodus/exofuncs.h>

#undef DEFAULT_EMPTY
#undef DEFAULT_DOT
#undef DEFAULT_SPACE
#undef DEFAULT_CSPACE
#undef DEFAULT__FM
#undef DEFAULT_0
#undef DEFAULT_1
#undef DEFAULT_M1
#undef DEFAULT_FALSE

#define DEFAULT_EMPTY
#define DEFAULT_DOT
#define DEFAULT_SPACE
#define DEFAULT_CSPACE
#define DEFAULT__FM
#define DEFAULT_0
#define DEFAULT_1
#define DEFAULT_M1
#define DEFAULT_FALSE

namespace exo {

//ND PUBLIC var   var::format(SV format_str) const {
//	return vformat(format_str,*this);
//}

ND PUBLIC var  osgetenv(in envcode DEFAULT_EMPTY) {var envvalue = ""; if (not envvalue.osgetenv(envcode)) {} return envvalue;}
ND PUBLIC bool osgetenv(in code, io value) {return value.osgetenv(code);}
   PUBLIC void ossetenv(in code, in value) {return value.ossetenv(code);}

ND PUBLIC var  ostempdirpath() {return var().ostempdirpath();}
ND PUBLIC var  ostempfilename() {return var().ostempfilename();}

ND PUBLIC bool assigned(in var1) {return var1.assigned();}
ND PUBLIC bool unassigned(in var1) {return !var1.assigned();}

   PUBLIC void move(io fromvar, io tovar) {fromvar.move(tovar);}
   PUBLIC void swap(io var1, io var2) {var1.swap(var2);}

// OS

ND PUBLIC var  date() {return var().date();}
ND PUBLIC var  time() {return var().time();}
ND PUBLIC var  ostime() {return var().ostime();}
ND PUBLIC var  timestamp() {return var().timestamp();}
ND PUBLIC var  timestamp(in date, in time) {return date.timestamp(time);}
//ND PUBLIC var  timedate();

   PUBLIC void ossleep(const int milliseconds) {var().ossleep(milliseconds);}
ND PUBLIC var  oswait(const int milliseconds, SV dirpath) {return var().oswait(milliseconds, dirpath);}

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
ND PUBLIC bool osopen(in osfilepath, io osfilevar, const char* locale DEFAULT_EMPTY) {return osfilevar.osopen(osfilepath, locale);}
   PUBLIC void osclose(in osfilevar) {osfilevar.osclose();}
// Versions where offset is input and output
ND PUBLIC bool osbread(io data, in osfilevar, io offset, const int length) {return data.osbread(osfilevar, offset, length);}
ND PUBLIC bool osbwrite(in data, in osfilevar, io offset) {return data.osbwrite(osfilevar, offset);}
// Versions where offset is const offset e.g. numeric ints
//#ifdef VAR_OSBREADWRITE_CONST_OFFSET
//ND PUBLIC bool osbread(io data, in osfilevar, in offset, const int length) {return data.osbread(osfilevar, const_cast<io>(offset), length);}
//ND PUBLIC bool osbwrite(in data, in osfilevar, in offset) {return data.osbwrite(osfilevar, const_cast<io>(offset));}
//#endif

// Read/Write whole osfile
ND PUBLIC bool oswrite(in data, in osfilepath, const char* codepage DEFAULT_EMPTY) {return data.oswrite(osfilepath, codepage);}
ND PUBLIC bool osread(io data, in osfilepath, const char* codepage DEFAULT_EMPTY) {return data.osread(osfilepath, codepage);}
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
ND PUBLIC var  osread(in osfilepath) {var data; if (data.osread(osfilepath)) return data; else return "";}

ND PUBLIC bool osremove(in ospath) {return ospath.osremove();}
ND PUBLIC bool osrename(in old_ospath, in new_ospath) {return old_ospath.osrename(new_ospath);}
ND PUBLIC bool oscopy(in from_ospath, in to_ospath) {return from_ospath.oscopy(to_ospath);}
ND PUBLIC bool osmove(in from_ospath, in to_ospath) {return from_ospath.osmove(to_ospath);}

ND PUBLIC var  oslist(in path DEFAULT_DOT, SV globpattern DEFAULT_EMPTY, const int mode DEFAULT_0) {return path.oslist(globpattern, mode);}
ND PUBLIC var  oslistf(in filepath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return filepath.oslistf(globpattern);}
ND PUBLIC var  oslistd(in dirpath DEFAULT_DOT, SV globpattern DEFAULT_EMPTY) {return dirpath.oslistd(globpattern);}

ND PUBLIC var  osinfo(in path, const int mode DEFAULT_0) {return path.osinfo(mode);}
ND PUBLIC var  osfile(in filepath) {return filepath.osfile();}
ND PUBLIC var  osdir(in dirpath) {return dirpath.osdir();}


ND PUBLIC bool osmkdir(in dirpath) {return dirpath.osmkdir();}
ND PUBLIC bool osrmdir(in dirpath, const bool evenifnotempty DEFAULT_FALSE) {return dirpath.osrmdir(evenifnotempty);}

ND PUBLIC var  oscwd() {return var().oscwd();}
ND PUBLIC var  oscwd(in dirpath) {return dirpath.oscwd(dirpath);}

   PUBLIC void osflush() {return var().osflush();}
ND PUBLIC var  ospid() {return var().ospid();}
ND PUBLIC var  ostid() {return var().ostid();}

ND PUBLIC bool osshell(in command) {return command.osshell();}
ND PUBLIC bool osshellwrite(in writestr, in command) {return writestr.osshellwrite(command);}
ND PUBLIC bool osshellread(io readstr, in command) {return readstr.osshellread(command);}
ND PUBLIC var  osshellread(in command) {var result = ""; if (not result.osshellread(command)) {} return result;}

ND PUBLIC var  backtrace();

   PUBLIC bool setxlocale(const char* locale) {return var(locale).setxlocale();}
ND PUBLIC var  getxlocale() {return var().getxlocale();}

// MATH

ND PUBLIC bool isnum(in num1) {return num1.isnum();}
ND PUBLIC var  num(in num1) {return num1.num();}

ND PUBLIC var  abs(in num1) {return num1.abs();}
ND PUBLIC var  pwr(in base, in exponent) {return base.pwr(exponent);}
ND PUBLIC var  exp(in power) {return power.exp();}
ND PUBLIC var  sqrt(in num1) {return num1.sqrt();}
ND PUBLIC var  sin(in degrees) {return degrees.sin();}
ND PUBLIC var  cos(in degrees) {return degrees.cos();}
ND PUBLIC var  tan(in degrees) {return degrees.tan();}
ND PUBLIC var  atan(in degrees) {return degrees.atan();}
ND PUBLIC var  loge(in num1) {return num1.loge();}
ND PUBLIC var  mod(in dividend, in divisor) {return dividend.mod(divisor);}
ND PUBLIC var  mod(in dividend, const double divisor) {return dividend.mod(divisor);}
ND PUBLIC var  mod(in dividend, const int divisor) {return dividend.mod(divisor);}

// integer() represents pick int() because int() is reserved word in c/c++
// Note that integer like pick int() is the same as floor()
// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional
// decimal places) to get the usual c/c++ effect use toInt() (although toInt() returns an int
// instead of a var like normal exodus functions)
ND PUBLIC var  integer(in num1) {return num1.integer();}
ND PUBLIC var  floor(in num1) {return num1.floor();}
ND PUBLIC var  round(in num1, const int ndecimals DEFAULT_0) {return num1.round(ndecimals);}

ND PUBLIC var  rnd(const int number) {return var(number).rnd();}
   PUBLIC void initrnd(in seed DEFAULT_0) {seed.initrnd();}

// INPUT

ND PUBLIC var  getprompt();
   PUBLIC void setprompt(in prompt);

   PUBLIC var  input() {var v; v.input(); return v;}

   PUBLIC var  input(in prompt) {var v; v.input(prompt); return v;}

   PUBLIC var  inputn(const int nchars) {var v; v.inputn(nchars); return v;}

ND PUBLIC bool isterminal() {return var().isterminal();}
ND PUBLIC bool hasinput(const int millisecs DEFAULT_0) {return var().hasinput(millisecs);}
ND PUBLIC bool eof() {return var().eof();}
   PUBLIC bool echo(const int on_off) {return var().echo(on_off);}

   PUBLIC void breakon();
   PUBLIC void breakoff();

// SIMPLE STRINGS

ND PUBLIC var  len(in var1) {return var1.len();}
ND PUBLIC var  textlen(in var1) {return var1.textlen();}
ND PUBLIC var  textwidth(in var1) {return var1.textwidth();}

ND PUBLIC var  convert(in instring, SV fromchars, SV tochars) {return instring.convert(fromchars, tochars);}
   PUBLIC io   converter(io iostring, SV fromchars, SV tochars) {return iostring.converter(fromchars, tochars);}

ND PUBLIC var  textconvert(in instring, SV fromchars, SV tochars) {return instring.textconvert(fromchars, tochars);}
   PUBLIC io   textconverter(io iostring, SV fromchars, SV tochars) {return iostring.textconverter(fromchars, tochars);}

ND PUBLIC var  replace(in instring, SV fromstr, SV tostr) {return instring.replace(fromstr, tostr);}
   PUBLIC io   replacer(io iostring, SV fromstr, SV tostr) {return iostring.replacer(fromstr, tostr);}

ND PUBLIC var  replace(in instring, const rex& regex, SV replacement) {return instring.replace(regex, replacement);}
   PUBLIC io   replacer(io iostring, const rex& regex, SV replacement) {return iostring.replacer(regex, replacement);}

ND PUBLIC var  ucase(in instring) {return instring.ucase();}
   PUBLIC io   ucaser(io iostring) {return iostring.ucaser();}

ND PUBLIC var  lcase(in instring) {return instring.lcase();}
   PUBLIC io   lcaser(io iostring) {return iostring.lcaser();}

ND PUBLIC var  tcase(in instring) {return instring.tcase();}
   PUBLIC io   tcaser(io iostring) {return iostring.tcaser();}

ND PUBLIC var  fcase(in instring) {return instring.fcase();}
   PUBLIC io   fcaser(io iostring) { return iostring.fcaser();}

ND PUBLIC var  normalize(in instring) {return instring.normalize();}
   PUBLIC io   normalizer(io iostring) {return iostring.normalizer();}

   PUBLIC io   uniquer(io iostring) {return iostring.uniquer();}
ND PUBLIC var  unique(in instring) {return instring.unique();}

ND PUBLIC var  invert(in instring) {return var(instring).invert();}
   PUBLIC io   inverter(io iostring) {return iostring.inverter();}
ND PUBLIC var  invert(var&& instring) {return instring.inverter();}

ND PUBLIC var  lower(in instring) {return instring.lower();}
   PUBLIC io   lowerer(io iostring) {return iostring.lowerer();}

ND PUBLIC var  raise(in instring) {return instring.raise();}
   PUBLIC io   raiser(io iostring) {return iostring.raiser();}

// PASTER

// 1. paste replace
ND PUBLIC var  paste(in instring, const int pos1, const int length, in str) {return instring.paste(pos1, length, str);}
   PUBLIC io   paster(io iostring, const int pos1, const int length, in str) {return iostring.paster(pos1, length, str);}

//// 2. paste over to end
//PUBLIC io   pasterall(io iostring, const int pos1, in str);
//ND PUBLIC var  pasteall(in instring, const int pos1, in str);

// 3. paste insert at pos1
ND PUBLIC var  paste(in instring, const int pos1, in str) {return instring.paste(pos1, str);}
   PUBLIC io   paster(io iostring, const int pos1, in str) {return iostring.paster(pos1, str);}

// PREFIX
ND PUBLIC var  prefix(in instring, in str) {return instring.prefix(str);}
   PUBLIC io   prefixer(io iostring, in str) {return iostring.prefixer(str);}

ND PUBLIC var  pop(in instring) {return instring.pop();}
   PUBLIC io   popper(io iostring) {return iostring.popper();}


ND PUBLIC var  quote(in instring) {return instring.quote();}
   PUBLIC io   quoter(io iostring) {return iostring.quoter();}

ND PUBLIC var  squote(in instring) {return instring.squote();}
   PUBLIC io   squoter(io iostring) {return iostring.squoter();}

ND PUBLIC var  unquote(in instring) {return instring.unquote();}
   PUBLIC io   unquoter(io iostring) {return iostring.unquoter();}


ND PUBLIC var  fieldstore(in instring, SV sepchar, const int fieldno, const int nfields, in replacement) {return instring.fieldstore(sepchar, fieldno, nfields, replacement);}
   PUBLIC io   fieldstorer(io iostring, SV sepchar, const int fieldno, const int nfields, in replacement) {return iostring.fieldstorer(sepchar, fieldno, nfields, replacement);}


ND PUBLIC var  trim(in instring, SV trimchars DEFAULT_SPACE) {return instring.trim(trimchars);}
ND PUBLIC var  trimfirst(in instring, SV trimchars DEFAULT_SPACE) {return instring.trimfirst(trimchars);}
ND PUBLIC var  trimlast(in instring, SV trimchars DEFAULT_SPACE) {return instring.trimlast(trimchars);}
ND PUBLIC var  trimboth(in instring, SV trimchars DEFAULT_SPACE) {return instring.trimboth(trimchars);}

   PUBLIC io   trimmer(io iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmer(trimchars);}
   PUBLIC io   trimmerfirst(io iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerfirst(trimchars);}
   PUBLIC io   trimmerlast(io iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerlast(trimchars);}
   PUBLIC io   trimmerboth(io iostring, SV trimchars DEFAULT_SPACE) {return iostring.trimmerboth(trimchars);}

ND PUBLIC var  first(in instring) {return instring.first();}
ND PUBLIC var  last(in instring) {return instring.last();}
ND PUBLIC var  first(in instring, const int nbytes) {return instring.first(nbytes);}
ND PUBLIC var  last(in instring, const int nbytes) {return instring.last(nbytes);}

ND PUBLIC var  chr(const int integer) {return var().chr(integer);}
ND PUBLIC var  textchr(const int integer) {return var().textchr(integer);}
ND PUBLIC var  match(in instring, in matchstr, in options DEFAULT_EMPTY) {return instring.match(matchstr, options);}
ND PUBLIC var  seq(in char1) {return char1.seq();}
ND PUBLIC var  textseq(in char1) {return char1.textseq();}
ND PUBLIC var  str(in instring, const int number) {return instring.str(number);}
ND PUBLIC var  space(const int number) {return var(number).space();}
ND PUBLIC var  fcount(in instring, SV substr) {return instring.fcount(substr);}
ND PUBLIC var  count(in instring, SV substr) {return instring.count(substr);}

ND PUBLIC var  substr(in instring, const int startindex) {return instring.b(startindex);}
ND PUBLIC var  substr(in instring, const int startindex, const int length) {return instring.b(startindex, length);}
   PUBLIC io   substrer(io iostring, const int startindex) {return iostring.substrer(startindex);}
   PUBLIC io   substrer(io iostring, const int startindex, const int length) {return iostring.substrer(startindex, length);}

ND PUBLIC bool starts(in instring, SV substr) {return instring.starts(substr);}
ND PUBLIC bool end(in instring, SV substr) {return instring.ends(substr);}
ND PUBLIC bool contains(in instring, SV substr) {return instring.contains(substr);}

ND PUBLIC var  index(in instring, SV substr, const int startindex DEFAULT_1) {return instring.index(substr, startindex);}
ND PUBLIC var  indexn(in instring, SV substr, int occurrence) {return instring.indexn(substr, occurrence);}
ND PUBLIC var  indexr(in instring, SV substr, const int startindex DEFAULT_M1) {return instring.indexr(substr, startindex);}

ND PUBLIC var  field(in instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field(substr, fieldno, nfields);}
ND PUBLIC var  field2(in instring, SV substr, const int fieldno, const int nfields DEFAULT_1) {return instring.field2(substr, fieldno, nfields);}

// STRINGS WITH FIELD MARKS

ND PUBLIC var  substr2(in fromstr, io startindex, io delimiterno) {return fromstr.substr2(startindex, delimiterno);}

ND PUBLIC dim split(in sourcevar, SV sepchar DEFAULT__FM) {return sourcevar.split(sepchar);}
ND PUBLIC var  join(const dim& sourcedim, SV sepchar DEFAULT__FM) {return sourcedim.join(sepchar);}

ND PUBLIC var  pickreplace(in instring, const int fieldno, const int valueno, const int subvalueno, in replacement) {return instring.pickreplace(fieldno, valueno, subvalueno, replacement);}
ND PUBLIC var  pickreplace(in instring, const int fieldno, const int valueno, in replacement) {return instring.pickreplace(fieldno, valueno, replacement);}
ND PUBLIC var  pickreplace(in instring, const int fieldno, in replacement) {return instring.pickreplace(fieldno, replacement);}

ND PUBLIC var  extract(in instring, const int fieldno DEFAULT_0, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.f(fieldno, valueno, subvalueno);}

ND PUBLIC var  insert(in instring, const int fieldno, const int valueno, const int subvalueno, in insertion) {return instring.insert(fieldno, valueno, subvalueno, insertion);}
ND PUBLIC var  insert(in instring, const int fieldno, const int valueno, in insertion) {return instring.insert(fieldno, valueno, insertion);}
ND PUBLIC var  insert(in instring, const int fieldno, in insertion) {return instring.insert(fieldno, insertion);}

ND PUBLIC var  remove(in instring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.remove(fieldno, valueno, subvalueno);}

   PUBLIC io   pickreplacer(io iostring, const int fieldno, const int valueno, const int subvalueno, in replacement) {return iostring.r(fieldno, valueno, subvalueno, replacement);}
   PUBLIC io   pickreplacer(io iostring, const int fieldno, const int valueno, in replacement) {return iostring.r(fieldno, valueno, replacement);}
   PUBLIC io   pickreplacer(io iostring, const int fieldno, in replacement) {return iostring.r(fieldno, replacement);}

   PUBLIC io   inserter(io iostring, const int fieldno, const int valueno, const int subvalueno, in insertion) {return iostring.inserter(fieldno, valueno, subvalueno, insertion);}
   PUBLIC io   inserter(io iostring, const int fieldno, const int valueno, in insertion) {return iostring.inserter(fieldno, valueno, insertion);}
   PUBLIC io   inserter(io iostring, const int fieldno, in insertion) {return iostring.inserter(fieldno, insertion);}

   PUBLIC io   remover(io iostring, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return iostring.remover(fieldno, valueno, subvalueno);}

ND PUBLIC bool locate(in target, in instring) {return instring.locate(target);}
ND PUBLIC bool locate(in target, in instring, io setting) {return instring.locate(target, setting);}
ND PUBLIC bool locate(in target, in instring, io setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locate(target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(const char* ordercode, in target, in instring, io setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(const char* ordercode, in target, in instring, io setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(in ordercode, in target, in instring, io setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(in ordercode, in target, in instring, io setting, const int fieldno, const int valueno DEFAULT_0) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateusing(in usingchar, in target, in instring) {return instring.locateusing(usingchar, target);}
ND PUBLIC bool locateusing(in usingchar, in target, in instring, io setting) {return instring.locateusing(usingchar, target, setting);}
ND PUBLIC bool locateusing(in usingchar, in target, in instring, io setting, const int fieldno, const int valueno DEFAULT_0, const int subvalueno DEFAULT_0) {return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);}

ND PUBLIC var  sum(in instring, SV sepchar) {return instring.sum(sepchar);}
ND PUBLIC var  sum(in instring) {return instring.sum();}

ND PUBLIC var  crop(in instring) {return instring.crop();}
   PUBLIC io   cropper(io iostring) {return iostring.cropper();}

ND PUBLIC var  sort(in instring, SV sepchar DEFAULT__FM) {return instring.sort(sepchar);}
   PUBLIC io   sorter(io iostring, SV sepchar DEFAULT__FM) {return iostring.sorter(sepchar);}

ND PUBLIC var  reverse(in instring, SV sepchar DEFAULT__FM) {return instring.reverse(sepchar);}
   PUBLIC io   reverser(io iostring, SV sepchar DEFAULT__FM) {return iostring.reverser(sepchar);}

ND PUBLIC var  shuffle(in instring, SV sepchar DEFAULT__FM) {return instring.shuffle(sepchar);}
   PUBLIC io   shuffler(io iostring, SV sepchar DEFAULT__FM) {return iostring.shuffler(sepchar);}

ND PUBLIC var  parser(in instring, char sepchar DEFAULT_CSPACE) {return instring.parse(sepchar);}
   PUBLIC io   parser(io iostring, char sepchar DEFAULT_CSPACE) {return iostring.parser(sepchar);}

// DATABASE

ND PUBLIC bool connect(in connectioninfo DEFAULT_EMPTY) {var conn1; return conn1.connect(connectioninfo);}
   PUBLIC void disconnect() {var().disconnect();}
   PUBLIC void disconnectall() {var().disconnectall();}

ND PUBLIC bool dbcreate(in dbname) {return dbname.dbcreate(dbname);}
ND PUBLIC var  dblist() {return var().dblist();}
ND PUBLIC bool dbcopy(in from_dbname, in to_dbname) {return var().dbcopy(from_dbname, to_dbname);}
ND PUBLIC bool dbdelete(in dbname) {return var().dbdelete(dbname);}

ND PUBLIC bool createfile(in dbfilename);
ND PUBLIC bool deletefile(in dbfilename_or_var);
ND PUBLIC bool clearfile(in dbfilename_or_var);
ND PUBLIC bool renamefile(in old_dbfilename, in new_dbfilename) {return old_dbfilename.renamefile(old_dbfilename.f(1), new_dbfilename);}
ND PUBLIC var  listfiles() {return var().listfiles();}

ND PUBLIC var  reccount(in dbfilename_or_var) {return dbfilename_or_var.reccount();}

ND PUBLIC bool createindex(in dbfilename_or_var, in fieldname DEFAULT_EMPTY, in dictfilename DEFAULT_EMPTY);
ND PUBLIC bool deleteindex(in dbfilename_or_var, in fieldname DEFAULT_EMPTY);
ND PUBLIC var  listindex(in dbfilename DEFAULT_EMPTY, in fieldname DEFAULT_EMPTY) {return var().listindex(dbfilename, fieldname);}

ND PUBLIC bool begintrans() {return var().begintrans();}
ND PUBLIC bool statustrans() {return var().statustrans();}
ND PUBLIC bool rollbacktrans() {return var().rollbacktrans();}
ND PUBLIC bool committrans() {return var().committrans();}
   PUBLIC void cleardbcache() {var().cleardbcache();}

ND PUBLIC bool lock(in dbfilevar, in key) {return static_cast<bool>(dbfilevar.lock(key));}
   PUBLIC void unlock(in dbfilevar, in key) {dbfilevar.unlock(key);}
   PUBLIC void unlockall() {var().unlockall();}

ND PUBLIC bool open(in dbfilename, io dbfilevar) {return dbfilevar.open(dbfilename);}
ND PUBLIC bool open(in dbfilename) {return var().open(dbfilename);}

ND PUBLIC bool read(io record, in dbfilevar, in key) {return record.read(dbfilevar, key);}
ND PUBLIC bool readc(io record, in dbfilevar, in key) {return record.readc(dbfilevar, key);}
ND PUBLIC bool readf(io record, in dbfilevar, in key, in fieldnumber) {return record.readf(dbfilevar, key, fieldnumber);}

   PUBLIC bool write(in record, in dbfilevar, in key) {return record.write(dbfilevar, key);}
   PUBLIC bool writec(in record, in dbfilevar, in key) {return record.writec(dbfilevar, key);}
   PUBLIC bool writef(in record, in dbfilevar, in key, const int fieldno) {return record.writef(dbfilevar, key, fieldno);}
ND PUBLIC bool updaterecord(in record, in dbfilevar, in key) {return record.updaterecord(dbfilevar, key);}
ND PUBLIC bool insertrecord(in record, in dbfilevar, in key) {return record.insertrecord(dbfilevar, key);}

ND PUBLIC bool dimread(dim& dimrecord, in dbfilevar, in key) {return dimrecord.read(dbfilevar, key);}
ND PUBLIC bool dimwrite(const dim& dimrecord, in dbfilevar, in key) {return dimrecord.write(dbfilevar, key);}

ND PUBLIC var  xlate(in dbfilename, in key, in fieldno, const char* mode);
ND PUBLIC var  xlate(in dbfilename, in key, in fieldno, in mode);

ND PUBLIC var  lasterror() {return var().lasterror();}
   PUBLIC var  loglasterror(in source DEFAULT_EMPTY) {return var().loglasterror(source);}



ND PUBLIC bool createfile(in dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "").replacer("REVBOOT ", "").replacer("DATAVOL ", "").trimmer();

	return dbfilename2.createfile(dbfilename2);
}

ND PUBLIC bool deletefile(in dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

ND PUBLIC bool clearfile(in dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

ND PUBLIC bool createindex(in dbfilename_or_var, in fieldname, in dictdbfilename) {

	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.createindex(fieldname2, dictdbfilename);
	}

	return dbfilename_or_var.createindex(fieldname, dictdbfilename);
}

ND PUBLIC bool deleteindex(in dbfilename_or_var, in fieldname) {
	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.deleteindex(fieldname2);
	}

	return dbfilename_or_var.deleteindex(fieldname);
}

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
PUBLIC int exodus_main(int exodus_argc, const char* exodus_argv[], ExoEnv& mv, int threadno) {
#pragma GCC diagnostic pop

	// signal/interrupt handlers
	// install_signals();
	//var().breakon();
	breakon();

	//	tss_environmentns.reset(new int(0));
	//	global_environments.resize(6);
	//int environmentn = 0;
	mv.init(threadno);
	// mv.DICT.outputl("DICT=");
	//	global_environments[environmentn] = &mv;

	// Done in exoenv contructors now
	// mv.EXECPATH = getexecpath();

	// No longer done here; to allow EXECPATH to be const
	//if (not mv.EXECPATH) {
	//	if (exodus_argc)
	//		mv.EXECPATH = var(exodus_argv[0]);
	//	if (not mv.EXECPATH.contains(OSSLASH))
	//		mv.EXECPATH.prefixer(oscwd() ^ OSSLASH);
	//}

	mv.SENTENCE = "";  // ALN:TODO: hm, again, char->var-> op=(var)
	mv.COMMAND = "";
	mv.OPTIONS = "";

	// reconstructs complete original sentence unfortunately quote marks will have been lost
	// unless escaped needs to go after various exodus definitions how the MSVCRT tokenizes
	// arguments http://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	// http://stackoverflow.com/questions/4094699/how-does-the-windows-command-interpreter-cmd-exe-parse-scripts
	for (int ii = 0; ii < exodus_argc; ++ii) {
		var word = var(exodus_argv[ii]);
		if (ii == 0) {
			word = word.field2(OSSLASH, -1);
			// remove trailing ".exe"
			if (word.lcase().ends(".exe"))
				word.cutter(-4);
		} else
			mv.SENTENCE ^= " ";

		//dont do this because easier to understand bash expansion without it
		// put back quotes if any spaces
		//if (word.contains(" "))
		//	word.quoter();

		mv.SENTENCE ^= word;
		mv.COMMAND ^= word ^ FM_;
	}
	mv.COMMAND.popper();

	// Cut off OPTIONS from end of COMMAND if present
	// *** SIMILAR code in
	// 1. exofuncs.cpp exodus_main()
	// 2. exoprog.cpp  perform()
	// OPTIONS are in either (AbC) or {AbC} in the last field of COMMAND
	mv.OPTIONS = mv.COMMAND.field2(_FM, -1);
	if ((mv.OPTIONS.starts("{") and mv.OPTIONS.ends("}")) or (mv.OPTIONS.starts("(") and mv.OPTIONS.ends(")"))) {
		// Remove last field of COMMAND TODO fpopper command?
		mv.COMMAND.cutter(-mv.OPTIONS.len() - 1);
		// Remove first { or ( and last ) } chars of OPTIONS
		mv.OPTIONS.cutter(1).popper();
	} else {
		mv.OPTIONS = "";
	}

	var temp;
	// DBTRACE=osgetenv("EXO_DBTRACE",temp)?1:-1;

	// would have to passed in as a function pointer
	// main2(exodus_argc, exodus_argv);

	//atexit(exodus_atexit);

	return 0;
}

//PUBLIC
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

}  // namespace exo
