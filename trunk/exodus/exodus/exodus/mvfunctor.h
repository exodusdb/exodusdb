/*
WARNING this documentation is from BEFORE adding enabling call of objects in external libraries
using programinit/exit and libraryinit/exit.

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
	printl("lib1/subr3 says "^var1);
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

#include <exodus/mv.h>

//MvEnvironment and ExodusProgramBase are forward declared classes (see below)
//because they only exist as pointers or references in mvfunctor.
//and MvEnvironment contains an actual ExodusFunctorBase
//#include <exodus/mvenvironment.h>
//#include <exodus/mvprogram.h>

//good programming practice
//http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.6
#define CALLMEMBERFUNCTION(object,ptrToMember)  ((object).*(ptrToMember)) 

namespace exodus {
//using namespace exodus;

class ExodusProgramBase;
class MvEnvironment;

//pExodusProgramBase - pointer to exodus program type
typedef ExodusProgramBase* pExodusProgramBase;

//pExodusProgramBaseMemberFunction - pointer to exodus program member function
typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

/*
//pExodusProgram - pointer to exodus program type
class ExodusProgram;
typedef ExodusProgram* pExodusProgram;
//pExodusProgramMemberFunction - pointer to exodus program member function
typedef var (ExodusProgram::*pExodusProgramMemberFunction)();
*/

//ExodusProgramCreateDeleteFunction - pointer to global function that creates and deletes exodus programs
typedef void (*ExodusProgramBaseCreateDeleteFunction)
(
	pExodusProgramBase&,
	MvEnvironment&,
	pExodusProgramBaseMemberFunction&
);

class DLL_PUBLIC ExodusFunctorBase
{

public:

ExodusFunctorBase();

//constructor to provide library and function names immediately
ExodusFunctorBase(const std::string libname,
				  const std::string funcname);

//constructor to provide environment immediately
ExodusFunctorBase(MvEnvironment& mv);

void ExodusFunctorBase::calldict();

/*
//constructor to provide library and function names immediately
ExodusFunctorBase(const std::string libname,const std::string funcname);
*/

//destructors of base classes must be virtual (if derived classes are going to be new/deleted?)
//otherwise the destructor of the derived class is not called when it should be
virtual ~ExodusFunctorBase();

//use void* to speed compilation of exodus applications on windows by avoiding
//inclusion of windows.h here BUT SEE ...
//Can I convert a pointer-to-function to a void*? NO!
//http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.11
//In the implementation cast to HINSTANCE and 
//conversion between function pointer and void* is not legal c++ and only works
//on architectures that have data and functions in the same address space
//since void* is pointer to data space and function is pointer to function space
//(having said that ... posix dlsym returns a void* for the address of the function!)
//void* pfunction_;

public:
	bool init(const char* libraryname, const char* functionname);
	bool init(const char* libraryname, const char* functionname, MvEnvironment& mv);
	pExodusProgramBase pobject_;
	pExodusProgramBaseMemberFunction pmemberfunction_;

	//only public for rather hacked mvipc getResponseToRequest()
	mutable MvEnvironment* mv_;

private:
	void* plibrary_;
	std::string libraryname_;
	std::string functionname_;
	std::string libraryfilename_;

	bool openlib();
	void closelib();
	bool openfunc();

protected:
	bool checkload();
	ExodusProgramBaseCreateDeleteFunction pfunction_;

};

}//namespace exodus

#endif //MVFUNCTOR_H
