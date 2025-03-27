//
// Copyright (c) 2010 steve.bush@neosys.com
//
// osfile var var_str holds the file name as opened
//
// osfile var_int is the file number as provided by fopen/read/write etc.
//
// var_typ has a special bit set to indicate that the int holds the fileno
// but the non-numeric bit will also be set to prevent the int being used
// for ordinary numerical functions

#ifndef EXODUS_LIBEXODUS_EXODUS_VAROSHANDLE_H_
#define EXODUS_LIBEXODUS_EXODUS_VAROSHANDLE_H_

#if EXO_MODULE
	import std;
#else
//#	include <cstdlib>
//#	include <cassert>
#	include <string>
#	include <vector>
#endif

#include <exodus/var.h>

namespace exo {

using CACHED_HANDLE = void*;
using DELETER_AND_DESTROYER = void (*)(CACHED_HANDLE);

class VarOSHandleEntry {
   public:
	VarOSHandleEntry();
	DELETER_AND_DESTROYER deleter;	// =0 means that slot is empty
	CACHED_HANDLE handle;
	std::string extra;
};

class VarOSfstreamHandles {
   public:
	VarOSfstreamHandles();
	int add_handle(CACHED_HANDLE handle_to_opened_file, DELETER_AND_DESTROYER del, std::string name);
	// VarOSHandleEntry & operator [] (int idx)
	//{
	//	return conntbl[ idx];
	//}
	CACHED_HANDLE get_handle(int index, std::string name);
	void del_handle(int index);
	virtual ~VarOSfstreamHandles();

   private:
	std::vector<VarOSHandleEntry> conntbl;
};

#ifndef INSIDE_VAR_OSHANDLE_CPP
extern
#endif								  // INSIDE_VAR_OSHANDLE_CPP
	thread_local VarOSfstreamHandles thread_fstream_handles;  // global table (intended usage: mvos.cpp and vardb.cpp)

}  // namespace exo

#endif  // EXODUS_LIBEXODUS_EXODUS_VAROSHANDLE_H_
