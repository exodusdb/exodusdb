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

//boost wpath is only in boost 1.34 but we hardcoded path at the moment which
//prevents unicode in osfilenames etc

#ifndef MVOS_H
#define MVOS_H

//EXCELLENT
//http://www.regular-expressions.info/
#include <boost/regex.hpp>

//TODO check all error handling
//http://www.ibm.com/developerworks/aix/library/au-boostfs/index.html
//catch(boost::filesystem::filesystem_error e) { 
//#include <boost/filesystem.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/config.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace boostfs = boost::filesystem;
#include <boost/thread/tss.hpp>

//#include NullCodecvt.h

//#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//for sleep
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

//for rnd and initrnd
#include <boost/random.hpp>

//for exodus_once
#include <boost/thread/once.hpp>

//#include <istream>
//#include <ostream>
#include <fstream>
#include <cstdlib> //for getenv and setenv/putenv
//#include <stdio.h> //for getenv

//oshell capturing output needs posix popen
#if defined(_MSC_VER) && !defined(popen)
#define popen _popen
#define pclose _pclose
#endif

#define MV_NO_NARROW

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

//#include "exodus/NullCodecvt.h"//used to prevent wifstream and wofstream widening/narrowing binary input/output to/from internal wchar_t

//traditional Unix file I/O interface declared in <fcntl.h>
// (under Unix and Linux) or <out.h> (Windows)

//FindFile family of Win32 SDK functions such as
// FindFirstFileEx, FindNextFile and CloseFind

//see http://www.boost.org/libs/filesystem/test/wide_test.cpp
//for boost filesystem wide operations and UTF8

boost::once_flag exodus_once_flag=BOOST_ONCE_INIT;

void exodus_once_func()
{
	//so wcscoll() sorts a<B etc instead of character number wise where B<a
	setlocale (LC_COLLATE, "en_US.utf-8");
}

class ExodusOnce
{
public:
	ExodusOnce()
	{
		boost::call_once(&exodus_once_func, exodus_once_flag);
	}
};
static ExodusOnce exodus_once_static;

