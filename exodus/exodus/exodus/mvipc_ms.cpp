//#if defined(_WIN32) || defined(_WIN64)  || defined(_MINGW)
#include <exodus/mvconfig.h>
#ifdef EXODUS_IPC_WINPIPES

#ifndef DEBUG
#define TRACING 0
#else
#define TRACING 5
#endif

#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include <stdio.h>
//#include <tchar.h>
#include <string>

//
//#define MV_NO_NARROW

#define EXODUS_IPC_EXTERN extern
#include <exodus/mvipc.h>

//#include "TConvert.h"//for creation of LPTSTR

//for documentation see end of file

//allow deprecated mbstowcs instead of "safe" mbstowcs_s
#pragma warning ( disable : 4995 4996)

//for mingw
#ifdef _MSC_VER
#include <strsafe.h>
#else
#include <aclapi.h>
#include <ddk\ntifs.h>
#endif

//actually limited to 256 characters long according to MS
#define MAXPIPENAMESIZE 1024

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
//neosys only one instance
//#define INSTANCES 4 
#define INSTANCES 1
#define PIPE_TIMEOUT 5000
#define BUFSIZE 1048576
 
namespace exodus
{

//open a pipe with the right environment
//to answer any requests from postgres server to evaluate data

typedef struct
{
   OVERLAPPED oOverlap;
   HANDLE hPipeInst;
   TCHAR chRequest[BUFSIZE];
   DWORD cbRead;
   TCHAR chReply[BUFSIZE];
   DWORD cbToWrite; 
   DWORD dwState; 
   BOOL fPendingIO; 
} PIPEINST, *LPPIPEINST; 
 
VOID DisconnectAndReconnect(DWORD); 
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED); 
 
PIPEINST Pipe[INSTANCES]; 
HANDLE hEvents[INSTANCES];

