#ifndef VAR_STR_H
#define VAR_STR_H

// gendoc: var - String functions

#if EXO_MODULE > 1
	import std;
#else
#	include <array>
#endif

#include <exodus/var_iter.h>

namespace exo {

	class rex;
	class var;
	class dim;

	// Concept to constrain replace()'s callback function
	template <typename F>
	concept ReplacementFunction  = requires(F f, const var& v) {
		{ f(v) } -> std::same_as<var>; // Must take const var& and return var
	};

class PUBLIC var_stg : public var_base {

public:

	// Inherit constructors
	using var_base::var_base;

	// Inherit assignment operators
	using var_base::operator=;

	// Inherit conversion to var;
	using var_base::operator var;

//	using VAR    =       var;
//	using VARREF =       var&;
//	using CVR    = const var&;
//	using TVR    =       var&&;

	using in     = const var_stg&;
	using out    =       var_stg&;
	using io     =       var_stg&;

	// Implicitly convert var_stg to var

//	// Type conversion since reference will be optimised away by -O2
//	// Note that most var_base function directly return a var since var's have full functionality
//	// and can be implicitly used as var_base but not vice versa.
//	CONSTEXPR
//	operator var&() &;

	///////////////////////
	///// String creation :
	///////////////////////

	/* fake for gendoc

	// String concatenation operator ^
	// At least one side must be a var.
	// "aa" ^ "22" will not compile but "aa" "22" will.
	// Floating point numbers are implicitly converted to strings with no more than 12 significant digits of precision. This practically eliminates all floatng point rounding errors.
	// `var v2 = "aa";
	//  var v1 = v2 ^ 22; // "aa22"`
	//
	ND var operator^(var);

	// String self concatention ^= (append)
	// `var v1 = "aa";
	//  v1 ^= 22; // v1 -> "aa22"`
	ND var operator^=(var);

	*/

	// obj is var()

	// Get a char.
	// chrnum: An integer 0-255.
	// return: A string containing a single char
	// 0-127 -> ASCII, 128-255 -> invalid UTF-8 which cannot be written to the database or used in many exodus string operations
	//
	// `let v1 = var::chr(0x61); // "a"
	//  // or
	//  let v2 = chr(0x61);`
	//
	ND static var  chr(const int chrnum);

	// Get a Unicode character.
	// codepoint: A Unicode Code Point (Number)
	// return: A single Unicode character in UTF8 encoding.
	//
	// `let v1 = var::textchr(171416); // "𩶘" // or "\xF0A9B698"
	//  // or
	//  let v2 = textchr(171416);`
	//
	ND static var  textchr(const int codepoint);

	// Get a Unicode character name
	// unicode_code_point: 0 - 0x10FFFF.
	// return: Text of the name or "" if not a valid Unicode Code Point
	//
	// `let v1 = var::textchrname(91); // "LEFT SQUARE BRACKET"
	//  // or
	//  let v2 = textchrname(91);`
	//
	ND static var  textchrname(const int unicode_code_point);

	// Get a string of repeated substrings.
	// var: The substring to be repeated
	// nreps: How many times to repeat the substring
	// return: A string
	// obj is strvar
	//
	// `let v1 = "ab"_var.str(3); // "ababab"
	//  // or
	//  let v2 = str("ab", 3);`
	//
	ND var  str(const int nreps) const;

	// Get a string as a given number of spaces.
	// nspaces: The number of spaces required.
	// return: A string of space chars.
	// obj is var()
	//
	// `let v1 = var::space(3); // "␣␣␣"
	//  // or
	//  let v2 = space(3);`
	//
	ND static var  space(const int nspaces);

	// Get a number written out in words insteads of digits.
	// return: A string.
	// locale: e.g. en_GB, ar_AE, el_CY, es_US, fr_FR etc or a language name e.g. "french".
	// obj is varnum
	//
	// `let softhyphen = "\xc2\xad";
	//  let v1 = var(123.45).numberinwords("de_DE").replace(softhyphen, " "); // "ein␣hundert␣drei␣und␣zwanzig␣Komma␣vier␣fünf"`
	//
	ND var  numberinwords(in locale = "");

	////////////
	// STRING // All UTF8 unless char or byte mentioned
	////////////

	///// STRING SCANNING:
	//////////////////////

	// obj is strvar

	/* fake for gendoc from var_base in varb.h

	// Get a single char from a string.
	// pos1: First char is 1. Last char is -1.
	// return: A single char if pos1 +/- the length of the string, or "" if greater. Returns the first char if pos1 is 0 or (-pos1) > length.
	// `var v1 = "abc";
	//  var v2 = v1.at(2);  // "b"
	//  var v3 = v1.at(-3); // "a"
	//  var v4 = v1.at(4);  // ""`
    ND var  at(const int pos1) const;
	*/

	// Get the char number of a char.
	// return: A number between 0 and 255.
	// If given a string, then only the first char is considered.
	// Equivalent to ord() in php
	//
	// `let v1 = "abc"_var.ord(); // 0x61 // decimal 97, 'a'
	//  // or
	//  let v2 = ord("abc");`
	//
	ND var  ord() const;

	// Get the Unicode Code Point of a Unicode character.
	// var: A UTF-8 string. Only the first Unicode character is considered.
	// return: A number 0 to 0x10FFFF.
	// Equivalent to ord() in python and ruby, mb_ord() php.
	//
	// `let v1 = "Γ"_var.textord(); // 915 // U+0393: Greek Capital Letter Gamma (Unicode character)
	//  // or
	//  let v2 = textord("Γ");`
	//
	ND var  textord() const;


	// Count the number of output columns required for a given source string.
	// return: A number
	// Allow wide multi-column Unicode characters that occupy more than one space in a text file or terminal screen.
	// Reduce combining characters to a single column. e.g. "e" followed by grave accent is multiple bytes but only occupies one output column.
	// Does not properly calculate all possible combining sequences of graphemes e.g. face followed by colour
	//
	// `let v1 = "🤡x🤡"_var.textwidth(); // 5
	//  // or
	//  let v2 = textwidth("🤡x🤡");`
	//
	ND var  textwidth() const;

	// Count the number of Unicode code points in a source string.
	// return: A number.
	//
	// `let v1 = "Γιάννης"_var.textlen(); // 7
	//  // or
	//  let v2 = textlen("Γιάννης");`
	//
	ND var  textlen() const;

	// Count the number of fields in a source string.
	// sepstr: The separator character or substr that delimits individual fields.
	// return: The count of the number of fields
	// This is similar to "var.count(sepstr) + 1" but it returns 0 for an empty source string.
	//
	// `let v1 = "aa**cc"_var.fcount("*"); // 3
	//  // or
	//  let v2 = fcount("aa**cc", "*");`
	//
	ND var  fcount(SV sepstr) const;

	// Count the number of occurrences of a given substr in a source string.
	// substr: The substr to count.
	// return: The count of the number of sepstr found.
	// Overlapping substrings are not counted.
	//
	// `let v1 = "aa**cc"_var.count("*"); // 2
	//  // or
	//  let v2 = count("aa**cc", "*");`
	//
	ND var  count(SV sepstr) const;

