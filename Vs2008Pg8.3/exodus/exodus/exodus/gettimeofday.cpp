//replacement for missing gettimeofday in mingw
//http://openbabel.sourceforge.net/wiki/Install_(MinGW)

#ifdef __MINGW32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <time.h>
#include <sys/time.h>

void __stdcall GetSystemTimeAsFileTime(FILETIME*);

void gettimeofday(struct timeval* p, void* tz /* IGNORED */)
{
      union {
         long long ns100; /*time since 1 Jan 1601 in 100ns units */
             FILETIME ft;
      } now;

  GetSystemTimeAsFileTime( &(now.ft) );
  p->tv_usec=(long)((now.ns100 / 10LL) % 1000000LL );
  p->tv_sec= (long)((now.ns100-(116444736000000000LL))/10000000LL);
}
#endif