namespace exodus{

//BOOST RANDOM
//http://www.boost.org/doc/libs/1_38_0/libs/random/random_demo.cpp

//set the generator type to ...
//typedef boost::minstd_rand random_base_generator_type;
typedef boost::mt19937 random_base_generator_type;
boost::thread_specific_ptr<random_base_generator_type> tss_random_base_generators;

random_base_generator_type* get_random_base_generator()
{
	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=tss_random_base_generators.get();
	if (!threads_random_base_generator)
	{
		tss_random_base_generators.reset(new random_base_generator_type);
		threads_random_base_generator=tss_random_base_generators.get();
		if (!threads_random_base_generator)
			throw MVException(L"Could not create random number generator");

		//seed to the os clock (secs since unix epoch)
		//Caveat: std::time(0) is not a very good truly-random seed.
		//logputl(L"Seeding random number generator to system clock");
		(*threads_random_base_generator).seed(static_cast<unsigned int>(std::time(0)));
		//(*thread_base_generator).seed(static_cast<unsigned int>(var().ostime().toInt()));

	}
	return threads_random_base_generator;
}

var var::rnd() const
{
	THISIS(L"var var::rnd() const")
	THISISNUMERIC()

	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=get_random_base_generator();

	//pick rnd(5) returns pseudo random integers in the range of 0-4
	int max=(*this).toInt()-1;
	if (max<0)
		return var(*this);

	//define a integer range (0 to n-1)
	boost::uniform_int<> uni_dist(0,max);

	//use the base generator and the range to create a specific generator
	boost::variate_generator<random_base_generator_type&, boost::uniform_int<> > uni(*threads_random_base_generator, uni_dist);

	//generate a pseudo random number
	return uni();
}

void var::initrnd() const
{
	THISIS(L"void var::initrnd(const var& seed) const")
	THISISNUMERIC()

	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=get_random_base_generator();

	//set the new seed
	//logputl(L"Seeding random number generator to " ^ (*this));
	(*threads_random_base_generator).seed(static_cast<unsigned int>( (*this).toInt() ));
}

//only here really because boost regex is included here for file matching
bool var::match(const var& matchstr, const var& options) const
{
	THISIS(L"bool var::match(const var& matchstr, const var& options) const")
	THISISSTRING()
	ISSTRING(matchstr)

	//TODO fully implement
	if (options.index(L"w"))
	{
		if (matchstr==L""||var_mvstr==matchstr||matchstr==L"*.*"){}
		else if (matchstr.substr(1,1)==L"*"&&substr(-matchstr.length()+1)==matchstr.substr(2)){}
		else if (matchstr.substr(-1,1)==L"*"&&substr(1,matchstr.length()-1)==matchstr.substr(1,matchstr.length()-1)){}
		else return false;
		return true;
	}

	//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	//TODO automatic caching of regular expressions or new exodus datatype to handle them
	//TODO automatic caching of regular expressions or new exodus datatype to handle them
	boost::wregex regex;
	try
	{
		if (options.index(L"i"))
                        regex.assign(toTstring(matchstr), boost::regex::extended|boost::regex_constants::icase);
		else
                        regex.assign(toTstring(matchstr), boost::regex::extended|boost::regex_constants::icase);
	//boost::wregex toregex_regex(with.towstring(), boost::regex::extended);
	}
	catch (boost::regex_error& e)
    {
		throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
	}

        return regex_match(toTstring((*this)), regex);

}

var var::swap(const var& what, const var& with, const var& options) const
{
	THISIS(L"var var::swap(const var& what, const var& with) const")
	THISISSTRING()

	return var(*this).swapper(what,with,options);
}

//only here really because boost regex is included here for file matching
var& var::swapper(const var& what, const var& with, const var& options)
{
	THISIS(L"var& var::swapper_regex(const var& regex, const var& with, const var& options)")
	THISISSTRING()
	ISSTRING(what)
	ISSTRING(with)
	ISSTRING(options)

	if (options.index(L"r"))
	{
		//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

		//TODO automatic caching of regular expressions or new exodus datatype to handle them
		boost::wregex regex;
		try
		{
			if (options.index(L"i"))
                                regex.assign(toTstring(what), boost::regex::extended|boost::regex_constants::icase);
			else
                                regex.assign(toTstring(what), boost::regex::extended);
		//boost::wregex toregex_regex(with.towstring(), boost::regex::extended);
		}
		catch (boost::regex_error& e)
		{
			throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
		}

		//return regex_match(var_mvstr, expression);

		//std::wostringstream outputstring(std::ios::out | std::ios::binary);
    //std::ostream_iterator<wchar_t, wchar_t> oiter(outputstring);
		//boost::regex_replace(oiter, var_mvstr.begin(), var_mvstr.end(),regex_regex, with, boost::match_default | boost::format_all);
                var_mvstr=var(boost::regex_replace(toTstring((*this)),regex, toTstring(with))).var_mvstr;

	}
	else
	{
		std::wstring str1=what.var_mvstr;
		std::wstring str2=with.var_mvstr;

		//nothing to do if oldstr is ""
		if (str1==L"")
			return *this;

		//find the starting position of the field or return
		std::wstring::size_type start_pos=0;
		while (true)
		{
			start_pos=var_mvstr.find(str1,start_pos);
			//past of of string?
			if (start_pos==std::wstring::npos) return *this;
			var_mvstr.replace(start_pos,str1.length(),str2);
			start_pos+=str2.length();
		}
	}

	return *this;
}


//in MVutils.cpp
void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);

