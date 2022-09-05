//
// Copyright (c) 2010 steve.bush@neosys.com
//
#include <algorithm>
#include <cassert>

#define INSIDE_VAR_OSHANDLE_CPP  // global obj in "varoshandle.h"
#include "varoshandle.h"

#include <mutex>
std::mutex mvhandles_mutex;

#define HANDLES_CACHE_SIZE 3

namespace exodus {

VarOSHandleEntry::VarOSHandleEntry()
	: deleter((DELETER_AND_DESTROYER)0), handle(0) {}

VarOSHandlesCache::VarOSHandlesCache()
	: conntbl(HANDLES_CACHE_SIZE) {}

int VarOSHandlesCache::add_handle(CACHED_HANDLE handle_to_cache, DELETER_AND_DESTROYER del, std::string name) {
	assert(del);
	// No longer need locking since mv_handlescache is thread_local
	//std::lock_guard lock(mvhandles_mutex);

	int ix;
	for (ix = 0; ix < (int)conntbl.size(); ix++)
		if (conntbl[ix].deleter == nullptr)
			break;

	if (ix == (int)conntbl.size())
		conntbl.resize(ix * 2);	 // double the table size

	conntbl[ix].deleter = del;
	conntbl[ix].handle = handle_to_cache;
	conntbl[ix].extra = name;
	return ix;
}

CACHED_HANDLE VarOSHandlesCache::get_handle(int index, std::string name) {
	//std::lock_guard lock(mvhandles_mutex);
	return (conntbl[index].deleter == nullptr) || (conntbl[index].extra != name)
			   ? nullptr
			   : conntbl[index].handle;
}

void VarOSHandlesCache::del_handle(int index) {
	//std::lock_guard lock(mvhandles_mutex);
	assert(conntbl[index].deleter);
	if (conntbl[index].deleter) {
		conntbl[index].deleter(conntbl[index].handle);
		//conntbl[index].handle;
	}
	conntbl[index].deleter = 0;	 //	nullptr
}

VarOSHandlesCache::~VarOSHandlesCache() {

	// crashes on linux and multithreaded destructor cant exist so  dont lock
	// std::lock_guard lock(mvhandles_mutex);

	int ix;
	for (ix = 0; ix < (int)conntbl.size(); ix++)
		if (conntbl[ix].deleter != nullptr) {
			// do not call 'del_handle(ix)' here because of deadlock
			conntbl[ix].deleter(conntbl[ix].handle);
			conntbl[ix].deleter = 0;  //	nullptr
		}
}

}  // namespace exodus
