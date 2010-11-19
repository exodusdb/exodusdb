//
// Copyright (c) 2010 Neosys
//
//
#include <algorithm>
#include "mvhandles.h"

#define HANDLES_CACHE_SIZE	3

namespace exodus {

MvHandlesCache::MvHandlesCache()
	: tbl( HANDLES_CACHE_SIZE)
{}

int MvHandlesCache::add_osfile( CACHED_HANDLE handle_to_opened_file)
{
	boost::mutex::scoped_lock lock(io_mutex);

	int ix;
	for( ix = 0; ix < ( int) tbl.size(); ix ++)
		if( tbl[ix].flags == HANDLE_ENTRY_FREE)
			break;

	if( ix == tbl.size())
	{
		int new_size = tbl.size() * 2;
		tbl.resize( new_size);	// adding lot of empty slots
//		ix = new_size - 1;		// fill last available slot, leaving free slots closer to beginning
	}

	tbl[ix].flags  = HANDLE_ENTRY_OSFILE;
	tbl[ix].handle = handle_to_opened_file;
	tbl[ix].extra  = 0;
	return ix;
}

CACHED_HANDLE MvHandlesCache::get_handle( int index)
{
	boost::mutex::scoped_lock lock(io_mutex);
	return tbl[index].flags == HANDLE_ENTRY_FREE ? BAD_CACHED_HANDLE : tbl[index].handle;
}

void MvHandlesCache::del_handle( int index)
{
	boost::mutex::scoped_lock lock(io_mutex);
	tbl[index].flags = HANDLE_ENTRY_FREE;
}

MvHandlesCache::~MvHandlesCache()
{
	boost::mutex::scoped_lock lock(io_mutex);
	int ix;
	for( ix = 0; ix < ( int) tbl.size(); ix ++)
		switch( tbl[ix].flags)
		{
		  case HANDLE_ENTRY_OSFILE:
			continue;
		  case HANDLE_ENTRY_FREE:
			continue;
		  default:
			continue;
		}
}

//int MvHandlesCache::add_osfile( int handle_to_opened_file)
//{
//	std::vector<MvHandleEntry>::iterator ix;
//	for( ix = tbl.begin(); ix != tbl.end(); ix ++)
//		if( ix->flags == HANDLE_ENTRY_FREE)
//			break;
//	if( ix == tbl.end())
//	{
//		int new_size = tbl.size() * 2;
//		tbl.resize( new_size);	// adding lot of empty slots
//		// let's fill last available slot, leaving free slots closer to head
//		ix = tbl.begin() + (new_size - 1);
//	}
//	ix->flags  = HANDLE_ENTRY_OSFILE;
//	ix->handle = handle_to_opened_file;
//	ix->extra  = 0;
//}

}