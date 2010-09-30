/*

Traditionally, mv functions and subroutines are implemented as separately compilable units
and there is only one function or subroutine per compilation unit (file/record).

the only difference between functions and subroutines is that functions return a value whereas
subroutines do not. Both functions and subroutines can be called with any number of arguments
including none.

exodus implements mv external functions and subroutines as C++ functors in shared libraries loaded
on demand/just in time.

=== example ===

imagine we want a subroutine called "subr3" that takes one argument (and returns nothing)

we decide that "subr3" will be in a library (ie a single compilable file/record) called "lib1"

==== library lib1.cpp ====

#include <exodus/exodus.h>
subroutine subr3(in var1) {
	println("lib1/subr3 says "^var1);
}

==== lib1.h ====

This header file is generated AUTOMATICALLY by the command "compile lib1"

==== client call1.cpp ====

To use the function all we have to do is include the lib1.h file. The library
binary is automatically loaded the first time that the function is called.

#include <exodus/exodus.h>
#include "lib1.h"
program(){
 subr3("xyz");
}

==== sample session ====

compile lib1
compile call1
call1
lib1/sub3 says xyz

=== Exodus Library Concept ===

exodus allows multiple functions and subroutines in a particular file using the following syntax

eg in a file lib1.cpp

#include <exodus/exodus.h>
function func1(args...) {
...
}
subroutine sub1(args...) {
...
}

To use and call the above functions in a program or another library you just need to include the header file
which is automatically generated when you do "compile lib1"

#include "lib1.h"

Alternatively, to keep it simple and just like traditional multivalue keep each function
and subroutine in a separate file with the same name as the function or subroutine.
That way you can forget the new exodus concept of "library" which is not a traditional mv concept

*/

#ifndef MVFUNCTOR_H
#define MVFUNCTOR_H

#if defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)
# define EXODUSLIBEXT ".dll"
#else
# define EXODUSLIBEXT ".so"
#endif

namespace exodus {

class DLL_PUBLIC ExodusFunctorBase
{

public:

ExodusFunctorBase(const std::string libname,const std::string funcname);

virtual ~ExodusFunctorBase();
//private:
protected:

//use void* to speed compilation of exodus applications on windows by avoiding
//inclusion of windows.h here. In the implementation cast to HINSTANCE and 
void* _plibrary;
void* _pfunction;

std::string _libraryname;
std::string _functionname;

void checkload();
bool openlib();
bool openfunc();

};

/*
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

}//namespace exodus

#endif //MVFUNCTOR_H
