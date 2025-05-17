#ifndef LIBEXODUS_VARIMPL_H_
#define LIBEXODUS_VARIMPL_H_

#include <exodus/var.h>
#include <exodus/varerr.h>

namespace exo {

	// clang-format off

// SIMILAR code in exofuncs.h and varimpl.h
// all defined to allow compile time concatenation
#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
//#	define _EOL      "\r\n"
#	define _OSSLASH  "\\"
#	define  OSSLASH_ '\\'
#	define  SLASH_IS_BACKSLASH true
#else
//#	define _EOL      "\n"
#	define _OSSLASH  "/"
#	define  OSSLASH_ '/'
#	define  SLASH_IS_BACKSLASH false
#endif

	// clang-format on

} // namespace exo

#endif // LIBEXODUS_VARIMPL_H_
