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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //for close()
#include <fnmatch.h> //for fnmatch() globbing

//#include <algorithm>  //for count in osrename
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <filesystem>
#include <chrono>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include <codecvt>

//used to convert to and from utf8 in osread and oswrite
#include <boost/locale.hpp>

//#include <boost/date_time/posix_time/posix_time.hpp>

#include <exodus/varimpl.h>
#include <exodus/varoshandle.h>

std::once_flag locale_once_flag;

void locale_once_func() {
	// so wcscoll() sorts a<B etc instead of character number wise where B<a
	// wcscoll currently only used on non-pc/non-mac ... see var::localeAwareCompare
	// so has no effect on those platform for < <= etc
	// only en_US.utf-8 seems to be widely available on all platforms
	//.utf8 causes a utf-8 to wchar conversion when converting string to wstring by iterators
	//.begin() .end() and we want to be able to control conversion more accurately than an
	// automatic conversion en_US is missing from Ubuntu. en_IN is available on Ubuntu and
	// Centos but not MacOSX if (!setlocale (LC_COLLATE, "en_US.utf8"))
	// en_US.utf8 
	//std::cout << std::cout.getloc().name() << std::endl;
	if (!setlocale(LC_ALL, "en_US.utf8")) {
		if (!setlocale(LC_ALL, "C.UTF-8"))
			std::cout << "Cannot setlocale LC_COLLATE to en_US.utf8" << std::endl;
	}
	//std::cout << std::cout.getloc().name() << std::endl;
}

class LocaleOnce {
   public:
	LocaleOnce() { std::call_once(locale_once_flag, locale_once_func); }
};

static LocaleOnce locale_once_static;

namespace exodus {

// this object caches fstream * pointers, to avoid multiple reopening files
// extern VarOSHandlesCache mv_handles_cache;
// Lifecircle of fstream object:
//	- created (by new) and opened in osbread()/osbwrite();
//	- pointer to created object stored in h_cache;
//  - when user calls osclose(), the stream is closed and the object is deleted, and removed from
//  h_cache;
//	- if user forgets to call osclose(), the stream remains opened (alive) until
//		~VarOSHandlesCache for h_cache closes/deletes all registered objects.

std::locale get_locale(const char* locale_name)	// throw (VarError)
{
	// assume is checked prior to calling since this is an internal exodus function
	// THISIS("std::locale get_locale(const char* locale_name)")
	// ISSTRING(locale_name)

	if (not *locale_name || strcmp(locale_name, "utf8") == 0) {
		std::locale old_locale;

		//https://exceptionshub.com/read-unicode-utf-8-file-into-wstring.html
		//wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		std::locale utf8_locale(old_locale,
								new std::codecvt_utf8<wchar_t>);

		return utf8_locale;
	} else {
		try {
			if (*locale_name) {
				std::locale mylocale(locale_name);
				return mylocale;
			} else {
				// dont trust default locale since on osx 10.5.6 it fails!
				std::locale mylocale("C");
				return mylocale;
			}
		} catch (std::runtime_error& re) {
			throw VarError("get_locale cannot create locale for " ^ var(locale_name));
		}
	}
}

bool checknotabsoluterootfolder(std::string dirpath) {
	// safety - will not rename/remove top level folders
	// cwd to top level and delete relative
	// top level folder has only one slash either at the beginning or, on windows, like x:\ .
	// NB copy/same code in osrmdir and osrename
	//if (!dirpath.ends_with(OSSLASH_))
	//	return true;
	if ((!SLASH_IS_BACKSLASH && dirpath[0] == OSSLASH_ &&
		 //std::count(dirpath.begin(), dirpath.end(), OSSLASH_) < 3) ||
		 var(dirpath).count(_OSSLASH) < 3) ||
		(SLASH_IS_BACKSLASH && (dirpath[1] == ':') && (dirpath[2] == OSSLASH_))) {

		std::cerr
			<< "Forced removal/renaming of top two level directories by absolute path is "
			   "not supported for safety but you can use cwd() and relative path."
			<< dirpath << std::endl;

		return false;
	}
	return true;
}

const std::string to_path_string(CVR str1) {
	return str1.toString();
}

const std::string to_oscmd_string(CVR cmd) {

	// while converting from DOS convert all backslashes in first word to forward slashes on
	// linux or leave as if exodus on windows

	// warning if any backslashes unless at least one of them is followed by $ which indicates
	// valid usage as an escape character aiming to recode all old windows-only code
	if (cmd.contains("\\") && !cmd.contains("\\$"))
		cmd.errputl("WARNING BACKSLASHES IN OS COMMAND:");

	return to_path_string(cmd.field(" ", 1)) + " " + cmd.field(" ", 2, 999999).toString();
}

// Returns with trailing OSSLASH
var var::ostempdirpath() const {
	std::error_code error_code;
	//TODO handle error code specifically
	std::string dirpath = std::string(std::filesystem::temp_directory_path(error_code));
	if (dirpath.back() != OSSLASH_)
		dirpath.push_back(OSSLASH_);
	return dirpath;
}

