//
// Copyright (c) 2010 Neosys
//
// Desicion making.
// 1. We should keep table of *handles*, so pointers to ifstream, etc. are bad.
// 2. We should keep to work with handles to files and handles to connections.
//		With files we have choice "int", "FILE *", HANDLE, but with PostgreSQL
//		its PGconn *, so lets try to use void * as basic stored type
// 3. Actually, fopen/fread stuff is much better for many short read/write operations,
//		because FILE * is buffered stream. open/read/write/close - unbuffered.
//
#ifndef MVHANDLES_H
#  define MVHANDLES_H

#include <vector>

namespace exodus {

#define HANDLE_ENTRY_FREE	0
#define HANDLE_ENTRY_OSFILE	1
#define HANDLE_ENTRY_TABLE	2

typedef void * CACHED_HANDLE;
#define BAD_CACHED_HANDLE	((void *)0)

class MvHandleEntry
{
  public:
	int flags;
	CACHED_HANDLE handle;
	int extra;
};

class MvHandlesCache
{
  public:
	MvHandlesCache();
//	void add_osfile( int flg, int handle, int extra);
	int add_osfile( CACHED_HANDLE handle_to_opened_file);
	MvHandleEntry & operator [] ( int idx)
	{ 
		return tbl[ idx];
	}
	CACHED_HANDLE get_handle( int index);
	void del_handle( int index);
   virtual ~MvHandlesCache();

  private:
	std::vector<MvHandleEntry> tbl;
};
}	// namespace
#endif // MVHANDLES_H
