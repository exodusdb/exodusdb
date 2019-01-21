#include <exodus/program.h>

programinit()

//.def file is one way on msvc to force library function names to be "undecorated"
// and therefore usable in on demand library loading
//Declaring functions with extern "C" is the other way and seems less complicated
//but msvc warns that functions defined with extern c cannot return var
//nevertheless, returning vars seems to work in practice and .def files seem
//more complicated so for now use extern "C" and not .def files
#define EXODUS_EXPORT_USING_DEF 0

#define EXODUS_FUNCTOR_MAXNARGS 20

//#include <exodus/exodus.h>

//#include <exodus/xfunctorf0.h>
//ExodusFunctorF0<int> xyz;

var verbose;
var silent;
var debugging;
var optimise;
var posix;
var windows;

function main()
{

	var command=COMMAND;

	//we use on demand/jit linking using dlopen/dlsym
	var loadtimelinking=false;

	var usedeffile=false;

	//extract options
	verbose=index(OPTIONS.ucase(),"V");
	silent=index(OPTIONS.ucase(),"S");
	debugging=not index(OPTIONS.ucase(),"R");//no symbols for backtrace
	//the backtrace seems to work fine with release mode at least in vs2005
	optimise=index(OPTIONS.ucase(),"O");//prevents backtrace

	//extract filenames
	var filenames=field(command,FM,2,999999999);
	var nfiles=dcount(filenames,FM);
	if (not filenames)
		abort("Syntax is compile filename ... {options}\nOptions are R=Release O=Optimise V=Verbose");

	//source extensions
	var src_extensions="cpp cxx cc";
	var inc_extensions="h hpp hxx";
	var noncompilable_extensions=inc_extensions^ " out so o";
	var default_extension="cpp";

	var compiler;
	var basicoptions="";
	var linkoptions=false;
	var manifest;
	var binoptions;
	var liboptions;
	var bindir;
	var libdir;
	var installcmd;
	var outputoption="";
	var objfileextension;
	var binfileextension;
	var libfileextension;
	var libfileprefix="";

	//you can allow updating inuse binaries in posix but with some messages
	//true may result in messages similar to the following from exodus compiler
	//Error: /home/neo/bin/server cannot be updated. In use or insufficient rights (2).
  //false may result in messages similar to the following from running exodus programs
  //BFD: reopening /home/neo/exodus/service/service2/sys/server: No such file or directory
  var updateinusebinposix=true;

	//hard coded compiler options at the moment
	//assume msvc (cl) on windows and g++ otherwise
	posix=SLASH eq "/";
	windows=not(posix);
	if (posix) {
		if (verbose)
			printl("Posix environment detected.");

		if (compiler=osgetenv("EXO_CXX")) {
			if (verbose)
				printl("Using EXO_CXX environment variable for compiler "^compiler.quote());
		}
		else if (compiler=osgetenv("CXX")) {
			if (verbose)
				printl("Using CXX environment variable for compiler "^compiler.quote());
		} else {
			compiler="g++";
#ifdef __APPLE__
			if (PLATFORM_ eq "x86")
				compiler="g++-4.0";
#endif
			if (verbose)
				printl("no EXO_CXX or CXX environment variable. Assuming "^compiler.quote());
		}
		//basic compiler options

		//c++11/17
		//basicoptions^=" -std=gnu++0x";//pre gcc 4.7 enables gnu extentions.
		//basicoptions^=" -std=gnu++11";//gcc 4.7 and later  enables gnu extensions.
		//__cplusplus is 201103L
		//if( __cplusplus >= 201103L )
			//basicoptions^=" -std=c++11";//gcc 4.7 and later DISABLES gnu extensions
		basicoptions^=" -std=c++17";//gcc 4.7 and later DISABLES gnu extensions

		//http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#Warning-Options
		basicoptions^=" -Wall";
		basicoptions^=" -Wextra";

		//not available on gcc 4.1.2 TODO work out gcc version
		//basicoptions^=" -Wno-unused-parameters"; //dont want if functions dont use their parameters
				//basicoptions^=" -pendantic -ansi";
		//following will result in 2 byte wchar in linux
		//but all exodus libs also need to be in this format too
		//leave as 4 byte for now to avoid forcing additional compilation option on exodus users
		//who use g++ directly (or put runtime check that program+lib have the same size wchar
		//advising the right compilation option.
		//basicoptions^=" -fshort-wchar";

		//exodus library
		//if (debugging)
		//	linkoptions=" -lexodus-gd";
		//else
			linkoptions=" -lexodus -lstdc++fs";

		//enable function names in backtrace
		if (debugging)
			basicoptions^=" -g -rdynamic";

		//optimiser unfortunately prevents backtrace
		if (optimise)
			basicoptions^=" -O1";

		//how to output to a named file
		outputoption=" -o ";

		//general options
		binoptions="";
		//binoptions=" -g -L./ -lfunc1 -Wl,-rpath,./";
		//binoptions=" -fPIC";

		//how to make a shared library
		liboptions=" -fPIC -shared";
		//soname?
#if __GNUC__ >= 4
		//use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
		liboptions^=" -fvisibility=hidden -fvisibility-inlines-hidden";
#endif

		//target directories
		bindir="~/bin";
		libdir="~/lib";

		//target extensions
		//make it easier for debuggers
		//objfileextension=".out";
		objfileextension="";
		binfileextension="";
		libfileextension=".so";
		libfileprefix="lib";

		installcmd="mv";


	//not posix
	} else {
		if (verbose)
			printl("Windows environment detected. Finding C++ compiler.");

		//this calls sdk setenv or vcvars
		set_environment();

		if (verbose)
			printl("Searching for Exodus for include and lib directories");

		//get current environment
		var path=osgetenv("PATH");
		var include=osgetenv("INCLUDE");
		var lib=osgetenv("LIB");

		//locate EXO_PATH by executable path, environment variable or current disk or C:
		//EXO_PATH is the parent directory of bin and include etc.
		//compile needs to locate the include and lib directories

		var ndeep=dcount(EXECPATH,SLASH);
		var exoduspath="";
		//first guess is the parent directory of the executing command
		//on the grounds that compile.exe is in EXO_PATH/bin
		if (ndeep>2)
			exoduspath=EXECPATH.field(SLASH,1,ndeep-2);
		var searched="";

		//check if EXO_PATH\bin\exodus.dll exists
		if (not exoduspath or not (osfile(exoduspath^SLASH^"bin\\exodus.dll") or osfile(exoduspath^SLASH^"exodus.dll"))) {
			if (exoduspath)
				searched^="\n"^exoduspath;
			exoduspath=osgetenv("EXO_PATH");
			if (exoduspath)
				searched^="\nEXO_PATH is "^exoduspath;
			else {
				searched^="\nEXO_PATH environment variable is not set";
				exoduspath=L"\\Program Files\\exodus\\" EXODUS_RELEASE L"\\";
				if (not osdir(exoduspath)) {
					searched^="\n" ^ exoduspath;
					exoduspath.splicer(1,0,"C:");
				}
			}
		}
		if (exoduspath.substr(-1,1) ne SLASH)
			exoduspath^=SLASH;
		var exodusbin=exoduspath;
		if (not osfile(exodusbin^"exodus.dll")) {
			searched^="\n" ^ exoduspath;
			exodusbin=exoduspath^"bin\\";
			if (not osfile(exodusbin^"exodus.dll")) {
				//exodusbin=EXECPATH;
				searched^="\n" ^ exoduspath;
				printl("Searching for Exodus",searched);
				abort("Cannot find Exodus. Set environment variable EXO_PATH to exodus directory and restart Exodus");
			}
		}

		/* now done by vcvarsall above
		var msvc=msvs^"..\\..\\VC\\";

		//add path to MSVC binaries
		path.splicer(0,0,";"^msvs^"..\\IDE;");
		path.splicer(0,0,";"^msvc^"bin;");

		//add msvc include
		include.splicer(0,0,";"^msvc^"include;");

		//add msvc lib
		lib.splicer(0,0,";"^msvc^"lib;");

		//express doesnt have win32
		if (not osfile(msvc^"lib\\kernel32.dll"))
		{
			//var libpath="\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\Lib"
		}
		*/

		//add exodus path, include and lib
		path.splicer(0,0,exodusbin^";");
		include.splicer(0,0,exoduspath^"include;");
		include.splicer(0,0,exoduspath^"include\\exodus;");
		lib.splicer(0,0,exoduspath^"lib;");

		//set the new environment
		if (not ossetenv("PATH",path))
			errputl("Failed to set PATH environment variable");
		if (not ossetenv("INCLUDE",include))
			errputl("Failed to set INCLUDE  environment variable");
		if (not ossetenv("LIB",lib))
			errputl("Failed to set BIN environment variable");
		if (verbose) {
			path.errputl("\nPATH=");
			include.errputl("\nINCLUDE=");
			lib.errputl("\nLIB=");
		}
		compiler="cl";
		basicoptions="";
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
		basicoptions^=" /EHsc";

		//"Enables all warnings, including warnings that are disabled by default."
		basicoptions^=" /W3";
		//basicoptions^=" /Wall";

				//Enables one-line diagnostics for error and warning messages when compiling C++ source code from the command line.
		basicoptions^=" /WL";

		//Generates complete debugging information.
		//needed for backtrace
		basicoptions^=" /Zi";

		//Specifies a C++ source file.
		basicoptions^=" /TP";

		//define compiling for console application
		basicoptions^=" /D \"_CONSOLE\"";
		//to capture output after macro expansion
		//basicoptions^=" /E";

		//define common windows indicator (applies to WIN64 too!)
		basicoptions^=" /D \"WIN32\"";

		//define common windows indicator
		if (PLATFORM_ eq "x64")
			basicoptions^=" /D \"WIN64\"";

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
			basicoptions^=" /MDd";
			//basicoptions^=" /MD";

			//Disables optimization.
			basicoptions^=" /Od";

			//renames program database?? remove?
			basicoptions^=" /FD";

			//macro used in exodus in some places to provide additional information eg mvdbpostgres
			basicoptions^=" /D \"DEBUG\"";

			//Enables run-time error checking.
			basicoptions^=" /RTC1";

		} else {
			//Creates a multithreaded DLL using MSVCRT.lib.
			//basicoptions^=" /MD";
			//needed for backtrace
			//Creates a debug multithreaded DLL using MSVCRTD.lib.
			basicoptions^=" /MD";

			//Creates fast code.
			//basicoptions^=" /O2";

			//Enable string pooling
			basicoptions^=" /GF";

			//Enables whole program optimization.
			//basicoptions^=" /GL";

			//turn off asserts
			basicoptions^=" /D \"NDEBUG\"";
		}

		linkoptions=" /link exodus.lib";

		//http://msdn.microsoft.com/en-us/library/f2c0w594%28v=VS.80%29.aspx
		manifest=true;
		if (not manifest)
			linkoptions^=" /MANIFEST:NO";

		if (not verbose)
			linkoptions^=" /nologo";

		outputoption="";

		//enable function names in backtrace
		//basicoptions^=" -rdynamic";

		//make an exe
		binoptions="";

		//make a dll
		liboptions="/LD /DLL /D \"DLL\"";
		//soname?

		//target extensions
		objfileextension=".exe";
		binfileextension=".exe";
		libfileextension=".dll";

		//target directories

		var homedir=osgetenv("USERPROFILE");
		if (homedir) {
			bindir=homedir^"\\Exodus\\bin";
			libdir=bindir;
			//installcmd="copy /y";
			installcmd="copy";
		} else {
			bindir="";
			libdir="";
			installcmd="";
		}


	}
	swapper(bindir,"~",osgetenv("HOME"));
	swapper(libdir,"~",osgetenv("HOME"));

	if (bindir[-1] ne SLASH)
		bindir ^= SLASH;
	if (libdir[-1]!=SLASH)
		libdir ^= SLASH;

	var ncompilationfailures=0;
	for (var fileno=1; fileno<=nfiles; ++fileno) {

		var text="";
		var filebase;

		//get the next file name
		var srcfilename=filenames.a(fileno).unquote();
		if (not srcfilename)
			continue;

		//printl("--- ",srcfilename, " ---");

		//check/add the default file extension
		srcfilename.trimmerb(".");
		var fileext=srcfilename.field2(".",-1).lcase();
		if (src_extensions.locateusing(fileext," ")) {
			filebase=srcfilename.splice(-len(fileext)-1,"");

			//skip non-compilable files
		} else if (noncompilable_extensions.locateusing(fileext," ")) {
			//errputln(srcfilename^" "^fileext^" is not compilable");
			continue;

			//skip backup files
		} else if (fileext.substr(-1,1)=="~") {
			continue;

			//pickup default file if it exists - even if base file also exists
		} else if (osfile(srcfilename^"."^default_extension)) {
			filebase=srcfilename;
			srcfilename^="."^default_extension;

			//silently skip files that exist with the wrong header
		} else if (osfile(srcfilename) or osdir(srcfilename)) {
			continue;
			//add the default extension
		} else {
			filebase=srcfilename;
			srcfilename^="."^default_extension;

		}
		//get file text
		if (verbose)
			print("sourcefilename=");
		if (not silent)
			printl(srcfilename);
		if (not text and not text.osread(srcfilename, "utf8")) {
			if (osfile(srcfilename)) {
				srcfilename.errputl("Cant read/convert srcfile:");
				errputl(" Encoding issue? non-utf8/ascii characters?) in ");
			} else
				srcfilename.errputl("srcfile doesnt exist: ");
			continue;
		}

		//determine if program or subroutine/function
		var isprogram=
			index(text,"<exodus/program.h>")
			or index(text,"int main(")
			or index(text, "program()");
		if (verbose)
			printl("Type=",isprogram?"Program":"Subroutine");
		var outputdir;
		var compileoptions;
		var binfilename=filebase;
		var objfilename=filebase;
		if (isprogram) {
			binfilename^=binfileextension;
			objfilename^=objfileextension;
			outputdir=bindir;
			compileoptions=binoptions;
		} else {
			//binfilename^=binfileextension;
			binfilename^=libfileextension;
			objfilename^=libfileextension;
			if (libfileprefix) {
				//binfilename=libfileprefix^binfilename;
				//objfilename=libfileprefix^objfilename;
				var nfields=binfilename.dcount(SLASH);
				binfilename=fieldstore(binfilename,SLASH,nfields,1,libfileprefix^field(binfilename,SLASH,nfields));
				nfields=objfilename.dcount(SLASH);
				objfilename=fieldstore(objfilename,SLASH,nfields,1,libfileprefix^field(objfilename,SLASH,nfields));

			}
			outputdir=libdir;
			compileoptions=liboptions;
		}

		//and, for subroutines and functions, create header file (even if compilation not successful)
		var crlf="\r\n";
		var headertext="";
		converter(text,crlf,FM^FM);
		dim text2;
		var nlines=matparse(text,text2);

#if EXODUS_EXPORT_USING_DEF
		var deftext="";
		var defordinal=0;
#endif
		//detect libraryinit
		var useclassmemberfunctions=false;

		for (int ln=1; ln<=nlines; ++ln) {
			var line=trimf(text2(ln));
			var word1=line.field(" ",1);

			//for external subroutines (dll/so libraries), build up .h
			// and maybe .def file text
			if (not isprogram and word1.substr(1,11) eq "libraryinit")
				useclassmemberfunctions=true;

			if (not(isprogram) and (word1 eq "function" or word1 eq "subroutine") ) {

				//extract out the function declaration in including arguments
				//eg "function xyz(in arg1, out arg2)"
				var funcdecl=line.field("{",1);

				var funcname=funcdecl.field(" ",2).field("(",1);

#if EXODUS_EXPORT_USING_DEF
				++defordinal;
				deftext^=crlf^" "^funcname^" @"^defordinal;
#endif
				if (loadtimelinking) {
					headertext^=crlf^funcdecl^";";
				} else {
					var libname=filebase;
					var returnsvarorvoid=(word1=="function")?"var":"void";
					var callorreturn=(word1=="function")?"return":"call";
					var funcreturnvoid=(word1=="function")?0:1;
					var funcargsdecl=funcdecl.field("(",2,999999);
					//funcargsdecl=funcargsdecl.field(")",1);
					int level=0;
					int charn;
					for (charn=1;charn<=len(funcargsdecl);++charn) {
						var ch=funcargsdecl.substr(charn,1);
						if (ch eq ")") {
							if (level eq 0)
								break;
							--level;
						} else if (ch eq "(")
							++level;
					}
					funcargsdecl.substrer(1,charn-1);

					//work out the function arguments without declaratives
					//to be inserted in the calling brackets.
					var funcargs="";
					//default to one template argument for functors with zero arguments

					var nodefaults=index(funcargsdecl,"=") eq 0;
					var funcargsdecl2=funcargsdecl;

					var funcargstype="int";
					if (useclassmemberfunctions)
						funcargstype="";

					int nargs=dcount(funcargsdecl,",");

/* no longer generate default arguments using a dumb process since some pedantic compilers eg g++ 4.2.1 on osx 10.6 refuse to default non-constant parameters (io/out)
for now, programmers can still manually define defaults eg "func1(in arg1=var(), in arg2=var())"

new possibility can default ALL arguments regardless of i/o status:
eg:
	var lib1(in aaa,out bbb)
could generate the following overloads in the lib's .h header
	var lib1(in aaa){var bbb;return lib1(aaa,bbb);}
	var lib1(){var aaa;var bbb;return lib1(aaa,bbb);}

*/
					nodefaults=0;
					for (int argn=1; argn<=nargs; ++argn) {
						var funcarg=field(funcargsdecl,',',argn).trim();

						//remove any default arguments (after =)
						//TODO verify this is ok with <exodus/mvlink.h> method below

						funcarg=field(funcarg,"=",1).trim();

						//default all if all are var (in io out)
						if (nodefaults) {
							if (var("in io out").locateusing(funcarg.field(" ",1)," "))
								fieldstorer(funcargsdecl2,",",argn,1,funcarg^"=var()");
							else
								//reset to original if anything except in io out
								funcargsdecl2=funcargsdecl;
						}

						//assume the last word (by spaces) is the variable name
						fieldstorer(funcargs,',',argn,1,funcarg.field2(" ",-1));

						//assume everything except the last word (by spaces) is the variable type
						//wrap it in brackets otherwise when filling in missing parameters
						//the syntax could be wrong/fail to compile eg "const var&()" v. "(const var&)()"
						var argtype=field(funcarg," ",1,dcount(funcarg," ")-1);
						fieldstorer(funcargstype,',',argn,1,argtype);
					}

/*
//new method using member functions to call external functions with mv environment
var inclusion=
"\r\n"
"\r\n//a member variable/object to cache a pointer/object for the shared library function"
"\r\nExodusFunctorBase efb_funcx;"
"\r\n"
"\r\n//a member function with the right arguments, returning a var or void"
"\r\nvarorvoid funcx(in arg1=var(), out arg2=var(), out arg3=var())"
"\r\n{"
"\r\n"
"\r\n //first time link to the shared lib and create/cache an object from it"
"\r\n //passing current standard variables in mv"
"\r\n if (efb_funcx.pmemberfunction_==NULL)"
"\r\n  efb_funcx.init(\"funcx\",\"exodusprogrambasecreatedelete_\",mv);"
"\r\n"
"\r\n //define a function type (pExodusProgramBaseMemberFunction)"
"\r\n //that can call the shared library object member function"
"\r\n //with the right arguments and returning a var or void"
"\r\n typedef varorvoid (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,out,out);"
"\r\n"
"\r\n //call the shared library object main function with the right args,"
"\r\n // returning a var or void"
"\r\n callorreturn CALLMEMBERFUNCTION(*(efb_funcx.pobject_),"
"\r\n ((pExodusProgramBaseMemberFunction) (efb_funcx.pmemberfunction_)))"
"\r\n  (arg1,arg2,arg3);"
"\r\n"
"\r\n}";
*/

//new method using member functions to call external functions with mv environment
//using a functor class that allows library name changing
var inclusion=
"\r\n"
"\r\n//a member variable/object to cache a pointer/object for the shared library function"
"\r\n//ExodusFunctorBase efb_funcx;"
"\r\nclass efb_funcx : private ExodusFunctorBase"
"\r\n{"
"\r\npublic:"
"\r\n"
"\r\nefb_funcx(MvEnvironment& mv) : ExodusFunctorBase(\"funcx\", \"exodusprogrambasecreatedelete_\", mv) {}"
"\r\n"
"\r\nefb_funcx& operator=(const var& newlibraryname) {"
"\r\n        closelib();"
"\r\n        libraryname_=newlibraryname.toString();"
"\r\n        return (*this);"
"\r\n}"
"\r\n"
"\r\n//a member function with the right arguments, returning a var or void"
"\r\nvar operator() (in arg1=var(), out arg2=var(), out arg3=var())"
"\r\n{"
"\r\n"
"\r\n //first time link to the shared lib and create/cache an object from it"
"\r\n //passing current standard variables in mv"
"\r\n //first time link to the shared lib and create/cache an object from it"
"\r\n //passing current standard variables in mv"
"\r\n //if (efb_getlang.pmemberfunction_==NULL)"
"\r\n // efb_getlang.init(\"getlang\",\"exodusprogrambasecreatedelete_\",mv);"
"\r\n if (this->pmemberfunction_==NULL)"
"\r\n  this->init();"
"\r\n"
"\r\n //define a function type (pExodusProgramBaseMemberFunction)"
"\r\n //that can call the shared library object member function"
"\r\n //with the right arguments and returning a var or void"
"\r\n typedef varorvoid (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,out,out);"
"\r\n"
"\r\n //call the shared library object main function with the right args,"
"\r\n // returning a var or void"
"\r\n //callorreturn CALLMEMBERFUNCTION(*(efb_funcx.pobject_),"
"\r\n //((pExodusProgramBaseMemberFunction) (efb_funcx.pmemberfunction_)))"
"\r\n // (mode);"
"\r\n return CALLMEMBERFUNCTION(*(this->pobject_),"
"\r\n ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))"
"\r\n  (arg1,arg2,arg3);"
"\r\n"
"\r\n}"
"\r\n"
"\r\n};"
"\r\nefb_funcx funcx{mv};";
					swapper(inclusion,"funcx",field2(libname, SLASH, -1));
					//swapper(example,"exodusprogrambasecreatedelete_",funcname);
					swapper(inclusion,"in arg1=var(), out arg2=var(), out arg3=var()",funcargsdecl2);
					swapper(inclusion,"in,out,out",funcargstype);
					swapper(inclusion,"arg1,arg2,arg3",funcargs);
					swapper(inclusion,"varorvoid", returnsvarorvoid);
					swapper(inclusion,"callorreturn", callorreturn);

					var usepredefinedfunctor=nargs<=EXODUS_FUNCTOR_MAXNARGS;
					if (useclassmemberfunctions) {
						if (funcname eq "main")
									headertext^=inclusion;
					}

					else if (usepredefinedfunctor) {
						//example output for a subroutine with 1 argument of "in" (const var&)
						//ending of s1 and S1 indicates subroutine of one argument
						//#include "xmvfunctors1.h"
						//ExodusFunctorS1<in> func2("func2","func2");

						var functype=funcreturnvoid?"s":"f";

						var funcdecl="ExodusFunctor"^functype.ucase()^nargs^"<"^funcargstype^"> ";
						funcdecl^=funcname^"("^libname.quote()^","^funcname.quote()^");";

						//dont include more than once in the header file
						//function might appear more than once if declared and defined separately
						if (not index(headertext,funcdecl)) {
							headertext^=crlf;
							headertext^=crlf;
							var includefunctor="#include <exodus/xfunctor"^functype^nargs^".h>"^crlf;
							if (not index(headertext,includefunctor))
								headertext^=includefunctor;
							headertext^=funcdecl;
						}

					} else {
						headertext^=crlf;
						headertext^="#define EXODUSLIBNAME "^libname.quote()^crlf;
						headertext^="#define EXODUSFUNCNAME "^funcname.quote()^crlf;
						headertext^="#define EXODUSFUNCNAME0 "^funcname^crlf;
						headertext^="#define EXODUSFUNCRETURN "^returnsvarorvoid^crlf;
						headertext^="#define EXODUSFUNCRETURNVOID "^funcreturnvoid^crlf;
						headertext^="#define EXODUSfuncargsdecl "^funcargsdecl^crlf;
						headertext^="#define EXODUSfuncargs "^funcargs^crlf;
						headertext^="#define EXODUSFUNCTORCLASSNAME ExodusFunctor_"^funcname^crlf;
						headertext^="#define EXODUSFUNCTYPE ExodusDynamic_"^funcname^crlf;
						//headertext^="#define EXODUSCLASSNAME Exodus_Functor_Class_"^funcname^crlf;
						headertext^="#include <exodus/mvlink.h>"^crlf;
						//undefs are automatic at the end of mvlink.h to allow multiple inclusion
					}
				}
			}

			//build up list of loadtime libraries required by linker
			if (loadtimelinking and word1 eq "#include") {
				var word2=line.field(" ",2);
				if (word2[1]==DQ) {
					word2=word2.substr(2,word2.len()-2);
					if (word2.substr(-2,2) eq ".h")
						word2.splicer(-2,2,"");
					//libnames^=" "^word2;
					if (compiler=="cl")
						linkoptions^=" "^word2^".lib";
				}
			}
		}
		if (headertext) {
			var filebaseend=filebase.field2(SLASH,-1);
			headertext.splicer(1,0,"#define EXODUSDLFUNC_"^ucase(filebaseend)^"_H");
			headertext.splicer(1,0,SLASH^SLASH^"#ifndef EXODUSDLFUNC_"^ucase(filebaseend)^"_H"^crlf);
			headertext.splicer(1,0,SLASH^SLASH^"generated by exodus \"compile "^filebase^DQ^crlf);
			headertext^=crlf^"//#endif"^crlf;
			var headerfilename=filebase^".h";

			if (verbose)
				print("header file "^headerfilename^" ");
			if (osread(headerfilename) ne headertext) {
				oswrite(headertext,headerfilename);
				if (verbose)
					printl("generated or updated.");
			} else if (verbose)
					printl("already generated and is up to date.");
		}

#if EXODUS_EXPORT_USING_DEF
		//add .def file to linker so that functions get exported without "c++ name decoration"
		//then the runtime loader dlsym() can find the functions by their original (undecorated) name
		//http://wyw.dcweb.cn/stdcall.htm
		//CL can accept a DEF file on the command line, and it simply passes the file name to LINK
		// cl /LD testdll.obj testdll.def
		//will become
		// link /out:testdll.dll /dll /implib:testdll.lib /def:testdll.def testdll.obj
		if (deftext) {
			deftext.splicer(1,0,"LIBRARY "^filebase^crlf^"EXPORTS");
			var deffilename=filebase^".def";
			oswrite(deftext,deffilename);
			if (compiler=="cl" and usedeffile)
				linkoptions^=" /def:"^filebase^".def";
		}
#endif

		if (verbose) {
			binfileextension.outputl("Bin file extension=");
			objfileextension.outputl("Obj file extension=");
			binfilename.outputl("Binary file=");
			objfilename.outputl("Object file=");
			outputdir.outputl("Output directory=");
			compileoptions.outputl("Compile options=");
		}

		//record the current bin file update timestamp so we can 
		//later detect if compiler produces anything to be installed
		var oldobjfileinfo=osfile(objfilename);

		//check object code can be produced.
 		if (oldobjfileinfo) {
			if (osopen(objfilename,objfilename))
				osclose(objfilename);
			else if (windows or (posix and not updateinusebinposix)) {
				objfilename.errput("Error: objectfile cannot be updated. (1) Insufficient rights on ");
				if (windows)
					errput(" or cannot compile programs while in use or blocked by anti-virus/anti-malware");
				errputl(".");
				continue;
			}
		}

		//build the compiler command
		var compilecmd=compiler ^ " " ^ srcfilename ^ " " ^ basicoptions ^ " " ^ compileoptions;
		if (outputoption)
			compilecmd^= " " ^ outputoption ^ " " ^ objfilename;
		compilecmd ^= linkoptions;
		//capture the output
		//compilecmd ^= " 2>&1 | tee " ^ srcfilename ^ ".err~";
		//no tee on windows so cannot monitor output at the moment until implement popen() call it osopen(cmd)?
		//similar tie syntax but different order
		var compileoutputfilename=srcfilename ^ ".~";
		if (SLASH eq "/")
			compilecmd ^= " 2>&1 | tee " ^ compileoutputfilename.quote();
		else
			compilecmd ^= " > " ^ compileoutputfilename.quote() ^ " 2>&1";

		//call the compiler
		///////////////////
		if (verbose)
			printl(compilecmd);
		osshell(compilecmd);

		//handle compiler output
		var compileroutput;
		var startatlineno;
		if (osread(compileroutput,compileoutputfilename)) {
			if (verbose) {
			compileroutput.outputl("Compiler output:");
			}
			//leave for editor
			//osdelete(compileoutputfilename);
			var charn=index(compileroutput, ": error:");
			if (charn) {
				startatlineno=compileroutput.substr(charn-9,9);
				startatlineno=startatlineno.field2(":",-1);
//				print("ERROR(S) FOUND IN " ^ srcfilename ^ " STARTING IN LINE "^startatlineno^" ... ");
//				var().input(1);
				continue;
			}
		}

		//get new objfile info or continue
		var newobjfileinfo=osfile(objfilename);
		if (not newobjfileinfo) {
			ncompilationfailures++;
			errputl(oscwd());
			objfilename.errputl("Error: Cannot output file ");
			//var("Press Enter").input();
			continue;
		}

		//if new objfile
		if (newobjfileinfo not_eq oldobjfileinfo) {
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

				//How to: Embed a Manifest Inside a C/C++ Application
				//http://msdn.microsoft.com/en-us/library/ms235591%28VS.80%29.aspx
				//mt.exe manifest MyApp.exe.manifest -outputresource:MyApp.exe;1
				//mt.exe manifest MyLibrary.dll.manifest -outputresource:MyLibrary.dll;2
				if (SLASH_IS_BACKSLASH and PLATFORM_ ne "x64") {
					var cmd="mt.exe";
					if (not verbose) {
						cmd^=" -nologo";
						cmd^=" -manifest "^objfilename^".manifest";
						cmd^=" -outputresource:"^objfilename^";"^(isprogram?"1":"2");
						cmd^=" 1> nul >2 nul";
						if (osshell(cmd)==0)
							osdelete(objfilename^".manifest");
					}
				}

				//copy the obj file to the output directory
				if (installcmd) {

					//make the target directory
					if (not osdir(outputdir)) {
						var cmd="mkdir " ^ outputdir;
						if (verbose)
							printl(cmd);
						//osshell(cmd);
						if (!osmkdir(outputdir))
							cmd.errputl("ERROR: Failed to make directory");
					}

					//check can install file
					var outputpathandfile=outputdir^field2(binfilename,SLASH,-1);
					if (osfile(outputpathandfile)) {
						if (osopen(outputpathandfile,outputpathandfile)) {
							osclose(outputpathandfile);

						} else if (windows or (posix and not updateinusebinposix)) {
							ncompilationfailures++;
							outputpathandfile.errput("Error: Cannot update ");
							errput(" In use or insufficient rights (2)");
							if (windows)
								errput(" or cannot install/catalog program while in use or blocked by antivirus/antimalware");
							errputl(".");
							continue;

						} else {
							outputpathandfile.logput("Warning: ");
							logputl(" updated while in use. Reload required.");
						}
					}

					var cmd=installcmd^" " ^ objfilename ^ " " ^ outputpathandfile;
					if (verbose)
						printl(cmd);
					osshell(cmd);
					if (osfile(outputpathandfile) and osfile(objfilename))
						osdelete(outputpathandfile);
					//if (!oscopy(objfilename,outputpathandfile))
					//	printl("ERROR: Failed to "^cmd);

					//delete any manifest from early versions of compile which didnt have the
					//MANIFEST:NO option
					//was try to copy ms manifest so that the program can be run from anywhere?
					if (SLASH_IS_BACKSLASH and PLATFORM_ ne "x64") {
						if (true or (isprogram and manifest)) {
							if (not oscopy(objfilename^".manifest",outputpathandfile^".manifest"))
								{}//errputl("ERROR: Failed to "^cmd);
						}
						else {
							osdelete(objfilename^".manifest");
							osdelete(outputpathandfile^".manifest");
						}
					}
				}
			}
		}//compilation
	}//fileno
	return ncompilationfailures;
}

