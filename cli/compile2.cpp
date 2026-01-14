#include <exodus/library.h>
libraryinit()

// Global options
let verbose  = OPTIONS.count("V");
let cleaning = OPTIONS.contains("L");
let force    = OPTIONS.contains("F") or OPTIONS.contains("P");
let silent   = OPTIONS.count("S");
let generateheadersonly = OPTIONS.contains("h");
let color_option        = OPTIONS.count("C") - OPTIONS.count("c");
let warnings            = OPTIONS.count("W") - OPTIONS.count("w");
let warnings_are_errors = OPTIONS.count("E") - OPTIONS.count("e");
let preprocess_only     = OPTIONS.count("P");

//var nasterisks = 0;
//var nfailures = 0;
var nwarnings = 0;

function main() {

	var srcfilename = COMMAND.f(2);

	auto [
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
		latest_exo_info,
		exo_post_compile
		, sentinel
	] = COMMAND.f(3).iconv("HEX").unpack<21>();
	latest_exo_info.raiser();
//	TRACE(srcfilename)
//	TRACE(basicoptions)
//	TRACE(bindir)
//	TRACE(binfileextension)
//	TRACE(binoptions)
//	TRACE(compiler)
	if (sentinel ne "sentinel")
		logputl("compile2: Invalid sentinel.");

	// Ticker
	if (verbose) {
		printl("thread: sourcefilename=", srcfilename);
	} else if (not silent) {
		printl(srcfilename);
		//printx(srcfilename ^ _EOL);
	} else if (silent eq 1) {
//		nasterisks++;
		printx("*");
		osflush();
	}

	// Get file text
	////////////////
	let fileext = srcfilename.field(".", -1).lcase();
	let filepath_without_ext = srcfilename.cut(-len(fileext) - 1);
	let filename_without_ext = filepath_without_ext.field(OSSLASH, -1);

	var srcfileinfo = osfile(srcfilename);
	if (!srcfileinfo)
		abort(srcfilename.quote() ^ "srcfile doesnt exist: ");

	// Try reading the source text is various locales starting with "" no/default locale
	var text;
	let alllocales = _FM "utf8" _FM "en_US.iso88591" _FM "en_GB.iso88591";
	var locales = "";
	var locale;
	let origlocale = getxlocale();
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
	if (not text)
		abort("Cant read/convert srcfile: " ^ srcfilename ^ ". Encoding issue? unusual characters? - tried " ^ locales ^ ". Use 'dpkg-reconfigure locale' to get more");

	{ // hotfixing src
		let orig_text = text;
//				text.replacer(R"__(^function )__"_rex, "func ");
//				text.replacer(R"__(^subroutine )__"_rex, "subr ");
//				text.replacer("ostempfile", "ostempfile");
//				text.replacer("ostempdir", "ostempdir");
//				text.replacer(R"__(space\(\d\))__", "space($1)");
		text.replacer(R"__(\bvar\((\d+)\).space\(\))__"_rex, "space($1)");


		if (srcfilename ne "compile2.cpp") {

			text.replacer("Exo" "Logoff", "ExoStop");

			// TODO: Leave it to fixdeprecated which will fix the source dat files too.
			// ST, _ST and ST_ become STM, _STM and STM_
			text.replacer(R"__(\b(_?)ST(_?)\b)__"_rex, "$1STM$2");
		}

		// Change obsolete programinit and libraryinit if exactly at end of file
		if (text.ends("\nprogramexit()\n") or text.ends("\nlibraryexit()\n"))
			text.paster(-14, 0, "}; /" "/ ");

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
								funcargsdecl.firster(static_cast<std::size_t>(charn - 1));
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
								// duplicate code above and below
								if (argtype == "in" || argtype == "out" || argtype == "io") {
									argname2 = argname ^ "_" ^ argtype;
									//declare it
									func_body ^= " var " ^ argname2;
								} else {
									// use unknown arg type as is/without ref
									argname2 = argname;
									//declare it
									func_body ^= " " ^ argtype.convert("&", "") ^ " " ^ argname2;
								}

								//default it. DOS doesnt have a "default" for missing args other than "var()" ie unassigned
								if (argdefault ne "" and argdefault ne "var()")
									func_body ^= " = " ^ argdefault;
								func_body ^= ";" _EOL;
							}
						} else {
							//build a new non-constant dummy variable name to be used as unassigned argument to the real function
							// duplicate code above and below
							if (argtype == "in" || argtype == "out" || argtype == "io") {
								argname2 = argname ^ "_" ^ argtype;
								//declare it
								func_body ^= " var " ^ argname2;
							} else {
								// use unknown arg type as is/without ref
								argname2 = argname;
								//declare it
								func_body ^= " " ^ argtype.convert("&", "") ^ " " ^ argname2;
							}
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

				//new method using member functions to call external functions with ev environment
				//using a callable class that allows library name changing
				//public inheritance only so we can directly access ev in exoprogram.cpp for oconv/iconv. should perhaps be private inheritance and ev set using .init(ev)
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
					_EOL "/" "/ 3. The current program's ev environment to share with it."
					_EOL "Callable_funcx(ExoEnv& ev) : Callable(ev) {}"
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
					_EOL " /" "/ passing current standard variables in ev"
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
					_EOL "/" "/ an Exodus program/function initialized with the current ev environment."
					_EOL "Callable_funcx funcx{ev};";

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
//		call make_include_dir(incdir);

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

		else if (filename_without_ext.starts("~")) {
			if (verbose)
				logputl("temp skipped.");
		}
		else if (oldheadertext ne newheadertext) {

			//over/write if changed
			if (not oswrite(newheadertext, headerfilename, locale))
				loglasterror();

			//verify written ok
			var chkheadertext;
			if (not osread(chkheadertext, headerfilename, locale))
				loglasterror();

			if (chkheadertext ne newheadertext) {
				nwarnings++;

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
		return "OK Header only";

	// Skip compilation if the output file is newer than source file and all include files
	//////////////////////////////////////////////////////////////////////////////////////

	let outfileinfo = osfile(outputdir ^ field(binfilename, OSSLASH, -1));
	if (outfileinfo
		and not(force)
		and not(generateheadersonly)
		and is_newer(outfileinfo, srcfileinfo)
		and is_newer(outfileinfo, latest_exo_info)
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
			return "OK Already compiled. Up to date.";
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
					nwarnings++;
					logputl(srcfilename ^ ":9999:99: warning: Use before constructed is undefined behaviour.");
				}
				logputl(match.f(1,1).trim(" \t\n").quote());
			}
		}
	}