// This function in Ubuntu/g++ returns *sequential* temporary file names which is perhaps not good
// A temporary file is actually created and deleted just to get a name that can be created by other processes.
// Any files created by the caller must be deleted or will hang around in /tmp unless reboot etc.
//
// PROBLEM:
// The file/filename returned could also be created by othet process because we are closing/deleting the file immediately.
//
// SOLUTION:
// TODO 1. Remove this function from exodus
//         REPLACE it with some kind of osopen(anonymoustempfile) that must be osclosed and will exist but can never be referred to by name
//         You could create ANOTHER file with the same name but that would be a different inode/file in the file system.
//      2. Provide osopen without filename to get a filehandle without name that will be automatically deleted when osclose is called.
//
// Linux's temporary file philosophy is to unlink temporary files immediately (remove dir entry) so they cannot be opened by other processes
// and will automatically be fully deleted when the file handle is closed/process terminates.
//

//var var::ostempfilename() const {
//
//	// Linux immediately unlinks the temporary file so it exists without a name.
//	// the actual file is automatically deleted from the file system when the file handle is closed/process is closed.
//	std::FILE* tempfile = std::tmpfile();
//
//	// Linux-specific method to acquire the tmpfile name
//  // BUT FILE WILL NOT EXIST SINCE IT IS UNLINKED IMMEDIATELY AND FULLY DELETED ON FCLOSE BELOW
//	var tempfilename;
//	namespace fs = std::filesystem;
//	tempfilename.var_str = fs::read_symlink(fs::path("/proc/self/fd") / std::to_string(fileno(tempfile)));
//	tempfilename.var_typ = VARTYP_STR;
//	if (auto pos = tempfilename.var_str.find(' '); pos != std::string::npos)
//		tempfilename.var_str.resize(pos);
//
//	// We close the file handle because this function returns temporary file names, not handles
//	// and would leak file handles if we did not.
//	fclose(tempfile);
//
//	return tempfilename;
//
//}

// This function actually creates a file which must be cleaned up by the caller
// The filenames are random names
// TODO is this threadsafe?
var var::ostempfilename() const {

	//https://cpp.hotexamples.com/examples/-/-/mkstemp/cpp-mkstemp-function-examples.html

	// Create a char* template of the temporary file name desired
	// (2*26+10)^6 possible filenames = 56,800,235,584
	// Note that 64 bit hash = 1.84467440737e+19 combinations
	var rvo_tempfilename = this->ostempdirpath() ^ "~exoXXXXXX";
	std::vector<char> buffer(rvo_tempfilename.var_str.begin(), rvo_tempfilename.var_str.end());
	buffer.push_back('\0');

	// Linux only function to create a temporary file
	int fd_or_error;
	if ((fd_or_error = mkstemp(&buffer[0])) == -1)
		throw VarError(var(__PRETTY_FUNCTION__) ^ " - Cannot create tempfilename " ^ rvo_tempfilename.quote());

	// Must close or we will leak file handles because this badly designed function returns a name not a handle
	::close(fd_or_error);

	// Get the actual generated temporary file name
	rvo_tempfilename.var_str = buffer.data();

	return rvo_tempfilename;
}