function set_environment() {

	if (verbose)
		printl("Searching standard directories");

	var searchdirs="";

	searchdirs^=FM ^ osgetenv("CC");

	//Visual Studio future?
	searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 11.0\\Common7\\Tools\\";
	searchdirs^=FM ^ osgetenv("VS110COMNTOOLS");

	searchdirs^=FM ^ "\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\bin\\";

	//Visual Studio 2010?
	searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 10.0\\Common7\\Tools\\";
	searchdirs^=FM ^ osgetenv("VS100COMNTOOLS");

	searchdirs^=FM ^ "\\Program Files\\Microsoft SDKs\\Windows\\v7.0a\\bin\\";

	//Visual Studio 2008 (Paid and Express)
	searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\Tools\\";
	searchdirs^=FM ^ osgetenv("VS90COMNTOOLS");

	//Visual Studio 2005
	searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 8\\Common7\\Tools\\";
	searchdirs^=FM ^ osgetenv("VS80COMNTOOLS");

	//http://syzygy.isl.uiuc.edu/szg/doc/VisualStudioBuildVars.html for VS6 and VS2003 folder info

	//Visual Studio .NET 2003
	searchdirs^= FM ^ "\\Program Files\\Microsoft Visual Studio .NET 2003\\Common7\\Tools\\";
	searchdirs^= FM ^ osgetenv("V70COMNTOOLS");

	//Visual Studio 6.0
	searchdirs^= FM ^ "\\Program Files\\Microsoft Visual Studio\\Common\\Tools\\";
	searchdirs^= FM ^ osgetenv("VS60COMNTOOLS");

	searchdirs.splicer(1,1,"").trimmer(FM);
	var searched="";
	var batfilename="";

	for (var ii=1;; ++ii) {

		var msvs=searchdirs.a(ii);
		if (not msvs)
			break;

		if (msvs[-1] ne SLASH)
			msvs^=SLASH;
		searched^="\n" ^ msvs;

		//get lib/path/include from batch file
		if (osdir(msvs)) {
			batfilename=msvs ^ "setenv.cmd";
			if (osfile(batfilename))
				break;
			batfilename=msvs ^ "..\\..\\vc\\vcvarsall.bat";
			if (osfile(batfilename))
				break;
			batfilename=msvs ^ "vsvars32.bat";
			if (osfile(batfilename))
				break;
		}

		//look for path on C:
		msvs.splicer(1,0,"C:");
		searched^="\n" ^ msvs;

		//get lib/path/include from batch file
		if (osdir(msvs)) {
			batfilename=msvs ^ "setenv.cmd";
			if (osfile(batfilename))
				break;
			batfilename=msvs ^ "..\\..\\vc\\vcvarsall.bat";
			if (osfile(batfilename))
				break;
			batfilename=msvs ^ "vsvars32.bat";
			if (osfile(batfilename))
				break;
		}

		batfilename="";
	}
	if (not batfilename)
			abort("Searching for C++ Compiler:\n" ^ searched ^ "\nCannot find C++ compiler. Set environment variable CC to the MSVS COMMON TOOLS directory");

	var script="call " ^ batfilename.quote();

	//work out the options from the PLATFORM_ (and perhaps debug mode)
	var options=PLATFORM_;
	if (index(batfilename,"setenv.cmd")) {
		//sdk71 wants x86 or x64
		//if (options=="x86") options="Win32";
		options="/"^options;
	}
	script^=" "^options;

	var tempfilenamebase=osgetenv("TEMP")^"\\exoduscomp$" ^rnd(99999999);

	//capture errors from the setenv
	//like the annoying
	//ERROR: The system was unable to find the specified registry key or value.
	script^=" 2> "^tempfilenamebase^".$2";

	//track first line of batch file which is the compiler configuration line
	if (verbose)
		outputl("COMPILER="^script);

	script^="\nset";
	if (verbose)
			printl("Calling script ", script);

	//create a temporary command file
	oswrite(script,tempfilenamebase^".cmd");

	//run and get the output of the command
	osshell(tempfilenamebase ^ ".cmd > " ^ tempfilenamebase ^ ".$$$");
	var result;
	if (osread(result, tempfilenamebase^".$$$")) {

		//if (verbose)
		//	printl(result);
		//printl(result);

		dim vars;
		var nvars=matparse(result.converter("\r\n",FM^FM),vars);
		for (var varn=1;varn<=nvars;++varn) {
			ossetenv(
				field(vars(varn),'=',1)
				,field(vars(varn),'=',2,999999)
				);
		}
	/*
		var value;
		if (getparam(result,"PATH",value))
				path=value;
		if (getparam(result,"INCLUDE",value))
				include=value;
		if (getparam(result,"LIB",value))
				lib=value;

		//if (verbose)
		//{
		//	printl("\nPATH=",path);
		//	printl("\nINCLUDE=",include);
		//	printl("\nLIB=",lib);
		//}

	*/
	}
	osdelete(tempfilenamebase^".cmd");
	osdelete(tempfilenamebase^".$$$");
	if (verbose) {
		var errtemp;
		if (osread(errtemp,tempfilenamebase^".$2")){
			//printl(errtemp);
		}
	}
	osdelete(tempfilenamebase^".$2");

	return true;
}

function getparam(in result, in paramname, out paramvalue)
{
	var posn=index(result.ucase(),"\n"^paramname.ucase()^"=");
	if (not posn)
		return false;
	paramvalue=result.substr(posn+len(paramname)+2).field("\n",1);
	return true;
}

programexit()

