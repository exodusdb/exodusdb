//// EXO_FORMAT required to append -lfmt to compile and link command
//#pragma GCC diagnostic push
//#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
//#include <exodus/format.h>
#if __has_include(<fmt/args.h>)
//# warning Using fmt library instead std::format
#	undef EXO_FORMAT
#	define EXO_FORMAT 2
#endif
//#pragma GCC diagnostic pop

#include <exodus/program.h>
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

	int nfailures = 0;

	//.def file is one way on msvc to force library function names to be "undecorated"
	// and therefore usable in on demand library loading
	//Declaring functions with extern "C" is the other way and seems less complicated
	//but msvc warns that functions defined with extern c cannot return var
	//nevertheless, returning vars seems to work in practice and .def files seem
	//more complicated so for now use extern "C" and not .def files
	//#define EXODUS_EXPORT_USING_DEF

	let verbose = OPTIONS.count("V");

func main() {

	// Get more options from environment and possibly the second word of the command
	var options = OPTIONS ^ osgetenv("EXO_COMPILE_OPTIONS");

	let exo_module =
#if EXO_MODULE
		EXO_MODULE;
#else
		false;
#endif

	// Global options
	let cleaning = options.contains("L");
	let force = options.contains("F") or options.contains("P");
	var exo_post_compile = "";

	// Options
	let silent = options.count("S");
	let debugging = not options.contains("R");  //no symbols for backtrace
	// Add one optimisation level if EXO_MODULE
	// Default optimisation level from Oct 2023 is 1. Can be reduced to 0 by option o.
	// Also compile will recognise "export CXX_options--O0" in shell
	let optimise = 1 + options.count("O") - options.count("o") + (!!exo_module);
	let generateheadersonly = options.contains("h");
	let color_option = options.count("C") - options.count("c");
	let warnings = options.count("W") - options.count("w");
	let warnings_are_errors = options.count("E") - options.count("e");
	let preprocess_only = options.count("P");

	// "i" - inline source code
	// "`~!@#$%^&*()_+-=[]\{}|;':",./<>?"
	//	var ascii = "!\"#$%&'()*+,-./:;<=>?@[\]^_`{|}~";

	// Option "i" - inline/immediate compilation
	// or if "filename" contains non-alphanumeric chars (except . / - _ ~)
	if (OPTIONS.contains("i") or not COMMAND.f(2).replace("[[:alnum:]]"_rex, "").convert("./-_~", "").empty()) {
//    if (OPTIONS.contains("i") or not COMMAND.f(2).match("[^[:alnum:]]").match("[^./-_~]").empty())
		return gosub oneline_compile(force);
	}

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

	// TODO Dont hard code for typical Ubuntu
	var latest_exo_info = "";
//	for (auto dirpattern : "/usr/local/lib]libexo*^/usr/local/include]exodus"_var) {
//		let dir = dirpattern.f(1, 1);
//		let pat = dirpattern.f(1, 2);
//		let osfiles = oslist(dir, pat);
//		for (var file : osfiles) {
//			let info = osinfo(dir ^ "/" ^ file);
//			if (is_newer(info, latest_exo_info))
//				latest_exo_info = info;
//		}
//	}
	for (auto dir : "/usr/local/lib^usr/local/bin^/usr/local/include/exodus"_var) {
		let info = osinfo(dir);
		if (is_newer(info, latest_exo_info))
			latest_exo_info = info;
	}
//	var exodus_include_dir_info = osdir("/usr/local/include/exodus");
//	if (is_newer(exodus_include_dir_info, latest_exo_info))
//		latest_exo_info = exodus_include_dir_info;

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
//#if defined(EXO_FORMAT) && EXO_FORMAT == 2
#if defined(EXO_FORMAT)
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

//		basicoptions ^= " -fno-omit-frame-pointer ";
		// Minor space savings
//		basicoptions ^= " -ffunction-sections -fdata-sections ";

		// Help fixdeprecated get all errors and deprecations
		if (clang)
			basicoptions ^= " -ferror-limit=9999";
		else if (gcc)
			basicoptions ^= " -fmax-errors=9999";

		// Use c++ (g++/clang) -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
#if __GNUC__ >= 4
		basicoptions ^= " -fvisibility=hidden ";

		// Required to allow exodebug to get absolute addresses instead of function name + offset
		// bad: "/root/lib/libnlist.so(_ZN11_ExoProgram4mainEv+0x403) [0x7aa6aa9c3ec3]"
		basicoptions ^= " -fvisibility-inlines-hidden ";
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

			// Never warn about macro expansion since it is essential for ev.RECORD etc.
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

	} else {


		//////////////////////
		// NOT POSIX - WINDOWS
		//////////////////////

		if (verbose)
			printl("Windows environment detected. Finding C++ compiler.");

		//this calls sdk setenv or vcvars
//		call set_environment();

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
			//using MDd causes weird inability to access ev.DICT and other variables in main() despite them being initialised in exodus_main()
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
					nfailures++;
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
						nfailures++;
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

    setmaxthreads();

	for (var srcfilename : srcfilenames) {

		if (srcfilename.empty())
			continue;

		// Pass over a lot of repetitive info as an FM delimited string converted to HEX
		var pack1 = {
		    basicoptions,
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
		    updateinusebinposix,
		    exodus_callable_maxnargs,
			latest_exo_info.lower(),
			exo_post_compile
			, "sentinel"_var
		};

		// Start an async parallel job and comtinue
		run("compile2 " ^ srcfilename ^ " " ^ pack1.oconv("HEX") ^ " {" ^ OPTIONS ^ "}");
	}

//	// Process results (Event driven. No polling. As they come in)
//	for (var i : range(1, run_count())) {
//		ExoEnv env;
//		result_queue_->wait_and_pop(env);
//		if (not env.ANS.starts("OK")) {
//			nfailures++;
//		}
//	}

	// Process results (Event driven. No polling. As they come in)
	for (auto& env : run_results()) {
		if (not env.ANS.starts("OK"))
			nfailures++;
	}

	// Asterisks dont generate new lines per compiled file
	if (silent eq 1 and run_count())
		printl();

	// Not needed here. Useful to reset run_count();
	//reset_run(4);

	return nfailures;

} // main()

