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

#define programexit() \
classexit() \
var main2(int exodus__argc, char *exodus__argv[]); \
int main(int exodus__argc, char *exodus__argv[]) \
{ \
        exodus_main(exodus__argc, exodus__argv); \
        return main2(exodus__argc, exodus__argv); \
} \
var main2(int exodus__argc, char *exodus__argv[]) \
 \
{ \
MvEnvironment mv; \
ExodusProgram exodusprogram1(mv); \
return exodusprogram1.main(); \
}
