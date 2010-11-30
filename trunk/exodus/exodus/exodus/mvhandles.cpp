//
// Copyright (c) 2010 Neosys
//
#include <algorithm>
#include <cassert>
#define INSIDE_MVHANDLES_CPP	// global obj in "mvhandles.h"
#include "mvhandles.h"

#include <boost/thread/mutex.hpp>
boost::mutex mvhandles_mutex;

#define HANDLES_CACHE_SIZE	3

namespace exodus {

	MvHandleEntry::MvHandleEntry()
		: deleter(0)//	HANDLE_ENTRY_FREE
	{}
MvHandlesCache::MvHandlesCache()
	: tbl( HANDLES_CACHE_SIZE)
{}

int MvHandlesCache::add_handle( CACHED_HANDLE handle_to_cache, DELETER_AND_DESTROYER del, std::wstring name)
{
	assert( del);
	boost::mutex::scoped_lock lock(mvhandles_mutex);

	int ix;
	for( ix = 0; ix < ( int) tbl.size(); ix ++)
		if( tbl[ix].deleter == HANDLE_ENTRY_FREE)
			break;

	if( ix == ( int) tbl.size())
		tbl.resize( ix * 2);	// double the table size

	tbl[ix].deleter  = del;
	tbl[ix].handle = handle_to_cache;
	tbl[ix].extra  = name;
	return ix;
}

CACHED_HANDLE MvHandlesCache::get_handle( int index, std::wstring name)
{
	boost::mutex::scoped_lock lock(mvhandles_mutex);
	return ( tbl[index].deleter == HANDLE_ENTRY_FREE) || ( tbl[index].extra != name)
			? BAD_CACHED_HANDLE : tbl[index].handle;
}

void MvHandlesCache::del_handle( int index)
{
	boost::mutex::scoped_lock lock(mvhandles_mutex);
	assert( tbl[index].deleter);
	if( tbl[index].deleter)
		tbl[index].deleter( tbl[index].handle);
	tbl[index].deleter = 0;	//	HANDLE_ENTRY_FREE
}

MvHandlesCache::~MvHandlesCache()
{
	boost::mutex::scoped_lock lock(mvhandles_mutex);
	int ix;
	for( ix = 0; ix < ( int) tbl.size(); ix ++)
		if( tbl[ix].deleter != HANDLE_ENTRY_FREE)
		{
			// do not call 'del_handle( ix)' here because of deadlock
			tbl[ix].deleter( tbl[ix].handle);
			tbl[ix].deleter = 0;	//	HANDLE_ENTRY_FREE
		}
}

} // namespace exodus
