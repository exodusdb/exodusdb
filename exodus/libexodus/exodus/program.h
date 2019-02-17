#include <exodus/exodus.h>

//the section of an exodus program delimited by programinit/exit statements simulates
//a normal multivalue basic program with global variables and local subroutines.
//unlike multivalue basic, local subroutines can have parameters, return values and private variables
//there can only be one programinit/exit section per compilable file

//an exodus program or library can have zero or more classes. classes are identical to program and 
//library sections except that they are not executed unless explicitly called by the other sections.
//For example if you have written a class called MyClass
//(using "classinit(MyClass)" and "classexit(MyClass)"
//you can create objects of that type in your main program or library, set their data elements
//and/or call their functions
//MyClass myclass1;
//myclass1.xyz="xx";
//myclass1.functionx("arg1");

#undef subroutine
#undef function
#define subroutine public: void
#define function public: var

//a program is just a class with an following
//int main() function that creates and calls it
#define programinit() \
classinit()

//SIMILAR CODE IN
//program.h programexit
//exodusmacros.h libraryexit()

#define programexit() \
classexit() \
int main(int exodus__argc, char *exodus__argv[]) \
{ \
	MvEnvironment mv; \
	exodus_main(exodus__argc, exodus__argv, mv); \
	try \
	{ \
		ExodusProgram exodusprogram1(mv); \
		return exodusprogram1.main(); \
	} \
	catch (MVStop exceptionx) \
	{ \
		if (exceptionx.description.length()) \
			exceptionx.description.outputl();\
		if (exceptionx.description.isnum()) \
			exit(exceptionx.description); \
		else \
			exit(0); \
	} \
	catch (MVAbort exceptionx) \
	{ \
		if (exceptionx.description.length()) \
			exceptionx.description.outputl();\
		if (exceptionx.description.isnum()) \
			exit(exceptionx.description); \
		else \
			exit(1); \
	} \
	catch (MVException exceptionx) \
	{ \
                printl(exceptionx.description, " - Aborting."); \
                printl(exceptionx.stack.convert(FM,L"\n")); \
		exit(999); \
	} \
	return 0; \
}

//same as programexit but no try/catch block so can go into a debugger
#define debugprogramexit() \
classexit() \
int main(int exodus__argc, char *exodus__argv[]) \
{ \
	MvEnvironment mv; \
	exodus_main(exodus__argc, exodus__argv, mv); \
	ExodusProgram exodusprogram1(mv); \
	int result=exodusprogram1.main(); \
	print("Debugging. Program finished. Press Enter"); \
	input(); \
	return result; \
}