	// Exodus   Javascript   PHP             Python       Go          Rust          C++
	// starts   startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	// ends     endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	// contains includes()   str_contains    contains()   Contains()  contains()    contains

	// Check if a var starts with a given prefix.
	// prefix: The substr to check for. Multiple substr can be provided.
	// return: True if the source string starts with one of the given prefixes.
 	// Always returns false if prefix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.starts("ab", "ba")) ... true
	//  // or
	//  if (starts("abc", "ab", "ba")) ... true`
	//
	template<typename... T>
	bool starts(T&&... prefix) const {assertString(__PRETTY_FUNCTION__); return (... || no_check_starts(static_cast<SV>(std::forward<T>(prefix))));}
//		static_assert((std::is_convertible_v<T, std::string_view> && ...),
//			"All prefix arguments must be convertible to std::string_view");

	// undocumented
	ND bool starts(SV prefix) const;

	// Check if a var ends with a given suffix.
	// suffix: The substr to check for. Multiple suffixes can be provided.
	// return: True if the source string ends with one of the given suffixes.
 	// Always returns false if suffix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.ends("bc", "cb")) ... true
	//  // or
	//  if (ends("abc", "bc", "cb")) ... true`
	//
	template<typename... T>
	bool ends(T&&... suffix) const {assertString(__PRETTY_FUNCTION__); return (... || no_check_ends(static_cast<SV>(std::forward<T>(suffix))));}

	// undocumented
	ND bool ends(SV suffix) const;

	// Check if a var contains a given substr.
	// substr: The substr to check for. Multiple substr can be provided.
	// return: True if the var starts with, ends with or contains one of the given substrs.
	// Always returns false if substr is "". DIFFERS from c++, javascript, python3. See contains() for more info.
    // Human logic: "" is not equal to "x" therefore x does not contain "".
    // Human logic: Check each item (character) in the list for equality with what I am looking for and return success if any are equal.
    // Programmer logic: Compare as many characters as are in the search string for presence in the list of characters and return success if there are no failures.
    //
	// `if ("abcd"_var.contains("bc", "cb")) ... true
	//  // or
	//  if (contains("abcd", "bc", "cb")) ... true`
	//
	template<typename... T>
	bool contains(T&&... substr) const {assertString(__PRETTY_FUNCTION__); return (... || no_check_contains(static_cast<SV>(std::forward<T>(substr))));}

	// undocumented
	ND bool contains(SV substr) const;

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find

	// Find a substr in a source string.
	// substr: The substr to search for.
	// startchar1: The char position (1 based) to start the search at. The default is 1, the first char.
	// return: The char position (1 based) that the substr is found at or 0 if not present.
	//
	// `let v1 = "abcd"_var.index("bc"); // 2
	//  // or
	//  let v2 = index("abcd", "bc");`
	//
	ND var  index(SV substr, const int startchar1 = 1) const;

	// Find the nth occurrence of a substr in a source string.
	// substr: The string to search for.
	// return: char position (1 based) or 0 if not present.
	//
	// `let v1 = "abcabc"_var.indexn("bc", 2); // 5
	//  // or
	//  let v2 = indexn("abcabc", "bc", 2);`
	//
	ND var  indexn(SV substr, const int occurrence) const;

	// Find the position of substr working backwards
	// Start at the end and work backwards.
	// substr: The string to search for.
	// return: The char position of the substr if found, or 0 if not.
	// startchar1: Defaults to -1 meaning start searching from the last char. Positive start1char1 counts from the beginning of the source string and negative startchar1 counts backwards from the last char.
	//
	// `let v1 = "abcabc"_var.indexr("bc"); // 5
	//  // or
	//  let v2 = indexr("abcabc", "bc");`
	//
	ND var  indexr(SV substr, const int startchar1 = -1) const;

	// Check if something is in a list.
	// The list must be comma separated without spaces.
	//
	// `let v1 = "def";
	//  if (v1.listed("abc,def")) ... ok
	//  // or
	//  if (listed(v1, "abc,def")) ... ok`
	//
	ND bool listed(SV list) const;

	// Check if something is in a list and return its position.
	//
	// `let v1 = "def";
	//  var posn;
	//  if (v1.listed("abc,def", posn)) ... ok // posn -> 2
	//  // or
	//  if (listed(v1, "abc,def", posn)) ... ok`
	//
	ND bool listed(SV list, out position) const;

	// Find all matches of a given regular expression.
	// return: Zero or more matching substrings separated by FMs. Any groups are in VMs.
	//
	// `let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"_var
	//  // or
	//  let v2 = match("abc1abc2", "BC(\\d)", "i");`
	//
	// regex_options:
    // *  l  * Literal (any regex chars are treated as normal chars)
    // *  i  * Case insensitive
    // *  p  * ECMAScript/Perl (the default)
    // *  b  * Basic POSIX (same as sed)
    // *  e  * Extended POSIX
    // *  a  * awk
    // *  g  * grep
    // *  eg * egrep or grep -E
	//
    // char ranges like a-z are locale sensitive if ECMAScript
	//
	// regex_options:
    // *  m * Multiline. Default in boost (and therefore exodus)
    // *  s * Single line. Default in std::regex
	// *  f * First only. Only for replace() (not match() or search())
    // *  w * Wildcard glob style (e.g. *.cfg) not regex style. Only for match() and search(). Not replace().
	//
	ND var  match(SV regex_str, SV regex_options = "") const;

	// Ditto
	ND var  match(const rex& regex) const;

	// Search for the first match of a regular expression.
	// startchar1: [in] char position to start the search from
	// startchar1[out]: char position to start the next search from or 0 if no more matches.
	// return: The 1st match like match()
	// regex_options as for match()
	//
	// `var startchar1 = 1;
	//  let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // "bc1]1"_var // startchar1 -> 5 /// Ready for the next search
	//  // or
	//  startchar1 = 1;
	//  let v2 = search("abc1abc2", "BC(\\d)", startchar1, "i");`
	//
	ND var  search(SV regex_str, io startchar1, SV regex_options = "") const;

	// Ditto starting from first char
	ND var  search(SV regex_str) const;

	// Ditto given a rex
	ND var  search(const rex& regex, io startchar1) const;

	// Ditto starting from first char.
	ND var  search(const rex& regex) const;

	// Get a hash of a source string.
	// modulus: The result is limited to [0, modulus)
	// return: A 64 bit signed integer.
	// MurmurHash3 is used.
	//
	// `let v1 = "abc"_var.hash(); assert(v1 == var(6'715'211'243'465'481'821));
	//  // or
	//  let v2 = hash("abc");`
	//
	ND var  hash(const std::uint64_t modulus = 0) const;

	///// STRING CONVERSION - Non-mutating - Chainable:
	//////////////////////////////////////////////////

	// obj is strvar

	// Convert to upper case
	//
	// `let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
	//  // or
	//  let v2 = ucase("Γιάννης");`
	//
	ND var  ucase() const&;

