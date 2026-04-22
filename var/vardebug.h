#ifndef EXODUS_EXODEBUG_H
#define EXODUS_EXODEBUG_H

// || (EXO_MODULE && LIBEXODUS_EXODUS_H_)
#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#endif

#include <var/vardefs.h>

namespace exo {

	// Capture the current stack.
	//
	// Note EXO_DEBUG = 0, 1, 2 = backtrace exodus libs, 3 = backtrace all.
	//
#	define SAVESTACK_ATTACHES_DEBUGGER 0
#	if SAVESTACK_ATTACHES_DEBUGGER
		// Returns false to indicates that gdb is available but already attached and caller can throw to get into gdb.
		//
		// Always get the basic stack addresses because it is quick
		// however is it not very accurate especially inside threads and fibers
		// so use gdb to get accurate backtrace in backtrace.$pid.log
		//
		// Use EXO_DEBUG=0 to suppress using gdb for backtracing
		// e.g. to speed up programs like test_exception which
		// deliberately trigger various VarError conditions
		// but use try/catch and dont need backtracing.
		//
#	endif
	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size, const std::string& context_msg) -> void;

	// Generate backtrace text including source lines
	PUBLIC auto exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit = 0) -> std::string;

	// Enable and disable SIGINT handler - Ctrl+C
	PUBLIC void breakon();
	PUBLIC void breakoff();

	// Get backtrace text for the current program execution location
 ND PUBLIC auto backtrace() -> std::string;

	// Trigger trace/breakpoint trap - TODO display stack and attach debugger if not already present, and in a terminal.
	PUBLIC void debug(std::string_view sv1 = "");

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
