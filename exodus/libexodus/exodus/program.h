#ifndef EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_
#define EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_

#include <exodus/exodus.h>
#include <exodus/exocallable.h>

// An Exodus command line program is a C++ class that inherits from
// exo::ExoProgram, created using the programinit() macro. This macro
// opens, but doesnt close the class definition and provides a standard
// int main() entry point that instantiates and runs the class's
// var main() function.
//
// Example:
//   #include <exodus/program.h>
//   programinit()
//   function main() {
//       printl("Hello, world!");
//       return 0;
//   }
//   };
//
// The code between programinit() and }; forms a class that acts like a
// program with global variables and local functions.
//
// You can add subprograms in the same file using programinit(xxx) with a
// unique xxx for each. These subprograms are separate classes, often used
// for tasks like multithreading.
//
// Example subprogram:
//   programinit(sub)
//   function main() {
//       printl("Subprogram running");
//       return 0;
//   }
//   };
//
// Setting EXO_DEBUG=1 disables exception handling for easier debugging:
//   env EXO_DEBUG=1 ./program

// COMPARABLE CODE IN
// program.h library.h

// An Exodus program is a class inheriting from exo::ExoProgram with a
// var main() member function.

// Inline template to instantiate and run an ExoProgram class.
//
// The main2() function, defined below, must call the ExoProgram's main()
// function but cannot do so directly because main2() is generated before
// the class is fully defined, making the class type incomplete. This
// template function solves this by using a forward-declared type as its
// parameter, with all other arguments being known types, allowing the
// compiler to defer instantiation until the class is complete.

namespace exodus_detail {
	template<typename T>
	inline var exoprogram_run(exo::ExoEnv& mv) {
		T exoprogram(mv);
		return exoprogram.main();
	}
}

//[[deprecated("Use }; instead of programexit()")]]
#define programexit(UNUSED) };

//////////////////////
// programinit() macro
//////////////////////
/*
 * 1. Instantiate and run an ExoProgram derived class on execution.
 * 2. Open *and leave open* the definition of the class.
 * Sections 3, 4 and 5 are identical in program.h nd library.h
 */
#define programinit(EXOPROGRAM_PREFIX)                                             \
/*                                                                                 \
 * 0. Forward declare the ExoProgram derived class definition                      \
 */                                                                                \
class EXOPROGRAM_PREFIX##_ExoProgram;                                              \
/*                                                                                 \
 * 1. Define a threadsafe main2() function                                         \
 */                                                                                \
/*_Pragma("GCC diagnostic push")*/                                                     \
/*_Pragma("clang diagnostic ignored \"-Wshadow-field\"")*/                             \
/*_Pragma("GCC diagnostic ignored \"-Winline\"")*/                                     \
static int EXOPROGRAM_PREFIX##main2(int exodus_argc, const char* exodus_argv[], int threadno) { \
    /* Create a block of environment variables for the exodus program */           \
    ExoEnv mv;                                                                     \
    exodus_main(exodus_argc, exodus_argv, mv, threadno);                           \
    /* Create an exodus program */                                                 \
    var result;                                                                    \
    var caught = "";                                                               \
    int exit_status = 0;                                                           \
    if (osgetenv("EXO_DEBUG")) {                                                   \
        /* Call main member function without catching stop, abort etc. and errors */     \
        errputl("Debug Init Thread:", THREADNO, " ", #EXOPROGRAM_PREFIX, " ", SENTENCE); \
        result = exodus_detail::exoprogram_run<EXOPROGRAM_PREFIX##_ExoProgram>(mv);      \
        errputl("Debug Exit Thread:", THREADNO, " ", #EXOPROGRAM_PREFIX, " ", SENTENCE); \
    } else {                                                                       \
        /* Call main member function catching stop, abort etc. and errors  */      \
        try {                                                                      \
            result = exodus_detail::exoprogram_run<EXOPROGRAM_PREFIX##_ExoProgram>(mv).or_default(""); \
        } catch (const ExoStop& e) {                                               \
            result = e.message;                                                    \
        } catch (const ExoAbort& e) {                                              \
            caught = "ExoAbort: ";                                                 \
            result = e.message;                                                    \
            exit_status = 1;                                                       \
        } catch (const ExoAbortAll& e) {                                           \
            caught = "ExoAbortAll: ";                                              \
            result = e.message;                                                    \
            exit_status = 2;                                                       \
        } catch (const VarError& e) {                                              \
            caught = "VarError: ";                                                 \
            result = e.message;                                                    \
            exit_status = 3;                                                       \
            /* stack dump */                                                       \
            errputl(var(e.stack()).convert(FM, "         \n"));                    \
        }/* catch (const std::exception& e) {                                      \
            errputl(e.what(), " - Aborting.");                                     \
            result = 1;                                                            \
        } catch (...) {                                                            \
            var msg = "Error: Unknown exception in " ^ var(__PRETTY_FUNCTION__);   \
            msg.errputl();                                                         \
            auto e = VarError(msg);                                                \
            errputl(var(e.stack()).convert(FM, "         \n"));                    \
            result = OPTIONS.contains("I") ? 0 : 999;                              \
        }*/                                                                        \
    }                                                                              \
    /*disconnect ALL db connections of this thread*/                               \
    disconnectall();                                                               \
    disconnect();                                                                  \
    if (not result.assigned())                                                     \
        result = 101;                                                              \
    if (not result.empty() and result.isnum())                                     \
        exit_status = result;                                                      \
    if (not result.isnum()) {                                                      \
        if (exit_status)                                                           \
            result.errputl(caught);                                                \
        else                                                                       \
            result.outputl(caught);                                                \
    }                                                                              \
    return exit_status;                                                            \
} /* of xxxxxxxx_main2 */                                                          \
/*_Pragma("GCC diagnostic pop")*/                                                      \
/*                                                                                 \
 * 2. Define the standard int main() global namespace/free function entry point    \
 */                                                                                \
int EXOPROGRAM_PREFIX##main(int exodus_argc, const char* exodus_argv[]) {          \
    /* int main simply calls the threadwise main2 as thread 0 */                   \
    return EXOPROGRAM_PREFIX##main2(exodus_argc, exodus_argv, 0);                  \
}                                                                                  \
/*                                                                                 \
 * 3. Open the ExoProgram derived class definition                                 \
 */                                                                                \
/*_Pragma("GCC diagnostic push")*/                                                     \
/*_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")*/                             \
/*_Pragma("GCC diagnostic ignored \"-Winline\"")*/                                     \
class EXOPROGRAM_PREFIX##_ExoProgram : public exo::ExoProgram {                    \
/*_Pragma("GCC diagnostic pop")*/                                                      \
/*                                                                                 \
 * 4. Inherit ExoProgram's constructor using an ExoEnv                             \
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

#endif // EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_
