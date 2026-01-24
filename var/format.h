#ifndef LIBEXODUS_FORMAT_H
#define LIBEXODUS_FORMAT_H

#ifdef EXODUS_MACROS
#	error exodus/format.h must be included BEFORE other exodus program headers since they define intrusive macros for general exodus programming
#endif

// Including the large fmt library header here so exo::format can precompile strings using std::vformat
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

#if EXO_FORMAT == 1

#if EXO_MODULE > 1
	import std;
	namespace fmt = std;
#else
#	include <format>
	namespace fmt = std;
#endif

#elif EXO_FORMAT == 2 || EXO_FORMAT == 3

//#	warning Using fmt library instead std::format

#	pragma GCC diagnostic push
#	pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#   pragma clang diagnostic ignored "-Wreserved-id-macro" //18 20.04
#   pragma clang diagnostic ignored "-Wduplicate-enum" //18 20.04
#	pragma GCC diagnostic ignored "-Winline"
#	pragma GCC diagnostic ignored "-Wswitch-default"

#	include <fmt/core.h>
#	include <fmt/format.h> // for std::formatter<std::string_view> etc.

#	pragma GCC diagnostic pop

#	if __GNUC__ >= 11 || __clang_major__ > 1

// Avoid the following error:
///root/exodus/fmt/include/fmt/core.h: In member function ‘constexpr auto std::v10::formatter<exo::var>::parse(ParseContext&) [with ParseContext =
// std::v10::basic_format_parse_context<char>]’:
///root/exodus/fmt/include/fmt/core.h:2712:22: warning: inlining failed in call to ‘constexpr const Char* std::v10::formatter<T, Char, typename std::enable_if<(std::v10::detail::type_constant<T, Char>::value != std::v10::detail::type::custom_type), void>::type>::parse(ParseContext&) [with ParseContext = std::v10::basic_format_parse_context<char>; T = std::v10::basic_string_view<char>; Char = char]’: --param max-inline-insns-single limit reached [-Winline]
// 2712 |   FMT_CONSTEXPR auto parse(ParseContext& ctx) -> const Char* {
//      |                      ^~~~~
///root/exodus/test/src/../../exodus/libexodus/exodus/exofuncs.h:850:58: note: called from here
//  850 |                 return formatter<std::string_view>::parse(ctx);
//      |                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~

#		pragma GCC diagnostic ignored "-Winline"
#	endif

#endif // EXO_FORMAT == 2 || 3

#if EXO_FORMAT
namespace exo {
	class var;
	class var_base;

template<typename T>
auto cast_var_to_var_base(T&& arg) ->
	std::conditional_t<
		std::is_same_v<std::decay_t<T>, exo::var>,
		exo::var_base&,
		T&&
	> {
	if constexpr (std::is_same_v<std::decay_t<T>, exo::var>) {
		// Ensure lvalue reference for exo::var
		return static_cast<exo::var_base&>(arg);
	} else {
		return std::forward<T>(arg);
	}
}

} // namespace exo

#endif // EXO_FORMAT

#endif // LIBEXODUS_FORMAT_H
