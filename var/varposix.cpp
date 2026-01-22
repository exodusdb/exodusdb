#if EXO_MODULE > 1
	import std;
#else
#	include <filesystem>
#endif

#include <cassert> // for assert
#include <limits.h> // for HOST_NAME_MAX
#include <unistd.h> // for getpid

// Abbreviated namespace
namespace fs = std::filesystem;

#include <var/var.h>

namespace exo {

using let = const var;

ND PUBLIC var getprocessn() {
	return var(getpid());
}

ND PUBLIC var getexecpath() {
//	let osenv;
//	if (not osenv.osgetenv("_"))
//		osenv = "";
//	return osenv;
	var path;
	try {
		fs::path p {"/proc/self/exe"};
		path = fs::read_symlink(p).string();
	} catch (...) {
		path = "";
	}
	return path;
}

ND PUBLIC std::string gethostname() {

	// Acquire std::string space buffer for c function
	// HOST_NAME_MAX is 64 on Ubuntu 24.04
	// On Linux, HOST_NAME_MAX is defined with the value 64
	std::string hostname(HOST_NAME_MAX, ' ');
	assert(hostname.size() == HOST_NAME_MAX);

	// man gethostname
	if (::gethostname(hostname.data(), HOST_NAME_MAX)) {
		// No hostname if any error
		hostname = "";
		return hostname;
	}

	// Trim std string from trailing \0 return by c function
	std::size_t pos = hostname.find('\0');
	if (pos != std::string::npos)
		hostname.erase(pos);

    return hostname;
}

}  // namespace exo