bool var::osgetenv(const var& envvarname)
{
	THISIS(L"bool var::osgetenv(const var& envvarname)")
	THISISDEFINED()
	ISSTRING(envvarname)

	#pragma warning ( disable : 4996)
	const char* cvalue=std::getenv(envvarname.tostring().c_str());
	if (cvalue==0)
	{
		var_mvstr=L"";
		var_mvtype=pimpl::MVTYPE_STR;
		return false;
	}
	else
		*this=var(cvalue);
	var_mvtype=pimpl::MVTYPE_STR;
	return true;
}

bool var::ossetenv(const var& envvarname) const
{
	THISIS(L"bool var::ossetenv(const var& envvarname) const")
	THISISDEFINED()
	ISSTRING(envvarname)

//#ifdef _MSC_VER
#ifndef setenv
        /* on windows this should be used
	BOOL WINAPI SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);
	*/

	//is this safe on windows??
	//https://www.securecoding.cert.org/confluence/display/seccode/POS34-C.+Do+not+call+putenv()+with+a+pointer+to+an+automatic+variable+as+the+argument
	std::string tempstr=envvarname.tostring();
	tempstr+="=";
	tempstr+=tostring();
	//#pragma warning ( disable : 4996)
	const int result=putenv((char*)(tempstr.c_str()));
	if (result==0)
		return true;
	else
		return false;

#else
	return setenv((char*)(envvarname.tostring().c_str()),(char*)(tostring().c_str()),1);
#endif

}

