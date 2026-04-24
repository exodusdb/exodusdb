#ifndef EXODUS_EXODEBUG_H
#define EXODUS_EXODEBUG_H

// || (EXO_MODULE && LIBEXODUS_EXODUS_H_)
#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#endif

#include <var.h>
//#include <var/vardefs.h>

namespace exo {

	using SV = std::string_view;

	// Capture the current stack.
	//
	// EXO_DEBUG = controls how VarError exceptions are handled.
	//
	// Not set or ""
	//     Backtrace to stderr then use the VarError exception handler.
	//     VarError is the parent of VarUnassigned and many others.
	//
	//     The standard ExoProgram VarError exception handler terminates execution or the current perform/execute()
	//     Try/catch VarError can be used to override that.
	//     Backtrace is quite slow and can take several seconds.
	//
	// 0 = Same as above without backtrace.
	//
	//     Useful if using try/catch to handle VarError exceptions and you want speed and no backtrace output. Used in test_exception.cpp
	//
	// 1 = Backtrace to stderr and break into debugger.
	//
	//     Attach debugger if not already attached.
	//     Backtrace only application code, ignoring exodus and system libs.
	//
	// 2 = Same but also backtrace exodus lib source lines
	//
	// 3 = Same but also backtrace system lib source lines
	//
	//#	define SAVESTACK_ATTACHES_DEBUGGER 0
	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size, const std::string& context_msg) -> void;

	// Generate backtrace text including source lines
	PUBLIC auto exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit = 0) -> var;

/////////////////
///// Debugging :
/////////////////

	// obj is none

	// Get stack backtrace text for the current program execution location.
 ND PUBLIC auto backtrace() -> var;

	// Display stack backtrace text and attach debugger.
	// The debugger can be used to inspect the current program internals.
	// Quitting the debugger causes the program to resume execution.
	// The debugger defaults to lldb if exodus was built with clang or gdb if built with g++. The enviroment variable EXO_DEBUGGER can be set to override the default.
	// Access to the debugger and backtrace can also be triggered by Ctrl+C in terminal mode as long as breakoff() is not active.
	// Ctrl+C: "Interrupted. (R)esume (Q)uit (A)bort (B)acktrace (D)ebug ?"
	// condition: If set to "" or 0 then debug will be skipped. This allows for conditional debugging.
	// EXO_DEBUG: An environment variable that causes automatic attachment of debugger on VarError exception.
	// * 0 * Suppress output of backtrace on VarError. Useful for speed if using try/catch to handle VarError exceptions.
	// * 1 * Attach debugger on VarError exception.
	// * 2 * Ditto but backtrace exodus library functions too.
	// * 3 * Ditto but backtrace system library functions too.
	PUBLIC auto debug(var condition = INT_MAX) -> void;

	// Enable Ctrl+C/SIGINT interrupt.
	PUBLIC auto breakon() -> void;

	// Disable Ctrl+C/SIGINT interrupt.
	PUBLIC auto breakoff() -> void;

	// Globals set by signals for threads to poll
	//
//	Possible plan to extend exodus signal handling
//	SIGHUP
//	Reload config. Same as issuing the command RELOAD on the console.
//	SIGTERM
//	Super safe shutdown. Wait for all existing clients to disconnect, but don’t accept new connections. This is the same as issuing SHUTDOWN WAIT_FOR_CLIENTS on the console. If this signal is received while there is already a shutdown in progress, then an “immediate shutdown” is triggered instead of a “super safe shutdown”. In PgBouncer versions earlier than 1.23.0, this signal would cause an “immediate shutdown”.
//	SIGINT
//	Safe shutdown. Same as issuing SHUTDOWN WAIT_FOR_SERVERS on the console. If this signal is received while there is already a shutdown in progress, then an “immediate shutdown” is triggered instead of a “safe shutdown”.
//	SIGQUIT
//	Immediate shutdown. Same as issuing SHUTDOWN on the console.
//	SIGUSR1
//	Same as issuing PAUSE on the console.
//	SIGUSR2
//	Same as issuing RESUME on the console.
	//
	// Service managers like systemd will send a polite SIGTERM signal
	// and wait for say 90 seconds before sending a kill signal
	//
	PUBLIC inline bool TERMINATE_req = false;
	PUBLIC inline bool RELOAD_req = false;

}

#endif // EXODUS_EXODEBUG_H
