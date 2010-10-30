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

var getexecpath()
{

	DWORD nSize=2048;
	TCHAR filename[2048];

	int bytes = GetModuleFileName(NULL, filename, nSize);
	if(bytes == 0)
		return "";
	else
	{
		//make a string of TCHAR
		typedef std::basic_string<TCHAR> tstring;
		tstring tempstr(filename,bytes);

		//transfer the TCHARS into a wstring
		std::wstring wtempstr;
		for (unsigned int ii=0;ii<tempstr.length();++ii)
			wtempstr.push_back((unsigned int)(tempstr[ii]));

		//which we can return to the 
		return wtempstr;
	}

}

}//of namespace exodus

#endif
