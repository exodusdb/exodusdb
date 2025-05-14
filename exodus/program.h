#ifndef EXODUS_PROGRAM_H_
#define EXODUS_PROGRAM_H_

#include <exodus/exodus.h>

// An Exodus program is a C++ class inheriting from exo::ExoProgram and
// contains a var main() member function with no arguments. It represents
// a "program" with "global" variables and "local" functions.
//
// The programinit() macro embeds an Exodus program (class) into a
// command-line executable. Upon execution, a program object is created
// and its var main() function called.
// The macro also starts the class (program), which the programmer completes.
//
// Example:
//   #include <exodus/program.h>
//   programinit()
//   func main() {
//       /*code*/;
//       return 0;
//   }
//   /*global vars, local functions*/
//   };

// Setting EXO_DEBUG=1 disables exception handling for easier debugging:
//   env EXO_DEBUG=1 ./program
//
// SEE ALSO
// library.h - For embedding Exodus programs in shared libraries.

// A templated function to instantiate the class and run main().
namespace exo {
	template<typename T>
	int run_exoprogram(int argc, const char* argv[], int threadno = 0) {
		ExoEnv ev(argc, argv, threadno);
		T exoprogram(ev);
		return exoprogram.run_main(
			static_cast<exo::var (ExoProgram::*)()>(&T::main),
			argc, argv
		);
	}
}

#define programinit(PREFIX)                                         \
                                                                    \
/* 1. Forward declare the ExoProgram-derived class */               \
class PREFIX##_ExoProgram;                                          \
                                                                    \
/* for backward compatibility during transition only*/              \
int PREFIX##main2(int argc, const char* argv[], int threadno) {     \
	return exo::run_exoprogram<PREFIX##_ExoProgram>(argc, argv, threadno);\
}                                                                   \
/* 2. Define int main() to instantiate and run the program */       \
int PREFIX##main(int argc, const char* argv[]) {                    \
	return exo::run_exoprogram<PREFIX##_ExoProgram>(argc, argv);    \
}                                                                   \
                                                                    \
/* 3. Open the ExoProgram-derived class definition */               \
class PREFIX##_ExoProgram : public exo::ExoProgram {                \
public:                                                             \
	using ExoProgram::ExoProgram;

// 4. The above class scope is left open.
//    Observe that there is no closing '};'.
//    a. User code (func main(){...}) MUST follow the above macro.
//    b. MUST BE TERMINATED BY }; to close the above class scope.
//
// func main() {
//     println("Hello World");
//     return 0;
// }
//
// }; // programexit()

#define programexit(UNUSED) };

#endif // EXODUS_PROGRAM_H_
