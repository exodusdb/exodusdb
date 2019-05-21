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


//dont define to allow narrow since these are the narrow definitions
#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus {

//ctor for char
//use initializers since cannot fail (but could find how to init the char1)
var::var(const char char1):
	var_typ(pimpl::VARTYP_STR)
	//,var_str(const_cast<char_t>char1)
{
	//cannot fail
	//THISIS("var::var(const char char1)")

	//not a pointer anymore for speed
	//priv=new pimpl;

	//var_typ=pimpl::VARTYP_STR;
	var_str=char1;
}

//ctor for memory block
//dont use initialisers and TODO protect against out of memory in expansion to string
var::var(const char* cstr1, const size_t int1)
{
	//cannot fail
	//THISIS("var::var(const char* cstr1, const size_t int1)")

	//TODO optimise and UTF conversion
	//var_typ=pimpl::VARTYP_STR;
	std::string cstr2(cstr1,int1);
	var_str=std::string(cstr2.begin(),cstr2.end());
	var_typ=pimpl::VARTYP_STR;
}

/*duplicates
//comparisons with narrow characters

bool operator== (const var&    var1    ,const char*  char2   ){return MVeq(var1,var(char2));}
bool operator== (const char*  char1  ,const var&    var2     ){return MVeq(var2,var(char1));}

bool operator !=(const var&    var1    ,const char*  char2   ){return !MVeq(var1,var(char2));}
bool operator !=(const char*  char1  ,const var&    var2     ){return !MVeq(var2,var(char1));}

bool operator < (const var&    var1    ,const char*  char2   ){return MVlt(var1,var(char2));}
bool operator < (const char*  char1  ,const var&    var2     ){return MVlt(var(char1),var2);}

bool operator >=(const var&    var1    ,const char*  char2   ){return !MVlt(var1,var(char2));}
bool operator >=(const char*  char1  ,const var&    var2     ){return !MVlt(var(char1),var2);}

bool operator > (const var&    var1    ,const char*  char2   ){return MVlt(var(char2),var1);}
bool operator > (const char*  char1  ,const var&    var2     ){return MVlt(var2,var(char1));}

bool operator <=(const var&    var1    ,const char*  char2   ){return !MVlt(var(char2),var1);}
bool operator <=(const char*  char1  ,const var&    var2     ){return !MVlt(var2,var(char1));}
*/
}// of namespace exodus
