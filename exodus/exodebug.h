#ifndef EXODUS_EXODEBUG_H
#define EXODUS_EXODEBUG_H
	bool exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size);
	var exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit);
	PUBLIC void breakon();
	PUBLIC void breakoff();
	ND PUBLIC var backtrace();
	PUBLIC void debug(in var1);
#endif // EXODUS_EXODEBUG_H
