#include <limits.h>
#include <unistd.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <exodus/var.h>

namespace exodus {

ND PUBLIC var getprocessn() {
	return var(getpid());
}

ND PUBLIC var getexecpath() {
//	var osenv;
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
//    char hostname[1024];
//    hostname[1023] = '\0';
//    ::gethostname(hostname, 1023);
	std::string hostname(' ', HOST_NAME_MAX);
	if (::gethostname(hostname.data(), HOST_NAME_MAX))
		hostname = "";
	else {
		size_t pos = hostname.find('\0');
		if (pos != std::string::npos)
			hostname.erase(pos);
	}

    return hostname;
}

}  // namespace exodus
