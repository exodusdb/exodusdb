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

namespace exo {

//ND PUBLIC var   var::format(SV format_str) const {
//	return vformat(format_str,*this);
//}

ND PUBLIC var  osgetenv(SV envcode /*=""*/) {var envvalue = ""; if (not envvalue.osgetenv(envcode)) {} return envvalue;}
ND PUBLIC bool osgetenv(SV code, out value) {return value.osgetenv(code);}
   PUBLIC void ossetenv(SV code, in value) {return value.ossetenv(code);}

ND PUBLIC var  ostempdir() {return var().ostempdir();}
ND PUBLIC var  ostempfile() {return var().ostempfile();}

ND PUBLIC bool assigned(in var1) {return var1.assigned();}
ND PUBLIC bool unassigned(in var1) {return !var1.assigned();}

ND PUBLIC var  move(io fromvar) {return fromvar.move();}
ND PUBLIC var  clone(in fromvar) {return fromvar.clone();}

   PUBLIC void swap(io var1, io var2) {var1.swap(var2);}
ND PUBLIC var  dump(in var1) {return var1.dump();}

ND PUBLIC var  or_default(in var1, in defaultvar) {return var1.or_default(defaultvar);}
   PUBLIC void defaulter(io iovar1, in defaultvar) {iovar1.defaulter(defaultvar);}

ND PUBLIC int  getprecision() {return var::getprecision();}
   PUBLIC int  setprecision(int newprecision) {return var::setprecision(newprecision);}

// OS

ND PUBLIC var  date() {return var::date();}
ND PUBLIC var  time() {return var::time();}
ND PUBLIC var  ostime() {return var::ostime();}
ND PUBLIC var  ostimestamp() {return var::ostimestamp();}
ND PUBLIC var  ostimestamp(in date, in time) {return date.ostimestamp(time);}

   PUBLIC void ossleep(const int milliseconds) {var::ossleep(milliseconds);}
ND PUBLIC var  oswait(SV file_dir_list, const int milliseconds) {return var(file_dir_list).oswait(milliseconds);}

// 4 argument version for statement format
// osbread(data from x at y length z)
// Read/write osfile at specified offset. Must open/close.
ND PUBLIC bool osopen(in osfilepath, out osfilevar, const bool utf8 /*=true*/) {return osfilevar.osopen(osfilepath, utf8);}
   PUBLIC void osclose(in osfilevar) {osfilevar.osclose();}
// Versions where offset is input and output
ND PUBLIC bool osbread(out data, in osfilevar, io offset, const int length) {return data.osbread(osfilevar, offset, length);}
ND PUBLIC bool osbwrite(in data, in osfilevar, io offset) {return data.osbwrite(osfilevar, offset);}
// Versions where offset is const offset e.g. numeric ints
//#ifdef VAR_OSBREADWRITE_CONST_OFFSET
//ND PUBLIC bool osbread(out data, in osfilevar, in offset, const int length) {return data.osbread(osfilevar, const_cast<io>(offset), length);}
//ND PUBLIC bool osbwrite(in data, in osfilevar, in offset) {return data.osbwrite(osfilevar, const_cast<io>(offset));}
//#endif

// Read/Write var from/into whole osfile
ND PUBLIC bool oswrite(in data, in osfilepath, const char* codepage /*=""*/) {return data.oswrite(osfilepath, codepage);}
ND PUBLIC bool osread(out data, in osfilepath, const char* codepage /*=""*/) {return data.osread(osfilepath, codepage);}
// Simple version without codepage returns the contents or "" if file cannot be read
// one argument returns the contents directly to be used in assignments
ND PUBLIC var  osread(in osfilepath) {var data; if (data.osread(osfilepath)) return data; else return "";}

// Read/Write dim from/into whole osfile
ND PUBLIC bool oswrite(const dim& data, in osfilepath, const char* codepage /*=""*/) {return data.oswrite(osfilepath, codepage);}
ND PUBLIC bool osread(dim& data, in osfilepath, const char* codepage /*=""*/) {return data.osread(osfilepath, codepage);}

ND PUBLIC bool osremove(in ospath) {return ospath.osremove();}
ND PUBLIC bool osrename(in old_ospath, in new_ospath) {return old_ospath.osrename(new_ospath);}
ND PUBLIC bool oscopy(in from_ospath, in to_ospath) {return from_ospath.oscopy(to_ospath);}
ND PUBLIC bool osmove(in from_ospath, in to_ospath) {return from_ospath.osmove(to_ospath);}

ND PUBLIC var  oslist(in path /*="."*/, SV globpattern /*=""*/, const int mode /*=0*/) {return path.oslist(globpattern, mode);}
ND PUBLIC var  oslistf(in filepath /*="."*/, SV globpattern /*=""*/) {return filepath.oslistf(globpattern);}
ND PUBLIC var  oslistd(in dirpath /*="."*/, SV globpattern /*=""*/) {return dirpath.oslistd(globpattern);}

ND PUBLIC var  osinfo(in path, const int mode /*=0*/) {return path.osinfo(mode);}
ND PUBLIC var  osfile(in filepath) {return filepath.osfile();}
ND PUBLIC var  osdir(in dirpath) {return dirpath.osdir();}


ND PUBLIC bool osmkdir(in dirpath) {return dirpath.osmkdir();}
ND PUBLIC bool osrmdir(in dirpath, const bool evenifnotempty /*=false*/) {return dirpath.osrmdir(evenifnotempty);}

ND PUBLIC var  oscwd() {return var::oscwd();}
ND PUBLIC var  oscwd(in dirpath) {return var::oscwd(dirpath);}

