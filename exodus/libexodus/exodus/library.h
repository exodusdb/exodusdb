#ifndef EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_
#define EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_

#include <exodus/exodus.h>

#ifndef EXO_LIBRARY
#   define EXO_LIBRARY
#endif

//[[deprecated("Use '};' instead of libraryinit()")]]
#define libraryexit(UNUSED) };

// A library section is a just a c++ class stored in an so/dll shared library that can be
// called like a function or subroutine from an exodus program or other library.
//
//	#include <exodus/library.h>
//
//  libraryinit()
//  function main(parameters ...) {
//   ...
//  }
//  }; // Close the class.
//
// A library can have multiple libraryinit/exit sections and all are publically available
//
//  libraryinit(funcx)
//  function main(parameters ...) {
//   ...
//  }
//  }; // Close the class.
//
// by including its xxxxxxxx.h file name in other programs and libraries.
//
// A library's header file is generated when compiled using exodos/cli/compile which also calls c++.
//
// A library can also include other classes. Classes are identical to library sections but they are
// private to the library since they are not published in its xxxxxxxx.h file.

// COMPARABLE CODE IN
// program.h library.h

// a library section is just a class plus a global exported factory function that allows the
// class to be instantiated and its main(...) function called from another program
// via so/dll delay loading AND share the mv environment variables of the calling program.

// Inline template for factory implementation
// TODO move to exocallable.h
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

//////////////////////////////////////////////////////////
// The libraryinit macro opens an ExoProgram derived class
// and provides a dynamically linkable factory function
//////////////////////////////////////////////////////////

#define libraryinit(EXOPROGRAM_PREFIX)                                             \
                                                                                   \
/*                                                                                 \
 * 1. Forward declare the ExoProgram derived class being created                   \
 */                                                                                \
class EXOPROGRAM_PREFIX##_ExoProgram;                                              \
                                                                                   \
/*                                                                                 \
 * 2. Define a factory function                                                    \
 */                                                                                \
_Pragma("clang diagnostic push")                                                   \
_Pragma("clang diagnostic ignored \"-Wmissing-prototypes\"")                       \
extern "C" PUBLIC void exoprogram_createdelete_##EXOPROGRAM_PREFIX(                \
    exo::pExoProgram& pexoprogram, exo::ExoEnv& mv,                                \
    exo::pExoProgram_MemberFunc& pmemberfunc) {                                    \
    exodus_detail::exoprogram_createdelete_impl<EXOPROGRAM_PREFIX##_ExoProgram>(   \
        pexoprogram, mv, pmemberfunc);                                             \
}                                                                                  \
_Pragma("clang diagnostic pop")                                                    \
                                                                                   \
/*                                                                                 \
 * 3. Open the ExoProgram derived class definition                                 \
 */                                                                                \
_Pragma("GCC diagnostic push")                                                     \
_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")                             \
_Pragma("GCC diagnostic ignored \"-Winline\"")                                     \
                                                                                   \
class EXOPROGRAM_PREFIX##_ExoProgram : public exo::ExoProgram {                    \
_Pragma("GCC diagnostic pop")                                                      \
                                                                                   \
/*                                                                                 \
 * 4. Inherit ExoProgram's constructor using an ExoEnv                             \
 */                                                                                \
public:                                                                            \
    _Pragma("clang diagnostic push")                                               \
    _Pragma("clang diagnostic ignored \"-Wshadow-field\"")                         \
                                                                                   \
    using ExoProgram::ExoProgram;                                                  \
    /*EXOPROGRAM_PREFIX##_ExoProgram(exo::ExoEnv& mv) : exo::ExoProgram(mv) {}*/   \
    _Pragma("clang diagnostic pop")

#endif // EXODUS_LIBEXODUS_EXODUS_LIBRARY_H_