	// Convert to lower case
	//
	// `let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
	//  // or
	//  let v2 = lcase("ΓΙΆΝΝΗΣ");`
	//
	ND var  lcase() const&;

	// Convert to title case.
	// return: Original source string with the first letter of each word is capitalised.
	//
	// `let v1 = "γιάννης παππάς"_var.tcase(); // "Γιάννης Παππάς"
	//  // or
	//  let v2 = tcase("γιάννης παππάς");`
	//
	ND var  tcase() const&;

	// Convert to folded case.
	// Case folding is the process of converting text to a case independent representation.
	// return: The source string standardised in a way to enable consistent indexing and searching,
	// https://www.w3.org/International/wiki/Case_folding
	// Accents can be significant. As in French cote, coté, côte and côté.
	// Case folding is not locale-dependent.
	//
	// `let v1 = "Grüßen"_var.fcase(); // "grüssen"
	//  // or
	//  let v2 = tcase("Grüßen");`
	//
	ND var  fcase() const&;

	// Replace Unicode character sequences with their standardised NFC form.
	// Unicode normalization is the process of converting Unicode strings to a standard form, making them binary comparable and suitable for text processing and comparison. It is an important part of Unicode text processing.
	// For example, Unicode character "é" can be represented by either a single Unicode character, which is Unicode Code Point (\u00E9" - Latin Small Letter E with Acute), or a combination of two Unicode code points i.e. the ASCII letter "e" and a combining acute accent (Unicode Code Point "\u0301"). Unicode NFC definition converts the pair of code points to the single code point.
	// Normalization is not locale-dependent.
	//
	// `let v1 = "cafe\u0301"_var.normalize(); // "caf\u00E9" // "café"
	//  // or
	//  let v2 = normalize("cafe\u0301");`
	//
	ND var  normalize() const&;

	// Simple reversible disguising of string text.
	// It works by treating the string as UTF8 encoded Unicode code points and inverting the first 8 bits of their Unicode Code Points.
	// return: A string.
	// invert(invert()) returns to the original text.
	// ASCII bytes become multibyte UTF-8 so string sizes increase.
	// Inverted characters remain on their original Unicode Code Page but are jumbled up.
	// Non-existant Unicode Code Points may be created but UTF8 encoding remains valid.
	//
	// `let v1 = "abc"_var.invert(); // "\xC2" "\x9E" "\xC2" "\x9D" "\xC2" "\x9C"
	//  // or
	//  let v2 = invert("abc");`
	//
	ND var  invert() const&;

	// Reduce all types of field mark chars by one level.
	// Convert all FM to VM, VM to SM etc.
	// return: The converted string.
	// Note that subtext STM chars are not converted because they are already the lowest level.
	// String size remains identical.
	//
	// `let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"_var
	//  // or
	//  let v2 = lower("a1^b2^c3"_var);`
	//
	ND var  lower() const&;

	// Increase all types of field mark chars by one level.
	// Convert all VM to FM, SM to VM etc.
	// return: The converted string.
	// The record mark char RM is not converted because it is already the highest level.
	// String size remains identical.
	//
	// `let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = "a1]b2]c3"_var;`
	//
	ND var  raise() const&;

	// Remove any redundant FM, VM etc. chars (Trailing FM; VM before FM etc.)
	//
	// `let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = crop("a1^b2]]^c3^^"_var);`
	//
	ND var  crop() const&;

	// Wrap in double quotes.
	//
	// `let v1 = "abc"_var.quote(); // "\"abc\""
	//  // or
	//  let v2 = quote("abc");`
	//
	ND var  quote() const&;

	// Wrap in single quotes.
	//
	// `let v1 = "abc"_var.squote(); // "'abc'"
	//  // or
	//  let v2 = squote("abc");`
	//
	ND var  squote() const&;

	// Remove one pair of surrounding double or single quotes.
	//
	// `let v1 = "'abc'"_var.unquote(); // "abc"
	//  // or
	//  let v2 = unquote("'abc'");`
	//
	ND var  unquote() const&;

	// Remove all leading, trailing and excessive inner bytes.
	// trimchars: The chars (bytes) to remove. The default is space.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
	//  // or
	//  let v2 = trim("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trim(SV trimchars = " ") const&;

	// Ditto but only leading.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
	//  // or
	//  let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimfirst(SV trimchars = " ") const&;

	// Ditto but only trailing.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimlast(SV trimchars = " ") const&;

	// Ditto but only leading and trailing, not inner.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimboth(SV trimchars = " ") const&;

	// Get the first char of a string.
	// return: A char, or "" if empty.
	// Equivalent to var.substr(1,length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(); // "a"
	//  // or
	//  let v2 = first("abc");`
	//
	ND var  first() const&;

	// Get the last char of a string.
	// return: A char, or "" if empty.
	// Equivalent to var.substr(-1, 1) or var[-1, 1] in Pick OS
	//
	// `let v1 = "abc"_var.last(); // "c"
	//  // or
	//  let v2 = last("abc");`
	//
	ND var  last() const&;

	// Get the first n chars of a source string.
	// length: The number of chars (bytes) to get.
	// return: A string of up to n chars.
	// Equivalent to var.substr(1, length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(2); // "ab"
	//  // or
	//  let v2 = first("abc", 2);`
	//
	ND var  first(const std::size_t length) const&;

	// Extract up to length trailing chars
	// Equivalent to var.substr(-length, length) or var[-length, length] in Pick OS
	//
	// `let v1 = "abc"_var.last(2); // "bc"
	//  // or
	//  let v2 = last("abc", 2);`
	//
	ND var  last(const std::size_t length) const&;

	// Remove n chars (bytes) from the source string.
	// length: Positive to remove first n chars or negative to remove the last n chars.
	// If the absolute value of length is >= the number of chars in the source string then all chars will be removed.
	// Equivalent to var.substr(length) or var[1, length] = "" in Pick OS
	//
	// `let v1 = "abcd"_var.cut(2); // "cd"
	//  // or
	//  let v2 = cut("abcd", 2);`
	//
	ND var  cut(const int length) const&;

	// Insert a substr at an given position after removing a given number of chars.
	// pos1:
	// * 0 or 1   * Remove length chars from the beginning and insert at the beginning.
	// *     -1   * Insert on or before the last char after removing up to length chars starting with the last char.
	// *     -2   * Ditto on or before the penultimate char.
	// * > strlen * If pos1 > length of the source string, insert after the last char.
	// Equivalent to var[pos1, length] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
	//  // or
	//  let v2 = paste("abcd", 2, 2, "XYZ");`
	//
	ND var  paste(const int pos1, const int length, SV replacestr) const&;

	// Insert text at char position without overwriting any following chars
	// Equivalent to var[pos1, 0] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYZbcd"
	//  // or
	//  let v2 = paste("abcd", 2, "XYZ");`
	//
	ND var  paste(const int pos1, SV insertstr) const&;

