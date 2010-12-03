//UPCOMING BOOST LOCALE!
//http://cppcms.sourceforge.net/boost_locale/html/index.html

//#include <unicode/ustring.h>
//#include <boost/regex/icu.hpp>
/*
http://www.unicode.org/versions/Unicode5.0.0/ch02.pdf#G13708

boost reg expressions and icu
http://www.boost.org/libs/regex/doc/icu_strings.html

http://en.wikipedia.org/wiki/UTF-8
http://unicode.org/faq/utf_bom.html

Test words (this document is saved as UTF8 unicode)
Iñtërnâtiônàlizætiøn windows cp1252
 في اليونا

 web page to try out multilingual
 http://www.unicode.org/iuc/iuc10/x-utf8.html

notes:

0. bytes c0 c1 f5-ff never appear in utf8
1. postgres bytea can be treated as UTF8 for comparisons/indexing etc without conversion
2. icu likes everything to be in utf16
3. icu offers a fast conversion between utf8 and utf16 but can only replace illegal bytes with one character fast
(neosys uses a modified one that converts between F8-FF and u2558-u255F in the web interface)

icu conversion functions allow custom function be convert illegal UTF8 pick field character bytes with some arbitrary unicode code points but what speed? this was not found/used ... maybe better than the custom function mentioned above

Strategy 1:

Store utf16 in postgres bytea and provide convertion to bytea(utf16)->utf8->text/number/date etc for sql comparisons/indexing etc

pros:
	no codeconversion during read/write/calculate
cons:
	continuous conversion on database operations with/by clauses
	double database size
	double data traffic read/write/calculate

Strategy 2:

Store utf8 in postgres bytea and convert to/from utf16 in the client program on read/write/calculate

pros:
	no code conversion during database operations with/by clauses
	smallest database size
	smallest data traffic read/write/calculate
pros:
	codeconversion during every read/write/calculate

UTF8 versus UTF16 implementation discussion
-------------------------------------------
this 2005 document advocates UTF8 but more for documents/interchange than implementations
http://www-128.ibm.com/developerworks/xml/library/x-utf8/

size issue is NOT the main thing - compatibility with api is the main problem
size issue is important for memory throughput and therefore processing speed
UTF16 is 100% larger for non ascii (0-127) characters
UTF8 is 50% larger than UTF16 for asian and CJK characters (but one CJK char = one word = many characters)
UTF8 can be thought of as a non-adaptive huffman compression format(ie "common" ascii requires fewer bits)
UTF8 is the only format not to have the endian issue
ORACLE/DB2/JBASE/POSTGRES is UTF8, SAP is UTF32, Windows/MSSQL is UTF16

UTF16 is very poorly supported on std Linux libraries since wchar_t is 4 bytes (gcc has compiler option for 2 bytes)
icu offers UTF16 string replacement but then we are intimately tied to icu library
UTF8 is supported very broadly in unix/linux libraries
windows/java/icu/iisjavascript is UTF16 (unavoidable conversion into UTF16 in IIS even if web page is sent as UTF8!)
i18n web pages are UTF8 in the end

UTF8 negatives
character indexing/iteration  using integers is slow (until code migrates to proper iterators)
slower for asian and CJK
complicated bit pattern
doesnt handle binary (lots of illegal characters/UTF16 doesnt have this problem IF you ignore surrogates as Java does)

UTF16 negatives
probably become highly dependent on icu (string instead of std::wstring)
slower for ascii
doesnt work with c std strings (unless you use java style encoding c0 80 to represent nulls)
endian issues not a problem for implementations only for transmissions

so in the end this is a compatibility issue

Unicode in DB2 version 9 (UTF8)
http://publib.boulder.ibm.com/infocenter/db2luw/v9/topic/com.ibm.db2.udb.admin.doc/doc/c0004821.htm
lots of good explanation including
- during utf16->utf8 conversion surrogate pairs used to be converted to two x three byte sequences but no more.
- they treat combining characters separately
- implements UCA for collation

qt strings are unicode

*/

//#include <boost/algorithm/string.hpp>
#include <wctype.h>

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <string.h>
#endif

#include <boost/scoped_array.hpp>
#include <locale.h>
#define MV_NO_NARROW
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus {

int var::localeAwareCompare(const std::wstring& str1, const std::wstring& str2) const
{
	if (str1.length()==0&&str2.length()==0)
		return 0;

#if defined(_MSC_VER) && defined(UNICODE)

	/*
	Now of course this points to what may be the best solution for a single function
    that will let you pass string length, ignore case, choose an appropriate locale,
    and work in different versions of Windows -- the master NLS collation function,
	CompareStringW !
	*/

	int comparison;

	//CompareStringW
	//comparison=CompareStringW(GetUserDefaultLCID(), 0,

	// UNICODE is always defined so CompareString is CompareStringW.
	comparison=CompareString(GetThreadLocale(), 0,
		(TCHAR*)str1.data(), int(str1.length()),
		(TCHAR*)str2.data(), int(str2.length()));
	switch (comparison) {
	case CSTR_LESS_THAN:
		return -1;
	case CSTR_GREATER_THAN:
		return 1;
	case CSTR_EQUAL:
		return 0;
	default:
		throw MVException( L"localeAwareCompare(" ^ str1 ^ L", " ^ str2 ^ L")\n");
	}

#elif defined(_MACOS)
    // order strings the same way as native applications do, and also respects
    // the "Order for sorted lists" setting in the International preferences panel
	const CFStringRef thisString=CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
		reinterpret_cast<const UniChar *>(str1.data()),str1.length(),kCFAllocatorNull);
	const CFStringRef otherString=CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
		reinterpret_cast<const UniChar *>(data2), length2, kCFAllocatorNull);

	const int result = CFStringCompare(thisString, otherString, kCFCompareLocalized);
	CFRelease(thisString);
	CFRelease(otherString);
	return result;

