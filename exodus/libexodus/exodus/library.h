#ifndef EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
#define EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
#define BUILDING_LIBRARY
#include <exodus/exodus.h>

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

// a library section is just a class plus a global exported function that allows the class
// to be instantiated and its main(...) called from another program via so/dll delay loading
// AND share the mv environment variables of the calling program!
#define libraryinit(PROGRAMCLASSNAME) \
class PROGRAMCLASSNAME##ExodusProgram : public ExodusProgramBase {

// to undo an ms optimisation that prevents casting between member function pointers
// http://social.msdn.microsoft.com/Forums/en/vclanguage/thread/a9cfa5c4-d90b-4c33-89b1-9366e5fbae74
//"VS compiler violates the standard in the name of optimization. It makes pointers-to-members
// have different size depending on the complexity of the class hierarchy.
// Print out sizeof(BaseMemPtr) and sizeof(&Derived::h) to see this.
// http://msdn.microsoft.com/en-us/library/yad46a6z.aspx
// http://msdn.microsoft.com/en-us/library/83cch5a6.aspx
// http://msdn.microsoft.com/en-us/library/ck561bfk.aspx"
//

// the above requirement could be removed if libraryexit() generated the exactly
// correct pointer to member WITH THE RIGHT ARGUMENTS
// this could be done by generating special code in funcx.h and requiring
//#include "funcx.h" in the bottom of every "funcx.cpp"

/*
without the above pragma, in msvc 2005+ you get an error when compiling libraries
(exodus external subroutines) in the libraryexit() line containing "&ExodusProgram::main;" as
follows:

f1.cpp(12) : error C2440: 'type cast' : cannot convert from 'exodus::var (__this
call ExodusProgram::* )(exodus::in)' to 'exodus::pExodusProgramBaseMemberFunction';
Pointers to members have different representations; cannot cast between them
*/

#define libraryexit(PROGRAMCLASSNAME)                                                          \
 public:                                                                                       \
	PROGRAMCLASSNAME##ExodusProgram(ExoEnv& mv) : ExodusProgramBase(mv) {}                     \
};                                                                                             \
extern "C" PUBLIC void exodusprogrambasecreatedelete_##PROGRAMCLASSNAME(                       \
		pExodusProgramBase& pexodusprogrambase, ExoEnv& mv,                                    \
		pExodusProgramBaseMemberFunction& pmemberfunction) {                                   \
		if (pexodusprogrambase) {                                                              \
			delete pexodusprogrambase;                                                         \
			pexodusprogrambase = nullptr;                                                      \
			pmemberfunction = nullptr;                                                         \
		} else {                                                                               \
			pexodusprogrambase = new PROGRAMCLASSNAME##ExodusProgram(mv);                      \
			_Pragma("GCC diagnostic push")                                                     \
            _Pragma("GCC diagnostic ignored \"-Wcast-function-type\"")                         \
				pmemberfunction =                                                              \
					(pExodusProgramBaseMemberFunction)&PROGRAMCLASSNAME##ExodusProgram::main;  \
			_Pragma("GCC diagnostic pop")                                                      \
		}                                                                                      \
		return;                                                                                \
	}
	// purpose of the above is to either return a new exodusprogram object
	// and a pointer to its main function - or to delete an exodusprogram object
#endif // EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_