	// Insert text at the beginning
	// Equivalent to var[0, 0] = substr in Pick OS
	//
	// `let v1 = "abc"_var.prefix("XYZ"); // "XYZabc"
	//  // or
	//  let v2 = prefix("abc", "XYZ");`
	//
	ND var  prefix(SV insertstr) const&;

//	template <typename... ARGS>
//	ND var  append(const ARGS&... appendable) const& {
//		var nrvo = *this;
//		(nrvo ^= ... ^= appendable);
//		return nrvo;
//	}

	// Append anything at the end of a string.
	//
	// `let v1 = "abc"_var.append(" is ", 10, " ok", '.'); // "abc is 10 ok."
	//  // or
	//  let v2 = append("abc", " is ", 10, " ok", '.');`
    ND var  append(const auto&... appendable) const&;
	//
	// TODO perfect forwarding on argument 1 to create the initial string?

	// Remove one trailing char.
	// Equivalent to var[-1, 1] = "" in Pick OS
	//
	// `let v1 = "abc"_var.pop(); // "ab"
	//  // or
	//  let v2 = pop("abc");`
	//
	ND var  pop() const&;

	// Copy one or more consecutive fields from a string.
	// delimiter: A Unicode character.
	// fieldno: The first field is 1, the last field is -1.
	// return: A substring
	//
	// `let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", 2);`
	//
	// `let v1 = "aa*bb*cc"_var.field("*", -1); // "cc"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", -1);`
	//
	ND var  field(SV delimiter, const int fieldnx = 1, const int nfieldsx = 1) const;

	// field2 is a version that treats fieldn -1 as the last field, -2 the penultimate field etc. -
	// TODO Should probably make field() do this (since -1 is basically an erroneous call) and remove field2
	// Same as var.field() but negative fieldnos work backwards from the last field.
	//
	[[deprecated ("Just use field() which now has the same behaviour as field()")]]
	ND var  field2(SV separator, const int fieldno, const int nfields = 1) const; /*IMPL
	{
		if (fieldno >= 0) LIKELY
			return field(separator, fieldno, nfields);
		return field(separator, this->count(separator) + 1 + fieldno + 1, nfields);
	}IMPL*/

