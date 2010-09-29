#include <exodus/exodus.h>

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

	var loadtimelinking=false;

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
		basicoptions^=" -rdynamic";

		outputoption=" -o ";
		//optimiser unfortunately prevents backtrace
		//basicoptions^="-O1";
		binoptions=" -g -L./ -lfunc1 -Wl,-rpath,./";
		//binoptions=" -fPIC";

#if __GNUC__ >= 4
		//use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
		binoptions^=" -Wl,-fvisibility=hidden";
#endif
		//make a shared library
		liboptions=" -fPIC -shared";
		//soname?

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

		//basic compiler options
		basicoptions=" /EHsc /W3 /Zi /TP /D \"_CONSOLE\"";

		//exodus library
		if (debugging)
		{
			basicoptions^=" /O2 /GL /FD /MD /D \"NDEBUG\"";
		} else
		{
			basicoptions^=" /Od /GF /Gm /Gd /RTC1 /MDd /D \"DEBUG\"";
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
		var deftext="";
		var defordinal=0;
		converter(text,"\r\n",FM^FM);
		var nlines=dcount(text,FM);
		varray text2(nlines);
		text.matparse(text2);
		for (int ln=1;ln<=nlines;++ln) {
			var line=trimf(text2(ln));
			var word1=line.field(" ",1);

			//for external subroutines (dll/so libraries), build up .h and .def text
			if (not(isprogram) and word1 eq "function" or word1 eq "subroutine") {

				//extract out the function declaration in including arguments
				//eg "function xyz(in arg1, out arg2)"
				var funcdecl=line.field("{",1);

				var funcname=funcdecl.field(" ",2).field("(",1);

				++defordinal;
				deftext^=crlf^" "^funcname^" @"^defordinal;

				if (loadtimelinking)
				{
					headertext^=crlf^funcdecl^";";
				}
				else
				{
					var libname=filebase;
					var funcreturn=(word1=="function")?"var":"void";
					var funcreturnvoid=(word1=="function")?0:1;
					var funcargs=funcdecl.field("(",2).field(")",1);

					//work out the function arguments without declaratives
					//to be inserted in the calling brackets.
					var funcargs2=funcargs;
					int nargs=dcount(funcargs,",");
					for (int argn=1;argn<=nargs;++argn)
						fieldstorer(funcargs2,',',argn,1,field(funcargs,',',argn).field2(" ",-1));

					headertext^=crlf;
					headertext^="#define EXODUSLIBNAME "^libname^crlf;
					headertext^="#define EXODUSFUNCNAME "^funcname^crlf;
					headertext^="#define EXODUSFUNCRETURN "^funcreturn^crlf;
					headertext^="#define EXODUSFUNCARGS "^funcargs^crlf;
					headertext^="#define EXODUSFUNCARGS2 "^funcargs2^crlf;
					headertext^="#define EXODUSFUNCTORCLASSNAME ExodusFunctor_"^funcname^crlf;
					headertext^="#define EXODUSFUNCTYPE ExodusDynamic_"^funcname^crlf;
					headertext^="#define EXODUSLIBNAMEQQ "^libname.quote()^crlf;
					headertext^="#define EXODUSFUNCNAMEQQ "^funcname.quote()^crlf;
					//headertext^="#define EXODUSCLASSNAME Exodus_Functor_Class_"^funcname^crlf;
					headertext^="#define EXODUSFUNCRETURNVOID "^funcreturnvoid^crlf;
					headertext^="#include <exodus/mvlink.h>"^crlf;
					headertext^="#undef EXODUSLIBNAME"^crlf;
					headertext^="#undef EXODUSFUNCNAME"^crlf;
					headertext^="#undef EXODUSFUNCRETURN"^crlf;
					headertext^="#undef EXODUSFUNCARGS"^crlf;
					headertext^="#undef EXODUSFUNCARGS2"^crlf;
					headertext^="#undef EXODUSFUNCTORCLASSNAME"^crlf;
					headertext^="#undef EXODUSFUNCTYPE"^crlf;
					headertext^="#undef EXODUSLIBNAMEQQ"^crlf;
					headertext^="#undef EXODUSFUNCNAMEQQ"^crlf;
					headertext^="#undef EXODUSCLASSNAME"^crlf;
					headertext^="#undef EXODUSFUNCRETURNVOID"^crlf;
				}
			}

			//build up list of libraries required by linker
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
			//if (compiler=="cl")
			//	linkoptions^=" /def:"^filebase^".def";
		}

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
				if (isprogram and installcmd) {
					if (not osdir(outputdir)) {
						var cmd="mkdir " ^ outputdir;
						if (verbose)
							println(cmd);
						//osshell(cmd);
						if (!osmkdir(outputdir))
							println("ERROR: Failed "^cmd);
					}
					var cmd=installcmd^" " ^ objfilename ^ " " ^ outputdir ^ binfilename;
					if (verbose)
						println(cmd);
					//osshell(cmd);
					if (!oscopy(objfilename,outputdir ^ binfilename))
						println("ERROR: Failed "^cmd);
				}
			}
		}//compilation
	}//fileno
}
