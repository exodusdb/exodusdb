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

#define subroutine                                                                 \
    public:                                                                        \
	void

#define function                                                                   \
    public:                                                                        \
	var

// a program is just a class with an following
// int main() function that creates and calls it
#define programinit(PROGRAMNAME) classinit(PROGRAMNAME)

// SIMILAR CODE IN
// program.h programexit
// exodusmacros.h libraryexit()

//OPTION I=Ignore causes error exit to be suppressed
#define programexit(PROGRAMNAME)                                                   \
	classexit(PROGRAMNAME)                                                         \
	int PROGRAMNAME##main2(int exodus__argc, const char* exodus__argv[], int threadno)   \
	{                                                                              \
		MvEnvironment mv;                                                          \
		exodus_main(exodus__argc, exodus__argv, mv, threadno);                     \
		try                                                                        \
		{                                                                          \
			PROGRAMNAME##ExodusProgram exodusprogram1(mv);                         \
			return OPTIONS.index("I") ? 0 : exodusprogram1.main().toInt();         \
		}                                                                          \
		catch (MVStop exceptionx)                                                  \
		{                                                                          \
			if (exceptionx.description.length())                                   \
				exceptionx.description.outputl();                                  \
			if (exceptionx.description.isnum())                                    \
				exit(OPTIONS.index("I") ? 0 : exceptionx.description.toInt());     \
			else                                                                   \
				exit(0);                                                           \
		}                                                                          \
		catch (MVAbort exceptionx)                                                 \
		{                                                                          \
			if (exceptionx.description.length())                                   \
				exceptionx.description.outputl();                                  \
			if (exceptionx.description.isnum() && exceptionx.description)          \
				exit(exceptionx.description);                                      \
			else                                                                   \
				exit(OPTIONS.index("I") ? 0 : 1);                                  \
		}                                                                          \
		catch (MVAbortAll exceptionx)                                              \
		{                                                                          \
			if (exceptionx.description.length())                                   \
				exceptionx.description.outputl();                                  \
			if (exceptionx.description.isnum())                                    \
				exit(OPTIONS.index("I") ? 0 : exceptionx.description.toInt());     \
			else                                                                   \
				exit(OPTIONS.index("I") ? 0 : 2);                                  \
		}                                                                          \
		/*catch (MVError exceptionx)                                               \
		{                                                                          \
			printl(exceptionx.description, " - Aborting.");                        \
			printl(exceptionx.stack.convert(FM, "\n"));                            \
			exit(OPTIONS.index("I") ? 0 : 999);                                    \
		}*/                                                                        \
		return 0;                                                                  \
	}                                                                              \
	int PROGRAMNAME##main(int exodus__argc, const char* exodus__argv[])            \
	{                                                                              \
		return PROGRAMNAME##main2(exodus__argc, exodus__argv, 0);                  \
	}
