#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing

#ifdef EXO_MODULE
	import var;
#else
#	include <exodus/var.h>
#endif

// We could bring the whole of exo namespace
//using namespace exo;
//or just the var with all its many member functions 
using exo::var;

int main() {

	// Test we can use var.h on its own without the exodus program environment

	// Only needs various include files in /usr/local/include/exodus
	// and, if database functionality is required, postgresql client
	// and libexodus.so installed in system lib

	// Compile simply using at least c++17
	//
	//  g++ test_var.cpp -std=c++17 -lexodus
	//  g++ test_var.cpp -std=c++20 -lexodus

	// If exodus was built with clang modules on Ubuntu 24.04
	//	c++ test_var.cpp  -std=c++2c  -DEXO_MODULE=1 -fmodule-file=var=/usr/local/lib/var.pcm -fmodule-file=std=/usr/local/lib/std.pcm -lexodus

	// Exodus programming environment is not available
	//printl(elapsedtimetext());

	var s = "Test passed";
	s.outputl();

	return 0;
}
