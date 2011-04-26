#define BUILDING_LIBRARY
#include <exodus/exodus.h>

//a common section is a just a class section that can be instantiated by other programs.

//common can have MULTIPLE commoninit/exit sections and all are publically available
//by including the libraryfilename.h file name in other programs and libraries.

#undef subroutine
#undef function
#define subroutine public: void
#define function public: var

//a library section is just a class plus some code
//that allows the class to be called from another program
#define commoninit() \
classinit()

#define commonexit() \
classexit() \
extern "C" DLL_PUBLIC void exodusprogrambasecreatedelete( \
                        pExodusProgramBase& pexodusprogrambase, \
                        MvEnvironment& mv, \
						pExodusProgramBaseMemberFunction& \
						 pmemberfunction) \
{ \
	if (pexodusprogrambase) {\
			delete pexodusprogrambase; \
			pexodusprogrambase=NULL; \
	} else { \
			pexodusprogrambase=new ExodusProgram(mv); \
			/*pmemberfunction=(pExodusProgramBaseMemberFunction) &ExodusProgram::main;*/ \
		} \
		return; \
}
//purpose of the above is to either return a new exodusprogram object
//and a pointer to its main function - or to delete an exodusprogram object