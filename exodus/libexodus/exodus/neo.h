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

// DOESNT SEEM TO BE USED ANYWHERE EXCEPT service/markets/market.cpp Is it obsolete?
#ifndef EXO_H
#define EXO_H 1

#ifndef EXODUS_POSIX
#define WIN32_LEAN_AND_MEAN
#include <exodus/var.h>
#include <exodus/mvenvironment.h>
#include <windows.h>

// http://msdn2.microsoft.com/en-us/library/28d6s79h(VS.80).aspx
/*
Module-definition (.def) files provide the linker with information about exports, attributes, and
other information about the program to be linked. A .def file is most useful when building a DLL.
Because there are linker options that can be used instead of module-definition statements, .def
files are generally not necessary. You can also use __declspec(dllexport) as a way to specify
exported functions.
*/

// http://en.wikipedia.org/wiki/Dynamic_link_library

/* http://en.wikipedia.org/wiki/Dynamic_link_library
DLL files may be explicitly loaded at run-time, a process referred to simply as run-time dynamic
linking by Microsoft, by using the LoadLibrary (or LoadLibraryEx) API function. The GetProcAddress
API function is used to lookup exported symbols by name, and FreeLibrary � to unload the DLL. These
functions are analogous to dlopen, dlsym, and dlclose in the POSIX standard API.
*/

// Allocating and freeing memory across module boundaries
// http://blogs.msdn.com/oldnewthing/archive/2006/09/15/755966.aspx

// http://www.icynorth.com/development/createdlltutorial.html
// Export this function - cannot use C linkage to pass UDT User Defined Types (anything other than
// POD Plain Old Data)
// use .def file?

/*
	//dynamic execution via function table (not implemented yet)
	//will throw if not found
	var().call("myfunction")
	


	//dynamic execution
	//will throw if not found
	var().call("mydll","myfunction")
	


	//dynamic linking with caching
	//will throw if not found
	var myfunction;
	myfunction.call("mydll","myfunction")
	myfunction.call()
	


	//error checking loading
	var myfunction;
	if (!myfunction.load("mydll"))
	{
	throw MVError("cannot load mydll");
	}
	


	//error checking linking not implemented yet
	var myfunction;
	if (!myfunction.link("myfunction"))
	{
	throw MVError("cannot link  myfunction");
	}
	


	//error checking loading and linking in one step
	var myfunction;
	if (!myfunction.link("mydll","myfunction"))
	{
	throw MVError("cannot link mydll, myfunction");
	}
	


*/

// this is not needed since it is automatically generated if missing
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
	return TRUE;
};

extern "C" {

#define function(funcname) \
	exodus::var __declspec(dllexport) funcname(exodus::MvEnvironment& env) {

#define functionexit }

#define libraryexit }

#endif EXODUS_POSIX
#endif	// EXO_H