func make_include_dir(in incdir) {
	if (not incdir.osdir()) {
		if (not incdir.osmkdir()) {
			errputl("Error: exodus compile could not create dir for header include files. Skipping creation of header file");
		} else if (verbose)
			printl("Created include directory " ^ incdir.squote() ^ " for header");
	}
	return true;
}

func oneline_compile(in force) {

	// log means how many zeros after 1
	// So 0, 1, 2, 3 -> 1, 10, 100, 1000
	let log_nreps = OPTIONS.count("B");
	let log_nops  = OPTIONS.count("b");

	// Work out the actual nreps/nops
	let act_nreps = pwr(10, log_nreps);
	let act_nops  = pwr(10, log_nops);

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
	if (source_is_expression)
		source = "ANS = " ^ source ^ ";";

	let raw_source =
////////////////////////////// raw source code ////////////////////////////////////
R"__(
	var nreps = COMMAND.f(2);
	var rep_optimes = "";
	var opunit = "";
	int nops = COMMAND.f(3);
	let col0 = TERMINAL ? AT(-40) : "";
	for (var repn : range(1, nreps)) {
		if (esctoexit())
			break;
		var i; // Using var to avoid optimisation away of int
		var framework_secs = 0;
		{
			if (<source_is_expression>) {
				var started = ostime();
				for (i = 1; i <= nops; ++i) {
					ANS = "";
				}
				framework_secs = i;
				var stopped = ostime();
				framework_secs = stopped - started;
				//TRACE(ANS)
				//TRACE(started)
				//TRACE(stopped)
				//TRACE(framework_secs)
			} else {
				var started = framework_secs + ostime();
				for (i = 1; i <= nops; ++i) {
					// optimiser will respect this loop if i is a var)
				}
				framework_secs = i;
				var stopped = ostime();
				framework_secs = stopped - started;
				//TRACE(framework_secs)
			}
		}
		{
			var started = ostime();
			for (i = 1; i <= nops; ++i) {

				<test_source>

			}
			var stopped = ostime();
			var elapsed_secs = stopped - started;
			//TRACE(elapsed_secs)
			if (framework_secs < elapsed_secs)
				elapsed_secs -= framework_secs;
			//TRACE(elapsed_secs)
			var elapsed_days = elapsed_secs / 86400;
			var optime_text = elapsedtimetext(0, elapsed_days / nops);
			output(col0);
//			errput(repn, "/", nreps, ". ", oconv(nops, "MD00,"), " ops in ", elapsedtimetext(0, elapsed_days), ". ",
//				optime_text, "/op", " ", ANS.squote() );

			// Accumulate optimes for stats
			var rep_opunit = field(optime_text, " ", 2);
			if (opunit eq "")
				// First opunit
				opunit = rep_opunit;
			if (rep_opunit eq opunit) {
				// Only add optimes if they match the first opunit
				rep_optimes ^= field(optime_text, " ", 1) ^ _FM;
			}
			var stats = rep_optimes.pop().stddev().oconv("MD30");
//			logputl();
			if (nreps > 1)
				errput("Rep:" , repn.oconv("MD0,"), "/", oconv(nreps, "MD0,"), ". ", oconv(nops, "MD0,"), " ops/rep  min:", stats.f(3), "  max:", stats.f(4), "  avg:", stats.f(5), " ± ", stats.f(6), " ", opunit, "/op");
			else if (nops > 1)
				errput(oconv(nops, "MD0,"), " ops. ", stats.f(5), " ", opunit, "/op");
			else
				errput(stats.f(5), " ", opunit);
			if (repn == nreps or ANS.len() <= 32)
				errput("  ", ANS.squote());
			osflush();
		}
	} // repn

	logputl();
)__";
/////////////////////// raw source ends ////////////////////////////

	source = raw_source.replace("<test_source>", source);

	source.replacer("<act_nreps>", act_nreps);
	source.replacer("<act_nops>", act_nops);
	source.replacer("<source_is_expression>", source_is_expression);

	// Generate full exodus program source code
	let prog =
		"#include <exodus/program.h>\n"
		"programinit()\n"
		"\n"
		"func main() {" ^
			source ^ "\n" ^
			"\treturn 0;\n"
		"}\n"
		"}; /" "/ programexit()\n";

	// Create a tmp cpp file
