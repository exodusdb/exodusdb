//
// Copyright (c) 2010 steve.bush@neosys.com
//
#if EXO_MODULE > 1
	import std;
#else
#	include <cstring>
//#	include <algorithm>
#	include <mutex>
#endif

#define INSIDE_VAR_OSHANDLE_CPP  // global obj in "varoshandle.h"
#include "varoshandle.h"

// TODO double check if this should be thread_local
std::mutex mvhandles_mutex;

#define HANDLES_CACHE_SIZE 3

namespace exo {

VarOSHandleEntry::VarOSHandleEntry()
	//: deleter((DELETER_AND_DESTROYER)0), handle(0) {}
	: deleter(static_cast<DELETER_AND_DESTROYER>(nullptr)), handle(nullptr) {}

VarOSfstreamHandles::VarOSfstreamHandles()
	: conntbl(HANDLES_CACHE_SIZE) {}

int VarOSfstreamHandles::add_handle(CACHED_HANDLE handle_to_cache, DELETER_AND_DESTROYER del, std::string name) {

	if (!del)
		throw VarDBException(std::string(__PRETTY_FUNCTION__).append(" del is missing."));

	// No longer need locking since mv_handlescache is thread_local
	//std::lock_guard lock(mvhandles_mutex);

	int ix;
	for (ix = 0; ix < static_cast<int>(conntbl.size()); ix++)
		if (conntbl[ix].deleter == nullptr)
			break;

	if (ix == static_cast<int>(conntbl.size()))
		conntbl.resize(ix * 2);	 // double the table size

	conntbl[ix].deleter = del;
	conntbl[ix].handle = handle_to_cache;
	conntbl[ix].extra = name;
	return ix;
}

CACHED_HANDLE VarOSfstreamHandles::get_handle(int index, std::string name) {
	//std::lock_guard lock(mvhandles_mutex);
	return (conntbl[index].deleter == nullptr) || (conntbl[index].extra != name)
			   ? nullptr
			   : conntbl[index].handle;
}

void VarOSfstreamHandles::del_handle(int index) {
	//std::lock_guard lock(mvhandles_mutex);
	if (!conntbl[index].deleter)
		throw VarDBException(std::string(__PRETTY_FUNCTION__).append(" deleter is missing."));
	if (conntbl[index].deleter) {
		conntbl[index].deleter(conntbl[index].handle);
		//conntbl[index].handle;
	}
	conntbl[index].deleter = nullptr;
}

VarOSfstreamHandles::~VarOSfstreamHandles() {

	// crashes on linux and multithreaded destructor cant exist so  dont lock
	// std::lock_guard lock(mvhandles_mutex);

	int ix;
	for (ix = 0; ix < static_cast<int>(conntbl.size()); ix++)
		if (conntbl[ix].deleter != nullptr) {
			// do not call 'del_handle(ix)' here because of deadlock
			conntbl[ix].deleter(conntbl[ix].handle);
			conntbl[ix].deleter = nullptr;
		}
}

}  // namespace exo
