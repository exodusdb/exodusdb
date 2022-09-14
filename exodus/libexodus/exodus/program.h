#include <exodus/exodus.h>

// the section of an exodus program delimited by programinit/exit statements simulates
// a normal multivalue basic program with global variables and local subroutines.
// unlike multivalue basic, local subroutines can have parameters, return values and private
// variables there can only be one programinit/exit section per compilable file

// an exodus program or library can have zero or more classes. classes are identical to program and
// library sections except that they are not executed unless explicitly called by the other
// sections. For example if you have written a class called MyClass (using "classinit(MyClass)" and
//"classexit(MyClass)" you can create objects of that type in your main program or library, set
// their data elements and/or call their functions MyClass myclass1; myclass1.xyz="xx";
// myclass1.functionx("arg1");

#undef subroutine
#undef function

#define subroutine \
   public:         \
	void

#define function \
   public:       \
	var

// a program is just a class with an following
// int main() function that creates and calls it
#define programinit(PROGRAMNAME) classinit(PROGRAMNAME)

// SIMILAR CODE IN
// program.h programexit
// exodusmacros.h libraryexit()

//OPTION I=Ignore. Causes error exit to be suppressed
//OPTION D=Debug. Suppress try/catch exception handling so debuggers can catch errors
#define programexit(PROGRAMNAME)                                                                                \
	classexit(PROGRAMNAME) int PROGRAMNAME##main2(int exodus__argc, const char* exodus__argv[], int threadno) { \
		MvEnvironment mv;                                                                                       \
		exodus_main(exodus__argc, exodus__argv, mv, threadno);                                                  \
		int result = 0;                                                                                         \
		PROGRAMNAME##ExodusProgram exodusprogram1(mv);                                                          \
		if (osgetenv("EXO_DEBUG")) {                                                                            \
			errputl("Debug Init Thread:", THREADNO, #PROGRAMNAME, SENTENCE);                                    \
			result = exodusprogram1.main().toInt();                                                             \
			errputl("Debug Exit Thread:", THREADNO, #PROGRAMNAME, SENTENCE);                                    \
		} else {                                                                                                \
			try {                                                                                               \
				result = exodusprogram1.main().toInt();                                                         \
			} catch (const MVStop& exceptionx) {                                                                \
				if (exceptionx.description.len())                                                               \
					exceptionx.description.outputl();                                                           \
				if (exceptionx.description.isnum())                                                             \
					result = exceptionx.description.toInt();                                                    \
			} catch (const MVAbort& exceptionx) {                                                               \
				if (exceptionx.description.len())                                                               \
					exceptionx.description.errputl();                                                           \
				if (exceptionx.description.isnum() && exceptionx.description)                                   \
					result = exceptionx.description;                                                            \
				else                                                                                            \
					result = 1;                                                                                 \
			} catch (const MVAbortAll& exceptionx) {                                                            \
				if (exceptionx.description.len())                                                               \
					exceptionx.description.errputl();                                                           \
				if (exceptionx.description.isnum() && exceptionx.description)                                   \
					result = exceptionx.description.toInt();                                                    \
				else                                                                                            \
					result = 2;                                                                                 \
			} catch (const MVLogoff& exceptionx) {                                                              \
				if (exceptionx.description.len())                                                               \
					exceptionx.description.outputl();                                                           \
				if (exceptionx.description.isnum())                                                             \
					result = exceptionx.description.toInt();                                                    \
			}                                                                                                   \
			catch (const VarError& exceptionx)                                                                  \
			{                                                                                                   \
				errputl(exceptionx.description, " - Aborting.");                                                \
				errputl(exceptionx.stack().convert(FM, "\n"));                                                  \
				result = OPTIONS.contains("I") ? 0 : 999;                                                       \
			}                                                                                                   \
		}                                                                                                       \
		/*disconnect ALL connections of this thread*/                                                           \
		disconnectall();                                                                                        \
		disconnect();                                                                                           \
		if (OPTIONS.contains("I"))                                                                              \
			result = 0;                                                                                         \
		return result;                                                                                          \
	}                                                                                                           \
	int PROGRAMNAME##main(int exodus__argc, const char* exodus__argv[]) {                                       \
		return PROGRAMNAME##main2(exodus__argc, exodus__argv, 0);                                               \
	}
