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

#ifndef _MSC_VER
#define _POSIX_SOURCE
#endif

//#define TRACING

#ifdef _POSIX_SOURCE
	#include <config.h>

	#include <cstdlib>
	#include <iostream>
	#include <string>
	#include <stdio.h>
#ifdef HAVE_BACKTRACE
	#include <execinfo.h>
#endif

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
	//ALN:NOTE: .osread could itself throw exception and we will have loop :(
	// Changed to read directly with default locale. If no locale specified (as it was in original version of the code)
			if (filetext.osread(filename))
//			if (filetext.osread(filename, L""))		// avoid to read C++ source as UTF8
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

void addbacktraceline(const var& sourcefilename, const var& lineno, var& returnlines)
{

#ifdef TRACING
	sourcefilename.outputl("SOURCEFILENAME=");
	lineno.outputl("LINENO=");
#endif

	if (not lineno || not lineno.isnum())
		return;

	var linetext=sourcefilename ^ ":" ^ lineno;

	//get the source file text
	var filetext;
	if (filetext.osread(sourcefilename)) {
	}

	//change DOS/WIN and MAC line ends to lf only
	filetext.swapper("\x0D\x0A","\x0A").converter("\x0D","\x0A");

	//extract the source line
	var line=filetext.field("\x0A",lineno).trimf(" \t");

	//suppress confusing and unhelpful exodus macros
	if (
		(line == "programexit()" || line == "libraryexit()" || line == "classexit()")
	or
		(line == "}" && sourcefilename.substr(-2,2) == ".h")
	)
		return;


	//outputl(linetext);
	linetext^=": " ^ line;

	returnlines^=FM^linetext;

	return;
}


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

#define BACKTRACE_MAXADDRESSES 500
	void *addresses[BACKTRACE_MAXADDRESSES];


/* example of TRACE from osx 64
Stack frames: 8
Backtrace 0: 0x10000c313
0   libexodus-11.5.0.dylib              0x000000010000c313 _ZN6exodus9backtraceEv + 99
Backtrace 1: 0x10001ec51
1   libexodus-11.5.0.dylib              0x000000010001ec51 _ZN6exodus11MVExceptionC2ERKNS_3varE + 129
Backtrace 2: 0x10001f314
2   libexodus-11.5.0.dylib              0x000000010001f314 _ZN6exodus12MVUnassignedC2ERKNS_3varE + 52
Backtrace 3: 0x10000e193
3   libexodus-11.5.0.dylib              0x000000010000e193 _ZNK6exodus3var3putERSo + 243
Backtrace 4: 0x10000e322
4   libexodus-11.5.0.dylib              0x000000010000e322 _ZNK6exodus3var7outputlEv + 34
Backtrace 5: 0x10000143f
5   steve                               0x000000010000143f _ZN13ExodusProgram4mainEv + 77
Backtrace 6: 0x1000010e6
6   steve                               0x00000001000010e6 main + 99
Backtrace 7: 0x100000f64
7   steve                               0x0000000100000f64 start + 52
*/

	//TODO autodetect if addr2line or dwalfdump/dSYM is available



#ifdef __APPLE__
	int size = ::backtrace(addresses, BACKTRACE_MAXADDRESSES);
	char **strings = backtrace_symbols(addresses, size);
//printf("Stack frames: %d\n", size);

	var returnlines="";

	for(int i = 0; i < size; i++) {

#ifdef TRACING
		//each string is like:
		//6   steve                               0x00000001000010e6 main + 99
		//////////////////////////////////////////////////////////////////////
		printf("%s\n", strings[i]);
#endif
		//parse one string for object filename and offset
		var onestring=var(strings[i]).trim();
		var objectfilename=onestring.field(L" ",2);
		var objectoffset=onestring.field(L" ",3);

		//looking for a dwarfdump line like this:
		//Line table file: 'steve.cpp' line 14, column 0 with start address 0x000000010000109e
		//////////////////////////////////////////////////////////////////////////////////////

		//get a dwarfdump line containing source filename and line number
		var debugfilename=objectfilename^".dSYM";
		var cmd=L"dwarfdump "^debugfilename^L" --lookup "^objectoffset^L" |grep \"Line table file: \" 2> /dev/null";
		var result=cmd.osshellread();
#ifdef TRACING
	cmd.outputl("CMD=");
	result.outputl("RESULT=");
#endif
		if (not result)
			continue;

		//parse the dwarfdump line for source filename and line number
		var sourcefilename=result.field("'",2);
		var lineno=result.field("'",3).trim().field(" ",2).field(",",1);

		addbacktraceline(sourcefilename,lineno,returnlines);

	}

	free(strings);
	return returnlines.substr(2);



//not __APPLE_
#else

	int size = ::backtrace(addresses, BACKTRACE_MAXADDRESSES);
#ifdef TRACING
	char **strings = backtrace_symbols(addresses, size);
	printf("Stack frames: %d\n", size);
#endif

	for(int i = 0; i < size; i++)
	{
		//#pragma warning (disable: 4311)
		internaladdresses^=" " ^ var((long long) addresses[i]).oconv("MX");

#ifdef TRACING
		if (sizeof addresses[i] == 4)
			printf("Backtrace %d: %X\n", i, (unsigned int)(long long)addresses[i]);
		else
			printf("Backtrace %d: %#llx\n", i, (long long)addresses[i]);
		printf("%s\n", strings[i]);
#endif

	}

#ifdef TRACING
	free(strings);
#endif

	FILE *fp;
	//int status;
	char path[1024];

	var binaryfilename=EXECPATH2.field(" ",1);
	//if (binaryfilename == binaryfilename.convert("/\\:",""))
	if (not binaryfilename.osdir())
	{
		var temp="which " ^ binaryfilename.field2(SLASH,-1);
		temp=temp.osshellread().field("\n",1).field("\r",1);
		if (temp)
			binaryfilename=temp;
	}
#ifdef __APPLE__
	var oscmd="atos -o " ^ binaryfilename.quote() ^ " " ^ internaladdresses;
#else
	var oscmd="addr2line -e " ^ binaryfilename.quote() ^ " " ^ internaladdresses;
#endif
	//oscmd.outputl();

#ifdef TRACING
	printf("EXECPATH = %s\n",EXECPATH2.toString().c_str());
	printf("executing %s\n",oscmd.toString().c_str());
#endif

	/* Open the command for reading. */
	fp = popen(oscmd.toString().c_str(), "r");
	if (fp == NULL)
	{
		printf("Failed to run command\n" );
		return L"";
	}

#ifdef TRACING
	printf("reading output of addr2line\n");
#endif

	var returnlines="";

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL)
	{

#ifdef TRACING
		printf("fgets:%s", path);
#endif

		var path2=var(path).convert("\x0d\x0a","");//.outputl("path=");
		var sourcefilename=path2.field(":",1);//.outputl("filename=");
		var lineno=path2.field(":",2);//.outputl("lineno=");

		addbacktraceline(sourcefilename,lineno,returnlines);
	}

	/* close */
	pclose(fp);
	
	return returnlines.substr(2);
#endif

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
		var cmd1=cmd[1].ucase();

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
