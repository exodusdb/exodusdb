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
#	define EXODUS_POSIX_SOURCE
#	define EXODUS_POSIX_OSSLASH "/"
#endif

// for debugging
//#define TRACING

#ifdef EXODUS_POSIX_SOURCE
#	include <stdio.h>
#	include <cstdlib>
#	include <iostream>
#	include <string>
#	include <execinfo.h> // for backtrace
#	include <unistd.h> // for getpid
#	include <signal.h>
#endif

#include <exodus/var.h>
#include <exodus/exoimpl.h>

namespace exodus {

static void addbacktraceline(CVR frameno, CVR sourcefilename, CVR lineno, VARREF returnlines) {

	//#ifdef TRACING
	//	sourcefilename.errputl("SOURCEFILENAME=");
	//	lineno.errputl("LINENO=");
	//#endif

	if (not lineno || not lineno.isnum())
		return;

	var linetext = (frameno + 1) ^ ": " ^ sourcefilename.field2(EXODUS_POSIX_OSSLASH, -1) ^ ":" ^ lineno;

	// get the source file text
	var filetext;
	if (filetext.osread(sourcefilename)) {
	}

	// change DOS/WIN and MAC line ends to lf only
	filetext.replacer("\x0D\x0A", "\x0A").converter("\x0D", "\x0A");

	// extract the source line
	var line = filetext.field("\x0A", lineno).trimfirst(" \t");

	// suppress confusing and unhelpful exodus macros
	//if ((line.first(12) == "programexit(" || line.first(12) == "libraryexit(" || line.first(10) == "classexit(") or
	//if (line.match("^(program|library|class)(init|exit)\\(") or
	//	(line == "}" && sourcefilename.ends(".h")) or (line == ""))
	//	return;
	if (line.match("^(program|library|class)(init|exit)\\("))
		return;

#ifdef TRACING
	line.errputl();
#endif

	// errputl(linetext);
	linetext ^= ": " ^ line;

	returnlines ^= FM ^ linetext;

	return;
}

////////////////////////////////////////////////////////////////////
//// Reserve space for snapshot of stack taken on every mv exception
////////////////////////////////////////////////////////////////////
//// in case the exception is caught then it is wasted time
//namespace {
//	#define BACKTRACE_MAXADDRESSES 100
//	thread_local void* thread_stack_addresses[BACKTRACE_MAXADDRESSES];
//	thread_local size_t thread_stack_size = 0;
//}

// Capture the current stack addresses for later decoding
void mv_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t* stack_size) {
	*stack_size = ::backtrace(stack_addresses, BACKTRACE_MAXADDRESSES);
}

