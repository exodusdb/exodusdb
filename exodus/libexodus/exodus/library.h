#ifndef EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
#define EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_

#include <exodus/exodus.h>

#ifndef EXO_LIBRARY
#   define EXO_LIBRARY
#endif

// Exodus programs can be built as shared libraries and may then be called 
// Exodus libraries, although they are functionally identical internally.
//
// Both programs and libraries allow a set of functions to work on a private 
// set of data. Library functions, however, have a callable interface via 
// their main() function. The main() function of libraries can have any 
// Exodus or C++ arguments, including references and constants, but it always 
// returns a var.
//
// Exodus libraries are often called "library functions" because their only 
// interface is their main() function, and they typically retain no state 
// between calls. They are however full blown objects. It is just that all 
// their member functions and data have no public interface.
//
// The inclusion of library.h instead of program.h causes Exodus programs to be built as shared libraries (.so files) instead of command line executables.
//
// Exodus libraries are dynamically loaded and called as ordinary functions by other Exodus programs and libraries.
//
// You can switch which library is loaded at runtime, even after using it, 
// offering rare flexibility to configure and reconfigure applications while 
// running. So if you have three libraries l1, l2, and l3 all with the same 
// interface (main arguments are identical), and you include l1.h, you can set 
// l1 = "l2" and calling l1() will use l2 instead, and afterwards you can set 
// l1 = "l3" and calling l1() will actually call l3.
//
// The libraryinit() macro creates a factory function to create an 
// instance of the program in the shared library (.so file) at runtime. 
// It also starts but leaves open a class definition inheriting from 
// exo::ExoProgram that is to be completed by the Exodus programmer.
//
// Example library (func1.cpp):
//   #include <exodus/library.h>
//   libraryinit()
//   function main(/*args*/) {
//       /*code*/
//       return "";
//   }
//   /*sibling funcs and data*/
//   };
//
// To use a library function, include its .h file and call it like a regular function. libraries .h files are generated automatically by exodus/cli/compile.
// The .h file must be included after the programinit() line since it needs access to its parent's environment (ExoEnv).
//
// Example program (prog1.cpp):
//   #include <exodus/program.h>
//   programinit()
//   #include <func1.h>
//   function main() {
//       /*code*/ func1(/*args*/);
//       return 0;
//   }
//   /*sibling funcs and data*/
//   };
//
// Each library function is a class, so its main() function can access other
// member functions and data in the same class, just like a C++ object.
// Multiple library functions can be defined in one file using libraryinit(xxx)
// with a unique xxx for each, creating separate classes callable as functions.
//
// COMPARABLE CODE IN
// program.h library.h
// Both create C++ classes inherited from exo::ExoProgram, with a var main()
// function, closed by };. Programs run as executables; libraries load dynamically.

// Inline template to create and delete an ExoProgram class for dynamic loading.
//
// PROBLEM: The factory function (exoprogram_createdelete_xxx) must 
// create an ExoProgram object and get a member function pointer to its 
// main() function. It cannot do so directly because the class is 
// defined after the factory, so its type is unknown.
//
// SOLUTION: We provide the following exoprogram_createdelete_impl() 
// factory function. This function has known arguments so that 
// exoprogram_createdelete_xxx may call it. This 
// _impl() function is templated on a forward-declared ExoProgram type. 
// That means that the compiler can defer instantiating it (fully 
// compiling it) until after the ExoProgram is fully defined.
// TODO: Consider moving to exocallable.h for reuse.
namespace exodus_detail {
    template<typename T>
    inline void exoprogram_createdelete_impl(exo::pExoProgram& pexoprogram, exo::ExoEnv& mv, exo::pExoProgram_MemberFunc& pmemberfunc) {
        if (pexoprogram) {
            delete pexoprogram;
            pexoprogram = nullptr;
            pmemberfunc = nullptr;
        } else {
            pexoprogram = new T(mv);
            pmemberfunc = reinterpret_cast<exo::pExoProgram_MemberFunc>(&T::main);
        }
    }
}

//[[deprecated("Use }; instead of libraryexit()")]]
#define libraryexit(UNUSED) };

//////////////////////
// libraryinit() macro
//////////////////////
/*
 * 1. Define a factory function for an ExoProgram.
 * 2. Open *and leave open* the definition of the class.
 * Sections 3, 4 and 5 are identical in program.h nd library.h
 */
#define libraryinit(EXOPROGRAM_PREFIX)                                             \
/*                                                                                 \
 * 1. Forward declare the ExoProgram-derived class being created                  \
 */                                                                                \
class EXOPROGRAM_PREFIX##_ExoProgram;                                              \
/*                                                                                 \
 * 2. Define a factory function for dynamic loading                               \
 */                                                                                \
/*ma("clang diagnostic push")*/                                                   \
/*ma("clang diagnostic ignored \"-Wmissing-prototypes\"")*/                       \
extern "C" PUBLIC void exoprogram_createdelete_##EXOPROGRAM_PREFIX(                \
    exo::pExoProgram& pexoprogram, exo::ExoEnv& mv,                                \
    exo::pExoProgram_MemberFunc& pmemberfunc) {                                    \
    exodus_detail::exoprogram_createdelete_impl<EXOPROGRAM_PREFIX##_ExoProgram>(   \
        pexoprogram, mv, pmemberfunc);                                             \
}                                                                                  \
/*ma("clang diagnostic pop")*/                                                    \
/*                                                                                 \
 * 3. Open the ExoProgram-derived class definition                                \
 */                                                                                \
/*_Pragma("GCC diagnostic push")*/                                                     \
/*_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")*/                             \
/*_Pragma("GCC diagnostic ignored \"-Winline\"")*/                                     \
class EXOPROGRAM_PREFIX##_ExoProgram : public exo::ExoProgram {                    \
/*_Pragma("GCC diagnostic pop")                */                                      \
/*                                                                                 \
 * 4. Inherit ExoProgram's constructor using an ExoEnv                            \
 */                                                                                \
public:                                                                            \
    /*_Pragma("clang diagnostic push")*/                                               \
    /*_Pragma("clang diagnostic ignored \"-Wshadow-field\"")*/                         \
    using ExoProgram::ExoProgram;                                                  \
    /*_Pragma("clang diagnostic pop")*/                                                \
/*                                                                                 \
 * User code follows, ending with }; to close the class                            \
 */                                                                                \
/*                                                                                 \
 * 5. The class scope is left open - observe that there is no closin '};' closing. \
 */
// 1. User code (including function main(){...}) MUST follow the above macro.
// 2. MUST BE TERMINATED BY }; to close the above class scope. e.g.
//
// function main() {
//     println("Hello World");
//     return 0;
// }
//
// }; // MANDATORY

#endif // EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
