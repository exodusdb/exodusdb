//#if defined(_WIN32) || defined(_WIN64)  || defined(_MINGW)y
#include "mvconfig.h"
#ifdef EXODUS_IPC_BOOST

//NB boost message_queue on windows are subject to security but there is no security control on the boost api
//so the only solution found so far is to run postgres as the same user as the exodus client is run under
//this requires that you grant the exodus client's user full access to the postgres data directory
//an alternative untested solution might be to somehow run the exodus client as the postgres user

/*problems using exceptions in postgres c functions
"In general it's possible to use exceptions in C++ code that's being
called from C so long as you make sure you catch all possible exceptions
at the C/C++ interface point." 
http://www.nabble.com/C%2B%2B-User-defined-functions-td22417031.html
*/

/* insert debug code like this
strncpy(response,"opened",BUFSIZE);
response[BUFSIZE-1]='\0'
nresponsebytes=strlen(response);
return 1;
*/

//evade warnings about using strncpy strncat on msvc
#define _CRT_SECURE_NO_WARNINGS

//http://www.boost.org/doc/libs/1_38_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.message_queue
#include <boost/interprocess/ipc/message_queue.hpp>

//needs to be linkable to C functions
extern "C" {

////////////TODO: NEEDS FIXING!!! limited by stack size of around 1mb for global functions in the exodus client (convert to classes to overcome)
#define BUFSIZE 262144

bool callexodus(const char* serverid, const char* request, const int nrequestbytes, char* response, int* nresponsebytes)
{

	//g++ asm(" int $03");

	std::string requestqueuename="request";
	requestqueuename+=serverid;
	std::string responsequeuename="response";
	responsequeuename+=serverid;

	/*** send request ***/

	try
	{
		///elog(WARNING, "callexodus: opening request_queue");
		boost::interprocess::message_queue request_queue
		(
			boost::interprocess::open_only		//only open existing
			,requestqueuename.c_str()		//name
		);

		//elog(WARNING, "callexodus: sending request");

		//some "send" macro interferring with request_queue.send() so remove it
		#undef send
		try
		{
			request_queue.send(request, nrequestbytes, 0);
		}
		catch(boost::interprocess::interprocess_exception &ex)
		{
			//std::cout << ex.what() << std::endl;
			strncpy(response,"pgexodus failure to send message_queue '",BUFSIZE);
			strncat(response,requestqueuename.c_str(),BUFSIZE);
			strncat(response,"' ",BUFSIZE);
			strncat(response,ex.what(),BUFSIZE);
			response[BUFSIZE-1]='\0';
			*nresponsebytes=strlen(response);
			return 0;
		}
	}
	catch(boost::interprocess::interprocess_exception &ex)
	{
		//std::cout << ex.what() << std::endl;
		strncpy(response,"pgexodus failure to open message_queue '",BUFSIZE);
		strncat(response,requestqueuename.c_str(),BUFSIZE);
		strncat(response,"' ",BUFSIZE);
		strncat(response,ex.what(),BUFSIZE);
		response[BUFSIZE-1]='\0';
		*nresponsebytes=strlen(response);
		return 0;
	}

	/***  get response ***/

	try
	{
		//open the response queue
		boost::interprocess::message_queue response_queue
		(
			boost::interprocess::open_only		//only open existing
			,responsequeuename.c_str()		//name TODO ensure unique
		);

		//receive the response
		unsigned int priority;
		std::size_t received_size;
		char chBuf[BUFSIZE];
		try
		{
			//elog(WARNING, "callexodus: receiving response");
			response_queue.receive(chBuf, BUFSIZE, received_size, priority);
		}

		//handle failure to receive
		catch(boost::interprocess::interprocess_exception &ex)
		{
			//std::cout << ex.what() << std::endl;
			strncpy(response,"pgexodus failure to receive message_queue '",BUFSIZE);
			strncat(response,responsequeuename.c_str(),BUFSIZE);
			strncat(response,"' ",BUFSIZE);
			strncat(response,ex.what(),BUFSIZE);
			response[BUFSIZE-1]='\0';
			*nresponsebytes=strlen(response);
			return 0;
		}

		//handle any other failure to receive
		catch(...)
		{
			//std::cout << ex.what() << std::endl;
			strncpy(response,"pgexodus unknown failure to receive message_queue '",BUFSIZE);
			strncat(response,responsequeuename.c_str(),BUFSIZE);
			response[BUFSIZE-1]='\0';
			*nresponsebytes=strlen(response);
			return 0;
		}

		//return the size of the response
		*nresponsebytes=received_size;		// number of bytes read 

		//copy the received data to the response
		memcpy(response,	// destination
			chBuf,	// starting from
			*nresponsebytes);		// how many bytes

		//return successfully with response
		return 1;

	}

	//handle failure to open response queue
	catch(boost::interprocess::interprocess_exception &ex)
	{
		//std::cout << ex.what() << std::endl;
		strncpy(response,"pgexodus failure to open message_queue '",BUFSIZE);
		strncat(response,responsequeuename.c_str(),BUFSIZE);
		strncat(response,"' ",BUFSIZE);
		strncat(response,ex.what(),BUFSIZE);
		response[BUFSIZE-1]='\0';
		*nresponsebytes=strlen(response);
		return 0;
	}

	//handle any other uncaught failures
	catch (...)
	{
		strncpy(response,"pgexodus unknown failure",BUFSIZE);
		response[BUFSIZE-1]='\0';
		*nresponsebytes=strlen(response);
		return 0;
	}

}

}// extern "C"

#endif //EXODUS_IPC_BOOST

