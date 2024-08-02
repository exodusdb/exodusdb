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

#ifndef LIBEXODUS_FORMAT_H
#define LIBEXODUS_FORMAT_H 1

#ifdef EXODUS_MACROS
#	error exodus/format.h must be included BEFORE other exodus program headers since they define intrusive macros for general exodus programming
#endif

// Including the large fmt library header here so exo::format can precompile strings using fmt::vformat
//
// Sadly that implies that it will be waste time in endless recompilations until it becomes
// a c++ module.
//
// Always use fmt library for now because of various bugs causing inconsistency
// in early versions of stdlibc++ implementation of std::format
// As of 2024
// 1. Width calculation doesn’t use grapheme clusterization. The latter has been implemented in a separate branch but hasn’t been integrated yet.
// 2. Most C++20 chrono types are not supported yet.
//
//#include <version>
//#ifdef __cpp_lib_format
//#if __has_include(<formatx>)
//#	warning has <format>
//#	define EXO_FORMAT 1
//#	include <format>
//	namespace fmt = std;

//#undef EXO_FORMAT // only to avoid warning about redefinition below

//#elif __has_include(<fmt/core.h>)
// Not available in libfmt6 which doesnt compile with exodus
#if __has_include(<fmt/args.h>)
//#	warning Using fmt library instead std::format
#	define EXO_FORMAT 2
#	pragma GCC diagnostic push
#	pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#   pragma clang diagnostic ignored "-Wreserved-id-macro" //18 20.04
#   pragma clang diagnostic ignored "-Wduplicate-enum" //18 20.04
#	pragma GCC diagnostic ignored "-Winline"
#	pragma GCC diagnostic ignored "-Wswitch-default"
#	include <fmt/core.h>
#	include <fmt/format.h> // for fmt::formatter<std::string_view> etc.
#	include <fmt/args.h> // only for fmt::dynamic_format_arg_store which we are not using ATM
//module #	include <variant>
#	pragma GCC diagnostic pop
#	if __GNUC__ >= 11 || __clang_major__ > 1
///root/exodus/fmt/include/fmt/core.h: In member function ‘constexpr auto fmt::v10::formatter<exo::var>::parse(ParseContext&) [with ParseContext =
// fmt::v10::basic_format_parse_context<char>]’:
///root/exodus/fmt/include/fmt/core.h:2712:22: warning: inlining failed in call to ‘constexpr const Char* fmt::v10::formatter<T, Char, typename std::enable_if<(fmt::v10::detail::type_constant<T, Char>::value != fmt::v10::detail::type::custom_type), void>::type>::parse(ParseContext&) [with ParseContext = fmt::v10::basic_format_parse_context<char>; T = fmt::v10::basic_string_view<char>; Char = char]’: --param max-inline-insns-single limit reached [-Winline]
// 2712 |   FMT_CONSTEXPR auto parse(ParseContext& ctx) -> const Char* {
//      |                      ^~~~~
///root/exodus/test/src/../../exodus/libexodus/exodus/exofuncs.h:850:58: note: called from here
//  850 |                 return formatter<std::string_view>::parse(ctx);
//      |                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
#		pragma GCC diagnostic ignored "-Winline"
#	endif
#endif

#endif // LIBEXODUS_FORMAT_H
