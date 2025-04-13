#ifndef EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
#define EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_

#include <exodus/exodus.h>

#ifndef EXO_LIBRARY
#	define EXO_LIBRARY
#endif

// A library section is a just a c++ class stored in an so/dll shared library that can be
// called like a function or subroutine from an exodus program or other library.
//
//  libraryinit()
//  function main(parameters ...){
//   ...
//  }
//  libraryexit()
//
// A library can have multiple libraryinit/exit sections and all are publically available
//
//  libraryinit(funcx)
//  function main(parameters ...){
//   ...
//  }
//  libraryexit(funcx)
//
// by including its xxxxxxxx.h file name in other programs and libraries.
//
// A library's header file is generated when compiled using exodos/cli/compile which also calls c++.
//
// A library can also include other classes. Classes are identical to library sections but they are
// private to the library since they are not published in its xxxxxxxx.h file.

// SIMILAR CODE IN
// program.h library.h

// a library section is just a class plus a global exported factory function that allows the
// class to be instantiated and its main(...) function called from another program
// via so/dll delay loading AND share the mv environment variables of the calling program.

/////////////////////////////////////////////////////////////////
// libraryinit simply opens a class that inherits from ExoProgram
// //////////////////////////////////////////////////////////////

#define libraryinit(EXOPROGRAM_PREFIX)                     \
_Pragma("GCC diagnostic push")                             \
_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")     \
_Pragma("GCC diagnostic ignored \"-Winline\"")             \
class EXOPROGRAM_PREFIX##_ExoProgram : public ExoProgram { \
_Pragma("GCC diagnostic pop")

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// libraryexit simply closes the class and provides a factory function to create an instance of the class
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#define libraryexit(EXOPROGRAM_PREFIX)                                                  \
 public:                                                                                \
	_Pragma("clang diagnostic push")                                                    \
	_Pragma("clang diagnostic ignored \"-Wshadow-field\"")                              \
	EXOPROGRAM_PREFIX##_ExoProgram(ExoEnv& mv) : ExoProgram(mv) {}                      \
	_Pragma("clang diagnostic pop")                                                     \
};                                                                                      \
_Pragma("clang diagnostic push")                                                        \
_Pragma("clang diagnostic ignored \"-Wmissing-prototypes\"")                            \
extern "C" PUBLIC void exoprogram_createdelete_##EXOPROGRAM_PREFIX(                     \
		pExoProgram& pexoprogram, ExoEnv& mv,                                           \
		pExoProgram_MemberFunc& pmemberfunc) {                                          \
_Pragma("clang diagnostic pop")                                                         \
		if (pexoprogram) {                                                              \
			delete pexoprogram;                                                         \
			pexoprogram = nullptr;                                                      \
			pmemberfunc = nullptr;                                                      \
		} else {                                                                        \
			pexoprogram = new EXOPROGRAM_PREFIX##_ExoProgram(mv);                       \
			_Pragma("GCC diagnostic push")                                              \
            _Pragma("GCC diagnostic ignored \"-Wcast-function-type\"")                  \
				pmemberfunc =                                                           \
					/*(pExoProgram_MemberFunc)&EXOPROGRAM_PREFIX##_ExoProgram::main;*/  \
					reinterpret_cast<pExoProgram_MemberFunc>(&EXOPROGRAM_PREFIX##_ExoProgram::main);  \
			_Pragma("GCC diagnostic pop")                                               \
		}                                                                               \
		return;                                                                         \
	}
	// purpose of the above is to either return a new _ExoProgram object
	// and a pointer to its main function - or to delete an _ExoProgram object
#endif // EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_