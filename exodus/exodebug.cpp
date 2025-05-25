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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#	define _OSSLASH "/"
#pragma clang diagnostic pop
#endif

// For debugging
#define TRACING 0

#ifdef EXODUS_POSIX_SOURCE
#	include <stdio.h>
#	include <cstdlib>
#	include <iostream>
#	include <string>
#	include <execinfo.h> // for backtrace
#	include <unistd.h> // for getpid
#	include <signal.h>
#	include <memory> // for make_unique
#endif

#if TRACING
#	include <boost/stacktrace.hpp>
#endif

#include <exodus/var.h>
#include <exodus/exoimpl.h>
#include <exodus/range.h>

namespace exo {

using let = const var;

static void addbacktraceline(in frameno, in sourcefilename, in lineno, io returnlines) {

	//#if TRACING
	//	sourcefilename.errputl("SOURCEFILENAME=");
	//	lineno.errputl("LINENO=");
	//#endif

	bool raw = true;

	if ((not lineno || not lineno.isnum()) and not raw)
		return;

	// Get the source file text
	var filetext;
	if (not filetext.osread(sourcefilename)) {
		if (not raw)
			return;
		filetext = "";
	}

	// Change DOS/WIN and MAC line ends to lf only
	filetext.replacer("\x0D\x0A", "\x0A");
	filetext.converter("\x0D", "\x0A");

	// Extract the source line
	let line = lineno.isnum() ? filetext.field("\x0A", lineno).trimfirst(" \t") : "";
	if ((not line or line.match("^\\s*}")) and not raw)
		return;

	// Suppress confusing and unhelpful exodus macros like programinit/exit, libraryinit/exit, classinit/exit
	if (/* not raw and */ line.match("^(program|library|class)(init|exit)\\("))
		return;

	// Skip "return nn"
	if (line.match("^\\s*return\\s*\\d*") and not raw)
		return;

#if TRACING
	line.errputl();
#endif

	// Example output line:
	// "8: p2.cpp:15: printl(v2);"
	let linetext = (frameno) ^ ": " ^ sourcefilename.field(_OSSLASH, -1) ^ ":" ^ lineno ^ ": " ^ line;
	//var linetext = std::format("{:0}: {:1}:{:2}: {:3}" , frameno, sourcefilename.field2(_OSSLASH, -1), lineno, line);

	returnlines ^= linetext ^ FM;

	return;
}

////////////////////////////////////////////////////////////////////
//// Reserve space for snapshot of stack taken on every mv exception
////////////////////////////////////////////////////////////////////
//// in case the exception is caught then it is wasted time
//namespace {
//	#define BACKTRACE_MAXADDRESSES 100
//	thread_local void* thread_stack_addresses[BACKTRACE_MAXADDRESSES];
//	thread_local std::size_t thread_stack_size = 0;
//}

// Capture the current stack addresses for later decoding
void exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) {
	*stack_size = ::backtrace(stack_addresses, BACKTRACE_MAXADDRESSES);
#if TRACING
	std::cout << boost::stacktrace::stacktrace();
#endif
}

