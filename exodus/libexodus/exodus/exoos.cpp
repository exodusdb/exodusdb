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

#include <exodus/var.h>
#include <exodus/exoenv.h>
//#include <exofuncs.h>

namespace exodus {

PUBLIC bool setxlocale(const char* locale) {
	return var(locale).setxlocale();
}

PUBLIC var getxlocale() {
	return var().getxlocale();
}

PUBLIC var date() {
	return var().date();
}

PUBLIC var time() {
	return var().time();
}

PUBLIC var ostime() {
	return var().ostime();
}

PUBLIC var timestamp() {
	return var().timestamp();
}
//PUBLIC var timedate() {
//	return var().timedate();
//}


PUBLIC void ossleep(const int milliseconds) {
	var().ossleep(milliseconds);
}

PUBLIC var oswait(const int milliseconds, SV dirpath) {
	return var().oswait(milliseconds, dirpath);
}


// osopen x to y else
PUBLIC bool osopen(CVR osfilepath, VARREF osfilevar, const char* locale) {
	return osfilevar.osopen(osfilepath, locale);
}

// osclose x
PUBLIC void osclose(CVR osfilevar) {
	osfilevar.osclose();
}

//// 4 argument version for statement format
//// osbread(data from x at y length z)
//// PUBLIC VARREF osbread(VARREF data, CVR osfilevar, const int offset, const int
//// length)
////VARREF osbread(VARREF data, CVR osfilevar, VARREF offset, const int length,
////	     const bool adjust)
//PUBLIC bool osbread(VARREF data, CVR osfilevar, VARREF offset, const int length) {
//	return data.osbread(osfilevar, offset, length);
//}
//
//#ifdef VAR_OSBREADWRITE_CONST_OFFSET
//// 4 argument version for statement format BUT ALLOWING offset TO BE A CONSTANT ie output
//// ignored osbread(data from x at y length z) PUBLIC VARREF osbread(VARREF data, CVR
//// osfilevar, const int offset, const int length)
////VARREF osbread(VARREF data, CVR osfilevar, CVR offset, const int length,
////	     const bool adjust)
//PUBLIC bool osbread(VARREF data, CVR osfilevar, CVR offset, const int length) {
//	return data.osbread(osfilevar, const_cast<VARREF>(offset), length);
//}
//#endif
//
////PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset,
////			 const bool adjust = true)
//PUBLIC bool osbwrite(CVR data, CVR osfilevar, VARREF offset) {
//	return data.osbwrite(osfilevar, offset);
//}
//
//#ifdef VAR_OSBREADWRITE_CONST_OFFSET
////PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset,
////			 const bool adjust)
//PUBLIC bool osbwrite(CVR data, CVR osfilevar, CVR offset) {
//	return data.osbwrite(osfilevar, const_cast<VARREF>(offset));
//}
//#endif

// two argument version returns success/failure to be used in if statement
// target variable first to be like "osread x from y else" and "read x from y else"
// unfortunately different from osgetenv which is the reverse
PUBLIC bool osread(VARREF data, CVR osfilepath, const char* codepage) {
	return data.osread(osfilepath, codepage);
}

// one argument returns the contents directly to be used in assignments
PUBLIC var osread(CVR osfilepath) {
	var data;
	if (data.osread(osfilepath))
		return data;
	else
		return "";
}

// oswrite x on y else
PUBLIC bool oswrite(CVR data, CVR osfilepath, const char* codepage) {
	return data.oswrite(osfilepath, codepage);
}

// if osremove x else
PUBLIC bool osremove(CVR path) {
	return path.osremove();
}

// if osrename x to y else
PUBLIC bool osrename(CVR old_path, CVR new_path) {
	return old_path.osrename(new_path);
}

// oscopy x to y
PUBLIC bool oscopy(CVR from_path, CVR to_path) {
	return from_path.oscopy(to_path);
}

// osmove x to y
PUBLIC bool osmove(CVR from_path, CVR to_path) {
	return from_path.osmove(to_path);
}


PUBLIC var oslist(CVR path, SV globpattern, const int mode) {
	return path.oslist(globpattern, mode);
}

PUBLIC var oslistf(CVR path, SV globpattern) {
	return path.oslistf(globpattern);
}

PUBLIC var oslistd(CVR path, SV globpattern) {
	return path.oslistd(globpattern);
}


PUBLIC var osinfo(CVR path, const int mode = 0) {
	return path.osinfo(mode);
}

PUBLIC var osfile(CVR filepath) {
	return filepath.osfile();
}

PUBLIC var osdir(CVR dirpath) {
	return dirpath.osdir();
}


PUBLIC bool osmkdir(CVR dirpath) {
	return dirpath.osmkdir();
}

PUBLIC bool osrmdir(CVR dirpath, const bool even_if_not_empty) {
	return dirpath.osrmdir(even_if_not_empty);
}

PUBLIC var ospid() {
	return var().ospid();
}

PUBLIC var oscwd() {
	return var().oscwd();
}

PUBLIC var oscwd(CVR dirpath) {
	return dirpath.oscwd(dirpath);
}

PUBLIC void osflush() {
	return var().osflush();
}

// version to get return int (conventionally 0 means success, otherwise error)
// example: if (osshell(somecommand)) { ...
PUBLIC bool osshell(CVR command) {
	return command.osshell();
}

// simple version to write one liner read (declare and assign in one line)
// example: var xx=osshellread("somecommand");
PUBLIC var osshellread(CVR command) {
	var result;
	result.osshellread(command);
	return result;
}

// versions to be written like a command

// example: osshellread(command,outputresult);
// for now returns nothing since popen cannot write and tell success
PUBLIC bool osshellread(VARREF datain, CVR command) {
	return datain.osshellread(command);
}

PUBLIC var ostempfilename() {
	return var().ostempfilename();
}

PUBLIC var ostempdirpath() {
	return var().ostempdirpath();
}

// example: osshellwrite(command,inputforcommand);
// for now returns nothing since popen cannot write and tell success
PUBLIC bool osshellwrite(CVR dataout, CVR command) {
	return dataout.osshellwrite(command);
}

// one argument returns the contents of an envvar (empty name returns the whole environment)
PUBLIC var osgetenv(CVR envcode) {
	var envvalue = "";
	envvalue.osgetenv(envcode);
	return envvalue;
}

// two argument returns the success/failure to be used in an if statement
// target variable last to be like ossetenv and traditional getenv name, value
// unfortunately different from osread which is the reverse
PUBLIC bool osgetenv(CVR code, VARREF value) {
	return value.osgetenv(code);
}

// like "if set xxx=yyy"
PUBLIC void ossetenv(CVR code, CVR value) {
	value.ossetenv(code);
}

}  // namespace exodus
