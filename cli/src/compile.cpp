
//.def file is one way on msvc to force library function names to be "undecorated"
// and therefore usable in on demand library loading
//Declaring functions with extern "C" is the other way and seems less complicated
//but msvc warns that functions defined with extern c cannot return var
//nevertheless, returning vars seems to work in practice and .def files seem
//more complicated so for now use extern "C" and not .def files
#define EXODUS_EXPORT_USING_DEF 0
#define EXODUS_FUNCTOR_MAXNARGS 20

#include <exodus/exodus.h>

//#include <exodus/xfunctorf0.h>
//ExodusFunctorF0<int> xyz;

function getparam(in result, in paramname, out paramvalue)
{
	var posn=index(result.ucase(),"\n"^paramname.ucase()^"=");
	if (not posn)
		return false;
	paramvalue=result.substr(posn+len(paramname)+2).field("\n",1);
	return true;
}

program() {

	var command=_COMMAND;

	//we use on demand/jit linking using dlopen/dlsym
	var loadtimelinking=false;

	var usedeffile=false;

	//extract options
	var verbose=index(_OPTIONS,"V");
	var debugging=index(_OPTIONS,"B");
//	verbose=1;

	//extract filenames
	var filenames=field(command,FM,2,999999999);
	var nfiles=dcount(filenames,FM);
	if (not filenames)
		abort("Syntax is compile filename ... {options}");

	if (osgetenv("EXODUS_DEBUG"))
		debugging=true;

	//source extensions
	var src_extensions="cpp cxx cc";
	var inc_extensions="h hpp hxx";
	var noncompilable_extensions=inc_extensions^ " out so o";
	var default_extension="cpp";

	var compiler;
	var basicoptions;
	var linkoptions;
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

	//hard coded compiler options at the moment
	//assume msvc (cl) on windows and g++ otherwise
	if (_SLASH=="/")
	{
		if (verbose)
			println("Posix environment detected. Assuming standard C++ compiler g++");

		compiler="g++";

		//basic compiler options
		basicoptions=" -Wall -ansi";
		//following will result in 2 byte wchar in linux
		//but all exodus libs also need to be in this format too
		//leave as 4 byte for now to avoid forcing additional compilation option on exodus users
		//who use g++ directly (or put runtime check that program+lib have the same size wchar
		//advising the right compilation option.
		//basicoptions^=" -fshort-wchar";

		//exodus library
		if (debugging)
			linkoptions=" -lexodus-gd";
		else
			linkoptions=" -lexodus";

		//enable function names in backtrace
		basicoptions^=" -g -rdynamic";

		outputoption=" -o ";
		//optimiser unfortunately prevents backtrace
		//basicoptions^="-O1";
		binoptions="";
		//binoptions=" -g -L./ -lfunc1 -Wl,-rpath,./";
		//binoptions=" -fPIC";

		//make a shared library
		liboptions=" -fPIC -shared";
		//soname?
#if __GNUC__ >= 4
		//use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
		liboptions^=" -Wl,-fvisibility=hidden,-fvisibility-inlines-hidden";
#endif

		//target directories
		bindir="~/bin";
		libdir="~/lib";

		//target extensions
		objfileextension=".out";
		binfileextension="";
		libfileextension=".so";
		libfileprefix="lib";

		installcmd="mv";
	}
	else
	{
		if (verbose)
			println("Windows environment detected. Finding C++ compiler.");

		//get current environment
		var path=osgetenv("PATH");
		var include=osgetenv("INCLUDE");
		var lib=osgetenv("LIB");

		//locate MS Visual Studio by environment variable or current disk or C:
		var searchvars="CC VS90COMNTOOLS VS80COMNTOOLS VS70COMNTOOLS";
		if (verbose)
			searchvars.outputln("Searching Environment Variables : " ^ searchvars);

		var msvs;
		for (var ii=1;;++ii)
		{
			var envname=searchvars.field(" ",ii);
			if (not envname)
				break;
			msvs=osgetenv(envname);
			if (verbose)
				msvs.outputln(envname^" is ");
			if (msvs)
				break;
		}

		if (not msvs)
		{
			if (verbose)
				println("Searching standard directories");

			var searched=searchvars ^ " environment variables";
			var searchdirs="";

			//Visual Studio future?
			searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 11.0\\Common7\\Tools\\";

			//Visual Studio 2010?
			searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 10.0\\Common7\\Tools\\";

			//Visual Studio 2008 (Paid and Express)
			searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\Tools\\";

			//Visual Studio 2005
			searchdirs^=FM ^ "\\Program Files\\Microsoft Visual Studio 8\\Common7\\Tools\\";

			//http://syzygy.isl.uiuc.edu/szg/doc/VisualStudioBuildVars.html for VS6 and VS2003 folder info

			//Visual Studio .NET 2003
			searchdirs^= FM ^ "\\Program Files\\Microsoft Visual Studio .NET 2003\\Common7\\Tools\\";

			//Visual Studio 6.0
			searchdirs^= FM ^ "\\Program Files\\Microsoft Visual Studio\\Common\\Tools\\";

			searchdirs.splicer(1,1,"");
			for (var ii=1;;++ii)
			{
				msvs=searchdirs.extract(ii);
				if (not msvs)
					break;
				if (osdir(msvs))
					break;
				searched^="\n" ^ msvs;
				msvs.splicer(1,0,"C:");
				if (osdir(msvs))
					break;
				searched^="\n" ^ msvs;
			}
			if (not msvs)
				abort("Searching for C++ Compiler:\n" ^ searched ^ "\nCannot find C++ compiler. Set environment variable CC to the MSVS COMMON TOOLS directory");
		}

		//call VSvars32 to get the path, include and lib
		if (msvs)
		{
			if (msvs.substr(-1) ne _SLASH)
				msvs^=_SLASH;
			//get lib/path/include from batch file
			var tempfilenamebase="comp$" ^rnd(99999999);
			var script="call " ^ (msvs ^ "vsvars32").quote();
			script^="\nset";
			if (verbose)
				println("Calling scriptscript");
			oswrite(script,tempfilenamebase^".cmd");
			osshell(tempfilenamebase ^ ".cmd > " ^ tempfilenamebase ^ ".$$$");
			var result;
			if (osread(result, tempfilenamebase^".$$$"))
			{
				//if (verbose)
				//	println(result);
				//println(result);
				result.converter("\r","\n");
				var value;
				if (getparam(result,"PATH",value))
					path=value;
				if (getparam(result,"INCLUDE",value))
					include=value;
				if (getparam(result,"LIB",value))
					lib=value;
				/*
				if (verbose)
				{
					println("\nPATH=",path);
					println("\nINCLUDE=",include);
					println("\nLIB=",lib);
				}
				*/

			}
			osdelete(tempfilenamebase^".cmd");
			osdelete(tempfilenamebase^".$$$");
		}

		if (verbose)
			println("Searching for Exodus for include and lib directories");

		//locate Exodus by executable path, environment variable or current disk or C:
		var ndeep=dcount(_EXECPATH,_SLASH);
		var exoduspath="";
		if (ndeep>2)
			exoduspath=_EXECPATH.field(_SLASH,1,ndeep-2);
		var searched="";
		if (not exoduspath or not (osfile(exoduspath^_SLASH^"bin\\exodus.dll") or osfile(exoduspath^_SLASH^"exodus.dll")))
		{
			if (exoduspath)
				searched^="\n"^exoduspath;
			exoduspath=osgetenv("EXODUS_PATH");
			if (not exoduspath)
			{
				searched^="\nEXODUS_PATH environment variable";
				exoduspath=L"\\Program Files\\exodus\\" EXODUS_RELEASE L"\\";
				if (not osdir(exoduspath))
				{
					searched^="\n" ^ exoduspath;
					exoduspath.splicer(1,0,"C:");
				}
			}
		}
		if (exoduspath.substr(-1,1) ne _SLASH)
			exoduspath^=_SLASH;
		var exodusbin=exoduspath;
		if (not osfile(exodusbin^"exodus.dll"))
		{
			searched^="\n" ^ exoduspath;
			exodusbin=exoduspath^"bin\\";
			if (not osfile(exodusbin^"exodus.dll"))
			{
				//exodusbin=_EXECPATH;
				searched^="\n" ^ exoduspath;
				println("Searching for Exodus",searched);
				abort("Cannot find Exodus. Set environment variable EXODUS_PATH to exodus directory and restart Exodus");
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
			println("Failed to set PATH environment variable");
		if (not ossetenv("INCLUDE",include))
			println("Failed to set INCLUDE  environment variable");
		if (not ossetenv("LIB",lib))
			println("Failed to set BIN environment variable");
		if (verbose)
		{
			println("\nPATH=",path);
			println("\nINCLUDE=",include);
			println("\nLIB=",lib);
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

		//Uses the __cdecl calling convention (x86 only ie 32bit).
		///Gd, the default setting, specifies the __cdecl calling convention
		//for all functions except C++ member functions and functions
		//marked __stdcall or __fastcall.
		basicoptions^=" /Gd";

		//exodus library
		if (debugging)
		{
			//Creates a debug multithreaded DLL using MSVCRTD.lib.
			basicoptions^=" /MDd";

			//Disables optimization.
			basicoptions^=" /Od";

			//Enables whole program optimization.
			basicoptions^=" /GL";

			//renames program database?
			basicoptions^=" /FD";

			//macro used in exodus in some places to provide additional information eg mvdbpostgres
			basicoptions^=" /D \"DEBUG\"";

			//Enables run-time error checking.
			basicoptions^=" /RTC1";

		} else
		{
			//Creates a multithreaded DLL using MSVCRT.lib.
			//basicoptions^=" /MD";
			//needed for backtrace
			//Creates a debug multithreaded DLL using MSVCRTD.lib.
			basicoptions^=" /MDd";

			//Creates fast code.
			basicoptions^=" /O2";

			//Enable string pooling
			basicoptions^=" /GF";

			//Enables minimal rebuild.
			basicoptions^=" /Gm";

			//turn off asserts
			basicoptions^=" /D \"NDEBUG\"";
		}

		linkoptions=" /link exodus.lib";

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

		//target directories
		bindir="~/bin";
		libdir="~/lib";

		//target extensions
		objfileextension=".exe";
		binfileextension=".exe";
		libfileextension=".dll";

		//installcmd="copy /y";
		installcmd="";
	}

	swapper(bindir,"~",osgetenv("HOME"));
	swapper(libdir,"~",osgetenv("HOME"));

	if (bindir.substr(-1) ne _SLASH)
		bindir ^= _SLASH;
	if (libdir.substr(-1)!=_SLASH)
		libdir ^= _SLASH;

	for (var fileno=1;fileno<=nfiles;++fileno) {

		var text="";
		var filebase;

		//get the next file name
		var srcfilename=filenames.extract(fileno).unquote();
		if (not srcfilename)
			continue;

		//println("--- ",srcfilename, " ---");

		//check/add the default file extension
		var fileext=srcfilename.field2(".",-1).lcase();
		if (src_extensions.locateusing(fileext," ")) {
			filebase=srcfilename.splice(-len(fileext)-1,999999,"");

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
		println(srcfilename);
		if (not text and not text.osread(srcfilename)) {
			errputln(srcfilename^" doesnt exist");
			continue;
		}

		//determine if program or subroutine/function
		var isprogram=index(text,"int main(") or index(text, "program()");
		if (debugging)
			isprogram.outputln("Is Program:");
		var outputdir;
		var compileoptions;
		var binfilename=filebase;
		var objfilename=filebase;
		if (isprogram) {
			binfilename^=binfileextension;
			objfilename^=objfileextension;
			outputdir=bindir;
			compileoptions=binoptions;
		}else{
			//binfilename^=binfileextension;
			binfilename^=libfileextension;
			objfilename^=libfileextension;
			if (libfileprefix)
			{
				binfilename=libfileprefix^binfilename;
				objfilename=libfileprefix^objfilename;
			}
			outputdir=libdir;
			compileoptions=liboptions;
		}

		//and, for subroutines and functions, create header file (even if compilation not successful)
		var crlf="\r\n";
		var headertext="";
		converter(text,"\r\n",FM^FM);
		var nlines=dcount(text,FM);
		varray text2(nlines);
		text.matparse(text2);

#if EXODUS_EXPORT_USING_DEF
		var deftext="";
		var defordinal=0;
#endif
		for (int ln=1;ln<=nlines;++ln) {
			var line=trimf(text2(ln));
			var word1=line.field(" ",1);

			//for external subroutines (dll/so libraries), build up .h
			// and maybe .def file text
			if (not(isprogram) and ( word1 eq "function" or word1 eq "subroutine") ) {

				//extract out the function declaration in including arguments
				//eg "function xyz(in arg1, out arg2)"
				var funcdecl=line.field("{",1);

				var funcname=funcdecl.field(" ",2).field("(",1);

#if EXODUS_EXPORT_USING_DEF
				++defordinal;
				deftext^=crlf^" "^funcname^" @"^defordinal;
#endif
				if (loadtimelinking)
				{
					headertext^=crlf^funcdecl^";";
				}
				else
				{
					var libname=filebase;
					var funcreturn=(word1=="function")?"var":"void";
					var funcreturnvoid=(word1=="function")?0:1;
					var funcargsdecl=funcdecl.field("(",2).field(")",1);

					//work out the function arguments without declaratives
					//to be inserted in the calling brackets.
					var funcargs="";
					//default to one template argument for functors with zero arguments
					var funcargstype="int";
					int nargs=dcount(funcargsdecl,",");
					for (int argn=1;argn<=nargs;++argn)
					{
						var funcarg=field(funcargsdecl,',',argn).trim();

						//remove any default arguments (after =)
						//TODO verify this is ok with <exodus/mvlink.h> method below
						funcarg=field(funcarg,"=",1).trim();

						//assume the last word (by spaces) is the variable name
						fieldstorer(funcargs,',',argn,1,funcarg.field2(" ",-1));

						//assume everything except the last word (by spaces) is the variable type
						//wrap it in brackets otherwise when filling in missing parameters
						//the syntax could be wrong/fail to compile eg "const var&()" v. "(const var&)()"
						var argtype=field(funcarg," ",1,dcount(funcarg," ")-1);
						fieldstorer(funcargstype,',',argn,1,argtype);
					}

					var usepredefinedfunctor=nargs<=EXODUS_FUNCTOR_MAXNARGS;
					if (usepredefinedfunctor)
					{
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
						headertext^="#define EXODUSFUNCRETURN "^funcreturn^crlf;
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
				if (word2.substr(1,1)==DQ) {
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
			headertext.splicer(1,0,"#ifndef "^ucase(filebase)^"_H");
			headertext^=crlf^"#endif"^crlf;
			var headerfilename=filebase^".h";
			oswrite(headertext,headerfilename);
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

		if (debugging) {
			binfileextension.outputln("Bin file extension");
			objfileextension.outputln("Obj file extension");
			binfilename.outputln("Binary file:");
			objfilename.outputln("Object file:");
			outputdir.outputln("Output directory:");
			compileoptions.outputln("Compile options:");
		}

		//record the current bin file update timestamp
		var oldobjfileinfo=osfile(objfilename);

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
		if (_SLASH eq "/")
			compilecmd ^= " 2>&1 | tee " ^ compileoutputfilename.quote();
		else
			compilecmd ^= " > " ^ compileoutputfilename.quote() ^ " 2>&1";

		//call the compiler
		///////////////////
		if (verbose or debugging)
			println(compilecmd);
		osshell(compilecmd);

		//handle compiler output
		var compileroutput;
		var startatlineno;
		if (osread(compileroutput,compileoutputfilename)){
			outputln(compileroutput);
			osdelete(compileoutputfilename);
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
		if (not objfilename) {
				print("CANNOT FIND OUTPUT FILE");
				var().input(1);
				continue;
		}

		//if new objfile
		if (newobjfileinfo not_eq oldobjfileinfo) {
			if (newobjfileinfo) {

				//create the bin file directory if missing
				//osdir isnt working as expected
				//                              if (!outputdir.oslistf()) {
				//                                      osshell("mkdir " ^ outputdir);
				//                                      println("Created " ^ outputdir);
				//                              }

				//var outputfilename=filename;
				//if (isprogram) {
        			//	//remove the .out file type ending
        			//	outputfilename=outputfilename.field(".",1,outputfilename.count("."));
				//}

				//copy the obj file to the output directory
				if (installcmd) {
					if (not osdir(outputdir)) {
						var cmd="mkdir " ^ outputdir;
						if (verbose)
							println(cmd);
						//osshell(cmd);
						if (!osmkdir(outputdir))
							println("ERROR: Failed "^cmd);
					}
					var outputpathandfile=outputdir^field2(binfilename,_SLASH,-1);
					var cmd=installcmd^" " ^ objfilename ^ " " ^ outputpathandfile;
					if (verbose)
						println(cmd);
					//osshell(cmd);
					if (osfile(outputpathandfile) and osfile(objfilename))
						osdelete(outputpathandfile);
					if (!oscopy(objfilename,outputpathandfile))
						println("ERROR: Failed to "^cmd);
				}
			}
		}//compilation
	}//fileno
}
