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

#ifndef EXODUS_LIBEXODUS_EXODUS_EXODUS_H_
#define EXODUS_LIBEXODUS_EXODUS_EXODUS_H_ 1

/////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE exodus.h MUST be included LAST to avoid its macros causing problems in standard libraries
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXO_MODULE
	import var;
#else
#	include <exodus/var.h>
#	include <exodus/dim.h>
#	include <exodus/rex.h>
#endif
#include <exodus/vardefs.h>

#include <exodus/exoimpl.h>
#include <exodus/exocallable.h>
#include <exodus/exoprog.h>

#include <exodus/exofuncs.h>
#include <exodus/exomacros.h>
#include <exodus/range.h>

// It usually poor practice to "pollute" the global space of the header user
// but in this case the header user is an exodus application programmer
// may not be using any other includes
// to avoid pollution, simply directly include the above headers as desired instead.
//
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
using namespace exo;
#pragma clang diagnostic pop

#endif	// EXODUS_LIBEXODUS_EXODUS_EXODUS_H_
