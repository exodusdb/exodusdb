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

#include "lib1.h"
#include <exodus/exodus.h>
exodusprogram(){
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

To use and call the above functions in a program or another library you just need to include the
header file which is automatically generated when you do "compile lib1"

#include "lib1.h"

Alternatively, to keep it simple and just like traditional multivalue keep each function
and subroutine in a separate file with the same name as the function or subroutine.
That way you can forget the new exodus concept of "library" which is not a traditional mv concept

*/

#ifndef MVFUNCTOR_H
#define MVFUNCTOR_H

#include <exodus/mv.h>

// MvEnvironment and ExodusProgramBase are forward declared classes (see below)
// because they only exist as pointers or references in mvfunctor.
// and MvEnvironment contains an actual ExodusFunctorBase
//#include <exodus/mvenvironment.h>
//#include <exodus/mvprogram.h>

// good programming practice to prevent many white hairs
// http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.6
#define CALLMEMBERFUNCTION(object, ptrToMember) ((object).*(ptrToMember))

namespace exodus
{
// using namespace exodus;

class ExodusProgramBase;
class MvEnvironment;

// pExodusProgramBase - "pointer to exodus program" type
using pExodusProgramBase = ExodusProgramBase*;

// pExodusProgramBaseMemberFunction - "pointer to exodus program" member function
using pExodusProgramBaseMemberFunction = var (ExodusProgramBase::*)();

// ExodusProgramCreateDeleteFunction - pointer to global function that creates and deletes exodus
// programs
using ExodusProgramBaseCreateDeleteFunction = void (*)(pExodusProgramBase&, MvEnvironment&,
						      pExodusProgramBaseMemberFunction&);

class DLL_PUBLIC ExodusFunctorBase
{

      public:
	ExodusFunctorBase();

	// constructor to provide everything immediately
	ExodusFunctorBase(const std::string libname, const std::string funcname, MvEnvironment& mv);

	// constructor to provide library and function names immediately
	ExodusFunctorBase(const std::string libname, const std::string funcname);

	// constructor to provide environment immediately
	ExodusFunctorBase(MvEnvironment& mv);

	// to allow function name to be assigned a name and this name is the name of the library
	// called arev call @
	ExodusFunctorBase& operator=(const char*);

	// call shared member function
	var callsmf();

	// call shared global function
	var callsgf();

	/*
	//constructor to provide library and function names immediately
	ExodusFunctorBase(const std::string libname,const std::string funcname);
	*/

	// destructors of base classes must be virtual (if derived classes are going to be
	// new/deleted?) otherwise the destructor of the derived class is not called when it should
	// be
	virtual ~ExodusFunctorBase();

	// use void* to speed compilation of exodus applications on windows by avoiding
	// inclusion of windows.h here BUT SEE ...
	// Can I convert a pointer-to-function to a void*? NO!
	// http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.11
	// In the implementation cast to HINSTANCE and
	// conversion between function pointer and void* is not legal c++ and only works
	// on architectures that have data and functions in the same address space
	// since void* is pointer to data space and function is pointer to function space
	//(having said that ... posix dlsym returns a void* for the address of the function!)
	// void* pfunction_;

      public:
	// for call or die (smf)
	bool init(const char* libraryname, const char* functionname, MvEnvironment& mv);

	// assumes name and mv setup on initialisation then opens library on first call
	bool init();

	// for dict/perform/execute (external shared member functions)
	// forcenew used by perform/execute to delete and create new object each time
	// so that global variables start out unassigned each time performed/executed
	bool initsmf(const char* libraryname, const char* functionname,
		     const bool forcenew = false);

	// external shared global functions (not member functions)
	bool initsgf(const char* libraryname, const char* functionname);

	// TODO move to private
	void closelib();

      private:
	bool openlib(std::string libraryname);
	bool openfunc(std::string functionname);
	void closefunc();

      protected:
	bool checkload(std::string libraryname, std::string functionname);

      public:
	// only public for rather hacked mvipc getResponseToRequest()
	mutable MvEnvironment* mv_;

	// TODO move to private
	// records the library opened so we can close and reopen new libraries automatically
	std::string libraryname_;

      private:
	// normally something like
	// exodusprogrambasecreatedelete_
	// or exodusprogrambasecreatedelete_{dictid}
	// one function is used to create and delete the shared library object
	std::string functionname_;

	// internal memory of the actual library file name. only used for error messages
	std::string libraryfilename_;
	// pointer to the shared library file
	void* plibrary_;

      protected:
	// functioname_ is used to open a dl shared function to this point
	ExodusProgramBaseCreateDeleteFunction pfunction_;

      public:
	// holds the
	// not used if functor is calling global functions in the shared object
	pExodusProgramBase pobject_;
	pExodusProgramBaseMemberFunction pmemberfunction_;

	std::string libfilename(std::string libraryname) const;

};

} // namespace exodus

#endif // MVFUNCTOR_H
