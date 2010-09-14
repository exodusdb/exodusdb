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

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <iostream>

#include <exodus/mv.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus
{
//http://unicode.org/faq/utf_bom.html
#define STRICT_OR_LENIENT_CONVERSION lenientConversion
/*
TODO improve efficiency by avoiding new/copy/delete
TODO for speed determine utf32/16/8 by compile time macros instead of runtime sizeof(wchar_t)
http://www.firstobject.com/wchar_t-string-on-linux-osx-windows.htm
msvc: google: c++ "predefined macros" site:msdn.microsoft.com
gcc: gcc -dM -E - </dev/null
mingw: g++ -dM -E - <nul
*/

/*
iconv -f WINDOWS-1252 -t UTF-8
http://www.unicode.org/versions/Unicode5.0.0/ch02.pdf#G13708
In UTF-16, if a pointer points to a leading surrogate, a single
backup is required. In UTF-8, if a pointer points to a byte starting with 10xxxxxx (in
binary), one to three backups are required to find the beginning of the character.
*/

DLL_PUBLIC
std::string var::tostring() const
{
	THISIS(L"std::string var::tostring() const")
        THISISSTRING()

	//TODO! convert from internal UTF16/32 to external UTF8

	//allow for max 4 bytes per single utf8 byte (utf16 max bytes is four)
	if (sizeof(wchar_t)==4)
	{
		return stringfromUTF32((UTF32*)((*this).towstring().data()), (*this).length());
	}
	else if (sizeof(wchar_t)==2)
	{
		return stringfromUTF16((UTF16*)( (*this).towstring().data() ), (*this).length());
	}
	else if (sizeof(wchar_t)==1)
	{
		std::string result(var_mvstr.begin(),var_mvstr.end());
		return result;
	}
	else
	{
		//std::cout<<" UTF8>>wstring ERROR ";
		std::cerr<<"toUTF8: sizeof wchar_t must be 1, 2 or 4"<<std::endl;
		throw MVException("toUTF8: sizeof wchar_t " ^ var(int(sizeof(wchar_t))) ^ L" must be 1, 2 or 4");
	}

}

std::string toUTF8(const std::wstring& wstr1)
{
	//allow for max 4 bytes per single utf8 byte (utf16 max bytes is four)
	if (sizeof(wchar_t)==4)
	{
		return stringfromUTF32((UTF32*)(wstr1.data()), (unsigned int) wstr1.length());
	}
	else if (sizeof(wchar_t)==2)
	{
		return stringfromUTF16((UTF16*)(wstr1.data()), (unsigned int) wstr1.length());
	}
	else if (sizeof(wchar_t)==1)
	{
		return std::string(wstr1.begin(),wstr1.end());
	}
	else
	{
		//std::cout<<" UTF8>>wstring ERROR ";
		std::cerr<<"toUTF8: sizeof wchar_t must be 1, 2 or 4"<<std::endl;
		throw MVException("toUTF8: sizeof wchar_t " ^ var(int(sizeof(wchar_t))) ^ L" must be 1, 2 or 4");
	}

}

std::wstring wstringfromUTF8(const UTF8* sourcestart, const int sourcelength)
{

//ConversionResult ConvertUTF8toUTF16 (
//		const UTF8** sourceStart, const UTF8* sourceEnd, 
//		UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

	if (sizeof(wchar_t)==4)
	{
		UTF32* targetbuffer=new(UTF32[sourcelength]);
		UTF32* targetbufferptr=targetbuffer;
		//TODO check if ok
		ConversionResult conversionresult=ConvertUTF8toUTF32 (
			&sourcestart, sourcestart+sourcelength,
			&targetbufferptr, targetbuffer+sourcelength, STRICT_OR_LENIENT_CONVERSION);
		//TODO check if ok
		if (conversionresult)
		{
			delete targetbuffer;
			std::cerr<<"UTF Conversion Error - UTF32 to wstring"<<std::endl;
			throw MVException("UTF Conversion Error - UTF32 to wstring");
		}
		std::wstring result((wchar_t*)targetbuffer,targetbufferptr-targetbuffer);

		delete targetbuffer;

		return result;
	}
	else if (sizeof(wchar_t)==2)
	{
		//allow for max 4 bytes per single utf8 byte (utf16 max bytes is four)
		UTF16* targetbuffer=new(UTF16[sourcelength*4]);
		UTF16* targetbufferptr=targetbuffer;
		//TODO check if ok
		ConversionResult conversionresult=ConvertUTF8toUTF16 (
			&sourcestart, sourcestart+sourcelength,
			&targetbufferptr, targetbuffer+sourcelength*2, STRICT_OR_LENIENT_CONVERSION);
		//TODO check if ok
		if (conversionresult)
		{
			delete targetbuffer;
			std::cerr<<"UTF Conversion Error - UTF8 to wstring"<<std::endl;
			throw MVException("UTF Conversion Error - UTF8 to wstring");
		}
		std::wstring result((wchar_t*)targetbuffer,targetbufferptr-targetbuffer);

		delete targetbuffer;

		return result;
	}
	else if (sizeof(wchar_t)==1)
	{
		//1 to 1
		std::wstring result((wchar_t*)sourcestart,sourcelength);
		return result;
	}
	else
	{
		//std::cout<<" UTF8>>wstring ERROR ";
		std::cerr<<"wstringfromUTF8: sizeof wchar_t must be 1, 2 or 4"<<std::endl;
		throw MVException("wstringfromUTF8: sizeof wchar_t must be 1, 2 or 4");
	}

}

std::string stringfromUTF16(const UTF16* sourcestart, const int sourcelength)
{

	//TODO improve efficiency by avoiding new/copy/delete

//ConversionResult ConvertUTF16toUTF8 (
//	const UTF16** sourceStart, const UTF16* sourceEnd, 
//	UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);

	//allow for max 4 UTF8 bytes per utf16 2 byte char (utf8 max bytes is four)
	//actually we will probably restrict ourselves to unicode code points 0000-FFFF
	//in order to avoid any four byte UTF16 characters
	//in order to ensure indexing characters in UTF16 strings can be lightning fast and proper
	UTF8* targetbuffer=new(UTF8[sourcelength*4]);
	UTF8* targetbufferptr=targetbuffer;
	//TODO check if ok
	ConversionResult conversionresult=ConvertUTF16toUTF8 (
		&sourcestart, sourcestart+sourcelength,
		&targetbufferptr, targetbuffer+sourcelength*4, STRICT_OR_LENIENT_CONVERSION);
	//TODO check if ok
	if (conversionresult)
	{
		delete targetbuffer;
		//std::cout<<" UTF16>>8ERROR ";
		std::cerr<<"UTF Conversion Error - UTF16 TO UTF8"<<std::endl;
		throw MVException("UTF Conversion Error - UTF16 TO UTF8");
	}
	std::string result((char*)targetbuffer,targetbufferptr-targetbuffer);

	delete targetbuffer;
	
	return result;
}

std::string stringfromUTF32(const UTF32* sourcestart, const int sourcelength)
{

	//TODO improve efficiency by avoiding new/copy/delete

//ConversionResult ConvertUTF32toUTF8 (
//	const UTF32** sourceStart, const UTF32* sourceEnd, 
//	UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);

	//allow for max 4 UTF8 bytes per utf16 2 byte char (utf8 max bytes is four)
	//actually we will probably restrict ourselves to unicode code points 0000-FFFF
	//in order to avoid any four byte UTF16 characters
	//in order to ensure indexing characters in UTF16 strings can be lightning fast and proper
	UTF8* targetbuffer=new(UTF8[sourcelength*4]);
	UTF8* targetbufferptr=targetbuffer;
	//TODO check if ok
	ConversionResult conversionresult=ConvertUTF32toUTF8 (
		&sourcestart, sourcestart+sourcelength,
		&targetbufferptr, targetbuffer+sourcelength*4, STRICT_OR_LENIENT_CONVERSION);
	//TODO check if ok
	if (conversionresult)
	{
		delete targetbuffer;
		//std::cout<<" UTF32>>8ERROR ";
		std::cerr<<"UTF Conversion Error - UTF32 TO UTF8 "<< conversionresult<<std::endl;
		//throw MVException("UTF Conversion Error - UTF32 TO UTF8");
		throw 1;//MVException("");
	}
	std::string result((char*)targetbuffer,targetbufferptr-targetbuffer);

	delete targetbuffer;
	
	return result;
}

}//of namespace exodus

