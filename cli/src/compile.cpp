// NOTE The rhs comment in the following line is in a special format
// REQUIRED instruction to compile and link with pthread library
#include <thread>  //gcc_link_options -lpthread
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
// Create a threadpool of suitable size to handle
// ncpus/threads obtained from std::thread::hardware_concurrency()
boost::asio::thread_pool threadpool1(std::thread::hardware_concurrency());

// Use an atomic to allow threads to increment global failure count
#include <atomic>
std::atomic<int> atomic_ncompilation_failures;

#include <exodus/program.h>
programinit()

	// CONSTANTS

	// Options
	let verbose = contains(OPTIONS, "V");
	let silent = count(OPTIONS, "S");
	let debugging = not contains(OPTIONS, "R");  //no symbols for backtrace
	let optimise = count(OPTIONS, "O") - count(OPTIONS, "o");
	let generateheadersonly = contains(OPTIONS, "h");
	let force = contains(OPTIONS, "F");
	let color_option = contains(OPTIONS, "C");
	let warnings = count(OPTIONS, "W") - count(OPTIONS, "w");

	// Source extensions
	let src_extensions = "cpp cxx cc";
	let inc_extensions = "h hpp hxx";
	let noncompilable_extensions = " out so o";
	let default_extension = "cpp";

	// Platform
	let posix = not OSSLASH_IS_BACKSLASH;
	let windows = not(posix);

	// Location of bin/lib/inc dirs
	let exo_HOME = osgetenv("EXO_HOME") ?: osgetenv("HOME");

	// Limits
	let exodus_callable_maxnargs = 20;

	// GLOBAL VARIABLES
	// Do NOT update in multithreaded compile function

	var exodus_include_dir_info = "";
	var nasterisks = 0;

	//.def file is one way on msvc to force library function names to be "undecorated"
	// and therefore usable in on demand library loading
	//Declaring functions with extern "C" is the other way and seems less complicated
	//but msvc warns that functions defined with extern c cannot return var
	//nevertheless, returning vars seems to work in practice and .def files seem
	//more complicated so for now use extern "C" and not .def files
	//#define EXODUS_EXPORT_USING_DEF

