#ifdef _MSC_VER

#include <exodus/mvconfig.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <exodus/var.h>

namespace exodus {

var getprocessn() {
	// DWORD WINAPI
	return var((int)GetCurrentProcessId());
}

var getexecpath() {
	// this might not convert windows characters to exodus characters properly but
	// if compiled with unicode options it is probably ok since TCHAR will be wide

	DWORD nSize = MAX_PATH;
	TCHAR filename[MAX_PATH];

	int bytes = GetModuleFileName(nullptr, filename, nSize);
	if (bytes == 0)
		return L"";
	else {
		// move to a standardised global function to allow usage
		// by other win functions?
		// the job here is to convert in a dumb way from win TCHAR to var wchar_t

		// make a string of TCHAR
		using tstring = std::basic_string<TCHAR>;
		tstring tempstr(filename, bytes);
		// transfer the TCHARS into a wstring
		std::wstring wtempstr;
		for (unsigned int ii = 0; ii < tempstr.size(); ++ii)
			wtempstr.push_back((unsigned int)(tempstr[ii]));

		// which we can return to the
		return wtempstr;
	}
}

}  // namespace exodus

#endif
