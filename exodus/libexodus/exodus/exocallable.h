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

class PUBLIC Callable {

 protected:

    mutable ExoEnv* mv_;

 private:

	// Records the library opened so we can close and reopen new libraries automatically
	std::string libname_;

	// Records the function opened so we can close and reopen new function automatically
	std::string funcname_;

	// Path to or searched for library file name
	std::string libfilepath_;

	// Pointer to the shared lib file
	void* plib_;

	// Function used to create and delete the ExodusProgramBase object
	ExodusProgramBaseCreateDeleteFunction pfunc_;

 protected:

	// Not used if callable is calling global functions in the shared object
	pExodusProgramBase plibobject_;
	pExodusProgramBaseMemberFunction pmemberfunc_;

 public:

	///////////////////////////
	// Special Member Functions
	///////////////////////////

	// 1. Default Constructor

	Callable();

	// 2. Destructor

	// Destructors of base classes must be virtual (if derived classes are going to be
	// new/deleted?) otherwise the destructor of the derived class is not called when it should
	// be
	virtual ~Callable();

	// 3. Copy and Move constructors - deleted

	Callable(const Callable&) = delete;


	//////////////////////
	// Custom Constructors
	//////////////////////

	// Constructor to provide everything immediately
	Callable(const std::string_view libname, const std::string_view funcname, ExoEnv& mv);

	// Constructor to provide library and function names immediately
	Callable(const std::string_view libname, const std::string_view funcname);

	// Constructor to provide environment immediately
	Callable(ExoEnv& mv);


	/////////////
	// Assignment
	/////////////

	// Allow actual lib/func called to be changed at will at runtime
	// PickOS 'call @xxxx(...)' calls lib/func named in the variable xxx
	// PickOS 'call xxxx(...)' calls lib/func actually called xxx
	virtual void operator=(const char* libname);

	///////////////
	// Other public
	///////////////

	// For dict/perform/execute (external shared member functions)
	// forcenew used by perform/execute to delete and create new object each time
	// so that global variables start out unassigned each time performed/executed
	bool initsmf(ExoEnv& mv, const char* libname, const char* funcname, const bool forcenew = false);

	// Call a shared member function
	var callsmf();

	// Call a shared global function
	//var callsgf();

	// Build a path to the shared library file
	std::string libfilepath(const std::string_view libname) const;

 protected:

	// Assumes name and mv setup on initialisation then opens library on first call
	//bool init();

	//bool attach(const char* libname, const char* funcname, ExoEnv& mv);
	bool attach(const char* libname);

	// External shared global functions (not member functions)
	//bool initsgf(const char* libname, const char* funcname);

private:

	bool openlib(const std::string libname);
	void closelib();

	bool openfunc(const std::string funcname);
	void delete_shared_object();

	bool checkload(const std::string libname, const std::string funcname);

};

}  // namespace exodus

#endif // EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_
