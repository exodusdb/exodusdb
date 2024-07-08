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

//#include <string>

#include <exodus/varimpl.h>

static bool desynced_with_stdio = false;

namespace exo {

// output/errput/logput not threadsafe but probably not a problem
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM_OR_NOT std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM_OR_NOT


/////////
// output
/////////

//warning put() is not threadsafe whereas output(), errput() and logput() are threadsafe
CVR var::put(std::ostream& ostream1) const {

	THISIS("CVR var::put(std::ostream& ostream1) const")
	assertString(function_sig);

	// prevent output to cout suppressing output to cout (by non-exodus routines)
	// http://gcc.gnu.org/ml/gcc-bugs/2006-05/msg01196.html
	// TODO optimise by calling once instead of every call to output()
	if (!desynced_with_stdio) {
		std::ios::sync_with_stdio(false);
		desynced_with_stdio = true;
	}

	// verify conversion to UTF8
	// std::string tempstr=(*this).toString();

	ostream1.write(var_str.data(), static_cast<std::streamsize>(var_str.size()));
	return *this;
}

// output -> cout which is buffered standard output
///////////////////////////////////////////////////

// output() buffered threadsafe output to standard output
CVR var::output() const {
	LOCKIOSTREAM_OR_NOT
	return this->put(std::cout);
}

// outputl() flushed threadsafe output to standard output
// adds \n and flushes so is slower than output("\n")
CVR var::outputl() const {
	LOCKIOSTREAM_OR_NOT
	this->put(std::cout);
	std::cout << std::endl;
	return *this;
}

// outputt() buffered threadsafe output to standard output
// adds \t
CVR var::outputt() const {
	LOCKIOSTREAM_OR_NOT
	this->put(std::cout);
	std::cout << '\t';
	return *this;
}

// overloaded output() outputs a prefix str
CVR var::output(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	str.put(std::cout);
	return this->put(std::cout);
}

// oveloaded outputl() outputs a prefix str
CVR var::outputl(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	str.put(std::cout);
	this->put(std::cout);
	std::cout << std::endl;
	return *this;
}

// overloaded outputt() outputs a prefix str
CVR var::outputt(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	std::cout << "\t";
	str.put(std::cout);
	std::cout << "\t";
	this->put(std::cout);
	return *this;
}

// errput -> cerr which is unbuffered standard error
////////////////////////////////////////////////////

// errput() unbuffered threadsafe output to standard error
CVR var::errput() const {
	LOCKIOSTREAM_OR_NOT
	//return put(std::cerr);
	std::cerr << *this;
	return *this;
}

// errputl() unbuffered threadsafe output to standard error
// adds "\n"
CVR var::errputl() const {
	LOCKIOSTREAM_OR_NOT
	//this->put(std::cerr);
	std::cerr << *this;
	std::cerr << std::endl;
	return *this;
}

// overloaded errput outputs a prefix str
CVR var::errput(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	//str.put(std::cerr);
	//return this->put(std::cerr);
	std::cerr << str;
	std::cerr << *this;
	return *this;
}

// overloaded errputl outputs a prefix str
CVR var::errputl(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	//str.put(std::cerr);
	//this->put(std::cerr);
	std::cerr << str;
	std::cerr << *this;
	std::cerr << std::endl;
	return *this;
}

// logput -> clog which is a buffered version of cerr standard error output
///////////////////////////////////////////////////////////////////////////

// logput() buffered threadsafe output to standard log
CVR var::logput() const {
	LOCKIOSTREAM_OR_NOT
	//this->put(std::clog);
	std::clog << *this;
	//std::clog.flush();
	return *this;
}

// logputl() flushed threadsafe output to standard log
CVR var::logputl() const {
	LOCKIOSTREAM_OR_NOT
	//this->put(std::clog);
	std::clog << *this;
	std::clog << std::endl;
	return *this;
}

// overloaded logput with a prefix str
CVR var::logput(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	//str.put(std::clog);
	std::clog << str;
	std::clog << *this;
	return *this;
}

// overloaded logputl with a prefix str
CVR var::logputl(CVR str) const {
	LOCKIOSTREAM_OR_NOT
	//str.put(std::clog);
	//this->put(std::clog);
	std::clog << str;
	std::clog << *this;
	std::clog << std::endl;
	return *this;
}

} // namespace exo