bool var::osshell() const {

	THISIS("var var::osshell() const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	assertString(function_sig);

	// breakoff();
	int shellresult = system(to_oscmd_string(*this).c_str());
	// breakon();

	return !shellresult;
}

bool var::osshellread(CVR oscmd) {

	THISIS("var var::osshellread() const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	ISSTRING(oscmd)

	// default is to return empty string in any case
	var_str.clear();
	var_typ = VARTYP_STR;

	// fflush?

	//"r" means read
	std::FILE* pfile = popen(to_oscmd_string(oscmd).c_str(), "r");
	// return a code to indicate program failure. but can this ever happen?
	if (pfile == nullptr) {
		this->setlasterror("osshell failed. " ^ oscmd.quote());
		return false;
	}

	// Read everything into a single var string
	char cstr1[4096] = {0x0};
	while (std::fgets(cstr1, sizeof(cstr1), pfile) != nullptr) {
		// std::printf("%s\n", result);
		// cannot trust that standard input is convertable from utf8
		// output.var_str+=wstringfromUTF8((const UTF8*)result,strlen(result));
		//std::string str1 = cstr1;
		//output.var_str += std::string(str1.begin(), str1.end());
		//readstr.var_str += std::string(cstr1);
		var_str += cstr1;
	}

	//return true if no error code
	return !pclose(pfile);
}

bool var::osshellwrite(CVR oscmd) const {

	THISIS("var var::osshellwrite(CVR oscmd) const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	assertString(function_sig);
	ISSTRING(oscmd)

	//"w" means read
	std::FILE* cmd = popen(to_oscmd_string(oscmd).c_str(), "w");
	// return a code to indicate program failure. but can this ever happen?
	if (cmd == nullptr)
		return 1;

	// decided not to convert slashes here .. may be the wrong decision
	fputs(this->var_str.c_str(), cmd);

	// return the process termination status
	return !pclose(cmd);
}

void var::osflush() const {
	std::cout << std::flush;
	return;
}

// optional locale (not the same as codepage)
bool var::osopen(CVR osfilename, const char* locale) const {

	THISIS("bool var::osopen(CVR osfilename, const char* locale)")
	assertDefined(function_sig);
	ISSTRING(osfilename)

	// if reopening an osfile that is already opened then close and reopen
	if (THIS_IS_OSFILE())
		osclose();

	return this->osopenx(osfilename, locale) != 0;
}

static void del_fstream(void* handle) {
	delete (std::fstream*)handle;
}

std::fstream* var::osopenx(CVR osfilename, const char* locale) const {

	// Try to get the cached file handle. the usual case is that you osopen a file before doing
	// osbwrite/osbread Using fstream instead of ofstream so that we can mix reads and writes on
	// the same filehandle
	std::fstream* pmyfile = 0;
	if (THIS_IS_OSFILE()) {
		pmyfile =
			static_cast<std::fstream*>(mv_handles_cache.get_handle(static_cast<int>(var_int), var_str));
		if (pmyfile == 0)  // nonvalid handle
		{
			var_int = 0;
			//			var_typ ^= VARTYP_OSFILE;	// clear bit
			var_typ ^= VARTYP_OSFILE | VARTYP_INT;	// only STR bit should remains
		}
	}

	// If not already cached
	if (pmyfile == 0) {

		// The file has NOT already been opened so open it now with the current default locale
		// and add it to the cache. but normally the filehandle will have been previously opened
		// with osopen and perhaps a specific locale.

		// Delay checking until necessary
		THISIS("bool var::osopenx(CVR osfilename, const char* locale)")
		ISSTRING(osfilename)

		// TODO replace new/delete with some object
		pmyfile = new std::fstream;

		// Apply the locale
		if (locale)
			pmyfile->imbue(get_locale(locale));

		// Open the file for i/o (fail if the file doesnt exist and do NOT delete any
		// existing file) binary and in/out to allow reading and writing from same file
		// handle
		pmyfile->open(to_path_string(osfilename).c_str(), std::ios::out | std::ios::in | std::ios::binary);
		if (!(*pmyfile)) {

			// Cannot open for output. Try to open read-only
			pmyfile->open(to_path_string(osfilename).c_str(),
						  std::ios::in | std::ios::binary);

			// Fail if cannot open read-only
			if (!(*pmyfile)) {
				this->setlasterror(osfilename.quote() ^ " cannot be opened.");
				delete pmyfile;
				return 0;
			}
		}

		// Cache the file handle (we use the int to store the "file number"
		// and NAN to prevent isnum trashing mvint in the possible case that the osfilename
		// is an integer can addhandle fail?
		var_int =
			mv_handles_cache.add_handle(pmyfile, del_fstream, osfilename.var_str);
		var_typ = VARTYP_NANSTR_OSFILE;
		var_str = osfilename.var_str;
	}

	return pmyfile;
}

// on unix or with iconv we could convert to or any character format
// for sequential output we could use popen to progressively read and write with no seek facility
// popen("iconv -f utf-16 -t utf-8 >targetfile","w",filehandle) followed by progressive writes
// popen("iconv -f utf-32 -t utf-16 <sourcefile","r",filehandle) followed by progressive reads
// use utf-16 or utf-32 depending on windows or unix

/* typical code pages on Linux (see "iconv -l")
ISO-8859-2
ISO-8859-3
ISO-8859-5
ISO-8859-6
ISO-8859-7
ISO-8859-8
ISO-8859-9
EUC-JP
EUC-KR
ISO-8859-13
ISO-8859-15
GBK
GB18030
UTF-8
GB2312
BIG5
KOI8-R
KOI8-U
CP1251
TIS-620
WINDOWS-31J
WINDOWS-936
WINDOWS-1250
WINDOWS-1251
WINDOWS-1252
WINDOWS-1253
WINDOWS-1254
WINDOWS-1255
WINDOWS-1256
WINDOWS-1257
WINDOWS-1258
*/

//// no binary conversion is performed on input unless
//// codepage is provided then exodus converts from the
//// specified codepage (not locale) on input to utf-8 internally
//bool var::osread(CVR osfilename, const char* codepage) {
//
//	THISIS("bool var::osread(CVR osfilename, const char* codepage")
//	ISSTRING(osfilename)
//	return osread(to_path_string(osfilename).c_str(), codepage);
//}
//
bool var::osread(const char* osfilename, const char* codepage) {

	THISIS("bool var::osread(const char* osfilename, const char* codepage")
	assertDefined(function_sig);

	// osread returns empty string in any case
	var_str.clear();
	var_typ = VARTYP_STR;

	// get a file structure
	std::ifstream myfile;

	// open in binary (and position "at end" to find the file size with tellg)
	// TODO check myfile.close() on all exit paths or setup an object to do that
	myfile.open(osfilename, std::ios::binary | std::ios::in | std::ios::ate);
	if (!myfile) {
		this->setlasterror("osread failed. " ^ var(osfilename).quote() ^ " does not exist or cannot be accessed.");
		return false;
	}

	// determine the file size since we are going to read it all
	// NB tellg and seekp goes by bytes regardless of normal/wide stream
	// max file size 4GB?
	unsigned int bytesize;
	//	#pragma warning( disable: 4244 )
	// warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of
	// data
	bytesize = (unsigned int)myfile.tellg();

	// if empty file then done ok
	if (bytesize == 0) {
		myfile.close();
		return true;
	}

	// reserve memory - now reading directly into var_str
	// get file size * wchar memory to load the file or fail
	try {
		// emergency memory - will be deleted at } - useful if OOM
		// cancelling this to avoid heap fragmentation and increase performance
		//std::unique_ptr<char[]> emergencymemory(new char[16384]);

		// resize the string to receive the whole file
		var_str.resize(bytesize);
	} catch (std::bad_alloc& ex) {
		myfile.close();
		throw VarOutOfMemory("Could not obtain " ^ var(bytesize * sizeof(char)) ^
							" bytes of memory to read " ^ var(osfilename));
	}

	// read the file into the reserved memory block
	myfile.seekg(0, std::ios::beg);
	// myfile.read (memblock.get(), (unsigned int) bytesize);
	//myfile.read(&var_str[0], (unsigned int)bytesize);
	//c++17 provides non-const access to data() :)
	myfile.read(var_str.data(), (unsigned int)bytesize);

	bool failed = myfile.fail();

	// in case we didnt read the whole file for some reason, remove garbage in the end of the
	// string #pragma warning( disable: 4244 ) warning C4244: '=' : conversion from
	// 'std::streamoff' to 'unsigned int', possible loss of data
	bytesize = (unsigned int)myfile.gcount();
	var_str.resize(bytesize);
	myfile.close();

	// failure can indicate that we didnt get as many characters as requested
	if (failed && !bytesize) {
		this->setlasterror("osread failed. " ^ var(osfilename).quote() ^ " 0 bytes read.");
		return false;
	}

	// ALN:JFI: actually we could use std::string 'tempstr' in place of 'memblock' by hacking
	//	.data member and reserve() or resize(), thus avoiding buffer-to-buffer-copying
	// var_str=std::string(memblock.get(), (unsigned int) bytesize);
	// SJB Done 20190604

	if (*codepage)
		// var_str=boost::locale::conv::to_utf<char>(var_str,"ISO-8859-5")};
		var_str = boost::locale::conv::to_utf<char>(var_str, codepage);

	return true;
}

var var::to_codepage(const char* codepage) const {

	THISIS(
		"bool var::to_codepage(const char* codepage"
		") const")
	assertString(function_sig);

	// NB to_codepage UTF32 adds a BOM at the front eg FFFE0000
	// conversion to UTF32BE and UTF32LE do not add but order is fixed
	// BE Big Endian, Little Endian (Intel)
	// little endian means adding small byte of number comes first
	// see iconv -l | grep utf32 -i

	//from utf8 to codepage
	//if (*codepage)
		return boost::locale::conv::from_utf<char>(var_str, codepage);
	//else
	//	return var_str;
}

var var::from_codepage(const char* codepage) const {

	THISIS(
		"bool var::from_codepage(const char* codepage"
		") const")
	assertString(function_sig);

	//to utf8 from codepage
	//if (codepage)
		return boost::locale::conv::to_utf<char>(var_str, codepage);
	//else
	//	return var_str;
}

// no binary conversion is performed on output unless
// codepage is provided (not locale) then exodus assumes internally
// utf-8 and converts all output to the specified codepage
bool var::oswrite(CVR osfilename, const char* codepage) const {

	THISIS(
		"bool var::oswrite(CVR osfilename, const char* codepage"
		") const")
	assertString(function_sig);
	ISSTRING(osfilename)

	// A file structure
	std::ofstream myfile;

	// Truncate any previous file,
	// TODO check myfile.close() on all exit paths or setup an object to do that
	myfile.open(to_path_string(osfilename).c_str(),
				std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile) {
		this->setlasterror("oswrite failed. " ^ osfilename.quote() ^ " cannot be opened for output.");
		return false;
	}

	// Write out the full string or fail
	if (*codepage) {
		std::string tempstr = boost::locale::conv::from_utf<char>(var_str, codepage);
		myfile.write(tempstr.data(), tempstr.size());
	} else {
		myfile.write(var_str.data(), var_str.size());
	}
	bool failed = myfile.fail();
	myfile.close();
	if (failed)
		this->setlasterror("oswrite failed. " ^ osfilename.quote() ^ " Unknown reason.");
	return !failed;
}

#ifdef VAR_OSBREADWRITE_CONST_OFFSET
// a version that accepts a const offset ie ignores return value
//bool var::osbwrite(CVR osfilevar, CVR offset, const bool adjust) const
// offset -1 appends by starting writing one byte after the current end of the file
// offset -2 updates the last byte of the file.
// etc.
bool var::osbwrite(CVR osfilevar, CVR offset) const {
	return this->osbwrite(osfilevar, const_cast<VARREF>(offset));
}
#endif

//NOTE: unlike osread/oswrite which rely on iconv codepages to do any conversion
//osbread and osbwrite rely on the locale being passed in on the osopen stage

//bool var::osbwrite(CVR osfilevar, VARREF offset, const bool adjust) const
bool var::osbwrite(CVR osfilevar, VARREF offset) const {
	// osfilehandle is just the filename but buffers the "file number" in the mvint too


	THISIS(
		"bool var::osbwrite(CVR osfilevar, VARREF offset) "
		"const")
	assertString(function_sig);
	ISNUMERIC(offset)

	// get the buffered file handle/open on the fly
	std::fstream* pmyfile = osfilevar.osopenx(osfilevar, "");
	if (pmyfile == 0) {
		//throw VarError(this->setlasterror(osfilevar.quote() ^ " osbwrite open failed"));
		this->setlasterror("osbwrite failed. " ^ this->lasterror());
		return false;
	}
	// std::cout << pmyfile->getloc().name();

	// NOTE 1/2 seekp goes by bytes regardless of the fact that it is a wide stream
	// myfile.seekp (offset*sizeof(char));
	// offset should be in bytes except for fixed multibyte code pages like UTF16 and UTF32
	if (offset < 0)
		pmyfile->seekp(offset.toInt() + 1, std::ios_base::end);
	else
		pmyfile->seekp(offset.toInt());

	// NOTE 2/2 but write length goes by number of wide characters (not bytes)
	pmyfile->write(var_str.data(), var_str.size());

	// on windows, fstream will try to convert to current locale codepage so
	// if you are trying to write an exodus string containing a GREEK CAPITAL GAMMA
	// unicode \x0393 and current codepage is *NOT* CP1253 (Greek)
	// then c++ wiofstream cannot convert \x0393 to a single byte (in CP1253)
	if (pmyfile->fail()) {
		// saved in cache, DO NOT CLOSE!
		// myfile.close();
		throw VarError(this->setlasterror(osfilevar.quote() ^ " osbwrite write failed"));
	}

	// pass back the file pointer offset
	offset = static_cast<int>(pmyfile->tellp());

	// although slow, ensure immediate visibility of osbwrites
	pmyfile->flush();

	// saved in cache, DO NOT CLOSE!
	// myfile.close();

	return true;
}

unsigned count_excess_UTF8_bytes(const std::string& str) {

	// Scans backward from the end of string.
	const char* cptr = &str.back();
	int num = 1;
	int numBytesToTruncate = 0;

	for (int i = 0; 6 > i; ++i) {
		numBytesToTruncate += 1;
		if ((*cptr & 0x80) == 0x80) {
			// If char bit starts with 1xxxxxxx
			// It's a part of unicode character!
			// Find the first byte in the unicode character!

			// if ((*cptr & 0xFC) == 0xFC) { if (num == 6) { return 0; } break; }
			// if ((*cptr & 0xF8) == 0xF8) { if (num == 5) { return 0; } break; }

			// If char binary is 11110000, it means it's a 4 bytes long unicode.
			if ((*cptr & 0xF0) == 0xF0) {
				if (num == 4) {
					return 0;
				}
				break;
			}

			// If char binary is 11100000, it means it's a 3 bytes long unicode.
			if ((*cptr & 0xE0) == 0xE0) {
				if (num == 3) {
					return 0;
				}
				break;
			}

			if ((*cptr & 0xC0) == 0xC0) {
				if (num == 2) {
					return 0;
				}
				break;
			}

			num += 1;
		} else {
			// If char bit does not start with 1, nothing to truncate!
			return 0;
		}

		cptr -= 1;

	}  // next char

	return numBytesToTruncate;
}

//NOTE if the locale is not C then any partial non-utf-8 bytes at the end (due to bytesize
//not being an exact number of valid utf-8 code units) are trimmed off the return value
//The new offset is changed to reflect the above and is simply increased by bytesize

bool var::osbread(CVR osfilevar, VARREF offset, const int bytesize) {

	THISIS("bool var::osbread(CVR osfilevar, VARREF offset, const int bytesize")
	assertDefined(function_sig);
	ISNUMERIC(offset)

	// default is to return empty string in any case
	var_str.clear();
	var_typ = VARTYP_STR;

	// strange case request to read 0 bytes
	if (bytesize <= 0)
		return true;

	// get the buffered file handle/open on the fly
	std::fstream* pmyfile = osfilevar.osopenx(osfilevar, "");
	if (pmyfile == 0) {
		this->setlasterror("osbread failed. " ^ this->lasterror());
		return false;
	}
	/*
		//NB all file sizes are in bytes NOT characters despite this being a wide character
	fstream
		// Position get pointer at the end of file, as expected it to be if we open file
	anew pmyfile->seekg(0, std::ios::end); unsigned int maxsize = pmyfile->tellg();

	var(maxsize).outputl("maxsize=");
		//return "" if start reading past end of file
		if ((unsigned long)(int)offset>=maxsize)	// past EOF
			return *this;

	*/
	// seek to the offset
	// if (pmyfile->tellg() != static_cast<long> (offset.var_int))
	{
		if (pmyfile->fail())
			pmyfile->clear();
		// pmyfile->seekg (static_cast<long> (offset.var_int), std::ios::beg);	//
		// 'std::streampos' usually 'long' seekg always seems to result in tellg being -1 in
		// linux (Ubunut 10.04 64bit)
		pmyfile->rdbuf()->pubseekpos(static_cast<uint64_t>(offset.var_int));
	}
	// var((int) pmyfile->tellg()).outputl("2 tellg=");

	// get a memory block to read into
	std::unique_ptr<char[]> memblock(new char[bytesize]);
	//std::unique_ptr memblock(new char[bytesize]);
	if (memblock == 0) {
		throw VarOutOfMemory("osbread could not obtain " ^ var(bytesize * sizeof(char)) ^
							" bytes of memory to read " ^ osfilevar);
	}

	// read the data (converting characters on the fly)
	pmyfile->read(memblock.get(), bytesize);

	// bool failed = pmyfile.fail();
	if (pmyfile->fail()) {
		pmyfile->clear();
		pmyfile->seekg(0, std::ios::end);
	}

	// update the offset function argument
	// if (readsize > 0)
	offset = static_cast<int>(pmyfile->tellg());

	// transfer the memory block to this variable's string
	//(is is possible to read directly into string data() avoiding a memory copy?
	// get the number of CHARACTERS read - utf8 bytes (except ASCII) convert to fewer wchar
	// characters. int readsize = pmyfile->gcount(); #pragma warning( disable: 4244 )
	// warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of
	// data
	var_str.assign(memblock.get(), (unsigned int)pmyfile->gcount());

	// trim off any excess utf8 bytes if utf8
	//	var(pmyfile->getloc().name()).outputl(L"loc name=");;
	if (pmyfile->getloc().name() != "C") {
		auto nextrabytes = count_excess_UTF8_bytes(var_str);
		if (nextrabytes) {
			offset -= static_cast<int>(nextrabytes);
			var_str.resize(var_str.size() - nextrabytes);
		}
	}

	return !var_str.empty();
}

void var::osclose() const {
	if (THIS_IS_OSFILE()) {
		mv_handles_cache.del_handle(static_cast<int>(var_int));
		var_int = 0L;
		var_typ ^= VARTYP_OSFILE | VARTYP_INT;	// only STR bit should remains
	}
	// in all other cases, the files should be closed.
}

bool var::osrename(CVR new_dirpath_or_filepath) const {

	THISIS("bool var::osrename(CVR new_dirpath_or_filepath) const")
	assertString(function_sig);
	ISSTRING(new_dirpath_or_filepath)

	std::string path1 = to_path_string(*this);
	std::string path2 = to_path_string(new_dirpath_or_filepath);

	// Prevent overwrite of existing file
    // *** ACQUIRE ***
	std::ifstream myfile;
	myfile.open(path2.c_str(), std::ios::binary);
	if (myfile) {
		// *** RELEASE ***
		myfile.close();
		this->setlasterror("osrename failed. " ^ new_dirpath_or_filepath.quote() ^ " already exists.");
		return false;
	}

	// Safety
	if (!checknotabsoluterootfolder(path1)) {
		this->setlasterror("osrename failed. " ^ var(path1).quote() ^ " cannot be renamed because it is a top level dir");
		return false;
	}

	if (!checknotabsoluterootfolder(path2)) {
		this->setlasterror("osrename failed. " ^ var(path2).quote() ^ " cannot be overwritten because it is a top level dir");
		return false;
	}

	return !std::rename(path1.c_str(), path2.c_str());
}

bool var::oscopy(CVR new_dirpath_or_filepath) const {

	THISIS("bool var::oscopy(CVR new_dirpath_or_filepath) const")
	assertString(function_sig);
	ISSTRING(new_dirpath_or_filepath)

	std::string path1 = to_path_string(*this);
	std::string path2 = to_path_string(new_dirpath_or_filepath);

    // copy recursively, overwriting
	std::filesystem::path frompathx(path1.c_str());
	std::filesystem::path topathx(path2.c_str());
	//https://en.cppreference.com/w/cpp/filesystem/copy
	std::error_code error_code;
	copy(frompathx, topathx,
		 std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive | std::filesystem::copy_options::copy_symlinks,
		 error_code);

    // Handle error
    if (error_code) {
		this->setlasterror("oscopy failed. " ^ this->quote() ^ " to " ^ new_dirpath_or_filepath.quote());
		return false;
    }

	return true;
}

bool var::osmove(CVR new_dirpath_or_filepath) const {

	THISIS("bool var::osmove(CVR new_dirpath_or_filepath) const")
	assertString(function_sig);
	ISSTRING(new_dirpath_or_filepath)

	std::string path1 = to_path_string(*this);
	std::string path2 = to_path_string(new_dirpath_or_filepath);

	// prevent overwrite of existing file or dir
	// ACQUIRE
	std::ifstream myfile;
	// binary?
	myfile.open(path2.c_str(), std::ios::binary);
	if (myfile) {
		// RELEASE
		myfile.close();
		this->setlasterror("osmove " ^ this->quote() ^ " failed. " ^ new_dirpath_or_filepath.quote() ^ " already exists.");
		return false;
	}

	// Safety
	if (!checknotabsoluterootfolder(path1)) {
	 	this->setlasterror(var(path1).quote() ^ " Cannot be moved or deleted because it is a top level dir");
		return false;
	}
	if (!checknotabsoluterootfolder(path2)) {
	 	this->setlasterror(var(path2).quote() ^ " Cannot be overwritten because it is a top level dir");
		return false;
	}

	// Try to rename but will fail to move across file systems
	//////////////////////////////////////////////////////////
	if (this->osrename(new_dirpath_or_filepath))
		return true;

	// To copy and delete instead of move
	////////////////////////////////////////
	if (!this->oscopy(new_dirpath_or_filepath)) {
		this->setlasterror("osmove failed. " ^ this->lasterror());
		return false;
	}

	// Delete original only after copy to target is successful
	if (!this->osremove()) {

	    // Too dangerous to remove target in case of failure to delete source
		// otherwise delete the target too
		// new_dirpath_or_filepath.osremove();

		this->setlasterror(this->quote() ^ " osmove failed to remove source after copying to " ^ new_dirpath_or_filepath.quote());
		return false;
	}

	return true;
}

bool var::osremove() const {

	assertDefined(__PRETTY_FUNCTION__);
	this->osclose();  // in case this is cached opened file handle

	// Prevent removal of dirs. Use osrmdir for that.
	if (std::filesystem::is_directory(this->toString())) {
		this->setlasterror(this->quote() ^ " osremove failed - is a directory.");
		return false;
	}

	if (std::remove(to_path_string(*this).c_str())) {
		this->setlasterror(this->quote() ^ " failed to osremove");
		return false;
	}
	return true;
}

bool var::osmkdir() const {

	assertString(__PRETTY_FUNCTION__);

	std::filesystem::path pathx(to_path_string(*this).c_str());

//	if (std::filesystem::exists(pathx))	{
//		this->setlasterror(this->quote() ^ " osmkdir failed. Target already exists.");
//		return false;
//	}
//
	std::error_code ec;

	// Creates parent directories automatically
	// https://en.cppreference.com/w/cpp/filesystem/create_directory
	bool created = std::filesystem::create_directories(pathx, ec);
	if (ec or ! created) {
		this->setlasterror(this->quote() ^ " osmkdir failed. " ^ ec.message());
		return false;
	}

	if (!std::filesystem::exists(pathx)) {
		this->setlasterror(this->quote() ^ " osmkdir failed. Target could not be created.");
		return false;
	}

	return true;
}

bool var::osrmdir(bool evenifnotempty) const {

	THISIS("bool var::osrmdir(bool evenifnotempty) const")
	assertString(function_sig);

	// get a handle and return "" if doesnt exist or is NOT a directory
	try {

		std::filesystem::path pathx(to_path_string(*this).c_str());

		if (!std::filesystem::exists(pathx)) {
			this->setlasterror("osrmdir failed. " ^ this->quote() ^ " does not exist.");
			return false;
		}

		if (!std::filesystem::is_directory(pathx)) {
			this->setlasterror("osrmdir failed. " ^ this->quote() ^ " is not a directory.");
			return false;
		}

		if (evenifnotempty) {

			// safety .. simply REFUSE to rm top level folders if not empty
			// find some other way e.g. shell command if you must
			if (!checknotabsoluterootfolder(toString())) {
				this->setlasterror("osrmdir failed " ^ this->quote() ^ " is a top level dir.");
				return false;
			}

			std::filesystem::remove_all(pathx);
		} else
			std::filesystem::remove(pathx);
	} catch (...) {
		this->setlasterror("osrmdir failed. " ^ this->quote() ^ " Unknown cause.");
		return false;
	}

	return true;
}

// utility defined in mvdatetime.cpp
//void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);
//WARNING ACTUALLY DEFINED WITH BOOST POSIX TIME BUT IT IS THE SAME
//void ptime2mvdatetime(const std::filesystem::file_time_type& ptimex, int& mvdate, int& mvtime);
void time_t_to_pick_date_time(time_t time, int* pick_date, int* pick_time);

//convert some clock to time_t (for osfile() and osdir()
//template <typename TP>
//std::time_t to_time_t(TP tp) {
//	using namespace std::chrono;
//	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
//	return system_clock::to_time_t(sctp);
//}

var var::osinfo(const int mode) const {

	assertString(__PRETTY_FUNCTION__);

	// get a handle and return "" if doesnt exist or is NOT a directory
	// std::filesystem::wpath pathx(toTstring((*this)).c_str());
	try {

		// Why use stat instead of std::filesystem?
		// https://eklitzke.org/std-filesystem-last-write-time

		// Using low level interface instead of std::filesystem to avoid multiple call to stat
		// 7,430 ns/op instead of  18,500 ns/op

		//https://stackoverflow.com/questions/21159047/get-the-creation-date-of-file-or-folder-in-c#21159305

		struct stat statinfo;

		// Get stat info or quit
		if (stat(to_path_string(*this).c_str(), &statinfo) != 0)
			return "";

		var size;

		// Quit if is not a required file/dir type
		switch (mode) {

			case 2:
				// Reject non-dirs. Links to dirs are ok.
				if ((statinfo.st_mode & S_IFMT) != S_IFDIR)
					return "";
				// Assume std::string ""
				size.var_typ = VARTYP_STR;
				break;

			case 1:
				// Reject non-files. Links to files are ok.
	 			if ((statinfo.st_mode & S_IFMT) != S_IFREG)
					return "";
				size = static_cast<varint_t>(statinfo.st_size);
				break;

			default:
	 			if ((statinfo.st_mode & S_IFMT) == S_IFREG)
					size = static_cast<varint_t>(statinfo.st_size);
				else
					size = "";
				break;
			}

		// convert c style time_t to pickos integer date and time
		int pick_date, pick_time;
		//boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(statinfo.st_mtime);
		//ptime2mvdatetime(ptimex, pick_date, pick_time);
		time_t_to_pick_date_time(statinfo.st_mtime, &pick_date, &pick_time);

		//file_size() is only available for files not directories
		return size ^ FM ^ pick_date ^ FM ^ pick_time;

	} catch (...) {
		return "";
	}
}

var var::osfile() const {
	return this->osinfo(1);
}

var var::osdir() const {
	return this->osinfo(2);
}

// old slower implementation using std::filesystem
//var var::osdir() const {
//
//	THISIS("var var::osdir() const")
//	assertString(function_sig);
//
//	// get a handle and return "" if doesnt exist or is NOT a directory
//	// std::filesystem::wpath pathx(toTstring((*this)).c_str());
//	try {
//
//		std::filesystem::path pathx(to_path_string(*this).c_str());
//
//		if (!std::filesystem::exists(pathx))
//			return "";
//		if (!std::filesystem::is_directory(pathx))
//			return "";
//
//		//not using low level interface
//		//https://stackoverflow.com/questions/21159047/get-the-creation-date-of-file-or-folder-in-c#21159305
//
//		// SIMILAR CODE IN OSFILE AND OSDIR
//
//		// get last write datetime
//		//std::time_t last_write_time = std::chrono::system_clock::to_time_t(std::filesystem::last_write_time(pathx));
//		std::filesystem::file_time_type file_time = std::filesystem::last_write_time(pathx);
//		std::time_t last_write_time = to_time_t(file_time);
//
//		// convert to posix time
//		boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
//		//std::filesystem::file_time_type ptimex = std::filesystem::last_write_time(pathx);
//
//		// convert posix time to mv date and time
//		int mvdate, mvtime;
//		ptime2mvdatetime(ptimex, mvdate, mvtime);
//
//		//file_size() is only available for files not directories
//		//return int(std::filesystem::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);
//		return FM ^ mvdate ^ FM ^ int(mvtime);
//
//	} catch (...) {
//		return "";
//	}
//}

var var::oslistf(SV globpattern) const {
	return this->oslist(globpattern, 1);
}

var var::oslistd(SV globpattern) const {
	return this->oslist(globpattern, 2);
}

/*
*@param	path	A directory or blank for current working directory. If globpattern if empty then the last part (after any slashes) is used as globpattern.
*@param	globpattern	Optional glob like "*",  "*.*", "*.???" etc. (CASE INSENSITIVE)
*@param	mode	1=files only, 2=directories only, otherwise both.
*@returns		List of directory and/or filenames depending on mode. fm separator
*/
var var::oslist(SV globpattern0, const int mode) const {

	THISIS("var var::oslist(CVR globpattern, const int mode) const")
	assertDefined(function_sig);
	//ISSTRING(path0)
	//ISSTRING(globpattern0)

	// returns an fm separated list of files and/or folders

	// http://www.boost.org/libs/filesystem/example/simple_ls.cpp

	var path = to_path_string(*this);
	var globpattern;
	if (globpattern0.size()) {
		globpattern = globpattern0;
	}
	// file globbing can and must be passed as tail end of path
	// perhaps could use <glob.h> in linux instead of regex
	// TODO bash-like globbing using something like
	// https://github.com/MrGriefs/glob-cpp/blob/main/README.md
	else {

		// If the last part of path looks like a glob
		// (has * or ?) use it.
		globpattern = path.field2(_OSSLASH, -1);
		if (globpattern.convert("*?", "") != globpattern)
			path.firster(path.len() - globpattern.len());
		else
			globpattern = "";
	}

	bool getfiles = true;
	bool getfolders = true;
	if (mode == 1)
		getfolders = false;
	else if (mode == 2)
		getfiles = false;

	var filelist = "";
	std::filesystem::path full_path(std::filesystem::current_path());
	if (path.len()) {
		//full_path = std::filesystem::absolute(std::filesystem::path(path.to_path_string().c_str()));
		std::error_code error_code;
		std::filesystem::path pathx = std::filesystem::path(to_path_string(path).c_str());
		full_path = std::filesystem::absolute(pathx, error_code);
		if (error_code) {
			std::cerr << "'" << to_path_string(path) << "' path : " << error_code.message() << std::endl;
			return filelist;
		}

	}
	if (globpattern == "*")
		globpattern = "";

	//std::clog << "fullpath='" << full_path << "'" <<std::endl;

	// quit if it isnt a folder
	if (!std::filesystem::is_directory(full_path))
		return filelist;

	std::filesystem::directory_iterator end_iter;
	for (std::filesystem::directory_iterator dir_itr(full_path); dir_itr != end_iter; ++dir_itr) {
		try {

			// skip unwanted items
			//TRACE(dir_itr->path().filename().string())
			//TRACE(globpattern)
			if (globpattern && fnmatch(globpattern.var_str.c_str(), dir_itr->path().filename().c_str(), 0) != 0)
				continue;

			// work/efficiently if (std::filesystem::is_directory(dir_itr->status() ) )
			if (std::filesystem::is_directory(*dir_itr)) {
				if (getfolders)
					filelist ^= dir_itr->path().filename().string() ^ FM;
			}
			else {
				if (getfiles)
					filelist ^= dir_itr->path().filename().string() ^ FM;
			}
		} catch (const std::exception& ex) {
			// evade warning: unused variable
			if (false)
				if (ex.what()) {
				}

			//++err_count;
			// std::cout << dir_itr->path().leaf() << " " << ex.what() << std::endl;
		}
	}

	// delete last separator
	filelist.popper();

	return filelist;
}

bool var::oscwd(CVR newpath) const {

	THISIS("var var::oscwd(const char* newpath) const")
	// doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	assertDefined(function_sig);	 // not needed if *this not used
	ISSTRING(newpath)

	try {
		std::filesystem::current_path(to_path_string(newpath));
		//std::filesystem::current_path(newpath);
	} catch (...) {
		// filesystem error: cannot set current path: No such file or directory
		// ignore all errors
		this->setlasterror(var(newpath).quote() ^ " oscwd failed - unknown cause.");
		return false;
	}

	return true;
}

var var::oscwd() const {

	THISIS("var var::oscwd() const")
	// doesnt use *this - should syntax be changed to ossetcwd? and osgetcwd()?
	assertDefined(function_sig);	 // not needed if *this not used

	// TODO consider using complete() or system_complete()
	//"[Note: When portable behavior is required, use complete(). When operating system
	// dependent behavior is required, use system_complete()."

	std::string currentpath = std::filesystem::current_path().string();

	return var(currentpath).convert("/", _OSSLASH);
}

}  // namespace exodus