   PUBLIC void osflush() {return var().osflush();}
ND PUBLIC var  ospid() {return var::ospid();}
ND PUBLIC var  ostid() {return var::ostid();}

ND PUBLIC bool osshell(in command) {return command.osshell();}
ND PUBLIC bool osshellwrite(in writestr, in command) {return writestr.osshellwrite(command);}
ND PUBLIC bool osshellread(out readstr, in command) {return readstr.osshellread(command);}
ND PUBLIC var  osshellread(in command) {var result = ""; if (not result.osshellread(command)) {} return result;}

ND PUBLIC var  backtrace();

ND PUBLIC bool setxlocale(const char* newlocalecode) {return var::setxlocale(newlocalecode);}
ND PUBLIC var  getxlocale() {return var::getxlocale();}

ND PUBLIC var  from_codepage(in instring, const char* codepage) {return instring.from_codepage(codepage);}
ND PUBLIC var  to_codepage(in instring, const char* codepage) {return instring.to_codepage(codepage);}

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
ND PUBLIC var  round(in num1, const int ndecimals /*=0*/) {return num1.round(ndecimals);}

ND PUBLIC var  rnd(const int number) {return var(number).rnd();}
   PUBLIC void initrnd(in seed /*=0*/) {seed.initrnd();}

// INPUT

ND PUBLIC var  getprompt();
   PUBLIC void setprompt(in prompt);

   PUBLIC var  
input(in prompt) {var v; bool _ = v.input(prompt); return v;}

   PUBLIC var  inputn(const int nchars) {var v; v.inputn(nchars); return v;}
   PUBLIC var  keypressed(const bool wait /*= false*/) {var v; v.keypressed(wait); return v;}

ND PUBLIC bool isterminal(const int in_out_err) {return var().isterminal(in_out_err);}
ND PUBLIC bool hasinput(const int millisecs /*=0*/) {return var().hasinput(millisecs);}
ND PUBLIC bool eof() {return var().eof();}
   PUBLIC bool echo(const int on_off) {return var().echo(on_off);}

