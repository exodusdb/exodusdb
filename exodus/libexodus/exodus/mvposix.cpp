#include <exodus/mvconfig.h>
#ifdef EXODUS_IPC_POSIX

#include <unistd.h>

#include <exodus/mv.h>

namespace exodus {

var getprocessn() {
	return var((int)getpid());
}

var getexecpath() {
	var osenv;
	osenv.osgetenv("_");
	return osenv;
}

}  // namespace exodus

#endif
