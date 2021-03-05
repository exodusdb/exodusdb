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

#ifndef _MSC_VER
#define _POSIX_SOURCE
#endif

// for debugging
//#define TRACING

#ifdef _POSIX_SOURCE
#include "config.h"

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
// defined in config.h but config.h created by ./configure (automake tools not cmake)
//#ifdef HAVE_BACKTRACE
#include <execinfo.h>
//#endif
// for getpid
#include <unistd.h>
#endif
// for signal
#include <signal.h>

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <exodus/mv.h>

// Simple implementation of an additional output to the console:
#ifdef _MSC_VER
#include <exodus/StackWalker.h>
class MyStackWalker : public StackWalker {
   public:
	exodus::var returnlines = "";

	MyStackWalker()
		: StackWalker(), returnlines("") {}

	MyStackWalker(DWORD dwProcessId, HANDLE hProcess)
		: StackWalker(dwProcessId, hProcess), returnlines("") {
	}
	virtual void OnOutput(LPCSTR szText) {
		// fprintf(stderr,szText);

		// fprintf(stderr,"fgets:%s", path);
		exodus::var line = exodus::var(szText).convert("\x0d\x0a", "");	 //.errputl("path=");
		exodus::var filename = line.field(":", 1, 2);					 //.errputl("filename=");
		exodus::var lineno = filename.field2(" ", -1);					 //.errputl("lineno=");
		filename.splicer(-(lineno.length() + 1), 999999, "");
		lineno.substrer(2, lineno.length() - 2);
		if (filename.index("stackwalker.cpp") || filename.index("debug.cpp") ||
			filename.index("crtexe.c"))
			return;
		if (lineno.isnum() && lineno) {
			exodus::var linetext = filename.field2(exodus::OSSLASH, -1) ^ ":" ^ lineno;
			exodus::var filetext;
			// ALN:NOTE: .osread could itself throw exception and we will have loop :(
			if (filetext.osread(filename))
			//			if (filetext.osread(filename, ""))		//
			//avoid to read C++ source as UTF8
			{
				linetext ^= ": " ^ filetext.field("\x0A", lineno).trimf(" \t");
				if (linetext.index("backtrace("))
					return;
			}
			// errputl(linetext);
			returnlines ^= exodus::FM ^ linetext;
		}

		//		StackWalker::OnOutput(szText);
	}
};
#endif

#ifndef HAVE_BACKTRACE
#define NOBACKTRACE
#endif

