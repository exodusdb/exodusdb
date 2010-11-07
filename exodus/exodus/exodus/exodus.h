/*
Copyright (c) 2009 Stephen John Bush

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

#ifndef EXODUS_H
#define EXODUS_H 1

////////////////////////////////////////////////////////////////////////////////////////////////
//NOTE exodus.h MUST be included LAST to avoid its macros causing problems in standard libraries
////////////////////////////////////////////////////////////////////////////////////////////////

//must be before exodus macros and functions otherwise lots of errors
//from standard libararies which are included here
#include <exodus/mvfunctor.h>
#include <exodus/mvenvironment.h>
#include <exodus/mvprogram.h>

//always last to avoid conflicting definitions
#include <exodus/exodusmacros.h>
#include <exodus/exodusfuncs.h>

//it usually poor practice to "pollute" the global space of the header user
//but in this case the header user is an exodus application programmer
//may not be using any other includes
//to avoid pollution, simply directly include the above headers as desired instead.
using namespace exodus;

#endif //EXODUS_H
