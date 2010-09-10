#include <exodus/mvconfig.h>
#ifdef EXODUS_IPC_POSIX

#include <unistd.h>

#include <exodus/mv.h>

namespace exodus {

var getprocessn()
{
	return var((int) getpid());
}

}//of namespace exodus

#endif
