// DONT use header guard since may required multiple times eg in dict_xxxxx.cpp files
//#ifndef SERVICE_SRC_SYS_SYSTEM_COMMON_H_
//#define SERVICE_SRC_SYS_SYSTEM_COMMON_H_

#define esc_ "\x1B"
//#define crlf_ "\n"

#define maxstrsize_ 1048576
#define OSSLASH_ "/"

//keep "centuryyear" in sync in frontend exodus.js
//and backend ADDCENT ADDCENT2 ADDCENT4
#define centuryyear_ 50

#define interactive_ not(SYSTEM.f(33))

#define request_ USER0
#define data_ USER1
#define response_ USER3
#define msg_ USER4
