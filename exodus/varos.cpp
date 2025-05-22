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

#if EXO_MODULE
	import std;
#else
#	include <sys/types.h>
#	include <algorithm>  //for count in osrename
#	include <fstream>
#	include <iostream>
#	include <locale>
#	include <mutex>
#	include <filesystem>
#	include <chrono>
#	include <memory>
#	include <string>
#	include <algorithm>
#	include <vector>
#	include <codecvt>
#endif

#include <fnmatch.h>  //for fnmatch() globbing
#include <sys/stat.h>
#include <unistd.h>	 //for close()

// used to convert to and from utf8 in osread and oswrite
// #include <boost/detail/utf8_codecvt_facet.hpp>
#include <boost/locale.hpp>

// #include <boost/date_time/posix_time/posix_time.hpp>

#include <exodus/varimpl.h>
#include <exodus/varoshandle.h>

namespace exo {

// setgloballocale object constructs to standard on program startup
// for correct collation
static class SetGlobalLocale {
   public:
	SetGlobalLocale() {

		//		// function to dump i/o stream locales for debugging
		//		auto dump = [](std::string is_was) {
		//			std::clog << "thrd locale " << is_was << " " << uselocale((locale_t) 0) << '\n';
		//			std::clog << "glob locale " << is_was << " " << std::locale("").name().c_str() << '\n';
		//			std::clog << "cout locale " << is_was << " " << std::cout.getloc().name() << '\n';
		//			std::clog << "cin  locale " << is_was << " " << std::cin.getloc().name() << '\n';
		//			std::clog << "cerr locale " << is_was << " " << std::cerr.getloc().name() << '\n';
		//		};

		// Dump io stream locales before standardisation
		// dump("was");
		/*
			glob locale was C.UTF-8
			cout locale was C
			cin  locale was C
			cerr locale was C
		*/

		// Set the global default locale
		if (!setlocale(LC_ALL, "en_US.utf8")) {
			if (!setlocale(LC_ALL, "C.UTF-8"))
				std::clog << "Cannot setlocale LC_COLLATE to en_US.utf8 or C.UTF-8" << std::endl;
		}

		// The calling thread's current locale is set to the global locale determined by setlocale(3).
		// uselocale(LC_GLOBAL_LOCALE);

		// Imbue io streams with exodus standard locale
		std::cout.imbue(std::locale(""));
		std::cin.imbue(std::locale(""));
		std::clog.imbue(std::locale(""));
		std::cerr.imbue(std::locale(""));

		// Dump io stream locales after standardisation
		// dump("is ");
		/*
			glob locale is C.UTF-8
			cout locale is C.UTF-8
			cin  locale is C.UTF-8
			cerr locale is C.UTF-8
		*/
	}

} setgloballocale;

// this object caches fstream * pointers, to avoid multiple reopening files
// extern VarOSfstreamHandles thread_fstream_handles;
// Lifecircle of fstream object:
//	- created (by new) and opened in osbread()/osbwrite();
//	- pointer to created object stored in h_cache;
//  - when user calls osclose(), the stream is closed and the object is deleted, and removed from
//  h_cache;
//	- if user forgets to call osclose(), the stream remains opened (alive) until
//		~VarOSfstreamHandles for h_cache closes/deletes all registered objects.

static std::locale get_locale(const char* locale_name)	// throw (VarError)
{
	// assume is checked prior to calling since this is an internal exodus function
	// THISIS("std::locale get_locale(const char* locale_name)")
	// ISSTRING(locale_name)

	if (not *locale_name || strcmp(locale_name, "utf8") == 0) {
		std::locale old_locale;

		// https://exceptionshub.com/read-unicode-utf-8-file-into-wstring.html
		// wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		//		std::locale utf8_locale(old_locale,
		//								new std::codecvt_utf8<wchar_t>);
		std::locale utf8_locale(old_locale, new std::codecvt<wchar_t, char, std::mbstate_t>);

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
			throw VarError("get_locale cannot create locale for " ^ var(locale_name).quote() ^ re.what());
		}
	}
}

static bool checknotabsoluterootfolder(std::string dirpath) {
	// safety - will not rename/remove top level folders
	// cwd to top level and delete relative
	// top level folder has only one slash either at the beginning or, on windows, like x:\ .
	// NB copy/same code in osrmdir and osrename
	// if (!dirpath.ends_with(OSSLASH_))
	//	return true;

	if ((!SLASH_IS_BACKSLASH && dirpath[0] == OSSLASH_ && var(dirpath).count(_OSSLASH) < 3) ||
		(SLASH_IS_BACKSLASH && (dirpath[1] == ':') && (dirpath[2] == OSSLASH_))) {

		// Exclude tmp dir
		if (dirpath.starts_with(var::ostempdir().toString()))
			return true;

		// Only check existing dirs
		if (not var(dirpath).osdir())
			return true;

		std::cerr
			<< "Forced removal/renaming of top two level directories by absolute path is "
			   "not supported for safety but you can use cwd() and relative path."
			<< dirpath << std::endl;

		return false;
	}
	return true;
}

static const std::string to_path_string(in str1) {
	return str1.toString();
}

static const std::string to_oscmd_string(in cmd) {

	//	// while converting from DOS convert all backslashes in first word to forward slashes on
	//	// linux or leave as if exodus on windows
	//
	//	// warning if any backslashes unless at least one of them is followed by $ which indicates
	//	// valid usage as an escape character aiming to recode all old windows-only code
	//	if (cmd.contains("\\") && !cmd.contains("\\$"))
	//		cmd.errputl("WARNING BACKSLASHES IN OS COMMAND:");

	return to_path_string(cmd.field(" ", 1)) + " " + cmd.field(" ", 2, 999999).toString();
}

