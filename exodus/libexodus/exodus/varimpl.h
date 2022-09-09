#ifndef EXODUS_LIBEXODUS_EXODUS_VARIMPL_H_
#define EXODUS_LIBEXODUS_EXODUS_VARIMPL_H_

#include <exodus/var.h>
#include <exodus/varerr.h>

namespace exodus {

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#	define _OSSLASH "\\"
#	define OSSLASH_ '\\'
#	define SLASH_IS_BACKSLASH true
#else
#	define _OSSLASH "/"
#	define OSSLASH_ '/'
#	define SLASH_IS_BACKSLASH false
#endif

} // namespace exodus

#endif // EXODUS_LIBEXODUS_EXODUS_VARIMPL_H_