//  /////////////////
//	// Actual compile
//	/////////////////

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
		else if (not updateinusebinposix) {
			abort(quote(objfilename) ^ " Error: objectfile cannot be updated. (1) Insufficient rights on ");
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

	// Preprocessor
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
		return "OK Cleaned.";
	}

	// Capture warnings and errors for post compilation processing
	var capturing = exo_post_compile ? " 2>&1" : "";

	if (verbose)
		printl(compilecmd, capturing);
	var compileoutput;
	var compileok;
//	var srcfileinfo = osfile(srcfilename);

	// Will return here if the source file is amended by the EXO_POST_COMPILE
	// Dont forget that we are probably doing many in parallel threads
compile_it:
	///////////////////////////////////////////////////////////////
	compileok = osshellread(compileoutput, compilecmd ^ capturing);
	///////////////////////////////////////////////////////////////
	if (not compileok and not capturing) {
		nwarnings++;
	}

	if (preprocess_only) {
		compileoutput.outputl();
		return "OK Preprocessed.";
	}

	// Handle compile output
	compileoutput.trimmerlast("\n\t\r ");
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
		}

	} // handle compile output

	// Get new objfile info or abort
	let newobjfileinfo = osfile(objfilename);
	if (not newobjfileinfo) {

		// linker errors might not give a compile error so bump count in that case
		//if (compileok)
		//	nfailures++;

		// Allow user to see compile command in case of lack of desired output
		errputl(compilecmd);
		abort(quote(objfilename) ^ " Error: Cannot output file ");
	}

	// Install new objfile
	if (newobjfileinfo ne oldobjfileinfo) {

		if (newobjfileinfo) {

			// Copy the obj file to the output directory
			////////////////////////////////////////////
			if (installcmd) {

				// Make the target directory
				if (not osdir(outputdir)) {
					if (verbose)
						printl("mkdir", outputdir);
					//osshell(cmd);
					if (!osmkdir(outputdir) && not osdir(outputdir))
						outputdir.errputl("ERROR: Failed to make directory ");
				}

				// Check can install file
				if (osfile(outputpathandfile)) {
					if (osopen(outputpathandfile, outputpathandfile)) {
						osclose(outputpathandfile);

					} else if (not updateinusebinposix) {
						abort(quote(outputpathandfile) ^ " Error: Cannot update In use or insufficient rights (2).");

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

				// If both exist after installing then rm the target
				if (osfile(outputpathandfile) and osfile(objfilename)) {
					if (not osremove(outputpathandfile)) {
						outputpathandfile.errput("compile: Could not remove output path and file ");
					}
				}
			}
		}
	}  // new objfile

////////
// exit:
////////

	return "OK";
}

func getparam(in result, in paramname, out paramvalue) {
	let posn = index(result.ucase(), "\n" ^ paramname.ucase() ^ "=");
	if (not posn)
		return false;
	paramvalue = result.b(posn + len(paramname) + 2).field("\n", 1);
	return true;
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