int MVipc(const int environmentn, var& pgconnparams)
{

	//TODO

	//flag to connectlocal NOT to be recursive and open yet another ipc thread
	tss_ipcstarted.reset(new bool(true));

	//clone the postgres connection because the parent thread is running a select with it
	if (!var().connect(pgconnparams))
	{
		throw var(L"MVipc Cannot connect additional thread to postgres");
		return false;
	}
	//TODO prevent or handle SELECT in dictionary functions

	//set the threads environment number (same as and provided by the parent thread)
	//AFTER opening the database connection
	setenvironmentn(environmentn);
	
	//"\\\\.\\pipe\\exoduspipexyz"
	//strings of MS tchars
	//typedef basic_string<TCHAR> tstring;
	//wchar_t* exoduspipename="\\\\.\\pipe\\exoduspipexyz";
	var exoduspipename=L"\\\\.\\pipe\\exodusservice-";
	exoduspipename^= getprocessn()^"."^environmentn;
	std::wstring wexoduspipename=exoduspipename.towstring();
	LPTSTR lpszPipename = (LPTSTR) wexoduspipename.c_str();

	//create a LPTSTR holding object that will automatically delete then LPTSTR when goes out of scope
//	_totchar texoduspipename(texoduspipename.towstring().c_str());
	//mbstowcs(texoduspipename, (LPTSTR)exoduspipename.toString().c_str(), wcslen(exoduspipename.toString().c_str())+1);
	//wcscpy(texoduspipename, (LPTSTR)exoduspipename.toString().c_str());

	// declare and initialize a security attributes structure
	SECURITY_ATTRIBUTES SA;
	ZeroMemory( &SA, sizeof(SA) );
	SA.nLength = sizeof( SA );
	SA.bInheritHandle = FALSE; // object uninheritable

	// declare and initialize a security descriptor
	SECURITY_DESCRIPTOR SD;

	BOOL bInitOk = InitializeSecurityDescriptor( &SD,
                         SECURITY_DESCRIPTOR_REVISION );

	if ( bInitOk )
	{
   	// give the security descriptor a Null Dacl
  	 // done using the  "TRUE, (PACL)NULL" here
   	BOOL bSetOk = SetSecurityDescriptorDacl( &SD,
                                            TRUE,
                                            (PACL)NULL,
                                            FALSE );
		if (bSetOk)
		{

			// Make the security attributes point
			// to the security descriptor
			SA.lpSecurityDescriptor = &SD;

		}

	};


   DWORD i, dwWait, cbRet, dwErr; 
   BOOL fSuccess;

// The initial loop creates several instances of a named pipe 
// along with an event object for each instance.  An 
// overlapped ConnectNamedPipe operation is started for 
// each instance. 
 
   for (i = 0; i < INSTANCES; i++) 
   { 
 
   // Create an event object for this instance. 
 
      hEvents[i] = CreateEvent( 
         NULL,    // default security attribute 
         TRUE,    // manual-reset event 
         TRUE,    // initial state = signaled 
         NULL);   // unnamed event object 

      if (hEvents[i] == NULL) 
      {
		  wprintf(L"MVipc() CreateEvent failed. WIN32 Error: %d.\n", GetLastError()); 
         return false;
      }
 
      Pipe[i].oOverlap.hEvent = hEvents[i]; 
 
      Pipe[i].hPipeInst = CreateNamedPipe( 
         lpszPipename,            // pipe name 
         PIPE_ACCESS_DUPLEX |     // read/write access 
         FILE_FLAG_OVERLAPPED,    // overlapped mode 
         PIPE_TYPE_MESSAGE |      // message-type pipe 
         PIPE_READMODE_MESSAGE |  // message-read mode 
         PIPE_WAIT,               // blocking mode 
         INSTANCES,               // number of instances 
         BUFSIZE*sizeof(TCHAR),   // output buffer size 
         BUFSIZE*sizeof(TCHAR),   // input buffer size 
         PIPE_TIMEOUT,            // client time-out 
	&SA);

	if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) 
	{
		if (GetLastError()==231)
		{
			wprintf(L"MVipc() CreateNamedPipe failed because pipe %s is busy. Is another server running?",lpszPipename);
			return false;
		}
		wprintf(L"MVipc() CreateNamedPipe %s failed. WIN32 Error: %d.\n", lpszPipename, GetLastError());
		return false;
	}
	else
#if TRACING >= 1
		wprintf(L"MVipc() CreateNamedPipe %s succeeded\n", lpszPipename);
#endif 
   // Call the subroutine to connect to the new client
 
      Pipe[i].fPendingIO = ConnectToNewClient( 
         Pipe[i].hPipeInst, 
         &Pipe[i].oOverlap); 
 
      Pipe[i].dwState = Pipe[i].fPendingIO ? 
         CONNECTING_STATE : // still connecting 
         READING_STATE;     // ready to read 
   } 
 
	//indicate to waiting/paused parent thread that the pipe is open
	//(the pipe is not actually waiting until the next step)
	//scoped so that the scoped_lock is automatically released after the notification
	{
		boost::mutex::scoped_lock lock(global_ipcmutex);
#if TRACING >= 1
		std::cout<<L"MVipc() Notifying that pipe has been opened\n";
#endif
		//TODO make sure notifies CORRECT parent thread by using an array of ipcmutexes and tss_environmentn
		global_ipccondition.notify_one();
#if TRACING >= 1
		std::wcout<<L"MVipc() Notified that pipe has been opened\n";
#endif
	}

   while (1) 
   { 
   // Wait for the event object to be signaled, indicating 
   // completion of an overlapped read, write, or 
   // connect operation. 
 
      dwWait = WaitForMultipleObjects( 
         INSTANCES,    // number of event objects 
         hEvents,      // array of event objects 
         FALSE,        // does not wait for all 
         INFINITE);    // waits indefinitely 
 
   // dwWait shows which pipe completed the operation. 
 
      i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
      if (i < 0 || i > (INSTANCES - 1)) 
      {
         wprintf(L"MVipc() Index out of range.\n"); 
         return false;
      }
 
   // Get the result if the operation was pending. 
 
      if (Pipe[i].fPendingIO) 
      { 
         fSuccess = GetOverlappedResult( 
            Pipe[i].hPipeInst, // handle to pipe 
            &Pipe[i].oOverlap, // OVERLAPPED structure 
            &cbRet,            // bytes transferred 
            FALSE);            // do not wait 
 
         switch (Pipe[i].dwState) 
         { 
			// Pending connect operation 
            case CONNECTING_STATE: 
               if (! fSuccess) 
               {
                   wprintf(L"MVipc() CONNECTING STATE WIN32 Error %d.\n", GetLastError()); 
                   return false;
               }
               Pipe[i].dwState = READING_STATE; 
               break;
 
			// Pending read operation
            case READING_STATE: 
               if (! fSuccess || cbRet == 0) 
               { 
                  DisconnectAndReconnect(i); 
                  continue; 
               } 
               Pipe[i].dwState = WRITING_STATE; 
               break; 
 
			// Pending write operation 
            case WRITING_STATE: 
               if (! fSuccess || cbRet != Pipe[i].cbToWrite) 
               { 
                  DisconnectAndReconnect(i); 
                  continue; 
               } 
               Pipe[i].dwState = READING_STATE; 
               break; 
 
            default: 
               wprintf(L"MVipc() (1) Invalid pipe state %d\n", Pipe[i].dwState); 
               return false;
         }  
      } 
 
	// The pipe state determines which operation to do next. 
	std::string response;
 
      switch (Pipe[i].dwState) 
      { 
      // READING_STATE: 
      // The pipe instance is connected to the client 
      // and is ready to read a request from the client. 
 
         case READING_STATE: 
            fSuccess = ReadFile( 
               Pipe[i].hPipeInst, 
               Pipe[i].chRequest, 
               BUFSIZE*sizeof(TCHAR), 
               &Pipe[i].cbRead, 
               &Pipe[i].oOverlap); 

			// The read operation completed successfully.
            if (fSuccess && Pipe[i].cbRead != 0) 
            { 
#if TRACING >= 3
wprintf(L"---------------------------------\nMVipc() read  %d bytes from pipe\n",Pipe[i].cbRead);
#endif
               Pipe[i].fPendingIO = FALSE; 
               Pipe[i].dwState = WRITING_STATE; 
               continue; 
            } 
 
			// The read operation is still pending. 
            dwErr = GetLastError(); 
            if (! fSuccess && (dwErr == ERROR_IO_PENDING)) 
            { 
               Pipe[i].fPendingIO = TRUE; 
               continue; 
            } 
 
			// An error occurred; disconnect from the client.
            DisconnectAndReconnect(i); 
            break; 
 
		// WRITING_STATE: 
		// The request was successfully read from the client. 
		// Get the reply data and write it to the client. 
		case WRITING_STATE: 

//neosys CLOSE PIPE MESSAGE
			 if (0==lstrcmp(Pipe[i].chReply,L"NEOSYS_CLOSE_THIS_PIPE"))
//			 if (0==lstrcmp(Pipe[i].chReply,TEXT(L"")))
			 {
				 return 0;
			 }

			getResponseToRequest((char*)Pipe[i].chRequest, (size_t) Pipe[i].cbRead, BUFSIZ*sizeof(TCHAR), response);
	/*
			//bytes to write to pipe
			memcpy(pipe->chReply,reply.towstring().data(),nbytes);
			//number of bytes to write to pipe
			pipe->cbToWrite=int(reply.length())*sizeof(TCHAR);
	*/

			//bytes to write to pipe
			memcpy(Pipe[i].chReply,response.data(),response.length());
			//number of bytes to write to pipe
			Pipe[i].cbToWrite=int(response.length());

            fSuccess = WriteFile( 
               Pipe[i].hPipeInst, 
               Pipe[i].chReply, 
               Pipe[i].cbToWrite, 
               &cbRet, 
               &Pipe[i].oOverlap); 

#if TRACING >= 3
//wprintf(L"MVipc() wrote %d bytes <- \"%s\"\n",Pipe[i].cbToWrite, Pipe[i].chReply);
wprintf(L"MVipc() wrote %d bytes <- \"%s\"\n",Pipe[i].cbToWrite, Pipe[i].chReply);
#endif
			// The write operation completed successfully. 
			if (fSuccess && cbRet == Pipe[i].cbToWrite) 
			{ 
               Pipe[i].fPendingIO = FALSE; 
               Pipe[i].dwState = READING_STATE; 
               continue; 
            } 
 
			// The write operation is still pending.
            dwErr = GetLastError(); 
            if (! fSuccess && (dwErr == ERROR_IO_PENDING)) 
            { 
               Pipe[i].fPendingIO = TRUE; 
               continue; 
            } 
 
			// An error occurred; disconnect from the client.
wprintf(L"MVipc() An error occurred; disconnect and reconnect to the client.");
            DisconnectAndReconnect(i); 
            break; 
 
         default: 
            wprintf(L"MVipc() (2) Invalid pipe state.\n"); 
            return false;
      } 
  }
 
  return 0;
}
 
// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 
 
VOID DisconnectAndReconnect(DWORD i) 
{ 
// Disconnect the pipe instance. 
 
   if (! DisconnectNamedPipe(Pipe[i].hPipeInst) ) 
   {
      wprintf(L"MVipc-DisconnectAndReconnect() DisconnectNamedPipe failed with %d.\n", GetLastError());
   }
 
// Call a subroutine to connect to the new client. 
 
   Pipe[i].fPendingIO = ConnectToNewClient( 
      Pipe[i].hPipeInst, 
      &Pipe[i].oOverlap); 
 
   Pipe[i].dwState = Pipe[i].fPendingIO ? 
      CONNECTING_STATE : // still connecting 
      READING_STATE;     // ready to read 
} 
 
// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 
 
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) 
{ 
   BOOL fConnected, fPendingIO = FALSE; 
 
// Start an overlapped connection for this pipe instance. 
   fConnected = ConnectNamedPipe(hPipe, lpo); 
 
// Overlapped ConnectNamedPipe should return zero. 
   if (fConnected) 
   {
      wprintf(L"MVipc-ConnectToNewClient() (1) ConnectNamedPipe failed. WIN32 Error %d.\n", GetLastError()); 
      return 0;
   }
 
   switch (GetLastError()) 
   {
		// The overlapped connection in progress. 
		case ERROR_IO_PENDING: 
         fPendingIO = TRUE; 
         break; 
 
		// Client is already connected, so signal an event. 
		case ERROR_PIPE_CONNECTED: 
			if (SetEvent(lpo->hEvent)) 
				break; 
 
		// If an error occurs during the connect operation... 
		default: 
         wprintf(L"MVipc-ConnectToNewClient() (2) ConnectNamedPipe failed. WIN32 Error %d.\n", GetLastError());
         return 0;
   }
 
   return fPendingIO; 
}

