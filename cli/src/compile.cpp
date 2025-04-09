// NOTE The rhs comment in the following line is in a special format
// REQUIRED instruction to compile and link with pthread library
#include <thread>  //gcc_link_options -lpthread

// Suppress warning
// warning: inlining failed in call to ‘constexpr ExodusProgram::main()::<lambda()>::<lambda>(ExodusProgram::main()::<lambda()>&&)’: --param max-inline-insns-single limit reached [-Winline] 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#pragma GCC diagnostic pop

// Create a threadpool of suitable size to handle
// ncpus/threads obtained from std::thread::hardware_concurrency()
static boost::asio::thread_pool threadpool1(std::thread::hardware_concurrency() + 1);
//static boost::asio::thread_pool threadpool1(1);

// Use an atomic to allow threads to increment global failure count
#include <atomic>
static std::atomic<int> atomic_ncompilation_failures;

// EXO_FORMAT required to append -lfmt to compile and link command
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include <exodus/format.h>
#pragma GCC diagnostic pop

#include <exodus/program.h>
var verbose;
var cleaning;
var exo_post_compile = "";

programinit()

	// CONSTANTS

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
	var exodus_libfile1_info = "";
	var exodus_libfile2_info = "";
	var exodus_libfile3_info = "";
	var nasterisks = 0;

	//.def file is one way on msvc to force library function names to be "undecorated"
	// and therefore usable in on demand library loading
	//Declaring functions with extern "C" is the other way and seems less complicated
	//but msvc warns that functions defined with extern c cannot return var
	//nevertheless, returning vars seems to work in practice and .def files seem
	//more complicated so for now use extern "C" and not .def files
	//#define EXODUS_EXPORT_USING_DEF