   PUBLIC void breakon();
   PUBLIC void breakoff();

// SIMPLE STRINGS

ND PUBLIC var  len(in var1) {return var1.len();}
ND PUBLIC var  textlen(in var1) {return var1.textlen();}
ND PUBLIC var  textwidth(in var1) {return var1.textwidth();}

ND PUBLIC bool empty(in var1) {return var1.empty();}

ND PUBLIC var  convert(  in instring, SV fromchars, SV tochars) {return instring.convert(fromchars, tochars);}
   PUBLIC IO   converter(io iostring, SV fromchars, SV tochars) {iostring.converter(fromchars, tochars); return IOSTRING;}

ND PUBLIC var  textconvert(  in instring, SV fromchars, SV tochars) {return instring.textconvert(fromchars, tochars);}
   PUBLIC IO   textconverter(io iostring, SV fromchars, SV tochars) {iostring.textconverter(fromchars, tochars); return IOSTRING;}

ND PUBLIC var  replace( in instring, SV fromstr, SV tostr) {return instring.replace(fromstr, tostr);}
   PUBLIC IO   replacer(io iostring, SV fromstr, SV tostr) {iostring.replacer(fromstr, tostr); return IOSTRING;}

ND PUBLIC var  replace( in instring, const rex& regex, SV replacement) {return instring.replace(regex, replacement);}
   PUBLIC IO   replacer(io iostring, const rex& regex, SV replacement) {iostring.replacer(regex, replacement); return IOSTRING;}

ND PUBLIC var  replace( in instring, const rex& regex, SomeFunction(in match_str)) {return instring.replace(regex, sf);}
   PUBLIC IO   replacer(io iostring, const rex& regex, SomeFunction(in match_str)) {iostring.replacer(regex, sf); return IOSTRING;}

ND PUBLIC var  ucase( in instring) {return instring.ucase();}
   PUBLIC IO   ucaser(io iostring) {iostring.ucaser(); return IOSTRING;}

ND PUBLIC var  lcase( in instring) {return instring.lcase();}
   PUBLIC IO   lcaser(io iostring) {iostring.lcaser(); return IOSTRING;}

ND PUBLIC var  tcase( in instring) {return instring.tcase();}
   PUBLIC IO   tcaser(io iostring) {iostring.tcaser(); return IOSTRING;}

ND PUBLIC var  fcase( in instring) {return instring.fcase();}
   PUBLIC IO   fcaser(io iostring) {iostring.fcaser(); return IOSTRING;}

ND PUBLIC var  normalize( in instring) {return instring.normalize();}
   PUBLIC IO   normalizer(io iostring) {iostring.normalizer(); return IOSTRING;}

ND PUBLIC var  unique( in instring) {return instring.unique();}
   PUBLIC IO   uniquer(io iostring) {iostring.uniquer(); return IOSTRING;}

   PUBLIC IO   inverter(io iostring) {iostring.inverter(); return IOSTRING;}
ND PUBLIC var  invert(  in instring) {return var(instring).invert();}
ND PUBLIC var  invert(  var&& instring) {instring.inverter(); return instring;}

ND PUBLIC var  lower(  in instring) {return instring.lower();}
   PUBLIC IO   lowerer(io iostring) {iostring.lowerer(); return IOSTRING;}

ND PUBLIC var  raise( in instring) {return instring.raise();}
   PUBLIC IO   raiser(io iostring) {iostring.raiser(); return IOSTRING;}

// CUT

ND PUBLIC var  cut(   in instring, const int length) {return instring.cut(length);}
          IO   cutter(io instring, const int length) {return instring.cutter(length);}

// PASTER

// 1. paste replace
ND PUBLIC var  paste( in instring, const int pos1, const int length, in str) {return instring.paste(pos1, length, str);}
   PUBLIC IO   paster(io iostring, const int pos1, const int length, in str) {iostring.paster(pos1, length, str); return IOSTRING;}

//// 2. paste over to end
//PUBLIC io   pasterall(io iostring, const int pos1, in str);
//ND PUBLIC var  pasteall(in instring, const int pos1, in str);

// 3. paste insert at pos1
ND PUBLIC var  paste( in instring, const int pos1, in str) {return instring.paste(pos1, str);}
   PUBLIC IO   paster(io iostring, const int pos1, in str) {iostring.paster(pos1, str); return IOSTRING;}

// PREFIX
ND PUBLIC var  prefix(  in instring, in str) {return instring.prefix(str);}
   PUBLIC IO   prefixer(io iostring, in str) {iostring.prefixer(str); return IOSTRING;}

ND PUBLIC var  pop(   in instring) {return instring.pop();}
   PUBLIC IO   popper(io iostring) {iostring.popper(); return IOSTRING;}


ND PUBLIC var  quote( in instring) {return instring.quote();}
   PUBLIC IO   quoter(io iostring) {iostring.quoter(); return IOSTRING;}

ND PUBLIC var  squote( in instring) {return instring.squote();}
   PUBLIC IO   squoter(io iostring) {iostring.squoter(); return IOSTRING;}

ND PUBLIC var  unquote( in instring) {return instring.unquote();}
   PUBLIC IO   unquoter(io iostring) {iostring.unquoter(); return IOSTRING;}


ND PUBLIC var  fieldstore( in instring, SV delimiter, const int fieldno, const int nfields, in replacement) {return instring.fieldstore(delimiter, fieldno, nfields, replacement);}
   PUBLIC IO   fieldstorer(io iostring, SV delimiter, const int fieldno, const int nfields, in replacement) {iostring.fieldstorer(delimiter, fieldno, nfields, replacement); return IOSTRING;}


ND PUBLIC var  trim(     in instring, SV trimchars /*="*/) {return instring.trim(trimchars);}
ND PUBLIC var  trimfirst(in instring, SV trimchars /*="*/) {return instring.trimfirst(trimchars);}
ND PUBLIC var  trimlast( in instring, SV trimchars /*="*/) {return instring.trimlast(trimchars);}
ND PUBLIC var  trimboth( in instring, SV trimchars /*="*/) {return instring.trimboth(trimchars);}