	// fieldstore() replaces, inserts or deletes subfields in a string.
	// fieldno: The field number to replace or, if not 1, the field number to start at. Negative fieldno counts backwards from the last field.
	// nfields: The number of fields to replace or, if negative, the number of fields to delete first. Can be 0 to cause simple insertion of fields.
	// replacement: A string that is the replacement field or fields.
	// return: A modified copy of the original string.
	// There is no way to simply delete n fields because the replacement argument cannot be omitted, however one can achieve the same result by replacing n+1 fields with the n+1th field.
	//
	// The replacement can contain multiple fields itself. If replacing n fields and the replacement contains < n fields then the remaining fields become "". Conversely, if the replacement contains more fields than are required, they are discarded.
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, 3, "11,22"); // "aa,11,22,,ee"
	//  // or
	//  let v2 = fieldstore("aa,bb,cc,dd,ee", ",", 2, 3, "11,22");`
	//
	// If nfields is 0 then insert the replacement field(s) before fieldno
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, 0, "11,22"); // "aa,11,22,bb,cc,dd,ee"`
	//
	// If nfields is negative then delete abs(n) fields before inserting whatever fields the replacement has.
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, -2, "11"); // "aa,11,dd,ee"`
	//
	// If nfields exceeds the number of fields in the input then additional empty fields are added.
	//
	// `let v1 = "aa,bb,cc"_var.fieldstore(",", 6, 2, "11"); // "aa,bb,cc,,,11,"`
	//
	ND var  fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;

	// substr version 1.
	// Copy a substr of length chars from a given a starting char position.
	// return: A substr or "".
	// pos1: The char position to start at. If negative then start from a position counting backwards from the last char
	// length: The number of chars to copy. If negative then copy backwards. This reverses the order of the chars in the returned substr.
	// Equivalent to var[start, length] in Pick OS
	// Not Unicode friendly.
	//
	// `let v1 = "abcd"_var.substr(2, 2); // "bc"
	//  // or
	//  let v2 = substr("abcd", 2, 2);`
	//
	// If pos1 is negative then start counting backwards from the last char
	//
	// `let v1 = "abcd"_var.substr(-3, 2); // "bc"
	//  // or
	//  let v2 = substr("abcd", -3, 2);`
	//
	// If length is negative then work backwards and return chars reversed
	//
	// `let v1 = "abcd"_var.substr(3, -2); // "cb"
	//  // or
	//  let v2 = substr("abcd", 3, -2); // "cb"`
	//
	ND var  substr(const int pos1, const int length) const&;

	// Abbreviated alias of substr version 1.
	ND var  b(const int pos1, const int length) const&;

	// substr version 2.
	// Copy a substr from a given char position up to the end of the source string
	// return: A substr or "".
	// pos1: The char position to start at. If negative then start from a position counting backwards from the last char
	// Equivalent to var[pos1, 9999999] in Pick OS
	// Partially Unicode friendly but pos1 is in chars.
	//
	// `let v1 = "abcd"_var.substr(2); // "bcd"
	//  // or
	//  let v2 = substr("abcd", 2);`
	//
	ND var  substr(const int pos1) const&;

	// Shorthand alias of substr version 2.
	ND var  b(const int pos1) const&;

	// substr version 3.
	// Copy a substr from a given char position up to (but excluding) any one of some given delimiter chars
	// return: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos2[out]: The position of the next delimiter char, or one char position after the end of the source string if no subsequent delimiter chars are found.
	// COL2: Is a predefined variable that can be used for pos2 instead of declaring a variable.
	// An empty string may be returned if pos1 [in] points to one of the delimiter chars or points beyond the end of the source string.
	// Equivalent to var[pos1, ",."] in Pick OS (non-numeric length).
	// Works with any encoding including UTF8 for the source string but the delimiter chars are bytes.
	// Add 1 to pos2 to skip over the next delimiter char to copy the next substr
	// Works with any encoding including UTF8 for the source string but the delimiter chars are bytes.
	// This function is similar to std::string::find_first_of but that function only returns pos2.
	//
	// `var pos1 = 4;
	//  let v1 = "12,45 78"_var.substr(pos1, ", ", COL2);  // v1 -> "45" // COL2 -> 6 // 6 is the position of the next delimiter char found.
	//  // or
	//  let v2 = substr("12,45 78", COL2 + 1, ", ", COL2); // v2 -> "78" // COL2 -> 9 // 9 is one after the end of the string meaning that none of the delimiter chars were found.`
	//
	   var  substr(const int pos1, SV delimiterchars, out pos2) const;

	// Shorthand alias of substr version 3.
	   var  b(const int pos1, SV delimiterchars, out pos2) const;

	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// offset to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(io pos1, io
	// delimiterno) const;

	// substr version 4.
	// Copy a substr from a given char position up to (but excluding) the next field mark char (RM, FM, VM, SM, TM, STM).
	// return: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos1[out]: The position of the first char of the next substr after whatever field mark char is found, or one char position after the end of the source string if no subsequent field mark char is found.
	// field_mark_no[out]: A number (1-6) indicating which of the standard field mark chars was found, or 0 if not.
	// An empty string may be returned if the pos1 [in] points to one of the field marks or beyond the end of the source string.
	// pos1 [out] is correctly positioned to copy the next substr.
	// Works with any encoding including UTF8. Was called "remove" in Pick OS.
	// The equivalent in Pick OS was the statement "Remove variable From string At column Setting flag"
	// This function is valuable for high performance processing of dynamic arrays.
	// It is notably used in "list" to print parallel columns of mixed combinations of multivalues/subvalues and text marks correctly lined up mv to mv, sv to sv, tm to tm even when particular values, subvalues and text fragments are missing from particular columns.
	// It is similar to version 3 of substr - substr(pos1, delimiterchars, pos2) except that in this version the delimiter chars are hard coded as the standard field mark chars (RM, FM, VM, SM, TM, STM) and it returns the first char position of the next substr, not the char position of the next field mark char.
	//
	// `var pos1 = 4, field_mark_no;
	//  let v1 = "12^45^78"_var.substr2(pos1, field_mark_no);  // "45" // pos1 -> 7 // field_mark_no -> 2 // field_mark_no 2 means that a FM was found.
	//  // or
	//  let v2 = substr2("12^45^78"_var, pos1, field_mark_no); // "78" // pos1 -> 9 // field_mark_no -> 0 // field_mark_no 0 means that none of the standard field marks were found.`
	//
	   var  substr2(io pos1, out delimiterno) const;

	// Shorthand alias of substr version 4.
	   var  b2(io pos1, out field_mark_no) const;

	// Convert or delete chars one for one to other chars
	// from_chars: chars to convert. If longer than to_chars then delete those characters instead of converting them.
	// to_chars: chars to convert to
	// Not UTF8 compatible.
	//
	// `let v1 = "abcde"_var.convert("aZd", "XY"); // "Xbce" // a is replaced and d is removed
	//  // or
	//  let v2 = convert("abcde", "aZd", "XY");`
	//
	ND var  convert(SV fromchars, SV tochars) const&;

	// Ditto for Unicode code points.
	//
	// `let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
	//  // or
	//  let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");`
	//
	ND var  textconvert(SV fromchars, SV tochars) const&;

	// Replace all occurrences of one substr with another.
	// Case sensitive.
	//
	// `let v1 = "Abc.Abc"_var.replace("bc", "X"); // "AX.AX"
	//  // or
	//  let v2 = replace("Abc Abc", "bc", "X");`
	//
	ND var  replace(SV fromstr, SV tostr) const&;

	// Replace substrings using a regular expression.
	// regex: A regular expression created by rex() or _rex.
	// replacement_str: A literal to replace all matched substrings.
	// The replacement string can include the following special replacement patterns:
	// Pattern
	// * $$ *    Inserts a "$".
	// * $& *    Inserts the matched substring. Equivalent to $0.
	// * ${backtick} *    Inserts the portion of the string that precedes the matched substring.
	// * $' *    Inserts the portion of the string that follows the matched substring.
	// * $n *    Inserts the nth (1-indexed) capturing group where n is a positive integer less than 100.
	//
	// `let v1 = "A a B b"_var.replace("[A-Z]"_rex, "'$0'"); // "'A' a 'B' b"
	//  // or
	//  let v2 = replace("A a B b", "[A-Z]"_rex, "'$0'");`
	//
	ND var  replace(const rex& regex, SV replacement_str) const&;

	// old non-templated version that handled functions and non-capturing lambdas without std::function
	// Replaced by templated version that can accept capturing lambdas as well
	//	var  replace(const rex& regex, SomeFunction(in match_str)) const;

	// Implementation can handle any type of function including capturing lambdas.
	// [[undocumented]]
	var replace(const rex& rex1, var (*func)(const var&, void*), void* context) const;

	// Template wrapper to accept any callable (including capturing lambdas)

	// Replace substrings using a regular expression and a custom function.
	// Allow complex string conversions.
	// repl_func: A function with arguments (in match_str) that returns a var to replace match_str. May be an inline anonymous lambda function (capturing or non-capturing).
	// e.g. [](auto match_str) {return match_str;} // Does nothing.
	// match_str: Text of a single match. If regex groups are used, match_str.f(1, 1) is the whole match, match_str.f(1, 2) is the first group, etc.
	//
	// `// Decode hex escape codes.
	//  let v1 = R"(--\0x3B--\0x2F--)";                                 // Hex escape codes.
	//  let v2 = v1.replace(
	//      R"(\\0x[0-9a-fA-F]{2,2})"_rex,                              // Find \0xFF.
	//      [](auto match_str) {return match_str.cut(3).iconv("HEX");}  // Decode to a char.
	//  );
	//  assert(v2 == "--;--/--");`
	//
	// `// Reformat dates using groups.
	//  let v3 = "Date: 03-15-2025";
	//  let v4 = v3.replace(
	//      R"((\d{2})-(\d{2})-(\d{4}))"_rex,
	//      [](auto match_str) {return match_str.f(1, 4) ^ "-" ^ match_str.f(1, 2) ^ "-" ^ match_str.f(1, 3);}
	//  );
	//  assert(v4 == "Date: 2025-03-15");`
	//
	ND var           replace(const rex& regex, ReplacementFunction auto repl_func) const &; /*IMPL{

		// Lambda to bridge the callable to a function pointer + context
		struct Context {decltype(repl_func)* lambda;};
		Context ctx{&repl_func};

		// Non-capturing lambda to adapt the callable to the function pointer signature. [[Undocumented]]
		auto bridge = [](const var& match_str, void* ctx) -> var {
			auto* context = static_cast<Context*>(ctx);
			return (*context->lambda)(match_str); // Invoke the original lambda
		};

		return replace(regex, bridge, &ctx);
	}IMPL*/

	// Remove duplicate fields, values or subvalues.
	// From a dynamic array.
	//
	// `let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"_var
	//  // or
	//  let v2 = unique("a1^b2^a1^c2"_var);`
	//
	ND var  unique() const&;

	// Reorder fields, values or subvalues.
	// In a dynamic array.
	// Numeric data:
	// `let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"_var
	//  // or
	//  let v2 = sort("20^10^2^1^1.1"_var);`
	// Alphabetic data:
	//  `let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"_var
	//  // or
	//  let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);`
	//
	ND var  sort(SV delimiter = _FM) const&;

	// Reorder fields in an FM or VM etc. separated list in descending order
	//
	// `let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"_var
	//  // or
	//  let v2 = reverse("20^10^2^1^1.1"_var);`
	//
	ND var  reverse(SV delimiter = _FM) const&;

	// Randomise the order of fields in an FM, VM separated list
	//
	// `let v1 = "20^10^2^1^1.1"_var.randomize(); /// e.g. "2^1^20^1.1^10" (random order depending on initrand())
	//  // or
	//  let v2 = randomize("20^10^2^1^1.1"_var);`
	//
	ND var  randomize(SV delimiter = _FM) const&;

	// Split a delimited string into a dynamic array.
	// Replace separator chars with FM chars except inside double or single quotes and ignoring escaped quotes \" \'
	// return: A dynamic array
	// Can be used to process CSV data.
	//
	// `let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^\"def,\"123\" fgh\"^12.34"_var
	//  // or
	//  let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');`
	//
	ND var  parse(char sepchar = ' ') const&;

	// Split a delimited string into a dim array.
	// Delimiter: Can be multibyte Unicode.
	// return: A dim array.
	//
	// `dim d1 = "a^b^c"_var.split(); // A dimensioned array with three elements (vars)
	//  // or
	//  dim d2 = split("a^b^c"_var);`
	//
	ND dim  split(SV delimiter = _FM) const;

	// Unpack a delimited string into N new vars.
	// Useful when marshalling multiple vars as a single string across interfaces.
	// Note the unusual syntax requiring literal angle brackets.
	// [v1, v2, ...]: New vars to be constructed.
	// N: Must agree with the number of vars being created or returned.
	// delim: Defaults to FM but can be multichar/unicode string. e.g. ",", VM, "| |", or "⭕".
	// return: A C++ object enabling inline creation of vars.
	// Prefix with const to make all the new vars const; partial const declaration is not supported.
	//
	// `let a = "aa", b = "bb";
	//  let pack1 = {a, b}; // "aa^bb"_var
	//  auto [a2, b2, c2] = pack1.unpack<3>(); // a2 -> "aa" // b2 -> "bb" // c2 -> unassigned
	//  // or
	//  auto [a3, b3, c3] = unpack<3>(pack1);
	//
	//  // Similar to:
	//  auto funcx = []() -> std::array<var, 2> {
	//      let a = "aa", b = "bb";
	//      return std::array{a, b};
	//  //  return unpack("aa^bb"_var);
	//  };
	//  auto [a4, b4] = funcx();`
	//
	template <std::size_t N>
	auto unpack/*<N>*/(SV delim = _FM) const -> std::array<var, N>; /*IMPL{
        THISIS("auto var::unpack<N>(SV delim = _FM) const")
        assertString(function_sig);

		// std::vector<var> vv1 = this->split(delim);
		// Utility somewhere in the forest
		// undocumented
		auto basic_split(in v1, SV delim) -> std::vector<var>;
		// undocumented
		auto vv1 = basic_split(var_str, delim);
		return [&vv1]<std::size_t... Is>(std::index_sequence<Is...>) {
		return std::array<var, N>{
			(Is < vv1.size() ? std::move(vv1[Is]) : std::move(var()))...
		};
		}(std::make_index_sequence<N>{});
	}IMPL*/

	// SAME ON TEMPORARIES - CALL MUTATORS FOR SPEED (not documenting since programmer interface is the same)
	/////////////////////////////////////////

	// UTF8/byte as for accessors

