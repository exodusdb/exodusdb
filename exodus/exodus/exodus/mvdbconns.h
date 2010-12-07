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

namespace exodus {

typedef PGconn * CACHED_CONNECTION;
typedef void ( * DELETER_AND_DESTROYER )( CACHED_CONNECTION);
typedef std::map<int, void *> CONN_MAP;

#define CONNECTION_ENTRY_FREE  (0)
#define ACTIVE_CONNECTION      (1)
#define INVALIDATED_CONNECTION (-1)

//ALN:TODO: remove MvConnectionEntry
class MvConnectionEntry		// used as 'second' in pair, stored in connection map
{
  public:
	int flag;	// =0 means that slot is empty
	CACHED_CONNECTION connection;
	int extra;
};

class MvConnectionsCache
{
  public:
	MvConnectionsCache( DELETER_AND_DESTROYER del_);
	int add_connection( CACHED_CONNECTION connection_with_file);
	CACHED_CONNECTION get_connection( int index) const;
	CACHED_CONNECTION get_current_connection() const;
	int get_current_id() const;
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
