#define BUILDING_LIBRARY
#include <exodus/exodus.h>

//a library section is a just a class section that can be called by other programs.

//library can have MULTIPLE libraryinit/exit sections and all are publically available
//by including the libraryfilename.h file name in other programs and libraries.

//a library can also have classes. classes are identical to library sections but they are
//private to the library since they are not published in the libraryfilename.h file.

#undef subroutine
#undef function
#define subroutine public: void
#define function public: var

//a library section is just a class plus some code
//that allows the class to be called from another program
#define libraryinit() \
classinit()

#define libraryexit() \
classexit() \
extern "C" DLL_PUBLIC void exodusprogrambasecreatedelete( \
                        pExodusProgramBase& pexodusprogrambase, \
                        MvEnvironment& mv, \
						pExodusProgramBaseMemberFunction& pmemberfunction) \
{ \
	if (pexodusprogrambase) {\
			delete pexodusprogrambase; \
			pexodusprogrambase=NULL; \
	} else { \
			pexodusprogrambase=new ExodusProgram(mv); \
			pmemberfunction=(pExodusProgramBaseMemberFunction) &ExodusProgram::main; \
			CALLMEMBERFUNCTION(*pexodusprogrambase,pmemberfunction); \
		} \
        return; \
}