//	let tempfilebase = ostempfile();
	let tempfilebase = ostempdir() ^ "~exo" ^ prog.hash(0xFF'FFFF).oconv("MX");

//	let tempfilebase = ostempdir() ^ "~eeval";
	let tempfilesrc	 = tempfilebase ^ ".cpp";
	if (prog ne osread(tempfilesrc) and not oswrite(prog on tempfilesrc))
		abort(lasterror());

	// Compile it
	let options = " {SS" ^ str("V", verbose)  ^ str("F", force) ^ "}";
	if (not osshell("CXX_OPTIONS='-Wno-unused-result' compile " ^ tempfilesrc ^ options))
		abort(lasterror());

	// Run it
	if (not osshell(tempfilebase.field("/", -1) ^ " " ^ act_nreps ^ " " ^ act_nops))
		{} // loglasterror();

	// Start a new line in case output left the cursor hanging mid line.
	if (not source_is_expression)
		printl();

	// Cleanup {L} removes ~/bin and ~/inc files
	//if (not osshell("compile " ^ tempfilesrc ^ " {LSS}"))
	//	loglasterror();

	return 0;
}

// Replicated in compile and compile2
func is_newer(in new_file_info, in old_file_info) {

	int new_file_date = new_file_info.f(2);
	int old_file_date = old_file_info.f(2);

	if (new_file_date > old_file_date)
		return true;

	if (new_file_date < old_file_date)
		return false;

	return new_file_info.f(3) > old_file_info.f(3);

}

}; // programexit()