////////////////////////////////////////////////////////////////////
// Given stack addresses, get the source file, line no and line text
////////////////////////////////////////////////////////////////////
// http://www.delorie.com/gnu/docs/glibc/libc_665.html
var exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit) {

	var returnlines = "";
	std::size_t nlines = 0;
	var mangled_addresses = "";

	//var("backtrace()").errputl();

	let internaladdresses = "";

//Backtrace 0: 0x7624d8bac192 "/root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07(_ZN3exo13exo_savestackEPPvPm+0x12) [0x7624d8bac192]"
//objdump --start-address=0000 --stop-address=0x12 --source --line-numbers /root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07
//
//Backtrace 1: 0x7624d8b9a8eb "/root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07(_ZN3exo8VarErrorC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE+0x9b) [0x7624d8b9a8eb]"
//objdump --start-address=0000 --stop-address=0x9b --source --line-numbers /root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07
//
//Backtrace 2: 0x7624d8b9b23a "/root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07(_ZN3exo13VarUnassignedC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE+0x3a) [0x7624d8b9b23a]"
//objdump --start-address=0000 --stop-address=0x3a --source --line-numbers /root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07
//
//Backtrace 3: 0x566f66d595cb "/root/exodus/build/cli/src/gendoc(+0x135cb) [0x566f66d595cb]"
//objdump --start-address=0x13000 --stop-address=0x135cb --source --line-numbers /root/exodus/build/cli/src/gendoc
//
//Backtrace 4: 0x566f66d594e6 "/root/exodus/build/cli/src/gendoc(+0x134e6) [0x566f66d594e6]"
//objdump --start-address=0x13000 --stop-address=0x134e6 --source --line-numbers /root/exodus/build/cli/src/gendoc
//
//Backtrace 5: 0x566f66d53c27 "/root/exodus/build/cli/src/gendoc(+0xdc27) [0x566f66d53c27]"
//objdump --start-address=0xd000 --stop-address=0xdc27 --source --line-numbers /root/exodus/build/cli/src/gendoc
//if (man) {
//
//Backtrace 6: 0x566f66d4baa1 "/root/exodus/build/cli/src/gendoc(+0x5aa1) [0x566f66d4baa1]"
//objdump --start-address=0x5000 --stop-address=0x5aa1 --source --line-numbers /root/exodus/build/cli/src/gendoc
//programexit()
//
//Backtrace 7: 0x7624d822a1ca "/lib/x86_64-linux-gnu/libc.so.6(+0x2a1ca) [0x7624d822a1ca]"
//Backtrace 8: 0x7624d822a28b "/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x8b) [0x7624d822a28b]"
//Backtrace 9: 0x566f66d4b8b5 "/root/exodus/build/cli/src/gendoc(+0x58b5) [0x566f66d4b8b5]"
//objdump --start-address=0x5000 --stop-address=0x58b5 --source --line-numbers /root/exodus/build/cli/src/gendoc

	// TODO autodetect if addr2line or dwalfdump/dSYM is available

	//warning: conversion from ‘std::size_t’ {aka ‘long unsigned int’} to ‘int’ may change value [-Wconversion]
	//char** strings = backtrace_symbols(stack_addresses, static_cast<int>(stack_size));
	auto strings = std::unique_ptr<char*[], void(*)(void*)>{backtrace_symbols(stack_addresses, static_cast<int>(stack_size)), ::free};

	for (std::size_t frameno = 0; frameno < stack_size; frameno++) {

		// To find source lines using objdump we need to know the file name and absolute address in the file

		var objfilename = var(strings[frameno]).field("(", 1).field(" ", 1).field("[", 1);
		var objaddress = var(strings[frameno]).field("[", 2).field("]", 1);

		// Skip libc
//#if TRACING < 2
		if (objfilename.contains("libc.so"))
			continue;
//#endif

#if TRACING
		fprintf(stderr, "Backtrace %d: %p \"%s\"\n", int(frameno), stack_addresses[frameno], strings[frameno]);
		// Backtrace 0: 0x7f9d247cf9fd
		// "/usr/local/lib/libexodus.so.19.01(_ZN6exodus9backtraceEv+0x62) [0x7f9d247cf9fd]"
		// Backtrace 5: 0x7f638280e3f6 "/root/lib/libl1.so(+0xa3f6) [0x7f638280e3f6]"
#endif

		// Use gdb by preference because it finds source file, line numbers AND source line all in one
		static bool gdb = var("which gdb").osshell();
		if (gdb) {
			var objaddress = var(strings[frameno]).field("(", 2).field(")", 1);
			//gdb --batch -ex "file /root/exodus/build/exodus/libexodus.so.24.07" -ex "list *(_ZN3exo13exo_savestackEPPvPm+0x12)"
			//0x124992 is in exo::exo_savestack(void**, unsigned long*) (/root/exodus/exodus/exodebug.cpp:120).
			//115     //      thread_local std::size_t thread_stack_size = 0;
			//116     //}
			//117
			//118     // Capture the current stack addresses for later decoding
			//119     void exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) {
			//120             *stack_size = ::backtrace(stack_addresses, BACKTRACE_MAXADDRESSES);
			//121     #if TRACING
			//122             std::cout << boost::stacktrace::stacktrace();
			//123     #endif
			//124     }
			//07:52:45 root@de1:~/exodus/exodus#
			var stdout;
			var stderr;
			var exit_status;
			let cmd = "gdb --batch -ex \"file " ^ objfilename ^ "\" -ex \"list *(" ^ objaddress ^ ")\"";
//				TRACE(cmd)
			if (var::osprocess(cmd, "", stdout, stderr, exit_status)) {
				let source_file_line = stdout.field(_NL, 1).field("(", -1).field(")", 1);
				let lineno = source_file_line.field(":", 2);
				var col2;
				let sourceline = stdout.substr(stdout.index(_NL ^ lineno ^ "\t") + 1, _NL, col2).cut(lineno.len()).trimfirst(" \t");
				addbacktraceline(frameno, source_file_line, sourceline, returnlines);
				continue;
			}
		}

		// Handle high addresses that are random loaded .so and therefore useless.
		if (objaddress.len() > 9) {
			objaddress = var(strings[frameno]).field("(", 2).field(")", 1);
			if (objaddress.starts("+")) {
				// "/root/lib/liblistsched.so(+0x8067) [0x7fe49722a067]"
				objaddress.cutter(1);
			}
			else {
				// Handle FUNCNAME+RELATIVE address by not adding source line for now.
				// TODO
				// Anything before the + means a relative address and we havent bothered to code for it.
				// /usr/local/lib/libexodus.so.24.07(_ZN3exo10ExoProgram7performERKNS_3varE+0x5d0) [0x7fe49b202710]
				if (not objfilename.contains("libexodus.so")) {
					addbacktraceline(frameno, objfilename.field("/", -1), "", returnlines);
					continue;
				}
			}
		}

		// objdump --stop-address=0xa3f6 -l --disassemble ~/lib/libl1.so |grep cpp|tail -n1

		// Skip libexodus source code
		//if (not exo_debug and objfilename.contains("libexodus.so"))
		//if (objfilename.contains("libexodus.so"))
		//	continue;

//#if TRACING
//		fprintf(stderr, " %s addr:%s\n", objfilename.c_str(), objaddress.c_str());
////		objfilename.errput("objfilename=");
////		objaddress.errputl(" objaddress=");
//#endif

		// if (objfilename == objfilename.convert("/\\:",""))
//TRACE(objfilename)      // "/root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07"
//TRACE(objfilename)      // "/root/exodus/build/cli/src/gendoc"

		if (not objfilename.osfile()) {
			// loadable program
			var which_out;
			if (not which_out.osshellread("which " ^ objfilename.field(_OSSLASH, -1))) {
				//null
			}
			which_out = which_out.field("\n", 1).field("\r", 1);
			if (which_out)
				objfilename = which_out;
			else
				// things like what?
				continue;
		}

		// Skip if hex address is invalid
		if (not objaddress.starts("0") || objaddress.at(2) != "x") {
			mangled_addresses.appender(objaddress, FM_);
			let linetext = var(frameno) ^ ": " ^ objfilename.field(_OSSLASH, -1) ^ ":" ^ objaddress;
			returnlines ^= linetext ^ FM;
			continue;
		}

		// Start at the beginning of a hex block
		let startaddress = objaddress.paste(-3, 3, "000");
//		let stopaddress = objaddress;
//		let startaddress = objaddress;

//		let startaddress = objaddress;
		// Stop 1 after after the start address
		let stopaddress = "0x" ^ (objaddress.cut(2).iconv("MX") + 1).oconv("MX");

		////////////////////////
		//objdump_out = objdump_out.osshellread();
		var objdump_out;
		let cmd =
			"objdump"
			" --start-address=" ^ startaddress ^
			" --stop-address=" ^ stopaddress ^
//			" --disassemble -l " ^ objfilename ^
			" --source"
			" --line-numbers " ^
			objfilename;

#if TRACING
		cmd.errputl();
#endif
		if (not objdump_out.osshellread(cmd))
			continue;
		////////////////////////

		objdump_out.converter("\r\n", _FM _FM);

		// Find the last line containing .cpp
		/// root/exodus/exodus/libexodus/exodus/l1.cpp:7 (discriminator 3)
		const var nn2 = objdump_out.fcount(FM);
		var line = "";
		let linesource = "";
		//for (var frameno2 = 1; frameno2 < nn2; ++frameno2) {
		for (var frameno2 : reverse_range(1, nn2)) {
			if (objdump_out.f(frameno2).contains(".cpp:")) {
//				let nextline = objdump_out.f(frameno2);
//				// Skip disassembly lines
//				if (not nextline.match("    [0-9a-f]: ")) {
//					// Skip lines like "return 1;" since they seem to be spurious
//					if (nextline.match("^\\s*return\\s*\\d+;"))
//						break;
					line = objdump_out.f(frameno2);
//					linesource = nextline;
//#if TRACING
//					TRACE(line)
//					TRACE(linesource)
//#endif
					break;
//				}
			}
		}

//#if TRACING
//		if (line)
//			line.errputl("line=");
//#endif

		// Skip library internals
		if (objfilename.contains("libexodus.so"))
			continue;

		// append the source line text and number to the output
		if (line) {
			let sourcefilename = line.field(":", 1);
			let lineno = line.field(":", 2).field(" ", 1);
			addbacktraceline(frameno, sourcefilename, lineno, returnlines);
			//7: p1.cpp:13: return 1;
			//returnlines ^= var(frameno + 1) ^ ": " ^ sourcefilename ^ ":" ^ lineno ^ ": " ^ linesource ^ FM;
			//returnlines ^= var(frameno + 1) ^ ": " ^ line.field2(_OSSLASH, -1) ^ " " ^ linesource ^ FM;
		} else if (not objfilename.contains("libexodus.so")) {
			addbacktraceline(frameno, objfilename.field("/", -1), "", returnlines);
		}

#if TRACING
		var("").errputl();
#endif
		// Quit if reached limit of desired backtrace
		if (limit && nlines++ >= limit)
			break;

	}

	if (mangled_addresses) {
		var stdout;
		var stderr;
		var exit_status;
		if (var::osprocess("c++filt", mangled_addresses.convert(_FM, _NL), stdout, stderr, exit_status)) {
			var ii = 0;
			stdout.converter(_NL, _FM);
			mangled_addresses.trimmerlast(_FM);
			for (var mangled_address : mangled_addresses)
				returnlines.replacer(mangled_address, stdout.f(++ii));
		}
	}

	// Handled automatically by unique_ptr
	//::free(strings);

	returnlines.popper();
	return returnlines;

}

