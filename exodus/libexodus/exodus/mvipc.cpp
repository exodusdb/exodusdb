/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef NDEBUG
#define TRACING 1
#else
#define TRACING 2
#endif
//#define TRACING 5

// C4530: C++ exception handler used, but unwind semantics are not enabled.
//#pragma warning (disable: 4530)
#include <iostream>

//#include <stdio.h>
#include <string>

//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h> //in postgres/include

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

// using namespace boost::interprocess;

//
//#define MV_NO_NARROW

#define EXODUS_IPC_EXTERN
#include <exodus/mvipc.h>

//#include <exodus/mvenvironment.h>
#include <exodus/mvexceptions.h>

// using namespace exodus;
//#include <exodus/xfunctorf6.h>
#include <exodus/mvfunctor.h>

DLL_PUBLIC
boost::thread_specific_ptr<int> tss_environmentns;

//#define BUFSIZE 1048576
#define BUFSIZE 262144

// using namespace std;

// for mingw

namespace exodus
{

// defined in MVdbPostgres
extern boost::thread_specific_ptr<PGconn> tss_pgconns;
extern boost::thread_specific_ptr<var> tss_pgconnparams;
extern boost::thread_specific_ptr<bool> tss_ipcstarted;

int getenvironmentn()
{
	if (!tss_environmentns.get())
		return 0;
	return *tss_environmentns.get();
}

void setenvironmentn(const int environmentn)
{
	tss_environmentns.reset(new int(environmentn));
	return;
}

bool startipc()
{

	// flag that ipc is started in this thread
	tss_ipcstarted.reset(new bool(true));

	// ensure thread's environment number is available or quit
	//	if (!tss_environmentns.get())
	//		return false;

	// get thread's mv environment number
	int environmentn = getenvironmentn();

	// start another thread (for this threads environment) to calculate any dictionary items
	// required by the db server backend ALN:DANGEROUS: following line works only if
	// tss_pgconnparams was reset with some string
	boost::thread thrd1(boost::bind(&MVipc, environmentn, *tss_pgconnparams.get()));

	// wait for the new thread to signal that it is listening for requests before resuming
	boost::mutex::scoped_lock lock(global_ipcmutex);
	// TODO make sure notifies CORRECT thread by using array of ipcmutexes and environmentn
	// TODO put a timeout in case the pipe doesnt open

#if TRACING > 3
	std::wclog << L"startipc() Waiting for pipe to be opened\n";
#endif

	global_ipccondition.wait(lock);

#if TRACING > 3
	std::wclog << L"startipc() Waited for pipe to be opened\n";
#endif

	return true;
}

void getResponseToRequest(char* chRequest, size_t request_size, int maxresponsechars,
			  std::string& response, ExodusFunctorBase& exodusfunctorbase)
{

	// TODO make independent of int size and byte ordering
	// otherwise we are restricted to accessing servers
	// with the same int size and byte ordering
	// MAYBE implement a 1 byte header to communicate version, int size and byte ordering

	// get a pointer to the beginning of request
	// NB not wchar_t*
	char* prequest = chRequest;

	// get a pointer to integer version of the input pointer (to aid getting integer lengths)
	int** plength;
	plength = (int**)&prequest;

	// get the total length from the first bytes of the request
	// int totlength=int(*prequest);
	int totlength = **plength;
	prequest += sizeof(int);

	// var reply=L"EXODUS_IPC_ERROR: ";
	response = "EXODUS_IPC_ERROR: ";

	// check it agrees with the number of bytes read from the pipe and fail if it doesnt
	if (request_size <= 0)
	{
		response = "";
#if TRACING >= 2
		// std::wclog<<L"*";
		std::wclog << L"mvipc: request size is zero";
#endif
	}

	// TODO resolve warning: comparison between signed and unsigned integer expressions
	else if (totlength != int(request_size))
	{
		var reply = response ^ L" Only " ^ int(request_size) ^ L" bytes read. Should be " ^
			    totlength;
		std::cerr << reply << std::endl;
		response = reply.toString();
		return;
	}

	else if (!exodusfunctorbase.mv_)
	{
		var reply = response ^ L" MvEnvironment is not initialised";
		std::cerr << reply << std::endl;
		response = reply.toString();
		return;
	}
	else
	{
#if TRACING >= 2
		std::wclog << L".";
#endif
		// extract the remainder of the fields in the request

		// filename
		// TODO find a way to pass filename into mv of dictionary routine
		//		var filename=fromutf8(prequest+sizeof(int),**plength);
		std::string str_libname(prequest + sizeof(int), **plength);
		// TODO deduplicate code to construct library name duplicated in ::calculate and
		// mvipc
		str_libname.insert(0, "dict_");
		prequest += *prequest + sizeof(int);

		// dict key
		// TODO find a way to pass dictkey into mv of dictionary routine
		//		var dictkey=fromutf8(prequest+sizeof(int),**plength);
		std::string str_funcname(prequest + sizeof(int), **plength);
		prequest += *prequest + sizeof(int);

		// record key into mv ID
		// var datakey=fromutf8(prequest+sizeof(int),**plength);
		// NB if you no longer access mv_ directly here
		// then make mv_ private in ExodusFunctorBase
		// exodusfunctorbase.mv_->ID=fromutf8(prequest+sizeof(int),**plength);
		exodusfunctorbase.mv_->ID = std::string(prequest + sizeof(int), **plength);
		prequest += *prequest + sizeof(int);

		// record data into mv RECORD
		// var data=fromutf8(prequest+sizeof(int),**plength);
		// exodusfunctorbase.mv_->RECORD=fromutf8(prequest+sizeof(int),**plength);
		exodusfunctorbase.mv_->RECORD = std::string(prequest + sizeof(int), **plength);
		prequest += *prequest + sizeof(int);

		// valueno into mv MV
		// var valueno=int(**plength);
		exodusfunctorbase.mv_->MV = int(**plength);
		prequest += sizeof(int);

		// subvalueno
		// TODO is this really required? if so, find a way to pass it into mv of dictionary
		// routine 		var subvalueno=int(**plength);
		prequest += sizeof(int);

#if TRACING >= 3
		std::wclog << L"MVipc()";
		std::wclog << L"\ntotal bytes:" << totlength;
		std::wclog << L"\nbytes read: " << request_size;
		std::wclog << L"\nfilename:   " << str_libname;
		std::wclog << L"\ndictkey:    " << str_funcname;
		std::wclog << L"\ndatakey:    " << exodusfunctorbase.mv_->ID;
		std::wclog << L"\ndata:       " << exodusfunctorbase.mv_->RECORD;
		std::wclog << L"\nvalueno:    " << exodusfunctorbase.mv_->MV;
		//			std::wclog<<L"\nsubvalue:   "<<subvalueno;
		std::wclog << std::endl;
#endif

		/* old way
				//call the relevent dictionary function to calculate the result
				var library;
				if (!library.load(filename))
				{
					//throw
					reply=L"MVCipc() " ^ filename ^ L" unknown filename";
				}
				else
					reply=library.call(filename,dictkey);
		*/
		// std::cerr<<L"mvipc: " << str_libname.c_str() << L" " <<
		// str_funcname.c_str()<<std::endl; if (not
		// exodusfunctorbase.initsmf(str_libname.c_str(),str_funcname.c_str()))
		std::string prefixed_funcname = "exodusprogrambasecreatedelete_" + str_funcname;
#if TRACING >= 3
		// hangs?
		// std::cerr<<L"mvipc: " << str_libname.c_str() << L" " <<
		// prefixed_funcname.c_str()<<std::endl;
#endif
		if (not exodusfunctorbase.initsmf(str_libname.c_str(), prefixed_funcname.c_str()))
		/////////////////////////////////////////////////////////////////////////////
		{
			var reply = response ^ L"Cannot find Library " ^ str_libname ^
				    L", or function " ^ str_funcname ^ L" is not present";
			std::cerr << reply << std::endl;
			response = reply.toString();
			return;
		}
		else
		{

			try
			{

				exodusfunctorbase.mv_->ANS = exodusfunctorbase.callsmf();
/////////////////////////////
#if TRACING >= 3
				std::cerr << "mvipc: called callsmf successfully " << std::endl;
#endif

				// dictionary subroutines return return in mv ANS.
				response = exodusfunctorbase.mv_->ANS.toString();

#if TRACING >= 3
				std::cerr << "mvipc: got ANS " << std::endl;
#endif

				// optionally limit the number of bytes of the reply sent
				int nresponsebytes = (int)response.length();
				if (maxresponsechars && nresponsebytes > maxresponsechars)
				{
					var reply = var(L"EXODUS_IPC_ERROR: Response bytes) " ^
							var(nresponsebytes) ^
							L" too many for ipc buffer bytes " ^
							var(maxresponsechars)) ^
						    " while calling " ^ str_libname ^ ", " ^
						    str_funcname;
					std::cerr << reply << std::endl;
					response = reply.toString();
					return;
				}

				return;
			}
			catch (MVException mve)
			{
				var reply = response ^ "ERROR: Calling " ^ str_libname ^ ", " ^
					    str_funcname ^ L". ERROR: " ^ mve.description;
				std::cerr << reply << std::endl;
				response = reply.toString();
				return;
			}
			catch (...)
			{
				var reply = response ^ "ERROR: Calling " ^ str_libname ^ ", " ^
					    str_funcname;
				std::cerr << reply << std::endl;
				response = reply.toString();
				return;
			}
		}
	}

	// shouldnt get here, but if by mistake we do then response is set to an error string.
	return;
}

} // namespace exodus
