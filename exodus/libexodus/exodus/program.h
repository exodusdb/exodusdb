#ifndef EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_
#define EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_

#include <exodus/exodus.h>

// An Exodus command line program is just a C++ class that inherits from _ExoProgram and
// is followed by a standard free "int main()" function. int main instantiates an object
// of the exodus program class and calls its int main member function.
//
// The above functionality is provided by the programinit() and programexit() macros.
//
// Programmers must write an int main member function between the programinit/exit statements.
//
// The section of an exodus program delimited by programinit/exit statements simulates
// a normal PickOS/multivalue Basic program with global variables and local subroutines.
//
// Unlike PickOS/multivalue Basic, local subroutines can have parameters, return values and private
// variables.

// There must be one matching pair of programinit/exit statements per compilable file with empty
// argument. Additional matching pairs of statements can delineate additional subprograms as long
// as each pair has a unique argument since their argument names are used as part of the class name.

// SIMILAR CODE IN
// program.h library.h

// A program is just a class with an following
// int main() function that creates and calls it

#define programinit(EXOPROGRAM_PREFIX)                     \
_Pragma("GCC diagnostic push")                             \
_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")     \
_Pragma("GCC diagnostic ignored \"-Winline\"")             \
class EXOPROGRAM_PREFIX##_ExoProgram : public ExoProgram { \
_Pragma("GCC diagnostic pop")

//OPTION I=Ignore. Causes error exit to be suppressed
//OPTION D=Debug. Suppress try/catch exception handling so debuggers can catch errors

#define programexit(EXOPROGRAM_PREFIX)                                                         \
 public:                                                                                       \
_Pragma("GCC diagnostic push")                                                                 \
_Pragma("clang diagnostic ignored \"-Wshadow-field\"")                                         \
                                                                                               \
	EXOPROGRAM_PREFIX##_ExoProgram(ExoEnv& mv) : ExoProgram(mv) {}                             \
                                                                                               \
_Pragma("GCC diagnostic pop")                                                                  \
};                                                                                             \
                                                                                               \
_Pragma("GCC diagnostic push")                                                                 \
_Pragma("clang diagnostic ignored \"-Wshadow-field\"")                                         \
_Pragma("GCC diagnostic ignored \"-Winline\"")                                                 \
                                                                                               \
/* main2 is a threadwise version main */                                                       \
static int EXOPROGRAM_PREFIX##main2(int exodus_argc, const char* exodus_argv[], int threadno) {\
                                                                                               \
    /* Create a block of environment variables for the exodus program */                       \
	ExoEnv mv;                                                                                 \
	exodus_main(exodus_argc, exodus_argv, mv, threadno);                                       \
                                                                                               \
    /* Create an exodus program */                                                             \
	EXOPROGRAM_PREFIX##_ExoProgram exoprogram1(mv);                                            \
                                                                                               \
	var result;                                                                                \
	var caught = "";                                                                           \
	int exit_status = 0;                                                                       \
	if (osgetenv("EXO_DEBUG")) {                                                               \
                                                                                               \
	    /* Call main member function without catching stop, abort etc. and errors */           \
		errputl("Debug Init Thread:", THREADNO, " ", #EXOPROGRAM_PREFIX, " ", SENTENCE);       \
		result = exoprogram1.main();                                                           \
		errputl("Debug Exit Thread:", THREADNO, " ", #EXOPROGRAM_PREFIX, " ", SENTENCE);       \
	} else {                                                                                   \
                                                                                               \
        /* Call main member function catching stop, abort etc. and errors  */                  \
		try {                                                                                  \
			result = exoprogram1.main().or_default("");                                        \
		} catch (const ExoStop& e) {                                                           \
			result = e.message;                                                                \
		}/*catch (const ExoLogoff& e) {                                                         \
			result = e.message;                                                                \
		}*/ catch (const ExoAbort& e) {                                                          \
			caught = "ExoAbort: ";                                                             \
			result = e.message;                                                                \
			exit_status = 1;                                                                   \
		} catch (const ExoAbortAll& e) {                                                       \
			caught = "ExoAbortAll: ";                                                          \
			result = e.message;                                                                \
			exit_status = 2;                                                                   \
		} catch (const VarError& e) {                                                          \
			caught = "VarError: ";                                                             \
			result = e.message;                                                                \
			exit_status = 3;                                                                   \
            /* stack dump */                                                                   \
			errputl(var(e.stack()).convert(FM, "         \n"));                                \
		}/* catch (const std::exception& e) {                                                  \
			errputl(e.what(), " - Aborting.");                                                 \
			result = 1;                                                                        \
		} catch (...) {                                                                        \
			var msg = "Error: Unknown exception in " ^ var(__PRETTY_FUNCTION__);               \
			msg.errputl();                                                                     \
			auto e = VarError(msg);                                                            \
			errputl(var(e.stack()).convert(FM, "         \n"));                                \
			result = OPTIONS.contains("I") ? 0 : 999;                                          \
		}*/                                                                                    \
	}                                                                                          \
	/*disconnect ALL db connections of this thread*/                                           \
	disconnectall();                                                                           \
	disconnect();                                                                              \
/*	if (OPTIONS.contains("I")) */                                                              \
/*		result = 0;            */                                                              \
	if (not result.assigned())                                                                 \
		result = 101;                                                                          \
	if (not result.empty() and result.isnum())                                                 \
		exit_status = result;                                                                  \
	if (not result.isnum()) {                                                                  \
		if (exit_status)                                                                       \
			result.errputl(caught);                                                            \
		else                                                                                   \
			result.outputl(caught);                                                            \
	}                                                                                          \
	return exit_status;                                                                        \
} /* of xxxxxxxx_main2 */                                                                      \
                                                                                               \
_Pragma("GCC diagnostic pop")                                                                  \
                                                                                               \
/* The standard main function */                                                               \
int EXOPROGRAM_PREFIX##main(int exodus_argc, const char* exodus_argv[]) {                      \
	/* Call the threadwise main2 as thread 0 */                                                \
	return EXOPROGRAM_PREFIX##main2(exodus_argc, exodus_argv, 0);                              \
}

#endif // EXODUS_LIBEXODUS_EXODUS_PROGRAM_H_
