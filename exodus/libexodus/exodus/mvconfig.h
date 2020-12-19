/*if modified, should be copied to exodus and pgexodus folders
  otherwise callexodus postgres function may not use the
  correct method (sockets/pipes/boost) when processing
  dictionary columns defined as exodus function calls
*/

/*following, if defined, overide the default method*/
/*#define EXODUS_IPC_BOOST*/
/*#define EXODUS_IPC_POSIX*/
/*#define EXODUS_IPC_WINPIPES*/
#if defined(EXODUS_IPC_POSIX) || defined(EXODUS_IPC_WINPIPES) || defined(EXODUS_IPC_BOOST)
#if defined(EXODUS_IPC_POSIX)
#undef EXODUS_IPC_WINPIPES
#undef EXODUS_IPC_BOOST
#endif
#if defined(EXODUS_IPC_WINPIPES)
#undef EXODUS_IPC_BOOST
#undef EXODUS_IPC_POSIX
#endif
#if defined(EXODUS_IPC_BOOST)
xxx
#undef EXODUS_IPC_WINPIPES
#undef EXODUS_IPC_POSIX
#endif
#else
#if defined(_WIN32) || defined(_WIN64) || defined(_MINGW)
#define EXODUS_IPC_WINPIPES
#else
#define EXODUS_IPC_POSIX
#endif
#endif