function main() {

	// Skip compile for option X - allows disabling compilations unless generating headers
	if (OPTIONS.contains("X") and not OPTIONS.contains("h"))
		return 0;

	// If no file/dirnames then default to previous edic
	// SIMILAR code in edic and compile
	if (fcount(COMMAND, FM) < 2) {
		var edic_hist = osgetenv("HOME") ^ "/.config/exodus/edic_hist.txt";
		if (osread(COMMAND, edic_hist)) {
			COMMAND = raise(COMMAND.f(1));
		}
	}

	// Extract file/dirnames
	var filenames = field(COMMAND, FM, 2, 999999999);
	var nfiles = fcount(filenames, FM);
	if (not filenames or OPTIONS.contains("H"))
		abort(
			"SYNTAX\n"
			"	compile FILENAME|DIRNAME ... [{OPTION...}]\n"
			"OPTIONS\n"
			"	R = Release (No symbols)\n"
			"	O/OO/OOO = Optimisation levels (Poorer debugging/backtrace)\n"
			"	o/oo/ooo = Unoptimise (cancels 'O's)\n"
			"	W = Increase warnings\n"
			"	w = Reduce warnings\n"
			"	V = Verbose\n"
			"	S = Silent (stars only)\n"
			"	h = Generate headers only\n"
			"	X = Skip compilation\n"
			"ENVIRONMENT\n"
			"	CXX_OPTIONS"
			"	CXX e.g. g++, clang, c++ with or without full path"
		);


	// DETERMINE COMPILER/LINKER OPTIONS AND FLAGS
	//////////////////////////////////////////////

	// Use on demand/jit linking using dlopen/dlsym
	var loadtimelinking = false;

	var usedeffile = false;

	var compiler = "";

	var basicoptions = "";
	let addoptions = osgetenv("CXX_OPTIONS");
	if (addoptions and verbose)
		printl("Using CXX_OPTIONS environment variable " ^ addoptions.quote());

	var linkoptions = "";
	let addlinkoptions = osgetenv("CXX_LINK_OPTIONS");
	if (addlinkoptions and verbose)
		printl("Using CXX_LINK_OPTIONS environment variable " ^ addlinkoptions.quote());

	var binoptions = "";
	var liboptions = "";

	var bindir = "";
	var libdir = "";
	var incdir = "";

	var installcmd = "";
	var outputoption = "";
	var objfileextension = "";
	var binfileextension = "";
	var libfileextension = "";
	var libfileprefix = "";

	//you can allow updating inuse binaries in posix but with some messages
	//true may result in messages similar to the following from exodus compiler
	//Error: /home/neo/bin/server cannot be updated. In use or insufficient rights (2).
	//false may result in messages similar to the following from running exodus programs
	//BFD: reopening /home/neo/exodus/service/service2/sys/server: No such file or directory
	var updateinusebinposix = true;

	// Hard coded compiler options at the moment
	// Assume msvc (cl) on windows and c++ (g++/clang) otherwise
	if (posix) {


		//////////////////////////
		// POSIX - linux, bsd etc.
		//////////////////////////

		if (verbose)
			printl("Posix environment detected.");

		var gcc = osshellread("c++ --version").contains("Free Software Foundation");

		//target directories
		bindir = exo_HOME ^ "/bin";
		libdir = exo_HOME ^ "/lib";
		incdir = exo_HOME ^ "/inc";

		//target extensions
		//make it easier for debuggers
		//objfileextension=".out";
		objfileextension = "";
		binfileextension = "";
		libfileextension = ".so";
		libfileprefix = "lib";

		installcmd = "mv";

		compiler = osgetenv("CXX");
		if (compiler) {
			if (verbose)
				printl("Using EXO_CXX environment variable for compiler " ^ compiler.quote());
		} else {
			compiler = "c++";
#ifdef __APPLE__
			if (PLATFORM eq "x86")
				compiler = "g++-4.0";
#endif
			if (verbose)
				printl("no EXO_CXX or CXX environment variable. Assuming " ^ compiler.quote());
		}
		//basic compiler options

		//c++11/17
		//basicoptions^=" -std=gnu++0x";//pre gcc 4.7 enables gnu extentions.
		//basicoptions^=" -std=gnu++11";//gcc 4.7 and later  enables gnu extensions.
		//__cplusplus is 201103L
		//if( __cplusplus >= 201103L )
		//basicoptions^=" -std=c++11";//gcc 4.7 and later DISABLES gnu extensions
		//if  (__cplusplus >= 202003L)
		if (__cplusplus >= 201709)
			basicoptions ^= " -std=c++2a";  //gcc 4.7 and later DISABLES gnu extensions
		else
			basicoptions ^= " -std=c++17";  //gcc 4.7 and later DISABLES gnu extensions

		// Minor space savings
		basicoptions ^= " -fvisibility=hidden -ffunction-sections -fdata-sections";

		//not available on gcc 4.1.2 TODO work out gcc version
		//basicoptions^=" -Wno-unused-parameters"; //dont want if functions dont use their parameters
		//basicoptions^=" -pendantic -ansi";
		//following will result in 2 byte wchar in linux
		//but all exodus libs also need to be in this format too
		//leave as 4 byte for now to avoid forcing additional compilation option on exodus users
		//who use c++ (g++/clang) directly (or put runtime check that program+lib have the same size wchar
		//advising the right compilation option.
		//basicoptions^=" -fshort-wchar";

		//exodus library
		//if (debugging)
		//	linkoptions=" -lexodus-gd";
		//else
		//linkoptions = " -lexodus -lstdc++fs -lpthread";
		linkoptions = " -lexodus -lstdc++fs -lstdc++";

		//always look in header install path eg ~/inc
		basicoptions ^= " -I" ^ incdir;

		if (debugging) {

			//nopie to prevent ALSR preventing backtrace
			// From man g++
			/*
				-no-pie
					Don't produce a dynamically linked position independent executable.
*/
			//basicoptions ^= " -no-pie";

			// Enable function names for backtrace in executables?
			// From man g++
			/*
				-rdynamic
					Pass the flag -export-dynamic to the ELF linker, on targets that support it. This instructs the linker to add all symbols, not only used ones, to the dynamic symbol table. This option is
					needed for some uses of "dlopen" or to allow obtaining backtraces from within a program.
			*/
			//basicoptions ^= " -rdynamic";

			// https://stackoverflow.com/questions/10475040/gcc-g-vs-g3-gdb-flag-what-is-the-difference
			//basicoptions ^= " -ggdb";
			//basicoptions ^= " -g";
			basicoptions ^= " -g3";

		} // debugging

		//optimiser unfortunately prevents backtrace
		//if (optimise) {
		//basicoptions^=" -O1";
		//basicoptions^=" -O3";
		//-Og means optimise but has compatible with gdb
		if (optimise > 0) {
//			if (optimise eq 1)
//				basicoptions ^= " -Og";
//			else
				basicoptions ^= " -O" ^ optimise;
		}
		//}

		if (color_option)
			basicoptions ^= " -fdiagnostics-color=always";

		// Show various warnings by default or by command
		if (warnings >= 0) {

			//http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#Warning-Options
//			This enables all the warnings about constructions that
//			some users consider questionable, and that are easy to avoid
//			(or modify to prevent the warning), even in conjunction with
//			macros. This also enables some language-specific warnings
//			described in C++ Dialect Options and Objective-C and
//			Objective-C++ Dialect Options.
//
//			Note that some warning flags are not implied by -Wall.
//			Some of them warn about constructions that users generally
//			do not consider questionable, but which occasionally you
//			might wish to check for; others warn about constructions
//			that are necessary or hard to avoid in some cases, and there
//			is no simple way to modify the code to suppress the warning.
//			Some of them are enabled by -Wextra but many of them must be
//			enabled individually.
			basicoptions ^= " -Wall";

			//This enables some extra warning flags that are not enabled by -Wall.
			basicoptions ^= " -Wextra";

			// Allow "Elvis operator" ?:
			//warning: ISO C++ does not allow ?: with omitted middle operand [-Wpedantic]
			//basicoptions ^= " -Wpedantic";

			// no-xxxxxxxx means switch off warning xxxxxxxx
			//basicoptions ^= " -Wno-cast-function-type";
		}

		// Suppress GCC and clang's warnings about each others warning flags
		if (warnings lt 2) {
			basicoptions ^= " -Wno-unknown-pragmas";
			if (not gcc) {
				// Presume clang
				basicoptions ^= " -Wno-unknown-warning-option";
				basicoptions ^= " -Wno-unknown-warning";
			}
		}

		// Suppress various warnings by default or by command
		if (warnings <= 0) {

			// Option to turn off all warnings
			if (warnings < 0) {
				basicoptions ^= " -w";
			} else {

				// CLANG -- how to tell?
				if (true) {

					// Handled in code with #pragma GCC/clang diagnostics ...
					// Allow strange call to exodus shared libs
					//basicoptions ^= " -Wno-bad-function-cast";

					// exodus var doesnt initialise all its members but does copy/move them
					// TODO disable the warning in code so real warnings can be show in general
					basicoptions ^= " -Wno-uninitialized";

					// Ignore these warnings caused by [[no-discard]] for now
					// TODO Remove when all such code in exodus service and apps is replaced
					// warning: ignoring return value of function declared with 'nodiscard' attribute [-Wunused-result]
					basicoptions ^= " -Wno-unused-result";

					// Ignore warnings for now about code like
					// open("PROCESSES") or createfile("PROCESSES");
					// warning: expression result unused [-Wunused-value]
					//basicoptions ^= " -Wno-unused-value";

					// Ignore GCC warning flags
					//basicoptions ^= " -Wno-unknown-warning";

					// Ignore GCC compile flags
					//basicoptions ^= " -Wno-unused-command-line-argument";

					// Ignore some warning related to exodus' "labelled commons"
					//basicoptions ^= " -Wno-unused-private-field";

				} //clang
			}
		}

		//how to output to a named file
		outputoption = " -o ";

		//general options
		binoptions = "";
		//binoptions=" -g -L./ -lfunc1 -Wl,-rpath,./";
		//binoptions=" -fPIC";

		//how to make a shared library
		liboptions = " -fPIC -shared";
		//soname?
#if __GNUC__ >= 4
		//use c++ (g++/clang) -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
		liboptions ^= " -fvisibility=hidden -fvisibility-inlines-hidden";
#endif

		//libexodusinfo = osfile("/usr/local/lib/libexodus.so");
		exodus_include_dir_info = osdir("/usr/local/include/exodus");

	} else {


		//////////////////////
		// NOT POSIX - WINDOWS
		//////////////////////

		if (verbose)
			printl("Windows environment detected. Finding C++ compiler.");

		//this calls sdk setenv or vcvars
		call set_environment();

		if (verbose)
			printl("Searching for Exodus for include and lib directories");

		//get current environment
		var path = osgetenv("PATH");
		var include = osgetenv("INCLUDE");
		var lib = osgetenv("LIB");

		//locate EXO_PATH by executable path, environment variable or current disk or C:
		//EXO_PATH is the parent directory of bin and include etc.
		//compile needs to locate the include and lib directories

		var ndeep = fcount(EXECPATH, OSSLASH);
		var exoduspath = "";
		//first guess is the parent directory of the executing command
		//on the grounds that compile.exe is in EXO_PATH/bin
		if (ndeep > 2)
			exoduspath = EXECPATH.field(OSSLASH, 1, ndeep - 2);
		var searched = "";

		//check if EXO_PATH\bin\exodus.dll exists
		if (not exoduspath or not(osfile(exoduspath ^ OSSLASH ^ "bin\\exodus.dll") or osfile(exoduspath ^ OSSLASH ^ "exodus.dll"))) {
			if (exoduspath)
				searched ^= "\n" ^ exoduspath;
			exoduspath = osgetenv("EXO_PATH");
			if (exoduspath)
				searched ^= "\nEXO_PATH is " ^ exoduspath;
			else {
				searched ^= "\nEXO_PATH environment variable is not set";
				exoduspath = "\\Program Files\\exodus\\" EXODUS_RELEASE "\\";
				if (not osdir(exoduspath)) {
					searched ^= "\n" ^ exoduspath;
					exoduspath.prefixer("C:");
				}
			}
		}
		if (exoduspath.last(1) ne OSSLASH)
			exoduspath ^= OSSLASH;
		var exodusbin = exoduspath;
		if (not osfile(exodusbin ^ "exodus.dll")) {
			searched ^= "\n" ^ exoduspath;
			exodusbin = exoduspath ^ "bin\\";
			if (not osfile(exodusbin ^ "exodus.dll")) {
				//exodusbin=EXECPATH;
				searched ^= "\n" ^ exoduspath;
				printl("Searching for Exodus", searched);
				abort("Cannot find Exodus. Set environment variable EXO_PATH to exodus directory and restart Exodus");
			}
		}

		//add exodus path, include and lib
		path.prefixer(exodusbin ^ ";");
		include.prefixer(exoduspath ^ "include;");
		include.prefixer(exoduspath ^ "include\\exodus;");
		lib.prefixer(exoduspath ^ "lib;");

		// Set the new environment (may not be received by child processes)
		ossetenv("PATH", path);
		ossetenv("INCLUDE", include);
		ossetenv("LIB", lib);
		if (verbose) {
			path.errputl("\nPATH=");
			include.errputl("\nINCLUDE=");
			lib.errputl("\nLIB=");
		}
		compiler = "cl";
		basicoptions = "";
		//complete compiler options
		//http://msdn.microsoft.com/en-us/library/fwkeyyhe%28v=VS.80%29.aspx

		//basic compiler options
		//"MSVC requires exception handling (eg compile with /EHsc or EHa?) for delayed dll loading detection" this is required to avoid loading the postgres dll unless and until required
		//"If /EH is not specified, the compiler will catch structured and C++ exceptions, but will not destroy C++ objects that will go out of scope as a result of the exception."
		//"Under /EHs, catch(...) will only catch C++ exceptions. Access violations and System.Exception exceptions will not be caught."
		//c tells the compiler to assume that extern C functions never throw a C++ exception"
		//however exceptions from exodus functions which are declared external "C" still seem to work
		//maybe could remove the c option
		//basicoptions=" /EHsc";
		basicoptions ^= " /EHsc";

		//"Enables all warnings, including warnings that are disabled by default."
		basicoptions ^= " /W3";
		//basicoptions^=" /Wall";

		//Enables one-line diagnostics for error and warning messages when compiling C++ source code from the command line.
		basicoptions ^= " /WL";

		//Generates complete debugging information.
		//needed for backtrace
		basicoptions ^= " /Zi";

		//Specifies a C++ source file.
		basicoptions ^= " /TP";

		//define compiling for console application
		basicoptions ^= " /D \"_CONSOLE\"";
		//to capture output after macro expansion
		//basicoptions^=" /E";

		//define common windows indicator (applies to WIN64 too!)
		basicoptions ^= " /D \"WIN32\"";

		//define common windows indicator
		if (PLATFORM eq "x64")
			basicoptions ^= " /D \"WIN64\"";

		//Uses the __cdecl calling convention (x86 only ie 32bit).
		///Gd, the default setting, specifies the __cdecl calling convention
		//for all functions except C++ member functions and functions
		//marked __stdcall or __fastcall.
		//basicoptions^=" /Gd";

		//Enables minimal rebuild.
		//dont do this by default to force recompilations to cater for new versions of exodus.dll
		//provide an options?
		//basicoptions^=" /Gm";

		//exodus library
		if (debugging) {
			//Creates a debug multithreaded DLL using MSVCRTD.lib.
			//using MDd causes weird inability to access mv.DICT and other variables in main() despite them being initialised in exodus_main()
			basicoptions ^= " /MDd";
			//basicoptions^=" /MD";

			//Disables optimization.
			basicoptions ^= " /Od";

			//renames program database?? remove?
			basicoptions ^= " /FD";

			//macro used in exodus in some places to provide additional information eg mvdbpostgres
			basicoptions ^= " /D \"DEBUG\"";

			//Enables run-time error checking.
			basicoptions ^= " /RTC1";

		} else {
			//Creates a multithreaded DLL using MSVCRT.lib.
			//basicoptions^=" /MD";
			//needed for backtrace
			//Creates a debug multithreaded DLL using MSVCRTD.lib.
			basicoptions ^= " /MD";

			//Creates fast code.
			//basicoptions^=" /O2";

			//Enable string pooling
			basicoptions ^= " /GF";

			//Enables whole program optimization.
			//basicoptions^=" /GL";

			//turn off asserts
			basicoptions ^= " /D \"NDEBUG\"";
		}

		linkoptions = " /link exodus.lib";

		if (not verbose)
			linkoptions ^= " /nologo";

		outputoption = "";

		//enable function names in backtrace
		//basicoptions^=" -rdynamic";

		//make an exe
		binoptions = "";

		//make a dll
		liboptions = "/LD /DLL /D \"DLL\"";
		//soname?

		//target extensions
		objfileextension = ".exe";
		binfileextension = ".exe";
		libfileextension = ".dll";

		//target directories

		var homedir = osgetenv("USERPROFILE");
		if (homedir) {
			bindir = homedir ^ "\\Exodus\\bin";
			libdir = bindir;
			//installcmd="copy /y";
			installcmd = "copy";
		} else {
			bindir = "";
			libdir = "";
			installcmd = "";
		}

	} // of not posix

	if (bindir[-1] ne OSSLASH)
		bindir ^= OSSLASH;
	if (libdir[-1] != OSSLASH)
		libdir ^= OSSLASH;

	var srcfilenames = "";

	basicoptions ^= " " ^ addoptions;
	linkoptions ^= " " ^ addlinkoptions;

///////////
//initfile:
///////////

	///////////////////////////////
	// find files, scan directories
	///////////////////////////////

	// nfiles may increase during the loop as directories may be included
	//for (const var fileno : range(1, nfiles)) {
	for (var fileno = 1; fileno <= nfiles; fileno++) {

		//var text = "";
		//var filepath_without_ext;

		//get the next file name
		var srcfilename = filenames.f(fileno).unquote();
		if (not srcfilename)
			continue;

		//printl("--- ",srcfilename, " ---", osfile(srcfilename));

		//OK if one of the compilable default file extension
		srcfilename.trimmerlast(".");
		var fileext = srcfilename.field2(".", -1).lcase();
		if (src_extensions.locateusing(" ", fileext)) {
			//filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
		}

		//install/copy header files to inc directory
		else if (inc_extensions.locateusing(" ", fileext)) {
			var srctext = osread(srcfilename);

			var abs_srcfilename = srcfilename;
			if (not abs_srcfilename.starts(OSSLASH))
				abs_srcfilename.prefixer(oscwd() ^ OSSLASH);
			srctext.prefixer("/" "/copied by exodus \"compile " ^ abs_srcfilename ^ DQ ^ EOL);

			//create include directory if doesnt already exist
			call make_include_dir(incdir);

			var targetfilename = incdir ^ OSSLASH ^ srcfilename.field2(OSSLASH, -1);

			if (osread(targetfilename) != srctext) {
				if (not srctext.oswrite(targetfilename)) {
					atomic_ncompilation_failures++;
					errputl(" Error: Could not copy '" ^ srcfilename ^ "' to '" ^ targetfilename ^ "'");
				} else if (not silent) {
					printl(srcfilename);
				}
			} else {
				if (not silent)
					printl(srcfilename);
			}
			continue;
		}

		//skip definitely non-compilable files
		else if (noncompilable_extensions.locateusing(" ", fileext)) {
			//errputl(srcfilename^" "^fileext^" is not compilable");
			continue;
		}

		//skip backup files
		else if (fileext.ends("~")) {
			continue;
		}

		//pickup default file if it exists - even if base file also exists
		else if (osfile(srcfilename ^ "." ^ default_extension)) {
			//filepath_without_ext = srcfilename;
			srcfilename ^= "." ^ default_extension;
		}

		//expand directories into subfiles/directories
		else if (osdir(srcfilename)) {

			var subdirs = oslistd(srcfilename ^
								  "/"
								  "*");
			dim t1;
			if (subdirs) {
				t1 = subdirs.split();
				subdirs = t1.sort().join();
				subdirs = srcfilename ^ "/" ^ subdirs;
				subdirs.replacer(FM, FM ^ srcfilename ^ "/");
				//subdirs.printl("subdirs=");
				filenames.inserter(fileno + 1, subdirs);
			}

			var subfiles = oslistf(srcfilename ^
								   "/"
								   "*");
			dim t2;
			if (subfiles) {
				//subfiles.outputl("subfiles1=");
				t2 = subfiles.split();
				subfiles = t2.sort().join();
				subfiles = srcfilename ^ "/" ^ subfiles;
				subfiles.replacer(FM, FM ^ srcfilename ^ "/");
				//subfiles.outputl("subfiles2=");
				filenames.inserter(fileno + 1, subfiles);
			}

			nfiles = fcount(filenames, FM);
			continue;
		}

		//silently skip files that exist with the wrong header
		else if (osfile(srcfilename) or osdir(srcfilename)) {
			//printl(srcfilename);
			continue;
		}
		//add the default extension
		else {
			//filepath_without_ext = srcfilename;
			srcfilename ^= "." ^ default_extension;
		}
		//search paths and convert to absolute filename
		//SIMILAR CODE IN EDIC and COMPILE
		if (not(osfile(srcfilename)) and not(srcfilename.contains(OSSLASH))) {
			var paths = osgetenv("CPLUS_INCLUDE_PATH").convert(";", ":");
			let npaths = fcount(paths, ":");
			for (const var pathn : range(1, npaths -1)) {
				var srcfilename2 = paths.field(":", pathn) ^ "/" ^ srcfilename;
				if (osfile(srcfilename2)) {
					srcfilename = srcfilename2;
					break;
				}
			}
			//var fileext = srcfilename.field2(".", -1).lcase();
			//filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
		}

		//also look in inc dir backlinks to source
		//SIMILAR CODE IN EDIC and COMPILE
		if (not(osfile(srcfilename)) and not(srcfilename.contains(OSSLASH))) {
			var headerfilename = osgetenv("HOME") ^ OSSLASH ^ "inc" ^ OSSLASH ^ srcfilename;
			// Try .h files for library subroutines first
			headerfilename.fieldstorer(".", -1, 1, "h");
			if (verbose)
				headerfilename.outputl("headerfile=");

			// Try .H header files for programs
			if (!osfile(headerfilename)) {
				headerfilename.fieldstorer(".", -1, 1, "H");
				if (verbose)
					headerfilename.outputl("headerfile=");
			}
			if (osfile(headerfilename)) {
				var headerline1 = osread(headerfilename).field("\n", 1), field("\r", 1);
				//generated by exodus "compile /root/neosys/src/gen/addcent.cpp"
				srcfilename = headerline1.field2(" ", -1).field("\"", 1);
			}
		}

		srcfilenames ^= srcfilename ^ FM;

	}
	srcfilenames.popper();

	// Sort by file size so that the largest files start compiling first
	// and therefore hold up the completion of all compilations the least.
	// This is assuming parallel compilationss.
	// Note that the compilation rate (compiled files per second) is slow
	// to start with but speeds up towards the end on the smaller files.
	var filesizes = "";
	for (var srcfilename : srcfilenames)
		filesizes ^= osfile(srcfilename).f(1) ^ VM;
	filesizes.popper();
	//sort array wants parallel mv fields
	srcfilenames.converter(FM, VM);
	srcfilenames ^= FM ^ filesizes;
	//sort by field 2 which is file sizes and drag field 1 into the same order
	call sortarray(srcfilenames, "2" _VM "1", "DR");
	srcfilenames = srcfilenames.f(1).convert(VM, FM);

///////////
//nextfile:
///////////

	// This loop consists of a single function call handled by a thread
	// with a very long lambda function argument
	for (var srcfilename : srcfilenames) {

		// Post to the thread pool a lambda expression that does the rest of the work
		//
		// WARNING!!! ALL CAPTURED VARIABLES MUST BE INITIALISED IN ADVANCE!
		// OTHERWISE WILL GET MVUNASSIGNED ERROR AT RUNTIME
		//
		// "this" means capture member variables by reference?
		// "=" means capture local variables by value. MAKE SURE ALL ARE ASSIGNED!
		//
		boost::asio::post(
			threadpool1,
			[
				=, this
/*				basicoptions,
				bindir,
				binfileextension,
				binoptions,
				compiler,
				incdir,
				installcmd,
				libdir,
				libfileextension,
				libfileprefix,
				liboptions,
				linkoptions,
				loadtimelinking,
				objfileextension,
				outputoption,
				srcfilename,
				updateinusebinposix
*/
			]() mutable -> void {

			// Ticker
			if (verbose)
				printl("thread: sourcefilename=", srcfilename);
			else if (not silent)
				printl(srcfilename);
			else if (silent eq 1) {
				nasterisks++;
				print("*");
				osflush();
			}

			// Get file text
			////////////////

			var fileext = srcfilename.field2(".", -1).lcase();
			var filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
			var filename_without_ext = filepath_without_ext.field2(OSSLASH, -1);

			var srcfileinfo = osfile(srcfilename);
			if (!srcfileinfo) {
				atomic_ncompilation_failures++;
				srcfilename.quote().errputl("srcfile doesnt exist: ");
				//continue;
				return;
			}

			// Try reading the source text is various locales
			var text;
			var alllocales = "utf8" _FM "en_US.iso88591" _FM "en_GB.iso88591";
			var locales = "";
			//var nlocales = fcount(alllocales, " ");
			var locale;
			var origlocale = getxlocale();
			//for (const var localen : range(1, nlocales)) {
			//for (var locale : alllocales) {
			for (var locale2 : alllocales) {

				//check all but utf8 and skip those not existing
				//use dpkg-reconfigure locales to get more
				//locale = alllocales.field(" ", localen);
				if (locale2 ne "utf8") {
					if (not setxlocale(locale2)) {
						continue;
					}
					setxlocale(origlocale);
				}

				locales ^= " " ^ locale2;

				if (text.osread(srcfilename, locale2)) {
					locale = locale2;
					break;
				}
			}
			if (not text) {
				atomic_ncompilation_failures++;
				srcfilename.errput("Cant read/convert srcfile:");
				errputl(" Encoding issue? unusual characters? - tried " ^ locales);
				errputl("Use 'dpkg-reconfigure locale' to get more");
				//continue;
				return;
			}

			// Determine if program or subroutine/function
			// and decide compile/link options
			var isprogram =
				contains(text, "<exodus/program.h>") or contains(text, "int main(") or contains(text, "program()");
			if (verbose)
				printl("Type=", isprogram ? "Program" : "Subroutine");
			var outputdir;
			var compileoptions;
			var binfilename = filepath_without_ext;
			var objfilename = filepath_without_ext;
			if (isprogram) {
				binfilename ^= binfileextension;
				objfilename ^= objfileextension;
				outputdir = bindir;
				compileoptions = binoptions;
			} else {
				//binfilename^=binfileextension;
				binfilename ^= libfileextension;
				objfilename ^= libfileextension;
				if (libfileprefix) {
					//binfilename=libfileprefix^binfilename;
					//objfilename=libfileprefix^objfilename;
					var nfields = binfilename.fcount(OSSLASH);
					binfilename = fieldstore(binfilename, OSSLASH, nfields, 1, libfileprefix ^ field(binfilename, OSSLASH, nfields));
					nfields = objfilename.fcount(OSSLASH);
					objfilename = fieldstore(objfilename, OSSLASH, nfields, 1, libfileprefix ^ field(objfilename, OSSLASH, nfields));
				}
				outputdir = libdir;
				compileoptions = liboptions;
			}

///////////
//initline:
///////////
			//and, for subroutines and functions, create header file (even if compilation not successful)
			var eol = EOL;
			var headertext = "";
			converter(text, "\r\n", _FM _FM);
			dim text2 = split(text);
			var nlines = text2.rows();

//#if EXODUS_EXPORT_USING_DEF
//			var deftext = "";
//			var defordinal = 0;
//#endif
			//detect libraryinit
			var useclassmemberfunctions = false;

///////////
//nextline:
///////////
//			for (int ln = 1; ln <= nlines; ++ln) {
//				var line = trimfirst(text2(ln));
			for (var& line : text2) {
				line.trimmer(" \t");
				var word1 = line.field(" ", 1);

				//for external subroutines (dll/so libraries), build up .h
				// and maybe .def file text
				if (not isprogram and word1.starts("libraryinit"))
					useclassmemberfunctions = true;

				/////////////////////////
				// GENERATE HEADER IF LIB
				/////////////////////////

				if (not(isprogram) and (word1 eq "function" or word1 eq "subroutine") and not filename_without_ext.starts("dict_")) {

					//extract out the function declaration in including arguments
					//eg "function xyz(in arg1, out arg2)"
					var funcdecl = line.field("{", 1);

					var funcname = funcdecl.field(" ", 2).field("(", 1);

//#if EXODUS_EXPORT_USING_DEF
//					++defordinal;
//					deftext ^= eol ^ " " ^ funcname ^ " @" ^ defordinal;
//#endif
					if (loadtimelinking) {
						headertext ^= eol ^ funcdecl ^ ";";
					} else {
						var libname = filepath_without_ext;
						var returnsvarorvoid = (word1 eq "function") ? "var" : "void";
						var callorreturn = (word1 == "function") ? "return" : "call";
						var funcreturnvoid = (word1 == "function") ? 0 : 1;
						var funcargsdecl = funcdecl.field("(", 2, 999999);

						// Allow for unused arguments to be annotated or commented out to avoid warnings
						// Wrapping parameter names in /* */ also works
						funcargsdecl.replacer("[[maybe_unused]]", "").trimmer();

						//funcargsdecl=funcargsdecl.field(")",1);
						int level = 0;
						int charn;
						for (charn = 1; charn <= len(funcargsdecl); ++charn) {
							var ch = funcargsdecl.b(charn, 1);
							if (ch eq ")") {
								if (level eq 0)
									break;
								--level;
							} else if (ch eq "(")
								++level;
						}
						funcargsdecl.firster(charn - 1);

						//replace comment outs like /*arg1*/ with arg1
						funcargsdecl.regex_replacer(
										"/"
										"\\*",
										"")
							.regex_replacer(
								"\\*"
								"/",
								"");

						//work out the function arguments without declaratives
						//to be inserted in the calling brackets.
						var funcargs = "";
						//default to one template argument for callables with zero arguments

						var nodefaults = contains(funcargsdecl, "=") eq 0;
						var funcargsdecl2 = funcargsdecl;

						var funcargstype = "int";
						if (useclassmemberfunctions)
							funcargstype = "";

						int nargs = fcount(funcargsdecl, ",");

						nodefaults = 0;
						for (int argn = 1; argn <= nargs; ++argn) {
							var funcarg = field(funcargsdecl, ",", argn).trim();

							//remove any default arguments (after =)
							//TODO verify this is ok with <exodus/mvlink.h> method below

							funcarg = field(funcarg, "=", 1).trim();

							//default all if all are var (in io out)
							if (nodefaults) {
								if (var("in io out").locateusing(" ", funcarg.field(" ", 1)))
									fieldstorer(funcargsdecl2, ",", argn, 1, funcarg ^ "=var()");
								else
									//reset to original if anything except in io out
									funcargsdecl2 = funcargsdecl;
							}

							//assume the last word (by spaces) is the variable name
							fieldstorer(funcargs, ",", argn, 1, funcarg.field2(" ", -1));

							//assume everything except the last word (by spaces) is the variable type
							//wrap it in brackets otherwise when filling in missing parameters
							//the syntax could be wrong/fail to compile eg "const var&()" v. "(const var&)()"
							var argtype = field(funcarg, " ", 1, fcount(funcarg, " ") - 1);
							fieldstorer(funcargstype, ",", argn, 1, argtype);
						}

						//develop additional function calls to allow constants to be provided for io/out arguments
						// ie allow out variables to be ignored if not required
						//funcargdecl2 example:	in arg1=var(), out arg2=var(), out arg3=var()
						//funcargstype example:	IN,OUT,OUT
						//funcargs example: 	ARG1,ARG2,ARG3
						var add_funcargsdecl = funcargsdecl2;
						var add_funcargs = funcargs;
						var add_vars = "";
						var add_funcs = "";
						var add_func = "";
						var can_default = true;

						//build nargs additional functions to cater for optional arguments in DOS

						for (int maxargn = 1; maxargn <= nargs; ++maxargn) {

							var inbound_args = "";
							var outbound_args = "";
							var func_body = "";
							var skip_func = false;

							//build a function with fewer arguments than the full set
							for (int argn = 1; argn <= nargs; ++argn) {

								//work out the variable name and default
								var argname = funcargs.field(",", argn);
								var argname2 = argname;

								//work out the argtype
								var argtype = funcargstype.field(",", argn);

								//io/out cannot at the moment have defaults
								var argdefault = funcargsdecl2.field(",", argn).field("=", 2);

								//do not generate functions for numbers of arguments that are equivalent to defaulted arguments in main function
								if (argdefault ne "" and argn eq maxargn) {
									skip_func = true;
									break;
								}

								if (argn < maxargn) {
									inbound_args ^= ", " ^ argtype ^ " " ^ argname;
									if (argdefault ne "") {
										//build a new non-constant dummy variable name to be used as unassigned argument to the real function
										argname2 = argname ^ "_" ^ argtype;
										//declare it
										func_body ^= " var " ^ argname2;
										//default it. DOS doesnt have a "default" for missing args other than "var()" ie unassigned
										if (argdefault ne "" and argdefault ne "var()")
											func_body ^= " = " ^ argdefault;
										func_body ^= ";" _EOL;
									}
								} else {
									//build a new non-constant dummy variable name to be used as unassigned argument to the real function
									argname2 = argname ^ "_" ^ argtype;
									//declare it
									func_body ^= " var " ^ argname2;
									//default it. DOS doesnt have a "default" for missing args other than "var()" ie unassigned
									if (argdefault ne "" and argdefault ne "var()")
										func_body ^= " = " ^ argdefault;
									func_body ^= ";" _EOL;
								}
								outbound_args ^= ", " ^ argname2;
							}

							//functions are not generated in some cases
							if (skip_func)
								continue;

							//remove initial commas
							inbound_args.substrer(3);
							outbound_args.substrer(3);

							//build a function with all the new arguments and dummy variables
							add_func ^= _EOL;
							add_func ^= _EOL "/" "/ Allow call with only " ^ var(maxargn-1) ^ " arg" ^ (maxargn eq 2 ? "" : "s");
							add_func ^= _EOL "var operator() (" ^ inbound_args ^ ") {";
							add_func ^= _EOL ^ func_body;
							add_func ^= " return operator()(" ^ outbound_args ^ ");";
							add_func ^= _EOL "}";
						}

						add_funcs ^= add_func;
						if (add_funcs)
							add_funcs ^= _EOL;

						//new method using member functions to call external functions with mv environment
						//using a callable class that allows library name changing
						//public inheritance only so we can directly access mv in mvprogram.cpp for oconv/iconv. should perhaps be private inheritance and mv set using .init(mv)
						var inclusion =
							_EOL
							_EOL "/" "/ A 'callable' class and object that allows function call syntax to actually open shared libraries/create Exodus Program objects on the fly."
							_EOL
							_EOL "class Callable_funcx : public Callable"
							_EOL "{"
							_EOL "public:"
							_EOL
							_EOL "/" "/ A constructor providing:"
							_EOL "/" "/ 1. The name of the shared library to open,"
							_EOL "/" "/ 2. The name of the function within the shared library that will create an exodus program object,"
							_EOL "/" "/ 3. The current program's mv environment to share with it."
							_EOL "Callable_funcx(ExoEnv& mv) : Callable(mv) {}"
							_EOL
							_EOL "/" "/ Allow assignment of library name to override the default constructed"
							_EOL "using Callable::operator=;"
							_EOL
							_EOL "/" "/ A callable member function with the right arguments, returning a var or void"
							_EOL "var operator() (in arg1=var(), out arg2=var(), out arg3=var())"
							_EOL "{"
							_EOL
							_EOL " /" "/ The first call will link to the shared lib and create/cache an object from it."
							_EOL " /" "/ passing current standard variables in mv"
							_EOL " if (this->pmemberfunc_==NULL)"
//							_EOL "  this->init();"
							_EOL "  this->attach(\"funcx\");"
							_EOL
							_EOL " /" "/ Define a function type (pExodusProgramBaseMemberFunction)"
							_EOL " /" "/ that can call the shared library object member function"
							_EOL " /" "/ with the right arguments and returning a var or void"
							//_EOL " typedef VARORVOID (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(IN,OUT,OUT);"
							_EOL " using pExodusProgramBaseMemberFunction = auto (ExodusProgramBase::*)(IN,OUT,OUT) -> VARORVOID;"
							_EOL
							_EOL " /" "/ Call the shared library object main function with the right args,"
							_EOL " /" "/  returning a var or void"
							_EOL " {before_call}"
							_EOL " return CALLMEMBERFUNCTION(*(this->plibobject_),"
							// 2022-10-23
							// Use reinterpret_cast instead of c-style cast for clarity in code review and assuming equivalence
							//_EOL " ((pExodusProgramBaseMemberFunction) (this->pmemberfunc_)))"
							_EOL " (reinterpret_cast<pExodusProgramBaseMemberFunction>(this->pmemberfunc_)))"
							_EOL "  (ARG1,ARG2,ARG3);"
							_EOL " {after_call}"
							_EOL
							_EOL "}"
							"{additional_funcs}"
							_EOL "};"
							_EOL
							_EOL "/" "/ A callable object of the above type that allows function call syntax to access"
							_EOL "/" "/ an Exodus program/function initialized with the current mv environment."
							_EOL "Callable_funcx funcx{mv};";

						replacer(inclusion, "funcx", field2(libname, OSSLASH, -1));
						//replacer(example,"exodusprogrambasecreatedelete_",funcname);
						replacer(inclusion, "in arg1=var(), out arg2=var(), out arg3=var()", funcargsdecl2);
						replacer(inclusion, "IN,OUT,OUT", funcargstype);
						replacer(inclusion, "ARG1,ARG2,ARG3", funcargs);
						replacer(inclusion, "VARORVOID", returnsvarorvoid);
						replacer(inclusion, "callorreturn", callorreturn);
						replacer(inclusion, "{additional_funcs}", add_funcs);

						if (true or text.contains("-Wcast-function-type")) {
							replacer(inclusion, "{before_call}",
									"#pragma GCC diagnostic push"
									_EOL " #pragma GCC diagnostic ignored \"-Wcast-function-type\"");
							replacer(inclusion, "{after_call}",
									"#pragma GCC diagnostic pop");
						} else {
							replacer(inclusion, _EOL " {before_call}", "");
							replacer(inclusion, _EOL " {after_call}", "");
						}
						var usepredefinedcallable = nargs <= exodus_callable_maxnargs;
						if (useclassmemberfunctions) {
							if (funcname eq "main")
								headertext ^= inclusion;
						}

						else if (usepredefinedcallable) {

							//example output for a subroutine with 1 argument of "in" (const var&)
							//ending of s1 and S1 indicates subroutine of one argument
							//#include "xmvcallables1.h"
							//ExodusCallableS1<in> func2("func2","func2");

							var functype = funcreturnvoid ? "s" : "f";

							var funcdecl = "ExodusCallable" ^ functype.ucase() ^ nargs ^ "<" ^ funcargstype ^ "> ";
							funcdecl ^= funcname ^ "(" ^ libname.quote() ^ "," ^ funcname.quote() ^ ");";

							//dont include more than once in the header file
							//function might appear more than once if declared and defined separately
							if (not headertext.contains(funcdecl)) {
								headertext ^= eol;
								headertext ^= eol;
								var includecallable = "#include <exodus/xcallable" ^ functype ^ nargs ^ ".h>" ^ eol;
								if (not headertext.contains(includecallable))
									headertext ^= includecallable;
								headertext ^= funcdecl;
							}

						} else {
							headertext ^= eol;
							headertext ^= "#define EXODUSLIBNAME " ^ libname.quote() ^ eol;
							headertext ^= "#define EXODUSFUNCNAME " ^ funcname.quote() ^ eol;
							headertext ^= "#define EXODUSFUNCNAME0 " ^ funcname ^ eol;
							headertext ^= "#define EXODUSFUNCRETURN " ^ returnsvarorvoid ^ eol;
							headertext ^= "#define EXODUSFUNCRETURNVOID " ^ funcreturnvoid ^ eol;
							headertext ^= "#define EXODUSfuncargsdecl " ^ funcargsdecl ^ eol;
							headertext ^= "#define EXODUSfuncargs " ^ funcargs ^ eol;
							headertext ^= "#define EXODUSCALLABLECLASSNAME ExodusCallable_" ^ funcname ^ eol;
							headertext ^= "#define EXODUSFUNCTYPE ExodusDynamic_" ^ funcname ^ eol;
							//headertext^="#define EXODUSCLASSNAME Exodus_Callable_Class_"^funcname^eol;
							headertext ^= "#include <exodus/mvlink.h>" ^ eol;
							//undefs are automatic at the end of mvlink.h to allow multiple inclusion
						}
					}

				} // generate header if lib

				//build up list of loadtime libraries required by linker
				if (loadtimelinking and word1 eq "#include") {
					var word2 = line.field(" ", 2);
					if (word2[1] == DQ) {
						word2 = word2.cut(1).pop();
						if (word2.ends(".h"))
							word2.cutter(-2);
						//libnames^=" "^word2;
						if (compiler == "cl")
							linkoptions ^= " " ^ word2 ^ ".lib";
					}
				}

			} // next line

			// embedded linker options
			if (var pos = text.index("/" "/gcc_link_options"); pos) {
				pos += 18;
				linkoptions ^= text.substr2(pos, pos);
			}

			// Update header in ~/inc dir
			/////////////////////////////

			// Generate headers even for executables that do not really need them
			// so that we can find the path to the source code for executables
			// so that we can edic and compile by simple file name without paths
			// Fake header files for executables end in .H instead of .h
			// This allows libraries and programs to have the same base name
			//if (headertext) {
			{
				var abs_srcfilename = srcfilename;
				if (abs_srcfilename[1] != OSSLASH)
					abs_srcfilename.prefixer(oscwd() ^ OSSLASH);
				var EXODUS_CALLABLE_XXXXX_H = "EXODUS_CALLABLE_" ^ ucase(filename_without_ext) ^ "_H";
				headertext.prefixer("#define " ^ EXODUS_CALLABLE_XXXXX_H);
				headertext.prefixer("/" "/#ifndef" ^ EXODUS_CALLABLE_XXXXX_H ^ eol);
				headertext.prefixer("/" "/generated by exodus \"compile " ^ abs_srcfilename ^ DQ ^ eol);
				headertext ^= eol ^ "/" "/#endif" ^ eol;
				//var headerfilename=filepath_without_ext^".h";
				var headerfilename = incdir ^ OSSLASH ^ filename_without_ext ^ (isprogram ? ".H" : ".h");

				//create include directory if doesnt already exist
				call make_include_dir(incdir);

				if (verbose)
					print("header file " ^ headerfilename ^ " ");

				//check if changed
				//var headertext2 = osread(headertext2, headerfilename, locale);
				var headertext2;
				if (not osread(headertext2, headerfilename, locale)) {
				}
				if (headertext2 ne headertext) {

					//over/write if changed
					if (not oswrite(headertext, headerfilename, locale))
						loglasterror();

					//verify written ok
					//var headertext3 = osread(headertext3, headerfilename, locale);
					var headertext3;
					if (not osread(headertext3, headerfilename, locale))
						loglasterror();
					if (headertext3 ne headertext) {
						atomic_ncompilation_failures++;

						errputl("Error: compile could not accurately update " ^ headerfilename ^ " locale: " ^ locale
							^ ". Perhaps multiple files with the same name.\n"
							^ " 1: " ^ headertext.f(1) ^ "\n"
							^ " 2: " ^ headertext2.f(2) ^ "\n"
							^ " 3: " ^ headertext3.f(3));
					}
					else if (verbose)
						printl("generated or updated.");

				} else if (verbose)
					printl("already generated and is up to date.");
			}

			// Skip compilation if generateheadersonly option
			////////////////////////////////////////////////

			if (generateheadersonly)
				return;
				///////

			// Skip compilation if the output file is newer than source file and all include files
			//////////////////////////////////////////////////////////////////////////////////////

			var outfileinfo = osfile(outputdir ^ field2(binfilename, OSSLASH, -1));
			if (outfileinfo and not(force) and not(generateheadersonly) && is_newer(outfileinfo, srcfileinfo) && is_newer(outfileinfo, exodus_include_dir_info)) {

				// Recompile is required if any include file is younger than the current output binary
				bool recompile_required = false;
				// TODO recode to find #index directly instead of by line since includes are generally only in the first part of a program
				for (var line : text) {

					// Skip lines unlike "#include <printplans7.h>"
					if (not line.contains("#include"))
						continue;

					// Acquire include file date/time
					var incfilename = incdir ^ OSSLASH ^ line.field("<", 2).field(">", 1);
					var incfileinfo = osfile(incfilename);

					if (incfileinfo) {
						if (is_newer(incfileinfo, outfileinfo)) {
							recompile_required = true;
							break;
						}
					}

				}

				// Only skip compilation if all headers are older than the current output file
				if (not recompile_required) {
					if (verbose)
						printl("Skipping compilation since the output file is newer than both the source code, its include files and libexodus, and no (F)orce option provided.");
					//continue;
					return;
				}
			}

			// Warn about undefined behaviour not picked up by compilers
			////////////////////////////////////////////////////////////

			// xxx cannot be used in the same statement that it is being defined
			// var xxx = osread(xxx, yyy, zzz);
			let matches = text.match("[\\n \\t]{2,}(var|let) (\\b[a-zA-Z0-9_]+\\b) [^\n:;]*?\\b\\2\\b");
			if (matches) {
				var nmatches = 0;
				for (var match : matches) {

					// Skip matches with an odd number of double quotes since it indicates quoted xxx on the right
					if (match.count(_DQ).mod(2))
						continue;

					// Skip if there is a function or member function with the same name as the variable
					// e.g. "var reccount = file1.reccount();"
					var varname = match.f(1, 1).field(" ", 2);
					if (match.contains("." ^ varname) or match.contains(varname ^ "("))
						continue;

					if (not nmatches++) {
						logputl(srcfilename ^ ":9999:99: warning: Use before constructed is undefined behaviour.");
						atomic_ncompilation_failures++;
					}
					logputl(match.f(1,1).trimmer(" \t\n").quote());
				}
			}

			// Call the actual compile function
			///////////////////////////////////

			// Compile separated out as a separate function in order to be passed to a thread
			// Now the caller is itself a lambda expression that is passed to a thread
			// and the call does not really need to be separated out.
			call compile(
				verbose,
				objfileextension,
				binfileextension,
				binfilename,
				objfilename,
				outputdir,
				windows,
				posix,
				updateinusebinposix,
				compiler,
				srcfilename,
				basicoptions,
				compileoptions,
				outputoption,
				linkoptions,
				installcmd
			);

		} // end of big lambda

		); // end of boost::asio::post function

	}  //goto nextfile

///////
//exit:
///////

	// Join any outstanding threads before terminating
	threadpool1.join();

	if (nasterisks)
		printl();

	return atomic_ncompilation_failures > 0;

}  //main program

