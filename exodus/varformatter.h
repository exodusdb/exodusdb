// formatter for var must be defined in global namespace

// All c++ format specifiers formatting is locale-independent by default.
// Use the 'L' format specifier to insert the appropriate number separator characters from the locale:
// Locale only affects arithmetic and bool types
// https://en.cppreference.com/w/cpp/utility/format/spec
//
// Exodus conversions likewise although note that some conversions by oconv member function of exoprog is dependent on exoenv globals

/////////////////
// std::formatter - for var.
/////////////////

// NOTE: The presence of a var in a format/println fumction adds 4 seconds to compile time
// possibly because var is a massive class since a formatter for var_base does not suffer
// from the same problem despite the multiple inheritance.
// Unfortunately a formatter for var_base will not be called because format will prefer
// var's implicit conversion to std::string.

// Needs to know how to delegate parse and format functions to standard string_view, double and int versions
// therefore multiple inheritance
template <>
struct std::formatter<exo::var_base> : formatter<std::string_view>, formatter<double>, formatter<int> {
//struct std::formatter<exo::var_base> : formatter<std::string_view>, formatter<double> {
//struct std::formatter<exo::var_base> {

	// Detect leading ':' -> exodus conversions/format specifiers
	// otherwise trailing characters -> standard fmt/std format specifiers
	//
	// parse determines which parse/format functions should be used
	//
	// ':' for exodus conversions which do not use standard parse/float functions
	// 'F' standard floating point f,F,e,E,g,G,a,A
	// 'I' standard intege b,B,d,o,x,X
	// 'S' standard string for s,c or not specified
	char fmt_code_ = ' ';

//    // Explicitly make it copy/move constructible (but immediately delete them)
//    // This satisfies the semiregular / copyable concept checks (is_constructible_v etc.)
//    // without actually allowing copies at runtime. 
//    formatter() = default;
//    formatter(const formatter&) = default;
//    formatter(formatter&&) = default;
//    formatter& operator=(const formatter&) = delete;
//    formatter& operator=(formatter&&) = delete;

	// Need to pass exodus format strings to formatter (starting :) e.g. as in "abc{::MD20}def"
	// since all parsing is done at runtime for exodus conversions
	std::string fmt_str_;