/* Possible plan to extend exodus signal handling
	SIGHUP
	Reload config. Same as issuing the command RELOAD on the console.
	SIGTERM
	Super safe shutdown. Wait for all existing clients to disconnect, but don’t accept new connections. This is the same as issuing SHUTDOWN WAIT_FOR_CLIENTS on the console. If this signal is received while there is already a shutdown in progress, then an “immediate shutdown” is triggered instead of a “super safe shutdown”. In PgBouncer versions earlier than 1.23.0, this signal would cause an “immediate shutdown”.
	SIGINT
	Safe shutdown. Same as issuing SHUTDOWN WAIT_FOR_SERVERS on the console. If this signal is received while there is already a shutdown in progress, then an “immediate shutdown” is triggered instead of a “safe shutdown”.
	SIGQUIT
	Immediate shutdown. Same as issuing SHUTDOWN on the console.
	SIGUSR1
	Same as issuing PAUSE on the console.
	SIGUSR2
	Same as issuing RESUME on the console.
*/
//service managers like systemd will send a polite SIGTERM signal
//and wait for say 90 seconds before sending a kill signal
static void SIGTERM_handler(int) {
	fprintf(stderr, "=== SIGTERM_handler: Set TERMINATE_req = true ===\n");
	TERMINATE_req = true;
}

//restart
static void SIGHUP_handler(int) {
	fprintf(stderr, "=== SIGHUP_handler: Set RELOAD_req = true ===\n");
	RELOAD_req = true;
}

PUBLIC void breakoff() {
	// ignore this signal (Typically Ctrl+C) until breakon
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

		var cmd = "A";

		if (var().isterminal()) {
			fprintf(stderr, "Interrupted. (C)ontinue (Q)uit (A)bort (B)acktrace (D)ebug ? ");
			fflush(stderr);
			if (!cmd.inputn(1))
				continue;
		}

		// only look at first character in uppercase
		let cmd1 = var(cmd.first()).ucase();

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
			var(err.stack()).convert(FM, "\n").errputl();

		} else if (cmd1 == "D") {

			fprintf(stderr, "ebugging ... ");
			fflush(stderr);			// duplicated in init and B

			// Debugging
			// duplicated in init and B
			let pid = getpid();
			let cmd = "gdb -p " ^ pid;
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

PUBLIC void debug(in var1) {

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
	//UNLIKELY throw VarDebug(var1);

// another way to break into the debugger by causing a seg fault
#elif 0
	*(int*)0 = 0;
#endif

	return;
}

}  // namespace exo