//why not collate::compare?
//#elseif defined(_UNIX)
#elif defined(strcoll)
#elif !defined(XYZZZZ)
//	setlocale (LC_COLLATE, "en_US.UTF-8");
	return wcscoll(str1.c_str(),str2.c_str());
#else
#error stop here
	return str1.compare(str2);
#endif
}

var& var::localeAwareChangeCase(const int lowerupper)
{
	if (var_mvstr.length()==0)
		return *this;

#if defined(_MSC_VER) && defined(UNICODE)

	//TODO http://stackoverflow.com/questions/1767946/getthreadlocale-returns-different-value-than-getuserdefaultlcid
	//Microsoft is deprecating the Locale ID in favor of the Locale Name from Vista onwards

	/*
	http://msdn.microsoft.com/en-us/library/dd318700%28v=VS.85%29.aspx
	int LCMapString(
		__in   LCID Locale,
		__in   DWORD dwMapFlags,
		__in   LPCTSTR lpSrcStr,
		__in   int cchSrc,
		__out  LPTSTR lpDestStr,
		__in   int cchDest
	);
	return 0 if it DOESNT succeed
	GetLastError()
    * ERROR_INSUFFICIENT_BUFFER. A supplied buffer size was not large enough, or it was incorrectly set to NULL.
    * ERROR_INVALID_FLAGS. The values supplied for flags were not valid.
    * ERROR_INVALID_PARAMETER. Any of the parameter values was invalid.
	*/

	//uppercasing can double the number of characters (eg german b to SS) can it triple?
	int buffersize=(int) var_mvstr.length()*2+2;
	boost::scoped_array<TCHAR> buffer( new TCHAR [buffersize]);
	if (buffer==0)
		throw MVException(var(L"Out of memory in changecase(). Need ")^int(buffersize)^L" characters");

	int tolowerupper=LCMAP_LINGUISTIC_CASING;
	if (lowerupper==1) 
		tolowerupper+=LCMAP_LOWERCASE;
	else if (lowerupper==2)
		tolowerupper+=LCMAP_UPPERCASE;

	//LCMapStringW
	int outputsize=LCMapStringW(
		GetThreadLocale(),
		tolowerupper,
		(TCHAR*)var_mvstr.data(),
		(int) var_mvstr.length(),
		buffer.get(),
		(int) buffersize);

	//cant convert for some reason. see above
	if (!outputsize)
	{
		//for now only ascii conversion
		if (lowerupper==1) 
			return converter(UPPERCASE_,LOWERCASE_);
		else if (lowerupper==2)
			return converter(LOWERCASE_, UPPERCASE_);
	}

	var_mvstr=std::wstring(buffer.get(),outputsize);

	return *this;

#elif 1
	//for now only ascii conversion
	//if (lowerupper==1)
	//	converter(UPPERCASE_,LOWERCASE_);
	//else if (lowerupper==2)
	//	converter(LOWERCASE_, UPPERCASE_);

	//for now only fairly simple one for one conversion

	//if (lowerupper==1) 
	//	boost::to_lower(var_mvstr);
	//else if (lowerupper==2)
	//	boost::to_upper(var_mvstr);

	size_t  length=var_mvstr.length();
	if (lowerupper==1) 
		for (size_t ptr=0; ptr<length; ++ptr)
			var_mvstr[ptr]=towlower(var_mvstr[ptr]);
	else if (lowerupper==2)
		for (size_t ptr=0; ptr<length; ++ptr)
			var_mvstr[ptr]=towupper(var_mvstr[ptr]);

	return *this;

#elif defined(_MACOS)

//why not collate::compare?
//#elseif defined(_UNIX)
#elif defined(strcoll)
#elif !defined(XYZZZZ)
////	setlocale (LC_COLLATE, "en_US.UTF-8");
//	return wcscoll(str1.c_str(),str2.c_str());
	//TODO unicode/native conversion
	converter(LOWERCASE_, UPPERCASE_);
#else
#error stop here
	return str1.compare(str2);
#endif
}

bool var::setxlocale() const
{

#if defined(_MSC_VER) && defined(UNICODE)

	/* http://msdn.microsoft.com/en-us/library/dd374051%28v=VS.85%29.aspx
	//locale list
	//XP 2003 http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
	//http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
	BOOL SetThreadLocale(
		__in  LCID Locale
	);
	*/
	//GetSystemDefaultLCID()
	//GetUserDefaultLCID()
	//LCID locale_lcid=1031;//German Standard
	//LCID locale_lcid=1032;//Greek
	//LCID locale_lcid=1055;//Turkish

	return SetThreadLocale((*this).toInt())!=NULL;

//#elif defined(_MACOS)
#else
	THISIS(L"bool var::setxlocale() const")
	THISISSTRING()

	//make a thread local locale if not done already
	//TODO do this in thread creation
	//TODO destroy threalocale in thread destruction *OTHERWISE MEMORY LEAK
	//to avoid checking on every setxlocale
	if (uselocale(NULL)==uselocale(LC_GLOBAL_LOCALE))
		uselocale(duplocale(uselocale(LC_GLOBAL_LOCALE)));

	return setlocale(LC_ALL,(*this).tostring().c_str())!=NULL;

#endif

}

var& var::getxlocale()
{
#if defined(_MSC_VER) && defined(UNICODE)
	*this=(int)GetThreadLocale();
	return *this;
//#elif defined(_MACOS)
#else
	//return "";
	*this=var(setlocale(LC_ALL,NULL));
	return *this;
#endif
}

} // namespace exodus

