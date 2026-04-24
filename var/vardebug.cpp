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

#if EXO_MODULE > 1
	import std;
#else
#	include <iostream>
#	include <fstream>
#	include <string>
#	include <memory>   // for make_unique
#	include <cstdio>
#	include <cstdlib>
#	include <csignal>
#	include <unistd.h> // for getpid
#endif

#include <stdio.h>     // for stderr etc.
#include <termios.h>
#include <execinfo.h>  // for backtrace

#include <var/var.h>
//#include <exodus/exoimpl.h>
#include <var/range.h>
#include <var/vardebug.h>

namespace exo {

using let = const var;

// In varposix.cpp
ND PUBLIC var getexecpath();

// TODO move to varposix.cpp?
bool in_debugger() {
	std::ifstream status("/proc/self/status");
	std::string line;

	while (std::getline(status, line)) {
		if (line.rfind("TracerPid:", 0) == 0) {
			int tracerPid = std::stoi(line.substr(10));
			return tracerPid != 0;
		}
	}
	return false;
}

static void addbacktraceline(in frameno, in sourcefilename, in lineno, in exo_debug, io returnlines) {

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

	if (exo_debug < 2) {

		// Suppress confusing and unhelpful exodus macros like programinit/exit, libraryinit/exit, classinit/exit
		if (/* not raw and */ line.match("^(program|library|class)(init|exit)\\("))
			return;

		// Skip "return nn"
		if (line.match("^\\s*return\\s*\\d*") and not raw)
			return;
	}

#if TRACING
	line.errputl();
#endif

	// Example output line:
	// "8: p2.cpp:15: printl(v2);"
	let linetext = (frameno) ^ ": " ^ sourcefilename.field(_OSSLASH, -1) ^ ":" ^ lineno ^ ":\t" ^ line;
	//var linetext = std::format("{:0}: {:1}:{:2}: {:3}" , frameno, sourcefilename.field2(_OSSLASH, -1), lineno, line);

	returnlines ^= linetext ^ FM;

	return;
}

//#if SAVESTACK_ATTACHES_DEBUGGER
//var summarise_debugger_bt(in osfilename) {
//
//	var result = "";
//	var currthreadno = "";
//	var currthreadtitle = "";
//	var txt;
//	if (not txt.osread(osfilename))
//		return "";
//	txt.converter(NL, FM);
//	for (var line : txt) {
////			TRACE(line);
//		if (line.starts("Thread ")) {
//			let threadno = line.field(" ", 2);
//			if (threadno > 1 or currthreadno)
//				currthreadtitle = line.field(" ", 1, 2);
//			currthreadno = threadno;
//		}
//		if (line.starts("#")) {
//
//			// e.g.
//			// #12 _ExoProgram::main3 (this=this@entry=0x7ffc342fdb00, call_id=..., N=..., mode=...) at /root/exodus/exodus/t1.cpp:75 {
//			//
//			//#10 0x00007fa494b44a9b in _ExoProgram::main (this=<optimized out>) at exo/waiting.cpp:56
//			// TODO also find sub_ExoProgram
//			if (not line.trim().field(" ", 2).contains("_ExoProgram") and not line.contains("in _ExoProgram"))
//				continue;
//
//			var srcfileline = line.field(" ", -1);
//			var srcfileline2 = srcfileline.field("/", -2, 2);
////				if (srcfileline2.starts("exodus/"))
////					continue;
////				TRACE(srcfileline)
//			if (srcfileline.field("/", -1).count(":") == 1) {
//
//				// Try to resolve relative path by lookup in ~/inc
//				if (not srcfileline.starts("/")) {
//					//exo/waiting.cpp:56
//					var incfilename = "~/inc/" ^ srcfileline.field("/", -1).field(":", 1).field(".", 1) ^ ".h";
//					var incfile;
//					if (incfile.osread(incfilename) or incfile.osread(incfilename.paste(-1, 1, "H"))) {
//						// //Generated by exodus "compile /root/exodus/service/src/exo/waiting.cpp"
//						var target = incfile.field("\"", 2).field(" ", 2).trim();
//						if (not target.starts("/"))
//							target = incfile.match(R"___([^/]/[a-zA-Z0-9_./]+)___").f(1, 1).cut(1);
//						if (target.starts("/"))
//							srcfileline = target ^ ":" ^ srcfileline.field(":", 2);
//					}
//				}
//
//				// Skip #11 inc/l1.h:43 return CALLMEMBERFUNCTION(*(this->plibobject_),
//				if (srcfileline.contains("/inc/"))
//					continue;
//
//				var src;
//				if (not src.osread(srcfileline.field(":",1)))
//					continue;
//				src = src.field(NL, srcfileline.field(":",2));
//				line = line.field(" ", 1) ^ " " ^ srcfileline2 ^ "\t" ^ src.trimboth(" \t");
//				if (currthreadtitle) {
//					// Thread 1 is usually last.
//					// and we dont output it unless other threads already output.
//					currthreadtitle.errputl();
//					currthreadtitle = "";
//				}
//				result.appender(line, _NL);
//			}
//		}
//	}
//	result.popper();
//	return result;
//}
//#endif // SAVESTACK_ATTACHES_DEBUGGER

// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
// Emulate Ctrl+C or otherwise interrupt execution.
// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
auto raise_SIGTRAP() -> void {
#if __has_include(<csignal>)
	std::raise(SIGTRAP);
#elif 1
	__asm__("int3");
// another way to break into the debugger by causing a seg fault
#elif 0
	*(int*)0 = 0;
#endif
}

// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
// Open or attach and open a debugger.
// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
auto open_debugger() -> void {

	// Use already-attached debugger if available.
	// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	if (in_debugger()) {
		raise_SIGTRAP();
		return;
	}

