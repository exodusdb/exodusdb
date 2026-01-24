module;
    #include <fmt/core.h>
    #include <fmt/format.h>

export module fmt;

export namespace fmt {
    // Include other fmt headers you need, e.g., <fmt/chrono.h>, <fmt/color.h>
	using std::vformat;
	using std::make_format_args;
	using std::format_error;
	using std::vprint;
	using std::formatter;
}