func main() {

	// "i" - inline source code
	// "`~!@#$%^&*()_+-=[]\{}|;':",./<>?"
	//	var ascii = "!\"#$%&'()*+,-./:;<=>?@[\]^_`{|}~";

	// Option "i" - inline/immediate compilation
	// or if "filename" contains non-alphanumeric chars (except . / - _ ~)
	if (OPTIONS.contains("i") or not COMMAND.f(2).replace("[[:alnum:]]"_rex, "").convert("./-_~", "").empty()) {
//    if (OPTIONS.contains("i") or not COMMAND.f(2).match("[^[:alnum:]]").match("[^./-_~]").empty())
		return gosub oneline_compile();
	}

	// Get more options from environment and possibly the second word of the command
	var options = OPTIONS ^ osgetenv("EXO_COMPILE_OPTIONS");
	let secondword = COMMAND.f(2);
	if (var("{(").contains(secondword.first()) && var("})").contains(secondword.last())) {
		COMMAND.remover(2);
		options ^= secondword;
	}

#if EXO_MODULE
	let exo_module = EXO_MODULE;
#else
	let exo_module = false;
#endif

	// Global options
	verbose = options.contains("V");
	cleaning = options.contains("L");

	// Options
	let cleaning = options.contains("L");
	let silent = options.count("S");
	let debugging = not options.contains("R");  //no symbols for backtrace
	// Add one optimisation level if EXO_MODULE
	// Default optimisation level from Oct 2023 is 1. Can be reduced to 0 by option o.
	// Also compile will recognise "export CXX_options--O0" in shell
	let optimise = 1 + options.count("O") - options.count("o") + (!!exo_module);
	let generateheadersonly = options.contains("h");
	let force = options.contains("F") or options.contains("P");
	let color_option = options.count("C") - options.count("c");
	let warnings = options.count("W") - options.count("w");
	let warnings_are_errors = options.count("E") - options.count("e");
	let preprocess_only = options.count("P");

	// Skip compile for option X - allows disabling compilations unless generating headers
	if (options.contains("X") and not options.contains("h"))
		return 0;

	if (not exo_post_compile.osgetenv("EXO_POST_COMPILE")) {
//		if (OPTIONS.contains("f"))
			exo_post_compile = "tac | fixdeprecated {U}";
			if (verbose)
				exo_post_compile.paster(-1, 0, "V");
	}

	// If no file/dirnames then default to previous edic
	// SIMILAR code in edic and compile
	if (fcount(COMMAND, FM) < 2) {
		let edic_hist = osgetenv("HOME") ^ "/.config/exodus/edic_hist.txt";
		if (osread(COMMAND, edic_hist)) {
			COMMAND = raise(COMMAND.f(1));
		}
	}

	// Extract file/dirnames
	var filenames = COMMAND.remove(1);
	var nfiles = fcount(filenames, FM);
	if (not filenames or options.contains("H"))
		abort(
R"V0G0N(
SYNTAX
	compile FILENAME|DIRNAME ... [{OPTION...}]
OPTIONS
	R  = Release (No symbols)
	O/OO/OOO = Optimisation levels (Default 'O') (Poorer debugging/backtrace)
	o/oo/ooo = Deoptimise (cancels 'O's)
	W/WW/WWW = Increase warnings
	w/ww/www = Reduce warnings
	E	= Warnings are errors
	e	= Warnings are not errors (default)/cancel E
	V	= Verbose
	S	= Silent (stars only)
	SS	= Super silent (no stars)
	C/c	= Color diagnostics On/off
	h	= Generate headers only
	F	= Force compilation even if output file is newer than all input files
	P	= C++ preprocessor output only, PP = cleanup, PPP reformat (and reindent if 'indent' is installed).
	X	= Skip compilation
	L	= Clean (remove) object and header files that compile installed.
	i	= Inline code                                               // If it looks like code, i is optional.
			compile 'oconv(date(), "D")'                            // Simple expression. NO semicolon.
			compile 'var v1 = date();oswrite(v1 on "t_test.txt");'  // Full code with mandatory semicolon(s).
	b	= Benchmark code. b for 10 test iterations, bb for 100 etc. // As for option i.
	B   = Benchmark code. B for 10 repeats, BB for 100 etc.
			compile 'oconv(date(), "D")' {bbbB}                     // 1000 ops x 10 reps ->  1. 1,000 ops in 1.034 ms. 1.034 μs/op '09 APR 2025'
ENVIRONMENT
	EXO_COMPILE_OPTIONS As above
	CXX_OPTIONS         Depends on c++ compiler used
	CXX                 e.g. g++, clang, c++ with or without full path
	EXO_POST_COMPILE    Can be something like 'tac|fixdeprecated'
)V0G0N");

	// DETERMINE COMPILER/LINKER OPTIONS AND FLAGS
	//////////////////////////////////////////////

	// Use on demand/jit linking using dlopen/dlsym
	let loadtimelinking = false;

	let usedeffile = false;

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
	var liboptions = " -DEXO_LIBRARY";

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
	let updateinusebinposix = true;

	// Hard coded compiler options at the moment
	// Assume msvc (cl) on windows and c++ (g++/clang) otherwise
	if (posix) {


		//////////////////////////
		// POSIX - linux, bsd etc.
		//////////////////////////

		if (verbose)
			printl("Posix environment detected.");

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

		let compiler_version = osshellread(compiler ^ " --version");
		let clang = compiler_version.contains("clang");
		let gcc = not(clang) and compiler_version.contains("Free Software Foundation");

		//c++11/17
		//basicoptions^=" -std=gnu++0x";//pre gcc 4.7 enables gnu extentions.
		//basicoptions^=" -std=gnu++11";//gcc 4.7 and later  enables gnu extensions.
		//__cplusplus is 201103L
		//if( __cplusplus >= 201103L )
		//basicoptions^=" -std=c++11";//gcc 4.7 and later DISABLES gnu extensions
		//if  (__cplusplus >= 202003L)

		// c++ standard feature support by g++ version
		// https://gcc.gnu.org/projects/cxx-status.html

		// Hard code best c++ standard version
		// Discovered from man pages
		// man g++-13|\grep c++2[0-9a-z] -o|sort|uniq
		int compiler_version_no = compiler_version.match("\\d+\\.\\d+\\.\\d+").f(1).field(".", 1);
		var std = "";
		if (gcc) {
			switch (compiler_version_no) {
				case 9:  // Ubuntu 20.04 g++ default
					std = "c++2a";
					break;
				case 10: // Ubuntu 20.04 g++ max
					std = "c++20"; // c++2a
					break;
				case 11: // Ubuntu 22.04 g++ default
				case 12: // Ubuntu 22.04 g++ max
				case 13: // Ubuntu 24.04 g++ default
					//std = "c++20"; // c++2a
					std = "c++23"; // c++2b
					break;
				case 14: // Ubuntu 24.04 g++ max
					//std = "c++20"; // c++2a
					//std = "c++23"; // c++2b
					std = "c++26"; // c++2c
					break;
				default:
					// Max supported will be discovered from man pages below
					{}
			}
		} else if (clang) {
			switch (compiler_version_no) {
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
					std = "c++2a";
					break;
				case 15:
				case 16:
					//std = "c++20";
					std = "c++2b"; // some c++23
					break;
				case 17:
				case 18:
					//std = "c++20";
					//std = "c++23";
					std = "c++2c"; // some c++26
					break;
				default:
					// Max supported will be discovered from man pages below
					{}
			}
		}

		// If unknown compiler name or version, try to find latest supported version in man pages
		if (not std) {

			// Determine the actual compiler being used
			let manpage_compiler = osshellread("readlink -f `which " ^ compiler ^ "`").trim("\n").field(_OSSLASH, -1);
			// /x86_64-linux-gnu-g++-13
			// /usr/lib/llvm-18/bin/clang

			// Find all man page occurrences of c++2X and c++3X where X is a digit or letter
			let manpage_versions = osshellread("man " ^ manpage_compiler ^ "|grep c++[23][0-9a-z] -o|sort -r|uniq").trim("\n");

			// Choose the greatest c++ standard available from the compiler according to its man page
			std = manpage_versions.field("\n", 1);
			if (verbose)
				printl("man pages for", compiler, manpage_compiler, "-> latest -std=" ^ std);
		}

		// Append c++ standard option
		if (std) {
			basicoptions ^= " -std=" ^ std ^ " ";
		}
//		if (__cplusplus >= 201709)
//			basicoptions ^= " -std=c++2a";  //gcc 4.7 and later DISABLES gnu extensions
//		else
//			basicoptions ^= " -std=c++17";  //gcc 4.7 and later DISABLES gnu extensions

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

		linkoptions = "  -lexodus";

		// Precompiled modules need -f ... var.pcm and -l ... libexovar.so
#if EXO_MODULE
		linkoptions ^= " -lexovar -lexoprog -lstd";
#endif

		// Mandatory c++ library and file system library
		linkoptions ^= " -lstdc++fs -lstdc++";

		// link to fmt library (only if using it instead of std::format)
//#if __has_include(<fmt/core.h>)
//#if EXO_FORMAT == 2
#if defined(EXO_FORMAT) && EXO_FORMAT == 2
		linkoptions ^= " -lfmt ";
#endif

		// Append ~/inc directory
		basicoptions ^= " -I" ^ incdir ^ " ";

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
			basicoptions ^= " -g3 ";

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
				basicoptions ^= " -O" ^ optimise ^ " ";
		}
		//}

		// Minor space savings
		basicoptions ^= " -ffunction-sections -fdata-sections ";

		// Help fixdeprecated get all errors and deprecations
		if (clang)
			basicoptions ^= " -ferror-limit=9999";
		else if (gcc)
			basicoptions ^= " -fmax-errors=9999";

		// Use c++ (g++/clang) -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
#if __GNUC__ >= 4
		liboptions ^= " -fvisibility=hidden -fvisibility-inlines-hidden ";
#endif

#ifdef EXO_FORMAT
		// Bequeath/perpetuate the state of EXO_FORMAT to compiled programs
		// as at original build time of the current compiler object
		basicoptions ^= " -DEXO_FORMAT";
#endif

#if EXO_MODULE

		// Bequeath/perpetuate the state of EXO_MODULE to compiled programs
		// as at original build time of the current compiler object
		basicoptions ^= " -DEXO_MODULE=1";

		// Add path to var and std modules
		// std exovar exoprog
		basicoptions ^= " -fmodule-file=var=/usr/local/lib/var.pcm";
		basicoptions ^= " -fmodule-file=std=/usr/local/lib/std.pcm";
		basicoptions ^= " -fmodule-file=exoprog=/usr/local/lib/exoprog.pcm";
#endif
		if (color_option) {
			basicoptions ^= " -fdiagnostics-color=";
			basicoptions ^= color_option > 0 ? "always" : "never";
		}

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

			// Warnings for dangerous implicit conversions e.g. int -> char
			basicoptions ^= " -Wconversion";

		}

		if (warnings > 0) {

			if (clang)
				basicoptions ^= " -Weverything ";
			else {
				//basicoptions ^= " -Weffc++ ";
				basicoptions ^= " -Wwrite-strings ";
				basicoptions ^= " -Wno-unknown-pragmas ";
			}

			// Show these warnings at any level > 0
			//if (warnings < 0) {
			//	basicoptions ^= " -Wno-extra-semi-stmt";
			//	basicoptions ^= " -Wno-newline-eof";
			//}

			// The following warnings will only appear at level 2 or greater
			if (warnings < 2) {
				basicoptions ^= " -Wno-shadow";
				basicoptions ^= " -Wno-unreachable-code";
				basicoptions ^= " -Wno-unreachable-code-return";
				basicoptions ^= " -Wno-unused-macros";
				basicoptions ^= " -Wno-missing-noreturn";
				//if (clang) {
					basicoptions ^= " -Wno-unreachable-code-return";
					// use of GNU ?: conditional expression extension, omitting middle operand
					basicoptions ^= " -Wno-gnu-conditional-omitted-operand";
				//}
			}

			if (warnings < 3) {
				basicoptions ^= " -Wno-padded";
			}

			// The following warnings will only appear at level 3 or greater
			if (warnings >= 3) {
				//basicoptions ^= " -Wno-missing-prototypes";
				//basicoptions ^= " -Wno-weak-vtables";
				basicoptions ^= " -Wpedantic";// Disallows "Elvis operator" ?:
			}
			//basicoptions ^= " -Wno-cast-function-type";
		}

		if (clang) {
			// Never warn about c++98 compatibility
			basicoptions ^= "  -Wno-c++98-compat";
			basicoptions ^= " -Wno-c++98-compat-pedantic";

			// Never warn about macro expansion since it is essential for mv.RECORD etc.
			basicoptions ^= " -Wno-disabled-macro-expansion";
		}

		if (warnings_are_errors) {
			// Omitting this by default to avoid blocking application programmers
			// over technical matters that may arise in new versions of compilers
			basicoptions ^= " -Werror";
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
					//basicoptions ^= " -Wno-uninitialized";
					basicoptions ^= " -Wuninitialized";

					// Ignore these warnings caused by [[no-discard]] for now
					// TODO Remove when all such code in exodus service and apps is replaced
					// warning: ignoring return value of function declared with 'nodiscard' attribute [-Wunused-result]
					//basicoptions ^= " -Wno-unused-result";

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
		outputoption = " -o";

		//general options
		binoptions = "";
		//binoptions=" -g -L./ -lfunc1 -Wl,-rpath,./";
		//binoptions=" -fPIC";

		//how to make a shared library
		liboptions ^= " -fPIC -shared";
		//soname?

		// TODO Dont hard code for typical Ubuntu
		exodus_libfile1_info = osfile("/usr/local/lib/libexodus.so");
		exodus_libfile2_info = osfile("/usr/local/lib/libexovar.so");
		exodus_libfile3_info = osfile("/usr/local/lib/libexovar.so");
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

		let ndeep = fcount(EXECPATH, OSSLASH);
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

		let homedir = osgetenv("USERPROFILE");
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

	if (not bindir.ends(OSSLASH))
		bindir ^= OSSLASH;
	if (not libdir.ends(OSSLASH))
		libdir ^= OSSLASH;

	var srcfilenames = "";

	basicoptions ^= "  " ^ addoptions;
	linkoptions ^= "  " ^ addlinkoptions;

///////////
//initfile:
///////////

	///////////////////////////////
	// find files, scan directories
	///////////////////////////////

	var common_filenames = "";

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
		let fileext = srcfilename.field(".", -1).lcase();
		if (src_extensions.locateusing(" ", fileext)) {
			//filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
		}

		// Install/copy and header files found to inc directory
		// .h files are not compiled. They are COPIED.
		// except xxx_common.h files are used to GENERATE two line .cpp files
		else if (inc_extensions.locateusing(" ", fileext)) {
			var srctext = osread(srcfilename);

			// Hotfix old commons from 2 arg macros to 1 arg
			if (srcfilename.ends("_common.h")) {
				let orig_srctext = srctext;
				srctext.replacer(R"__(common(init|exit)\(([a-z]{2,3}),[a-z]{2,3}_common_no\))__"_rex, "common$1($2)");
				if (srctext ne orig_srctext) {
					if (oswrite(srctext on srcfilename))
						srcfilename.logputl("Hotfix commoninit/exit:");
					else
						loglasterror();
				}
			}

			var abs_srcfilename = srcfilename;
			if (not abs_srcfilename.starts(OSSLASH))
				abs_srcfilename.prefixer(oscwd() ^ OSSLASH);
			srctext.prefixer("/" "/copied by exodus \"compile " ^ abs_srcfilename ^ DQ ^ EOL);

			let targetfilename = incdir ^ OSSLASH ^ srcfilename.field(OSSLASH, -1);

			if (cleaning) {
				if (osfile(targetfilename)) {
					if (not osremove(targetfilename)) {
						loglasterror();
						continue;
					}
					if (verbose)
						logputl(targetfilename ^ " removed.");
				}
				continue;
			}

			//create include directory if doesnt already exist
			call make_include_dir(incdir);

			if (osread(targetfilename) != srctext) {

				if (not srctext.oswrite(targetfilename)) {
					atomic_ncompilation_failures++;
					errputl(" Error: Could not copy '" ^ srcfilename ^ "' to '" ^ targetfilename ^ "'");
				} else if (not silent) {
					printl(srcfilename);
					//printx(srcfilename ^ _EOL);
				}

			}

			// Create .cpp files for xxx_common.h
			if (targetfilename.match("\\b[a-z]{2,3}_common.h")) {

				// .cpp for a common library is very simple
				let hdrfilename = field(srcfilename, "/", -1);
				let cpp =
					"#define EXO_COMMON_EXTERN\n"
					"#include <" ^ hdrfilename ^ ">\n";

				// Update the cpp file only if changed
				let cppfilename = fieldstore(srcfilename, ".", -1, 1, "cpp");
				let oldcpp = osread(cppfilename);
				if (cpp ne oldcpp) {
					if (not oswrite(cpp on cppfilename)) {
						atomic_ncompilation_failures++;
						errputl(lasterror());
					} else {
						outputl(" Created/updated:" ^ cppfilename);

					}
				}
				common_filenames ^= cppfilename ^ FM;
			}

			continue;

		} // .h file

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
			let paths = osgetenv("CPLUS_INCLUDE_PATH").convert(";", ":");
			let npaths = fcount(paths, ":");
			for (const var pathn : range(1, npaths -1)) {
				let srcfilename2 = paths.field(":", pathn) ^ "/" ^ srcfilename;
				if (osfile(srcfilename2)) {
					srcfilename = srcfilename2;
					break;
				}
			}
			//var fileext = srcfilename.field((".", -1).lcase();
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
				let headerline1 = osread(headerfilename).field("\n", 1), field("\r", 1);
				//generated by exodus "compile /root/neosys/src/gen/addcent.cpp"
				srcfilename = headerline1.field(" ", -1).field("\"", 1);
			}
		}

		srcfilenames ^= srcfilename ^ FM;

	} // fileno

	// Compile commons asynchronously in advance in case new or something has changed
	/////////////////////////////////////////////////////////////////////////////////
	//
	// /~lib/libxxx_common.so
	// lib file MUST be present and up to date when compiling and linking any cpp file
	// that has "#include <xxx_common.h>"
	//
	if (common_filenames) {
		var cmd = "compile " ^ common_filenames.pop().convert(FM, " ");
		if (verbose)
			cmd ^= " {V}";
		if (not osshell(cmd))
			loglasterror();
	}

	srcfilenames.popper();

	// Sort by file size so that the largest files start compiling first
	// and therefore hold up the completion of all compilations the least.
	// This is assuming parallel compilationss.
	// Note that the compilation rate (compiled files per second) is slow
	// to start with but speeds up towards the end on the smaller files.
	var filesizes = "";
	var fn = 0;
	for (var srcfilename : srcfilenames) {
		fn++;
		if (locate(srcfilename, common_filenames)) {
			// Suppress xxx_common.cpp files already compiled above
			srcfilenames.updater(fn, "");
		} else {
			filesizes ^= osfile(srcfilename).f(1);
		}
		filesizes ^= _VM;
	}
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
	// with a VERY long lambda function argument
	for (var srcfilename : srcfilenames) {

		if (srcfilename.empty())
			continue;

		// Post to the thread pool a lambda expression that does the rest of the work
		//
		// WARNING!!! ALL CAPTURED VARIABLES MUST BE INITIALISED IN ADVANCE!
		// OTHERWISE WILL GET MVUNASSIGNED ERROR AT RUNTIME
		//
		// "this" means capture member variables by reference?
		// "=" means capture local variables by value. MAKE SURE ALL ARE ASSIGNED!
		//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
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
				preprocess_only,
				srcfilename,
				updateinusebinposix
*/
			]() mutable -> void {

			// big lamda

			//threads need to catch errors otherwise they will terminate the program without printing any error.
			try {

			// Ticker
			if (verbose)
				printl("thread: sourcefilename=", srcfilename);
			else if (not silent)
				printl(srcfilename);
				//printx(srcfilename ^ _EOL);
			else if (silent eq 1) {
				nasterisks++;
				printx("*");
				osflush();
			}

			// Get file text
			////////////////

			let fileext = srcfilename.field(".", -1).lcase();
			let filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
			let filename_without_ext = filepath_without_ext.field(OSSLASH, -1);

			let srcfileinfo = osfile(srcfilename);
			if (!srcfileinfo) {
				atomic_ncompilation_failures++;
				srcfilename.quote().errputl("srcfile doesnt exist: ");
				//continue;
				return;
			}

			// Try reading the source text is various locales starting with "" no/default locale
			var text;
			let alllocales = _FM "utf8" _FM "en_US.iso88591" _FM "en_GB.iso88591";
			var locales = "";
			//var nlocales = fcount(alllocales, " ");
			var locale;
			let origlocale = getxlocale();
			//for (const var localen : range(1, nlocales)) {
			//for (var locale : alllocales) {
			for (var locale2 : alllocales) {

				//check all but utf8 and skip those not existing
				//use dpkg-reconfigure locales to get more
				//locale = alllocales.field(" ", localen);
				if (locale2 && locale2 ne "utf8") {
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

			{ // hotfixing src
				let orig_text = text;
//				text.replacer(R"__(^function )__"_rex, "func ");
//				text.replacer(R"__(^subroutine )__"_rex, "subr ");
//				text.replacer("ostempfile", "ostempfile");
//				text.replacer("ostempdir", "ostempdir");
//				text.replacer(R"__(space\(\d\))__", "space($1)");
				text.replacer(R"__(\bvar\((\d+)\).space\(\))__"_rex, "space($1)");

				if (text ne orig_text) {
					if (oswrite(text on srcfilename, locale))
						srcfilename.logputl("Hotfix various:");
					else
						loglasterror();
				}
			}

			// Determine if program or subroutine/function
			// and decide compile/link options
			let isprogram =
//				text.contains("exodus/program.h") or text.contains("int main(") or text.contains("programinit(");
//				 text.contains("exodus/program.h")
//				 or text.contains("int main(")
//				 or text.contains("programinit(")
				 text.match("^\\s*#\\s*include\\s*[<\"]exodus/program.h")
				 or text.match("^\\s*int\\s*main\\s*\\(")
				 or text.match("^\\s*programinit\\s*\\(")
				;
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
			let eol = EOL;
			var newheadertext = "";
			converter(text, "\r\n", _FM _FM);
			dim text2 = split(text);
			let nlines = text2.rows();

//#if EXODUS_EXPORT_USING_DEF
//			let deftext = "";
//			let defordinal = 0;
//#endif
			//detect libraryinit
			var useclassmemberfunctions = false;

			bool past_programinit_libraryinit = false;

///////////
//nextline:
///////////
//			for (int ln = 1; ln <= nlines; ++ln) {
//				let line = trimfirst(text2(ln));
			for (var& line : text2) {
				line.trimmer(" \t");
				let word1 = line.field(" ", 1);

				//for external subroutines (dll/so libraries), build up .h
				// and maybe .def file text
				if (not isprogram and word1.starts("libraryinit"))
					useclassmemberfunctions = true;

				if (word1.starts("libraryinit(") or word1.starts("libraryinit("))
					past_programinit_libraryinit = true;

				/////////////////////////
				// GENERATE HEADER IF LIB
				/////////////////////////

				if (
					not(isprogram)
					and (word1 eq "function" or word1 eq "subroutine" or word1 eq "func" or word1 eq "subr")
					and not filename_without_ext.starts("dict_")
//					and not srcfilename.ends("_common.h")
				) {
//TRACE(srcfilename)
					//extract out the function declaration in including arguments
					//eg "function xyz(in arg1, out arg2)"
					let funcdecl = line.field("{", 1);

					let funcname = funcdecl.field(" ", 2).field("(", 1);

//#if EXODUS_EXPORT_USING_DEF
//					++defordinal;
//					deftext ^= eol ^ " " ^ funcname ^ " @" ^ defordinal;
//#endif
					if (loadtimelinking) {
						newheadertext ^= eol ^ funcdecl ^ ";";
					} else {
						let libname = filepath_without_ext;
						let returntype = word1 eq "subroutine" ? "void" : "var";
						//var returnsvarorvoid = (word1 eq "function") ? "var" : "void";
						let callorreturn = (word1 == "function" or word1 == "func") ? "return" : "call";
						let funcreturnvoid = (word1 == "function" or word1 == "func") ? 0 : 1;
						var funcargsdecl = funcdecl.field("(", 2, 999999);

						// Allow for unused arguments to be annotated or commented out to avoid warnings
						// Wrapping parameter names in /* */ also works
						funcargsdecl.replacer("[[maybe_unused]]", "");
						funcargsdecl.trimmer();

						// Cut after the closing ), ignoring any additional pairs of ( ) beforehand
						{
							int level = 0;
							for (int charn : range(1, len(funcargsdecl))) {
								let ch = funcargsdecl.at(charn);
								if (ch eq ")") {
									if (level eq 0) {
										//funcargsdecl.firster(charn - 1);
										funcargsdecl.firster(static_cast<size_t>(charn - 1));
										break;
									}
									--level;
								} else if (ch eq "(")
									++level;
							}
						}
//						funcargsdecl.firster(charn - 1);

						// Replace comment outs like /*arg1*/ with arg1
						funcargsdecl.replacer(rex("/" "\\*"), "");
						funcargsdecl.replacer(rex("\\*" "/"), "");

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
							fieldstorer(funcargs, ",", argn, 1, funcarg.field(" ", -1));

							//assume everything except the last word (by spaces) is the variable type
							//wrap it in brackets otherwise when filling in missing parameters
							//the syntax could be wrong/fail to compile eg "const var&()" v. "(const var&)()"
							let argtype = field(funcarg, " ", 1, fcount(funcarg, " ") - 1);
							fieldstorer(funcargstype, ",", argn, 1, argtype);
						}

						//develop additional function calls to allow constants to be provided for io/out arguments
						// ie allow out variables to be ignored if not required
						//funcargdecl2 example:	in arg1=var(), out arg2=var(), out arg3=var()
						//funcargstype example:	IN,OUT,OUT
						//funcargs example: 	ARG1,ARG2,ARG3
						let add_funcargsdecl = funcargsdecl2;
						let add_funcargs = funcargs;
						let add_vars = "";
						var add_funcs = "";
						var add_func = "";
						let can_default = true;

						//build nargs additional functions to cater for optional arguments in DOS

						for (int maxargn = 1; maxargn <= nargs; ++maxargn) {

							var inbound_args = "";
							var outbound_args = "";
							var func_body = "";
							var skip_func = false;

							//build a function with fewer arguments than the full set
							for (int argn = 1; argn <= nargs; ++argn) {

								//work out the variable name and default
								let argname = funcargs.field(",", argn);
								var argname2 = argname;

								//work out the argtype
								let argtype = funcargstype.field(",", argn);

								//io/out cannot at the moment have defaults
								let argdefault = funcargsdecl2.field(",", argn).field("=", 2);

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
							add_func ^= _EOL ^ returntype ^ " operator() (" ^ inbound_args ^ ") {";
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
							_EOL "#pragma clang diagnostic push"
							_EOL "#pragma clang diagnostic ignored \"-Wweak-vtables\""
							_EOL "class Callable_funcx : public Callable"
							_EOL "{"
							_EOL "#pragma clang diagnostic pop"
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
							_EOL "#pragma GCC diagnostic push"
							_EOL "#pragma GCC diagnostic ignored \"-Wshadow\""
							_EOL "RETURNTYPE operator() (in arg1=var(), out arg2=var(), out arg3=var())"
							_EOL "{"
							_EOL
							_EOL " /" "/ The first call will link to the shared lib and create/cache an object from it."
							_EOL " /" "/ passing current standard variables in mv"
							_EOL " if (this->pmemberfunc_== nullptr)"
//							_EOL "  this->init();"
							_EOL "  this->attach(\"funcx\");"
							_EOL
							_EOL " /" "/ Define a function type (funcx_pExoProgram_MemberFunc)"
							_EOL " /" "/ that can call the shared library object member function"
							_EOL " /" "/ with the right arguments and returning a var or void"
							//_EOL " typedef RETURNTYPE (ExoProgram::*funcx_pExoProgram_MemberFunc)(IN,OUT,OUT);"
							_EOL " using funcx_pExoProgram_MemberFunc = auto (ExoProgram::*)(IN,OUT,OUT) -> RETURNTYPE;"
							_EOL
							_EOL " /" "/ Call the shared library object main function with the right args,"
							_EOL " /" "/  returning a var or void"
							_EOL " {before_call}"
							_EOL " return CALLMEMBERFUNCTION(*(this->plibobject_),"
							// 2022-10-23
							// Use reinterpret_cast instead of c-style cast for clarity in code review and assuming equivalence
							//_EOL " ((funcx_pExoProgram_MemberFunc) (this->pmemberfunc_)))"
							_EOL " (reinterpret_cast<funcx_pExoProgram_MemberFunc>(this->pmemberfunc_)))"
							_EOL "  (ARG1,ARG2,ARG3);"
							_EOL " {after_call}"
							_EOL
							_EOL "}"
							"{additional_funcs}"
							_EOL "};"
							_EOL "#pragma GCC diagnostic pop"
							_EOL
							_EOL "/" "/ A callable object of the above type that allows function call syntax to access"
							_EOL "/" "/ an Exodus program/function initialized with the current mv environment."
							_EOL "Callable_funcx funcx{mv};";

						replacer(inclusion, "funcx", field(libname, OSSLASH, -1));
						//replacer(example,"exoprogram_createdelete_",funcname);
						replacer(inclusion, "in arg1=var(), out arg2=var(), out arg3=var()", funcargsdecl2);
						replacer(inclusion, "IN,OUT,OUT", funcargstype);
						replacer(inclusion, "ARG1,ARG2,ARG3", funcargs);
						replacer(inclusion, "RETURNTYPE", returntype);
						replacer(inclusion, "callorreturn", callorreturn);
						replacer(inclusion, "{additional_funcs}", add_funcs);

						//if (true or text.contains("-Wcast-function-type")) {
						replacer(inclusion, "{before_call}",
								"#pragma GCC diagnostic push"
								_EOL " #pragma GCC diagnostic ignored \"-Wcast-function-type\"");
						replacer(inclusion, "{after_call}",
								"#pragma GCC diagnostic pop");
//						} else {
//							replacer(inclusion, _EOL " {before_call}", "");
//							replacer(inclusion, _EOL " {after_call}", "");
//						}
						let usepredefinedcallable = nargs <= exodus_callable_maxnargs;
						if (useclassmemberfunctions) {
							if (funcname eq "main") {
								// Functions return var and subroutines return void
								newheadertext ^= inclusion;
							}
						}

						else if (usepredefinedcallable) {

							//example output for a subroutine with 1 argument of "in" (const var&)
							//ending of s1 and S1 indicates subroutine of one argument
							//#include "xmvcallables1.h"
							//ExodusCallableS1<in> func2("func2","func2");

							let functype = funcreturnvoid ? "s" : "f";

							var funcdecl = "ExodusCallable" ^ functype.ucase() ^ nargs ^ "<" ^ funcargstype ^ "> ";
							funcdecl ^= funcname ^ "(" ^ libname.quote() ^ "," ^ funcname.quote() ^ ");";

							//dont include more than once in the header file
							//function might appear more than once if declared and defined separately
							if (not newheadertext.contains(funcdecl)) {
								newheadertext ^= eol;
								newheadertext ^= eol;
								let includecallable = "#include <exodus/xcallable" ^ functype ^ nargs ^ ".h>" ^ eol;
								if (not newheadertext.contains(includecallable))
									newheadertext ^= includecallable;
								newheadertext ^= funcdecl;
							}

						} else {
							newheadertext ^= eol;
							newheadertext ^= "#define EXODUSLIBNAME " ^ libname.quote() ^ eol;
							newheadertext ^= "#define EXODUSFUNCNAME " ^ funcname.quote() ^ eol;
							newheadertext ^= "#define EXODUSFUNCNAME0 " ^ funcname ^ eol;
							newheadertext ^= "#define EXODUSFUNCRETURN " ^ returntype ^ eol;
							newheadertext ^= "#define EXODUSFUNCRETURNVOID " ^ funcreturnvoid ^ eol;
							newheadertext ^= "#define EXODUSfuncargsdecl " ^ funcargsdecl ^ eol;
							newheadertext ^= "#define EXODUSfuncargs " ^ funcargs ^ eol;
							newheadertext ^= "#define EXODUSCALLABLECLASSNAME ExodusCallable_" ^ funcname ^ eol;
							newheadertext ^= "#define EXODUSFUNCTYPE ExodusDynamic_" ^ funcname ^ eol;
							//newheadertext^="#define EXODUSCLASSNAME Exodus_Callable_Class_"^funcname^eol;
							newheadertext ^= "#include <exodus/mvlink.h>" ^ eol;
							//undefs are automatic at the end of mvlink.h to allow multiple inclusion
						}
					}

				} // generate header if lib

				// Add linker options like -lxxx_common for #include <xxx_common.h>
				// iif the xxx_common.so lib exists
				if (word1 eq "#include") {
					let libname = line.match(R"__(\b[a-z]{2,3}_common.h)__").field(".h", 1);
					if (libname and not srcfilename.contains(libname)) {
						if (past_programinit_libraryinit) {
							//errput("compile:" ^srcfilename ^ " Should not occur after programinit or libraryinit: ");
							//TRACE(line)
						}
						if (not generateheadersonly and not libinfo(libname))
							errputl(libname.quote() ^ " required but missing for " ^ srcfilename);
						if (not linkoptions.contains(" -L" ^ libdir))
							linkoptions ^= " -L" ^libdir;
						linkoptions ^= " -l" ^ libname;
					}
				}

				//build up list of loadtime libraries required by linker
				if (loadtimelinking and word1 eq "#include") {
					var word2 = line.field(" ", 2);
					if (word2.starts(DQ)) {
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
			//if (newheadertext) {
			// TODO only skip files containing common_init common_exit
			// and continue to process agency_common.h
			if (not srcfilename.match("\\b[a-z]{2,3}_common.cpp"))
			{
				var abs_srcfilename = srcfilename;
				if (not abs_srcfilename.starts(OSSLASH))
					abs_srcfilename.prefixer(oscwd() ^ OSSLASH);
				let EXODUS_CALLABLE_XXXXX_H = "EXODUS_CALLABLE_" ^ ucase(filename_without_ext) ^ "_H";
				newheadertext.prefixer("#define " ^ EXODUS_CALLABLE_XXXXX_H);
				newheadertext.prefixer("/" "/#ifndef " ^ EXODUS_CALLABLE_XXXXX_H ^ eol);
				newheadertext.prefixer("/" "/Generated by exodus \"compile " ^ abs_srcfilename ^ DQ ^ eol);
				newheadertext ^= eol ^ "/" "/#endif" ^ eol;
				//var headerfilename=filepath_without_ext^".h";
				let headerfilename = incdir ^ OSSLASH ^ filename_without_ext ^ (isprogram ? ".H" : ".h");

				//create include directory if doesnt already exist
				call make_include_dir(incdir);

				if (verbose)
					printx("header file " ^ headerfilename ^ " ");

				//check if changed
				//var oldheadertext = osread(oldheadertext, headerfilename, locale);
				var oldheadertext;
				osflush();
				if (not osread(oldheadertext, headerfilename, locale)) {
				}

				// Cleaning
				if (cleaning) {
					if (osfile(headerfilename)) {
						if (not osremove(headerfilename))
							loglasterror();
						else if (verbose)
							logputl(headerfilename ^ " cleaned.");
					}
				}

				else if (oldheadertext ne newheadertext) {

//					TRACE(headerfilename)
//					TRACE(newheadertext)
//					TRACE(oldheadertext)

					//over/write if changed
					if (not oswrite(newheadertext, headerfilename, locale))
						loglasterror();

					//verify written ok
					//var chkheadertext = osread(chkheadertext, headerfilename, locale);
					var chkheadertext;
					if (not osread(chkheadertext, headerfilename, locale))
						loglasterror();

//					TRACE(chkheadertext)
//					oswrite(newheadertext, "1");
//					oswrite(oldheadertext, "2");
//					oswrite(chkheadertext, "3");

					if (chkheadertext ne newheadertext) {
						atomic_ncompilation_failures++;

						errputl("Error: compile could not accurately update " ^ headerfilename ^ " locale: " ^ locale
							^ ". Perhaps multiple files in different dirs with the same name compiling at the same same.\n"
							^ " 1: " ^ newheadertext.f(1).quote() ^ "\n"
							^ " 2: " ^ oldheadertext.f(1).quote() ^ "\n"
							^ " 3: " ^ chkheadertext.f(1).quote()
						);
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

			let outfileinfo = osfile(outputdir ^ field(binfilename, OSSLASH, -1));
//TRACE(is_newer(outfileinfo, exodus_include_dir_info))
//TRACE(is_newer(outfileinfo, exodus_libfile_info))
			if (outfileinfo
				and not(force)
				and not(generateheadersonly)
				and is_newer(outfileinfo, srcfileinfo)
				and is_newer(outfileinfo, exodus_include_dir_info)
				and is_newer(outfileinfo, exodus_libfile1_info)
				and is_newer(outfileinfo, exodus_libfile2_info)
				and is_newer(outfileinfo, exodus_libfile3_info)
				) {

				// Recompile is required if any include file is younger than the current output binary
				bool recompile_required = false;
				// TODO recode to find #include directly instead of by line since includes are generally only in the first part of a program
				for (var line : text) {

					// Skip lines unlike "#include <printplans7.h>"
					if (not line.contains("#include"))
						continue;

					let incfilename = line.field("<", 2).field(">", 1);

					// TODO also handle include filename in quotes
					if (incfilename)
						continue;

					// Acquire include file date/time
					var incfileinfo = incdir ^ OSSLASH ^ osfile(incfilename);

					// Also check system include dir
					if (not incfileinfo)
						incfileinfo = osfile("/usr/local/include/" ^ incfilename);

					// Force compilation if include file is newer
					if (incfileinfo) {
						if (is_newer(incfileinfo, outfileinfo)) {
							recompile_required = true;
							break;
						}
					}

				}

				// Only skip compilation if all headers are older than the current output file
				if (not cleaning and not recompile_required) {
					if (verbose)
						printl("Skipping compilation since the output file is newer than both the source code, its include files and libraries, and no (F)orce option provided.");
					//continue;
					return;
				}
			}

			// Warn about undefined behaviour not picked up by compilers
			////////////////////////////////////////////////////////////

			// xxx cannot be used in the same statement that it is being defined
			// var xxx = osread(xxx, yyy, zzz);
			if (warnings ge 0) {
				let matches = text.match("[\\n \\t]{2,}(var|let)\\s+(\\b[a-zA-Z0-9_]+\\b)[ \t]+[^\n:;]*?\\b\\2\\b");
				if (matches) {
					var nmatches = 0;
					for (var match : matches) {

						// Skip matches with an odd number of double quotes since it indicates quoted xxx on the right
						if (match.count(_DQ).mod(2))
							continue;

						// Skip if there is a function or member function with the same name as the variable
						// e.g. "var reccount = file1.reccount();"
						let varname = match.f(1, 1).field(" ", 2);
						if (match.contains("." ^ varname) or match.contains(varname ^ "("))
							continue;

						if (not nmatches++) {
							logputl(srcfilename ^ ":9999:99: warning: Use before constructed is undefined behaviour.");
							atomic_ncompilation_failures++;
						}
						logputl(match.f(1,1).trim(" \t\n").quote());
					}
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
				preprocess_only,
				linkoptions,
				installcmd
			);

		// Must catch all errors because crashing a thread will crash the whole program
		} catch (VarError& e) {
			errputl(e.message);
			errputl(var(e.stack()).convert(_FM, _EOL));
		}

		} // end of big lambda

		); // end of boost::asio::post function
#pragma GCC diagnostic pop

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

func static compile(

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
	in preprocess_only,
	in linkoptions,
	in installcmd) {

//	atomic_count++;

	let result = compile2(
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
	preprocess_only,
	linkoptions,
	installcmd);

//	atomic_count--;

	return result;

}

func static compile2(
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
	in preprocess_only,
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
	let oldobjfileinfo = osfile(objfilename);

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
	if (preprocess_only)
		compilecmd ^= " -E ";
	else {
		 if (outputoption)
			compilecmd ^= " " ^ outputoption ^ " " ^ objfilename;
		compilecmd ^= linkoptions;
	}

	if (preprocess_only) {

		// Remove lines starting # which indicate source file and line number
		if (preprocess_only > 1)
			compilecmd ^= " 2>&1 | grep -v ^# | uniq";

		// Expand single line macro code (all ; unfortunately) TODO do only macro code
		if (preprocess_only > 2) {
			compilecmd ^= " | sed 's/;/;\\n/g' | uniq";
				// indent properly if available
				if (osshellread("which indent"))
					compilecmd ^= " | indent";
		}

	}

	var outputpathandfile = outputdir ^ field(binfilename, OSSLASH, -1);

	// If cleaning, not compiling
	if (cleaning) {
		if (osfile(outputpathandfile)) {
			if (not osremove(outputpathandfile))
				loglasterror();
			else if (verbose)
				errputl(outputpathandfile ^ " cleaned.");
		}
		return "";
	}

	// Capture warnings and errors for post compilation processing
//	let exo_post_compile = osgetenv("EXO_POST_COMPILE");
	var capturing = exo_post_compile ? " 2>&1" : "";

	////////////////////
	////////////////////
	if (verbose)
		printl(compilecmd, capturing);
	var compileoutput;
	var compileok;
	var srcfileinfo = osfile(srcfilename);

	// Will return here if the source file is amended by the EXO_POST_COMPILE
	// Dont forget that we are probably doing many in parallel threads
compile_it:
	///////////////////////////////////////////////////////////////
	compileok = osshellread(compileoutput, compilecmd ^ capturing);
	///////////////////////////////////////////////////////////////
	if (not compileok and not capturing) {
		atomic_ncompilation_failures++;
	}

	if (preprocess_only) {
		compileoutput.outputl();
		return "";
	}

	// Handle compile output
	compileoutput.trimmerlast("\n\t\r ");
//	var startatlineno;
	{
		if (compileoutput) {

			// Only calling postcompile if error: or warning:
			if (exo_post_compile and compileoutput.match("error:|warning:")) {

				// e.g. tac|fixdeprecated {U}
				////////////////////////////////////////////////////////
				if (not osshellwrite(compileoutput on exo_post_compile))
					loglasterror();
				////////////////////////////////////////////////////////

				// Recompile if src was changed by exo_post_compile
				if (osfile(srcfilename) != srcfileinfo) {
					srcfileinfo = osfile(srcfilename);
					capturing = "";
					goto compile_it;
				}

			// Compile it again this time without capturing stderr so that syntax highlighting works.
				capturing = "";
				goto compile_it;
			}
//			compileoutput.outputl();
		}

	}

	// Get new objfile info or continue
	let newobjfileinfo = osfile(objfilename);
	if (not newobjfileinfo) {

		// linker errors might not give a compile error so bump count in that case
		if (compileok)
			atomic_ncompilation_failures++;

		// Allow user to see compile command in case of lack of desired output
		errputl(compilecmd);

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
					let cmd = "mkdir " ^ outputdir;
					if (verbose)
						printl(cmd);
					//osshell(cmd);
					if (!osmkdir(outputdir))
						cmd.errputl("ERROR: Failed to make directory");
				}

				// Check can install file
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
				let cmd = installcmd ^ " " ^ objfilename ^ " " ^ outputpathandfile;
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

func set_environment() {

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

	searchdirs.cutter(1);
	searchdirs.trimmer(FM);
	var searched = "";
	var batfilename = "";

	for (const var ii : range(1, 999999)) {

		var msvs = searchdirs.f(ii);
		if (not msvs)
			break;

		if (not msvs.ends(OSSLASH))
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

	let tempfilenamebase = osgetenv("TEMP") ^ "\\exoduscomp$" ^ rnd(99999999);

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

		dim vars = result.convert("\r\n", _FM _FM).split();
		for (in line : vars) {
			ossetenv(
				field(line, "=", 1), field(line, "=", 2, 999999));
		}
	}
	if (not osremove(tempfilenamebase ^ ".cmd")) {}
	if (not osremove(tempfilenamebase ^ ".$$$")) {}
	if (verbose) {
		var errtemp;
		if (osread(errtemp, tempfilenamebase ^ ".$2")) {
			//printl(errtemp);
		}
	}
	if (not osremove(tempfilenamebase ^ ".$2")) {}

	return true;
}

func getparam(in result, in paramname, out paramvalue) {
	let posn = index(result.ucase(), "\n" ^ paramname.ucase() ^ "=");
	if (not posn)
		return false;
	paramvalue = result.b(posn + len(paramname) + 2).field("\n", 1);
	return true;
}

func make_include_dir(in incdir) {
	if (not incdir.osdir()) {
		if (not incdir.osmkdir()) {
			errputl("Error: exodus compile could not create dir for header include files. Skipping creation of header file");
		} else if (verbose)
			printl("Created include directory " ^ incdir.squote() ^ " for header");
	}
	return true;
}

func is_newer(in new_file_info, in old_file_info) {

	int new_file_date = new_file_info.f(2);
	int old_file_date = old_file_info.f(2);

	if (new_file_date > old_file_date)
		return true;

	if (new_file_date < old_file_date)
		return false;

	return new_file_info.f(3) > old_file_info.f(3);

}

func oneline_compile() {

	let benchmark = OPTIONS.count("b");
	let nrepeats = OPTIONS.count("B");

//	var source = SENTENCE.fieldstore(" ", 1, -1, "");
	var source = COMMAND.remove(1).convert(FM, " ");

	if (not source)
		abort(
R"__(
Syntax is compile 'EXPRESSION' or 'COMMAND;... {i}'
e.g.
	compile 'oconv("date()", "D")' {i}                        // Simple expression. NO semicolon.
	compile 'write("f1^f2"_var on "definitions", "abc");' {i} // Raw source code.   MANDATORY semicolon(s).
)__"
		);

	// source ending with a ; is assumed to be one or more commands which dont return a value
	// and is compiled as is, otherwise wrap in TRACE() to display the output
	let source_is_expression = not source.ends(";");
	if (true or benchmark or nrepeats) {
		if (source_is_expression)
			source = "ANS = " ^ source ^ ";";
		source =
R"__(
		var nreps = pwr(10, <nrepeats>);
		for (var repn : range(1, nreps)) {
			var started = ostimestamp();
			int nops = pwr(10, <benchmark>);
			for (auto i = 1; i <= nops; ++i) {
				)__" ^ source ^ R"__(
			};
		var stopped = ostimestamp();
		errputl(repn, ". ", oconv(nops, "MD00,"), " ops in ", elapsedtimetext(started, stopped), ". ", elapsedtimetext(0, (stopped - started) / nops), "/op", " ", ANS.squote() );
	}
)__";
		source.replacer("<nrepeats>", nrepeats);
		source.replacer("<benchmark>", benchmark);
	}

	else if (source_is_expression)
		source = "TRACE(" ^ source ^ ")";

	// Generate exodus program source code
	let prog =
		"#include <exodus/program.h>\n"
		"programinit()\n"
		"\n"
		"func main() {\n" ^
			source ^ "\n" ^
			"\n"
			"\treturn 0;\n"
		"}\n"
		"programexit()\n";

	// Create a tmp cpp file
	let tempfilebase = ostempfile();
//	let tempfilebase = ostempdir() ^ "~eeval";
	let tempfilesrc	 = tempfilebase ^ ".cpp";
	if (not oswrite(prog, tempfilesrc))
		abort(lasterror());

	// Compile it
	if (not osshell("CXX_OPTIONS='-Wno-unused-result' compile " ^ tempfilesrc ^ " {SS}"))
		abort(lasterror());

	// Run it
	if (not osshell(tempfilebase.field("/", -1)))
		loglasterror();

	// Start a new line in case output left the cursor hanging mid line.
	if (not source_is_expression)
		printl();

	// Cleanup {L} removes ~/bin and ~/inc files
	if (not osshell("compile " ^ tempfilesrc ^ " {LSS}"))
		loglasterror();

	return 0;
}

programexit()
