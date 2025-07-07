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

/* UTF-8 bytewise encoding
Binary    Hex          Comments
0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding
*/

// ICU
//#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>
//
#include <boost/locale.hpp>
////#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include <exodus/varimpl.h>

namespace exo {

// exodus uses one locale per thread defined in varbfuncs.cpp
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
extern thread_local std::locale thread_boost_locale1;
#pragma clang diagnostic pop

static void init_boost_locale1() {
	if (thread_boost_locale1.name() != "*") {
		boost::locale::generator generator1;
		thread_boost_locale1 = generator1("");
	}
}

// Utility to check char ASCII (0-127)
//
// On x86, comparing a char for equality with an int is generally quicker than comparing it with another char.
//
// This is because on x86, int is the native word size, so operations on ints are usually faster than operations
// on smaller types like char. When comparing a char with an int, the char is usually sign-extended to an int
// (if it's a signed char) or zero-extended to an int (if it's an unsigned char),
// and then the comparison is performed as an int comparison. This can be done with a single instruction.
//
static inline bool char_is_ASCII(char c1) {
	// c++ char type signedness is implementation defined. typically unsigned on x86 and signed on arm except IOS
	// Therefore must NOT do comparisons < unless we know the chars are ASCII 0-127, nor do arithmetic outside 0-127.
	// 8th/top bit indicates non-ASCII. Could be signed or unsigned depending on compiler
	return (c1 & 0b1000'0000) == 0;
}

// Utility to check SV for ASCII (0-127)
//
static inline bool sv_is_ASCII(std::string_view str1) {
	// Optimise for ASCII
	// try ASCII uppercase to start with for speed
	// this may not be correct for all locales. eg Turkish I i İ ı mixing Latin and Turkish
	// letters.
	//for (const char& c : str1) {
	for (const char c : str1) {
		if (! char_is_ASCII(c))
			return false;
	}
	return true;
}

// Generic helper to handle char and u32_char wise conversion (mapping)
//
template<typename T1, typename T2, typename T3>
static void string_converter(T1& var_str, const T2 fromchars, const T3 tochars) {
	typename T1::size_type pos = T1::npos;

	// Optimise for single character replacement
	// No observable speedup
	//if (fromchars.len() == 1 and tochars.len() == 1) {
	//	std::replace(var_str.begin(), var_str.end(), fromchars[0], tochars[0]);
	//}

	// TODO separate algorithm for handling the removal of excess characters
	// instead of erasing them one by one

	auto tochars_size = tochars.size();
	while (true) {
		// locate (backwards) any of the from characters
		// because we might be removing characters
		// and it is faster to remove last character first
		pos = var_str.find_last_of(fromchars, pos);

		if (pos == T1::npos)
			break;

		// find which from character we have found
		//int fromcharn = static_cast<int>(fromchars.find(var_str[pos]));
		auto fromcharn = fromchars.find(var_str[pos]);

		if (fromcharn < tochars_size)
			//var_str.replace(pos, 1, tochars.substr(fromcharn, 1));
			var_str[pos] = tochars[fromcharn];
			//var_str.replace(pos, 1, tochars[fromcharn]);
		else
			var_str.erase(pos, 1);

		if (pos == 0)
			break;

		pos--;
	}
	return;
}

var  var_os::version() {
	std::ostringstream oss;
	oss << "Local:  " << GIT_BRANCH << " " << GIT_LOCAL_COMMIT_TIMESTAMP << " " << GIT_LOCAL_COMMIT_HASH << "\n"
	    << "Remote: " << GIT_BRANCH << " " << GIT_REMOTE_COMMIT_TIMESTAMP << " " << GIT_REMOTE_COMMIT_HASH_SHORT;
	var v = oss.str();

	v ^= "\n" ^ std::string(GIT_REPO_URL) + "/commit/" + GIT_LOCAL_COMMIT_HASH;

	v^= "\n" ^ std::string(GIT_REPO_URL) + "/archive/" + GIT_REMOTE_COMMIT_HASH_SHORT + ".tar.gz";

	return v;

}

bool var_os::eof() const {
	THISIS("bool var::eof() const")
	// assertVar(function_sig);

	return std::cin.eof();
}

// 3 usecs
bool var_os::hasinput(const int milliseconds) const {

	// Defined in haskey.cpp
	bool haskey(int milliseconds);

	return haskey(milliseconds);
}

bool var_os::input(in prompt /*=""*/) {

	THISIS("out  var::input(in prompt")
	assertVar(function_sig);
	ISSTRING(prompt)

	var default_input = this->assigned() ? (*this) : "";

	var_str.clear();
	var_typ = VARTYP_STR;

	// If stdin is not a terminal
	if (not this->isterminal(0)) {
		if (!std::cin.eof())
			std::getline(std::cin, var_str);
		return *this;
	}

	//linux terminal input line editing

//		//replace double quotes with \"
//		default_input.replacer("\"", "\\\"");
//		let cmd = "bash -c 'read -i " ^ default_input.quote() ^ " -e EXO_TEMP_READ && printf \"%s\" \"$EXO_TEMP_READ\"'";
//		//cmd.outputl("cmd=");
//		if (not this->osshellread(cmd)) {
//			// Report any error
//			// except bash terminal read returns 256 if Ctrl+D provided.
//			const var msg = lasterror();
//			const var exit_status = msg.field(" ", 2);
//			if (exit_status != 256)
//				msg.logputl();
//		}
//		if ((*this) == "")
//			std::cout << std::endl;

	//output any prompt and flush
	//if (prompt.len())
	//	prompt.output().osflush();

	// Defined in readline.cpp
	int input_readline(var&, const char* = "");

	int exit_key = input_readline(*this, prompt);

	return !exit_key;
}

// For nchars, use int instead of var to trigger error at point of calling not here
// Not binary safe if nchars = 0 because we allow line editing assuming terminal console
var&  var_os::inputn(const int nchars) {

	THISIS("out  var::inputn(const int nchars")
	assertVar(function_sig);

	// Return empty string by default
	var_str.clear();
	var_typ = VARTYP_STR;

	// nchars is 1
	//////////////

	// Wait for a key to be pressed and return it immediately.
	if (nchars == 1) {
		return this->keypressed(true);
	}

	// nchars < 0
	/////////////

	// Return current key pressed or "" if no key currently pressed.
	else if (nchars < 0) {
		return this->keypressed(false);
	}

	// chars > 1
	////////////

	// Wait for and return up to chars bytes
	else if (nchars > 1) {

		// Reserve space
		var_str.resize(nchars);

		// cin.read
		std::cin.read(var_str.data(), nchars);

		// Trim space in case not all chars available
		var_str.resize(std::cin.gcount());

	}

	// nchars = 0
	/////////////

	// Return all bytes available
	else if (nchars == 0) {

		// How many bytes available?
		std::streamsize bytes_available = std::cin.rdbuf()->in_avail();

		// Try to read only in std::string SSO (15 bytes)
		// Plus 1 to trigger fast exit
		int blocksize = bytes_available ?: 15;

		//	std::streamsize available = std::cin.rdbuf()->in_avail();
		int offset = 0;
		while (!std::cin.eof()) {

			// Reserve space/more space
			var_str.resize(offset + blocksize);

			std::cin.read(var_str.data() + offset, blocksize);

			// Trim space in case not all chars available
			int nchars_read = std::cin.gcount();
			if (nchars_read < blocksize) {
				var_str.resize(offset + nchars_read);
				break;
			}

			offset += blocksize;

			// Exponential increase in block size read
			//blocksize *= 2;

		}

	}

	return *this;
}

var&  var_os::keypressed(const bool wait /*=false*/) {

	THISIS("out  var::keypressed(const bool wait = false)")
	assertVar(function_sig);

	// Function declared in term_getkey.cpp
	char term_getkey(void);

	// Return empty string by default
	var_str.clear();
	var_typ = VARTYP_STR;

	// Quit if not a terminal
	if (!this->isterminal())
		return *this;

	// Wait for and return a single byte immediately a key is pressed.
	if (wait) {

		const int nchars = 1;

		while (!eof()) {

			char char1;
			{
				char1 = term_getkey();
			}

			// try again after a short delay if no key and not enough characters yet
			// TODO implement as poll/epoll/select
			// Really should test for EOF which is usually -1
			// Ctrl+D usually terminates input in posix
			// TODO Use waitfor()
			if (char1 < 0) {
				this->ossleep(10);
				continue;
			}

			// Enter/Return key always returns whatever has been entered so far
			//if (char1 < 0 || char1 == 0x0d)
			//	break;

			// add the character to the output
			//var_str += char1;
			var_str.push_back(char1);

			// quit if got the desired number of characters
			//nchars cannot be negative at this point
			if (var_str.size() >= static_cast<unsigned int>(nchars))
				break;
		}

	}

	// Input whatever key is currently pressed or "" if no key is currently pressed.
	// Can be multibyte for e.g. PgUp -> and Esc sequence
	////////////////////////////////////////////////////////////////////////////////
	else {

		if (not this->hasinput())
			return *this;

		for (;;) {
			char char1;
			{
				char1 = term_getkey();
			}

			// Quit if no (more) characters available
			// really should test for EOF which is usually -1
			// Ctrl+D usually terminates input in posix
			if (char1 < 0)
				break;

			//var_str += char1;
			var_str.push_back(char1);
		}
	}

	return *this;
}

//// Moved to var_base
//
////////
//// LEN
////////
//
//// Synonym for length for compatibility with pick's len() which is bytes
//var  var_stg::len() const {
//
//	THISIS("var  var_stg::len() const")
//	assertString(function_sig);
//
//	return var_str.size();
//}
//
//////////
//// EMPTY
//////////
//
//bool var_stg::empty() const {
//
//	THISIS("bool var_stg::empty() const")
//	assertString(function_sig);
//
//	return var_str.empty();
//}

/////////////
// TEXT WIDTH
/////////////

// Implementation of os wcswidth for terminal output of wide characters
// https://mitchellh.com/writing/grapheme-clusters-in-terminals
// list of terminal program support. Need switch on/off for programming?
// https://www.man7.org/linux/man-pages/man3/wcswidth.3.html
// The behavior of wcswidth() depends on the LC_CTYPE category of
// the current locale.
var  var_stg::textwidth() const {

	THISIS("var  var_stg::textwidth() const")
	assertString(function_sig);

	// If all ASCII then return count of non-control chars
	auto size = var_str.size();
	for (const unsigned char c : var_str) {
		if (!char_is_ASCII(c)) UNLIKELY
			goto unicode;
		if (c < '\x20') UNLIKELY
			size--;
	}
	return size;

//int wcswidth(const wchar_t *s, std::size_t n);
unicode:
	auto wstr1 = this->to_wstring();
	auto width = wcswidth(wstr1.data(),std::string::npos);

	// If any control chars present then replace them with spaces and try again
	if (width < 0) {
		int ncontrolchars = 0;
		std::replace_if(
			wstr1.begin(),
			wstr1.end(),
			// lambda function to replace and count control chars (0x00 to 0x1f)
			[&ncontrolchars](const char ch) {
				if (ch >= 32) LIKELY
					return false;
				ncontrolchars++;
				return true;
			},
			' '
		);
		width = wcswidth(wstr1.data(),std::string::npos) - ncontrolchars;

		// If still cannot get length then report an error and terminate
		if (width < 0) {
//			if (not this->oswrite("error_textwidth.txt")) {
//				this->lasterror().errputl();
//			}
			throw VarError("EXODUS: Error in " ^ var(__PRETTY_FUNCTION__) ^ " wcswidth failed - see error_textwidth.txt");
		}
	}

	return width;
}

//////////
// TEXTLEN utf-8 code points
//////////

var  var_stg::textlen() const {

	THISIS("var  var_stg::textlen()")
	assertString(function_sig);

	var result = 0;
	for (const char c : var_str) {
		// Count bytes that dont start with 10 in the high end two bits, since these are utf-8 continuation bytes
		result.var_int += (c & 0b1100'0000) != 0b1000'0000;
		//std::cout << c << " " << std::bitset<8>(c) << " " << result.var_int << std::endl;
	}
	return result;
}

///////////////////////////////////////////////////////////////////////
// TRIM - remove leading, trailing and excess internal spaces/character
///////////////////////////////////////////////////////////////////////

// trim

// Const
var  var_stg::trim(SV trimchars /*= " "*/) const& {

	THISIS("io   var_stg::trim(SV trimchars)")
	assertString(function_sig);

	return boost::algorithm::trim_all_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
IO   var_stg::trimmer(SV trimchars /*= " "*/) REF {

	// TODO reimplement with boost string trim_if algorithm
	// http://www.boost.org/doc/libs/1_39_0/doc/html/string_algo/reference.html

	THISIS("void var_stg::trimmer(SV trimchars) &")
	assertStringMutator(function_sig);

//	trimmerlast_helper(var_str, trimchars);
//	trimmerfirst_helper(var_str, trimchars);
//	//trimmerinner_helper only works after first and last trimchars are removed
//	trimmerinner_helper(var_str, trimchars);
	boost::algorithm::trim_all_if(var_str, boost::algorithm::is_any_of(trimchars));

	return THIS;
}

// trimfirst - remove leading/left bytes

// Const
var  var_stg::trimfirst(SV trimchars /*= " "*/) const& {

	THISIS("io   var_stg::trimfirst(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_left_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
IO   var_stg::trimmerfirst(SV trimchars /*= " "*/) REF {

	THISIS("void var_stg::trimmerfirst(SV trimchars) &")
	assertStringMutator(function_sig);

	boost::algorithm::trim_left_if(var_str, boost::algorithm::is_any_of(trimchars));

	return THIS;
}

// trimlast() - trim trailing/right bytes

// Const
var  var_stg::trimlast(SV trimchars /*= " "*/) const& {

	THISIS("io   var_stg::trimlast(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_right_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
IO   var_stg::trimmerlast(SV trimchars /*= " "*/) REF {

	THISIS("void var_stg::trimmerlast(SV trimchars) &")
	assertStringMutator(function_sig);

	//trimmerlast_helper(var_str, trimchars);
	boost::algorithm::trim_right_if(var_str, boost::algorithm::is_any_of(trimchars));

	return THIS;
}

//trimboth() - remove leading and trailing spaces/characters but not inner

// Const
var  var_stg::trimboth(SV trimchars /*= " "*/) const& {

	THISIS("io   var_stg::trimboth(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
IO   var_stg::trimmerboth(SV trimchars /*= " "*/) REF {

	THISIS("void var_stg::trimmerboth(SV trimchars) &")
	assertStringMutator(function_sig);

	boost::algorithm::trim_if(var_str, boost::algorithm::is_any_of(trimchars));

	return THIS;
}

// invert() - inverts lower 8 bits of UTF8 codepoints (not bytes)

// Mutate
IO   var_stg::inverter() REF {

	THISIS("void var_stg::inverter() &")
	assertStringMutator(function_sig);

	// xor each unicode code point, with the bits we want to toggle ... ie the bottom 8
	// since we will keep inversion within the same 256 byte pages of unicode codepoints
	// TODO invert directly in the UTF8 bytes - requires some cleverness

	// convert to char32.t string - four bytes per code point
	//std::u32string u32string1 = this->to_u32string();
	std::u32string u32_str1(*this);

	// invert only the lower 8 bits to keep the resultant code points within the same unicode
	// 256 byte page
	for (auto& c : u32_str1)
		c ^= char32_t(255);

	// convert back to utf8
	//this->from_u32string(u32_str1);
	*this = var(u32_str1);

	return THIS;
}

////////
// UCASE - Upper case
////////

//// Const
//var  var::ucase() const& {
//
//// Performance testing indicates 20% faster to
//// bulk clone a string and then amend it char by char.
//// than copy the characters one by one to the new location.
//// When there are few char conversions required, the
//// performance advantage seems to be up to 50%.
////
//// so we will use clone and ucaser.
////
////#if __cpp_lib_string_resize_and_overwrite < 202110L
//	let nrvo = this->clone();
//	nrvo.ucaser();
//	return nrvo;
////#else
////
////	THISIS("var  var::ucase() const&")
////	assertString(function_sig);
////
////	let rvo;
////	rvo.var_typ = VARTYP_STR;
////
////	rvo.var_str.resize_and_overwrite(
////		var_str.size(),
////		[this](char* buf, std::size_t /*buf_size*/) noexcept {
////
//////			assert(buf_size == var_str.size());
////
////			for (char c : var_str) {
////
////				// Quit if any non-ASCII
////				if (!char_is_ASCII(c))
////					return std::size_t(0);
////
////				// ASCII ucase
////				if (c >= 'a' && c <= 'z')
////					c -= 'z' - 'Z';
////
////				// rvo.var_str
////				*buf++ = c;
////			}
////
////			// Flag to resize_and_overwrite its new string size
////			// which we arrange to be the same as the requested buffer size.
////			return var_str.size();
////		}
////	);
////
////	//  If any unicode then run full unicode uppercase
////
//////	// On unconverted remainder of var_str
//////	if (rvo.var_str.size() != var_str.size())
//////		rvo.var_str += unicode_to_upper(var_str.substr(rvo.var_str.size()));
////
////	if (rvo.var_str.size() != var_str.size()) {
////
////		// ~6ms! For very first call for non-ASCII string ucaser or lcaser
////		// ~ 70ns for a one char non-ASCII string
////		// ~ 2.5ns per additional char in non-ASCII string
////		init_boost_locale1();
////
////		// Using boost (which calls icu)
////		rvo.var_str = boost::locale::to_upper(var_str, thread_boost_locale1);
////	}
////
////	return rvo;
////#endif
//}

// Mutate
IO   var_stg::ucaser() REF {

	THISIS("void var_stg::ucaser() &")
	assertStringMutator(function_sig);

#define EXO_ULCASE_TRY_ASCII
#ifdef EXO_ULCASE_TRY_ASCII

	// Optimise for ASCII
	// Attempt fast byte-wise conversion of ASCII
	// Try ASCII uppercase to start with for speed
	// TODO This may not be correct for all locales.
	// e.g. Turkish I i İ ı mixing ASCII Latin and UNICODE Turkish letters.
	// Perhaps only detect ASCII if the default locale is currently C.

	// If pure ASCII
	//
	// ~ 10 times faster than unicode case conversion
	//
	// 'in function' time measured using timebank/RAII)

	// 100,000 chars with no conversion required.

	// gcc 14 Ubuntu 24.04
	// 0.45 ns/char ucase and lcase regardless of if need to convert or not

	// clang 18 Ubuntu 24.04
	//      ~ 1ns to "convert" an empty string
	// + ~ 0.33ns per ASCII character unconverted (clang 18)
	// + ~ 0.45ns per ASCII character converted (all other compilers and versions)

	// Strangely clang 18 can ucase faster when no conversion is required
	// but lcase does not show the same ability.
	// Putting LIKELY destroys this optimisation.

	// return localeAwareChangeCase(1);

	auto allASCII = true;
	for (char& c : var_str) {

		// C/C++ standard says that it is implementation defined whether char is signed or unsigned.
		// On x86 char is generally signed and char is numerically from 0 up to +255
		// On ARM char is generally unsigned (except IOS) and char is numerically -128 up to +127
		// So char < char is implementation defined.

		// Check for ASCII by setting the lower 7 bits to zero and checking if 0.
		// Checking for ASCII will eliminate the need for static_cast<signed char>(c) below

		if (!(allASCII = char_is_ASCII(c)))
			break;

		// Using std::toupper - 2ns/char
		// toupper returns an int despite being given a char
		// Presumably safe to cast back to char
		// c = static_cast<char>(std::toupper(c));

		// Hand written code ~.33ns/op 5-6 times faster than std::toupper (which returns an int)
//		if (std::islower(c))
//			c = std::toupper(c);
		// Focus on lower case ASAP by testing >= 'a' 1st
		// 0x41 = A
		// 0x61 = a
		if (c >= 'a' && c <= 'z') {
			// LIKELY   // Stops clang 18 optimising ucase when no conversion is required
			// UNLIKELY // No change in performance
			c -= '\x20'; // 'a' - 'A';
		}
	}
	if (allASCII)
		return THIS;
#endif

	// ~6ms! For very first call for non-ASCII string ucaser or lcaser
	// ~ 70ns for a one char non-ASCII string
	// ~ 2.5ns per additional char in non-ASCII string
	init_boost_locale1();

	// boost calls icu

//	// Using boost locale
//	// gcc 14 100,000 chars with no conversion required and no ASCII preconv above
//	// 5.2 ns/char ucase
//	// 5.0 ns/char lcase
	var_str = boost::locale::to_upper(var_str, thread_boost_locale1);

	// Using boost algorithm - doesnt work
	// gcc 14 100,000 chars with no conversion required and no ASCII preconv above.
	// 4.2 ns/char ucase
	// 4.0 ns/char lcase
//	boost::algorithm::to_upper(var_str, thread_boost_locale1);

	return THIS;
}

////////
// LCASE - Lower case
////////

// Mutate
IO   var_stg::lcaser() REF {

	THISIS("void var_stg::lcaser() &")
	assertStringMutator(function_sig);

	// For comments, see ucaser above.

#ifdef EXO_ULCASE_TRY_ASCII

	auto allASCII = true;
	for (char& c : var_str) {

		if (!(allASCII = char_is_ASCII(c)))
			break;

		// Skip already lower case ASAP by testing <= 'Z' 1st
		if (c <= 'Z' && c >= 'A') {
			c += '\x20';
			continue;
		}

	}
	if (allASCII)
		return THIS;
#endif

	init_boost_locale1();

	var_str = boost::locale::to_lower(var_str, thread_boost_locale1);
//	boost::algorithm::to_lower(var_str, thread_boost_locale1);

	return THIS;
}


////////
// TCASE - Title Case
////////

// Mutate
IO   var_stg::tcaser() REF {

	THISIS("void var_stg::tcaser() &")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	init_boost_locale1();

	// should not title 1 2 or 3 letter words or perhaps just a list of exceptions as follows:
	// a abaft about above afore after along amid among an apud as aside at atop below but by
	// circa down for from given in into lest like mid midst minus near next of off on onto out
	// over pace past per plus pro qua round sans save since than thru till times to under until
	// unto up upon via vice with worth the and nor or yet so or a an as at but by circa for
	// from in into like mid of on onto out over per pro qua sans than thru to until unto up
	// upon via vice with the and nor or yet so

	var_str = boost::locale::to_title(var_str, thread_boost_locale1);

	return THIS;
}

////////
// FCASE - Fold case
////////

// fold case - standardise text for indexing/searching
// https://www.w3.org/International/wiki/Case_folding
// Note that accents are sometimes significant and sometime not. e.g. in French
//  cote (rating)
//  coté (highly regarded)
//  côte (coast)
//  côté (side)
// Case Folding - is a process of converting a text to case independent representation.
// For example case folding for a word "Grüßen" is "grüssen"
// where the letter "ß" is represented in case independent way as "ss".

// Mutate
IO   var_stg::fcaser() REF {

	THISIS("void var_stg::fcaser() &")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	init_boost_locale1();

	var_str = boost::locale::fold_case(var_str, thread_boost_locale1);

	return THIS;
}

////////////
// NORMALISE - Standardise chars
////////////

// *** USED TO NORMALISE ALL KEYS BEFORE READING OR WRITING ***
// *** otherwise can have two record with similar keys á and á
// postgres gives FALSE for the following:
// SELECT 'á' = 'á';

// normalise()
// normalise (unicode NFC, C=Compact ... norm_nfc)
// see "Unicode Normalization Forms" https://unicode.org/reports/tr15/
//"It is crucial that Normalization Forms remain stable over time. That is, if a string that does
// not
// have any unassigned characters is normalized under one version of Unicode,
// it must remain normalized under all future versions of Unicode."

// Mutate
IO   var_stg::normalizer() REF {

	THISIS("void var_stg::normalizer() &")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// optimise for ASCII which needs no normalisation
	if (sv_is_ASCII(var_str))
		return THIS;

	init_boost_locale1();

	// TODO see if the quick way to check if already in NFC format
	// is available in some library or if it is already built into boost locale normalize
	// because checking for being in normal form is very fast according to unicode docs

	// norm_nfc
	var_str = boost::locale::normalize(var_str, boost::locale::norm_nfc, thread_boost_locale1);
//	var_str = boost::locale::normalize(var_str, boost::locale::norm_default, thread_boost_locale1);

	return THIS;
}


/////////
// UNIQUE
/////////

// There is no memory or performance advantage for mutable call, only a consistent syntax for user

// Mutate
IO   var_stg::uniquer() REF {
	*this = this->unique();
	return THIS;
}

// Constant
var  var_stg::unique() const& {

	THISIS("var  var_stg::unique()")
	assertString(function_sig);

	// linemark
	var result = "";
	var start = 0;
	var bit;
	var delimiter;
	var sepchar = VM;
	int RMseq_plus1 = RM.ord() + 1;
	// bool founddelimiter = false;
	while (true) {

		// bit=this->remove(start, delimiter);
		bit = this->substr2(start, delimiter);

		// if (!founddelimiter && delimiter)
		if (delimiter)
			// sepchar=RM_-static_cast<int>(delimiter)+1;
			sepchar = var().chr(RMseq_plus1 - delimiter);

		if (bit.len()) {
			if (not(result.locateusing(sepchar, bit))) {
				//if (delimiter)
				result ^= bit ^ sepchar;
			}
		}
		if (not delimiter)
			break;
	}  // loop;
	//result.popper();
	if (not result.var_str.empty())
		result.var_str.pop_back();

	return result;
}

//////
// ORD - Ordinal of 1st byte (0-255)
//////

// BINARY - 1st byte
var  var_stg::ord() const {

	THISIS("var  var_stg::ord() const")
	assertString(function_sig);

	if (var_str.empty())
		return "";

	int byteno = var_str[0];
	if (byteno >= 0)
		return byteno;
	else
		return byteno + 256;
}

//////////
// TEXTORD - Ordinal of first UTF8 code point
//////////

var  var_stg::textord() const {

	THISIS("var  var_stg::textord() const")
	assertString(function_sig);

	if (var_str.empty())
		return "";

	// Get up to four bytes from input string since in UTF8 a unicode code point may occupy up to 4 bytes
	// Sadly boost::locale::conv cannot accept a string_view
	std::u32string str1 = boost::locale::conv::utf_to_utf<char32_t>(var_str.substr(0, 4));

	return uint32_t(str1[0]);
}

//////
// CHR - One char/byte from ordinal (0-255)
//////

// only returns BINARY bytes 0-255 (128-255) cannot be stored in the database unless with other
// bytes making up UTF8
var  var_stg::chr(const int charno) {
	return static_cast<char>(charno % 256);
}

////////////////
// CHAR_TO_WORDS - Official name of first UTF8 code point
////////////////

std::string char_to_words(UChar32 c) {
	char name[256];
	UErrorCode status = U_ZERO_ERROR;
	u_charName(c, U_UNICODE_CHAR_NAME, name, sizeof(name), &status);
	if (U_SUCCESS(status)) {
		return std::string(name);
	}
	UChar utf16[2];
	int32_t utf16_len = 0;
	u_strFromUTF32(utf16, 2, &utf16_len, &c, 1, &status);
	if (U_SUCCESS(status)) {
		char utf8[8];
		int32_t len = 0;
		u_strToUTF8(utf8, sizeof(utf8), &len, utf16, utf16_len, &status);
		if (U_SUCCESS(status)) {
			return std::string(utf8, len);
		}
	}
	return "";
}

// Utility to convert a Unicode name back to its single character number
UChar32 words_to_char(const std::string& words) {
	UErrorCode status = U_ZERO_ERROR;
	UChar32 c = u_charFromName(U_UNICODE_CHAR_NAME, words.c_str(), &status);
	if (U_SUCCESS(status) && c != U_SENTINEL) {
		return c;
	}
	return U_SENTINEL; // -1 if no match
}

//////////////
// TEXTCHRNAME - Official name of Unicode code point number.
//////////////

var  var_stg::textchrname(const int utf_codepoint) {
	// doesnt use *this at all (do we need a version that does?)

	// return var((char) int1);

//	if (!utf_codepoint)
//		return std::string("\0", 1);

//	std::wstring wstr1;
//	wstr1.push_back(wchar_t(uint32_t(utf_codepoint)));
//	return boost::locale::conv::utf_to_utf<char>(wstr1);
	return char_to_words(utf_codepoint);
}

	// Maximum C++ int: 2147483647 (hex: 0x7fffffff)
	// Maximum Unicode code point: 1114111 (hex: U+10ffff)
	// Max int (2147483647) is greater than max Unicode code point (1114111) by 2146369536.

//////////
// TEXTCHR - UTF8 char (1-4 bytes) from Unicode code point number.
//////////

// returns unicode 1-4 byte sequences (in utf8)
// returns empty string for some invalid unicode points like 0xD800 to 0xDFFF which is reserved for
// UTF16 0x110000 ... is invalid too
var  var_stg::textchr(const int utf_codepoint) {
	// doesnt use *this at all (do we need a version that does?)

	// return var((char) int1);

	if (!utf_codepoint)
		return std::string("\0", 1);

	std::wstring wstr1;
	wstr1.push_back(wchar_t(uint32_t(utf_codepoint)));
	return boost::locale::conv::utf_to_utf<char>(wstr1);
}

////////
// QUOTE - Wrap in double quotes
////////

// Const
var  var_stg::quote() const& {

	THISIS("var  var_stg::quote() const&")
	assertString(function_sig);

	var rvo = DQ_;
	rvo.var_str.append(var_str);
	rvo.var_str.push_back(DQ_);

	return rvo;
}

// Mutate
IO   var_stg::quoter() REF {

	THISIS("void var_stg::quoter() &")
	assertStringMutator(function_sig);

	// Use std::string "replace" to insert
	var_str.replace(0, 0, _DQ);
	var_str.push_back(DQ_);

	return THIS;
}


/////////
// SQUOTE - Wrap in single quotes
/////////

// Const
var  var_stg::squote() const& {

	THISIS("var  var_stg::squote() const&")
	assertString(function_sig);

	var rvo = SQ_;
	rvo.var_str.append(var_str);
	rvo.var_str.push_back(SQ_);

	return rvo;
}

// Mutate
IO   var_stg::squoter() REF {

	THISIS("void var_stg::squoter() &")
	assertStringMutator(function_sig);

	// std::string "replace" to insert
	var_str.replace(0, 0, _SQ);
	var_str.push_back('\'');

	return THIS;
}


//////////
// UNQUOTE - Remove outer pait of double OR single quotes
//////////

// Const
var  var_stg::unquote() const& {

	THISIS("var  var_stg::unquote() const&")
	assertString(function_sig);

	// Removes MATCHING beginning and terminating " or ' characters.
	// Also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such
	// characters.

	var rvo;
	rvo.var_typ = VARTYP_STR;
	char char0;

	// No change if no length
	std::size_t len = var_str.size();
	if (len < 2) {
return_this:
		rvo.var_str = var_str;
		return rvo;
	}

	char0 = var_str[0];

	// No change if not starting " or '
	if (char0 != DQ_ && char0 != SQ_)
		goto return_this;

	// No change if terminating character ne starting character
	if (var_str.back() != char0)
		goto return_this;

	// Copy over all except the first character
	rvo.var_str = var_str.substr(1);

	// Remove the last char if any
	if (!rvo.var_str.empty())
		rvo.var_str.pop_back();

	return rvo;
}

// Mutate
IO   var_stg::unquoter() REF {

	THISIS("void var_stg::unquoter() &")
	assertStringMutator(function_sig);

	// removes MATCHING beginning and terminating " or ' characters
	// also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such
	// characters

	// no change if no length
	std::size_t len = var_str.size();
	if (len < 2)
		return THIS;

	char char0 = var_str[0];

	// no change if not starting " or '
	if (char0 != '\"' && char0 != '\'')
		return THIS;

	// no change if terminating character ne starting character
	if (var_str[len - 1] != char0)
		return THIS;

	// erase first (and last character if more than one)
	var_str.erase(0, 1);
	if (len)
		var_str.erase(len - 2, 1);

	return THIS;
}


/////////
// PASTER - Insert a substr after deleting 0-N chars.
/////////

// 1. paste replace

// Mutate
IO   var_stg::paster(const int pos1, const int length, SV replacestr) REF {

	THISIS("void var_stg::paster(const int pos1, const int length, SV replacestr) &")
	assertStringMutator(function_sig);
	//ISSTRING(insertstr)

	std::size_t start0;
	std::size_t lengthb;

	// First work out start index from start position
	// Depends on length of string, if position is negative
	if (pos1 <= 0) {
		if (pos1 == 0 || static_cast<std::size_t>(-pos1) > var_str.size())
			// Negative start index means 0
			// abcdef[-8,2] -> abcdef[1,2] ie ab will be replaced
			start0 = 0;
		else
			// abcdef[-3,2] -> abcdef[4,2] ie de will be replaced
			start0 = var_str.size() + pos1;
	}
	else
		start0 = pos1 - 1;

	// Negative length simply moves the start char backwards
	// and the length is up to and including the start char
	if (length < 0) {

		// abcdef[4,-2]  -> abcdef[3,2]
		// abcdef[4,-99] -> abcdef[1,4]

		//int start0_save = start0;
		lengthb = start0 + 1;

		//start0 = start0 + length + 1;
		if (static_cast<std::size_t>(-length) > start0) {
			lengthb = start0 + 1;
			start0 = 0;
		} else {
			start0 = start0 + length + 1;
			lengthb = -length;
		}
//		if (start0 < 0) {
//			start0 = 0;
//			//lengthb = start0_save + 1;
//		} else
//			lengthb = -length;

		//std::cerr << "start0  = " << start0  << std::endl;
		//std::cerr << "lengthb = " << lengthb << std::endl;
	} else
		lengthb = length;

	if (static_cast<unsigned int>(start0) >= var_str.size()) {
		//if (newstr.var_str.size())
			var_str += replacestr;
	} else {
		//if (replacestr.var_str.size())
			var_str.replace(start0, lengthb, replacestr);
		//else
		//	var_str.erase(start0,lengthb);
	}

	return THIS;
}

/////////
// PASTER - Insert a substr at char position.
/////////

// 2. paste insert at

// Mutate
IO   var_stg::paster(const int pos1, SV insertstr) REF {

	THISIS("void var_stg::paster(const int pos1, SV insertstr) &")
	assertStringMutator(function_sig);
	//ISSTRING(insertstr)

	if (pos1 > 0) {
		if (static_cast<unsigned int>(pos1) > var_str.size())
			// abc(4, x) -> abcx
			// abc(5, x) -> abcx
			var_str += insertstr;
		else
			// abc(1, x) -> xabc
			// abc(2, x) -> axbc
			// abc(3, x) -> abxc
			var_str.insert(pos1 - 1, insertstr);
	}

	else if (pos1 < 0) {
		if (static_cast<unsigned int>(-pos1) >= var_str.size())
			// abc(-4, x) -> xabc
			// abc(-3, x) -> xabc
			var_str.insert(0, insertstr);
		else
			// abc(-2, x) -> axbc
			// abc(-1, x) -> abxc
			var_str.insert(var_str.size() + pos1, insertstr);
	}
	else
		// abc(0, x) -> xabc
		var_str.insert(0, insertstr);

	return THIS;
}

/////////
// PREFIX - Insert a substr at beginning.
/////////

// Constant
var  var_stg::prefix(SV prefixstr) const& {

	THISIS("var  var_stg::prefix(SV prefixstr)")
	assertString(function_sig);

	var nrvo = prefixstr;

	nrvo.var_str.append(this->var_str);

	return nrvo;
}

// Mutate
IO   var_stg::prefixer(SV prefixstr) REF {

	THISIS("void var_stg::prefixer(SV prefixstr) &")
	assertStringMutator(function_sig);

	var_str.insert(0, prefixstr);

	return THIS;
}


//////
// POP - Remove last the char/byte of string.
//////

// Mutate
IO   var_stg::popper() REF {

	THISIS("void var_stg::popper() &")
	assertStringMutator(function_sig);

	if (!var_str.empty())
		var_str.pop_back();

	return THIS;
}

//////
// STR - Generate a repeated substr.
//////

var  var_stg::str(const int num) const {

	THISIS("var  var::str(const int num) const")
	assertString(function_sig);

	var newstr = "";

	//negative num returns "" in loop below
	if (num < 0)
		return newstr;

	int basestrlen = static_cast<int>(var_str.size());
	if (basestrlen == 1) {
		newstr.var_str.resize(num, var_str.at(0));
	}
	else if (basestrlen) {
		for (int ii = num; ii > 0; --ii)
			newstr.var_str.append(var_str);
	}
	return newstr;
}

////////
// SPACE - Generate a string of N spaces.
////////

var  var_stg::space(const int nspaces) {

	THISIS("var  var::space() static")
//	assertNumeric(function_sig);

	var nrvo;
	nrvo.var_typ = VARTYP_STR;
	if (nspaces > 0)
		nrvo.var_str.resize(nspaces, ' ');

	return nrvo;
}

//////
//crop - Remove superfluous FM, VM. e.g. VM before FM etc.
//////

// Mutate
IO   var_stg::cropper() REF {

	THISIS("void var::cropper() &")
	assertStringMutator(function_sig);

	auto cur_iter = var_str.begin();
	auto end_iter = var_str.end();

	auto beg_iter = var_str.begin();
	auto out_iter = beg_iter;

	while (cur_iter != end_iter) {

		char charx = *cur_iter++;

		// Simply append ordinary characters
		if (!char_is_ASCII(charx) || charx > RM_ || charx < STM_) {
			*out_iter++ = charx;
			continue;
		}

		// Found one of the fms

		// "Remove" all lower separators from the end of the current output
		// by decrementing the output iterator
		// e.g. aaa]]]^bbb -> aaa^bbb
		while (out_iter != beg_iter) {
			char lastchar = *(out_iter-1);
			if (char_is_ASCII(lastchar) && lastchar >= STM_ && lastchar < charx)
				out_iter--;
			else
				break;
		}

		// "Append" the separator by incrementing the output iterator
		*out_iter++ = charx;
	}

	// Trim the string to its new size
	// Determined by the output iterator
	var_str.resize(static_cast<std::size_t>(out_iter - var_str.begin()));

	// Remove all trailing fms.
	// The above algorithm only trims superfluous fms within the string.
	auto last_not_fm = var_str.find_last_not_of(_RM _FM _VM _SM _TM _STM);
	if (last_not_fm == std::string::npos)
		var_str.clear();
	else
		var_str.resize(last_not_fm + 1);

	return THIS;
}

////////
// Lower - Changes FM to VM, VM to SM etc. STM is left unchanged.
////////

// Mutate
IO   var_stg::lowerer() REF {

	THISIS("void var::lowerer() &")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// note: rotate lowest sep to highest
	//this->converter(_RM _FM _VM _SM _TM _STM, _FM _VM _SM _TM _STM _RM);

	//bottom marks get crushed together but STM is infrequently used
	// reversible by raiser only if no STM chars are present - which are not common
//	this->converter(_RM _FM _VM _SM _TM, _FM _VM _SM _TM _STM);
//	string_converter(var_str, _RM _FM _VM _SM _TM, _FM _VM _SM _TM _STM);

	for (char& c : var_str) {

		// static_cast<unsigned char>(c) is well-defined ONLY if the value of c is within the range of unsigned char which is 0-127.
		// If the value is outside the range 0-127, the behavior is implementation-defined.
		// In other words you cannot work with non-ASCII characters reliably.

		// Skip non-ASCII
		if (!char_is_ASCII(c))
			continue;

		// Skip > RM_
		if (c > RM_) LIKELY
			continue;

		// Bump DOWN all field marks except the bottom one (STM_)
		if (c > STM_)
			c -=1;
	}

	return THIS;
}

///////
// RAISE - Change VM to FM, SM to VM etc. RM is left unchanged.
////////

// Mutate
IO   var_stg::raiser() REF {

	THISIS("void var::raiser() &")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// note: rotate highest sep to lowest
	// advantage is it is reversible by lowerer but the problem is that the smallest delimiter becomes the largest
	//this->converter(_FM _VM _SM _TM _STM _RM, _RM _FM _VM _SM _TM _STM);

	// top two marks get crushed together but RM is rarely used
	// reversible by lowerer only if no RM are present - which are rare
//	this->converter(_FM _VM _SM _TM _STM, _RM _FM _VM _SM _TM);
//	string_converter(var_str, _FM _VM _SM _TM _STM, _RM _FM _VM _SM _TM);

	for (char& c : var_str) {

		// static_cast<unsigned char>(c) is well-defined ONLY if the value of c is within the range of unsigned char which is 0-127.
		// If the value is outside the range 0-127, the behavior is implementation-defined.
		// In other words you cannot work with non-ASCII characters reliably.

		// Skip non-ASCII
		if (!char_is_ASCII(c))
			continue;

		// Skip > FM_
		if (c > FM_) LIKELY
			continue;

		// Bump UP all field marks except the top one (RT_)
		if (c >= STM_)
			c +=1;
	}

	return THIS;
}

//////////
// CONVERT - Replaces chars/bytes with other chars/bytes or deleted them.
//////////

// convert() - replaces one by one in string, a list of characters with another list of characters
// If the target list is shorter than the source list of characters then characters are deleted
//var  var_stg::convert(in fromchars, in tochars) const& {

// Mutate
//io   var_stg::converter(in fromchars, in tochars) & {
IO   var_stg::converter(SV fromchars, SV tochars) REF {

	THISIS("void var::converter(SV fromchars, SV tochars) &")
	assertStringMutator(function_sig);

	//string_converter(var_str, fromchars.var_str, tochars.var_str);
	string_converter(var_str, fromchars, tochars);

	return THIS;
}


// Mutate
IO   var_stg::textconverter(SV fromchars, SV tochars) REF {

	THISIS("void var::converter(in fromchars, in tochars) &")
	assertStringMutator(function_sig);

	// all ASCII -> bytewise conversion for speed
	if (sv_is_ASCII(fromchars) && sv_is_ASCII(tochars)) {
		string_converter(var_str, fromchars, tochars);
	}

	// any non-ASCI -> convert to wide before conversion, then back again
	else {

		// convert everything to from UTF8 to wide string
		std::u32string u32_str1 = this->to_u32string();
		//std::u32string u32_fromchars = var(fromchars).to_u32string();
		//std::u32string u32_tochars = var(tochars).to_u32string();
		//std::u32string u32_fromchars = boost::locale::conv::utf_to_utf<char32_t>(fromchars);
		//std::u32string u32_tochars = boost::locale::conv::utf_to_utf<char32_t>(tochars);
		std::u32string u32_fromchars = boost::locale::conv::utf_to_utf<char32_t>(std::string(fromchars));
		std::u32string u32_tochars = boost::locale::conv::utf_to_utf<char32_t>(std::string(tochars));

		// convert the wide characters
		string_converter(u32_str1, u32_fromchars, u32_tochars);

		// convert the string back to UTF8 from wide string
		//this->from_u32string(u32_str1);
		*this = var(u32_str1);
	}
	return THIS;
}

////////
// PARSE - Replaces seps with FMs except inside double and single quotes. Backslash escapes.
////////

// Mutate
IO   var_stg::parser(char sepchar) REF {

	THISIS("void var::parser(char sepchar) &")
	assertStringMutator(function_sig);

	// e.g. parse words using spaces into an FM delimited string leaving quoted phrases intact.
	//
	// e.g. "select xo_clients with type = " " or name contains ' ' and with balance > 100 {R}"
	// ---> "select^xo_clients^with^type^=^" "^or^name^contains^' '^and^with^balance^>^100^{R}"
	//
	// e.g. abc xyz qwe Spaces are used to parse fields.
	// ---> abc^xyz^qwe
	//
	// e.g. abc " "      Spaces are preserved inside double quotes.
	// ---> abc^" "
	//
	// e.g. abc ' '      Spaces are preserved inside single quotes.
	// ---> abc^' '
	//
	// e.g. abc "a \"b " Escaped double quotes are preserved inside double quotes.
	// ---> abc^"a \"b "
	//
	// e.g. abc 'a \'b ' Escaped single quotes are preserved inside single quotes.
	// ---> abc^'a \'b '
	//
	// e.g. abc d\ ef    Escaped spaces are preserved as spaces.
	// ---> abc^d\ ef
	//
	// e.g. abc a\b      All backslashes are preserved.
	// ---> abc^a\b
	//

	for (auto it = var_str.begin(); it != var_str.end(); ++it) {
		char ch1 = *it;
		if (ch1 == sepchar) {
			*it = FM_; // Replace one sepchar
		} else if (ch1 == '"' || ch1 == '\'') {
			for (;;) {
				if (++it == var_str.end()) return THIS;
				char ch2 = *it;
				if (ch2 == ch1) {
					break; // Found closing quote
				}
				if (ch2 == '\\') {
					++it; // Skip '\'
					if (it == var_str.end()) return THIS;
				}
			}
		} else if (ch1 == '\\') {
			++it; // Skip '\'
			if (it == var_str.end()) return THIS;
		} else {
			// Normal char
		}
	} // it
	return THIS;
}

/////////
// FCOUNT - Field count. Empty string is zero.
/////////

// TODO make a char and char version for speed
var  var_stg::fcount(SV sepstr) const {

	THISIS("var  var::fcount(SV sepstr) const")
	assertString(function_sig);
	//ISSTRING(sep)

	if (var_str.empty())
		return 0;

	if (sepstr.empty())
		return "";

	return this->count(sepstr) + 1;
}
////////
// COUNT - Counts occurrences of a given char/byte/substr.
////////

var  var_stg::count(SV sepstr) const {

	THISIS("var  var::count(SV sepstr) const")
	assertString(function_sig);

	if (sepstr.empty())
		return "";

	std::size_t substr_len = sepstr.size();

	// find the starting position of the field or return ""
	std::size_t start_pos = 0;
	int fieldno = 0;
	while (true) {
		start_pos = var_str.find(sepstr, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return fieldno;
		// start_pos++;
		start_pos += substr_len;
		fieldno++;
	}
}

////////
// INDEX - Returns the char/byte no (1 base position) of the first occurrence of a given substr starting from a given position. 0 If not found.
////////

var  var_stg::index(SV substr, const int startindex) const {

	THISIS("var  var::index(SV substr, const int startindex) const")
	assertString(function_sig);

	if (substr.empty())
		return "";

	// find the starting position of the field or return ""
	std::size_t start_pos = startindex - 1;
	start_pos = var_str.find(substr, start_pos);

	// not found, return 0
	if (start_pos == std::string::npos)
		return 0;

	return var(static_cast<int>(start_pos) + 1);
}

/////////
// INDEXR - Like index() but starting from the last char/byte working backwards.
/////////

// reverse search
var  var_stg::indexr(SV substr, const int startindex) const {

	THISIS("var  var::indexr(SV substr, const int startindex) const")
	assertString(function_sig);

	if (substr.empty())
		return "";

	std::size_t start_pos;

	if (startindex == 0) {
		start_pos = std::string::npos;

	} else if (startindex > 0) {
		start_pos = startindex - 1;

	} else {

		// startindex < 1

		// prevent negative index starting before beginning of string
		// e.g. -startindex(-2) > strsize(1) is true
		if (static_cast<std::size_t>(-startindex) > var_str.size())
			return 0;

		// e.g. startpos(0) = strsize(1) + startindex(-1)
		start_pos = var_str.size() + startindex;
	}

	// find the starting position of the target
	start_pos = var_str.rfind(substr, start_pos);

	// not found, return 0
	if (start_pos == std::string::npos)
		return 0;

	return var(static_cast<int>(start_pos) + 1);
}

/////////
// INDEXN // 1 based starting byte no of the Nth occurrence of a given char/byte/subst. 0 if not present.
/////////

var  var_stg::indexn(SV substr, const int occurrenceno) const {

	//THISIS("var  var::index(SV substr, const int occurrenceno) const")
	THISIS("var  var::index(SV substr) const")
	assertString(function_sig);

	//TODO implement negative occurenceno as meaning backwards from the end
	//eg -1 means the last occurrence

	if (substr.empty())
		return "";

	std::size_t start_pos = 0;
	std::size_t substr_len = substr.size();

	// negative and 0th occurrence mean the first
	int countdown = occurrenceno >= 1 ? occurrenceno : 1;

	for (;;) {

		// find the starting position of the field or return ""
		start_pos = var_str.find(substr, start_pos);

		// past of of string?
		if (start_pos == std::string::npos)
			return 0;

		--countdown;

		// found the right occurrence
		if (countdown == 0)
			return (static_cast<int>(start_pos) + 1);

		// skip to character after substr (not just next character)
		// start_pos++;
		start_pos += substr_len;
	}

	// should never get here
	//std::unreachable();
	//return 0;
}

////////////////
// NUMBERINWORDS - Convert a number to it's long form in words.
////////////////

var  var_stg::numberinwords(in langname_or_locale_id) {

	THISIS("var  var::numberinwords(in langname_or_locale_id)")
	assertNumeric(function_sig);
	langname_or_locale_id.assertString(function_sig);

	var exo_languages =
		"english" _VM
		"arabic" _VM
		"greek" _VM
		"spanish" _VM
		"french";

	var iso_languages =
		"en_GB" _VM
		"ar_AE" _VM
		"el_CY" _VM
		"es_US" _VM
		"fr_FR";

	// Determine locale_id
	//SV locale_name = "ar_AE.utf8";
	var langn;
	var langcode;
	if (exo_languages.locate(langname_or_locale_id.lcase(),langn))
		langcode = iso_languages(1, langn);
	else if (langname_or_locale_id == "")
		langcode = "en_GB";
	else
		langcode = langname_or_locale_id;

	var locale_id = langcode;
	if (not locale_id.ends(".utf8"))
		locale_id ^= ".utf8";

	// skip locale-gen since
	// 1. It is not necessary since boost::locale:generator seems to manage creation of all languages
	// 2. It is slow and doesnt take effect until a new process is started

	// Create the right language locale
	boost::locale::generator locale_generator1;
	var result;
	try {
		std::locale boost_generated_locale1=locale_generator1(locale_id.toString());
		// create a locale imbued stringstream
		std::ostringstream ss;
		ss.imbue(boost_generated_locale1);

		ss << boost::locale::as::spellout << this->toDouble();

		result = ss.str();
	}
	catch (...) {
			throw VarError("number_in_words: boost_locale_generator failed for " ^ locale_id);
	}

	return result;

}

	ND var  var_stg::ucase() const& {var nrvo = this->clone(); nrvo.ucaser(); return nrvo;}
	ND var  var_stg::lcase() const& {var nrvo = this->clone(); nrvo.lcaser(); return nrvo;}
	ND var  var_stg::tcase() const& {var nrvo = this->clone(); nrvo.tcaser(); return nrvo;}
	ND var  var_stg::fcase() const& {var nrvo = this->clone(); nrvo.fcaser(); return nrvo;}
	ND var  var_stg::normalize() const& {var nrvo = this->clone(); nrvo.normalizer(); return nrvo;}
	ND var  var_stg::invert() const& {var nrvo = this->clone(); nrvo.inverter(); return nrvo;}
	ND var  var_stg::lower() const& {var nrvo = this->clone(); nrvo.lowerer(); return nrvo;}
	ND var  var_stg::raise() const& {var nrvo = this->clone(); nrvo.raiser(); return nrvo;}
	ND var  var_stg::crop() const& {var nrvo = this->clone(); nrvo.cropper(); return nrvo;}
	ND var  var_stg::paste(const int pos1, const int length, SV replacestr) const& {var nrvo = this->clone(); nrvo.paster(pos1, length, replacestr); return nrvo;}
	ND var  var_stg::paste(const int pos1, SV insertstr) const& {var nrvo = this->clone(); nrvo.paster(pos1, insertstr); return nrvo;}
	ND var  var_stg::pop() const& {var nrvo = this->clone(); nrvo.popper(); return nrvo;}
	ND var  var_stg::fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const& {var nrvo = this->clone(); nrvo.fieldstorer(separator, fieldno, nfields, replacement); return nrvo;}
	ND var  var_stg::substr(const int pos1, const int length) const& {var nrvo = this->clone(); nrvo.substrer(pos1, length); return nrvo;}
	ND var  var_stg::substr(const int pos1) const& {var nrvo = this->clone(); nrvo.substrer(pos1); return nrvo;}
	ND var  var_stg::convert(SV fromchars, SV tochars) const& {var nrvo = this->clone(); nrvo.converter(fromchars,tochars); return nrvo;}
	ND var  var_stg::textconvert(SV fromchars, SV tochars) const& {var nrvo = this->clone(); nrvo.textconverter(fromchars,tochars); return nrvo;}
	ND var  var_stg::parse(char sepchar /*= ' '*/) const& {var nrvo = this->clone(); nrvo.parser(sepchar); return nrvo;}
	ND var  var_stg::update(const int fieldno, const int valueno, const int subvalueno, in replacement) const& {var nrvo = this->clone(); nrvo.updater(fieldno, valueno, subvalueno, replacement); return nrvo;}
	ND var  var_stg::update(const int fieldno, const int valueno, in replacement)                    const& {var nrvo = this->clone(); nrvo.updater(fieldno, valueno, 0, replacement); return nrvo;}
	ND var  var_stg::update(const int fieldno, in replacement)                                       const& {var nrvo = this->clone(); nrvo.updater(fieldno, 0, 0, replacement); return nrvo;}
	ND var  var_stg::insert(const int fieldno, const int valueno, const int subvalueno, in insertion) const& {var nrvo = this->clone(); nrvo.inserter(fieldno, valueno, subvalueno, insertion); return nrvo;}
	ND var  var_stg::insert(const int fieldno, const int valueno, in insertion)                      const& {var nrvo = this->clone(); nrvo.inserter(fieldno, valueno, 0, insertion); return nrvo;}
	ND var  var_stg::insert(const int fieldno, in insertion)                                         const& {var nrvo = this->clone(); nrvo.inserter(fieldno, 0, 0, insertion); return nrvo;}
	ND var  var_stg::remove(const int fieldno, const int valueno /*= 0*/, const int subvalueno /*= 0*/)      const& {var nrvo = this->clone(); nrvo.remover(fieldno, valueno, subvalueno); return nrvo;}

	ND var  var_stg::field2(SV separator, const int fieldno, const int nfields /*= 1*/) const
	{
		if (fieldno >= 0) LIKELY
			return field(separator, fieldno, nfields);
		return field(separator, this->count(separator) + 1 + fieldno + 1, nfields);
	}

	ND var  var_stg::b(const int pos1, const int length) const& {return this->substr(pos1, length);}
	ND var  var_stg::b(const int pos1) const& {return this->substr(pos1);}
	   var  var_stg::b(const int pos1, SV delimiterchars, out pos2) const {return this->substr(pos1, delimiterchars, pos2);}
	   var  var_stg::b2(io pos1, out field_mark_no) const {return this->substr2(pos1, field_mark_no);}

	ND var  var_stg::search(SV regex_str) const {var startchar1 = 1; return this->search(regex_str, startchar1);}
	ND var  var_stg::search(const rex& regex) const {var startchar1 = 1; return this->search(regex, startchar1);}

//	ND var  var_stg::replace(const rex& regex, ReplacementFunction auto repl_func) const {
//
//		// Lambda to bridge the callable to a function pointer + context
//		struct Context {decltype(repl_func)* lambda;};
//		Context ctx{&repl_func};
//
//		// Non-capturing lambda to adapt the callable to the function pointer signature. [[Undocumented]]
//		auto bridge = [](const var& match_str, void* ctx) -> var {
//			auto* context = static_cast<Context*>(ctx);
//			return (*context->lambda)(match_str); // Invoke the original lambda
//		};
//
//		return replace(regex, bridge, &ctx);
//	}

	ND var  var_stg::ucase()                             && {ucaser();     return move();}
	ND var  var_stg::lcase()                             && {lcaser();     return move();}
	ND var  var_stg::tcase()                             && {tcaser();     return move();}
	ND var  var_stg::fcase()                             && {fcaser();     return move();}
	ND var  var_stg::normalize()                         && {normalizer(); return move();}
	ND var  var_stg::invert()                            && {inverter();   return move();}

	ND var  var_stg::lower()                             && {lowerer();    return move();}
	ND var  var_stg::raise()                             && {raiser();     return move();}
	ND var  var_stg::crop()                              && {cropper();    return move();}

	ND var  var_stg::quote()                             && {quoter();     return move();}
	ND var  var_stg::squote()                            && {squoter();    return move();}
	ND var  var_stg::unquote()                           && {unquoter();   return move();}

	ND var  var_stg::trim(     SV trimchars /*= " "*/)       && {trimmer(trimchars);      return move();}
	ND var  var_stg::trimfirst(SV trimchars /*= " "*/)       && {trimmerfirst(trimchars); return move();}
	ND var  var_stg::trimlast( SV trimchars /*= " "*/)       && {trimmerlast(trimchars);  return move();}
	ND var  var_stg::trimboth( SV trimchars /*= " "*/)       && {trimmerboth(trimchars);  return move();}

	ND var  var_stg::first()                             && {firster();               return move();}
	ND var  var_stg::last()                              && {laster();                return move();}
	ND var  var_stg::first(const std::size_t length)     && {firster(length);         return move();}
	ND var  var_stg::last( const std::size_t length)     && {laster(length);          return move();}
	ND var  var_stg::cut(  const int length)             && {cutter(length);          return move();}
	ND var  var_stg::paste(const int pos1, const int length, SV replacestr)
                                                && {paster(pos1, length, replacestr);    return move();}
	ND var  var_stg::paste(const int pos1, SV insertstr) && {paster(pos1, insertstr);             return move();}
	ND var  var_stg::prefix(               SV prefixstr) && {prefixer(prefixstr);                 return move();}
	ND var  var_stg::pop()                               && {popper();                            return move();}

//	ND var  var_stg::append(SV appendstr)                && {appender(appendstr);                 return move();}
//	template <typename... ARGS>
//	ND var  var_stg::append(const ARGS&... appendable) && {
//				this->createString();
//				(var_str += ... += appendable);
	//

//    // Helper to append one argument, handling var differently
//    template<typename T>
//    void append_one(const T& value) {
//        if constexpr (std::is_same_v<std::decay_t<T>, var>) {
//            var_str += static_cast<const std::string&>(value);  // Explicit cast to string&
//        } else {
//            var_str += value;  // Other types (string, int, etc.)
//        }
//    }
//	ND var  var_stg::append(const auto&... appendable) && {
////				((*this) ^= ... ^= appendable);
//				this->createString();
//				//(var_str += ... += appendable);
//				(append_one(appendable), ...);  // Comma fold calls append_one per arg
//				return move();
//			}

	ND var  var_stg::fieldstore(SV delimiter, const int fieldno, const int nfields, in replacement)
                                                      && {fieldstorer(delimiter, fieldno, nfields, replacement);
                                                                                             return move();}

	ND var  var_stg::substr(const int pos1, const int length)  && {substrer(pos1, length);              return move();}
	ND var  var_stg::substr(const int pos1)                    && {substrer(pos1);                      return move();}

	ND var  var_stg::convert(    SV fromchars, SV tochars)     && {this->converter(fromchars, tochars); return move();}
	ND var  var_stg::textconvert(SV fromchars, SV tochars)     && {textconverter(fromchars, tochars);   return move();}
	ND var  var_stg::replace(    SV fromstr,   SV tostr)       && {replacer(fromstr, tostr);            return move();}
	ND var  var_stg::replace(const rex& regex, SV replacement) && {replacer(regex, replacement);        return move();}
//	ND var  var_stg::replace(const rex& regex, ReplacementFunction auto repl_func)
//	                                                           && {replacer(regex,repl_func);    return move();}

	ND var  var_stg::unique()                                  && {uniquer();           return move();}
	ND var  var_stg::sort(   SV delimiter /*= _FM*/)           && {sorter(delimiter);   return move();}
	ND var  var_stg::reverse(SV delimiter /*= _FM*/)           && {reverser(delimiter); return move();}
	ND var  var_stg::randomize(SV delimiter /*= _FM*/)         && {randomizer(delimiter); return move();}
	ND var  var_stg::parse(char delimiter /*= ' '*/)           && {parser(delimiter);   return move();}

	   IO   var_stg::substrer(const int pos1) REF {return this->substrer(pos1, this->len());}
//     IO   var_stg::replacer(const rex& regex, ReplacementFunction auto repl_func) REF {*this = replace(regex, repl_func);}



	ND var  var_stg::extract(const int fieldno, const int valueno /*= 0*/, const int subvalueno /*= 0*/)      const {return this->f(fieldno, valueno, subvalueno);}

	// SAME AS ABOVE ON TEMPORARIES TO USE MUTATING (not documented because used difference in implementation is irrelevant to exodus users)
	///////////////////////////////////////////////

	ND var  var_stg::update(const int fieldno, const int valueno, const int subvalueno, in replacement) && {this->updater(fieldno, valueno, subvalueno, replacement); return move();}
	ND var  var_stg::update(const int fieldno, const int valueno, in replacement)                       && {this->updater(fieldno, valueno, 0, replacement); return move();}
	ND var  var_stg::update(const int fieldno, in replacement)                                          && {this->updater(fieldno, 0, 0, replacement); return move();}

	ND var  var_stg::insert(const int fieldno, const int valueno, const int subvalueno, in insertion)   && {this->inserter(fieldno, valueno, subvalueno, insertion); return move();}
	ND var  var_stg::insert(const int fieldno, const int valueno, in insertion)                         && {this->inserter(fieldno, valueno, 0, insertion); return move();}
	ND var  var_stg::insert(const int fieldno, in insertion)                                            && {this->inserter(fieldno, 0, 0, insertion); return move();}

	ND var  var_stg::remove(const int fieldno, const int valueno /*= 0*/, const int subvalueno /*= 0*/)         && {this->remover(fieldno, valueno, subvalueno); return move();}


}  // namespace exo
