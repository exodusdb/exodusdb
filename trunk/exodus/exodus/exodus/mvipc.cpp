/*
Copyright (c) 2009 Stephen John Bush

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

#define TRACING 0

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
//#pragma warning (disable: 4530)
#include <iostream>

//#include <stdio.h>
#include <string>

//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h>//in postgres/include

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

//using namespace boost::interprocess;

//
//#define MV_NO_NARROW

#define EXODUS_IPC_EXTERN
#include <exodus/mvipc.h>

#include <exodus/mvenvironment.h>
#include <exodus/mvexceptions.h>

DLL_PUBLIC
boost::thread_specific_ptr<int> tss_environmentns;

//#define BUFSIZE 1048576
#define BUFSIZE 262144

//using namespace std;

//for mingw

namespace exodus
{

//defined in MVdbPostgres
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

	//flag that ipc is started in this thread
	tss_ipcstarted.reset(new bool(true));

	//ensure thread's environment number is available or quit
//	if (!tss_environmentns.get())
//		return false;

	//get thread's mv environment number
	int environmentn=getenvironmentn();

	//start another thread (for this threads environment) to calculate any dictionary items required by the db server backend
	boost::thread thrd1(boost::bind(&MVipc, environmentn, *tss_pgconnparams.get()));

	//wait for the new thread to signal that it is listening for requests before resuming
	boost::mutex::scoped_lock lock(global_ipcmutex);
	//TODO make sure notifies CORRECT thread by using array of ipcmutexes and environmentn
	//TODO put a timeout in case the pipe doesnt open

	#if TRACING > 0
		std::wcout<<L"startipc() Waiting for pipe to be opened\n";
	#endif

	global_ipccondition.wait(lock);

	#if TRACING > 0
		std::wcout<<L"startipc() Waited for pipe to be opened\n";
	#endif

	return true;

}

void getResponseToRequest(char* chRequest, size_t request_size, int maxresponsechars, std::string& response)
{

	//get a pointer to the beginning of request
	//NB not wchar_t*
	char* prequest=chRequest;

	//get a pointer to integer version of the input pointer (to aid getting integer lengths)
	int** plength;
	plength=(int**)&prequest;

	//get the total length from the first bytes of the request
	//int totlength=int(*prequest);
	int totlength=**plength;
	prequest+=sizeof(int);
	
	var reply=L"NEOSYS_IPC_ERROR: ";

	//check it agrees with the number of bytes read from the pipe and fail if it doesnt
	if (request_size<=0)
	{
		reply=L"";
		#if TRACING >= 2
			std::wcout<<L"*";
		#endif
	}
	else if (totlength!=request_size)
	{
		reply^=L" Only ";
		reply^=int(request_size);
		reply^=L" bytes read. Should be ";
		reply^=totlength;
		std::wcout<<reply<<std::endl;

	}

	else
	{
		#if TRACING >= 2
			std::wcout<<L".";
		#endif
		//extract the remainder of the fields in the request

		//filename
		var filename=fromutf8(prequest+sizeof(int),**plength);
		prequest+=*prequest+sizeof(int);

		//dict key
		var dictkey=fromutf8(prequest+sizeof(int),**plength);
		prequest+=*prequest+sizeof(int);

		//record key
		var datakey=fromutf8(prequest+sizeof(int),**plength);
		prequest+=*prequest+sizeof(int);

		//record data
		var data=fromutf8(prequest+sizeof(int),**plength);
		prequest+=*prequest+sizeof(int);

		//valuen
		var valueno=int(**plength);
		prequest+=sizeof(int);

		//subvaluen
		var subvalueno=int(**plength);
		prequest+=sizeof(int);

		#if TRACING >= 3
			std::wcout<<L"MVipc()";
			std::wcout<<L"\ntotal bytes:"<<totlength;
			std::wcout<<L"\nbytes read: "<<request_size;
			std::wcout<<L"\ndictkey:    "<<dictkey;
			std::wcout<<L"\ndatakey:    "<<datakey;
			std::wcout<<L"\ndata:       "<<data;
			std::wcout<<L"\nvalueno:    "<<valueno;
			std::wcout<<L"\nsubvalue:   "<<subvalueno<<std::endl;
		#endif

		//call the relevent dictionary function to calculate the result

		var library;
		if (!library.load(filename))
		{
			//throw
			reply=L"MVCipc() " ^ filename ^ L" unknown filename";
		}
		else
			reply=library.call(filename,dictkey);

	}

	//optionally limit the number of bytes of the reply sent
	response=reply.tostring();
	int nresponsebytes=(int)response.length();
	if (maxresponsechars&&nresponsebytes>maxresponsechars)
	{
		reply=var(L"NEOSYS_IPC_ERROR: Response bytes) " ^ var(nresponsebytes) ^ L" too many for ipc buffer bytes " ^ var(maxresponsechars));
		response=reply.tostring();
	}

	return;

}

}//of namespace exodus
