#include <unistd.h> // for getpid

#include <exodus/var.h>

namespace exodus {

var getprocessn() {
	return var(getpid());
}

var getexecpath() {
	var osenv;
	osenv.osgetenv("_");
	return osenv;
}

}  // namespace exodus
