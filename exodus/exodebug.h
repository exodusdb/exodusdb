#ifndef EXODUS_EXODEBUG_H
#define EXODUS_EXODEBUG_H

#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#endif

#include "vardefs.h"

namespace exo {

	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) -> bool;
	PUBLIC auto exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit) -> std::string;
	PUBLIC void breakon();
	PUBLIC void breakoff();
 ND PUBLIC auto backtrace() -> std::string;
	PUBLIC void debug(std::string_view sv1 = "");

}
#endif // EXODUS_EXODEBUG_H
