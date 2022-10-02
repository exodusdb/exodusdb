/*
WARNING this documentation is from BEFORE adding enabling call of objects in external libraries
using programinit/exit and libraryinit/exit.

Traditionally, mv functions and subroutines are implemented as separately compilable units
and there is only one function or subroutine per compilation unit (file/record).

the only difference between functions and subroutines is that functions return a value whereas
subroutines do not. Both functions and subroutines can be called with any number of arguments
including none.

exodus implements mv external functions and subroutines as C++ callables in shared libraries loaded
on demand/just in time.

=== example ===

imagine we want a subroutine called "subr3" that takes one argument (and returns nothing)

we decide that "subr3" will be in a library (ie a single compilable file/record) called "lib1"

==== library lib1.cpp ====

#include <exodus/library.h>
libraryinit()
function main(in var1) {
	printl("lib1 says " ^ var1);
}
libraryexit()

==== lib1.h ====

This header file is generated AUTOMATICALLY by the command "compile lib1"

==== program call1.cpp ====

To use the function all we have to do is include the lib1.h file. The library
binary is automatically loaded the first time that the function is called.

#include <exodus/program.h>
programinit()
#include <lib1.h>
function main (){
 call lib1("xyz");
}
programexit()

==== sample session ====

compile lib1
compile call1
call1
lib1 says xyz

=== Exodus Library Concept ===

exodus allows multiple functions and subroutines in a particular file using the following syntax

This is currently only used to provide c++ dictionary functions at the moment
but the machinery would be similar for multi-function libraries.

e.g. in a file dict1.cpp

#include <exodus/dict.h>
dictinit(aaa)
...
dictexit(aaa)

dictinit(bbb)
...
dictexit(bbb)
*/

#ifndef EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_
#define EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_

#include <exodus/var.h>

// good programming practice to prevent many white hairs
// http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.6
#define CALLMEMBERFUNCTION(object, ptrToMember) ((object).*(ptrToMember))

namespace exodus {
// using namespace exodus;

class ExodusProgramBase;
class ExoEnv;

// pExodusProgramBase - "pointer to exodus program" type
using pExodusProgramBase = ExodusProgramBase*;

// pExodusProgramBaseMemberFunction - "pointer to exodus program" member function
using pExodusProgramBaseMemberFunction = auto (ExodusProgramBase::*)() -> var;

// ExodusProgramCreateDeleteFunction - pointer to global function that creates and deletes exodus
// programs
using ExodusProgramBaseCreateDeleteFunction = auto (*)(pExodusProgramBase&, ExoEnv&, pExodusProgramBaseMemberFunction&) -> void;

class PUBLIC CallableBase {

	//TODO remove this
	friend ExodusProgramBase;

 public:
    // only public for rather hacked mvipc getResponseToRequest()
    mutable ExoEnv* mv_;

 public:

	// Default
	CallableBase();

	// Destructor
	// destructors of base classes must be virtual (if derived classes are going to be
	// new/deleted?) otherwise the destructor of the derived class is not called when it should
	// be
	virtual ~CallableBase();

	// Copy constructor
	CallableBase(const CallableBase&) = delete;

	// Constructor to provide everything immediately
	CallableBase(const std::string libname, const std::string funcname, ExoEnv& mv);

	// Constructor to provide library and function names immediately
	CallableBase(const std::string libname, const std::string funcname);

	// constructor to provide environment immediately
	CallableBase(ExoEnv& mv);

	// Assignment
	// to allow function name to be assigned a name and this name is the name of the library
	// called pickos call @
	virtual void operator=(const char* libname);

 protected:
	// for call or die (smf)
	bool init(const char* libraryname, const char* functionname, ExoEnv& mv);
	//bool init(const char* libraryname, const char* functionname);

	// assumes name and mv setup on initialisation then opens library on first call
	bool init();

	// for dict/perform/execute (external shared member functions)
	// forcenew used by perform/execute to delete and create new object each time
	// so that global variables start out unassigned each time performed/executed
	bool initsmf(const char* libraryname, const char* functionname, const bool forcenew = false);

	// external shared global functions (not member functions)
	bool initsgf(const char* libraryname, const char* functionname);

	// TODO move to private
	void closelib();

 private:
	bool openlib(std::string libraryname);
	bool openfunc(std::string functionname);
	void closefunc();

 //protected:
	bool checkload(std::string libraryname, std::string functionname);

	// call shared member function
	var callsmf();

	// call shared global function
	var callsgf();

	/*
	//constructor to provide library and function names immediately
	CallableBase(const std::string libname,const std::string funcname);
	*/

	// only public for rather hacked mvipc getResponseToRequest()
	//mutable ExoEnv* mv_;

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

// public:
	// holds the
	// not used if callable is calling global functions in the shared object
	pExodusProgramBase pobject_;
	pExodusProgramBaseMemberFunction pmemberfunction_;

	std::string libfilename(std::string libraryname) const;
};

}  // namespace exodus

#endif // EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_
