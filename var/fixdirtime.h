#pragma once

// Self test with compile 'osinfo("/", 8)'

#if EXO_MODULE > 1
	import std;
#else
#	include <filesystem>
#	include <string>
#	include <chrono>
#endif

namespace fs  = std::filesystem;
namespace chr = std::chrono;

namespace exo {

struct WalkOptions {
	bool update        = false;
	bool verify        = false;
	bool ignore_errors = true;
	int  verbose       = 0;
	int  max_depth     = 64;
};

// typdef for missing file_size_type - use instead of raw uintmax
using fs__file_size_type =
	decltype(std::filesystem::file_size(std::declval<std::filesystem::path>()));

// ──────────────────────────────────────────────────────────────────────────────
// Returns the max mtime of the whole subtree (including this dir)
// ──────────────────────────────────────────────────────────────────────────────
fs::file_time_type post_order_process(
	const fs::path& dir,
	const WalkOptions& opt,
	fs__file_size_type& recursive_size,
	int depth = 0
);

} // namespace exo
