/*
WARNING this documentation is from BEFORE adding enabling call of objects in external libraries
using programinit/exit and libraryinit/exit.

Traditionally, ev functions and subroutines are implemented as separately compilable units
and there is only one function or subroutine per compilation unit (file/record).

the only difference between functions and subroutines is that functions return a value whereas
subroutines do not. Both functions and subroutines can be called with any number of arguments
including none.

exodus implements ev external functions and subroutines as C++ callables in shared libraries loaded
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
}; // libraryexit()

==== lib1.h ====

This header file is generated AUTOMATICALLY by the command "compile lib1"

==== program call1.cpp ====

To use the function all we have to do is include the lib1.h file. The library
binary is automatically loaded the first time that the function is called.

#include <exodus/program.h>
programinit()
#include <lib1.h>
function main (){
 call lib1("xyz");  // "call" is an empty macro
}
}; // programexit()

==== sample session ====

compile lib1
compile call1
call1
lib1 says xyz

=== Exodus Library Concept ===

exodus allows multiple function objects in a particular file using the following syntax

This is currently only used to provide c++ "dictionary" functions at the moment
but the machinery would be similar for multi-function libraries.

e.g. in a file dict1.cpp

#include <exodus/dict.h>
dictinit(aaa)
...
};

dictinit(bbb)
...
};
*/

#ifndef EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_
#define EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_

#if EXO_MODULE
	import var;
#else
//#	include <mutex>
//#include <future>
//#include <map>
//#include <mutex>
//#include <string>
#	include <exodus/var.h>
#endif

#include <exodus/exoimpl.h>

// good programming practice to prevent many white hairs
// http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.6
#define CALLMEMBERFUNCTION(object, ptrToMember) ((object).*(ptrToMember))

namespace exo {
// using namespace exo;

class ExoProgram;
class ExoEnv;

// pExoProgram - "pointer to exodus program" type
using pExoProgram = ExoProgram*;

// pExoProgram_MemberFunc - "pointer to exodus program" member function
using pExoProgram_MemberFunc = auto (ExoProgram::*)() -> var;

// ExodusProgramCreateDeleteFunction - pointer to global function that creates and deletes exodus
// programs
using ExoProgramCreateDeleteFunction = auto (*)(pExoProgram&, ExoEnv&, pExoProgram_MemberFunc&) -> void;

//class Callable
class PUBLIC Callable {

 protected:

    mutable ExoEnv* mv_;

// private:

	// Records the library opened so we can close and reopen new libraries automatically
	std::string libname_;

	// Records the function opened so we can close and reopen new functions automatically
	std::string funcname_;

	// Full path to library file name
	// or just the file name if opened by standard lib search
	std::string libfilepath_;

	// Handle on the opened shared lib
	void* plib_;

	// Pointer to a function used to create and delete a ExoProgram object defined in the shared library.
	// Could also be used to call any global function in the shared library
	ExoProgramCreateDeleteFunction pfunc_;

 protected:

	// Not used if callable is only calling global functions in the shared library
	pExoProgram plibobject_;

	/*
		Note: We will be using the following pointer to member function to call
		member functions (in shared libraries) that have different arguments.

		ISO C++ doesnt support casting between member function pointers unless they are "similar".

		But many C++ compilers relax this rule, as a non-standard language extension,
		to allow wrong-type access through the inactive member of a union.
		Such access is not undefined in C.
		https://gcc.gnu.org/onlinedocs/gcc/Cast-to-Union.html#Cast-to-Union

		This has worked in gcc and clang 2000 to date (2022) and worked in msvc at least 2000-2015
		using a c-style cast. Broad and long standing compiler support is unlikely to be revoked.
		It also works with reinterpret_cast in gcc and clang 2022.

		So we will be getting the following warning which will have to ignored.

		warning: cast between incompatible pointer to member types from
		‘exo::pExoProgram_MemberFunc’ {aka ‘exo::var (exo::ExoProgram::*)()’} to
		        ‘pExoProgram_MemberFunc’ {aka ‘exo::var (exo::ExoProgram::*)(const exo::var&)’}

		We can ignore such warnings using something like [-Wcast-function-type] in gcc and clang.

		TODO Reimplement as a call to a global function in the shared library.

	*/

	// Not used if callable is only calling global functions in the shared library
	pExoProgram_MemberFunc pmemberfunc_;

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
	Callable(const std::string_view libname, const std::string_view funcname, ExoEnv& ev);

	// Constructor to provide library and function names immediately
	Callable(const std::string_view libname, const std::string_view funcname);

	// Constructor to provide environment immediately
	Callable(ExoEnv& ev);


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
	bool initsmf(ExoEnv& ev, const char* libname, const char* funcname, const bool forcenew = false);

	// Call a shared member function
	var callsmf() const;

	// Call a shared global function
	//var callsgf();

	// Build a path to the shared library file
	static std::string libfilepath(const std::string_view libname);

 protected:

	// Assumes name and ev setup on initialisation then opens library on first call
	//bool init();

	//bool attach(const char* libname, const char* funcname, ExoEnv& ev);
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

}  // namespace exo

#endif // EXODUS_LIBEXODUS_EXODUS_EXOCALLABLE_H_
