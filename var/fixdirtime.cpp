#include "fixdirtime.h"

#if EXO_MODULE > 1
	import std;
#else
#	include <format>
#	include <system_error>
#	include <chrono>
#	include <filesystem>
#	include <string>
#	include <print>		  // C++23 — for std::print / std::println
#endif
#include <version>

// clang-20 with libc++
//__cpp_lib_chrono     = 201611
// → only older chrono (201611L or similar)
//__cpp_lib_chrono_udls = 201304 (ok)
//The big jump to 201907L (C++20 calendars + time zones + full clock interop including clock_cast and reliable file_clock::to_sys) is still partial / in progress in libc++.

#ifdef EXO_PRESERVE_ATIME
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#endif

// Still needed unfortunately for popen/pclose + FILE* which isnt easy in std c++
#include <cstdio>

namespace fs  = std::filesystem;
namespace chr = std::chrono;

// ──────────────────────────────────────────────────────────────────────────────
// Comparison & conversion helpers
// ──────────────────────────────────────────────────────────────────────────────

static std::string file_time_to_string(
	const fs::file_time_type& file_time
) {

// Amend to ??0000 when libc++ supports chr::file_clock_to_sys(file_time)

#if ! _LIBCPP_VERSION || _LIBCPP_VERSION > 000000

    // Modern path: libstdc++ / recent libc++ / other

	auto sys_time = chr::file_clock::to_sys(file_time);   // may not compile or may be wrong on older libc++

	// %S outputs 99.999999999999 very conveniently
	return std::format("{:%Y-%m-%d %H:%M:%S}", sys_time);

//    // Floor to whole seconds → %S will now output integer seconds only
//    auto whole_seconds = chr::floor<chr::seconds>(sys_time);
//
//    return std::format("{:%Y-%m-%d %H:%M:%S}.{:09}",
//                       whole_seconds,
//                       (sys_time - whole_seconds).count());  // ns since last whole second

#elif __cpp_lib_chrono >= 201907L  // C++20 feature-test macro for chrono enhancements

    // Modern portable path: clock_cast (C++20+)
    auto sys_time = std::chrono::clock_cast<std::chrono::system_clock>(file_time);

    // %S outputs 99.999999999999 very conveniently if we have subseconds
    return std::format("{:%Y-%m-%d %H:%M:%S}", sys_time);

#else

	// Manual formatting since fs::file_time_clock is not fully implemented on libc++

	//Get duration since epoch
	auto dur = file_time.time_since_epoch();

	//Extract seconds and sub-second remainder
	auto sec = chr::duration_cast<chr::seconds>(dur);
	auto ns  = chr::duration_cast<chr::nanoseconds>(dur - sec);

	//Convert seconds to days (time_point needed)
	auto days_since_epoch = chr::floor<chr::days>(sec);
	chr::sys_days sd{days_since_epoch};       // <-- wrap in sys_days
	chr::year_month_day ymd{sd};

	//Extract hours/minutes/seconds
	chr::hh_mm_ss hms{sec - days_since_epoch};

	//Format manually with std::format
	return std::format(
		"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:09}",
		int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
		hms.hours().count(), hms.minutes().count(), hms.seconds().count(),
		ns.count()
	);
#endif
}

// Only used inside EXO_PRESERVE_ATIME block
#ifdef EXO_PRESERVE_ATIME
timespec file_time_to_timespec(
	const fs::file_time_type& ft
) {
#if __cpp_lib_chrono >= 201907L
	auto sys = std::chrono::clock_cast<std::chrono::system_clock>(ft);
#else
	auto sys = chr::file_clock::to_sys(ft);  // fallback
#endif
	auto d   = sys.time_since_epoch();
	auto s   = chr::duration_cast<chr::seconds>(d);
	auto ns  = chr::duration_cast<chr::nanoseconds>(d - s);
	return {static_cast<time_t>(s.count()), static_cast<long>(ns.count())};
}
#endif

// ──────────────────────────────────────────────────────────────────────────────
// Slow find-based verification (still uses timespec string format for comparison)
// ──────────────────────────────────────────────────────────────────────────────
bool verify_proposed(
	const fs::path& dir,
	const fs::file_time_type& proposed,
	const exo::WalkOptions& opt
) {

#ifndef __unix__   // or #if defined(__linux__) || defined(__APPLE__) || ...
	std::print(stderr, "fixdirtime: Verify not implemented - skipping\n");
#else
	std::string expected_str = file_time_to_string(proposed);

	// find command to output the latest file/dir time found
	std::string cmd =
		"find '" + dir.string() +
		"' -exec stat -c '%y %n' {} + 2>/dev/null | sort | tail -n 1";

	// Execute
	// TODO convert to use var::process or var::osshellread?
	FILE* p = popen(cmd.c_str(), "r");
	if (!p) return opt.ignore_errors;

	// Collect output
	char buf[1024];
	std::string result;
	while (fgets(buf, sizeof(buf), p)) result += buf;
	pclose(p);

	// Trim trailing whitespace
	while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) result.pop_back();

	// Handle mismatch
	bool match = result.starts_with(expected_str);
	if (!match || opt.verbose) {
		std::print("[FIND  ] {}\n", cmd);
		std::print(
			stderr,
			"[{: <6}] Verify {} → exp {}   got {}\n",
			match ? "MATCH" : "DIFFER",
			dir.string(),
			expected_str,
			result.empty() ? "<empty>" : result
		);
	}

	if (!match && !opt.ignore_errors) {
		std::print(stderr, "Mismatch - aborting\n");
		return false;
	}