/* from http://msdn2.microsoft.com/EN-US/library/aa365603.aspx

Named Pipe Server Using Overlapped I/O

The following is an example of a single-threaded pipe server that uses overlapped operations to service simultaneous connections to multiple pipe clients. The pipe server creates a fixed number of pipe instances. Each pipe instance can be connected to a separate pipe client. When a pipe client has finished using its pipe instance, the server disconnects from the client and reuses the pipe instance to connect to a new client. This pipe server can be used with the pipe client described in Named Pipe Client.

The OVERLAPPED structure is specified as a parameter in each ReadFile, WriteFile, and ConnectNamedPipe operation on the pipe instance. Although the example shows simultaneous operations on different pipe instances, it avoids simultaneous operations on a single pipe instance by using the event object in the OVERLAPPED structure. Because the same event object is used for read, write, and connect operations for each instance, there is no way to know which operation's completion caused the event to be set to the signaled state for simultaneous operations using the same pipe instance.

The event handles for each pipe instance are stored in an array that is passed to the WaitForMultipleObjects function. This function waits for one of the events to be signaled, and returns the array index of the event that caused the wait operation to complete. The example in this topic uses this array index to retrieve a structure containing information for the pipe instance. The server uses the fPendingIO member of the structure to keep track of whether the most recent I/O operation on the instance was pending, which requires a call to the GetOverlappedResult function. The server uses the dwState member of the structure to determine the next operation that must be performed for the pipe instance.

Overlapped ReadFile, WriteFile, and ConnectNamedPipe operations can finish by the time the function returns. Otherwise, if the operation is pending, the event object in the specified OVERLAPPED structure is set to the nonsignaled state before the function returns. When the pending operation finishes, the system sets the state of the event object to signaled. The state of the event object is not changed if the operation finishes before the function returns.

Because the example uses manual-reset event objects, the state of an event object is not changed to nonsignaled by the WaitForMultipleObjects function. This is important, because the example relies on the event objects remaining in the signaled state, except when there is a pending operation.

If the operation has already finished when ReadFile, WriteFile, or ConnectNamedPipe returns, the function's return value indicates the result. For read and write operations, the number of bytes transferred is also returned. If the operation is still pending, the ReadFile, WriteFile, or ConnectNamedPipe function returns zero and the GetLastError function returns ERROR_IO_PENDING. In this case, use the GetOverlappedResult function to retrieve the results after the operation has finished. GetOverlappedResult returns only the results of pending operations. It does not report the results of operations that were completed before the overlapped ReadFile, WriteFile, or ConnectNamedPipe function returned.

Before disconnecting from a client, you must wait for a signal indicating the client has finished. (Flushing the file buffers would defeat the purpose of overlapped I/O, because the flush operation would block the execution of the server thread while it waits for the client to empty the pipe.) In this example, the signal is the error generated by trying to read from the pipe after the pipe client closes its handle.

*/

}//of namespace neo

#endif //EXODUS_IPC_WINPIPES
