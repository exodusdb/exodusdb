//
// Exodus -
//
// mvdbconn.h - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// Requirement 1. Current default connection: PGconn* thread_pgconn=tss_pgconns.get();
// Requirement 2. When open( filename) - if filename is 'only' STR variable, store current connection within
// Requirement 3. read/write: if filename is 'only' STR variable, store current connection within
// Requirement 4. 'filename' variable with stored connection has special mvtyp = MVTYPE_SQLOPENED
// Requirement 5. NO ! There is connection table, where all connection variables should be registered (at 2 and 3)
// Requirement 6. No action is performed in destructor of 'filename' variable
// Requirement 7. disconnect() without parameters closes current connection
// Requirement 8. filename.disconnect() closes the connection, linked to the 'filename', and 'frees' this var
// Requirement 9. Closed connection: been closed and marked as 'invalidated'
// Requirement 10. Attempt to use 'invalidated' connection raises exception
// Requirement 11. 

#ifndef MVDBCONNS_H
#  define MVDBCONNS_H

#include <map>
#include <libpq-fe.h>//in postgres/include
#include <boost/thread/mutex.hpp>

// INtroducing the type for LockTable, it should be unordered set

//#define HAVE_TR1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  HAVE_CXX0X
#  include <unordered_set>
#  define UNORDERED_SET_FOR_LOCKTABLE std::unordered_set<uint64_t>

//expect bug here since centos5.3 32bit has tr1 but not with an unordered set
#elif defined(HAVE_TR1)
#  include <tr1/unordered_set>
#  define UNORDERED_SET_FOR_LOCKTABLE std::tr1::unordered_set<uint64_t>

//assume we have a recent boost on windows
//boost 1.33 doesnt seem to have unordered set
#elif defined(HAVE_BOOST_UNORDERED_SET) || defined(_MSC_VER)
#  include <boost/unordered_set.hpp>
#  define UNORDERED_SET_FOR_LOCKTABLE boost::unordered_set<uint64_t>
#else
#  define USE_MAP_FOR_UNORDERED
#  include <map>
#  define UNORDERED_SET_FOR_LOCKTABLE std::map<uint64_t,int>
#endif

#include "MurmurHash2_64.h"

//typedef boost::unordered_set<uint64_t> LockTable; 
//typedef UNORDERED_SET_FOR_LOCKTABLE<uint64_t> LockTable;
typedef UNORDERED_SET_FOR_LOCKTABLE LockTable;
//typedef tr1::unordered_set<uint64_t> LockTable;

namespace exodus {

//#define CONNECTION_ENTRY_FREE  (0)
//#define ACTIVE_CONNECTION      (1)
//#define INVALIDATED_CONNECTION (-1)

typedef PGconn * CACHED_CONNECTION;
typedef void ( * DELETER_AND_DESTROYER )( CACHED_CONNECTION/*, UNORDERED_SET_FOR_LOCKTABLE * */);

class MvConnectionEntry		// used as 'second' in pair, stored in connection map
{
  public:
	int flag;
	CACHED_CONNECTION connection;
	LockTable * plock_table;
	int extra;

	MvConnectionEntry()
		: flag(0), connection(0), plock_table( 0), extra(0)
	{}
	MvConnectionEntry( CACHED_CONNECTION connection_, UNORDERED_SET_FOR_LOCKTABLE * LockTable_)
		: flag(0), connection(connection_), plock_table( LockTable_), extra(0)
	{}
};

typedef std::map<int, MvConnectionEntry> CONN_MAP;

class MvConnectionsCache
{
  public:
	MvConnectionsCache( DELETER_AND_DESTROYER del_);
	int add_connection( CACHED_CONNECTION connection_with_file);
	CACHED_CONNECTION get_connection( int index) const;
	UNORDERED_SET_FOR_LOCKTABLE * get_lock_table( int index) const;
	CACHED_CONNECTION _get_current_connection() const;
	int _get_current_id() const;
	void del_connection( int index);
	virtual ~MvConnectionsCache();

  private:
	DELETER_AND_DESTROYER del;
	int connection_id;
//	std::map<MvConnectionEntry> __tbl__;//
	CONN_MAP tbl;
	mutable boost::mutex mvconnections_mutex;
};

}	// namespace
#endif // MVDBCONNS_H
