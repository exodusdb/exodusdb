// DONT use header guard since may required multiple times eg in dict_xxxxx.cpp files
//#ifndef SERVICE_SRC_SRV_SERVICE_COMMON_H_
//#define SERVICE_SRC_SRV_SERVICE_COMMON_H_

#define esc_ "\x1B"
//#define crlf_ "\n"

#define interactive_ not(SYSTEM.f(33))

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wattributes"
#endif

[[maybe_unused]] var& request_  = USER0;
[[maybe_unused]] var& data_     = USER1;
[[maybe_unused]] var& response_ = USER3;
[[maybe_unused]] var& msg_      = USER4;

[[maybe_unused]] const int maxstrsize_ = 1048576;
//#define OSSLASH_ "/"

//keep "centuryyear" in sync in frontend exodus.js
//and backend ADDCENT ADDCENT2 ADDCENT4
[[maybe_unused]] const int centuryyear_ = 50;

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
//#endif // SERVICE_SRC_SRV_SERVICE_COMMON_H_
