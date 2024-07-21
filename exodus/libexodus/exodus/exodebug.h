
	void exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size);
	var exo_backtrace(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit);
	PUBLIC void breakon();
	PUBLIC void breakoff();
	ND PUBLIC var backtrace();
	PUBLIC void debug(CVR var1);