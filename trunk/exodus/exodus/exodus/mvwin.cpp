#include <exodus/mvconfig.h>
#ifndef EXODUS_POSIX

#define WIN32_LEAN_AND_MEAN
#include <windows.h> 

#include <exodus/mv.h>

namespace exodus {

var getprocessn()
{
	//DWORD WINAPI 
	return var((int) GetCurrentProcessId());
}

}//of namespace exodus

#endif
