#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <exodus/var.h>

//using namespace exodus;
using exodus::var;

int main() {

	// Test we can use var.h on its own without the exodus program environment

	// Only needs various include files in /usr/local/include/exodus
	// and, if database functionality is required, postgresql client
	// and libexodus.so installed in system lib

	// Compile simply using at least c++17
	//
	//  g++ test_var.cpp -std=c++17 -lexodus
	//  g++ test_var.cpp -std=c++20 -lexodus

	// Exodus programming environment is not available
	//printl(elapsedtimetext());

	var s = "Test passed";
	s.outputl();

	return 0;
}
