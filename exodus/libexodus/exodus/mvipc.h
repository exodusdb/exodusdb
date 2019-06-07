#ifndef MVIPC_H
#define MVIPC_H

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include <boost/locale.hpp>

#include <libpq-fe.h> //in postgres/include

#include <exodus/mv.h>

// needed to access functorbase.mv.ID and RECORD MV
#include <exodus/mvenvironment.h>
#include <exodus/mvfunctor.h>

//#include <exodus/mvutf.h>

namespace exodus
{

// defined in MVdbPostgres
extern boost::thread_specific_ptr<PGconn> tss_pgconns;
extern boost::thread_specific_ptr<var> tss_pgconnparams;
extern boost::thread_specific_ptr<bool> tss_ipcstarted;

EXODUS_IPC_EXTERN boost::mutex global_ipcmutex;
EXODUS_IPC_EXTERN boost::condition global_ipccondition;

// definition here prevents this header from being included in more than one file
inline std::wstring fromutf8(const char* start, const int length)
{
	// return wstringfromUTF8((UTF8*)(start), length);
	return boost::locale::conv::utf_to_utf<wchar_t>(start, start + length);
}

// this function is started as a thread by startipc()
int MVipc(const int environmentn, var& pgconnparams);

void getResponseToRequest(char* chRequest, size_t request_size, int maxresponsechars,
			  std::string& response, ExodusFunctorBase& exodusfunctorbase);

} // namespace exodus

#endif // MVIPC_H
