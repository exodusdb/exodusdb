#include <exodus/mvconfig.h>
#ifdef EXODUS_IPC_POSIX

//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/stat.h> //for umask
#include <errno.h>

//NB errorno and strerror is not thread safe but perhaps not a problem
//since it is errors with wrong configuration and not normal usage

#include <iostream>
using namespace std;

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

void respondToRequests(int sock, const std::string& socketpath)
{

	unsigned int priority;
	socklen_t fromlen;

	char chRequest[BUFSIZE];
	//char chResponse[BUFSIZE];
	//int nresponsebytes;
	std::string response;

	while (true)
	{

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc: accepting on socket %d\n",sock);
		#endif
		
		/*server waits for a connection*/
		struct sockaddr_in from;
		fromlen = sizeof(from);
		listen(sock,1);
		int sock2;
		if ((sock2=accept(sock, (struct sockaddr *) &from, &fromlen))<0)
		{
			std::cout << "mvipc: error " << errno << " " << strerror(errno) << " accepting socket " << socketpath << std::endl;
			continue;
		}

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc: reading %d byte size from socket %d\n",sizeof(int),sock2);
		#endif
		
		int nn;

		if ((nn=read(sock2,chRequest,sizeof(int)))<0)
		{
			std::cout << "mvipc: error " << errno << " " << strerror(errno) << " reading" << sizeof(int) << " byte size from socket " << socketpath << std::endl;
			continue;
		}

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc() read  %d bytes from socket\n",nn);
		#endif
		
		int request_size=62;//((int)chRequest)+sizeof(int);

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc: reading %d more bytes from data socket\n",request_size-sizeof(int));
		#endif
		
		if ((nn=read(sock2,chRequest+sizeof(int),request_size-sizeof(int)))<0)
		{
			std::cout << "mvipc: error " << errno << " " << strerror(errno) << " reading data from socket " << socketpath << std::endl;
			continue;
		}

		//log
		#if TRACING >= 3
			wprintf(L"---------------------------------\nMVipc() read  %d more bytes from socket\n",nn);
		#endif
		
		//determine a response
		getResponseToRequest(chRequest,request_size,0,response);

		//send a response
		if (write(sock2,response.data(),response.length())<0)
		{
			std::cout << "mvipc: error" << errno << " " << strerror(errno) << " writing socket " << socketpath << std::endl;
			continue;
		}

		close(sock2);

		//log
		#if TRACING >= 3
			wprintf(L"MVipc() wrote %d bytes <- \"\"\n",response.length());
		#endif

	}
}

void closeipcqueues(int sock, std::string& socketpath)
{
	if (sock)
		close(sock);
	unlink(socketpath.c_str());
}

//this function is started as a thread by startipc()
int MVipc(const int environmentn, var& pgconnparams)
{

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
		
	/*create the socket directory*/
	std::string socketdir="/tmp/exodus";
	if (!var(socketdir).osdir())
		var(socketdir).osmkdir();

	std::string socketpath=socketdir+"/ipc";
	socketpath+="exodusservice-"+processn.tostring()+"."+var(environmentn).tostring();

	closeipcqueues(0,socketpath);


	int sock;
	int fromlen;
	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	/*grant owner/group/others rw access to the socket*/
	/*TODO review security of this*/
	mode_t oldumask=umask(0111);

	/*server binds to the socket*/
	struct sockaddr_un addr;
	strncpy(addr.sun_path, socketpath.c_str(),sizeof(addr.sun_path));
	addr.sun_family = AF_UNIX;
	if (bind (sock, (struct sockaddr *) &addr, strlen(addr.sun_path) + sizeof (addr.sun_family))<0)
	{
		boost::mutex::scoped_lock lock(global_ipcmutex);
		global_ipccondition.notify_one();
		std::cout << "cannot open " << socketpath << " " << errno << " " << strerror(errno) << std::endl;
		closeipcqueues(sock, socketpath);
		return 0;
	}

	/*restore permissions mask*/
	umask(oldumask);

	//log
	#if TRACING >= 3
		wprintf(L"---------------------------------\nMVipc: bound socket %s\n",socketpath.c_str());
	#endif
		
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

	respondToRequests(sock,socketpath);
	
	std::cout << "stopped responding to socket " << socketpath << std::endl;

	closeipcqueues(sock, socketpath);

	return 1;
}


}//namespace exodus

#endif //EXODUS_IPC_POSIX