// clang-format off

// Many of the non-mutating functions are forwarded with a clone to the mutating function

// On temporaries the mutator function is called to avoid creating a temporary in many cases

	ND var  ucase()                             &&;
	ND var  lcase()                             &&;
	ND var  tcase()                             &&;
	ND var  fcase()                             &&;
	ND var  normalize()                         &&;
	ND var  invert()                            &&;

	ND var  lower()                             &&;
	ND var  raise()                             &&;
	ND var  crop()                              &&;

	ND var  quote()                             &&;
	ND var  squote()                            &&;
	ND var  unquote()                           &&;

	ND var  trim(     SV trimchars = " ")       &&;
	ND var  trimfirst(SV trimchars = " ")       &&;
	ND var  trimlast( SV trimchars = " ")       &&;
	ND var  trimboth( SV trimchars = " ")       &&;

	ND var  first()                             &&;
	ND var  last()                              &&;
	ND var  first(const std::size_t length)     &&;
	ND var  last( const std::size_t length)     &&;
	ND var  cut(  const int length)             &&;
	ND var  paste(const int pos1, const int length, SV replacestr)
                                                &&;
	ND var  paste(const int pos1, SV insertstr) &&;
	ND var  prefix(               SV prefixstr) &&;
	ND var  pop()                               &&;

//	ND var  append(SV appendstr)                &&;
//	template <typename... ARGS>
//	ND var  append(const ARGS&... appendable) &&; //IMPL{
//				this->createString();
//				(var_str += ... += appendable);
	//
	ND var  append(const auto&... appendable)   &&;

//    // Helper to append one argument, handling var differently
//    template<typename T>
//    void append_one(const T& value) {
//        if constexpr (std::is_same_v<std::decay_t<T>, var>) {
//            var_str += static_cast<const std::string&>(value);  // Explicit cast to string&
//        } else {
//            var_str += value;  // Other types (string, int, etc.)
//        }
//    }
//	ND var  append(const auto&... appendable) &&; //IMPL{
////				((*this) ^= ... ^= appendable);
//				this->createString();
//				//(var_str += ... += appendable);
//				(append_one(appendable), ...);  // Comma fold calls append_one per arg
//				return std::move(*this);
//			}

	ND var  fieldstore(SV delimiter, const int fieldno, const int nfields, in replacement)
                                                      &&;

	ND var  substr(const int pos1, const int length)  &&;
	ND var  substr(const int pos1)                    &&;

	ND var  convert(    SV fromchars, SV tochars)     &&;
	ND var  textconvert(SV fromchars, SV tochars)     &&;
	ND var  replace(    SV fromstr,   SV tostr)       &&;
	ND var  replace(const rex& regex, SV replacement) &&;
	ND var  replace(const rex& regex, ReplacementFunction auto repl_func)
	                                                  &&;

	ND var  unique()                                  &&;
	ND var  sort(   SV delimiter = _FM)               &&;
	ND var  reverse(SV delimiter = _FM)               &&;
	ND var  randomize(SV delimiter = _FM)             &&;
	ND var  parse(char delimiter = ' ')               &&;

	///// STRING MUTATION - Standalone commands:
	////////////////////////////////////////////

	// obj is strvar

	// To upper case
	// All string mutators follow the same pattern as ucaser. See the non-mutating functions for details.
	//
	// `var v1 = "abc";
	//  v1.ucaser(); // "ABC"
	//  // or
	//  ucaser(v1);`
	//
	   IO   ucaser() REF ;

	   IO   lcaser() REF ;
	   IO   tcaser() REF ;
	   IO   fcaser() REF ;
	   IO   normalizer() REF ;
	   IO   inverter() REF ;

	   IO   quoter() REF ;
	   IO   squoter() REF ;
	   IO   unquoter() REF ;

	   IO   lowerer() REF ;
	   IO   raiser() REF ;
	   IO   cropper() REF ;

	   IO   trimmer(SV trimchars = " ") REF ;
	   IO   trimmerfirst(SV trimchars = " ") REF ;
	   IO   trimmerlast(SV trimchars = " ") REF ;
	   IO   trimmerboth(SV trimchars = " ") REF ;

	   IO   firster() REF ;
	   IO   laster() REF ;
	   IO   firster(const std::size_t length) REF ;
	   IO   laster(const std::size_t length) REF ;
	   IO   cutter(const int length) REF ;
	   IO   paster(const int pos1, const int length, SV insertstr) REF ;
	   IO   paster(const int pos1, SV insertstr) REF ;
	   IO   prefixer(SV insertstr) REF ;
			template <typename... ARGS>
	   IO   appender(const ARGS&... appendable) REF {
				((*this) ^= ... ^= appendable);

				//false && (*this);
//				return void;
			}

	   IO   popper() REF ;

	// TODO look at using erase to speed up

	   IO   fieldstorer(SV delimiter, const int fieldno, const int nfields, in replacement) REF ;
	   IO   substrer(const int pos1, const int length) REF ;
	   IO   substrer(const int pos1) REF;
	   IO   converter(SV from_chars, SV to_chars) REF;
	   IO   textconverter(SV from_characters, SV to_characters) REF;
	   IO   replacer(const rex& regex, SV tostr) REF;
	   IO            replacer(const rex& regex, ReplacementFunction auto repl_func) REF;
	   IO   replacer(SV fromstr, SV tostr) REF;