	// Prepare debugger config files.
	// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	// Patch ./lldbinit or .gdbinit.
	// Suppress long list of system file symbol loading
	{
		bool lldb = true;
		let initfile = lldb ? "~/.lldbinit" : "~/.gdbinit";
		var conftext;
		if (not conftext.osread(initfile))
			conftext = "";
		if (lldb) {
			//
			// ~./lldbinit - TODO!
			// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
			var v1 = R"!(
				# Quiet symbol loading
				settings set symbols.load-on-demand true
				settings set target.preload-symbols false

				# Disable debuginfod / external symbol lookups
				settings set symbols.enable-external-lookup false
				settings set plugin.symbol-locator.debuginfod.server-urls ""

				# Suppress SIGCHLD and SIGTRAP silently (no "2" lines)
				script ci = lldb.debugger.GetCommandInterpreter(); res = lldb.SBCommandReturnObject(); ci.HandleCommand("process handle -n false -s false SIGCHLD", res); ci.HandleCommand("process handle -n false -s true SIGTRAP", res)

				# Hide disassembly when stopping on SIGTRAP (keeps your intentional debug trap clean)
				settings set stop-disassembly-display never

				# Other quiet / convenience settings
				settings set target.process.stop-on-exec false
				settings set auto-confirm true
				settings set target.load-cwd-lldbinit true
			)!"_heredoc;
		} else {
			//
			// ~./gdbinit
			// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
			// set print symbol-loading off
			// set print inferior-events off
			// set debuginfod enabled off
			for (let code : "symbol-loading^inferior-events^debuginfod"_var) {
				if (not conftext.contains(code))
					conftext.appender("set print ", code, " off\n");
			}
		}
		let conftext_orig = conftext;
		if (conftext != conftext_orig) {
			if (conftext.oswrite(initfile))
				var(initfile ^ " updated.").errputl();
			else
				var::loglasterror();
		}
	}

	// Determine debugger command
	// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
	let attach_debugger0 = [](){

		// Get EXO_DEBUGGER env once.
		// Default to lldb if compiler is clang otherwise gdb
		static let exo_debugger = [](){
			var v1;
			if (v1.osgetenv("EXO_DEBUGGER")) {
			};
			return v1 ?:
#ifdef __clang__
					"lldb";
#else
					"gdb";
#endif
		}();
		// Slightly different commands to attach lldb and gdb
		if (exo_debugger.starts("lldb"))
			return exo_debugger ^ " -Q -p " ^ var::ospid() ^ " " ^ getexecpath();
		else
			return exo_debugger ^ " -q -p " ^ var::ospid();
	}();

	// Attach interactive debugger
	// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
