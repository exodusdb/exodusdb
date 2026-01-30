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

	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) -> bool;
	PUBLIC auto exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit = 0) -> std::string;
	PUBLIC void breakon();
	PUBLIC void breakoff();
 ND PUBLIC auto backtrace() -> std::string;
	PUBLIC void debug(std::string_view sv1 = "");

//	PUBLIC void debug(in = "");
//	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) -> bool;
// ND PUBLIC auto exo_backtrace( void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit = 0) -> std::string;

	// Set by signals for threads to poll
	PUBLIC inline bool TERMINATE_req = false;
	PUBLIC inline bool RELOAD_req = false;

}

#endif // EXODUS_EXODEBUG_H