//	   IO   regex_replacer(SV regex, SV replacement, SV regex_options = "") REF ;

	   IO   uniquer() REF ;
	   IO   sorter(SV delimiter = _FM) REF ;
	   IO   reverser(SV delimiter = _FM) REF ;
	   IO   randomizer(SV delimiter = _FM) REF ;
	   IO   parser(char sepchar = ' ') REF ;


	///// DYNAMIC ARRAY FUNCTIONS:
	/////////////////////////////

	// obj is strvar

	// EXTRACT() AND F()

	// Pick OS
	//   xxx=yyy<10>;
	//   xxx=extract(yyy,10);
	// becomes c++
	//   xxx=yyy.f(10);

	// f() is a highly abbreviated alias for the Pick OS field/value/subvalue extract() function.
	// "f()" can be thought of as "field" although the function can extract values and subvalues as well.
	// The convenient Pick OS angle bracket syntax for field extraction (e.g. xxx<20>) is not available in C++.
	// The abbreviated exodus field extraction function (e.g. xxx.f(20)) is provided instead since field access is extremely heavily used in source code.
	//
	// `let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
	//  let v2 = v1.f(2, 2); // "f2v2"`
	//
	ND var  f(const int fieldno, const int valueno = 0, const int subvalueno = 0)            const;

	// Extract a specific field, value or subvalue from a dynamic array.
	//
	// `let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
	//  let v2 = v1.extract(2, 2); // "f2v2"
	//  //
	//  // For brevity the function alias "f()" (standing for "field") is normally used instead of "extract()" as follows:
	//  var v3 = v1.f(2, 2);`
	//
	ND var  extract(const int fieldno, const int valueno = 0, const int subvalueno = 0)      const;

	// UPDATE

	// This function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better. It was called replace() in Pick Basic but we are now using "replace()" to
	// change substrings using regex (similar to the old Pick Basic replace function) its mutator function
	// is .updater()
	// Replace a specific subvalue in a dynamic array. Normally one uses the updater() function to replace in place.

	// Update (replace or insert) a specific subvalue  in a dynamic array.
	// Same as var.updater() function but returns a new string instead of updating a variable in place. Rarely used.
	// "update()" was called "replace()" in Pick OS/Basic.
	ND var  update(const int fieldno, const int valueno, const int subvalueno, in replacement) const&;

	// Update (replace or insert) a specific value in a dynamic array.
	ND var  update(const int fieldno, const int valueno, in replacement)                    const&;

	// Update (replace or insert) a specific field a dynamic array.
	ND var  update(const int fieldno, in replacement)                                       const&;

	// INSERT

	// Insert a subvalue in a dynamic array.
	// Same as var.inserter() function but returns a new string instead of updating a variable in place.
	ND var  insert(const int fieldno, const int valueno, const int subvalueno, in insertion) const&;

	// Insert a value in a dynamic array.
	ND var  insert(const int fieldno, const int valueno, in insertion)                      const&;

	// Insert a field in a dynamic array.
	ND var  insert(const int fieldno, in insertion)                                         const&;

	// REMOVE

	// Remove a field, value or subvalue from a dynamic array.
	// Same as var.remover() function but returns a new string instead of updating a variable in place.
	// "remove()" was called "delete()" in Pick OS/Basic.
	ND var  remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)      const&;

	// SAME AS ABOVE ON TEMPORARIES TO USE MUTATING (not documented because used difference in implementation is irrelevant to exodus users)
	///////////////////////////////////////////////

	ND var   update(const int fieldno, const int valueno, const int subvalueno, in replacement) &&;
	ND var   update(const int fieldno, const int valueno, in replacement)                       &&;
	ND var   update(const int fieldno, in replacement)                                          &&;

	ND var   insert(const int fieldno, const int valueno, const int subvalueno, in insertion)   &&;
	ND var   insert(const int fieldno, const int valueno, in insertion)                         &&;
	ND var   insert(const int fieldno, in insertion)                                            &&;

	ND var   remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)         &&;

	///// DYNAMIC ARRAY FILTERS:
	///////////////////////////

	// obj is strvar

	// Sum up multiple values in a dynamic array.
	// Whatever is the lowest level is summed up into a higher level.
	//
	// `let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"_var
	//  // or
	//  let v2 = sum("1]2]3^4]5]6"_var);`
	//
	ND var  sum() const;

	// Sum up everything in a dynamic array.
	//
	// `let v1 = "1]2]3^4]5]6"_var.sumall(); // 21
	//  // or
	//  let v2 = sumall("1]2]3^4]5]6"_var);`
	//
	ND var  sumall() const;

	// Sum all fields using a given delimiter.
	//
	// `let v1 = "10,20,30"_var.sum(","); // 60
	//  // or
	//  let v2 = sum("10,20,30", ",");`
	//
	ND var  sum(SV delimiter) const;

	// Calculate basic statistics include stddev.
	// return: An FM delimited string containing n, tot, min, max, tot, mean and stddev
	// strvar: A dynamic array containing numbers using any field, value or subvalue mark delimiters.
	//
	// `let v1 = "-11.2^0^11.5^12^13.9^14"_var.stddev(); // "6^40.2^-11.2^14^6.7^9.32344714506"_var
	//  // or
	//  let v2 = stddev("-11.2^0^11.5^12^13.9^14"_var);`
	//
	ND var  stddev() const;

	// Binary ops on parallel multivalues
	//
	// opcode:
	// * + * Add
	// * - * Subtract
	// * ∗ * Multiply
	// * / * Divide
	// * ^ * Concatenate (or :)
	//
	// `let v1 = "10]20]30"_var.mv("+","2]3]4"_var); // "12]23]34"_var`
	//
	ND var  mv(const char* opcode, in var2) const;
	//
	// --- NOTE ABOVE --> ∗ <-- is not a * asterisk. It is Unicode character U+2217 ∗ ASTERISK OPERATOR
	// to enable parsing of *?* correctly

	///// DYNAMIC ARRAY MUTATORS Standalone commands:
	////////////////////////////

	// obj is strvar

	// Mutable versions update lvalue vars and dont return anything so that they cannot be chained. This is to prevent accidental misuse and bugs.
	//
	// Pick Basic
	//   xyz<10> = "abc";
	// becomes in exodus c++
	//   xyz(10) = "abc";
	// or
	//   xyz.updater(10, "abc");
	// or
	//   updater(xyz, 10, "abc");

	// Replace a specific field in a dynamic array
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, "X"); // "f1^X^f3"_var
	//  // or
	//  v1(2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, "X");`
	//
	   IO   updater(const int fieldno, in replacement) REF {this->updater(fieldno, 0, 0, replacement); return THIS;}

	// Replace a specific value in a dynamic array.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, 2, "X"); // "f1^v1]X^f3"_var
	//  // or
	//  v1(2, 2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, 2, "X");`
	//
	   IO   updater(const int fieldno, const int valueno, in replacement) REF {this->updater(fieldno, valueno, 0, replacement); return THIS;}

	// Replace a specific subvalue in a dynamic array.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, 2, 2, "X"); // "f1^v1]v2}X}s3^f3"_var
	//  // or
	//  v1(2, 2, 2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, 2, 2, "X");`
	//
	   IO   updater(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;

	// Insert a specific field in a dynamic array
	// All other fields are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, "X"); // "f1^X^v1]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, "X");`
	//
	   IO   inserter(const int fieldno, in insertion) REF {this->inserter(fieldno, 0, 0, insertion); return THIS;}

	// Insert a specific value in a dynamic array.
	// All other values are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, "X"); // "f1^v1]X]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, 2, "X");`
	//
	   IO   inserter(const int fieldno, const int valueno, in insertion) REF {this->inserter(fieldno, valueno, 0, insertion); return THIS;}

	// Insert a specific subvalue in a dynamic array.
	// All other subvalues are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, 2, "X"); // "f1^v1]v2}X}s2}s3^f3"_var
	//  // or
	//  v1.inserter(2, 2, 2, "X");`
	//
	   IO   inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;

	// Remove a specific field, value, or subvalue from a dynamic array.
	// All other fields, values, or subvalues are moved down.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.remover(2, 2); // "f1^v1^f3"_var
	//  // or
	//  remover(v1, 2, 2);`
	//
	   IO   remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;

	//-er version could be extract and erase in one go
	// IO   extracter(int fieldno, int valueno=0, int subvalueno=0) const;

	///// DYNAMIC ARRAY SEARCH:
	//////////////////////////

	// obj is strvar

	// LOCATE

	// locate the position of a substr in dynamic array.
	// Find a substr in an unordered dynamic array regardless of delimiter.
	// return: The field, value or subvalue number or 0 if not found.
	// Searching for empty fields, values etc. (i.e. "") will work. Locating "" returns:
	//  * "]yy" *    1
	//  * "xx]]zz" * 2
	//  * "xx]yy]" * 3
	//  * "xx" *     0
	//  * "" *       1
	//
	// `let pos1 = "UK]US^UA"_var.locate("US"); // 2
	//  let pos2 = "UK]US^UA"_var.locate("GB"); // 0
	//  // or
	//  let pos3 = locate("US", "UK]US^UA"_var);`
	//
	ND var locate(in substr) const;

	// locate substr in dynamic array returning bool and position.
	// Searches an unordered dynamic array regardless of delimiter.
	// return: True if found or False if not.
	// position[out]: The field, value or subvalue number if found or the max such number + 1 if not. If not found then position [out] is suitable for creating a new value.
	//
	// `var pos;
	//  if (    "UK]US^UA"_var.locate("US", pos)) ... ok // pos -> 2
	//  if (not "UK]US^UA"_var.locate("GB", pos)) ... ok // pos -> 4
	//  // or
	//  if (locate("US", "UK]US^UA"_var, pos)) ... ok`
	//
	ND bool locate(in substr, out position) const;

	// locate substr in a dynamic array or field or value.
	// Check if substr is present in an unordered dynamic array.
	// fieldno: If fieldno is non-zero then search the specified field number otherwise, if fieldno is 0, search the whole value using FM as delimiter.
	// valueno: If provided, search the specified value number for a subvalue.
	// position[out]: If found, the field, value or subvalue number where it was found. If not found, the max field, value or subvalue number + 1.
	// If not found then position[out] is suitable for appending a new field, value or subvalue.
	// return: True if found or False if not.
	//
	// `var pos;
	//  if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", pos, 2, 3)) ... ok // pos -> 4 // Return true`
	//
	ND bool locate(in substr, out position, const int fieldno, const int valueno = 0) const;

	// LOCATE BY

	// locate substr in ordered dynamic array
	// locateby() without fieldno or valueno arguments, searches ordered values separated by VM chars.
	// Data must already be in the correct order for searching to work properly.
	// ordercode:
	// * AL * Ascending  - Left Justified (Alphabetic)
	// * DL * Descending - Left Justified (Alphabetic)
	// * AR * Ascending  - Right Justified (Numeric/Natural)
	// * DR * Descending - Right Justified (Numeric/Natural)
	// return: True if found, otherwise false.
	// valueno[out]: Either the value no found or the correct value no for inserting the substr
	//
	// `var valueno; if (not "aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) ... ok // valueno -> 2 // Return false and valueno = where it could be correctly inserted.`
	//
	ND bool locateby(const char* ordercode, in substr, out valueno) const;

	// locate substr in ordered dynamic array
	// locateby() with fieldno and/or valueno arguments, searches fields if fieldno is 0, or values in a specific fieldno, or subvalues in a specific valueno.
	// For more info, see locateby() without fieldno or valueno arguments.
	//
	// `var pos;
	//  if (not "f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", pos, 3)) ... ok // pos -> 2 // return false and where it could be correctly inserted.`
	//
	ND bool locateby(const char* ordercode, in substr, out pos, const int fieldno, const int valueno = 0) const;

	// LOCATE USING

	// locate substr using any delimiter.
	//
	// `if ("AB,EF,CD"_var.locateusing(",", "EF")) ... ok`
	//
	ND bool locateusing(const char* usingchar, in substr) const;

	// locate substr in dynamic array using any delimiter
	// Search in a specific field, value or subvalue.
	// pos[out]:  If found, the number where found, otherwise the maximum number of delimited fields + 1.
	// return: True if found and False if not.
	// This is similar to the main locate command but the delimiter char can be specified e.g. a comma or TM etc.
	//
	// `var pos;
	//  if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", pos, 3)) ... ok // pos -> 2 // Return true`
	//
	ND bool locateusing(const char* usingchar, in substr, out pos, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// LOCATE BY, USING

	// locatebyusing() supports all the locate features in a single function.
	ND bool locatebyusing(const char* ordercode, const char* usingchar, in substr, out pos, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// Stop gendoc
	/// :

	// Iterate over fields
	///////////////////////

	// friend class to iterate over the fields of a var
	friend class var_iter;

	ND var_iter begin() const {return var_iter(*this);}
	ND var_iter end() const   {return var_iter();}

private:
	ND bool no_check_starts(SV substr) const;
	ND bool no_check_ends(SV substr) const;
	ND bool no_check_contains(SV substr) const;

};

} // namespace exo

#endif // VAR_STR_H
