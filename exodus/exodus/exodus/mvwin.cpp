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

	DWORD nSize=2048;			//ALN:TODO: true but not ideal, ... try to move to classic code
	TCHAR filename[2048];

	int bytes = GetModuleFileName(NULL, filename, nSize);
	if(bytes == 0)
		return "";
	else
	{
		//make a string of TCHAR
		typedef std::basic_string<TCHAR> tstring;
		tstring tempstr(filename,bytes);	//ALN:TODO: make this conversion as standard, or:
											//ALN:TODO:up to explicit operator = (TCHAR)
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