//	attach_debugger0.errputl();
	let attach_debugger = "bash -c 'sleep 0.25 && " ^ attach_debugger0 ^ "'";
	// Use std::system instead of var::osshell to avoid additional stack entries?
	auto ec = std::system(attach_debugger);
	if (ec != 0) {
		attach_debugger.errputl();
		var(strerror(errno)).errputl();
	}
	return;
}

// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
// Capture stack. Display source backtrace and maybe drop into debugger.
// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size, const std::string& context_msg) -> void {

	// Get the low-level stack addresses.
	*stack_size = ::backtrace(stack_addresses, BACKTRACE_MAXADDRESSES);

	// Pure stack collection to allow backtrace without actual VarError or debugging
	if (context_msg == "Stack-Only")
		return;

	// EXO_DEBUG=1,2,3 signifies desire to attach debugger is not already debugging.
	// Otherwise VarError should use standard handler or try/catch
	// EXO_DEBUG=0 suppresses backtracing.

	// Get EXO_DEBUG env once.
	static let exo_debug = [](){var v1; if (v1.osgetenv("EXO_DEBUG")){}; return v1;}();

	// Detect if debugger attached
	auto debugging = in_debugger();

	// Suppress backtracing if EXO_DEBUG=0.
	// Use normal handler or try/catch if present.
	// Useful to speed up testing of VarError exceptions that use try/catch but do not need stack tracing.
	if (exo_debug == 0)
		return;

//#if SAVESTACK_ATTACHES_DEBUGGER
//
//#if TRACING
//	std::cerr << "Attaching GDB to PID: " << getpid() << "\n";
//#endif
//	std::string pid = std::to_string(getpid());
//
////	std::string gdb_cmd = "gdb -batch -p " + pid + " -ex 'thread apply all bt full' -ex 'detach' -ex 'quit' > backtrace." + pid + ".log 2>/dev/null";
//
//	if (not "which gdb") {
//		std::cerr << "----- gdb not installed/available. -----" << std::endl;
//		return;
//	}
//
//	let logfilename = "backtrace." ^ var::ospid() ^ ".log";
//	std::cerr << "----- gdb " << logfilename << " -----" << std::endl;
//
//	// Get all thread info because there is no gdb command to switch to thread (LWP) by LWP id.
//	var gdb_cmds = " -ex 'thread apply all bt' -ex 'detach' -ex 'quit'";
//	var gdb_cmd = "gdb -batch -p " ^ (pid ^ gdb_cmds) ^ " > backtrace." ^ pid ^ ".log 2>/dev/null < /dev/null";
//
//	if (std::system(gdb_cmd.c_str()) != 0) {
//		// gdb not installed. Rely on ::backtrace above which isnt very accurate in threads or fibers.
//		return;
//	}
////	if (not gdb_cmd.osshell()) {
////		var::lasterror().errputl();
////		return;
////	}
//
//	var txt;
//	if (not txt.osread(logfilename)) {
//		// gdb not installed. Rely on ::backtrace above which isnt very accurate in threads or fibers.
////		std::cerr << "Could not attach gdb" << std::endl;
//		return;
//	}
//
//	// If running under gdb or gdb already attached then we cannot attach again.
//	if (not txt.contains("#0")) {
//		std::clog << "gdb is already attached to pid " << var::ospid() << ". Search backtrace starting ExoProgram." << std::endl;
//		// Could not attach to process.  If your uid matches the uid of the target
//		// process, check the setting of /proc/sys/kernel/yama/ptrace_scope, or try
//		// again as the root user.  For more details, see /etc/sysctl.d/10-ptrace.conf
//		return;
//	}
//
//#endif // SAVESTACK_ATTACHES_DEBUGGER

	// If no debugger, and no debugging requested.
	// quit and use normal handler or try/catch.
	if (not exo_debug and not debugging)
		return;

	// Display backtrace - source lines
	// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
//	std::cout << std::flush;
	exo_backtrace(stack_addresses, *stack_size).errputl();
	var(context_msg).errputl();
	var("⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯").errputl();
	//	var("-").str(32).errputl();

	open_debugger();
}

//}

// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
// Given stack addresses, get the source file, line no and line text
// ⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯
// http://www.delorie.com/gnu/docs/glibc/libc_665.html
auto exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit) -> var {
//
//#if SAVESTACK_ATTACHES_DEBUGGER
//	// Utilise debugger backtrace.$pid.log if available
//	var returnlines = summarise_debugger_bt("backtrace." ^ var::ospid() ^ ".log");
//	if (returnlines)
//		return returnlines;
//#else
	var returnlines = "";
//#endif

	std::size_t nlines = 0;
	var mangled_addresses = "";

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

	// 1=normal debugging, 2=exodus lib debugging, 3=std lib debugging)
	var exo_debug;
	if (exo_debug.osgetenv("EXO_DEBUG")) {};

	//warning: conversion from ‘std::size_t’ {aka ‘long unsigned int’} to ‘int’ may change value [-Wconversion]
	//char** strings = backtrace_symbols(stack_addresses, static_cast<int>(stack_size));
	auto strings = std::unique_ptr<char*[], void(*)(void*)>{backtrace_symbols(stack_addresses, static_cast<int>(stack_size)), std::free};

	for (std::size_t frameno = 0; frameno < stack_size; frameno++) {

		// To find source lines using objdump we need to know the file name and absolute address in the file

		var objfilename = var(strings[frameno]).field("(", 1).field(" ", 1).field("[", 1);
		var objaddress = var(strings[frameno]).field("[", 2).field("]", 1);

#if TRACING
		std::fprintf(stderr, "Backtrace %d: %p \"%s\"\n", int(frameno), stack_addresses[frameno], strings[frameno]);
		// Backtrace 0: 0x7f9d247cf9fd
		// "/usr/local/lib/libexodus.so.19.01(_ZN6exodus9backtraceEv+0x62) [0x7f9d247cf9fd]"
		// Backtrace 5: 0x7f638280e3f6 "/root/lib/libl1.so(+0xa3f6) [0x7f638280e3f6]"
#endif

		// 0, 1=normal debugging, 2=exodus lib debugging, 3=std lib debugging)
		auto skippable = [exo_debug](in osfilename) -> bool {
			if (osfilename.starts("/usr/local") and exo_debug < 2) {
				// Skip exodus libs
//var("SKIP " ^ osfilename).errputl();
				return true;
			}
			if ((osfilename.starts("/lib/") || osfilename.starts("/usr/lib/") || osfilename.empty()) and exo_debug < 3) {
				// Skip system libs
//var("SKIP " ^ osfilename).errputl();
				return true;
			}
//var("KEEP " ^ osfilename).errputl();
			return false;
		};

		// Skip various files
		if (skippable(objfilename))
			continue;

		var reladdress = var(strings[frameno]).field("(", 2).field(")", 1);

		// Use gdb by preference because it finds source file, line numbers AND source line all in one
		// BUT it resolves inlined code wrongly unless attached at the stack capture site (which is slow)
		// whereas objdump scans backwards for the actual .cpp line.
		static bool gdb = var("which gdb > /dev/null").osshell();
		auto gdb2source = [&exo_debug, &skippable](in frameno, in objfilename, in reladdress, io returnlines) -> bool {
			//gdb --batch -ex "file /root/exodus/build/exodus/libexodus.so.24.07" -ex "list *(_ZN3exo13exo_savestackEPPvPm+0x12)"
			//0x124992 is in exo::exo_savestack(void**, unsigned long*) (/root/exodus/exodus/vardebug.cpp:120).
			//115	 //	  thread_local std::size_t thread_stack_size = 0;
			//116	 //}
			//117
			//118	 // Capture the current stack addresses for later decoding
			//119	 void exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) {
			//120			 *stack_size = ::backtrace(stack_addresses, BACKTRACE_MAXADDRESSES);
			//121	 #if TRACING
			//122			 std::cout << boost::stacktrace::stacktrace();
			//123	 #endif
			//124	 }
			//07:52:45 root@de1:~/exodus/exodus#
			var gdb_out;
			let cmd = "gdb --batch -ex \"file " ^ objfilename ^ "\" -ex \"list *(" ^ reladdress ^ ")\"";
			if (not gdb_out.osshellread(cmd))
				return false;

			let source_file_line = gdb_out.field(_NL, 1).field("(", -1).field(")", 1);
			let lineno = source_file_line.field(":", 2);

			// Skip various files
			if (skippable(source_file_line))
				return false;

			var col2;
			// Extract the desired line starting with the desired line_no
			let sourceline = gdb_out.substr(gdb_out.index(_NL ^ lineno ^ "\t") + 1, _NL, col2).cut(lineno.len()).trimfirst(" \t");
//				addbacktraceline(frameno, source_file_line, sourceline, exo_debug, returnlines);
			returnlines ^= var(frameno) ^ ": " ^ source_file_line ^ ":\t" ^ sourceline ^ FM;

			return true;
		};

		// Handle high addresses that are random loaded .so
		// and they cannot be discovered by objdump
		if (objaddress.len() > 9) {
			objaddress = var(strings[frameno]).field("(", 2).field(")", 1);
			if (objaddress.starts("+")) {
				// "/root/lib/liblistsched.so(+0x8067) [0x7fe49722a067]"
				objaddress.cutter(1);
			}
			else {

				// Handle FUNCNAME+RELATIVE address
				// Anything before the + means a relative address
				// /usr/local/lib/libexodus.so.24.07(_ZN3exo10ExoProgram7performERKNS_3varE+0x5d0) [0x7fe49b202710]
				if (gdb && gdb2source(frameno, objfilename, reladdress, returnlines))
					continue;
				continue;
			}
		}

		// objdump --stop-address=0xa3f6 -l --disassemble ~/lib/libl1.so |grep cpp|tail -n1

		// Skip libexodus source code
		//if (not exo_debug and objfilename.contains("libexodus.so"))
		//if (objfilename.contains("libexodus.so"))
		//	continue;

//#if TRACING
//		std::fprintf(stderr, " %s addr:%s\n", objfilename.c_str(), objaddress.c_str());
////		objfilename.errput("objfilename=");
////		objaddress.errputl(" objaddress=");
//#endif

		// if (objfilename == objfilename.convert("/\\:",""))
//TRACE(objfilename)	  // "/root/exodus/build/exodus/libexodus/exodus/libexodus.so.24.07"
//TRACE(objfilename)	  // "/root/exodus/build/cli/src/gendoc"

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
		// Find previous .cpp or .c
		for (var fn : reverse_range(1, nn2)) {
			if (objdump_out.f(fn).match("\\.cpp:|\\.c:")) {
					line = objdump_out.f(fn);
					break;
			}
		}
		// Find previous .h if no .cpp can be found
		if (not line) {
			for (var fn : reverse_range(1, nn2)) {
				if (objdump_out.f(fn).contains(".h:")) {
						line = objdump_out.f(fn);
						break;
				}
			}
		}

		// Skip various files
		if (skippable(line))
			continue;

		// append the source line text and number to the output
		if (line) {
			let sourcefilename = line.field(":", 1);
			let lineno = line.field(":", 2).field(" ", 1);
			addbacktraceline(frameno, sourcefilename, lineno, exo_debug, returnlines);
			//7: p1.cpp:13: return 1;
			//returnlines ^= var(frameno + 1) ^ ": " ^ sourcefilename ^ ":" ^ lineno ^ ": " ^ linesource ^ FM;
			//returnlines ^= var(frameno + 1) ^ ": " ^ line.field2(_OSSLASH, -1) ^ " " ^ linesource ^ FM;
		} else {
			addbacktraceline(frameno, objfilename.field("/", -1), "", exo_debug, returnlines);
		}

#if TRACING
		var("").errputl();
#endif
		// Quit if reached limit of desired backtrace
		if (limit && nlines++ >= limit)
			break;

	} // frameno

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
	returnlines.converter(_FM, _NL);
	return returnlines;

}

