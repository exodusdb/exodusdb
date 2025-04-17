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

#include <unistd.h> //for getpid
#if EXO_MODULE
	import std;
#else
#	include <thread> //for sleep
#	include <chrono>
#	include <random>
#	include <memory> //for make_unique
#endif

#include <exodus/var.h>
#include <exodus/varimpl.h>

// to get whole environment
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
extern char** environ;
#pragma GCC diagnostic pop

namespace exo {

void var::ossleep(const int milliseconds) {

	THISIS("void var::ossleep(const int milliseconds)")
//	assertVar(function_sig);	 // not needed if *this not used

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
//			throw VarError("Could not create random number generator");
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
var  var::rnd() const {

	THISIS("var  var::rnd() const")
	assertNumeric(function_sig);

	// Create a base generator per thread on the heap. Will be destroyed on thread termination.
	if (not thread_RNG.get())
		var(0).initrnd();

	int top = (*this).round().toInt();

	// rnd(-1 ... 1) will always return 0 which is not random
	if (std::abs(top) <= 1)
		UNLIKELY
		throw VarDivideByZero("rnd(" ^ *this ^ ") will always return 0 which is not random");

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
	std::uint64_t seed;
	if (this->isnum()) {

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

	} else UNLIKELY {
		// Cant get here because already checked but need to evade a warning
		// warning: ‘seed’ may be used uninitialized in this function [-Wmaybe-uninitialized]
		seed = 0;
		throw VarUnassigned(__PRETTY_FUNCTION__);
	}

	// Set the new seed
	thread_RNG = std::make_unique<RNG_typ>(seed);
}

static thread_local std::string thread_environ;

bool var::osgetenv(SV envcode) {

	// TIP if you cant seem to set osgetenv vars in bash/sh
	// then ensure you set them with "export"
	// otherwise child processes don't see them.

	THISIS("bool var::osgetenv(SV envcode)")
	assertVar(function_sig);
	//assertStringMutator(function_sig);
	//ISSTRING(envcode)

	// Initialise the thread local cache if not already done
	if (thread_environ.empty()) {

		// The cache should already be empty actually.
		thread_environ.clear();

		// The cache will start with a FM to speed up
		// later location of specific codes
		thread_environ.push_back(FM_);

		// Read the global environ pairs into a FM separated std::string list of pairs
		// e.g. *cstr = HOME=/root
		// man environ
		// extern char **environ;
		// environ is a pointer to an array of pointers to char* env pairs like xxx=yyy
		// the last pointer in the array is nullptr
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		for (auto envn = 0; environ[envn]; ++envn) {
			//printf("%s\n", environ[i]);
			thread_environ.append(environ[envn]);
			thread_environ.push_back(FM_);
		}
#pragma GCC diagnostic pop

		// Remove the last (or only) trailing FM_
		thread_environ.pop_back();
	}

	// return whole environment if blank envcode
	if (envcode.empty()) {

		// Return a copy of the cached list of pairs
		if (thread_environ.size() >= 2)
			// excluding the initial FM
			var_str = thread_environ.substr(1);
		else
			var_str.clear();
		var_typ = VARTYP_STR;

		return true;

	}

	// Example:
	// target "^a="
	// environ "^a=1^b=2^c=3"

	// Create the target like "^a="
	std::string target(_FM);
	target.append(envcode);
	target.push_back('=');

	// *this defaults to empty string
	var_str.clear();
	var_typ = VARTYP_STR;

	// Search for "^a="
	// Example: pos -> 0
	auto pos = thread_environ.find(target);

	// If not found then return false (no error)
	// If environ was "^x=1^y=2" then pos->npos
	if (pos == std::string::npos)
		return false;

	// Move the pointer to one after the target
	// Assume not > max str size
	// Example: pos -> 3
	pos += target.size();

	// If nothing after the target then success
	// If environ was just "^a=" then 3 >= 3 true
	if (pos >= thread_environ.size())
		return true;

	// Search for the next FM after the target
	// Example: pos = 3. pos2 -> 4
	auto pos2 = thread_environ.find(FM_, pos);

	// If not found then extract everything
	// after the "=" and succeed
	// If environ was "^a=1" then extract from pos 3
	if (pos2 == std::string::npos) {
		var_str = thread_environ.substr(pos);
		return true;
	}

	// Extract everything after the target
	// up to the next FM and succeed
	// Example: pos = 3, length = 4 - 3 = 1
	var_str = thread_environ.substr(pos, pos2 - pos);
	return true;

}

void var::ossetenv(SV envcode) const {

	THISIS("bool var::ossetenv(SV envcode) const")
	assertString(function_sig);

	// Create the new pair like "^a=c"
	std::string new_pair(_FM);
	new_pair.append(envcode);
	new_pair.push_back('=');
	new_pair.append(this->var_str);

	// Start to create the old pair like "^a="
	std::string old_pair(_FM);
	old_pair.append(envcode);
	old_pair.push_back('=');

	// Try to get the old value from environ
	// The old value may be ""
	if (var oldvalue; oldvalue.osgetenv(envcode)) {

		// Finish creating the old pair "^a=b"
		old_pair.append(oldvalue.var_str);
//TRACE(old_pair)
		// Replace in thread_environ, old pair with new pair
		auto pos = thread_environ.find(old_pair.data(), 0, old_pair.length());
		if (pos != old_pair.npos)
			thread_environ.replace(pos, old_pair.length(), new_pair.data(), new_pair.length());

	} else {
//TRACE(new_pair)
		// Append new pair
		thread_environ.append(new_pair);
	}

	return;
}

var  var::ospid() {
	//THISIS("var  var::ospid() const")
	return getpid();
}

var  var::ostid() {
	//THISIS("var  var::ospid() const")
	return gettid();
}

}  // namespace exo