////////////////////////////////////////////////////////////////////
// Convert the stack addresses to source file, line no and line text
////////////////////////////////////////////////////////////////////
// http://www.delorie.com/gnu/docs/glibc/libc_665.html
var mv_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t stack_size, size_t limit) {

	var returnlines = "";
	size_t nlines = 0;

	//var("backtrace()").errputl();

	var internaladdresses = "";

//    var exo_debug;
//    exo_debug.osgetenv("EXO_DEBUG");

	/* example of TRACE from osx 64
	Stack frames: 8
	Backtrace 0: 0x10000c313
	0   libexodus-11.5.0.dylib              0x000000010000c313 _ZN6exodus9backtraceEv + 99
	Backtrace 1: 0x10001ec51
	1   libexodus-11.5.0.dylib              0x000000010001ec51
	_ZN6exodus11VarErrorC2ERKNS_3varE + 129 Backtrace 2: 0x10001f314 2 libexodus-11.5.0.dylib
	0x000000010001f314 _ZN6exodus12VarUnassignedC2ERKNS_3varE + 52 Backtrace 3: 0x10000e193 3
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

	//warning: conversion from ‘size_t’ {aka ‘long unsigned int’} to ‘int’ may change value [-Wconversion]
	char** strings = backtrace_symbols(stack_addresses, static_cast<int>(stack_size));

	for (size_t ii = 0; ii < stack_size; ii++) {

#ifdef TRACING
		fprintf(stderr, "Backtrace %d: %p \"%s\"\n", ii, stack_addresses[ii], strings[ii]);
		// Backtrace 0: 0x7f9d247cf9fd
		// "/usr/local/lib/libexodus.so.19.01(_ZN6exodus9backtraceEv+0x62) [0x7f9d247cf9fd]"
		// Backtrace 5: 0x7f638280e3f6 "/root/lib/libl1.so(+0xa3f6) [0x7f638280e3f6]"
#endif

		// objdump --stop-address=0xa3f6 -l --disassemble ~/lib/libl1.so |grep cpp|tail -n1

		var objfilename = var(strings[ii]).field("(", 1).field(" ", 1).field("[", 1);
		var objaddress = var(strings[ii]).field("[", 2).field("]", 1);
		if (objaddress.len() > 9)
			objaddress = var(strings[ii]).field("(", 2).field(")", 1).field("+", 2);

		// Skip libc
		if (objfilename.contains("libc.so"))
			continue;

		// Skip libexodus source code
		//if (not exo_debug and objfilename.contains("libexodus.so"))
		//if (objfilename.contains("libexodus.so"))
		//	continue;

#ifdef TRACING
		objfilename.errput("objfilename=");
		objaddress.errputl(" objaddress=");
#endif

		// if (objfilename == objfilename.convert("/\\:",""))
		if (not objfilename.osfile()) {
			// loadable program
			var temp;
			if (not temp.osshellread("which " ^ objfilename.field2(EXODUS_POSIX_OSSLASH, -1))) {
				//null
			}
			temp = temp.field("\n", 1).field("\r", 1);
			if (temp)
				objfilename = temp;
			else
				// things like what?
				continue;
		}

		if (objaddress[1] != "0" || objaddress[2] != "x")
			continue;

		var startaddress = objaddress.paste(-3, 3, "000");

		////////////////////////
		//temp = temp.osshellread();
		var temp;
		var cmd = "objdump --start-address=" ^ startaddress ^ " --stop-address=" ^
						 objaddress ^ " --disassemble -l " ^ objfilename;
		//cmd.errputl();
		if (not temp.osshellread(cmd))
			continue;
		////////////////////////

		temp.converter("\r\n", _FM _FM);

		// find the last line containing .cpp
		/// root/exodus/exodus/libexodus/exodus/l1.cpp:7 (discriminator 3)
		var nn2 = temp.fcount(FM);
		var line = "";
		var linesource = "";
		for (var ii2 = 1; ii2 < nn2; ++ii2) {
			if (temp.f(ii2).contains(".cpp")) {
				line = temp.f(ii2);
				linesource = temp.f(ii2 + 1);
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
			addbacktraceline(ii, sourcefilename, lineno, returnlines);
			// returnlines^=FM^sourcefilename.field2(OSSLASH,-1) ^ ":" ^ lineno ^ " " ^
			// linesource;
		}

		// Quit if reached limit of desired backtrace
		if (limit && nlines++ >= limit)
			break;

	}

	free(strings);
	return returnlines.cut(1);

}

//service managers like systemd will send a polite SIGTERM signal
//and wait for say 90 seconds before sending a kill signal
static void SIGTERM_handler(int) {
	fprintf(stderr, "=== SIGTERM received ===\n");
	TERMINATE_req = true;
}

//restart
static void SIGHUP_handler(int) {
	fprintf(stderr, "=== SIGHUP received ===\n");
	RELOAD_req = true;
}

//void var::breakoff() const {
PUBLIC void breakoff() {
	// ignore more of this signal
	signal(SIGINT, SIG_IGN);
}

PUBLIC void breakon();

//signals are received by one thread at random
//void SIGINT_handler(int sig) {
static void SIGINT_handler(int sig [[maybe_unused]]) {

	// Ignore more of this signal - restore on exit
	breakoff();
	// Faster/safer?
	//signal(sig, SIG_IGN);

	// Hide input characters
	//var().echo(true);

	// duplicated in init and B
	// backtrace().convert(FM,"\n").errputl();

	VarError err("backtrace()");

	// interact on a new line
	fprintf(stderr, "\n");

	while (true) {

		// printf ("\nInterrupted. (C)ontinue (E)nd (B)acktrace\n");

		// errput("? ");
		var cmd = "A";

		if (var().isterminal()) {
			// if (!cmd.input("Interrupted. (C)ontinue (E)xit (B)acktrace (A)bort ?"))
			fprintf(stderr, "Interrupted. (C)ontinue (Q)uit (B)acktrace (D)ebug (A)bort ? ");
			fflush(stderr);
			if (!cmd.inputn(1))
				continue;
		}

		// only look at first character in uppercase
		var cmd1 = var(cmd[1]).ucase();

		if (cmd1 == "C") {

			fprintf(stderr, "ontinuing ...\n");
			fflush(stderr);

			// Continue
			break;

		} else if (cmd1 == "Q") {

			// Quit ... unfortunately to the caller at the moment.
			// TODO flag to caller(s) to exit

			// var().abort("Aborted. User interrupt");
			fprintf(stderr, "uitting ...\n");

			// Quit
			exit(1);

		} else if (cmd1 == "B") {

			fprintf(stderr, "acktracing ...\n");
			fflush(stderr);			// duplicated in init and B

			// Backtrace
			err.stack().convert(FM, "\n").errputl();

		} else if (cmd1 == "D") {

			fprintf(stderr, "ebugging ... ");
			fflush(stderr);			// duplicated in init and B

			// Debugging
			// duplicated in init and B
			var pid = getpid();
			var cmd = "gdb -p " ^ pid;
			cmd.logputl(" ");
			if (not cmd.osshell())
				var().lasterror().logputl();

		} else if (cmd1 == "A") {

			fprintf(stderr, "borting ...\n");
			fflush(stderr);			// duplicated in init and B

			// Abort using SIGABT
			// fprintf(stderr,"To continue from here in GDB: \"signal 0\"\n");
			raise(SIGABRT);
			// breakpoint();

		} else {

			// Invalid option - try again.
			fprintf(stderr, "\n");
		}

	}

	// Show input characters
	//var().echo(true);

	// Stop ignoring this signal
	breakon();
	// faster/safer?
	//signal(SIGINT, SIGINT_handler);

}

//called in exodus_main to initialise signals
//void var::breakon() const {
PUBLIC void breakon()  {
	signal(SIGINT, SIGINT_handler);	   // Ctrl+C from termio
	signal(SIGTERM, SIGTERM_handler);  // a polite request to TERMINATE
									   // probably followed by SIGABORT after some delay
	signal(SIGHUP, SIGHUP_handler);	   // a request to reload

	//turn off text input and output signals to prevent breaking out into gdb debugger
	//http://curiousthing.org/sigttin-sigttou-deep-dive-linux
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
}

// Get a stack list on demand
ND PUBLIC var backtrace() {
	VarError e("backtrace()");
	return e.stack();
}

//void var::debug(CVR var1) const {
PUBLIC void debug(CVR var1) {
	// THISIS("var var::debug() const")

	std::clog << "debug(" << var1 << ")" << std::endl;
	std::cout << std::flush;

	//use gdb "n" command(s) to single step
	//use gdb "c" command(s) to resume

#if __has_include(<signal.h>)
	::raise(SIGTRAP);

#elif 1
	__asm__("int3");
	//__asm__("int3");
	//__asm__("int3");

#elif defined(_MSC_VER)
	// this will terminate the program rather than invoke the debugger but is catchable
	//[[unlikely]] throw VarDebug(var1);

// another way to break into the debugger by causing a seg fault
#elif 0
	*(int*)0 = 0;
#endif

	return;
}

}  // namespace exodus
