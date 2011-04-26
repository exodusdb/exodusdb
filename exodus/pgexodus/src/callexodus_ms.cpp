//#if defined(_WIN32) || defined(_WIN64)  || defined(_MINGW)
#include "mvconfig.h"
#ifdef EXODUS_IPC_WINPIPES

//"postgres uses 32 bit time whereas windows uses 64 bit time"
#define _USE_32BIT_TIME_T

#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include <stdio.h> //for sprintf
//#include <conio.h>
//#include <tchar.h>
#include <string>

//evade msvc warning on _snprintf
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 4996)

//////////// NEEDS FIXING!!!
#define BUFSIZE 1048576

//needs to be linkable to C functions
extern "C" {

//hack to prevent postgres' inclusion of windows header with template in c linkage (illegal)
//#define _WSPIAPI_H_

//just for postgres elog to do debugging //like //elog(WARNING, "callexodus: stage xxx");
//unfortunately gives an error
//error C2894: templates cannot be declared to have 'C' linkage	C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\include\wspiapi.h	44
//so we cannot use elog here
//#include <postgres.h>
//#include <fmgr.h>
//#include <utils\elog.h>

bool callexodus(const char* serverid, const char* request, const int nrequestbytes, char* response, int& nresponsebytes)
{

	//g++ asm(" int $03");

	//elog(WARNING, "callexodus: stage 01");
	//	strcpy(response,"callexodus");
	//elog(WARNING, "callexodus: stage 02");
	HANDLE hPipe; 
	//   LPTSTR lpvMessage=TEXT("Hello from postgres server function");
	TCHAR chBuf[BUFSIZE];
	BOOL fSuccess; 
	DWORD cbRead, cbWritten, dwMode; 
	////LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\exoduspipexyz"); 
	//TCHAR lpszPipename[4096];
	//memset(lpszPipename,0,4096);
	//strcat(lpszPipename,"\\\\.\\pipe\\exoduspipe"); 
	//strcat(lpszPipename,serverid);
	std::wstring pipename=L"\\\\.\\pipe\\";
	std::string str1=serverid;
	std::wstring wserverid=std::wstring(str1.begin(),str1.end());
	pipename+=wserverid;

	//pipe opening logic is roughly
	//1. Open an existing pipe using windows.h CreateFile
	//2. if fails then
	//3.  Abort with error if any error other than busy
	//4.  Wait for up to 10 seconds for the pipe to become ready and abort if it doesnt
	//5.  repeat from 1.
	while (1) 
	{
		//elog(WARNING, "callexodus: stage 03"); 
		// Try to open a named pipe; wait for it, if necessary. 
		hPipe = CreateFile(
			pipename.c_str(),   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 
 
		//elog(WARNING, "callexodus: stage 04");
		// Break if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE) 
			break;
 
		// Exit if an error other than ERROR_PIPE_BUSY occurs.

		//elog(WARNING, "callexodus: stage 05");
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			//elog(ERROR, "pgexodus callexodus() Could not open pipe %s. Windows Error No: %d",serverid, GetLastError());
			_snprintf(response,1000, "Could not open pipe %s. Windows Error No: %d",pipename.c_str(), GetLastError());
			nresponsebytes=strlen(response);
			return 0;
		}
 
		// All pipe instances are busy, so wait for some seconds and try again
 
		//elog(WARNING, "callexodus: stage 06");
		if (!WaitNamedPipe(pipename.c_str(), 10000)) 
		{
///			elog(ERROR, "pgexodus callexodus() Could not open pipe. WaitNamedPipe failed");
			_snprintf(response,1000, "Could not open pipe. WaitNamedPipe failed");
			nresponsebytes=strlen(response);
			return 0;
		}

   }
 
	// The pipe connected; change to message-read mode. 

	//elog(WARNING, "callexodus: stage 08");
	dwMode = PIPE_READMODE_MESSAGE; 
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 

	//elog(WARNING, "callexodus: stage 09");
	if (!fSuccess) 
	{
///		elog(ERROR,"pgexodus callexodus() SetNamedPipeHandleState failed"); 
		_snprintf(response,1000, "SetNamedPipeHandleState failed");
		nresponsebytes=strlen(response);
		return 0;
	}
 
	// Send a message to the pipe server. 
	//elog(WARNING, "callexodus: stage 10"); 
	fSuccess = WriteFile(
		hPipe,                  // pipe handle 
		(LPTSTR) request,             // message 
		nrequestbytes, // message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped

	//elog(WARNING, "callexodus: stage 11");
	if (!fSuccess) 
	{
///		elog(ERROR,"pgexodus callexodus() WriteFile failed");
		_snprintf(response,1000, "WriteFile failed");
		nresponsebytes=strlen(response);
		return 0;
	}
 
	do 
	{

		// Read from the pipe.
		//elog(WARNING, "callexodus: stage 12");
		fSuccess = ReadFile(
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			BUFSIZE,  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

		//elog(WARNING, "callexodus: stage 13"); 
		if (! fSuccess && GetLastError() != ERROR_MORE_DATA) 
			break; 
	 
		//_tprintf(TEXT("%s\n"), chBuf ); 

		//elog(WARNING, "callexodus: stage 14");
		//strcpy(response,chBuf);
		nresponsebytes=cbRead;  // number of bytes read 
		memcpy((void *) response,	// destination
			(void *) int(chBuf),	// starting from
			nresponsebytes);		// how many bytes

   } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

   //pause
   //getch();
 
	//elog(WARNING, "callexodus: stage 15");
	CloseHandle(hPipe); 

	//elog(WARNING, "callexodus: stage 16");    
	return 1;
}

}// extern "C"

#endif //windows pipes