   PUBLIC IO   trimmer(     io iostring, SV trimchars /*="*/) {iostring.trimmer(trimchars); return IOSTRING;}
   PUBLIC IO   trimmerfirst(io iostring, SV trimchars /*="*/) {iostring.trimmerfirst(trimchars); return IOSTRING;}
   PUBLIC IO   trimmerlast( io iostring, SV trimchars /*="*/) {iostring.trimmerlast(trimchars); return IOSTRING;}
   PUBLIC IO   trimmerboth( io iostring, SV trimchars /*="*/) {iostring.trimmerboth(trimchars); return IOSTRING;}

ND PUBLIC var  first(in instring) {return instring.first();}
ND PUBLIC var  last( in instring) {return instring.last();}
ND PUBLIC var  first(in instring, const int nbytes) {return instring.first(nbytes);}
ND PUBLIC var  last( in instring, const int nbytes) {return instring.last(nbytes);}

ND PUBLIC var  chr(        const int char_no) {return var::chr(char_no);}
ND PUBLIC var  textchr(    const int unicode_char_no) {return var::textchr(unicode_char_no);}
ND PUBLIC var  textchrname(const int unicode_char_no) {return var::textchrname(unicode_char_no);}

// Match
ND PUBLIC var  match(in instring, SV regex_str, SV regex_options /*=""*/) {return instring.match(regex_str, regex_options);}
ND PUBLIC var  match(in instring, const rex& regex) {return instring.match(regex);}

// Search
ND PUBLIC var  search(in instring, SV regex_str, io startchar1, SV regex_options /*=""*/) {return instring.search(regex_str, startchar1, regex_options);}
// Ditto starting from first char
ND PUBLIC var  search(in instring, SV regex_str) {return instring.search(regex_str);}
// Ditto given a rex
ND PUBLIC var  search(in instring, const rex& regex, io startchar1) {return instring.search(regex, startchar1);}
// Ditto starting from first char.
ND PUBLIC var  search(in instring, const rex& regex) {return instring.search(regex);}

ND PUBLIC var  ord(    in char1) {return char1.ord();}
ND PUBLIC var  textord(in char1) {return char1.textord();}
ND PUBLIC var  str(    in instring, const int number) {return instring.str(number);}
ND PUBLIC var  space(  const int nspaces) {return var::space(nspaces);}
ND PUBLIC var  fcount( in instring, SV substr) {return instring.fcount(substr);}
ND PUBLIC var  count(  in instring, SV substr) {return instring.count(substr);}

ND PUBLIC var  substr(  in instring, const int startindex) {return instring.substr(startindex);}
ND PUBLIC var  substr(  in instring, const int startindex, const int length) {return instring.b(startindex, length);}
   PUBLIC IO   substrer(io iostring, const int startindex) {iostring.substrer(startindex); return IOSTRING;}
   PUBLIC IO   substrer(io iostring, const int startindex, const int length) {iostring.substrer(startindex, length); return IOSTRING;}

ND PUBLIC var  substr(in instring, const int startindex, SV delimiterchars, out pos2){return instring.substr(startindex, delimiterchars, pos2);}
//   PUBLIC IO   substrer(io iostring, const int startindex, SV delimiterchars, out pos2) {iostring.substrer(startindex, delimiterchars, pos2) return IOSTRING;}

ND PUBLIC bool starts  (in instring, SV substr) {return instring.starts(substr);}
ND PUBLIC bool ends(    in instring, SV substr) {return instring.ends(substr);}
ND PUBLIC bool contains(in instring, SV substr) {return instring.contains(substr);}

ND PUBLIC var  index( in instring, SV substr, const int startindex /*=1*/) {return instring.index(substr, startindex);}
ND PUBLIC var  indexn(in instring, SV substr, int occurrence) {return instring.indexn(substr, occurrence);}
ND PUBLIC var  indexr(in instring, SV substr, const int startindex /*=-1*/) {return instring.indexr(substr, startindex);}

ND PUBLIC var  field (in instring, SV substr, const int fieldno, const int nfields /*=1*/) {return instring.field(substr, fieldno, nfields);}
ND PUBLIC var  field2(in instring, SV substr, const int fieldno, const int nfields /*=1*/) {return instring.field(substr, fieldno, nfields);}

// STRINGS WITH FIELD MARKS

ND PUBLIC var  substr2(in fromstr, io startindex, io delimiterno) {return fromstr.substr2(startindex, delimiterno);}

ND PUBLIC dim split(   in sourcevar, SV delimiter /*=_FM*/) {return sourcevar.split(delimiter);}
ND PUBLIC var  join(const dim& sourcedim, SV delimiter /*=_FM*/) {return sourcedim.join(delimiter);}

ND PUBLIC var  update(in instring, const int fieldno, const int valueno, const int subvalueno, in replacement) {return instring.update(fieldno, valueno, subvalueno, replacement);}
ND PUBLIC var  update(in instring, const int fieldno, const int valueno, in replacement) {return instring.update(fieldno, valueno, replacement);}
ND PUBLIC var  update(in instring, const int fieldno, in replacement) {return instring.update(fieldno, replacement);}

ND PUBLIC var  extract(in instring, const int fieldno /*=0*/, const int valueno /*=0*/, const int subvalueno /*=0*/) {return instring.f(fieldno, valueno, subvalueno);}

ND PUBLIC var  insert( in instring, const int fieldno, const int valueno, const int subvalueno, in insertion) {return instring.insert(fieldno, valueno, subvalueno, insertion);}
ND PUBLIC var  insert( in instring, const int fieldno, const int valueno, in insertion) {return instring.insert(fieldno, valueno, insertion);}
ND PUBLIC var  insert( in instring, const int fieldno, in insertion) {return instring.insert(fieldno, insertion);}

ND PUBLIC var  remove( in instring, const int fieldno, const int valueno /*=0*/, const int subvalueno /*=0*/) {return instring.remove(fieldno, valueno, subvalueno);}

