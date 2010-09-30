/*

NOTE: Do *not* add #ifndef header protector since this file is designed to be included more than once

This header file is used to generate functors for functions and subroutines that have
arguments that cannot be represented by vars eg ("inarray" arguments)

For example, the following function cannot be handled by the standard exodus functor
because it contains a varray which cannot be converted to a var.

function xyz(in arg1,inarray arg2)

Wherease, the following function CAN be handled by the standard exodus functor because all its
arguments are vars

function xyz(in arg1,in arg2)

This header (the one you are reading) is designed to be included with macro parameters
to generate classes that implement "functors" ie "function-like" objects to simulate
mv external functions and subroutines with any combination of arguments

it is included ONCE PER FUNCTION/SUBROUTINE used to generate multiple versions
of the class to implement different number of arguments. each version of
the class is assigned a different classname

the last line of this file instantiates a specific object that can be used
like a function/subroutine

==== header lib1.h ====

NB this header is generated AUTOMATICALLY by the command "compile lib1"

#define EXODUSFUNCNAME subr3
#define EXODUSLIBNAME lib1
#define EXODUSFUNCNAME subr3
#define EXODUSFUNCRETURN void
#define EXODUSFUNCARGS in arg1
#define EXODUSFUNCARGS2 arg1
#define EXODUSFUNCTORCLASSNAME ExodusFunctor_subr3
#define EXODUSFUNCTYPE ExodusDynamic_subr3
#define EXODUSLIBNAMEQQ "lib1"
#define EXODUSFUNCNAMEQQ "subr3"
#define EXODUSFUNCRETURNVOID 1
#include <exodus/mvlink.h>
#undef EXODUSLIBNAME
#undef EXODUSFUNCNAME
#undef EXODUSFUNCRETURN
#undef EXODUSFUNCARGS
#undef EXODUSFUNCARGS2
#undef EXODUSFUNCTORCLASSNAME
#undef EXODUSFUNCTYPE
#undef EXODUSLIBNAMEQQ
#undef EXODUSFUNCNAMEQQ
#undef EXODUSCLASSNAME
#undef EXODUSFUNCRETURNVOID

*/

//NB do NOT protect this from multiple inclusion since
//if can be called multiple time with different macro parameters
//to create multiple classes with/for different functor argument signatures

#include "mvfunctor.h"

namespace exodus {

// declare DLL/SO function type
typedef EXODUSFUNCRETURN (*EXODUSFUNCTYPE)(EXODUSFUNCARGS);

class EXODUSFUNCTORCLASSNAME : private ExodusFunctorBase
{
public:

 //base constructors are not inherited but can be called with an initialiser list
 EXODUSFUNCTORCLASSNAME(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

 EXODUSFUNCRETURN operator() (EXODUSFUNCARGS)
 {
	checkload();
#if EXODUSFUNCRETURNVOID == 1
	((EXODUSFUNCTYPE) _pfunction)(EXODUSFUNCARGS2);
	return;
#else
	return ((EXODUSFUNCTYPE) _pfunction)(EXODUSFUNCARGS2);
#endif
 }

};


//define a functor object so the function can be called like aaa=func1(a,b,c)
//with automatic just in  time loading of the shared library (dll/so etc)
//ExodusFunctor_func1 func1("lib1","func1");
EXODUSFUNCTORCLASSNAME EXODUSFUNCNAME(EXODUSLIBNAMEQQ,EXODUSFUNCNAMEQQ);

}//namespace exodus