	// TODO allow dynamic arguments for exodus conversions (MD R# etc.)
	// Future use for dynamic arguments in exodus conversions ("format specifiers")
	std::string arg_str; // only here because clang on Ubuntu 22.04 cannot declare str string in constexpr functions
	int dynargn1 = -1;
	// Will require storage of the argument number(s) (either automatic or manual)
	// and using those argument numbers to extract the right argument value in the format stage
	// argument values are available in ctx.arg(n) in the format stage
	// but as they are variants they require careful extraction using visit_format_arg(...) or basic_format_arg.visit(...)
	//
	// 1. For exodus conversions?
	//
	// MD{}{}P ?
	//
	// 2. standard c++ fmt/std::format specifiers already work
	//  because we are calling the standard parse and format functions
	//
	// std::format dynamic arguments
	// https://hackingcpp.com/cpp/libs/fmt.html
	//
	// strings        -> field-width, cut-width
	// chars          -> pad-width
	// integers       -> pad-width
	// floating point -> pad-width, precision
	//
	// Note: int/float are not cut down in size if they exceed the pad-width


//////////////////////////
// std::formatter::parse() - maybe at compile time
//////////////////////////
//
//	NOTE that providing a parse function means the context object passed into format
//  is no longer capable of being forwarded to the standard format functions as in ...
//
//	Works but only supports string format specifier
//
//	template <>
//	struct formatter<exo::var> : formatter<std::string_view> {
//		auto format(const exo::var& var1, format_context& ctx) {
//  	return formatter<std::string_view>::format(var1.toString(), ctx);
//	};
//
//
template<typename ParseContext>
constexpr auto parse(ParseContext& ctx) {

	// Cannot use static timebank in a constexpr function
	//THISIS("auto std::formatter::parse(...)");

	//std::cerr << " \n>>> exofuncs.h parse  '" << std::string(ctx.begin(), ctx.end()) << "'" << std::endl;

	auto it = ctx.begin();

	// We might have been given nothing if pattern was {}
	if (it == ctx.end()) {
		fmt_code_ = 'S';
		return formatter<std::string_view>::parse(ctx);
	}

	// Pick/Exodus conversion codes if starts with :
	const bool exodus_style_conversion = *it == ':';

	while (it != ctx.end()) {

		// Acquire (and skip over?) any dynamic argument
		// TODO handle multiple dynamic arguments?
		if (*it == '{') {
			//fmt_str_.push_back('{');
			//std::string arg_str;
			arg_str.clear();
			it++;

			while (*it != '}' && it != ctx.end()) {
			//	fmt_str_.push_back(*it);
				arg_str.push_back(*it);
				it++;
			}

			if (! arg_str.empty()) {
				try {
					dynargn1 = std::stoi(arg_str);
				} catch (...) {
					//throw std::format_error("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'");
					//throw_format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg ") + arg_str);
					//throw_format_error(std::string("exofuncs.h: formatter_parse: invalid dynamic arg '" + arg_str + "'").c_str());
					// Cannot throw VarError in constexpr parse expression
					throw std::format_error(
						std::string("exofuncs.h: formatter_parse: invalid dynamic arg '")
						.append(arg_str)
						.append("'")
					);
				}
			}

			if (*it == '}') {
				it++;
			}

			continue;
		}

		try {
			// Terminate parse if we reach } char
			if (*it == '}') {

				// 1. exodus style conversions/format specifiers
				// need the whole fmt string (e.g. "MD20P") in the format stage
				// so save it in a member data of the this formatter object
				if (exodus_style_conversion) {
					fmt_code_ = ':';
					fmt_str_ = std::string(ctx.begin() + 1, it);
					return it;
				}

				// 2. C++ style format codes need parsing
				switch (fmt_code_) {

					// Floating point
					case 'a':
					case 'A':
					case 'e':
					case 'E':
					case 'f':
					case 'F':
					case 'g':
					case 'G': {
						fmt_code_ = 'F';
						return formatter<double>::parse(ctx);
					}
					// Integer
					case 'd':
					case 'b':
					case 'B':
					case 'o':
					case 'X':
					case 'x': {
						fmt_code_ = 'I';
						return formatter<int>::parse(ctx);
					}
					// String
					case 'c':
					default:
						fmt_code_ = 'S';
						return formatter<std::string_view>::parse(ctx);
				}
			}
		} catch (std::format_error e) {
//			throw exo::VarError("Format error: " ^ exo::var(std::string(ctx.begin(), ctx.end())).squote() + " " + e.what());
			throw exo::VarError(
				std::string("Format error: '")
				.append(std::string(ctx.begin(), ctx.end()))
				.append("' ")
				.append(e.what())
			);

		} catch (...) {
//			throw exo::VarError("Format error: " ^ exo::var(std::string(ctx.begin(), ctx.end())).squote());
			throw exo::VarError(
				std::string("Format error: '")
				.append(std::string(ctx.begin(), ctx.end()))
				.append("'")
			);
		}

		fmt_code_ = *it;
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		it++;
#pragma GCC diagnostic pop
	}
	// If we dont find a closing "}" char

//#if EXO_FORMAT == 1
	[[unlikely]]
	// Cannot throw VarError in constexpr parse
	throw std::format_error("exofuncs.h: formatter_parse: format missing trailing }");
//#else
//	throw_format_error("exofuncs.h: formatter_parse: format missing trailing }");
//		fmt_str_ = "{}";
//		return it;
//#endif
	//std::unreachable();

} // formatter::parse()


//////////////////////
// formatter::format() - run time
//////////////////////

// Good format code description although not from "fmt", not official c++
// https://fmt.dev/latest/syntax.html#formatspec

template <typename FormatContext>
auto format(const exo::var_base& var1, FormatContext& ctx) const {

	THISIS("auto std::formatter::format(var) const")
	//std::cerr << ">>> exofuncs.h format '" << fmt_str_ << "' '" << fmt_code_ << "' '" << var1 << "'\n";

	switch (fmt_code_) {

		// 1. EXODUS conversions

		case ':': {

			// Unfortunately without time zone or number format currently.
			// TODO allow thread_local global timezone, number format?
			// {::MD20PZ}
			// {::D2/E} etc.
			// {::MTHS} etc.
			//return formatter<std::string_view>::format(var1, ctx);
			exo::var_base converted_var1 = static_cast<const exo::var&>(var1).oconv(fmt_str_.c_str());
			auto sv1 = std::string_view(converted_var1);
			return vformat_to(ctx.out(), "{:}", std::make_format_args(sv1));

		}

		// 2. C++ style format codes

		// Standard floating point on var toDouble()
		case 'F': {
			return formatter<double>::format(var1.toDouble(), ctx);
		}

		// Standard integer on var toInt()
		case 'I': {
			return formatter<int>::format(var1.toInt(), ctx);
		}

		// Standard string on var toString()
		case 'S':
		default:
			return formatter<std::string_view>::format(std::string_view(var1), ctx);

	}
} // formatter::format

}; //std::formatter



// How to format user defined types.
//
// 1. https://en.cppreference.com/w/cpp/utility/format/formatter
//
// 2. https://fmt.dev/latest/api.html
// Section: Formatting User-Defined Types
//
// *** NEEDS #include fmt/format.h not fmt/core.h
//
//template<class T>
//struct formatter {
//	constexpr auto parse(format_parse_context&);
//
//	typename format_context::iterator
//	format(const T&, format_context&);
//};
//


//namespace fmt {
//
//// Works fine but only supports string format specifiers
//template <>
//struct formatter<exo::var> : formatter<std::string_view> {
//
//auto format(const exo::var& var1, format_context& ctx) {
//	// Sadly we dont have access to the formatstring that parse has access to
//	//auto s = std::string(ctx.begin(), ctx.end());
//	return formatter<std::string_view>::format(var1.toString(), ctx);
//}
//
//}; // formatter
//
//} // namespace fmt