   PUBLIC IO   updater(io iostring, const int fieldno, const int valueno, const int subvalueno, in replacement) {iostring.updater(fieldno, valueno, subvalueno, replacement); return IOSTRING;}
   PUBLIC IO   updater(io iostring, const int fieldno, const int valueno, in replacement) {iostring.updater(fieldno, valueno, replacement); return IOSTRING;}
   PUBLIC IO   updater(io iostring, const int fieldno, in replacement) {iostring.updater(fieldno, replacement); return IOSTRING;}

   PUBLIC IO   inserter(io iostring, const int fieldno, const int valueno, const int subvalueno, in insertion) {iostring.inserter(fieldno, valueno, subvalueno, insertion); return IOSTRING;}
   PUBLIC IO   inserter(io iostring, const int fieldno, const int valueno, in insertion) {iostring.inserter(fieldno, valueno, insertion); return IOSTRING;}
   PUBLIC IO   inserter(io iostring, const int fieldno, in insertion) {iostring.inserter(fieldno, insertion); return IOSTRING;}

   PUBLIC IO   remover(io iostring, const int fieldno, const int valueno /*=0*/, const int subvalueno /*=0*/) {iostring.remover(fieldno, valueno, subvalueno); return IOSTRING;}

ND PUBLIC var  locate( in target, in instring) {return instring.locate(target);}
ND PUBLIC bool locate( in target, in instring, out setting) {return instring.locate(target, setting);}
ND PUBLIC bool locate( in target, in instring, out setting, const int fieldno, const int valueno /*=0*/) {return instring.locate(target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(const char* ordercode, in target, in instring, out setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(const char* ordercode, in target, in instring, out setting, const int fieldno, const int valueno /*=0*/) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateby(in ordercode, in target, in instring, out setting) {return instring.locateby(ordercode, target, setting);}
ND PUBLIC bool locateby(in ordercode, in target, in instring, out setting, const int fieldno, const int valueno /*=0*/) {return instring.locateby(ordercode, target, setting, fieldno, valueno);}

ND PUBLIC bool locateusing(in usingchar, in target, in instring) {return instring.locateusing(usingchar, target);}
ND PUBLIC bool locateusing(in usingchar, in target, in instring, out setting) {return instring.locateusing(usingchar, target, setting);}
ND PUBLIC bool locateusing(in usingchar, in target, in instring, out setting, const int fieldno, const int valueno /*=0*/, const int subvalueno /*=0*/) {return instring.locateusing(usingchar, target, setting, fieldno, valueno, subvalueno);}

ND PUBLIC var  sum(    in instring, SV delimiter) {return instring.sum(delimiter);}
ND PUBLIC var  sum(    in instring) {return instring.sum();}
ND PUBLIC var  sumall( in instring) {return instring.sumall();}

ND PUBLIC var  hash(   in instring, const std::uint64_t modulus) {return instring.hash(modulus);}

ND PUBLIC var  crop(   in instring) {return instring.crop();}
   PUBLIC IO   cropper(io iostring) {iostring.cropper(); return IOSTRING;}

ND PUBLIC var  sort(   in instring, SV delimiter /*=_FM*/) {return instring.sort(delimiter);}
   PUBLIC IO   sorter( io iostring, SV delimiter /*=_FM*/) {iostring.sorter(delimiter); return IOSTRING;}

ND PUBLIC var  reverse( in instring, SV delimiter /*=_FM*/) {return instring.reverse(delimiter);}
   PUBLIC IO   reverser(io iostring, SV delimiter /*=_FM*/) {iostring.reverser(delimiter); return IOSTRING;}

ND PUBLIC var  shuffle( in instring, SV delimiter /*=_FM*/) {return instring.shuffle(delimiter);}
   PUBLIC IO   shuffler(io iostring, SV delimiter /*=_FM*/) {iostring.shuffler(delimiter); return IOSTRING;}

ND PUBLIC var  parse(   in instring, char sepchar /*=''*/) {return instring.parse(sepchar);}
   PUBLIC IO   parser(  io iostring, char sepchar /*=''*/) {iostring.parser(sepchar);}

// DATABASE

ND PUBLIC bool connect( in connectioninfo /*=""*/) {var conn1; return conn1.connect(connectioninfo);}
   PUBLIC void disconnect() {var().disconnect();}
   PUBLIC void disconnectall() {var().disconnectall();}

ND PUBLIC bool sqlexec( in sqlcmd) {return var().sqlexec(sqlcmd);}
ND PUBLIC bool sqlexec( in sqlcmd, out response) {return var().sqlexec(sqlcmd, response);}

ND PUBLIC bool attach(  in filenames) {return var("").attach(filenames);}

ND PUBLIC bool dbcreate(in dbname, in from_dbname) {return dbname.dbcreate(dbname, from_dbname);}
ND PUBLIC var  dblist() {return var().dblist();}
ND PUBLIC bool dbcopy(  in from_dbname, in to_dbname) {return var().dbcopy(from_dbname, to_dbname);}
ND PUBLIC bool dbdelete(in dbname) {return var().dbdelete(dbname);}

ND PUBLIC bool createfile(in dbfilename);
ND PUBLIC bool deletefile(in dbfilename_or_var);
ND PUBLIC bool clearfile( in dbfilename_or_var);
ND PUBLIC bool renamefile(in old_dbfilename, in new_dbfilename) {return old_dbfilename.renamefile(old_dbfilename.f(1), new_dbfilename);}
ND PUBLIC var  listfiles() {return var().listfiles();}

ND PUBLIC var  reccount(   in dbfilename_or_var) {return dbfilename_or_var.reccount();}

ND PUBLIC bool createindex(in dbfilename_or_var, in fieldname /*=""*/, in dictfilename /*=""*/);
ND PUBLIC bool deleteindex(in dbfilename_or_var, in fieldname /*=""*/);
ND PUBLIC var  listindex(  in dbfilename /*=""*/, in fieldname /*=""*/) {return var().listindex(dbfilename, fieldname);}

ND PUBLIC bool begintrans()    {return var().begintrans();}
ND PUBLIC bool statustrans()   {return var().statustrans();}
ND PUBLIC bool rollbacktrans() {return var().rollbacktrans();}
ND PUBLIC bool committrans()   {return var().committrans();}

ND PUBLIC bool lock(      in dbfile, in key) {return static_cast<bool>(dbfile.lock(key));}
   PUBLIC bool unlock(    in dbfile, in key) {return dbfile.unlock(key);}
   PUBLIC bool unlockall( in conn) {return conn.unlockall();}

   PUBLIC void clearcache(in conn) {return conn.clearcache();}

ND PUBLIC bool open( in dbfilename, io dbfile) {return dbfile.open(dbfilename);}
ND PUBLIC bool open( in dbfilename) {return var().open(dbfilename);}

   PUBLIC void close(in dbfile) {return dbfile.close();}

ND PUBLIC bool read( out record, in dbfile, in key) {return record.read(dbfile, key);}
ND PUBLIC bool readc(out record, in dbfile, in key) {return record.readc(dbfile, key);}
ND PUBLIC bool readf(out field, in dbfile, in key, in fieldnumber) {return field.readf(dbfile, key, fieldnumber);}

   PUBLIC void write(       in record, in dbfile, in key) {record.write(dbfile, key);}
   PUBLIC void writec(      in record, in dbfile, in key) {record.writec(dbfile, key);}
   PUBLIC void writef(      in record, in dbfile, in key, const int fieldno) {record.writef(dbfile, key, fieldno);}
ND PUBLIC bool updaterecord(in record, in dbfile, in key) {return record.updaterecord(dbfile, key);}
ND PUBLIC bool updatekey(   in dbfile, in key, in newkey) {return dbfile.updatekey(key, newkey);}
ND PUBLIC bool insertrecord(in record, in dbfile, in key) {return record.insertrecord(dbfile, key);}
//ND PUBLIC bool deleterecord(in dbfile, in key) {return dbfile.deleterecord(key);}
ND PUBLIC bool deletec(     in dbfile, in key) {return dbfile.deletec(key);}

//ND PUBLIC bool dimread(dim& dimrecord, in dbfile, in key) {return dimrecord.read(dbfile, key);
//   PUBLIC void dimwrite(const dim& dimrecord, in dbfile, in key) {dimrecord.write(dbfile, key);}
ND PUBLIC bool read(       dim& dimrecord, in dbfile, in key) {return dimrecord.read(dbfile, key);}
   PUBLIC void write(const dim& dimrecord, in dbfile, in key) {dimrecord.write(dbfile, key);}

ND PUBLIC var  xlate(in dbfilename, in key, in fieldno, const char* mode);
ND PUBLIC var  xlate(in dbfilename, in key, in fieldno, in mode);

ND PUBLIC var  lasterror() {return var::lasterror();}
   PUBLIC void loglasterror(in source /*=""*/) {return var::loglasterror(source);}

ND PUBLIC var  version() {return var::version();}

ND PUBLIC bool createfile(in dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "");
	dbfilename2.replacer("REVBOOT ", "");
	dbfilename2.replacer("DATAVOL ", "");
	dbfilename2.trimmer();

	return dbfilename2.createfile(dbfilename2);
}

ND PUBLIC bool deletefile(in dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "");
	dbfilename2.trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

ND PUBLIC bool clearfile(in dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "");
	dbfilename2.trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

ND PUBLIC bool createindex(in dbfilename_or_var, in fieldname, in dictdbfilename) {

	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		let dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		let indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.createindex(fieldname2, dictdbfilename);
	}

	return dbfilename_or_var.createindex(fieldname, dictdbfilename);
}

ND PUBLIC bool deleteindex(in dbfilename_or_var, in fieldname) {
	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		let dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		let indextype2 = dbfilename_or_var.field(" ", 3);
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
			word = word.field(OSSLASH, -1);
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
	mv.OPTIONS = mv.COMMAND.field(_FM, -1);
	if ((mv.OPTIONS.starts("{") and mv.OPTIONS.ends("}")) or (mv.OPTIONS.starts("(") and mv.OPTIONS.ends(")"))) {
		// Remove last field of COMMAND TODO fpopper command?
		mv.COMMAND.cutter(-mv.OPTIONS.len() - 1);
		// Remove first { or ( and last ) } chars of OPTIONS
		mv.OPTIONS.cutter(1);
		mv.OPTIONS.popper();
	} else {
		mv.OPTIONS = "";
	}

	let temp;
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
