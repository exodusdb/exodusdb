/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// for sleep
#include <thread>
#include <chrono>
#include <random>
#include <unistd.h> //for getpid

#include <exodus/varimpl.h>


// to get whole environment
extern char** environ;

namespace exodus {

void var::ossleep(const int milliseconds) const {

	THISIS("void var::ossleep(const int milliseconds) const")
	assertDefined(function_sig);	 // not needed if *this not used

	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

}

// BOOST RANDOM
// http://www.boost.org/doc/libs/1_38_0/libs/random/random_demo.cpp

//// set the generator type to ...
//// using RNG_typ = boost::minstd_rand;
//using RNG_typ = boost::mt19937;
//boost::thread_specific_ptr<RNG_typ> tss_random_base_generators;
//
//RNG_typ* get_random_base_generator() {
//	// get/init the base generator
//	RNG_typ* threads_random_base_generator =
//		tss_random_base_generators.get();
//	if (!threads_random_base_generator) {
//		tss_random_base_generators.reset(new RNG_typ);
//		threads_random_base_generator = tss_random_base_generators.get();
//		if (!threads_random_base_generator)
//			throw MVError("Could not create random number generator");
//
//		// seed to the os clock (secs since unix epoch)
//		// Caveat: std::time(0) is not a very good truly-random seed.
//		// logputl("Seeding random number generator to system clock");
//		// decimal constants is unsigned only in C99" ie this number exceed max SIGNED
//		// integer
//		//		(*threads_random_base_generator).seed(static_cast<unsigned
//		// int>(std::time(0)+2375472354));
//		(*threads_random_base_generator)
//			.seed(static_cast<unsigned int>(std::time(0) + 2075472354));
//		//(*thread_base_generator).seed(static_cast<unsigned int>(var().ostime().toInt()));
//	}
//	return threads_random_base_generator;
//}

using RNG_typ = std::mt19937;

thread_local std::unique_ptr<RNG_typ> thread_RNG;

// PickOS returns pseudo random integers in the range of 0-4 for rnd(5)
// Exodus is symmetrical for negative numbers
var var::rnd() const {

	THISIS("var var::rnd() const")
	assertNumeric(function_sig);

	// Create a base generator per thread on the heap. Will be destroyed on thread termination.
	if (not thread_RNG.get())
		var(0).initrnd();

	int top = (*this).round().toInt();

	// rnd(-1 ... 1) will always return 0 which is not random
	if (std::abs(top) <= 1)
		throw MVDivideByZero("rnd(" ^ *this ^ ") will always return 0 which is not random");

	// Define a integer range (0 to n-1) or (n+1 to 0)
	int min,max;
	if (top >= 0) {
		min = 0;
		max = top - 1;
	} else {
		min = top + 1;
		max = 0;
	}
	std::uniform_int_distribution<int> uniform_dist(min, max);

	// Generate a pseudo random number from the distribution using the seeded RNG
	return uniform_dist(*thread_RNG);

	//return x;

}

void var::initrnd() const {

	THISIS("void var::initrnd() const")
	assertAssigned(__PRETTY_FUNCTION__);

	// Get a seed for the RNG
	uint64_t seed;
	if (this->unassigned()) {
	} else if (this->isnum()) {

		// Seed from number
		//seed = this->toLong();
		seed = this->toInt();

		if (not seed) {

			// Seed from low resolution clock per second
			//seed.var_int = static_cast<unsigned int>(std::time(0) + 2'075'472'354);

			// Seed from high resolution clock
			seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		}

	} else if (var_typ & VARTYP_STR) {

		// Seed from string
		seed = 1;
		for (size_t ii = 0; ii < var_str.size(); ii++)
			seed *= var_str[ii];
		// seed=MurmurHash64((char*)var_str.data(),int(var_str.size()*sizeof(char)),0);

	}

	// Set the new seed
	thread_RNG = std::make_unique<RNG_typ>(seed);
}

bool var::osgetenv(const char* envvarname) {

	THISIS("bool var::osgetenv(const char* envvarname)")
	assertDefined(function_sig);
	//assertStringMutator(function_sig);
	//ISSTRING(envvarname)

	// return whole environment if blank envvarname
	//if (envvarname.var_str.empty()) {
	if (*envvarname == 0) {

		var_str.clear();
		//var_typ = VARTYP_STR;

		int i = 1;
		char* s = *environ;
		for (; s; i++) {
			//printf("%s\n", s);
			//var_str.append(boost::locale::conv::utf_to_utf<wchar_t>(s));
			var_str.append(s);
			var_str.append("\n");
			s = *(environ + i);
		}
		return true;
	}

	//TIP if you cant seem to osgetenv vars set in bash, then ensure you set them in bash with "export"

	//const char* cvalue = std::getenv(envvarname.var_str.c_str());
	const char* cvalue = std::getenv(envvarname);
	if (cvalue == 0) {
		var_str.clear();
		var_typ = VARTYP_STR;
		return false;
	}

	// *this = var(cvalue);
	var_str = cvalue;
	var_typ = VARTYP_STR;

	return true;
}

bool var::ossetenv(const char* envvarname) const {

	THISIS("bool var::ossetenv(const char* envvarname) const")
	assertString(function_sig);
	//ISSTRING(envvarname)

//#ifdef _MSC_VER
#ifndef setenv

	/* on windows this should be used
	BOOL WINAPI SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);
	*/
	// var("USING PUTENV").outputl();
	// is this safe on windows??
	// https://www.securecoding.cert.org/confluence/display/seccode/POS34-C.+Do+not+call+putenv()+with+a+pointer+to+an+automatic+variable+as+the+argument
	//std::string tempstr = envvarname.toString();
	//tempstr += "=";
	//tempstr += toString();
	// var(tempstr).outputl("temp std:string");
	// std::cout<<tempstr<<" "<<tempstr.length()<<std::endl;

	// this will NOT work reliably since putenv will NOT COPY the local (i.e. temporary)
	// variable string

	//var("putenv " ^ var(tempstr) ).outputl();
	//#pragma warning (disable : 4996)
	//const int result = putenv((char*)(tempstr.c_str()));
	//putenv("EXO_DATA=C:\\");
	//std::cout<<getenv("EXO_DATA");

	//char winenv[1024];
	char* env = (char*)malloc(1024);
	//snprintf(env, 1024, "%s=%s", envvarname.var_str.c_str(), var_str.c_str());
	snprintf(env, 1024, "%s=%s", envvarname, var_str.c_str());
	//std::cout << winenv;
	int result = putenv(env);

	if (result == 0)
		return true;
	else
		return false;

#else
	var("setenv " ^ envvarname ^ "=" ^ (*this)).outputl();
	return setenv((char*)(envvarname.toString().c_str()), (char*)(toString().c_str()), 1);
#endif
}

var var::ospid() const {
	//THISIS("var var::ospid() const")
	return getpid();
}

}  // namespace exodus

