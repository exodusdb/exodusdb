#define BUILDING_LIBRARY
#include <exodus/exodus.h>

// a library section is a just a class stored in an so/dll shared library that can be
// called like a function or subroutine from another executable.

// library could have MULTIPLE libraryinit/exit sections and all are publically available
// by including the libraryfilename.h file name in other programs and libraries.
// the libraryinit() libraryexit() macros would have to modified to create function names
// eg libraryinit(funcx) libraryexit(funcx)

// a library can also have classes. classes are identical to library sections but they are
// private to the library since they are not published in the libraryfilename.h file.

// exodus subroutines and functions in libraries are just local subroutines and functions
// and are redefined without DLLEXPORT here
#undef subroutine
#undef function
#define subroutine                                                                                 \
      public:                                                                                      \
	void
#define function                                                                                   \
      public:                                                                                      \
	var

// a library section is just a class plus a global exported function that allows the class
// to be instantiated and its main(...) called from another program via so/dll delay loading
// AND share the mv environment variables of the calling program!
#define libraryinit(CLASSNAME) classinit(CLASSNAME)

// to undo an ms optimisation that prevents casting between member function pointers
// http://social.msdn.microsoft.com/Forums/en/vclanguage/thread/a9cfa5c4-d90b-4c33-89b1-9366e5fbae74
//"VS compiler violates the standard in the name of optimization. It makes pointers-to-members
// have different size depending on the complexity of the class hierarchy.
// Print out sizeof(BaseMemPtr) and sizeof(&Derived::h) to see this.
// http://msdn.microsoft.com/en-us/library/yad46a6z.aspx
// http://msdn.microsoft.com/en-us/library/83cch5a6.aspx
// http://msdn.microsoft.com/en-us/library/ck561bfk.aspx"
//
// Interestingly, Exodus up to subversion version 129 worked fine without this but I wasnt able to
// detect what complexity had been added that caused MS to optimise and break mfp compatibility
// warning C4158: assuming #pragma pointers_to_members(full_generality, virtual_inheritance)
#pragma warning(disable : 4158)
#pragma pointers_to_members(full_generality)

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

#define libraryexit(CLASSNAME)                                                                     \
	classexit(CLASSNAME) extern "C" DLL_PUBLIC void exodusprogrambasecreatedelete_##CLASSNAME( \
	    pExodusProgramBase& pexodusprogrambase, MvEnvironment& mv,                             \
	    pExodusProgramBaseMemberFunction& pmemberfunction)                                     \
	{                                                                                          \
		if (pexodusprogrambase)                                                            \
		{                                                                                  \
			delete pexodusprogrambase;                                                 \
			pexodusprogrambase = NULL;                                                 \
			pmemberfunction = NULL;                                                    \
		}                                                                                  \
		else                                                                               \
		{                                                                                  \
			pexodusprogrambase = new CLASSNAME##ExodusProgram(mv);                     \
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wcast-function-type\"")\
			pmemberfunction =                                                          \
			    (pExodusProgramBaseMemberFunction)&CLASSNAME##ExodusProgram::main;     \
			_Pragma("GCC diagnostic pop")\
		}                                                                                  \
		return;                                                                            \
	}
// purpose of the above is to either return a new exodusprogram object
// and a pointer to its main function - or to delete an exodusprogram object