function static compile(

	in verbose,
	in objfileextension,
	in binfileextension,
	in binfilename,
	in objfilename0,
	in outputdir,
	in windows,
	in posix,
	in updateinusebinposix,
	in compiler,
	in srcfilename,
	in basicoptions,
	in compileoptions,
	in outputoption,
	in linkoptions,
	in installcmd) {

//	atomic_count++;

	var result = compile2(
	verbose,
	objfileextension,
	binfileextension,
	binfilename,
	objfilename0,
	outputdir,
	windows,
	posix,
	updateinusebinposix,
	compiler,
	srcfilename,
	basicoptions,
	compileoptions,
	outputoption,
	linkoptions,
	installcmd);

//	atomic_count--;

	return result;

}

function static compile2(
	in verbose,
	in objfileextension,
	in binfileextension,
	in binfilename,
	in objfilename0,
	in outputdir,
	in windows,
	in posix,
	in updateinusebinposix,
	in compiler,
	in srcfilename,
	in basicoptions,
	in compileoptions,
	in outputoption,
	in linkoptions,
	in installcmd) {

	var objfilename = objfilename0;

	// Logging
	if (verbose) {
		binfileextension.outputl("Bin file extension=");
		objfileextension.outputl("Obj file extension=");
		binfilename.outputl("Binary file=");
		objfilename.outputl("Object file=");
		outputdir.outputl("Output directory=");
		compileoptions.outputl("Compile options=");
	}

	// Record the current bin file update timestamp so we can
	// later detect if compiler produces anything to be installed
	var oldobjfileinfo = osfile(objfilename);

	// Check object code can be produced.
	if (oldobjfileinfo) {
		if (osopen(objfilename, objfilename))
			osclose(objfilename);
		else if (windows or (posix and not updateinusebinposix)) {
			atomic_ncompilation_failures++;
			objfilename.errput("Error: objectfile cannot be updated. (1) Insufficient rights on ");
			if (windows)
				errput(" or cannot compile programs while in use or blocked by anti-virus/anti-malware");
			errputl(".");
			return "";	//continue;
		}
	}

	// Build the compiler command
	var compilecmd = compiler ^ " " ^ srcfilename ^ " " ^ basicoptions ^ " " ^ compileoptions;
	if (outputoption)
		compilecmd ^= " " ^ outputoption ^ " " ^ objfilename;
	compilecmd ^= linkoptions;

	////////////////////
	// Call the compiler
	////////////////////
	if (verbose)
		printl(compilecmd);
	var compileroutput;
	var compileok = osshellread(compileroutput, compilecmd);
	if (not compileok) {
		atomic_ncompilation_failures++;
	}

	// Handle compiler output
	var startatlineno;
	{
		if (compileroutput)
			compileroutput.outputl();
		var charn = index(compileroutput, ": error:");
		if (charn) {
			startatlineno = compileroutput.b(charn - 9, 9);
			startatlineno = startatlineno.field2(":", -1);
			return "";	//continue;
		}
	}

	// Get new objfile info or continue
	var newobjfileinfo = osfile(objfilename);
	if (not newobjfileinfo) {
		if (compileok)
			atomic_ncompilation_failures++;
		errputl(oscwd());
		objfilename.errputl("Error: Cannot output file ");
		return "";	//continue;
	}

	// Handle new objfile

	if (newobjfileinfo ne oldobjfileinfo) {
		if (newobjfileinfo) {

			//create the bin file directory if missing
			//osdir isnt working as expected
			//			      if (!outputdir.oslistf()) {
			//				      osshell("mkdir " ^ outputdir);
			//				      printl("Created " ^ outputdir);
			//			      }

			//var outputfilename=filename;
			//if (isprogram) {
			//	//remove the .out file type ending
			//	outputfilename=outputfilename.field(".",1,outputfilename.count("."));
			//}

			// Copy the obj file to the output directory
			////////////////////////////////////////////
			if (installcmd) {

				// Make the target directory
				if (not osdir(outputdir)) {
					var cmd = "mkdir " ^ outputdir;
					if (verbose)
						printl(cmd);
					//osshell(cmd);
					if (!osmkdir(outputdir))
						cmd.errputl("ERROR: Failed to make directory");
				}

				// Check can install file
				var outputpathandfile = outputdir ^ field2(binfilename, OSSLASH, -1);
				if (osfile(outputpathandfile)) {
					if (osopen(outputpathandfile, outputpathandfile)) {
						osclose(outputpathandfile);

					} else if (windows or (posix and not updateinusebinposix)) {
						atomic_ncompilation_failures++;
						outputpathandfile.errput("Error: Cannot update ");
						errput(" In use or insufficient rights (2)");
						if (windows)
							errput(" or cannot install/catalog program while in use or blocked by antivirus/antimalware");
						errputl(".");
						return "";	//continue;

					} else {
						outputpathandfile.output("Warning: ");
						printl(" updated while in use. Reload required.");
					}
				}

				// Install the file
				var cmd = installcmd ^ " " ^ objfilename ^ " " ^ outputpathandfile;
				if (verbose)
					printl(cmd);
				if (not osshell(cmd))
					lasterror().errputl("compile could not install ");
				if (osfile(outputpathandfile) and osfile(objfilename)) {
					if (not osremove(outputpathandfile))
						outputpathandfile.errput("compile: Could not remove output path and file ");
				}
			}
		}
	}  //compilation
	return "";
}

