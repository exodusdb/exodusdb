#ifndef EXOIMPL_H
#define EXOIMPL_H

namespace exodus {

PUBLIC void debug(CVR = "");
PUBLIC void mv_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t* stack_size);
PUBLIC ND var mv_backtrace( void* stack_addresses[BACKTRACE_MAXADDRESSES], size_t stack_size);

// Set by signals for threads to poll
PUBLIC inline bool TERMINATE_req = false;
PUBLIC inline bool RELOAD_req = false;

} //namespace exodus

#endif //EXOIMPL_H
