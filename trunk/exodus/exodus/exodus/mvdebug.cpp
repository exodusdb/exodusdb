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

#ifndef _MSC_VER
#define _POSIX_SOURCE
#endif

//#define TRACING 1

#ifdef _POSIX_SOURCE
	#include <config.h>

	#include <cstdlib>
	#include <iostream>
	#include <string>
	#include <stdio.h>
	//for backtrace
	#include <execinfo.h>

#endif
	//for signal
	#include <signal.h>

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mvimpl.h>
#include <exodus/mv.h>

//Simple implementation of an additional output to the console:
#ifdef _MSC_VER
#include <exodus/StackWalker.h>
class MyStackWalker : public StackWalker
{
public:

	exodus::var returnlines;

	MyStackWalker() : StackWalker(), returnlines("")
	{}

	MyStackWalker(DWORD dwProcessId, HANDLE hProcess)
	: StackWalker(dwProcessId, hProcess), returnlines("")
	{}
	virtual void OnOutput(LPCSTR szText)
	{
		//printf(szText);

		//printf("fgets:%s", path);
		exodus::var line=exodus::var(szText).convert("\x0d\x0a","");//.outputl("path=");
		exodus::var filename=line.field(":",1,2);//.outputl("filename=");
		exodus::var lineno=filename.field2(" ",-1);//.outputl("lineno=");
		filename.splicer(-(lineno.length()+1),999999,"");
		lineno.substrer(2,lineno.length()-2);
		if (filename.index("stackwalker.cpp")||filename.index("debug.cpp")||filename.index("crtexe.c"))
			return;
		if (lineno.isnum()&&lineno)
		{
			exodus::var linetext=filename.field2(exodus::SLASH,-1) ^ ":" ^ lineno;
			exodus::var filetext;
			if (filetext.osread(filename))
			{
				linetext^=": " ^ filetext.field("\x0A",lineno).trimf(" \t");
				if (linetext.index("backtrace("))
					return;
			}
			//outputl(linetext);
			returnlines^=exodus::FM^linetext;
		}

//		StackWalker::OnOutput(szText);
	}
};
#endif

#ifndef HAVE_BACKTRACE
#define NOBACKTRACE
#endif

namespace exodus {

//http://www.delorie.com/gnu/docs/glibc/libc_665.html
var backtrace()
{

#ifdef _MSC_VER
	//logputl("backtrace() not implemented on windows yet");
	//var().abort("");
	MyStackWalker sw;
	sw.ShowCallstack();
	return sw.returnlines;
#elif !defined(HAVE_BACKTRACE)
	printf("backtrace() not available");
	return L"";
#else

	var internaladdresses="";

	void *addresses[100];

	int size = ::backtrace(addresses, 100);
#ifdef TRACING
	char **strings = backtrace_symbols(addresses, size);
	printf("Stack frames: %d\n", size);
#endif
	for(int i = 0; i < size; i++)
	{
		//#pragma warning (disable: 4311)
		internaladdresses^=" " ^ var((long long) addresses[i]).oconv("MX");
#ifdef TRACING
		printf("%d: %X\n", i, (int)addresses[i]);
		printf("%s\n", strings[i]);
#endif
	}
#ifdef TRACING
	free(strings);
#endif
	FILE *fp;
	//int status;
	char path[1024];

	var binaryfilename=EXECPATH.field(" ",1);
	if (binaryfilename == binaryfilename.convert("/\\:",""))
		binaryfilename="`which " ^ binaryfilename ^ "`";
	var oscmd="addr2line -e " ^ binaryfilename.quote() ^ " " ^ internaladdresses;

	//oscmd.outputl();

#ifdef TRACING
	printf("EXECPATH = %s\n",EXECPATH.tostring().c_str());
	printf("executing %s\n",oscmd.tostring().c_str());
#endif
	/* Open the command for reading. */
	fp = popen(oscmd.tostring().c_str(), "r");
	if (fp == NULL)
	{
		printf("Failed to run command\n" );
		return L"";
	}

	var returnlines="";

#ifdef TRACING
	printf("reading output of addr2line\n");
#endif
	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL)
	{
#ifdef TRACING
		printf("fgets:%s", path);
#endif
		var path2=var(path).convert("\x0d\x0a","");//.outputl("path=");
		var filename=path2.field(":",1);//.outputl("filename=");
		var lineno=path2.field(":",2);//.outputl("lineno=");
		if (lineno)		
		{
			var linetext=filename ^ ":" ^ lineno;
			var filetext;
			if (filetext.osread(filename))
			{
				linetext^=": " ^ filetext.field("\x0A",lineno).trimf(" \t");
			}
			//outputl(linetext);
			returnlines^=FM^linetext;
		}
	}

	/* close */
	pclose(fp);
	
	return returnlines.substr(2);
#endif
	
}

void SIGINT_handler (int sig)
{
	//ignore more of this signal
	//var().breakoff()
	//faster/safer
	signal(sig, SIG_IGN);

	//duplicated in init and B
	backtrace().convert(FM,"\n").outputl();

	for (;;)
	{

		printf ("\nInterrupted. (C)ontinue (E)nd (B)acktrace\n");

		output("? ");
		var cmd;
		if (! cmd.input())
		continue;
		var cmd1=cmd.substr(1,1).ucase();

		if (cmd1 == "C")
			break;

		else if (cmd1=="E")

			var().abort("Aborted. User interrupt");

		else if (cmd1=="B")

			//duplicated in init and B
			backtrace().convert(FM,"\n").outputl();

	}

	//stop ignoreing this signal
	//var().breakon()
	//faster/safer
	signal(SIGINT, SIGINT_handler);
}

void var::breakoff() const
{
	//ignore more of this signal
	signal(SIGINT, SIG_IGN);
}

void var::breakon() const
{
	signal(SIGINT,  SIGINT_handler); /* this line will redirect ctrl+c signal*/
}

}//namespace exodus
