#ifndef LIBEXODUS_LIBRARY_H_
#define LIBEXODUS_LIBRARY_H_

#include <exodus/exodus.h>

#ifndef EXO_LIBRARY
#   define EXO_LIBRARY
#endif

// PROGRAM, LIBRARY or FUNCTION?
//
// An Exodus program is technically any C++ class derived from the 
// ExoProgram base class and that has a main() function. Exodus 
// programs can be built either as standalone programs or as shared 
// libraries. As standalone programs they may be called Exodus 
// standalone programs, Exodus programs or just programs. As shared 
// libraries they may be called Exodus libraries or just libraries. 
// Regardless of what they are called, under the hood they are 
// fully programs in the sense of allowing a set of related functions 
// to operate on a private set of data. They are two different ways of 
// packaging the same program structure
//
// Exodus libraries have a callable interface via their main() 
// function. It can have any Exodus or C++ arguments, including 
// references and constants. It always returns a var.
//
// Exodus libraries are often called "library functions" because their 
// only interface is their main() function, hidden 
// behind a callable object with the same name as the library. They 
// also commonly retain little or no state between calls. They are, 
// however, full-blown objects. Their member functions and data, 
// except main(), have no public interface.
//
// Exodus libraries are implemented as function objects in the caller. 
// They retain state between calls by creating an instance of the 
// library class on first call to their function object. Subsequent 
// calls use this instance.
//
// Calling a library function with fewer than its defined number of 
// arguments is supported by providing default constructed objects for 
// any or all missing arguments starting from the rightmost.

// CREATING LIBRARIES
//
// To create an Exodus program as a shared library (.so file) instead 
// of a command line executable, include library.h instead of 
// program.h.
//
// The libraryinit() macro creates a factory function to create an 
// instance of the program in the shared library (.so file) at 
// runtime. It also starts, but leaves open, a class definition 
// inheriting from exo::ExoProgram.
//
// It is the Exodus programmer's task to complete the class by 
// providing a func main(), any other functions or data members, 
// then close it with a }; as follows:
//
// Example library (func1.cpp):
//   #include <exodus/library.h>
//   libraryinit()
//   func main(/*args*/) {
//       /*code*/
//       return "";
//   }
//   /*optional sibling funcs and data*/
//   };

// USING LIBRARIES
//
// To use a library function in any other Exodus program or library, 
// include its .h file and call it like a regular function. 
//
// exodus/cli/compile automatically generates the necessary header file 
// for every library.
//
// Note that the .h file must be included *after* the programinit() 
// line because it must share the program’s environment with the 
// library.
//
// Example program (prog1.cpp):
//   #include <exodus/program.h>
//   programinit()
//   #include <func1.h>
//   func main() {
//       /*code*/ func1(/*args*/);
//       return 0;
//   }
//   /*sibling funcs and data*/
//   };

// RUNTIME
//
// Exodus libraries are dynamically loaded at runtime on first call to 
// the function object.
//
// You can change which library a function loads at runtime, even 
// after calling it. For example, if libraries l1, l2, and l3 have the 
// same main() arguments, include l1.h, set l1 = "l2", and call l1() 
// to load l2. Later, set l1 = "l3", and calling l1() will switch to 
// l3, all without restarting. This enables you to reconfigure 
// high-performance C++ applications on the fly.

// See Also
//
// program.h library.h
//
// Both create C++ classes inherited from exo::ExoProgram, with a var 
// main() function, closed by };. Programs run as executables; 
// libraries load dynamically.

// FACTORY FUNCTION
///////////////////
//
// An inline template to create and delete an ExoProgram class for dynamic loading.
//
// PROBLEM: The factory function (exoprogram_createdelete_xxx) must 
// create an ExoProgram object and get a member function pointer to its 
// main() function. It cannot do so directly because the class is 
// defined after the factory, so its type is incomplete.
//
// SOLUTION: We provide the following exoprogram_createdelete_impl() 
// factory function. This function has known arguments so that 
// exoprogram_createdelete_xxx may call it. This _impl() function is 
// templated on a forward-declared ExoProgram type. That means that 
// the compiler can defer instantiating the template (finally 
// compiling it) until after the ExoProgram is fully defined.
// TODO: Consider moving to exocallable.h for reuse.
namespace exodus_detail {
	template<typename T>
	inline void exoprogram_createdelete_impl(
		exo::pExoProgram& pexoprogram,
		exo::ExoEnv& ev,
		exo::pExoProgram_MemberFunc& pmemberfunc
	) {
		if (pexoprogram) {
			delete pexoprogram;
			pexoprogram = nullptr;
			pmemberfunc = nullptr;
		} else {
			pexoprogram = new T(ev);
			pmemberfunc = reinterpret_cast<exo::pExoProgram_MemberFunc>(&T::main);
		}
	}
}

// libraryexit() MACRO
//////////////////////
//
// Deprecated: Use }; instead of libraryexit()
#define libraryexit(UNUSED) };

#define PUBLIC __attribute__((visibility("default")))


// libraryinit() MACRO
//////////////////////
//
// 1. Define a factory function for an ExoProgram.
// 2. Open *and leave open* the definition of the class.
// Sections 3, 4, and 5 are identical in program.h and library.h
//
#define libraryinit(EXOPROGRAM_PREFIX)                                             \
                                                                                   \
/* 1. Forward declare the ExoProgram-derived class being created */                \
class EXOPROGRAM_PREFIX##_ExoProgram;                                              \
                                                                                   \
/* 2. Define a factory function for dynamic loading              */                \
extern "C" PUBLIC void exoprogram_createdelete_##EXOPROGRAM_PREFIX(                \
    exo::pExoProgram& pexoprogram, exo::ExoEnv& ev,                                \
    exo::pExoProgram_MemberFunc& pmemberfunc) {                                    \
    exodus_detail::exoprogram_createdelete_impl<EXOPROGRAM_PREFIX##_ExoProgram>(   \
        pexoprogram, ev, pmemberfunc);                                             \
}                                                                                  \
                                                                                   \
/* 3. Open the ExoProgram-derived class definition               */                \
class PUBLIC EXOPROGRAM_PREFIX##_ExoProgram : public exo::ExoProgram {             \
                                                                                   \
/* 4. Inherit ExoProgram's constructor using an ExoEnv           */                \
public:                                                                            \
    using ExoProgram::ExoProgram;                                                  \

// User code follows, ending with }; to close the class

// 5. The class scope is left open - observe that there is no closing '};' in the macro.
//
// a. User code (including func main(){...}) MUST follow the above macro.
// b. MUST BE TERMINATED BY }; to close the above class scope. e.g.
//
// func main() {
//     println("Hello World");
//     return 0;
// }
//
// }; // <- MANDATORY

#endif // LIBEXODUS_LIBRARY_H_

