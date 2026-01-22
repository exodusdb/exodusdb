#ifndef DBCONN_H
#define DBCONN_H

#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#	include <cstdint>
#	include <set>
#	include <map>
#endif

#include <libpq-fe.h> // for PGconn

#include "task_mutex.h"
#include "stream_ptr.h"

namespace exo {

// Using map generally instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// Perhaps switch to this https://youtu.be/M2fKMP47slQ?t=476

using DBlocks = std::set<std::uint64_t>;
using DBcache = std::map<std::uint64_t, std::string>;
class DBconn;
using DBconns = std::map<int, DBconn>;

// Holds a pointer to a PGconn, caches for records, and a lock table
class DBconn {
public:
	// Constructors
	explicit DBconn();
	DBconn(PGconn* pgconn, std::string conninfo, int dbconn_no);

	// Destructor
	~DBconn();

	// Delete copy operations
	DBconn(const DBconn&) = delete;
	DBconn& operator=(const DBconn&) = delete;

	// Move operations
	DBconn(DBconn&& other) noexcept;
	DBconn& operator=(DBconn&& other) noexcept;

	// Members
	PGconn* pgconn_ = nullptr;
    StreamPtr stream_ptr_; // Owns the socket and its cleanup. socket added in async_PQexec
	// postgres locks per dbconn
	// Used to fail repetitive lock (per mv standard_ instead of stack locks (per postgres standard)
	DBlocks locks_;
	DBcache dbcache_;
	std::string conninfo_;
	bool in_transaction_ = false;

	TaskMutex mutex_;
	int dbconn_no_ = 0;

};

// Non-owning ptr to DBconn
// Implicitly converts to whichever ptr is needed in function calls
// DBconn contains a PGconn* so it is PGconn* plus.
class DBconn_ptr {
public:
	DBconn* dbconn = nullptr;

	// Conversion to DBconn*
	operator DBconn*() const { return dbconn; }

	// Conversion to PGconn*
	operator PGconn*() const { return dbconn ? dbconn->pgconn_ : nullptr; }

	// Conversion to bool
	operator bool() const { return dbconn != nullptr; }
};

} // namespace exo

#endif // DBCONN_H
