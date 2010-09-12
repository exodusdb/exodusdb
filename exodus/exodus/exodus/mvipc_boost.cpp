#include <exodus/mvconfig.h>
#ifdef EXODUS_IPC_BOOST

//http://www.boost.org/doc/libs/1_39_0/doc/html/interprocess/acknowledgements_notes.html
#include <boost/interprocess/ipc/message_queue.hpp>

#define EXODUS_IPC_EXTERN extern
#include <exodus/mvipc.h>

//#define BUFSIZE 1048576
//cant be bigger than process stacksize .. this LIMITS boost messaging
#define BUFSIZE 262144

#ifndef DEBUG
#define TRACING 0
#else
#define TRACING 2
#endif

namespace exodus {

void respondToRequests(boost::interprocess::message_queue& request_queue, boost::interprocess::message_queue& response_queue)
{

	std::size_t request_size;
	unsigned int priority;

	char chRequest[BUFSIZE];
	//char chResponse[BUFSIZE];
	//int nresponsebytes;
	std::string response;

	while (true)
	{

		//get a request
		try
		{
			request_queue.receive(&chRequest, BUFSIZE, request_size, priority);
		}

		//handle failure to get a request
		catch(boost::interprocess::interprocess_exception &ex)
		{
			std::cout << "mvipc: failed to receive " << ex.what() << std::endl;
			continue;
		}

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc() read  %d bytes from pipe\n",request_size);
		#endif
		
		//determine a response
		getResponseToRequest(chRequest,request_size,0,response);

		//send a response
		try
		{
			response_queue.send(response.data(), response.length(), 0);
		}

		//handle failure to send response
		catch(boost::interprocess::interprocess_exception &ex)
		{
			std::cout << "mvipc: failed send " << ex.what() << std::endl;
			continue;
		}

		//log
		#if TRACING >= 3
			wprintf(L"MVipc() wrote %d bytes <- \"\"\n",nresponsebytes);
		#endif

	}
}

void closeipcqueues(const std::string& requestqueuename, const std::string& responsequeuename)
{
	//close the request queue
	try
	{
		boost::interprocess::message_queue::remove(requestqueuename.c_str());
	}
	catch(boost::interprocess::interprocess_exception &ex)
	{
		std::cout << "cannot close " << requestqueuename << " " << ex.what() << std::endl;
	}

	//close the response queue
	try
	{
		boost::interprocess::message_queue::remove(responsequeuename.c_str());
	}
	catch(boost::interprocess::interprocess_exception &ex)
	{
		std::cout << "cannot close " << responsequeuename << " " << ex.what() << std::endl;
	}

}

//this function is started as a thread by startipc()
int MVipc(const int environmentn, var& pgconnparams)
{

	//TODO prevent or handle SELECT in dictionary functions

	//flag to connect NOT to be recursive and open yet another ipc thread
	tss_ipcstarted.reset(new bool(true));

	//clone the postgres connection because the parent thread is running a select with it
	if (!var().connect(pgconnparams))
	{
		throw var(L"MVipc Cannot connect additional thread to postgres");
		return false;
	}

	//set the threads environment number (same as and provided by the parent thread)
	//AFTER opening the database connection
	setenvironmentn(environmentn);
	var processn=getprocessn();
		
	//"\\\\.\\pipe\\exoduspipexyz"
	//strings of MS tchars
	//typedef basic_string<TCHAR> tstring;
	//wchar_t* exoduspipename="\\\\.\\pipe\\exoduspipexyz";
	var temp="requestexodusservice-"^processn^L"."^environmentn;
	std::string requestqueuename=temp.tostring();
	var temp2="responseexodusservice-"^processn^L"."^environmentn;
	std::string responsequeuename=temp2.tostring();
	//string requestqueuename="requestexodusqueue";
	//requestqueuename+=environmentn;
	//string responsequeuename="responseexodusqueue";
	//responsequeuename+=environmentn;

	closeipcqueues(requestqueuename, responsequeuename);

	try
	{
		//open request queue
		boost::interprocess::message_queue request_queue
		(
			boost::interprocess::open_or_create	//only create
			,requestqueuename.c_str()	//name
			,100		//max message number
			,BUFSIZ		//max message size
		);

		try
		{
			//open response queue
			boost::interprocess::message_queue response_queue
			(
				boost::interprocess::open_or_create	//only create
				,responsequeuename.c_str()			//name
				,100		//max message number
				,BUFSIZ		//max message size
			);

			//indicate to waiting/paused parent thread that the pipe is open
			//(the pipe is not actually waiting until the next step)
			//scoped so that the scoped_lock is automatically released after the notification
			{
				boost::mutex::scoped_lock lock(global_ipcmutex);
				#if TRACING >= 3
					wcout<<L"MVipc() Notifying that pipe has been opened\n";
				#endif
				//TODO make sure notifies CORRECT parent thread by using an array of ipcmutexes and tss_environmentn
				global_ipccondition.notify_one();
				#if TRACING >= 3
						wcout<<L"MVipc() Notified that pipe has been opened\n";
				#endif
			}

			respondToRequests(request_queue,response_queue);
			
			std::cout << "stopped responding to queue " << requestqueuename<<std::endl;

		}
		catch(boost::interprocess::interprocess_exception &ex)
		{
			global_ipccondition.notify_one();
			std::cout << "cannot open " << responsequeuename << " " << ex.what() << std::endl;
			closeipcqueues(requestqueuename, responsequeuename);
			return 0;
		}

	}
	catch(boost::interprocess::interprocess_exception &ex)
	{
		global_ipccondition.notify_one();
		std::cout << "cannot open " << requestqueuename << " " << ex.what() << std::endl;
		closeipcqueues(requestqueuename, responsequeuename);
		return 0;
	}

	closeipcqueues(requestqueuename, responsequeuename);
	return 1;
}


}//namespace exodus

#endif //not windows

