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

#include <boost/regex.hpp>
#include "rex.h"

// based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

// one based two dimensional array but (0,0) is a separate element set or got if either or both
// index is zero

namespace exo {

// User defined literal "_rex"

rex::rex(SV expression) :expression_(expression) {
}

rex::rex(SV expression, SV options) :expression_(expression), options_(options) {
}

var var::replace(rex const& regex, SV replacement) const& {
	return this->regex_replace(regex.expression_, replacement, regex.options_);
}

// "[a-z]"_rex
ND rex operator""_rex(const char* cstr, std::size_t size) {
	return rex(std::string_view(cstr, size));
}


}  // namespace exo