namespace exodus {

void addbacktraceline(const var& sourcefilename, const var& lineno, var& returnlines) {

	//#ifdef TRACING
	//	sourcefilename.errputl("SOURCEFILENAME=");
	//	lineno.errputl("LINENO=");
	//#endif

	if (not lineno || not lineno.isnum())
		return;

	var linetext = sourcefilename.field2(OSSLASH, -1) ^ ":" ^ lineno;

	// get the source file text
	var filetext;
	if (filetext.osread(sourcefilename)) {
	}

	// change DOS/WIN and MAC line ends to lf only
	filetext.swapper("\x0D\x0A", "\x0A").converter("\x0D", "\x0A");

	// extract the source line
	var line = filetext.field("\x0A", lineno).trimf(" \t");

	// suppress confusing and unhelpful exodus macros
	//if ((line.substr(1,12) == "programexit(" || line.substr(1,12) == "libraryexit(" || line.substr(1,10) == "classexit(") or
	if (line.match("^(program|library|class)(init|exit)\\(") or
		(line == "}" && sourcefilename.substr(-2, 2) == ".h") or (line == ""))
		return;

#ifdef TRACING
	line.errputl();
#endif

	// errputl(linetext);
	linetext ^= ": " ^ line;

	returnlines ^= FM ^ linetext;

	return;
}

// http://www.delorie.com/gnu/docs/glibc/libc_665.html
var backtrace() {

	var returnlines = "";
	//var("backtrace()").errputl();

#ifdef _MSC_VER
	// logputl("backtrace() not implemented on windows yet");
	// var().abort("");
	MyStackWalker sw;
	sw.ShowCallstack();
	return sw.returnlines;
#elif !defined(HAVE_BACKTRACE)
	fprintf(stderr, "backtrace() not available\n");
	return "";
#else

	var internaladdresses = "";

#define BACKTRACE_MAXADDRESSES 500
	void* addresses[BACKTRACE_MAXADDRESSES];

	/* example of TRACE from osx 64
	Stack frames: 8
	Backtrace 0: 0x10000c313
	0   libexodus-11.5.0.dylib              0x000000010000c313 _ZN6exodus9backtraceEv + 99
	Backtrace 1: 0x10001ec51
	1   libexodus-11.5.0.dylib              0x000000010001ec51
	_ZN6exodus11MVErrorC2ERKNS_3varE + 129 Backtrace 2: 0x10001f314 2 libexodus-11.5.0.dylib
	0x000000010001f314 _ZN6exodus12MVUnassignedC2ERKNS_3varE + 52 Backtrace 3: 0x10000e193 3
	libexodus-11.5.0.dylib              0x000000010000e193 _ZNK6exodus3var3putERSo + 243
	Backtrace 4: 0x10000e322
	4   libexodus-11.5.0.dylib              0x000000010000e322 _ZNK6exodus3var7outputlEv + 34
	Backtrace 5: 0x10000143f
	5   steve                               0x000000010000143f _ZN13ExodusProgram4mainEv + 77
	Backtrace 6: 0x1000010e6
	6   steve                               0x00000001000010e6 main + 99
	Backtrace 7: 0x100000f64
	7   steve                               0x0000000100000f64 start + 52
	*/

	// TODO autodetect if addr2line or dwalfdump/dSYM is available

#ifdef __APPLE__
	int size = ::backtrace(addresses, BACKTRACE_MAXADDRESSES);
	char** strings = backtrace_symbols(addresses, size);
	// fprintf(stderr,"Stack frames: %d\n", size);

	for (int i = 0; i < size; i++) {

#ifdef TRACING
		// each string is like:
		// 6   steve                               0x00000001000010e6 main + 99
		//////////////////////////////////////////////////////////////////////
		fprintf(stderr, "%s\n", strings[i]);
#endif
		// parse one string for object filename and offset
		var onestring = var(strings[i]).trim();
		var objectfilename = onestring.field(" ", 2);
		var objectoffset = onestring.field(" ", 3);

		// looking for a dwarfdump line like this:
		// Line table file: 'steve.cpp' line 14, column 0 with start address
		// 0x000000010000109e
		//////////////////////////////////////////////////////////////////////////////////////

		// get a dwarfdump line containing source filename and line number
		var debugfilename = objectfilename ^ ".dSYM";
		var cmd = "dwarfdump " ^ debugfilename ^ " --lookup " ^ objectoffset ^
				  " |grep \"Line table file: \" 2> /dev/null";
		//var result = cmd.osshellread();
		result.osshellread(cmd);
#ifdef TRACING
		cmd.errputl("CMD=");
		result.errputl("RESULT=");
#endif
		if (not result)
			continue;

		// parse the dwarfdump line for source filename and line number
		var sourcefilename = result.field("'", 2);
		var lineno = result.field("'", 3).trim().field(" ", 2).field(",", 1);

		addbacktraceline(sourcefilename, lineno, returnlines);
	}

	free(strings);

	return returnlines.substr(2);

// not __APPLE_ probably LINUX
#else

	int size = ::backtrace(addresses, BACKTRACE_MAXADDRESSES);
	char** strings = backtrace_symbols(addresses, size);

	for (int ii = 0; ii < size; ii++) {

#ifdef TRACING
		fprintf(stderr, "Backtrace %d: %p \"%s\"\n", ii, addresses[ii], strings[ii]);
		// Backtrace 0: 0x7f9d247cf9fd
		// "/usr/local/lib/libexodus.so.19.01(_ZN6exodus9backtraceEv+0x62) [0x7f9d247cf9fd]"
		// Backtrace 5: 0x7f638280e3f6 "/root/lib/libl1.so(+0xa3f6) [0x7f638280e3f6]"
#endif

		// objdump --stop-address=0xa3f6 -l --disassemble ~/lib/libl1.so |grep cpp|tail -n1

		var objfilename = var(strings[ii]).field("(", 1).field(" ", 1).field("[", 1);
		var objaddress = var(strings[ii]).field("[", 2).field("]", 1);
		if (objaddress.length() > 9)
			objaddress = var(strings[ii]).field("(", 2).field(")", 1).field("+", 2);

#ifdef TRACING
		objfilename.errput("objfilename=");
		objaddress.errputl(" objaddress=");
#endif

		// if (objfilename == objfilename.convert("/\\:",""))
		if (not objfilename.osfile()) {
			// loadable program
			var temp;
			temp.osshellread("which " ^ objfilename.field2(OSSLASH, -1));
			temp = temp.field("\n", 1).field("\r", 1);
			if (temp)
				objfilename = temp;
			else
				// things like what?
				continue;
		}

		if (objaddress[1] != "0" || objaddress[2] != "x")
			continue;

		var startaddress = objaddress.splice(-3, 3, "000");
		// var temp="objdump -S --start-address=" ^ startaddress ^ " --stop-address=" ^
		// objaddress ^ " --disassemble -l " ^ objfilename;
		//var temp = "objdump --start-address=" ^ startaddress ^ " --stop-address=" ^
		//	   objaddress ^ " --disassemble -l " ^ objfilename;
#ifdef TRACING
		temp.errputl("");
#endif

		////////////////////////
		//temp = temp.osshellread();
		var temp;
		temp.osshellread("objdump --start-address=" ^ startaddress ^ " --stop-address=" ^
						 objaddress ^ " --disassemble -l " ^ objfilename);
		////////////////////////

		temp.converter("\r\n", _FM_ _FM_);

		// find the last line containing .cpp
		/// root/exodus/exodus/libexodus/exodus/l1.cpp:7 (discriminator 3)
		var nn2 = temp.dcount(FM);
		var line = "";
		var linesource = "";
		for (var ii2 = 1; ii2 < nn2; ++ii2) {
			if (temp.a(ii2).index(".cpp")) {
				line = temp.a(ii2);
				linesource = temp.a(ii2 + 1);
			}
		}

#ifdef TRACING
		if (line)
			line.errputl("line=");
#endif

		// append the source line text and number to the output
		if (line) {
			var sourcefilename = line.field(":", 1);
			var lineno = line.field(":", 2).field(" ", 1);
			addbacktraceline(sourcefilename, lineno, returnlines);
			// returnlines^=FM^sourcefilename.field2(OSSLASH,-1) ^ ":" ^ lineno ^ " " ^
			// linesource;
		}

		// trying another way
		// char syscom[1024];
		// syscom[0] = '\0';
		// snprintf(syscom, 1024, "eu-addr2line '%p' --pid=%d > /dev/stderr\n", strings[ii],
		// getpid());
		////snprintf(syscom, 1024, "eu-addr2line '%p' --pid=%d \n", strings[ii], getpid());
		// printf("syscom %s\n",syscom);
		// if (system(syscom) != 0)
		//	fprintf(stderr, "eu-addr2line failed\n");
	}

	free(strings);
	/*
		FILE *fp;
		//int status;
		char path[1024*1024];

		var binaryfilename=EXECPATH2.field(" ",1);
		//if (binaryfilename == binaryfilename.convert("/\\:",""))
		if (not binaryfilename.osdir())
		{
			var temp="which " ^ binaryfilename.field2(OSSLASH,-1);
			temp=temp.osshellread().field("\n",1).field("\r",1);
			if (temp)
				binaryfilename=temp;
		}

	#ifdef __APPLE__
		var oscmd="atos -o " ^ binaryfilename.quote() ^ " " ^ internaladdresses;
	#else

		///////////////////////////////////////////////////////////////////////////////////
		//NOTE WELL ... PIE/ALSR may prevent addr2line from identifying source line numbers
		//https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=860394#15
		//solved by adding -no-pie to exodus compile options in compile.cpp
		///////////////////////////////////////////////////////////////////////////////////

		var oscmd="addr2line -e " ^ binaryfilename.quote() ^ " " ^ internaladdresses;
	#endif
		//oscmd.errputl();

	#ifdef TRACING
		fprintf(stderr,"EXECPATH = %s\n",EXECPATH2.toString().c_str());
		fprintf(stderr,"executing %s\n",oscmd.toString().c_str());
	#endif

		//Open the command for reading
		fp = popen(oscmd.toString().c_str(), "r");
		if (fp == NULL)
		{
			fprintf(stderr,"Failed to run command\n" );
			return "";
		}

	#ifdef TRACING
		fprintf(stderr,"reading output of addr2line\n");
	#endif

		//Read the output a line at a time - output it.
		while (fgets(path, sizeof(path)-1, fp) != NULL)
		{

	#ifdef TRACING
			fprintf(stderr,"fgets:%s", path);
	#endif

			var path2=var(path).convert("\x0d\x0a","");//.errputl("path=");
			var sourcefilename=path2.field(":",1);//.errputl("filename=");
			var lineno=path2.field(":",2).field("
	",1).field("(",1);//.errputl("lineno=");

	#ifdef TRACING
			sourcefilename.errput("sourcefilename: ");
			lineno.errputl(" lineno:");
	#endif
			addbacktraceline(sourcefilename,lineno,returnlines);
		}

		//close
		pclose(fp);
	*/
	return returnlines.substr(2);
#endif

#endif
}

//service managers like systemd will send a polite SIGTERM signal
//and wait for say 90 seconds before sending a kill signal
void SIGTERM_handler(int) {
	fprintf(stderr, "=== SIGTERM received ===\n");
	TERMINATE_req = true;
}

//restart
void SIGHUP_handler(int) {
	fprintf(stderr, "=== SIGHUP received ===\n");
	RELOAD_req = true;
}

//signals are received by one thread at random
void SIGINT_handler(int sig) {
	// ignore more of this signal
	// var().breakoff()
	// faster/safer
	signal(sig, SIG_IGN);

	// duplicated in init and B
	// backtrace().convert(FM,"\n").errputl();

	// separate our prompting onto a new line
	fprintf(stderr, "\n");

	while (true) {

		// printf ("\nInterrupted. (C)ontinue (E)nd (B)acktrace\n");

		// errput("? ");
		var cmd;
		// if (!cmd.input("Interrupted. (C)ontinue (E)xit (B)acktrace (A)bort ?"))
		fprintf(stderr, "Interrupted. (C)ontinue (E)xit (B)acktrace (D)ebug (A)bort ?");
		fflush(stderr);
		if (!cmd.input())
			break;

		// only look at first character in uppercase
		var cmd1 = var(cmd[1]).ucase();

		// continue
		if (cmd1 == "C") {
			break;

			// exit ... unfortunately to the caller at the moment.
			// TODO flag to caller(s) to exit
		} else if (cmd1 == "E") {

			// var().abort("Aborted. User interrupt");
			fprintf(stderr, "Aborted. User interrupt\n");
			exit(1);
		} else if (cmd1 == "B") {

			// duplicated in init and B
			backtrace().convert(FM, "\n").errputl();
		} else if (cmd1 == "D") {
			// duplicated in init and B
			var pid = getpid();
			var("gdb -p " ^ pid).osshell();
		} else if (cmd1 == "A") {
			// fprintf(stderr,"To continue from here in GDB: \"signal 0\"\n");
			raise(SIGABRT);
			// breakpoint();
		}
	}

	// stop ignoreing this signal
	// var().breakon()
	// faster/safer
	signal(SIGINT, SIGINT_handler);
}

void var::breakoff() const {
	// ignore more of this signal
	signal(SIGINT, SIG_IGN);
}

//called in exodus_main to initialise signals
void var::breakon() const {
	signal(SIGINT, SIGINT_handler);	   // Ctrl+C from termio
	signal(SIGTERM, SIGTERM_handler);  // a polite request to TERMINATE
									   // probably followed by SIGABORT after some delay
	signal(SIGHUP, SIGHUP_handler);	   // a request to reload

	//turn off text input and output signals to prevent breaking out into gdb debugger
	//http://curiousthing.org/sigttin-sigttou-deep-dive-linux
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
}

}  // namespace exodus
