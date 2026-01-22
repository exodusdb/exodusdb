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

////module #include <string>

#include <var/varimpl.h>
#include <var/vardefs.h>
#include <var/timebank.h>

static bool desynced_with_stdio = false;

namespace exo {

using let = const var;

// output/errput/logput not threadsafe but probably not a problem
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM_OR_NOT std::lock_guard guard(global_mutex_threadstream);
//#define LOCKIOSTREAM_OR_NOT


/////////
// output
/////////

//NOT THREADSAFE? warning put() is not threadsafe whereas output(), errput() and logput() are threadsafe
CVR  var_os::put(std::ostream& ostream1) const {

	THISIS("CVR  var::put(std::ostream& ostream1) const")
	assertString(function_sig);

//	// prevent output to cout suppressing output to cout (by non-exodus routines)
//	// http://gcc.gnu.org/ml/gcc-bugs/2006-05/msg01196.html
//	// TODO optimise by calling once instead of every call to output()
//	if (!desynced_with_stdio) {
//		std::ios::sync_with_stdio(false);
//		desynced_with_stdio = true;
//	}

	// verify conversion to UTF8
	// std::string tempstr=(*this).toString();

	ostream1.write(var_str.data(), static_cast<std::streamsize>(var_str.size()));
	return *this;
}

// output -> cout which is buffered standard output
///////////////////////////////////////////////////

// output()
CVR  var_os::output(in prefix) const {
	THISIS("CVR  var::output(in prefix) const")
	prefix.put(std::cout);
	return this->put(std::cout);
}

// outputl()
CVR  var_os::outputl(in prefix) const {
	// FLUSHED not buffered
	THISIS("CVR  var::outputl(in prefix) const")
	prefix.put(std::cout);
	this->put(std::cout);
	std::cout << std::endl;
	return *this;
}

// outputt()
CVR  var_os::outputt(in prefix) const {
	// BUFFERED not flushed
	THISIS("CVR  var::outputt(in prefix) const")
	std::cout << "\t";
	prefix.put(std::cout);
	std::cout << "\t";
	this->put(std::cout);
	return *this;
}

// errput -> cerr which is unbuffered standard error
////////////////////////////////////////////////////

// errput
CVR  var_os::errput(in prefix) const {
	// BUFFERED? not flushed
	THISIS("CVR  var::errput(in prefix) const")
	//prefix.put(std::cerr);
	//return this->put(std::cerr);
	std::cerr << prefix;
	std::cerr << this->convert(_ALL_FMS, _VISIBLE_FMS);
	return *this;
}

// errputl
CVR  var_os::errputl(in prefix) const {
	// FLUSHED not buffered
	THISIS("CVR  var::errputl(in prefix) const")
	//prefix.put(std::cerr);
	//this->put(std::cerr);
	std::cerr << prefix;
	std::cerr << this->convert(_ALL_FMS, _VISIBLE_FMS);
	std::cerr << std::endl;
	return *this;
}

// logput -> clog which is a buffered version of cerr standard error output
///////////////////////////////////////////////////////////////////////////

// logput
CVR  var_os::logput(in prefix) const {
	// BUFFERED not flushed
	THISIS("CVR  var::logput(in prefix) const")
	//prefix.put(std::clog);
	std::clog << prefix;
	std::clog << this->convert(_ALL_FMS, _VISIBLE_FMS);
	return *this;
}

// logputl
CVR  var_os::logputl(in prefix) const {
	// BUFFERED not flushed
	THISIS("CVR  var::logputl(in prefix) const")
	//prefix.put(std::clog);
	//this->put(std::clog);
	std::clog << prefix;
	std::clog << this->convert(_ALL_FMS, _VISIBLE_FMS);
//	std::clog << std::endl;
	std::clog << "\n";
	return *this;
}

} // namespace exo
