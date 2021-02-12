//
// Exodus -
//
// mvdbconn.h - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// Requirement 1. Current threadwise default connection id: tss_pgconnids.get();
// Requirement 2. When open(filename) - if filename is 'only' STR variable, store current connection
// within Requirement 3. read/write: if filename is 'only' STR variable, use default connection
// Requirement 4. 'filename' variable with stored connection has special mvtyp bit = VARTYP_DBCONN
// Requirement 5. NO ! There is connection table, where all connection variables should be
// registered (at 2 and 3) Requirement 6. No action is performed in destructor of 'filename'
// variable Requirement 7. disconnect() without parameters closes current (default) connection
// Requirement 8. filename.disconnect() closes the connection, linked to the 'filename', and 'frees'
// this var Requirement 9. Closed connection: just erased from map Requirement 10. Attempt to use
// 'invalidated' connection raises exception Requirement 11. LockTables (which accompany DB table
// locks), are added to connection table and
//					stored/deleted within connection record

#ifndef MVDBCONNS_H
#define MVDBCONNS_H

#include <boost/thread/mutex.hpp>
#include <libpq-fe.h> //in postgres/include
//#include <map>
#include <unordered_map>

// INtroducing the type for LockTable, it should be unordered set

#if 0

//#define HAVE_TR1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CXX0X
#include <unordered_set>
#define UNORDERED_SET_FOR_LOCKTABLE std::unordered_set<uint64_t>

// expect bug here since centos5.3 32bit has tr1 but not with an unordered set
#elif defined(HAVE_TR1)
#include <tr1/unordered_set>
#define UNORDERED_SET_FOR_LOCKTABLE std::tr1::unordered_set<uint64_t>

// assume we have a recent boost on windows
// boost 1.33 doesnt seem to have unordered set
#elif defined(HAVE_BOOST_UNORDERED_SET) || defined(_MSC_VER)
#include <boost/unordered_set.hpp>
#define UNORDERED_SET_FOR_LOCKTABLE boost::unordered_set<uint64_t>
#else
#define USE_MAP_FOR_UNORDERED
#include <map>

#define UNORDERED_SET_FOR_LOCKTABLE std::map<uint64_t, int>
#endif

#endif

#define USE_MAP_FOR_UNORDERED
//using LockTable = UNORDERED_SET_FOR_LOCKTABLE;
using LockTable = std::unordered_map<uint64_t, int>;

namespace exodus
{

using CACHED_CONNECTION = PGconn*;
using DELETER_AND_DESTROYER = void (*)(CACHED_CONNECTION /*, UNORDERED_SET_FOR_LOCKTABLE * */);

using RecordCache = std::unordered_map<std::string, std::string>;

class MvConnectionEntry // used as 'second' in pair, stored in connection map
{
      public:
	// ctors
	MvConnectionEntry() : flag(0), connection(0), plock_table(0), extra(0), precordcache(0) {}
	MvConnectionEntry(CACHED_CONNECTION connection_, LockTable* LockTable_,
			  RecordCache* RecordCache_)
	    : flag(0), connection(connection_), plock_table(LockTable_), extra(0),
	      precordcache(RecordCache_)
	{
	}

	// 1=entry is in use
	// 0=entry is not longer used and may be reused
	int flag;

	// postgres connection handle
	CACHED_CONNECTION connection;

	// postgres locks per connection
	// used to fail lock (per mv standard_ instead of stack locks (per postgres standard)
	LockTable* plock_table;

	//?
	int extra;

	RecordCache* precordcache;
};

//using CONN_MAP = std::map<int, MvConnectionEntry>;
using CONN_MAP = std::unordered_map<int, MvConnectionEntry>;

class MvConnectionsCache
{
      public:
	// ctors/dctors
	MvConnectionsCache(DELETER_AND_DESTROYER del_);
	virtual ~MvConnectionsCache();

	// manipulators
	int add_connection(CACHED_CONNECTION connection_with_file);
	void del_connection(int index);

	// observers
	CACHED_CONNECTION get_connection(int index) const;
	LockTable* get_lock_table(int index) const;
	RecordCache* get_recordcache(int index) const;
	std::string getrecord(const int connid, const std::string filename,
			       const std::string key) const;
	void putrecord(const int connid, const std::string filename, const std::string key,
			 const std::string& record);
	void delrecord(const int connid, const std::string filename, const std::string key);
	void clearrecordcache(const int connid);

      private:
	// function to close pg connection?
	DELETER_AND_DESTROYER del;

	// cache size
	int connection_id;

	// cache container to hold connections
	CONN_MAP conntbl;

	// mutex to coordinate updates
	mutable boost::mutex mvconnections_mutex;
};

} // namespace exodus
#endif // MVDBCONNS_H
