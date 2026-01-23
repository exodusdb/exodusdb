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

// indicates that global const var FM etc. to be defined (omit extern keyword)
#define EXO_VAR_CPP

#include "var.h"

#include "varimpl.h"

namespace exo {

using let = const var;

// TODO replace .f with a version of .f that returns a string_view
// instead of wasting time constructing a temporary var only to extract a single char from it
ND RETVAR var_proxy1::at(const int pos1) const {
	//TODO! allow at() to work directly on var_str without extracting the required field first
	let v1 = var_.f(fn_);
	return v1.at(pos1);
}

ND var var_proxy2::at(const int pos1) const {
	let v1 = var_.f(fn_, vn_);
	return v1.at(pos1);
}

ND var var_proxy3::at(const int pos1) const {
	let v1 = var_.f(fn_, vn_, sn_);
	return v1.at(pos1);
}

static thread_local var thread_lasterror = "";

void  var::setlasterror(in msg) {
	// No checking for speed
	// THISIS("void var::lasterror(in msg")
	// msg.assertString(function_sig, "msg");

	// tcache_get (tc_idx=12) at malloc.c:2943
	// 2943    malloc.c: No such file or directory.
	// You have heap corruption somewhere -- someone is running off the end of an array or
	// dereferencing an invalid pointer or using some object after it has been freed. EVADE
	// error for now by commenting next line

	thread_lasterror = msg;

//	if (DBTRACE>1)
//		TRACE(thread_lasterror)

//	return lasterror();
}

var  var::lasterror() {
	return thread_lasterror;
}

void var::loglasterror(in source) {
	thread_lasterror.logputl(source ^ " ");
}

}  // namespace exo
