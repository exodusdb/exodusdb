//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// NOTE_OSX: Changing order of member construction could cause warning under MacOSX, like:
//	mvdbconns.h: In constructor 'exodus::MvConnectionsCache::MvConnectionsCache(void (*)(PGconn*))':
//	mvdbconns.h:57: warning: 'exodus::MvConnectionsCache::connection_id' will be initialized after
//	mvdbconns.h:56: warning:   'void (* exodus::MvConnectionsCache::del)(PGconn*)'
//	mvdbconns.cpp:15: warning:   when initialized here
//
#include "mv.h"
//#define INSIDE_MVDBCONNS_CPP
#include "mvdbconns.h"

namespace exodus {

MvConnectionsCache::MvConnectionsCache( DELETER_AND_DESTROYER del_)
	// see NOTE_OSX in header
	: del( del_)
	, connection_id( 0)
	, mvconnections_mutex()
	, tbl()
{}

int MvConnectionsCache::add_connection( CACHED_CONNECTION conn_to_cache)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);

	connection_id++;
	tbl[connection_id] = MvConnectionEntry( conn_to_cache, new LockTable);
	return connection_id;
}

CACHED_CONNECTION MvConnectionsCache::get_connection( int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = tbl.find( index);
	return ( CACHED_CONNECTION)( iter == tbl.end() ? 0 : iter->second.connection);
}

UNORDERED_SET_FOR_LOCKTABLE * MvConnectionsCache::get_lock_table( int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = tbl.find( index);
	return ( UNORDERED_SET_FOR_LOCKTABLE *)( iter == tbl.end() ? 0 : iter->second.plock_table);
}

void MvConnectionsCache::del_connection( int index)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator iter = tbl.find( index);
	if( iter != tbl.end())
	{
//	CACHED_CONNECTION p /*std::pair<int, void*> p*/ = ;
		del((CACHED_CONNECTION) iter/*tbl.find( index)*/->second.connection);
		delete /*tbl.find( index)*/iter->second.plock_table;
		tbl.erase( index);
	}
}

MvConnectionsCache::~MvConnectionsCache()
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator ix;
	for( ix = tbl.begin(); ix != tbl.end(); ix ++)
	{
		del(( CACHED_CONNECTION) ix->second.connection);
		delete ix->second.plock_table;
	}
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	lock.unlock();
}

}	// namespace