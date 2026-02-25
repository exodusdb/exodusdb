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

#if EXO_MODULE > 1
	import std;
#else
#	include <iostream>
#	include <string>
#	include <limits>
#endif

#include <var/var.h>

namespace exo {

////////////////////
// USER LITERAL _var
////////////////////

// "abc^def"_var

ND var operator""_var(const char* cstr, std::size_t size) {

	var rvo = var(cstr, size);

	// Convert _VISIBLE_FMS to _ALL_FMS
	for (char& c : rvo.var_str) {
		switch (c) {
			// Most common first to perhaps aid optimisation
			case VISIBLE_FM_: c = FM_; break;
			case VISIBLE_VM_: c = VM_; break;
			case VISIBLE_SM_: c = SM_; break;
			case VISIBLE_TM_: c = TM_; break;
			case VISIBLE_STM_: c = STM_; break;
			case VISIBLE_RM_: c = RM_; break;
			// All other chars left unconverted
			default:;
		}
	}

	return rvo;

}

// 123456_var
/////////////

ND var operator""_var(unsigned long long int i) {
    return var(i);
}

// 123.456_var
//////////////

ND var operator""_var(long double d) {
    return var(d);
}

// R"(
//    aaa
//        bbb
//    ccc
//    )"_heredoc

//////////////
ND var operator""_heredoc(const char* in_cstr, std::size_t in_len) {

	std::string_view in_sv{in_cstr, in_len};

	// Skip leading \n right away — before loop
	std::size_t pos = in_sv.starts_with('\n') ? 1 : 0;

	var out_var = "";

	// Early exit
	if (pos >= in_sv.size())
		return out_var;

	// Minus at least 1 tab or 4 or 8 spaces per line
	std::string out_str;
	out_str.reserve(in_len);

	std::string_view indent;
	bool got_indent = false;

	while (pos < in_sv.size()) {

		auto next_nl_pos = in_sv.find('\n', pos);
		if (next_nl_pos == std::string_view::npos)
			next_nl_pos = in_sv.size();

		std::string_view line = in_sv.substr(pos, next_nl_pos - pos);

		if (!got_indent && line.size()) {
			got_indent = true;
			auto first = line.find_first_not_of(" \t");
			if (first == std::string_view::npos)
				indent = line;
			else
				indent = line.substr(0, first);
		}

		if (got_indent && line.starts_with(indent))
			out_str.append(line.substr(indent.size()));
		else
			out_str.append(line);

		out_str.push_back('\n');

		pos = next_nl_pos + 1;
	}

	// remove final \n if in_sv didn't end with one
	if (in_sv.back() != '\n')
		out_str.pop_back();

	out_var = std::move(out_str);

	return out_var;
}

// Inheriting from var_base now
//////////////////////////////////////////
//// SELF INCREMENT/DECREMENT var versions
//////////////////////////////////////////
//
//// All forwarded to var_base
//
//// var ++
//
//var  var::operator++(int) & {
//	const var orig = this->clone();
//	var_base::operator++();
//	return orig;
//}
//
//// var --
//
//var  var::operator--(int) & {
//	const var orig = this->clone();
//	var_base::operator--(0);
//	return orig;
//}
//
//// ++ var
//
//var& var::operator++() & {
//	var_base::operator++(0);
//	return *this;
//}
//
//// -- var
//
//var& var::operator--() & {
//	var_base::operator--();
//	return *this;
//}

} // namespace exo