function set_environment() {

	if (verbose)
		printl("Searching standard directories");

	var searchdirs = "";

	searchdirs ^= FM ^ osgetenv("CC");

	// Visual Studio future?
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio 11.0\\Common7\\Tools\\";
	searchdirs ^= FM ^ osgetenv("VS110COMNTOOLS");

	searchdirs ^= FM ^ "\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\bin\\";

	// Visual Studio 2010?
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio 10.0\\Common7\\Tools\\";
	searchdirs ^= FM ^ osgetenv("VS100COMNTOOLS");

	searchdirs ^= FM ^ "\\Program Files\\Microsoft SDKs\\Windows\\v7.0a\\bin\\";

	// Visual Studio 2008 (Paid and Express)
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\Tools\\";
	searchdirs ^= FM ^ osgetenv("VS90COMNTOOLS");

	// Visual Studio 2005
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio 8\\Common7\\Tools\\";
	searchdirs ^= FM ^ osgetenv("VS80COMNTOOLS");

	//http://syzygy.isl.uiuc.edu/szg/doc/VisualStudioBuildVars.html for VS6 and VS2003 folder info

	// Visual Studio .NET 2003
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio .NET 2003\\Common7\\Tools\\";
	searchdirs ^= FM ^ osgetenv("V70COMNTOOLS");

	// Visual Studio 6.0
	searchdirs ^= FM ^ "\\Program Files\\Microsoft Visual Studio\\Common\\Tools\\";
	searchdirs ^= FM ^ osgetenv("VS60COMNTOOLS");

	searchdirs.cutter(1).trimmer(FM);
	var searched = "";
	var batfilename = "";

	for (const var ii : range(1, 999999)) {

		var msvs = searchdirs.f(ii);
		if (not msvs)
			break;

		if (msvs[-1] ne OSSLASH)
			msvs ^= OSSLASH;
		searched ^= "\n" ^ msvs;

		// Get lib/path/include from batch file
		if (osdir(msvs)) {
			batfilename = msvs ^ "setenv.cmd";
			if (osfile(batfilename))
				break;
			batfilename = msvs ^ "..\\..\\vc\\vcvarsall.bat";
			if (osfile(batfilename))
				break;
			batfilename = msvs ^ "vsvars32.bat";
			if (osfile(batfilename))
				break;
		}

		// Look for path on C:
		msvs.prefixer("C:");
		searched ^= "\n" ^ msvs;

		// Get lib/path/include from batch file
		if (osdir(msvs)) {
			batfilename = msvs ^ "setenv.cmd";
			if (osfile(batfilename))
				break;
			batfilename = msvs ^ "..\\..\\vc\\vcvarsall.bat";
			if (osfile(batfilename))
				break;
			batfilename = msvs ^ "vsvars32.bat";
			if (osfile(batfilename))
				break;
		}

		batfilename = "";
	}
	if (not batfilename)
		abort("Searching for C++ Compiler:\n" ^ searched ^ "\nCannot find C++ compiler. Set environment variable CC to the MSVS COMMON TOOLS directory");

	var script = "call " ^ batfilename.quote();

	// Work out the options from the PLATFORM_ (and perhaps debug mode)
	var options = PLATFORM;
	if (contains(batfilename, "setenv.cmd")) {
		//sdk71 wants x86 or x64
		//if (options=="x86") options="Win32";
		options = "/" ^ options;
	}
	script ^= " " ^ options;

	var tempfilenamebase = osgetenv("TEMP") ^ "\\exoduscomp$" ^ rnd(99999999);

	// Capture errors from the setenv
	// like the annoying
	// ERROR: The system was unable to find the specified registry key or value.
	script ^= " 2> " ^ tempfilenamebase ^ ".$2";

	// Track first line of batch file which is the compiler configuration line
	if (verbose)
		printl("COMPILER=" ^ script);

	script ^= "\nset";
	if (verbose)
		printl("Calling script ", script);

	// Create a temporary command file
	if (not oswrite(script, tempfilenamebase ^ ".cmd"))
		loglasterror();

	// Run and get the output of the command
	if (not osshell(tempfilenamebase ^ ".cmd > " ^ tempfilenamebase ^ ".$$$"))
		lasterror().errputl("compile failed. ");
	var result;
	if (osread(result, tempfilenamebase ^ ".$$$")) {

		dim vars = split(result.converter("\r\n", _FM _FM));
		for (in line : vars) {
			ossetenv(
				field(line, "=", 1), field(line, "=", 2, 999999));
		}
	}
	osremove(tempfilenamebase ^ ".cmd") or true;
	osremove(tempfilenamebase ^ ".$$$") or true;
	if (verbose) {
		var errtemp;
		if (osread(errtemp, tempfilenamebase ^ ".$2")) {
			//printl(errtemp);
		}
	}
	osremove(tempfilenamebase ^ ".$2") or true;

	return true;
}

function getparam(in result, in paramname, out paramvalue) {
	var posn = index(result.ucase(), "\n" ^ paramname.ucase() ^ "=");
	if (not posn)
		return false;
	paramvalue = result.b(posn + len(paramname) + 2).field("\n", 1);
	return true;
}

function make_include_dir(in incdir) {
	if (not incdir.osdir()) {
		if (not incdir.osmkdir()) {
			errputl("Error: exodus compile could not create dir for header include files. Skipping creation of header file");
		} else if (verbose)
			printl("Created include directory " ^ incdir.squote() ^ " for header");
	}
	return true;
}

function is_newer(in new_file_info, in old_file_info) {

	int new_file_date = new_file_info.f(2);
	int old_file_date = old_file_info.f(2);

	if (new_file_date > old_file_date)
		return true;

	if (new_file_date < old_file_date)
		return false;

	return new_file_info.f(3) > old_file_info.f(3);

}

programexit()