// Terminate - Signal handler
static void SIGTERM_handler(int) {
	std::fprintf(stderr, "=== SIGTERM_handler: Set TERMINATE_req = true ===\n");
	TERMINATE_req = true;
}

// Restart - signal handler
static void SIGHUP_handler(int) {
	std::fprintf(stderr, "=== SIGHUP_handler: Set RELOAD_req = true ===\n");
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

	// sig 0 means called to open debugger

	// Save stdin terminal attributes
	// Probably not available if running as a service
	struct termios oldtio, curtio;
	bool has_oldtio = false;
	if (tcgetattr(STDIN_FILENO, &oldtio) < 0) {
		//EBADF - The filedes argument is not a valid file descriptor.
		//ENOTTY - The filedes is not associated with a terminal.
		//	  var("no std input").outputl();
		//	  return false;
	} else {
		has_oldtio = true;
//		nfds = 2;

		//Make sure we exit cleanly
		// memset(&sa, 0, sizeof(struct sigaction));
		// sa.sa_handler = sighandler;
		// sigaction(SIGINT, &sa, nullptr);
		// sigaction(SIGQUIT, &sa, nullptr);
		// sigaction(SIGTERM, &sa, nullptr);

		//This is needed to be able to tcsetattr() after a hangup (Ctrl-C)
		//see tcsetattr() on POSIX

		// memset(&sa, 0, sizeof(struct sigaction));
		// sa.sa_handler = SIG_IGN;
		// sigaction(SIGTTOU, &sa, nullptr);

		// Set stdin mode
		// a) non-canonical (i.e. characterwise not linewise input)
		// b) no-echo
		// https://man7.org/linux/man-pages/man3/termios.3.html
		//
		tcgetattr(STDIN_FILENO, &curtio);
		curtio.c_lflag |= (ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &curtio);
	}

	// Ignore more of this signal - restore on exit
//	breakoff();
	// Faster/safer?
	signal(sig, SIG_IGN);

	// Initially break straight into debugger
	// a) If called with sig 0
	// b) if EXO_DEBUG not "" or 0
	thread_local let exo_debug = [](){var v1; if (v1.osgetenv("EXO_DEBUG")){}; return v1;}();
	if (!sig || exo_debug)
		open_debugger();

	std::fprintf(stderr, "\n");
	while (true) {

		// (A)bort if no terminal for input.
		var cmd = "A";

//		// (R)esume if no terminal for input.
//		var cmd = "R";

		// Prompt and wait for one char input. Enter = repeat.
		if (var().isterminal()) {
			var().echo(false);
			std::fprintf(stderr, "Interrupted. (R)esume (q)uit (a)bort (b)acktrace (d)ebug ? ");
			fflush(stderr);
			if (!cmd.inputn(1) || cmd == "\r" || cmd == "\n")
				cmd = "R";
			var().echo(true);
		}
		// Uppercase first char
		let cmd1 = var(cmd.first()).ucase();

		if (cmd1 == "R") {

			// (R)esume
			std::fprintf(stderr, "Resuming ...\n");
			fflush(stderr);

			// Exit the SIGINT_handler - Return to and resume interrupted code.
			break;

		} else if (cmd1 == "Q") {

			// (Q)uit
			std::fprintf(stderr, "Quitting ...\n");

			// Exit the program with errcode 1.
			std::exit(1);

		} else if (cmd1 == "B") {

			// (B)acktrace
			std::fprintf(stderr, "Backtracing ...\n");
			fflush(stderr);

			// Display the current stack source code lines.
			VarError err("Stack-Only");
			var(err.stack()).errputl();

		} else if (cmd1 == "D") {

			// (D)ebug
			std::fprintf(stderr, "Debugging ... ");
			fflush(stderr);

			// Attach or open the debugger.
			breakon();
			var().echo(1);
			open_debugger();

		} else if (cmd1 == "A") {

			std::fprintf(stderr, "Aborting ...\n");
			fflush(stderr);			// duplicated in init and B

			// Abort using SIGABT
			// std::fprintf(stderr,"To continue from here in GDB: \"signal 0\"\n");
			raise(SIGABRT);
			// breakpoint();

		} else {

			// Invalid option - try again.
			std::fprintf(stderr, "Invalid option. Try again.\n");
		}

	}

	// Restore terminal attributes (probably linewise input and echo)
//	if (nfds == 2) {
	if (has_oldtio)
		tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
//	}
	// Show input characters
	//var().echo(true);

	// Stop ignoring this signal
//	breakon();
	// faster/safer?
//	std::fprintf(stderr, "reestablish SIGINT_handler\n");
	signal(SIGINT, SIGINT_handler);

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

ND PUBLIC auto backtrace() -> var {
	VarError e("Stack-Only");
	return e.stack();
}

PUBLIC auto debug(var condition) -> void {

	// Option to skip
	if (!condition)
		return;

	std::cout << std::flush;
	if (condition != INT_MAX)
		std::clog << "debug(" << condition << ")" << std::endl;

	// Signal 0 phantom to open_debugger()
	SIGINT_handler(0);

	return;
}

}  // namespace exo
