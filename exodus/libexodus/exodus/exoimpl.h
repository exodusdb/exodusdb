#ifndef EXOIMPL_H
#define EXOIMPL_H

namespace exodus {

PUBLIC void debug(CVR = "");
PUBLIC void exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t* stack_size);
ND PUBLIC var exo_backtrace( void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t stack_size, size_t limit = 0);

// Set by signals for threads to poll
PUBLIC inline bool TERMINATE_req = false;
PUBLIC inline bool RELOAD_req = false;

} //namespace exodus

#endif //EXOIMPL_H