var var::osshell() const
{
	THISIS(L"var var::osshell() const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	breakoff();
	int shellresult=system(tostring().c_str());
	breakon();

	return shellresult;
}

var var::osshellread() const
{
	THISIS(L"var var::osshellread() const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	var output=L"";

	//"r" means read
	std::FILE *cmd=popen(tostring().c_str(), "r");
	//return a code to indicate program failure. but can this ever happen?
	if (cmd==NULL)
		return 1;
	//TODO buffer overflow check
    char cstr1[1024]={0x0};
	while (std::fgets(cstr1, sizeof(cstr1), cmd) !=NULL) {
		//std::printf("%s\n", result);
		//cannot trust that standard input is convertable from utf8
		//output.var_mvstr+=wstringfromUTF8((const UTF8*)result,strlen(result));
		std::string str1=cstr1;
		output.var_mvstr+=std::wstring(str1.begin(),str1.end());
	}

	//we are going to throw away the process termination status
	//because we are going to return the output text
	int result=pclose(cmd);

    return output;

}

var var::osshellwrite(const var& writestr) const
{
	THISIS(L"var var::osshellwrite(const var& writestr) const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()
	ISSTRING(writestr)

	//"w" means read
	std::FILE *cmd=popen(tostring().c_str(), "w");
	//return a code to indicate program failure. but can this ever happen?
	if (cmd==NULL)
		return 1;
	fputs(writestr.tostring().c_str(),cmd);

	//return the process termination status (pity cant do this for read too)
	return pclose(cmd);

}

var var::suspend() const
{
	THISIS(L"var var::suspend() const")

	breakoff();
	//use dummy to avoid warning in gcc4 "warning: ignoring return value of int system(const char*), declared with attribute warn_unused_result"
	int dummy=system(tostring().c_str());
	breakon();

	return L"";
}

void var::osflush() const
{
	THISIS(L"void var::osflush() const")

	//wcout<<L"var::flush not implemented yet"<<endl;
	return;
}

bool var::osopen(const var& osfilename)
{
	THISIS(L"bool var::osopen(const var& osfilename)")
	THISISDEFINED()
	ISSTRING(osfilename)

	//TODO buffer filehandles to prevent continual reopening

	if (!(osfilename.var_mvtype&pimpl::MVTYPE_STR))
	{
		if (!osfilename.var_mvtype)
			throw MVUnassigned(L"osopen(osfilename)");
		osfilename.createString();
	}

	std::fstream myfile;
	//	myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));
//var(int(sizeof(myfile))).outputl(L"filesize");

	myfile.open(osfilename.tostring().c_str(), std::ios::out |std::ios::in | std::ios::binary);
	if (!myfile) return false;

	var_mvstr=osfilename.var_mvstr;
	var_mvtype=pimpl::MVTYPE_STR;

	//TODO for now reopen file on every access
	myfile.close();

	return true;
}

//on unix or with iconv we could convert to or any character format
//for sequential output we could use popen to progressively read and write with no seek facility
//popen("iconv -f utf-16 -t utf-8 >targetfile","w",filehandle) followed by progressive writes
//popen("iconv -f utf-32 -t utf-16 <sourcefile","r",filehandle) followed by progressive reads
//use utf-16 or utf-32 depending on windows or unix

bool var::osread(const var& osfilename)
{
	THISIS(L"bool var::osread(const var& osfilename)")
	//will be checked by nested osread
	//THISISDEFINED()
	ISSTRING(osfilename)
	return osread(osfilename.tostring().c_str());
}

bool var::osread(const char* osfilename)
{
	THISIS(L"bool var::osread(const var& osfilename)")
	THISISDEFINED()
	//ISSTRING(osfilename)

	var_mvstr=L"";
	var_mvtype=pimpl::MVTYPE_STR;

	//std::wifstream myfile;
	std::ifstream myfile;
	//myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	//ios:ate to go to the end to find the size in the next statement with tellg
	//myfile.open(osfilename.tostring().c_str(), std::ios::binary | std::ios::ate );
	myfile.open(osfilename, std::ios::binary | std::ios::ate );
	if (!myfile)
		return false;

	//NB tellg and seekp goes by bytes regardless of normal/wide stream

	unsigned int bytesize;
    bytesize = myfile.tellg();

	if (bytesize>0)
	{

		//new
		char* memblock;
		memblock = new char [bytesize];
   		if (memblock==0)
   		{
   			myfile.close();
   			return false;
   		}

    	myfile.seekg (0, std::ios::beg);
    	myfile.read (memblock, (unsigned int) bytesize);
    	myfile.close();

		//for now dont accept UTF8 in order to avoid failure if non-utf characters
		//var_mvstr=wstringfromUTF8((UTF8*) memblock, (unsigned int) bytesize);
		std::string tempstr=std::string(memblock, bytesize);
		var_mvstr=std::wstring(tempstr.begin(),tempstr.end());

		//delete
    	delete[] memblock;

	}

	return true;

}

bool var::oswrite(const var& osfilename) const
{
	THISIS(L"bool var::oswrite(const var& osfilename) const")
	THISISSTRING()
	ISSTRING(osfilename)

	std::ofstream myfile;
	//myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	myfile.open(osfilename.tostring().c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile)
	 return false;

	//myfile.write(var_mvstr.data(),int(var_mvstr.length()));
	std::string output=toUTF8(var_mvstr);
	myfile.write(output.data(),(unsigned int)output.length());
	if (myfile.fail())
	{
		myfile.close();
		return false;
	}

   	myfile.close();
	return true;

}

void var::osbwrite(const var& osfilehandle, const int startoffset) const
{
	THISIS(L"void var::osbwrite(const var& osfilehandle, const int startoffset) const")
	THISISSTRING()
	ISSTRING(osfilehandle)

	//TODO buffer filehandles to prevent continual reopening

	//TODO buffer the fileopen ... is ate needed here?
	//using wfstream instead of wofstream so that we can seek to the end of the file?
	std::wofstream myfile;
	//what is the purpose of the following?
        //to prevent locale conversion but for binary?
        //myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	myfile.open(osfilehandle.tostring().c_str(), std::ios::out | std::ios::in | std::ios::binary | std::ios::ate);
	if (!myfile)
		return;

	//NB seekp goes by bytes regardless of the fact that it is a wide stream
	myfile.seekp (startoffset*sizeof(wchar_t));
	//NB but length goes by number of wide characters not bytes
#ifndef __MINGW32__
		myfile.write(var_mvstr.data(),int(var_mvstr.length()));
#else
	//TODO avoid string copy if wofstream not defined to be ofstream
        std::string tempstr=tostring();
        myfile.write(tempstr.data(),int(tempstr.length()));
#endif

	if (myfile.fail())
	{
		myfile.close();
		return;
	}

   	myfile.close();
	return;

}

var& var::osbread(const var& osfilehandle, const int startoffset, const int size)
{
	THISIS(L"var& var::osbread(const var& osfilehandle, const int startoffset, const int size)")
	THISISDEFINED()
	ISSTRING(osfilehandle)

	//TODO buffer filehandles to prevent continual reopening

	var_mvstr=L"";
	var_mvtype=pimpl::MVTYPE_STR;

	if (size<=0) return *this;
	//use ate to position at end so tellg below can determine file size
	//std::wifstream myfile;
	std::ifstream myfile;
        //myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	myfile.open(osfilehandle.tostring().c_str(), std::ios::binary | std::ios::ate);
	if (!myfile)
		return *this;
//NB all file sizes are in bytes NOT characters despite this being a wide character fstream
	unsigned int maxsize = myfile.tellg();
	//int readsize=maxsize-startoffset*sizeof(wchar_t);
	//if (readsize>size*2) readsize=size*sizeof(wchar_t);
	unsigned int readsize=maxsize-startoffset;

	if (readsize<=0)
	{
		myfile.close();
		return *this;
	}

	//new
	//wchar_t* memblock;
	//memblock = new wchar_t [readsize/sizeof(wchar_t)];
	char* memblock;
	memblock = new char [readsize];
	if (memblock==0)
	{//TODO NEED TO THROW HERE
		myfile.close();
		return *this;
	}

	//myfile.seekg (startoffset/sizeof(wchar_t), std::ios::beg);
	myfile.seekg (startoffset, std::ios::beg);
	myfile.read (memblock, readsize);
	myfile.close();

	//var_mvstr=std::wstring(memblock,readsize/sizeof(wchar_t));

	//for now dont accept UTF8 in order to avoid failure if non-utf characters
	//var_mvstr=wstringfromUTF8((UTF8*) memblock, (unsigned int) bytesize);
	std::string tempstr=std::string(memblock, readsize);
	var_mvstr=std::wstring(tempstr.begin(),tempstr.end());

	//delete
	delete[] memblock;

//	var_mvtype=pimpl::MVTYPE_STR;

	return *this;

}

void var::osclose() const
{
	THISIS(L"void var::osclose() const")
	THISISSTRING()

	//TODO buffer filehandles to prevent continual reopening

	return;
}

bool var::osrename(const var& newosfilename) const
{
	THISIS(L"bool var::osrename(const var& newosfilename) const")
	THISISSTRING()
	ISSTRING(newosfilename)
	
	//prevent overwrite of existing file
	//ACQUIRE
	std::wifstream myfile;
	myfile.open(newosfilename.tostring().c_str(), std::ios::binary );
	if (myfile)
	{
		//RELEASE
		myfile.close();
		return false;
	}

	return !std::rename(tostring().c_str(),newosfilename.tostring().c_str());

}

bool var::oscopy(const var& to_osfilename) const
{
	THISIS(L"bool var::oscopy(const var& to_osfilename) const")
	THISISSTRING()
	ISSTRING(to_osfilename)
	
    //boostfs::wpath frompathx(toTstring((*this)).c_str());
    //boostfs::wpath topathx(toTstring(to_osfilename).c_str());
    boostfs::path frompathx((*this).tostring().c_str());
    boostfs::path topathx(to_osfilename.tostring().c_str());
    try
    {
		//will not overwrite so this is redundant
		//option to overwrite is not in initial versions of boost copy_file
		if (boostfs::exists(topathx)) return false;

		boostfs::copy_file(frompathx, topathx);
	}
	catch(...)
	{
		return false;
	}
	return true;

}

bool var::osdelete() const
{
	return osdelete(*this);
}

//not boost ... only removes files?
bool var::osdelete(const var& osfilename) const
{
	THISIS(L"bool var::osdelete(const var& osfilename) const")
	THISISDEFINED()
	ISSTRING(osfilename)
	
	return !std::remove(osfilename.tostring().c_str());
}

var var::oslistf(const var& path, const var& spec) const
{
	return oslist(path, spec, 1);
}

var var::oslistd(const var& path, const var& spec) const
{
	return oslist(path, spec, 2);
}

var var::osfile() const
{
	THISIS(L"var var::osfile() const")
	THISISSTRING()
	
	//get a handle and return L"" if doesnt exist or isnt a regular file
    try
    {
		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return L"";
        //is_regular is only in boost > 1.34
		//if (!boostfs::is_regular(pathx)) return L"";
        if (boostfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=boostfs::last_write_time(pathx);
        //convert to ptime
        boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
        //convert to mv date and time
        int mvdate, mvtime;
        ptime2mvdatetime(ptimex, mvdate, mvtime);

		return int(boostfs::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);

	}
	catch(...)
	{
		return L"";
	};
}

bool var::osmkdir() const
{
	THISIS(L"bool var::osmkdir() const")
	THISISSTRING()
	
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

		if (boostfs::exists(pathx)) return false;
		boostfs::create_directories(pathx);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool var::osrmdir(bool evenifnotempty) const
{
	THISIS(L"bool var::osrmdir(bool evenifnotempty) const")
	THISISSTRING()
	
	//get a handle and return L"" if doesnt exist or is NOT a directory
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return false;
        if (!boostfs::is_directory(pathx)) return false;
		
		if (evenifnotempty)
			boostfs::remove_all(pathx);
		else
			boostfs::remove(pathx);
	}
	catch(...)
	{
		return false;
	}

	return true;
}

var var::osdir() const
{
	THISIS(L"var var::osdir() const")
	THISISSTRING()

	//get a handle and return L"" if doesnt exist or is NOT a directory
    //boostfs::wpath pathx(toTstring((*this)).c_str());
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return L"";
        if (!boostfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=boostfs::last_write_time(pathx);
        //convert to ptime
        boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
        //convert to mv date and time
        int mvdate, mvtime;
        ptime2mvdatetime(ptimex, mvdate, mvtime);

		return L"" ^ FM ^ mvdate ^ FM ^ int(mvtime);

	}
	catch(...)
	{
		return L"";
	};
}

var var::oslist(const var& path, const var& spec, const int mode) const
{
	THISIS(L"var var::oslist(const var& path, const var& spec, const int mode) const")
	THISISDEFINED()
	ISSTRING(path)
	ISSTRING(spec)
	
    //returns an fm separated list of files and/or folders

    //http://www.boost.org/libs/filesystem/example/simple_ls.cpp

    bool filter=false;
    //boost::wregex re;
    boost::regex re;
    if (spec)
    {
     try
     {
         // Set up the regular expression for case-insensitivity
         //re.assign(toTstring(spec).c_str(), boost::regex_constants::icase);
         re.assign(spec.tostring().c_str(), boost::regex_constants::icase);
         filter=true;
     }
	 catch (boost::regex_error& e)
     {
         std::wcout << spec.towstring() << L" is not a valid regular expression: \""
         << e.what() << L"\"" << std::endl;
         return L"";
     }
    }

	bool getfiles=true;
	bool getfolders=true;
	if (mode==1) getfolders=false;
	else if (mode==2) getfiles=false;

	var filelist=L"";

	//get handle to folder
	//wpath or path before boost 1.34
	//boostfs::wpath full_path( boostfs::initial_path<boostfs::wpath>());
    //full_path = boostfs::system_complete(boostfs::wpath( toTstring(path), boostfs::native ));
	boostfs::path full_path( boostfs::initial_path());
    full_path = boostfs::system_complete(boostfs::path( path.tostring().c_str(), boostfs::native ));

	//quit if it isnt a folder
	if (!boostfs::is_directory(full_path))
		return filelist;

	//errno=0;
    //boostfs::wdirectory_iterator end_iter;
    //for ( boostfs::wdirectory_iterator dir_itr( full_path );
    boostfs::directory_iterator end_iter;
    for ( boostfs::directory_iterator dir_itr( full_path );
          dir_itr != end_iter;
          ++dir_itr )
    {
      try
      {

		//dir_itr->path().leaf()  changed to dir_itr->leaf() in three places
		//also is_directory(dir_itr->status()) changed to is_directory(*dir_itr)
		//to avoid compile errors on boost 1.33
		//http://www.boost.org/doc/libs/1_33_1/libs/filesystem/doc/index.htm

        //skip unwanted items
		if (filter&&!boost::regex_match(dir_itr->leaf(), re)) continue;

		//using .leaf instead of .status provided in boost 1.34 .. but does it work/efficiently
        //if ( boostfs::is_directory( dir_itr->status() ) )
        if ( boostfs::is_directory( *dir_itr ) )
        {
            if (getfolders)
				filelist^=FM ^ dir_itr->leaf();
        }
        //is_regular is only in boost > 1.34
        //else if ( boostfs::is_regular( dir_itr->status() ) )
        else// if ( boostfs::is_regular( dir_itr->status() ) )
        {
            if (getfiles)
				filelist^=FM ^ dir_itr->leaf();
        }
        //else
        //{
        //  //++other_count;
        //  //std::wcout << dir_itr->path().leaf() << L" [other]\n";
        //}

      }
      catch ( const std::exception & ex )
      {
          ex;
        //++err_count;
        //std::wcout << dir_itr->path().leaf() << L" " << ex.what() << std::endl;
      }
	}

	//delete first separator
	//NB splice is 1 based
	if (filelist!=L"") filelist.splicer(1,1,L"");

	return filelist;
}

var var::oscwd(const var& newpath) const
{
	THISIS(L"var var::oscwd(const var& newpath) const")
	//doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()//not needed if *this not used
	ISSTRING(newpath)

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	//wont compile on boost 1.33 so comment it out and make non-functional
	//until we have a reliable way to detect boost version
	//boost::filesystem::current_path(newpath.tostring());

	return oscwd();

}

var var::oscwd() const
{
	THISIS(L"var var::oscwd() const")
	//doesnt use *this - should syntax be changed to ossetcwd? and osgetcwd()?
	THISISDEFINED()//not needed if *this not used

	//TODO consider using complete() or system_complete()
	//"[Note: When portable behavior is required, use complete(). When operating system dependent behavior is required, use system_complete()."

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	std::string currentpath=boost::filesystem::current_path().string();

	return currentpath;

/*
	//http://www.boost.org/libs/filesystem/doc/tr2_proposal.html#Class-template-basic_path
	//boost::filesystem::path full_path( boost::filesystem::current_path());
	//get the current path as wstring
	std::string currentpath=boost::filesystem::current_path().string();
	std::wstring wcurrentpath=std::wstring(currentpath.begin(),currentpath.end());
    boost::filesystem::wpath full_path(wcurrentpath);

    return full_path.string();
*/
}

void var::ossleep(const int milliseconds) const
{
	THISIS(L"void var::ossleep(const int milliseconds) const")
	//doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()//not needed if *this not used

    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.nsec += milliseconds*1000000;
    boost::thread::sleep(xt);

//	usleep((int)(1000.0*milliseconds));
//	Sleep(1000*milliseconds);
}

}// namespace exodus

#endif