#endif
	return true;
}

namespace exo {

// ──────────────────────────────────────────────────────────────────────────────
// Returns the max mtime of the whole subtree (including this dir)
// ──────────────────────────────────────────────────────────────────────────────
fs::file_time_type post_order_process(
	const fs::path& dir,
	const WalkOptions& opt,
	fs__file_size_type& recursive_size,
	int depth
) {

	// lambda for logging
	auto log = [&](
		const char* mode,
		const fs::path& p,
		const fs::file_time_type& file_time
	) {
		if (opt.verbose || std::strcmp(mode, "DIFFER") == 0) {
			std::string file_time_str = file_time_to_string(file_time);
			std::print("[{: <6}] {}: {: <40} → {}\n",
					   mode, depth, p.string(), file_time_str);
		}
	};

	// Omit levels > max depth
	if (depth > opt.max_depth) {
		// Secondary logging
		if (opt.verbose > 1)
			std::print(stderr, "Depth limit: {}\n", dir.string());
		return fs::last_write_time(dir);
	}

	// Start with the dir time
	fs::file_time_type max_file_time = fs::last_write_time(dir);

	// Scan all files and dirs permitted
	// Not using fs::recursive_directory_iterator since we need post subdir processing and depth first.
	std::error_code ec;
	thread_local int error_count = 0;
	for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {

		// Ignore or quit on errors;
		if (ec) {
			if (error_count++ < 5) {
				std::print(
					stderr,
					"[{: <6}] Iterating {}. {} - {}\n",
					"ERROR",
					dir.string(),
					ec.value(),
					ec.message()
				);
			}
			if (!opt.ignore_errors)
				return max_file_time;
            ec.clear();
            continue;			continue;
		}

		// Skipping symlink file and dirs
		if (entry.is_symlink())
			continue;

		const fs::path& p = entry.path();

		// file
		if (entry.is_regular_file()) {

			// Accumulate all file sizes
			recursive_size += entry.file_size();
			auto file_mtime = fs::last_write_time(entry);

			// Tertiary logging
			if (opt.verbose > 2)
				log("FILE", p, file_mtime);

			// Bump and log mtime
			if (file_mtime > max_file_time) {
				max_file_time = file_mtime;

				// Secondary logging
				if (opt.verbose > 1)
					log("BUMP F", p, file_mtime);
			}
		}

		// dir
		else if (entry.is_directory()) {

			// Skip volatile / special filesystems
			std::string subdir_path = p.lexically_normal().string();
			if (subdir_path == ("/proc")   ||
				subdir_path == ("/sys")  ||
				subdir_path == ("/run")   ||
				subdir_path == ("/dev")   ||
				subdir_path == ("/var")) {

				// Log skipped
				if (opt.verbose)
					std::print("[SKIP] Volatile/special: {}\n", subdir_path);

				// Bump and log mtime even if skipped.
				// Even skipped dirs are skipping content, not the dir itself
				// although their parent / can never be updated we can show the max mtime
				auto sub_file_time = fs::last_write_time(p);
				if (sub_file_time > max_file_time) {
					max_file_time = sub_file_time;
					// Secondary logging
					if (opt.verbose > 1)
						log("BUMP D (skipped)", p, sub_file_time);
				}
				continue;
			}

			// ── Recurse ─────────────────────────────────────────────────────
			auto sub_max_file_time = post_order_process(p, opt, recursive_size, depth + 1);

			// Propagate abort
			if (sub_max_file_time == fs::file_time_type::min() && !opt.ignore_errors) {
				// Log that we're aborting this level too
				if (opt.verbose)
					std::print(stderr, "[ABORT ] Propagating verify failure from {}\n", p.string());
				return fs::file_time_type::min();  // propagate abort upward
			}

			// Bump time
			if (sub_max_file_time > max_file_time)
				max_file_time = sub_max_file_time;
		}
	}

	// Post-order: consider updating directory itself
	//////////////////////////////////////////////////

	// Update top dir.
	// Note: Cannot update root dir /
	if (max_file_time > fs::last_write_time(dir)
		&& dir != dir.root_directory()
	) {

		// Verify
		if (opt.verify && !verify_proposed(dir, max_file_time, opt)) {
			// signal failure
			return fs::file_time_type::min();
		}

		// Update dir mtime
		if (opt.update) {
#ifdef EXO_PRESERVE_ATIME
			struct stat st{};
			if (::stat(dir.c_str(), &st) == 0) {
				timespec ts[2] = {st.st_atim, file_time_to_timespec(max_file_time)};
				::utimensat(AT_FDCWD, dir.c_str(), ts, 0);
			} else if (!opt.ignore_errors) {
				// failed to stat → cannot preserve atime
			}
#else
			try {
				fs::last_write_time(dir, max_file_time);
			} catch (const fs::filesystem_error&) {
				if (!opt.ignore_errors)
					return fs::file_time_type::min(); // abort marker
			}
#endif
			// Primary logging
			log("UPDATE", dir, max_file_time);
		} else {
			log("UPDREQ", dir, max_file_time);
		}
	}

	return max_file_time;
}

} // namespace exo