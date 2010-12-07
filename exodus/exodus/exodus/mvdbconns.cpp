//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
#include "mv.h"
#define INSIDE_MVDBCONNS_CPP
#include "mvdbconns.h"
#include <boost/thread/mutex.hpp>
boost::mutex mvconnections_mutex;

namespace exodus {

MvConnectionsCache::MvConnectionsCache( DELETER_AND_DESTROYER del_)
	: del( del_)
	, connection_id( 0)
{}

//MvConnectionsCache::set_deleter( DELETER_AND_DESTROYER del_)
//{
//	del = del_;
//}

int MvConnectionsCache::add_connection( CACHED_CONNECTION conn_to_cache)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);

	connection_id++;
	tbl[connection_id] = conn_to_cache;
	return connection_id;
}

CACHED_CONNECTION MvConnectionsCache::get_connection( int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = tbl.find( index);
	return ( CACHED_CONNECTION)( iter == tbl.end() ? 0 : iter->second);
}

CACHED_CONNECTION MvConnectionsCache::get_current_connection() const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = tbl.find( connection_id);
	return ( CACHED_CONNECTION)( iter == tbl.end() ? 0 : iter->second);
}

void MvConnectionsCache::del_connection( int index)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator iter = tbl.find( index);
	if( iter != tbl.end())
	{
//	CACHED_CONNECTION p /*std::pair<int, void*> p*/ = ;
		del((CACHED_CONNECTION) tbl.find( index)->second);
		tbl.erase( index);
	}
}

int MvConnectionsCache::get_current_id() const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	return connection_id; 
}

MvConnectionsCache::~MvConnectionsCache()
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator ix;

/*ALN:TODO: make following code not to crash
	Seems that after 2 threads closed, PQfinish( PGconn *) throws exception,
	may be because PGconn * pointer is invalidated already.
	Investigate this.
    May be use PQstatus(PGconn *)
*/
	for( ix = tbl.begin(); ix != tbl.end(); ix ++)
		del(( CACHED_CONNECTION) ix->second);
}

}	// namespace