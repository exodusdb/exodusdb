#ifndef EXODUS_COMMON_H
#define EXODUS_COMMON_H

#include <exodus/exodus.h>

// Provides commoninit() and commonexit()
// to create blocks of thread_local global common variables.
//
// commoninit and commonexit just wrap a simple C++ class
//
// "compile xxx_common.h" will:
//
// 1. Create a temporary .cpp file and shared library xxx_common.so
//
// 2. Link any program or library that includes xxx_common.h to xxx_common.so

#ifndef EXO_COMMON_EXTERN
#	define EXO_COMMON_EXTERN extern
//#	undef PUBLIC
//#	define PUBLIC
#endif

#ifndef PUBLIC
#	define PUBLIC __attribute__((visibility("default")))
#endif

//////////////////////////
// Open a class xxx_common
//////////////////////////

#define commoninit(COMMON_CODE)     \
class COMMON_CODE##_common { \
public:

//////////////////////////////////////////////////////////////////
// Close the class and define a thread_local variable to access it
//////////////////////////////////////////////////////////////////

#define commonexit(COMMON_CODE)                                  \
~COMMON_CODE##_common() = default;                               \
};                                                               \
                                                                 \
_Pragma("GCC diagnostic push")                                   \
_Pragma("GCC diagnostic ignored \"-Wpadded\"")                   \
[[maybe_unused]]                                                 \
PUBLIC EXO_COMMON_EXTERN thread_local COMMON_CODE##_common COMMON_CODE; \
_Pragma("GCC diagnostic pop")

#endif // EXODUS_COMMON_H