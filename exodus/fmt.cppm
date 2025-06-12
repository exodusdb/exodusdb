module;
    #include <fmt/core.h>
    #include <fmt/format.h>

export module fmt;

export namespace fmt {
    // Include other fmt headers you need, e.g., <fmt/chrono.h>, <fmt/color.h>
	using fmt::vformat;
	using fmt::make_format_args;
	using fmt::format_error;
	using fmt::vprint;
	using fmt::formatter;
}