// Returns with trailing OSSLASH
var var::ostempdir() {
	std::error_code error_code;
	// TODO handle error code specifically
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
//         REPLACE it with some kind of  osopen(anonymoustempfile) that must be osclosed and will exist but can never be referred to by name
//         You could create ANOTHER file with the same name but that would be a different inode/file in the file system.
//      2. Provide osopen without filename to get a filehandle without name that will be automatically deleted when osclose is called.
//
// Linux's temporary file philosophy is to unlink temporary files immediately (remove dir entry) so they cannot be opened by other processes
// and will automatically be fully deleted when the file handle is closed/process terminates.
//

// var  var::ostempfile() const {
//
//	// Linux immediately unlinks the temporary file so it exists without a name.
//	// the actual file is automatically deleted from the file system when the file handle is closed/process is closed.
//	std::FILE* tempfile = std::tmpfile();
//
//	// Linux-specific method to acquire the tmpfile name
//   // BUT FILE WILL NOT EXIST SINCE IT IS UNLINKED IMMEDIATELY AND FULLY DELETED ON FCLOSE BELOW
//	let tempfilename;
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
// }

// This function actually creates a file which must be cleaned up by the caller
// The filenames are random names
// TODO is this threadsafe?
var var::ostempfile() {

	// https://cpp.hotexamples.com/examples/-/-/mkstemp/cpp-mkstemp-function-examples.html

	// Create a char* template of the temporary file name desired
	// (2*26+10)^6 possible filenames = 56,800,235,584
	// Note that 64 bit hash = 1.84467440737e+19 combinations
	var				  rvo_tempfilename = var::ostempdir() ^ "~exoXXXXXX";
	std::vector<char> buffer(rvo_tempfilename.var_str.begin(), rvo_tempfilename.var_str.end());
	buffer.push_back('\0');

	// Linux only function to create a temporary file
	int fd_or_error;
	if ((fd_or_error = mkstemp(&buffer[0])) == -1)
		UNLIKELY
	throw VarError(var(__PRETTY_FUNCTION__) ^ " - Cannot create tempfilename " ^ rvo_tempfilename.quote());

	// Must close or we will leak file handles because this badly designed function returns a name not a handle
	::close(fd_or_error);

	// Get the actual generated temporary file name
	rvo_tempfilename.var_str = buffer.data();

	return rvo_tempfilename;
}

bool var::osshell() const {

	THISIS("bool var::osshell() const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	assertString(function_sig);

	// breakoff();
	int shellresult = system(to_oscmd_string(*this).c_str());

	if (shellresult)
		setlasterror("osshell failed: " ^ this->quote());

	// TRACE(*this)
	// TRACE(shellresult)
	//  breakon();

	return !shellresult;
}

bool var::osshellread(in oscmd) {

	THISIS("bool var::osshellread(in oscmd)")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	ISSTRING(oscmd)

	// THIS = ""
	// In case popen or pclose fail
	var_str.clear();
	var_typ = VARTYP_STR;

	// fflush?

	//"r" means read
	// std::FILE* pfile = popen(to_oscmd_string(oscmd).c_str(), "r");
	auto* pfile = popen(to_oscmd_string(oscmd).c_str(), "r");

	// Detect program failure.
	// Use status 0 to indicate that the program could not be started.
	// Not that it returned exit status 0
	if (pfile == nullptr) {
		var::setlasterror("Status: 0 osshellread:. " ^ oscmd.quote());
		return false;
	}

	// Read everything into a single var string
	char cstr1[4096] = {0x0};
	while (std::fgets(cstr1, sizeof(cstr1), pfile) != nullptr) {
		// std::printf("%s\n", result);
		// cannot trust that standard input is convertable from utf8
		// output.var_str+=wstringfromUTF8((const UTF8*)result,strlen(result));
		// std::string str1 = cstr1;
		// output.var_str += std::string(str1.begin(), str1.end());
		// readstr.var_str += std::string(cstr1);
		var_str += cstr1;
	}

	// Get the exit status
	int shellresult = pclose(pfile);

	// Return the exit status in last error if not 0
	if (shellresult)
		setlasterror("Status: " ^ var(shellresult) ^ " osshellread: " ^ oscmd.quote());

	// Return true if the exit status was 0
	return !shellresult;
}

bool var::osshellwrite(in oscmd) const {

	THISIS("bool var::osshellwrite(in oscmd) const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	assertString(function_sig);
	ISSTRING(oscmd)

	//"w" means read
	// std::FILE* pfile = popen(to_oscmd_string(oscmd).c_str(), "w");
	auto* pfile = popen(to_oscmd_string(oscmd).c_str(), "w");

	// Detect program failure.
	if (pfile == nullptr) {
		var::setlasterror("osshellwrite failed. " ^ oscmd.quote());
		return false;
	}

	// decided not to convert slashes here .. may be the wrong decision
	fputs(this->var_str.c_str(), pfile);

	int shellresult = pclose(pfile);

	if (shellresult)
		setlasterror("osshellwrite failed. " ^ oscmd.quote());

	// return true if no error code
	return !shellresult;
}

//bool run_piped_process_with_timeout(const std::string& input, std::string& stdout_output, 
//std::string& stderr_output, int& exit_status, int timeout_seconds, const std::string& command)

// Monster function defined in varos3.cpp
bool run_piped_process_with_timeout(
	const std::string& input,
	std::string& stdout_output,
	std::string& stderr_output,
	int& process_exit_status,
	int timeout_seconds,
	const std::string& command);

bool var::osprocess(in oscmd, in stdin_for_process, out stdout_from_process, out stderr_from_process, out exit_status, in timeout_secs) {

	// Does not use bash or support sh-like functionality e.g. ; | || && > etc.

	THISIS("bool var::osprocess(in oscmd, in stdin_for_process, out stdout_from_process, out stderr_from_process, out exit_status, in timeout_secs) static")
	ISSTRING(oscmd)
	ISSTRING(stdin_for_process)
	ISNUMERIC(timeout_secs)

	stdout_from_process = "";
	stderr_from_process = "";
	int process_exit_status = 0;

	bool result = run_piped_process_with_timeout(
		stdin_for_process.var_str,
		stdout_from_process.var_str,
		stderr_from_process.var_str,
		process_exit_status,
		int(timeout_secs),
		oscmd.var_str
	);
/*
	exit_status if false
	not zero from client or
	User createable:
	127: No args—treated as "no program provided."
	127: Program not found—child exits with 127.
	-1: Timeout—internal error when we terminate the child.
	Internal: Throwable
	-2: Pipe fail.
	-3: Fork fail.
	-4: Rare internal failure (poll fail, abnormal termination).
*/

	exit_status = process_exit_status;

	const var errmsg = (result and not process_exit_status) ? "" :
		"var::osprocess failed. " ^ var(exit_status)
		^ " " ^ oscmd.quote()
		;
	// False only when exit_status is >= 0
	if (not result) {
		if (std::abs(process_exit_status) <= -2 and std::abs(process_exit_status) >= -4)
			// TODO test
			throw VarError(errmsg ^ " piped_process: " ^ "Timeout^Pipe fail^Fork fail^Logic error"_var.f(std::abs(process_exit_status)));
		var::setlasterror(errmsg);
		return false;
	}
	var::setlasterror(errmsg);

	return exit_status == 0;
}

void var::osflush() const {
	std::cout << std::flush;
	//	std::cerr << std::flush; // self flushing
	std::clog << std::flush;
	return;
}

// optional locale (not the same as codepage)
bool var::osopen(in osfilename, const bool utf8 /*=true*/) const {

	THISIS("bool var::osopen(in osfilename, const bool utf8)")
	assertVar(function_sig);
	ISSTRING(osfilename)

	// if reopening an osfile that is already opened then close and reopen
	if (THIS_IS_OSFILE())
		this->osclose();

	return this->osopenx(osfilename, utf8, /*autocreate_or_throw*/ false) != nullptr;
}

static void del_fstream(void* handle) {
	delete static_cast<std::fstream*>(handle);
}

std::fstream* var::osopenx(in osfilename, const bool utf8, const bool autocreate_or_throw) const {

	// Try to get the cached file handle. the usual case is that you osopen a file before doing
	// osbwrite/osbread Using fstream instead of ofstream so that we can mix reads and writes on
	// the same filehandle
	std::fstream* fs_nrvo = nullptr;
	if (THIS_IS_OSFILE()) {
		fs_nrvo =
			static_cast<std::fstream*>(thread_fstream_handles.get_handle(static_cast<int>(var_int), var_str));
		if (fs_nrvo == nullptr)  // nonvalid handle
		{
			var_int = 0;
			//			var_typ ^= VARTYP_OSFILE;	// clear bit
			var_typ ^= VARTYP_OSFILE | VARTYP_INT | VARTYP_DBL;	// only STR bit should remains
		}
	}
	// Return cached fs
	if (fs_nrvo != nullptr)
		return fs_nrvo;

	// The file has NOT already been opened so open it now if possible.
	// Also add it to the cache.

	// Delay checking args until necessary
	THISIS("bool var::osopenx(in osfilename, const bool utf8, const bool autocreate_or_throw)")
	ISSTRING(osfilename)

	// Use unique_ptr since we have many return paths
	auto fs = std::make_unique<std::fstream>();

	auto opening_options = std::fstream::out | std::fstream::in | std::fstream::binary;

	if (osfilename.starts("std") and (osfilename == "stdout" or osfilename == "stderr")) {

		// Open /dev/stdout or /dev/stderr  - binary w
		opening_options ^= std::fstream::in;
		fs->open(osfilename.prefix("/dev/").c_str(), opening_options);
		if (not fs->is_open()) {
			const var errmsg = osfilename.quote() ^ " cannot be accessed";
			if (autocreate_or_throw)
				throw VarError(errmsg);
			var::setlasterror(errmsg);
			return nullptr;
		}

	}
	else if (std::filesystem::is_regular_file(osfilename.c_str())) {

		// Open the existing regular file - binary r/w append
		opening_options |= std::fstream::ate;
		fs->open(to_path_string(osfilename).c_str(), opening_options);
		if (not fs->is_open()) {

			// Try read only
			opening_options ^= std::fstream::out;
			fs->open(to_path_string(osfilename).c_str(), opening_options);
			if (not fs->is_open()) {
				const var errmsg = osfilename.quote() ^ " cannot be accessed";
				if (autocreate_or_throw)
					throw VarError(errmsg);
				var::setlasterror(errmsg);
				return nullptr;
			}
		}

	}
	else if (std::filesystem::exists(osfilename.c_str())) {

		// Return or throw info that it's not a regular file
		const var errmsg = osfilename.quote() ^ " is not a regular file.";
		if (autocreate_or_throw)
			throw VarError(errmsg);
		var::setlasterror(errmsg);
		return nullptr;
	}
	else if (not autocreate_or_throw) {

		// Inform file AWOL
		var::setlasterror(osfilename.quote() ^ " does not exist or cannot be accessed.");
		return nullptr;
	}
	else {

		// Create new file - binary r/w trunc, or throw
		opening_options |= std::fstream::trunc;
		fs->open(to_path_string(osfilename).c_str(), opening_options);
		if (not fs->is_open())
			throw(osfilename.quote() ^ " cannot be created.");
	}

	fs_nrvo = fs.release();

	// Cache
	// 1. var_str <- osfilename
	// 2. var_int <- the exodus os file cache handle no
	// 3. var_dbl <- the utf8 flag
	var_str = osfilename.var_str;
	// Add the file into the cache of thread_fstream_handles
	var_int = thread_fstream_handles.add_handle(fs_nrvo, del_fstream, osfilename.var_str);
	var_dbl = utf8;

	// VARTYP_OSFILE
	// VARTYP_NAN is set to prevent isnum trashing var_int or var_dbl
	// in the possible case that the osfilename a number
	var_typ = VARTYP_NANSTR_OSFILE;

	return fs_nrvo;
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
// bool var::osread(in osfilename, const char* codepage) {
//
//	THISIS("bool var::osread(in osfilename, const char* codepage")
//	ISSTRING(osfilename)
//	return osread(to_path_string(osfilename).c_str(), codepage);
// }
//
bool var::osread(const char* osfilename, const char* codepage) {

	THISIS("bool var::osread(const char* osfilename, const char* codepage")
	assertVar(function_sig);

	// osread returns empty string in any case
	var_str.clear();
	var_typ = VARTYP_STR;

	// get a file structure
	std::ifstream myfile;

	// Check if the file exists and is a regular file
	if (!std::filesystem::is_regular_file(osfilename)) {
		//        std::cout << "Error: File '" << osfilename << "' does not exist.\n";
		var::setlasterror("osread failed. " ^ var(osfilename).quote() ^ " does not exist, or cannot be accessed, or is not a regular file.");
		return false;
	}

	// open in binary (and position "at end" to find the file size with tellg)
	// TODO check myfile.close() on all exit paths or setup an object to do that
	myfile.open(osfilename, std::ios::binary | std::ios::in | std::ios::ate);
	if (!myfile) {
		var::setlasterror("osread failed. " ^ var(osfilename).quote() ^ " does not exist or cannot be accessed.");
		return false;
	}

	// determine the file size since we are going to read it all
	// NB tellg and seekp goes by bytes regardless of normal/wide stream
	// max file size 4GB?
	unsigned int bytesize;
	//	#pragma warning( disable: 4244 )
	// warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of
	// data
	// myfile.seekg(0, std::ios::end);
	bytesize = static_cast<unsigned int>(myfile.tellg());

	// if empty file then done ok
	if (bytesize == 0) {
		// TRACE(var(osfilename) ^ " bytesize:" ^ var(bytesize) ^ " dir:" ^ var(osfilename).osfile())
		// TRACE(var("ls -l " ^ var(osfilename)).osshell())
		myfile.close();
		return true;
	}
	// reserve memory - now reading directly into var_str
	// get file size * wchar memory to load the file or fail
	try {
		// emergency memory - will be deleted at } - useful if OOM
		// cancelling this to avoid heap fragmentation and increase performance
		// std::unique_ptr<char[]> emergencymemory(new char[16384]);

		// resize the string to receive the whole file
		var_str.resize(bytesize);
	} catch (std::bad_alloc& ex) {
		myfile.close();
		throw VarOutOfMemory("Could not obtain " ^ var(bytesize * sizeof(char)) ^
							 " bytes of memory to read " ^ var(osfilename) ^ " - " ^ ex.what());
	}

	// read the file into the reserved memory block
	myfile.seekg(0, std::ios::beg);
	// myfile.read (memblock.get(), (unsigned int) bytesize);
	// myfile.read(&var_str[0], (unsigned int)bytesize);
	// c++17 provides non-const access to data() :)
	myfile.read(var_str.data(), static_cast<unsigned int>(bytesize));

	bool failed = myfile.fail();

	// in case we didnt read the whole file for some reason, remove garbage in the end of the
	// string #pragma warning( disable: 4244 ) warning C4244: '=' : conversion from
	// 'std::streamoff' to 'unsigned int', possible loss of data
	bytesize = static_cast<unsigned int>(myfile.gcount());
	var_str.resize(bytesize);
	myfile.close();

	// failure can indicate that we didnt get as many characters as requested
	if (failed && !bytesize) {
		var::setlasterror("osread failed. " ^ var(osfilename).quote() ^ " 0 bytes read.");
		return false;
	}

	if (*codepage)
		// var_str=boost::locale::conv::to_utf<char>(var_str,"ISO-8859-5")};
		var_str = boost::locale::conv::to_utf<char>(var_str, codepage);

	return true;
}

var var::to_codepage(const char* codepage) const {

	THISIS("bool var::to_codepage(const char* codepage) const")
	assertString(function_sig);

	// NB to_codepage UTF32 adds a BOM at the front eg FFFE0000
	// conversion to UTF32BE and UTF32LE do not add but order is fixed
	// BE Big Endian, Little Endian (Intel)
	// little endian means adding small byte of number comes first
	// see iconv -l | grep utf32 -i

	// from utf8 to codepage
	// if (*codepage)
	return boost::locale::conv::from_utf<char>(var_str, codepage);
	// else
	//	return var_str;
}

var var::from_codepage(const char* codepage) const {

	THISIS("bool var::from_codepage(const char* codepage) const")
	assertString(function_sig);

	// to utf8 from codepage
	// if (codepage)
	return boost::locale::conv::to_utf<char>(var_str, codepage);
	// else
	//	return var_str;
}

// Used by obread
static unsigned count_invalid_trailing_UTF8_bytes(const std::string& str) {

	// Scans backward from the end of string.
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	const char* cptr = &str.back();
#pragma GCC diagnostic pop
	int num				   = 1;
	int numBytesToTruncate = 0;

	// UTF8 bytes sequences
	// 1 byte  for U+0000 tp U+007f:    0xxxxxxx
	// 2 bytes for U+0080 to U+07FF:    110xxxxx 10xxxxxx
	// 3 bytes for U+0800 to U+FFFF:    1110xxxx 10xxxxxx 10xxxxxx
	// 4 bytes for U+10000 to U+10FFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

	for (int i = 0; 6 > i; ++i) {
		numBytesToTruncate += 1;
		if ((*cptr & 0x80) == 0x80) {
			// If char bit starts with 1xxxxxxx
			// It's a part of unicode character!
			// Find the first byte in the unicode character!

			// if ((*cptr & 0xFC) == 0xFC) { if (num == 6) { return 0; } break; }
			// if ((*cptr & 0xF8) == 0xF8) { if (num == 5) { return 0; } break; }

			// If char binary is 1111xxxx, it means it's a 4 bytes long unicode.
			if ((*cptr & 0xF0) == 0xF0) {
				if (num == 4) {
					return 0;
				}
				break;
			}

			// If char binary is 1110xxxx, it means it's a 3 bytes long unicode.
			if ((*cptr & 0xE0) == 0xE0) {
				if (num == 3) {
					return 0;
				}
				break;
			}

			// If char binary is 110xxxxx, it means it's a 2 bytes long unicode.
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

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		cptr -= 1;
#pragma GCC diagnostic pop

	}  // next char

	return numBytesToTruncate;
}

// no binary conversion is performed on output unless
// codepage is provided (not locale) then exodus assumes internally
// utf-8 and converts all output to the specified codepage
bool var::oswrite(in osfilename, const char* codepage) const {

	THISIS("bool var::oswrite(in osfilename, const char* codepage) const")
	assertString(function_sig);
	ISSTRING(osfilename)

	// A file structure
	std::ofstream myfile;

	// Truncate any previous file,
	// TODO check myfile.close() on all exit paths or setup an object to do that
	myfile.open(to_path_string(osfilename).c_str(),
				std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile) {
		var::setlasterror("oswrite failed. " ^ osfilename.quote() ^ " cannot be opened for output.");
		return false;
	}

	if (*codepage) {
		// Convert to codepage before writing
		std::string tempstr = boost::locale::conv::from_utf<char>(var_str, codepage);
		myfile.write(tempstr.data(), tempstr.size());
	} else {
		// Raw write
		myfile.write(var_str.data(), var_str.size());
	}

	// Detect if failed
	bool failed = myfile.fail();

	if (failed)
		var::setlasterror("oswrite failed. " ^ osfilename.quote() ^ " Unknown reason.");

	return !failed;
}

// #ifdef VAR_OSBREADWRITE_CONST_OFFSET
//  a version that accepts a const offset ie ignores return value
// bool var::osbwrite(in osfilevar, in offset, const bool adjust) const
//  offset -1 appends by starting writing one byte after the current end of the file
//  offset -2 updates the last byte of the file.
//  etc.
// bool var::osbwrite(in osfilevar, in offset) const {
//	return this->osbwrite(osfilevar, const_cast<io>(offset));
// }
// #endif

// NOTE: unlike osread/oswrite which rely on iconv codepages to do any conversion
// osbread and osbwrite rely on the locale being passed in on the osopen stage

// bool var::osbwrite(in osfilevar, io offset, const bool adjust) const
bool var::osbwrite(in osfilevar, io offset) const {

	// osfilehandle is just the filename but has a special var_typ
	// and buffers the "file number" in the var_int

	THISIS("bool var::osbwrite(in osfilevar, io offset) const")
	assertString(function_sig);
	ISNUMERIC(offset)

	// Get the buffered file handle/open on the fly
	// Create missing file IIF offset is 0
	std::fstream* fs = osfilevar.osopenx(osfilevar, /*utf8*/ true, /*autocreate_or_throw*/ offset == 0);

	// Similar code in osbread and osbwrite. Soft error.
	if (fs == nullptr) UNLIKELY {
		const var errmsg = "osbwrite failed. " ^ var::lasterror();
		var::setlasterror(errmsg);
		return false;
	}

	if (offset < 0)
		fs->seekp(offset.toInt() + 1, std::ios_base::end);
	else
		fs->seekp(offset.toInt());

	fs->write(var_str.data(), var_str.size());

	if (fs->fail()) UNLIKELY {
		// saved in cache, DO NOT CLOSE!
		// myfile.close();
		throw VarError(osfilevar.quote() ^ " osbwrite write failed");
	}

	// Pass back the file pointer offset
	offset = static_cast<int>(fs->tellp());

	// Although slow, ensure immediate visibility of osbwrites
	fs->flush();

	// Saved in cache, DO NOT CLOSE!
	// myfile.close();

	return true;
}

// NOTE if the locale is not C then any partial non-utf-8 bytes at the end (due to bytesize
// not being an exact number of valid utf-8 code units) are trimmed off the return value
// The new offset is changed to reflect the above and is simply increased by bytesize
bool var::osbread(in osfilevar, io offset, const int bytesize) {

	THISIS("bool var::osbread(in osfilevar, io offset, const int bytesize")
	assertVar(function_sig);
	ISNUMERIC(offset)

	// default is to return empty string in any case
	var_str.clear();
	var_typ = VARTYP_STR;

	// strange case request to read 0 bytes
	if (bytesize <= 0)
		return true;

	// Get the buffered file handle/open on the fly or return a soft error
	// likely indicating that the file was not preopened and does not exist
	std::fstream* fs = osfilevar.osopenx(osfilevar /*utf8 = true*/ /*autocreate_or_throw = false*/);
	if (fs == nullptr) {
		var::setlasterror("osbread failed. " ^ this->lasterror());
		return false;
	}

	// Seek to the offset
	{
		if (fs->fail())
			fs->clear();
		fs->rdbuf()->pubseekpos(static_cast<std::uint64_t>(offset.var_int));
	}

	// It is NOT possible to transfer directly to std::string.data() after using .reserve()
	// It IS possible to transfer directly to std string.data() after using .resize() but this initialises all bytes to zero "for safety"?
	// So we will stick to our tried and tested method
	// Using new char[bytesize] method. Note: not char[bytesize]()
	// followed by std::string.assign()
	bool hard_fail = false;
	if (true) {

		// branch using unique_ptr of new char[bytesize]

		// Get an (uninitalised) memory block to read into
		std::unique_ptr<char[]> memblock(new char[bytesize]);

		// Read the file data into the memory block
		fs->read(memblock.get(), bytesize);

		// Capture bytes read immediately although cppref says that gcount()
		// "returns number of characters extracted by last unformatted input operation"
		// https://en.cppreference.com/w/cpp/io/basic_istream/gcount
		// Possibly 0 in case of hard fs fail other than eof
		// Note than we return false finally if returning "" data
		const auto bytes_actually_read = fs->gcount();

		// "fail" includes normal eof fail
		if (fs->fail()) {

			// In case of any fs error (including eof) reposition to eof

			// In case of fail not eof the we will soldier on
			// as always in the past and return what data we can
			// but now we will in the end forcibly return false to the caller
			// and leave it up to them to decide what to do with any data
			// that we managed to read although it is quite likely none
			// which always triggered false in the past anyway.
			if (!fs->eof())
				hard_fail = true;

			// Renable calls to seekg? and tellg? after eof() or any hard fail.
			fs->clear();

			// Force tellg() to report  past end of file regardless of eof or hard fail.
			fs->seekg(0, std::ios::end);
		}

		// DONT FORGET TO update the [io] function argument "offset"
		offset = static_cast<int>(fs->tellg());

		// Create our string from the memory block
		// but only the bytes actually read.
		var_str.assign(memblock.get(), bytes_actually_read);

	} else {

		// branch using std::string.resize(bytesize)

		// Prepare the string. MUST be resize and CANNOT be reserve.
		var_str.resize(bytesize);

		// Read the data
		fs->read(var_str.data(), bytesize);

		// Capture bytes read immediately
		const auto bytes_actually_read = fs->gcount();

		// Case of any error reposition to end of file. Why exactly?
		// TODO same issue above
		// fs.fail();
		if (fs->fail()) {
			if (!fs->eof())
				hard_fail = true;
			fs->clear();
			fs->seekg(0, std::ios::end);
		}

		// DONT FORGET TO opdate the [io] function argument "offset"
		offset = static_cast<int>(fs->tellg());

		// Adjust the string to the bytes actually read.
		// TODO Same issue above
//		var_str.resize(static_cast<unsigned int>(bytes_actually_read));
		var_str.resize(bytes_actually_read);

	}

	// Trim incomplete UTF-8
	if (osfilevar.var_dbl) {
		auto nextra_bytes = count_invalid_trailing_UTF8_bytes(var_str);
		if (nextra_bytes) {

			// Dont forget to reduce the io arg offset!
			offset -= static_cast<int>(nextra_bytes);

			// Cut off the excess bytes from var_str
			var_str.resize(var_str.size() - nextra_bytes);
		}
	}

	// Fail if hard fail or no data read (offset >= file size)
	return !hard_fail && !var_str.empty();
}

void var::osclose() const {
	if (THIS_IS_OSFILE()) {
		thread_fstream_handles.del_handle(static_cast<int>(var_int));
//		var_typ ^= (VARTYP_OSFILE | VARTYP_INT | VARTYP_NAN | VARTYP_DBL);	// only STR bit should remains
		var_typ = VARTYP_STR;
	}
	// in all other cases, the files should be closed.
}

bool var::osrename(in new_dirpath_or_filepath) const {

	THISIS("bool var::osrename(in new_dirpath_or_filepath) const")
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
		var::setlasterror("osrename failed. " ^ new_dirpath_or_filepath.quote() ^ " already exists.");
		return false;
	}

	// Safety
	if (!checknotabsoluterootfolder(path1)) {
		var::setlasterror("osrename failed. " ^ var(path1).quote() ^ " cannot be renamed because it is a top level dir");
		return false;
	}

	if (!checknotabsoluterootfolder(path2)) {
		var::setlasterror("osrename failed. " ^ var(path2).quote() ^ " cannot be overwritten because it is a top level dir");
		return false;
	}

	//	if (!std::rename(path1.c_str(), path2.c_str())) {
	//	// TODO problem no informative error message
	//		var::setlasterror("osrename failed. " ^ this->quote() ^ " to " ^ new_dirpath_or_filepath.quote() ^ " unknown error.");
	//		return false;
	//	}

	// https://en.cppreference.com/w/cpp/filesystem/rename
	std::error_code error_code;
	std::filesystem::rename(
		path1.c_str(),
		path2.c_str(),
		error_code
	);

	// Handle error
	if (error_code) {
		var::setlasterror("osrename failed. " ^ this->quote() ^ " to " ^ new_dirpath_or_filepath.quote() ^ " Error: 0" ^ error_code.message());
		return false;
	}

	return true;
}

bool var::oscopy(in new_dirpath_or_filepath) const {

	THISIS("bool var::oscopy(in new_dirpath_or_filepath) const")
	assertString(function_sig);
	ISSTRING(new_dirpath_or_filepath)

	std::string path1 = to_path_string(*this);
	std::string path2 = to_path_string(new_dirpath_or_filepath);

	// copy recursively, overwriting
	std::filesystem::path frompathx(path1.c_str());
	std::filesystem::path topathx(path2.c_str());
	// https://en.cppreference.com/w/cpp/filesystem/copy
	std::error_code error_code;
	std::filesystem::copy(
		frompathx,
		topathx,
		//		std::filesystem::copy_options::overwrite_existing
		//		| std::filesystem::copy_options::recursive
		//		| std::filesystem::copy_options::copy_symlinks,
		// Cludge to avoid compile error about missing binary operator when working under home brew module std
		std::filesystem::copy_options(
			int(std::filesystem::copy_options::overwrite_existing) | int(std::filesystem::copy_options::recursive) | int(std::filesystem::copy_options::copy_symlinks)),
		error_code);

	// Handle error
	if (error_code) {
		var::setlasterror("oscopy failed. " ^ this->quote() ^ " to " ^ new_dirpath_or_filepath.quote() ^ " Error: " ^ error_code.message());
		return false;
	}

	return true;
}

bool var::osmove(in new_dirpath_or_filepath) const {

	THISIS("bool var::osmove(in new_dirpath_or_filepath) const")
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
		var::setlasterror("osmove " ^ this->quote() ^ " failed. " ^ new_dirpath_or_filepath.quote() ^ " already exists.");
		return false;
	}

	// Safety
	if (!checknotabsoluterootfolder(path1)) {
		var::setlasterror(var(path1).quote() ^ " Cannot be moved or deleted because it is a top level dir");
		return false;
	}
	if (!checknotabsoluterootfolder(path2)) {
		var::setlasterror(var(path2).quote() ^ " Cannot be overwritten because it is a top level dir");
		return false;
	}

	// Try to rename but will fail to move across file systems
	//////////////////////////////////////////////////////////
	if (this->osrename(new_dirpath_or_filepath))
		return true;

	// To copy and delete instead of move
	////////////////////////////////////////
	if (!this->oscopy(new_dirpath_or_filepath)) {
		var::setlasterror("osmove failed. " ^ this->lasterror());
		return false;
	}

	// Delete original only after copy to target is successful
	if (!this->osremove()) {

		// Too dangerous to remove target in case of failure to delete source
		// otherwise delete the target too
		// new_dirpath_or_filepath.osremove();

		var::setlasterror(this->quote() ^ " osmove failed to remove source after copying to " ^ new_dirpath_or_filepath.quote());
		return false;
	}

	return true;
}

bool var::osremove() const {

	assertVar(__PRETTY_FUNCTION__);
	this->osclose();  // in case this is cached opened file handle

	// Prevent removal of dirs. Use osrmdir for that.
	if (std::filesystem::is_directory(this->toString())) {
		var::setlasterror(this->quote() ^ " osremove failed - is a directory.");
		return false;
	}

	if (std::remove(to_path_string(*this).c_str())) {
		var::setlasterror(this->quote() ^ " failed to osremove");
		return false;
	}
	return true;
}

bool var::osmkdir() const {

	assertString(__PRETTY_FUNCTION__);

	std::filesystem::path pathx(to_path_string(*this).c_str());

	//	if (std::filesystem::exists(pathx))	{
	//		var::setlasterror(this->quote() ^ " osmkdir failed. Target already exists.");
	//		return false;
	//	}
	//
	std::error_code ec;

	// Creates parent directories automatically
	// https://en.cppreference.com/w/cpp/filesystem/create_directory
	bool created = std::filesystem::create_directories(pathx, ec);
	if (ec or !created) {
		var::setlasterror(this->quote() ^ " osmkdir failed. " ^ ec.message());
		return false;
	}

	if (!std::filesystem::exists(pathx)) {
		var::setlasterror(this->quote() ^ " osmkdir failed. Target could not be created.");
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
			var::setlasterror("osrmdir failed. " ^ this->quote() ^ " does not exist.");
			return false;
		}

		if (!std::filesystem::is_directory(pathx)) {
			var::setlasterror("osrmdir failed. " ^ this->quote() ^ " is not a directory.");
			return false;
		}

		if (evenifnotempty) {

			// safety .. simply REFUSE to rm top level folders if not empty
			// find some other way e.g. shell command if you must
			if (!checknotabsoluterootfolder(toString())) {
				var::setlasterror("osrmdir failed " ^ this->quote() ^ " is a top level dir.");
				return false;
			}

			std::filesystem::remove_all(pathx);
		} else
			std::filesystem::remove(pathx);
	} catch (...) {
		var::setlasterror("osrmdir failed. " ^ this->quote() ^ " Unknown cause.");
		return false;
	}

	return true;
}

// utility defined in mvdatetime.cpp
// void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);
// WARNING ACTUALLY DEFINED WITH BOOST POSIX TIME BUT IT IS THE SAME
// void ptime2mvdatetime(const std::filesystem::file_time_type& ptimex, int& mvdate, int& mvtime);
void time_t_to_pick_date_time(time_t time, int* pick_date, int* pick_time);

// convert some clock to time_t (for osfile() and osdir()
// template <typename TP>
// std::time_t to_time_t(TP tp) {
//	using namespace std::chrono;
//	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
//	return system_clock::to_time_t(sctp);
// }

// Return for a file or dir, a short string containing modified_date ^ FM ^ modified_time ^ FM ^size
var var::osinfo(const int mode /*=0*/) const {

	assertString(__PRETTY_FUNCTION__);

	// get a handle and return "" if doesnt exist or is NOT a directory
	// std::filesystem::wpath pathx(toTstring((*this)).c_str());
	try {

		// Why use stat instead of std::filesystem?
		// https://eklitzke.org/std-filesystem-last-write-time

		// Using low level interface instead of std::filesystem to avoid multiple call to stat
		// 7,430 ns/op instead of  18,500 ns/op

		// https://stackoverflow.com/questions/21159047/get-the-creation-date-of-file-or-folder-in-c#21159305

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
		// boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(statinfo.st_mtime);
		// ptime2mvdatetime(ptimex, pick_date, pick_time);
		time_t_to_pick_date_time(statinfo.st_mtime, &pick_date, &pick_time);

		// file_size() is only available for files not directories
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

	THISIS("var  var::oslist(in globpattern, const int mode) const")
	assertVar(function_sig);
	// ISSTRING(path0)
	// ISSTRING(globpattern0)

	// returns an FM separated list of files and/or folders

	// http://www.boost.org/libs/filesystem/example/simple_ls.cpp

	var this_path = to_path_string(*this);
	var globpattern;
	if (globpattern0.size()) {
		globpattern = globpattern0;
	}
	// File globbing can and must be passed as tail end of path
	// perhaps could use <glob.h> in linux instead of regex
	// TODO bash-like globbing using something like
	// https://github.com/MrGriefs/glob-cpp/blob/main/README.md
	else {

		// If the last part of path looks like a glob
		// (has * or ?) use it.
		globpattern = this_path.field(_OSSLASH, -1);
		if (globpattern.convert("*?", "") != globpattern)
			this_path.firster(this_path.len() - globpattern.len());
		else
			globpattern = "";
	}

	const bool getfiles	  = mode != 2;
	const bool getfolders = mode != 1;

	// nrvo
	var filelist = "";

	// Work out the full absolute path
	std::filesystem::path full_path(std::filesystem::current_path());
	if (this_path.len()) {
		// full_path = std::filesystem::absolute(std::filesystem::path(this_path.to_path_string().c_str()));
		std::error_code		  error_code;
		std::filesystem::path pathx = std::filesystem::path(to_path_string(this_path).c_str());
		full_path					= std::filesystem::absolute(pathx, error_code);
		if (error_code) {
			std::cerr << "'" << to_path_string(this_path) << "' path : " << error_code.message() << std::endl;
			return filelist;
		}
	}

	if (globpattern == "*")
		globpattern = "";

	// std::clog << "fullpath='" << full_path << "'" <<std::endl;

	// Quit if it isnt a folder
	if (!std::filesystem::is_directory(full_path))
		return filelist;

	std::filesystem::directory_iterator end_iter;
	for (std::filesystem::directory_iterator dir_itr(full_path); dir_itr != end_iter; ++dir_itr) {
		try {

			// Skip unwanted items not matching the glob if provided
			// TRACE(dir_itr->path().filename().string())
			// TRACE(globpattern)
			if (globpattern && fnmatch(globpattern.var_str.c_str(), dir_itr->path().filename().c_str(), 0) != 0)
				continue;

			// Work/efficiently if (std::filesystem::is_directory(dir_itr->status() ) )
			if (std::filesystem::is_directory(*dir_itr)) {
				if (getfolders) {
append_it:
					//					filelist ^= dir_itr->path().filename().string() ^ FM;
					filelist.var_str += dir_itr->path().filename().string();
					filelist.var_str.push_back(FM_);
				}
			} else {
				if (getfiles) {
					//					filelist ^= dir_itr->path().filename().string() ^ FM;
					goto append_it;
				}
			}
		} catch ([[maybe_unused]] const std::exception& ex) {
			//++err_count;
			// std::cout << dir_itr->path().leaf() << " " << ex.what() << std::endl;
		}
	}

	// Delete last separator
	if (!filelist.var_str.empty())
		filelist.var_str.pop_back();

	return filelist;
}

bool var::oscwd(SV newpath) {

	THISIS("bool var::oscwd(SV newpath) static")

	try {
		std::filesystem::current_path(newpath);
	} catch (...) {
		// ignore all errors
		var::setlasterror(var(newpath).quote() ^ " oscwd failed - unknown cause.");
		return false;
	}

	return true;
}

var var::oscwd() {

	THISIS("var  var::oscwd() static")

	std::string currentpath = std::filesystem::current_path().string();

	return var(currentpath).convert("/", _OSSLASH);
}

}  // namespace exo
