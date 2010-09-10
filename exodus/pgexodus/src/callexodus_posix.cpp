#include "mvconfig.h"
#ifdef EXODUS_IPC_POSIX

//could be .c instead of .cpp actually

//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>

//#include <winsock2.h>
//#include <ws2tcpip.h>

#include <string>

/* insert debug code like this
strncpy(response,"opened",BUFSIZE);
response[BUFSIZE-1]='\0'
nresponsebytes=strlen(response);
return 1;
*/

//evade warnings about using strncpy strncat on msvc
#define _CRT_SECURE_NO_WARNINGS

//needs to be linkable to C functions
extern "C" {

////////////TODO: NEEDS FIXING!!! limited by stack size of around 1mb for global functions in the exodus client (convert to classes to overcome)
#define BUFSIZE 262144

bool callexodus(const char* serverid, const char* request, const int nrequestbytes, char* response, int* nresponsebytes)
{

	std::string socketpath="/tmp/exodus/ipc";
	socketpath+=serverid;

	/*prepare a socket*/
    int s;
    int newsock;
    int fromlen;
    s = socket(AF_UNIX, SOCK_STREAM, 0);

	/*connect to the socket or fail*/
	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, socketpath.c_str(), sizeof(server.sun_path));
	if (connect(s, (struct sockaddr *)&server, strlen(server.sun_path) + sizeof (server.sun_family))<0) {
		sprintf(response,"pgexodus failure '%d' to connect to socket '%s'",errno,socketpath.c_str());
		return 0;
		//exit(2);
		//std::cout << ex.what() << std::endl;
		strncpy(response,"pgexodus failure to connect socket '",BUFSIZE);
		strncat(response,socketpath.c_str(),BUFSIZE);
		strncat(response,"' ",BUFSIZE);
		response[BUFSIZE-1]='\0';
		return 0;
    }

	/*send the request*/
    if (write(s,request,nrequestbytes)<0)
	{
		sprintf(response,"pgexodus failure '%d' writing socket '%s'",errno,socketpath.c_str());
		return 0;
	}

	//sprintf(response,"pgexodus debug point 1 %d - %s",nrequestbytes);
	//return 0;

	/*get response*/
	if (read(s,response,BUFSIZE)<0)
	{
		sprintf(response,"pgexodus failure '%d' reading socket '%s'",errno,socketpath.c_str());
		return 0;
	}

	/*clean up*/
	close(s);

	return 1;
}

}// extern "C"

#endif //EXODUS_IPC_BOOST

