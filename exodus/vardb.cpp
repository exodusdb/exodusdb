//https://wiki.postgresql.org/wiki/Don't_Do_This

/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// in case of migration to db2 or mysql here is a very detailed comparison in 2006
// http://www.ibm.com/developerworks/data/library/techarticle/dm-0606khatri/

// http://www.postgresql.org/docs/8.3/static/libpq-envars.html
// PGHOST/PGHOSTADDR
// PGPORT
// PGDATABASE
// PGUSER
// PGPASSWORD
// PGPASSFILE defaults to ~/.pgpass
// PGSERVICE in pg_service.conf in PGSYSCONFDIR

// TODO sql quoting of all parameters like dbname filename etc to prevent sql injection.

// 0=silent, 1=errors, 2=warnings, 3=results, 4=tracing, 5=debugging
// 0=silent, 1=errors, 2=warnings, 3=failures, 4=successes, 5=debugging ?

// MSVC requires exception handling (eg compile with /EHsc or EHa?) for delayed dll loading
// detection

////ALN:TODO: REFACTORING NOTES
// Proposed to split content of mvdbpostres.cpp into 3 layers (classic approach):
//	mvdbfuncs.cpp - api things, like exedusfuncs.cpp
//	mvdbdrv.cpp - base abstract class mv_db_drv or MvDbDrv to define db access operations (db
// driver interface); mvdbdrvpostgres.cpp - subclass of mv_db_drv, specific to PostgreSQL things
// like
// PGconn and PQfinish; mvdbdrvmsde.cpp - possible subclass of mv_db_drv, specific to MSDE (AKA
// MSSQL Express); mvdblogic.cpp - intermediate processing (most of group 2) functions.
// Proposed refactoring would:
//		- improve modularity of the Exodus platform;
//		- allow easy expanding to other DB engines.

/* How do transactions work in EXODUS?

Exodus uses postgresql's default transaction isolation level which is "READ COMMITTED"

See https://www.postgresql.org/docs/current/transaction-iso.html#XACT-READ-COMMITTED

1. Visibility of updates you make while in a transaction.

Any and all updates that you make while in a transaction will not be seen by any other connection until you commit your transaction.

On committing, any and all updates that you made will be immediately visible to all other connections that were not in a transaction at the time you committed.

2. Visibility of other connections updates while you are in a transaction.

During your transaction you will not see any updates from transactions committed after you started your transaction.

3. Lock visibility

Locks are essentially independent of transactions and can be seen by other connections in real time.

However, locks placed during a transaction cannot be unlocked and are automatically released after you commit. Unlock commands are therefore ignored.

Within transactions, lock requests for locks that have already been obtained always SUCCEED. This is the opposite of duplicate locks outside of transactions, which FAIL.

4. How to coordinate updates between asynchronous processes using locks.

	This process allows multiple asynchronous processes to update the database concurrently/in parallel as long as they are updating different data.

	a. Start a transaction

	b. Acquire an agreed lock appropriate for the data in c. and d.,
		OR wait for a limited period of time,
		OR or cancel the whole process i.e. rollback to to the state at a.

	c. Read the data
	d. Write the data

	e. Repeat from b. as required for multiple updates

	e. Commit the transaction (will also release all locks)

	Generally *LOCK BEFORE READ* and *WRITE BEFORE UNLOCK* so that all READ and WRITE operations are "protected" by locks.

	Locks form a barrier between changed state.

	In the above, "WRITE" encompasses INSERT, UPDATE and DELETE.

5. Postgres facilities used

	Transaction isolation level used is the default:

		READ COMMITTED (default)

		SERIALIZABLE or REPEATABLE READ - pending implementation to facilite consistent reports based on snapshots

		See https://www.postgresql.org/docs/12/sql-set-transaction.html

	Locks are obtained and released:

		Outside a transaction using pg_try_advisory_lock() and pg_try_advisory_unlock()

		Inside a transaction  using pg_try_advisory_xact_lock() and commit or rollback.

		See https://www.postgresql.org/docs/12/functions-admin.html#FUNCTIONS-ADVISORY-LOCKS

6. How Postgres handles versioning

	Summary explanation: https://vladmihalcea.com/how-does-mvcc-multi-version-concurrency-control-work/

	Detailed explanation: https://www.interdb.jp/pg/pgsql05.html

	PostgreSQL stores all row versions in the table data structure.

	This is called MVCC and postgres uses a version of it called "Snapshot Isolation".

	It never updates existing rows, only adds new rows and marks the old rows as "deleted".

	Automatic and manual VACUUM processes remove stale deleted rows at convenient times.

	Every row has two additional columns:

		t_xmin - the transaction id that inserted the record
		t_xmax - the transaction id that deleted the row, if deleted.

	Transaction id is an ever increasing 32 bit integer so it is possible to determine the
	state of the database "as at" any transaction id. Every query eg SELECT gets its
	own transaction id. Wrap around to zero after approx. 4.2 billion transactions is inevitable
	and the concept of before and after is not absolute.

	See SELECT txid_current();

	Every row also has the following control info.

		t_cid - holds the command id (cid), which means how many SQL commands were executed
				before this command was executed within the current transaction beginning from 0.

		t_ctid - holds the tuple identifier (tid) that points to itself or a new tuple replacing it.

*/

#if EXO_MODULE
	import std;
#else
#	include <cstring>	//for std::strcmp strlen

#	include <string>
#	include <string_view>
#	include <array>
#	include <iostream>

// Using map for caches instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
#	include <map>
#	include <utility> //for pair
#endif

//#define sigint_received false // in exoimpl.h (TERMINATE_req || RELOAD_req)
// Thread-safe mutex for pgconn (remove if get_pgconn ensures exclusivity)
//static std::mutex conn_mutex;

#ifdef EXO_PREPARED
// EXO_PREPARED = 1
#else
#define EXO_PREPARED false
#endif

//#undef EXO_BOOST_FIBER
//#define EXO_BOOST_FIBER
#ifdef EXO_BOOST_FIBER
#	include "vardb_async.h"
#	define XPQexecParams   async_PQexec<1>
#	define XPQexec         async_PQexec<2>
#	define XPQexecPrepared async_PQexec<3>
#else
#	define XPQexecParams   PQexecParams
#	define XPQexec         PQexec
#	define XPQexecPrepared PQexecPrepared_shim
#endif

// Global SIGINT flag
//static std::atomic<bool> sigint_received{false};
// TODO convert RELOAD|TERMINATE_req to atomic

// To see Postgres PQlib calls
//////////////////////////////
//
// All calls
//
//  grep -P '\bX?PQ[\w]+' vardb.cpp --color=always|less
//
// or without transaction related calls
//
//  grep -P '\bX?PQ[\w]+' *.cpp --color=always|grep -vP 'errorMessage|resultStatus|ntuples|getisnull|cmdTuples|PQexec|getvalue|getlength|nfields|PQfname|resultError'
//
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
// Probably in /usr/include/postgresql/libpq-fe.h
#include <libpq-fe.h>
#pragma clang diagnostic pop

//#include <arpa/inet.h>//for ntohl()

// All three hashing methods are about 80-90ns
// but c++ std::hash is not guaranteed stable except within a single running process
// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
#define USE_MURMURHASH3
//#define USE_WYHASH

#if defined (USE_WYHASH)
#	include "wyhash.h"

#elif defined(USE_MURMURHASH3)
#	include "murmurhash3.h"
	static const uint32_t murmurhash_seed = 0xf00dba11;

#elif defined(USE_MURMURHASH2)
#	include "murmurhash2_64.h" // it has included in DBpool.h (std::uint64_t defined)

#else
	// c++ std Hash functions are only required to produce the same result for the same input within a single execution of a program;
	// therefore different processes would not be able to perform coordinated record locking
	// therefore we will use one of the above hashing functions which hash the same on different platforms (hopefully)
	// C++ std hash functions are designed for use in c++ containers, nothing else.
#endif

#include <exodus/varimpl.h>
#include "DBpool.h"         // placed as last include, causes boost header compiler errors
#include <exodus/dim.h>
#include <exodus/rex.h>

#ifdef EXO_TIMEBANK
#include "timebank.h"
#endif

#define DBTRACE_SELECT DBTRACE>1

#define DBTRACE_CONN DBTRACE

#define DEBUG_LOG_SQL0 if (DBTRACE) {\
	std::clog << dbconn_ptr.dbconn->dbconn_no_ << ": SQL0 " << \
	sql.convert("\n\t", "  ").trim() << "\n";\
}

#define DEBUG_LOG_SQL1 if (DBTRACE) {\
	std::clog << pgconn.dbconn->dbconn_no_ << ": SQL1 " << \
	(sql.contains("$1")\
	?\
	sql.replace("$1", var(paramValues[0]).first(50).squote())\
	:\
	(sql ^ " " ^ var(paramValues[0]).first(50).squote())).convert("\n\t", "  ").trim() << "\n";\
}

#include "DBresult.h"

namespace exo {

// Obtained from env EXO_DBTRACE in get_dbconn_no_or_default on first connection
static inline int DBTRACE = -1;
//#define EXO_DBTRACE
using let = const var;

// The idea is for exodus to have access to one standard database without secret password
static const var defaultconninfo =
	"host=127.0.0.1 port=5432 dbname=exodus user=exodus "
	"password=somesillysecret connect_timeout=10";

// this front end C interface is based on postgres
// http://www.postgresql.org/docs/8.2/static/libpq-exec.html
// doc/postgres/libpq-example.html
// src/test/examples/testlibpq.c

//(backend pg functions extract and dateextract are based
// on samples in src/tutorial/funcs_new.c)

// SQL EXAMPLES
// create or replace view testview as select exodus.extract_text(data,1,0,0) as field1 from test;
// create index testfield1 on test (exodus.extract_text(data,1,0,0));
// select * from testview where field1  > 'aaaaaaaaa';
// analyse;
// explain select * from testview where field1  > 'aaaaaaaaa';
// explain analyse select * from testview where field1  > 'aaaaaaaaa';e

// THREAD_LOCAL data - shared by all dynamically loaded shared libraries
static thread_local int                                thread_default_data_dbconn_no = 0;
static thread_local int                                thread_default_dict_dbconn_no = 0;
static thread_local DBpool                             thread_dbpool;
static thread_local std::map<std::string, std::string> thread_file_handles;

// Few entries so map will be much faster than unordered_map
static thread_local std::map<std::string, DBresult>    thread_dbresults;

static var getpgresultcell(PGresult* pgresult, int rown, int coln) {
	return var(PQgetvalue(pgresult, rown, coln), PQgetlength(pgresult, rown, coln));
}

// shim to synchonise function sig.
auto PQexecPrepared_shim(
	PGconn *conn,
	const char *stmtName,
	int nParams,
	void*,
	const char *const *paramValues,
	const int *paramLengths,
	const int *paramFormats,
	int resultFormat
) -> PGresult* {
	return PQexecPrepared(conn, stmtName, nParams, paramValues, paramLengths, paramFormats, resultFormat);
}


//void dump_pgresult(PGresult* pgresult) {
//
//	/* first, print out the attribute names */
//	printf("----------");
//	printf("%p", pgresult);
//	printf("----------\n");
//	int nFields = PQnfields(pgresult);
//	int i,j;
//	for (i = 0; i < nFields; i++)
//		printf("%-15s", PQfname(pgresult, i));
//	printf("\n");
//	printf("----------\n");
//
//	/* next, print out the rows and columns */
//	for (i = 0; i < PQntuples(pgresult); i++)
//	{
//		for (j = 0; j < nFields; j++)
//			printf("%-15s", PQgetvalue(pgresult, i, j));
//		printf("\n");
//	}
//	printf("==========\n");
//}

// Given a file name or handle, extract filename, standardize utf8, lowercase and change . to _
// Normalise all alternative utf8 encodings of the same unicode points so they are identical
static var get_normalized_filename(in filename_or_handle) {

	// No longer convert . in filenames to _
	return filename_or_handle.f(1).normalize().lcase();
}

// Detect sselect command words that are values like quoted words or plain numbers.
// eg "xxx" 'xxx' 123 .123 +123 -123
static const var valuechars("\"'.0123456789-+");

// hash64 a std::string
///////////////////////
static std::uint64_t vardb_hash_stdstr(std::string str1) {

#if defined(USE_WYHASH)
	return wyhash(str1.data(), str1.size(), 0, _wyp);

#elif defined(USE_MURMURHASH3)

//	std::uint64_t u128[2];
//	MurmurHash3_x64_128(str1.data(), static_cast<int>(str1.size()), murmurhash_seed, u128);
//
//	static_cast<std::uint64_t*>(u128)[0] ^= static_cast<std::uint64_t*>(u128)[1];
//	return static_cast<std::uint64_t*>(u128)[0];

	// Pass two std::uint64_t to MurmurHash3
	std::array<std::uint64_t, 2> u128;

	// MurmurHash3
	MurmurHash3_x64_128(str1.data(), static_cast<int>(str1.size()), murmurhash_seed, &u128[0]);

	// From the MurmurHash3 function's finalizer.
	//
	//  ((std::uint64_t*)out)[0] = h1;
	//  ((std::uint64_t*)out)[1] = h2;

	// Return a mix of first 64 bits with last 64 bits using xor because we can only return 64 bits
	return u128[0] ^ u128[1];

#elif defined(USE_MURMURHASH2)
	return MurmurHash64(str1.data(), static_cast<int>(str1.size()), 0);
#else
	return std::hash<std::string>{}(str1);
#endif

}

// Cross platform lock number
/////////////////////////////
// Create a cross platform stable unique lock number per filename & key to manipulate advisory locks on a postgres connection
// TODO Provide an endian identical version. required if and when exodus processes connect to postgres from different endian hosts
static std::uint64_t vardb_hash_file_and_key(in file, in key) {

	// Use the pure filename and disregard any connection number
	std::string fileandkey = get_normalized_filename(file);

	// Separate the filename from the key to avoid alternative interpretations
	// eg "FILENAMEKEY" could be FILE, NAMEKEY or FILENAME, KEY
	// but "FILENAME KEY" can only be one thing
	fileandkey.push_back('\0');

	// Append the key
	// Normalise all alternative utf8 encodings of the same unicode points so they hash identically
	fileandkey.append(key.normalize().toString());

	return vardb_hash_stdstr(fileandkey);

}

static int get_dbconn_no(in dbhandle) {

	if (not dbhandle.assigned()) UNLIKELY {
		// var("get_dbconn_no() returning 0 - unassigned").logputl();
		return 0;
	}
	let dbconn_no = dbhandle.f(2);
	if (dbconn_no.isnum()) {
		/// var("get_dbconn_no() returning " ^ dbconn_no).logputl();
		return dbconn_no;
	}

	// var("get_dbconn_no() returning 0 - not numeric").logputl();

	return 0;
}

static int get_dbconn_no_or_default(in dbhandle) {

	// Cache DBTRACE on first attempt at any connection
	if (DBTRACE < 0) {
		var dbtrace;
		DBTRACE = dbtrace.osgetenv("EXO_DBTRACE") ? dbtrace.toInt() : 0;
	}

	int dbconn_no = get_dbconn_no(dbhandle);
	if (dbconn_no) LIKELY
		return dbconn_no;

	// Otherwise get the default connection
	if (DBTRACE_CONN >= 3 and dbhandle.assigned())
		TRACE(dbhandle)

	// dbhandle MUST always arrive in lower case to detect if "dict."
	bool isdict = dbhandle.unassigned() ? false : dbhandle.starts("dict.", "DICT.");

	if (isdict)
		dbconn_no = thread_default_dict_dbconn_no;
	else
		dbconn_no = thread_default_data_dbconn_no;

	if (DBTRACE_CONN >= 3 ) {
		TRACE(thread_default_data_dbconn_no)
		TRACE(thread_default_dict_dbconn_no)
		if (dbhandle.assigned()) TRACE(dbhandle)
		TRACE(dbconn_no)
	}

	// Otherwise try the default connection
	if (not dbconn_no) {

		var defaultdb;

		// Look for dicts in the following order
		// 1. $EXO_DICT if defines
		// 2. db "dict" if present
		// 3. the default db connection
		if (isdict) {
			if (not defaultdb.osgetenv("EXO_DICT") or not defaultdb)
				// Must be the same in vardb.cpp and dict2sql
				defaultdb="exodus";
		} else {
			defaultdb = "";
		}

		if (DBTRACE_CONN >= 3)
			TRACE(defaultdb)

		// Try to connect
		if (defaultdb.connect()) {
			dbconn_no = get_dbconn_no(defaultdb);

			if (DBTRACE_CONN >= 3 )
				TRACE("defaultdb connected")

		// If cannot connect then for dictionaries look on default connection
		} else if (isdict) {

			// Attempt a default connection if not already done
			if (not thread_default_data_dbconn_no) {
				defaultdb = "";
				if (not defaultdb.connect()) {
					//null
				}
			}

			dbconn_no = thread_default_data_dbconn_no;

			if (DBTRACE_CONN >= 3)
				TRACE(thread_default_data_dbconn_no)

		}

		if (DBTRACE_CONN >= 3)
			TRACE(dbconn_no)

		// Save default dict/data connections
		if (isdict) {
			thread_default_dict_dbconn_no = dbconn_no;
			if (DBTRACE>1 or DBTRACE_CONN) {
				var(dbconn_no).logputl("DBTR NEW DEFAULT DICT CONN ");
			}
		}
		else {
			thread_default_data_dbconn_no = dbconn_no;
			if (DBTRACE>1 or DBTRACE_CONN) {
				var(dbconn_no).logputl("DBTR NEW DEFAULT DATA CONN ");
			}
		}
	}

	// Save the connection number in the dbhandle
	//if (dbconn_no) {
	//	dbhandle.defaulter("");
	//	dbhandle.r(2, dbconn_no);
	//}

	return dbconn_no;
}

// 1. return the associated db connection
// this could be a previously opened filevar, a previous connected connectionvar
// or any variable previously used for a default connection
// OR
// 2. return the thread-default connection
// OR
// 3. do a default connect if necessary
//
// NB in case 2 and 3 the connection id is recorded in the var
// use void pointer to avoid need for including postgres headers in mv.h or any fancy class
// hierarchy (assumes accurate programming by system programmers in exodus mvdb routines)
static DBconn_ptr get_pgconn(in dbhandle) {

	// var("--- connection ---").logputl();
	// Get the connection associated with *this
	int dbconn_no = get_dbconn_no_or_default(dbhandle);
	// var(dbconn_no).logputl("dbconn_no1=");

//	// Otherwise fail
//	if (not dbconn_no) UNLIKELY
////		throw VarDBException("pgconnection() requested when not connected.");
//		return nullptr;

	auto pgconn=thread_dbpool.get_pgconn(dbconn_no);
	if (DBTRACE>1) UNLIKELY {
		std::clog << std::endl;
//		std::clog << "CONN " << dbconn_no << " " << pgconn << std::endl;
		std::clog << "CONN " << dbconn_no << std::endl;
	}

	// Return the relevent pg_connection structure
//	const auto pgconn = thread_dbpool.get_dbconn(dbconn_no)->pgconn_;
	// TODO error abort if zero
	return pgconn;

}

// Gets lock_table, associated with connection, associated with this object
static DBconn* get_dbconn(in dbhandle) {
	int dbconn_no = get_dbconn_no_or_default(dbhandle);
	if (not dbconn_no) UNLIKELY {
		let errmsg = "get_dbconn() attempted when not connected";
		throw VarDBException(errmsg);
	}
	return thread_dbpool.get_dbconn(dbconn_no);
}

// Used for sql commands that require no parameters
// Returns 1 for success
// Returns 0 for failure
// dbresult is returned to caller to extract any data and call PQclear(dbresult) in destructor of DBresult
static DBresult get_dbresult(in sql, DBconn_ptr dbconn_ptr, out ok) {
	DEBUG_LOG_SQL0

	/* Dont use PQexec because is cannot be told to return binary results
	and use PQexecParams with zero parameters instead
	//execute the command
	dbresult = get_dbresult(dbconn_ptr, sql.var_str.c_str());
	dbresult = dbresult;
	*/

	// Parameter array with no parameters
	const char* paramValues[1] = {nullptr};
	const int paramLengths[1] = {0};

	// Will contain any dbresult IF successful
	DBresult dbresult = XPQexecParams(dbconn_ptr, sql.toString().c_str(), 0, /* zero params */
							nullptr,								/* let the backend deduce param type */
							paramValues, paramLengths,
							nullptr,  // text arguments
							0);	// text results

	// Handle serious error. Why not throw?
	if (not dbresult) UNLIKELY {
		dbresult.pqerrmsg = "ERROR: vardb: PQexecParams command failed, no error code: ";
		var(dbresult.pqerrmsg).errputl();
		ok = false;
		return dbresult;
	}

	switch (PQresultStatus(dbresult)) {

		case PGRES_COMMAND_OK:
			LIKELY
			dbresult.pqerrmsg = "";
			ok = true;
			if (DBTRACE > 0)
				TRACE(sql ^ " OK");
			return dbresult;

		case PGRES_TUPLES_OK:
			ok = PQntuples(dbresult) > 0;
			if (ok)
				dbresult.pqerrmsg = "";
			else
				dbresult.pqerrmsg = "PQntuples" + std::to_string(PQntuples(dbresult));
			return dbresult;

		case PGRES_NONFATAL_ERROR:

			dbresult.pqerrmsg = std::string("ERROR: vardb: SQL non-fatal error code ") +
				PQresStatus(PQresultStatus(dbresult)) + ", " +
				PQresultErrorMessage(dbresult);
			var(dbresult.pqerrmsg).errputl();
			ok = false;
			return dbresult;

		case PGRES_EMPTY_QUERY:
		case PGRES_COPY_OUT:
		case PGRES_COPY_IN:
		case PGRES_BAD_RESPONSE:
		case PGRES_FATAL_ERROR:
		case PGRES_COPY_BOTH:
		case PGRES_SINGLE_TUPLE:
#ifdef PGRES_PIPELINE_SYNC
		case PGRES_PIPELINE_SYNC:
#endif
#ifdef PGRES_PIPELINE_ABORTED
		case PGRES_PIPELINE_ABORTED:
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
		default:
			UNLIKELY
			dbresult.pqerrmsg = "ERROR: vardb: pqexecparams " + sql.toString() +
					" ERROR: vardb: pqexecparams " +
					PQresStatus(PQresultStatus(dbresult)) + ": " +
					PQresultErrorMessage(dbresult);
			var(dbresult.pqerrmsg).errputl();
			ok = false;
			return dbresult;
#pragma clang diagnostic push
	}

	// Should never get here
	// std::unreachable
	ok = false;
	dbresult.pqerrmsg = "Unknown error in " + sql.toString();
	return dbresult;

}

static var build_conn_info(in conninfo) {
	// Priority is
	// 1) given parameters //or last connection parameters
	// 2) individual environment parameters
	// 3) environment connection string
	// 4) config file parameters
	// 5) hard coded default parameters

	var result(conninfo);
	result.trimmer();

	// If no conninfo details provided then use last connection details if any
	//if (not conninfo)
	//	result = thread_connparams;

	// Remove spaces around = to enable parsing using space
	result.replacer(R"(\s+=\s+)"_rex, "=");

	// Otherwise search for details from exodus config file
	// if incomplete connection parameters provided
	if (not result.contains("host=") or not result.contains("port=") or not result.contains("dbname=") or
		not result.contains("user=") or not result.contains("password=")) {

		// Discover any configuration file parameters
		// TODO parse config properly instead of just changing \r\n to spaces!
		var configfilename = "";
		var home = "";
		if (home.osgetenv("HOME"))
			configfilename = home ^ "/.config/exodus/exodus.cfg";
		else if (home.osgetenv("USERPROFILE"))
			configfilename ^= home ^ "\\Exodus\\.exodus";
		var configconn = "";
		if (not configconn.osread(configfilename) and not configconn.osread("exodus.cfg"))
			configconn = "";
		// postgres ignores after \n?
		configconn.converter("\r\n","  ");

		// Discover any configuration in the environment
		var envconn = "";
		var temp;
		if (temp.osgetenv("EXO_CONN") and temp)
			envconn ^= " " ^ temp;

		// Specific variables are appended ie override
		if (temp.osgetenv("EXO_HOST") and temp)
			envconn ^= " host=" ^ temp;

		if (temp.osgetenv("EXO_PORT") and temp)
			envconn ^= " port=" ^ temp;

		if (temp.osgetenv("EXO_USER") and temp)
			envconn ^= " user=" ^ temp;

		if (temp.osgetenv("EXO_DATA") and temp) {
			envconn.replacer(R"(dbname\s*=\s*\w*)"_rex, "");
			envconn ^= " dbname=" ^ temp;
		}

		if (temp.osgetenv("EXO_PASS") and temp)
			envconn ^= " password=" ^ temp;

		if (temp.osgetenv("EXO_TIME") and temp)
			envconn ^= " connect_timeout=" ^ temp;

		result = defaultconninfo ^ " " ^ configconn ^ " " ^ envconn ^ " " ^ result;
	}

	// Remove excess spaces. Especially around = to enable parsing using space
	result.replacer(R"(\s+=\s+)"_rex, "=");
	result.trimmer();

	// Work backwards through parts
	dim parts = result.split(" ").reverse();
	result = "";
	var keys = "";
	for (const var& part : parts) {

		// Ignore params already seen
		const var key = part.field("=", 1);
		if (keys.locate(key))
			continue;
		keys ^= key ^ VM;

		result ^= part ^ " ";
	}
	result.popper();

	return result;
}

static inline void unquoter_inline(io iovar) {
	// Remove "", '' and {}
	static var quotecharacters("\"'{");
	if (quotecharacters.contains(iovar.first())) {
		iovar.cutter(1);
		iovar.popper();
	}
}

static void tosqlstring(io string1) {

	// If double quoted then convert to sql style single quoted strings
	// double up any internal single quotes
	if (string1.starts("\"")) {
		string1.replacer("'", "''");
		string1.paster(1, 1, "'");
		string1.paster(-1, 1, "'");
	}
}

static var get_fileexpression(in /*mainfilename*/, in filename, in keyordata) {

	// if (filename == mainfilename)
	//	return keyordata;
	// else
	//return filename.convert(".", "_") ^ "." ^ keyordata;
	return get_normalized_filename(filename) ^ "." ^ keyordata;

	// If you dont use STRICT in the postgres function declaration/definitions then nullptr
	// parameters do not abort functions

	// Use COALESCE function in case this is a joined but missing record (and therefore null)
	// in MYSQL this is the ISNULL expression?
	// xlatekeyexpression="exodus.extract_text(coalesce(" ^ filename ^ ".data,''::text), " ^
	// xlatefromfieldname.cut(8); if (filename==mainfilename) return expression; return
	// "coalesce(" ^ expression ^", ''::text)";
}

// Used in var::selectx
static void to_extract_text(io dictexpression) {
				dictexpression.replacer("^exodus.extract_number\\("_rex, "exodus.extract_text(");
				dictexpression.replacer("^exodus.extract_sort\\("_rex, "exodus.extract_text(");
				dictexpression.replacer("^exodus.extract_date\\("_rex, "exodus.extract_text(");
				dictexpression.replacer("^exodus.extract_time\\("_rex, "exodus.extract_text(");
				dictexpression.replacer("^exodus.extract_datetime\\("_rex, "exodus.extract_text(");
}

bool var_db::connect(in conninfo) {

	THISIS("bool var::connect(in conninfo")
	assertVar(function_sig);
	ISSTRING(conninfo)

//	// NB DONT
//  // log/trace or otherwise output the full connection info without HIDING the
//	// password
//	if (DBTRACE>1 or DBTRACE_CONN) {
//		TRACE(__PRETTY_FUNCTION__)
//		TRACE(conninfo)
//	}

	var fullconninfo = conninfo.trimboth();

	// Use *this if conninfo not specified;
	bool isdefault = false;
	if (not fullconninfo) {
		if (this->assigned())
			fullconninfo = *this;
		isdefault = not fullconninfo;
		if (DBTRACE_CONN > 1) {
			TRACE(isdefault)
			TRACE(thread_default_data_dbconn_no)
		}
		if (isdefault and thread_default_data_dbconn_no) {
			return thread_default_data_dbconn_no;
		}
	}

	// Prefix "dbname=" if only dbname provided
	if (fullconninfo and  not fullconninfo.contains("="))
		fullconninfo = "dbname=" ^ fullconninfo.lcase();

	// Build all connection defaults
	fullconninfo = build_conn_info(fullconninfo);

	// Log the conninfo without password
	if (DBTRACE_CONN > 1) {
//		fullconninfo.replace(R"(password\s*=\s*\w*)"_rex, "password=**********").logputl("\nvar::connect( ) ");
		fullconninfo.replace(R"(password\s*=\s*\w*)"_rex, "").logputl("\nvar::connect( ) ");
	}

	PGconn* pgconn;
	for (;;) {

		// Attempt connection
		pgconn = PQconnectdb(fullconninfo.var_str.c_str());

		// Connected OK
		if (PQstatus(pgconn) == CONNECTION_OK or fullconninfo)
			break;

		// Required even if connect fails according to docs
		PQfinish(pgconn);

		// Try again with default conninfo
		fullconninfo = defaultconninfo;

	}

	// Failed to connect so return false
	if (PQstatus(pgconn) != CONNECTION_OK) UNLIKELY {

		let errmsg = "ERROR: vardb: connect() Connection to database failed: " ^ var(PQerrorMessage(pgconn));

		var::setlasterror(errmsg);

		// Required even if connect fails according to docs
		PQfinish(pgconn);

		return false;
	}

#ifdef PQisthreadsafe
	// Abort if multithreading and it is not supported
	if (not PQisthreadsafe()) UNLIKELY {
		// TODO only abort if environmentn>0
		let errmsg = "connect(): Postgres PQ library is not threadsafe";
		throw VarDBException(errmsg);

	}
#endif

	// At this point we have good new connection to database

	// Cache the new connection handle and get the cache index no (starting 1)
	int dbconn_no = thread_dbpool.add_dbconn(pgconn, fullconninfo.var_str);
	if (DBTRACE_CONN > 1) {
		TRACE(thread_default_data_dbconn_no)
		TRACE(thread_default_dict_dbconn_no)
	}

	// Save the db name and connection numbers
	if (not this->assigned())
		(*this) = "";

	this->updater(1,fullconninfo.field("dbname=", 2).field(" ", 1));

	this->updater(2, dbconn_no);
	this->updater(3, dbconn_no);

	if (DBTRACE_CONN > 0) {
//		fullconninfo.replace(R"(password\s*=\s*\w*)"_rex, "password=**********").logputl("var::connect() OK ");
		this->f(1).logputl(this->f(2) ^ ": DBTR var::connect() OK ");
//		std::clog << " " << pgconn << std::endl;
	}

	// Set default connection - ONLY IF THERE ISNT ONE ALREADY
	if (isdefault and not thread_default_data_dbconn_no) {
		thread_default_data_dbconn_no = dbconn_no;
		if (DBTRACE_CONN > 0) {
			this->f(1).logputl(this->f(2) ^ ": DBTR NEW DEFAULT DATA CONN " ^ var(dbconn_no) ^ " on ");
		}

	}

	// Save the last connection string (used in startipc())
	//thread_connparams = fullconninfo;

	// Doesnt work
	// need to set PQnoticeReceiver to suppress NOTICES like when creating files
	// PQsetErrorVerbosity(pgconn, PQERRORS_TERSE);
	// but this does
	// this turns off the notice when creating tables with a primary key
	// DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, LOG, NOTICE, WARNING, ERROR, FATAL, and PANIC
//	if (not this->sqlexec(var("SET client_min_messages = ") ^ (DBTRACE ? "LOG" : "WARNING"))) UNLIKELY
//		var::loglasterror();

	return true;
}

bool var_db::attach(in filenames) const {

	THISIS("bool var::attach(in filenames")
	assertVar(function_sig);
	ISSTRING(filenames)

	// Option to attach all dict files
	var filenames2;
	if (filenames == "dict") {
		filenames2 = "";
		let allfilenames = this->listfiles();
		for (var filename : allfilenames) {
			if (filename.starts("dict.", "DICT.")) {
				filenames2 ^= filename ^ FM;
			}
		}
		filenames2.popper();
	}
	else {
		filenames2 = filenames;
	}

	// Cache file handles in thread_file_handles
	var notattached_filenames = "";
	for (var filename : filenames2) {
		const var normalized_filename = get_normalized_filename(filename);
		var file;
		if (file.open(normalized_filename, *this)) {
			// Similar code in dbattach and open
			thread_file_handles[normalized_filename] = file.var_str;
			if (DBTRACE>1)
				file.logputl("DBTR var::attach() ");
		}
		else {
			notattached_filenames ^= normalized_filename ^ " ";
		}
	}

	// Fail if anything not attached
	if (notattached_filenames) UNLIKELY {
		let errmsg = "ERROR: vardb:/attach: " ^ notattached_filenames ^ "cannot be attached on connection " ^ this->f(1).quote();
		var::setlasterror(errmsg);
		return false;
	}

	return true;
}

void var_db::detach(in filenames) {

	THISIS("bool var::detach(in filenames")
	assertVar(function_sig);
	ISSTRING(filenames)

	for (var filename : filenames) {
		// Similar code in detach and deletefile
		thread_file_handles.erase(get_normalized_filename(filename));
	}
	return;
}

// if this->obj contains connection_id, then such connection is disconnected with this-> becomes UNA
// Otherwise, default connection is disconnected
void var_db::disconnect() {

	THISIS("bool var::disconnect()")
	assertVar(function_sig);
//	assertString(function_sig);

	if (DBTRACE>1 or DBTRACE_CONN > 0)
		(this->assigned() ? *this : var("")).logputl("DBTR var::disconnect() ");

	// Disconnect the specified connection or default data connection
	var dbconn_no = get_dbconn_no(*this);
	if (not dbconn_no)
		dbconn_no = thread_default_data_dbconn_no;

	// Quit if no connection
	if (not dbconn_no) UNLIKELY
		return;

	// Disconnect
	// Note singular form of dbconn
	thread_dbpool.del_dbconn(dbconn_no);
	var_typ = VARTYP_UNA;

	// If we happen to be disconnecting the same connection as the default connection
	// then reset the default connection so that it will be reconnected to the next
	// connect this is rather too smart but will probably do what people expect
	if (dbconn_no == thread_default_data_dbconn_no) {
		thread_default_data_dbconn_no = 0;
		if (DBTRACE>1 or DBTRACE_CONN > 0) {
			var(dbconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DATA ");
		}
	}

	// If we happen to be disconnecting the same connection as the default connection FOR DICT
	// then reset the default connection so that it will be reconnected to the next
	// connect this is rather too smart but will probably do what people expect
	if (dbconn_no == thread_default_dict_dbconn_no) {
		thread_default_dict_dbconn_no = 0;
		if (DBTRACE>1 or DBTRACE_CONN > 0) {
			var(dbconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DICT ");
		}
	}

	// Remove all cached files handles referring to the disconnected connection
	std::erase_if(
		thread_file_handles,
		[dbconn_no](auto iter){
			let file = iter.second;
			// Don't erase if not the desired dbconn_no
			if (file.f(2) != dbconn_no) UNLIKELY
				return false;
			if (DBTRACE_CONN >= 3) {
				var(iter.second).logputl("var::disconnect() remove cached file ");
			}
			// Do erase this file handle cache entry
			return true;
		}
	);

	return;
}

// Disconnect the current connection no (or start at 2) and all above
void var_db::disconnectall() {

	THISIS("bool var::disconnectall()")
	assertVar(function_sig);

	// Start at the current dbconn_no (default to 2)
	var dbconn_no = get_dbconn_no(*this);
	if (not dbconn_no)
		dbconn_no = 2;

	if (DBTRACE>1)
		dbconn_no.logputl("DBTR var::disconnectall() >= ");

	// Disconnect all connections >= dbconn_no
	auto max_dbconn_no = thread_dbpool.max_dbconn_no();
	for (auto dbconn_no2 = dbconn_no; dbconn_no2 <= max_dbconn_no; ++dbconn_no2) {
		var connection2 = FM ^ dbconn_no2;
		connection2.disconnect();
	}

	// Make sure the max dbconn_no is reduced
	// although the connections will have already been disconnected
	// Note the PLURAL form of dbconn"S" to delete all starting from
	thread_dbpool.del_dbconns(dbconn_no);

	return;
}

// Open a given filename on given connection or current thread-default connection to file var
// We are using strict filename/file syntax (even though we could use one variable for both!)
// We store the filename in the file since that is what we need to generate read/write sql
// later usage example:
// var file;
// var filename="customers";
// if (not file.open(filename)) ...

// Connection is optional and default connection may be used instead
bool var_db::open(in filename, in connection /*DEFAULTNULL*/) {

	THISIS("bool var::open(in filename, in connection)")
	assertVar(function_sig);
	ISSTRING(filename)

	const var normalized_filename = get_normalized_filename(filename);

	// If filename == dos or DOS  means osread/oswrite/osremove
	if (normalized_filename.var_str.size() == 3 and normalized_filename.var_str == "dos") UNLIKELY {
		var_str = "dos";
		var_typ = VARTYP_NANSTR;
		return true;
	}

	// Either use connection provided
	var connection2;
	if (connection) {
		connection2 = connection;
	}
	else {

		// Or use any preopened or preattached file handle if available
		auto entry = thread_file_handles.find(normalized_filename);
		if (entry != thread_file_handles.end()) {

			auto cached_file_handle = entry->second;

			// Make sure the connection is still valid otherwise redo the open
			const auto pgconn = get_pgconn(cached_file_handle);
			if (not pgconn) {
				thread_file_handles.erase(normalized_filename);
			} else {
				var_str = cached_file_handle;
				var_typ = VARTYP_NANSTR;
				if (DBTRACE>1) UNLIKELY
					this->logputl("DBTR open() cached or attached ");
				return true;
			}
		}

		// Or determine connection from filename in case filename is a handle
		// Use default data or dict connection
		connection2 = normalized_filename;

	}

#ifdef EXO_TIMEBANK
	//Timer t2(245);//open cache_miss
	Timer timer1(get_timebank_acno("bool var::open cache_miss"));
	//if (DBTRACE>1) {
	//	connection2.logputl("DBTR var::open-1 ");
	//}
#endif

	// Determine schema
	var tablename;
	var schema;
	var and_schema_clause;
	if (normalized_filename.contains(".")) {
		// e.g. filenames starting dict.
		schema = normalized_filename.field(".",1);
		tablename = normalized_filename.field(".",2,999);
		and_schema_clause = " AND table_schema = " ^ schema.squote();
	} else {

		// postgresl search_path
		// psql exodus -c "show search_path;" -> "$user", public
		// psql exodus -c "set search_path to "$user", public;"

		// 1. Since postgres 15? users no longer have default rights to create tables in public schema.
		// 2. schema exodus is created by pgexodus extension to hold the exodus functions.
		// The result is that since postgres 15? libexodus creates tables by default in exodus schema when using the default user exodus.

		// We will accept any schema for open
		// FIXME limit to schemas on the search path otherwise will not be able to access the file without explicit schema
		schema = "public";

		tablename = normalized_filename;
		//no schema filter allows opening temporary files with exist in various pg_temp_xxxx schemata
		//and_schema_clause = "";
		and_schema_clause = " AND table_schema != 'dict'";
	}

	// 1. Look in information_schema.tables
	// 2. Look in materialised views
	var sql =
		"SELECT"
		" EXISTS	("
		"	SELECT table_name"
		"	FROM   information_schema.tables"
		"	WHERE  table_name = " ^ tablename.squote() ^ and_schema_clause ^ " "
		" )"
		" OR"
		" EXISTS	("
		"	SELECT  matviewname as table_name"
		"	FROM    pg_matviews"
		"	WHERE"
		"		schemaname = " ^ schema.squote() ^ " "
		"		and matviewname = " ^ tablename.squote() ^ " "
		" )";

	var result;
	if (not connection2.sqlexec(sql, result)) UNLIKELY
		throw VarDBException(var::lasterror());

	// Failure if not found. It is not an error.
	if (not result.ends("t")) UNLIKELY {
		let errmsg = filename.quote() ^ " file does not exist.";
		var::setlasterror(errmsg);
		return false;
	}

	// Opening a file triggers prepare.
	if (EXO_PREPARED) {
		let code = normalized_filename.convert(".", "_");
		let sql =
//			"DO $$"
//			"BEGIN"
//			"    IF NOT EXISTS (SELECT 1 FROM pg_prepared_statements WHERE name = 'vardb_read_" ^ code ^ "') THEN"
//			"        EXECUTE 'PREPARE vardb_read_" ^ code ^ " (text) AS SELECT data FROM " ^ normalized_filename ^ " WHERE key = $1';"
//			"        EXECUTE 'PREPARE vardb_delete_" ^ code ^ " (text) AS DELETE FROM " ^ normalized_filename ^ " WHERE key = $1';"
//			"        EXECUTE 'PREPARE vardb_write_" ^ code ^ " (text, text) AS INSERT INTO " ^ normalized_filename ^ " (key, data) VALUES ($1, $2) ON CONFLICT (key) DO UPDATE SET data = $2';"
//			"    END IF;"
//			"END $$;";
//
			"DO $$"
			"BEGIN"
			// Delete any existing prepared statements first
			// In case they relate to a dropped and recreated table with the same name (but different oid)
			"    IF EXISTS (SELECT 1 FROM pg_prepared_statements WHERE name = 'vardb_read_" ^ code ^ "') THEN"
			"        EXECUTE 'DEALLOCATE vardb_read_" ^ code ^ "';"
			"        EXECUTE 'DEALLOCATE vardb_delete_" ^ code ^ "';"
			"        EXECUTE 'DEALLOCATE vardb_write_" ^ code ^ "';"
			"    END IF;"
			"    EXECUTE 'PREPARE vardb_read_" ^ code ^ " (text) AS SELECT data FROM " ^ normalized_filename ^ " WHERE key = $1';"
			"    EXECUTE 'PREPARE vardb_delete_" ^ code ^ " (text) AS DELETE FROM " ^ normalized_filename ^ " WHERE key = $1';"
			"    EXECUTE 'PREPARE vardb_write_" ^ code ^ " (text, text) AS INSERT INTO " ^ normalized_filename ^ " (key, data) VALUES ($1, $2) ON CONFLICT (key) DO UPDATE SET data = $2';"
			"END $$;"
		;

		if (not connection2.sqlexec(sql)) {
			throw VarDBException(var::lasterror());
		}
	}

	//var::setlasterror();

	// var becomes a file containing the filename and connection no
	var_str = normalized_filename.var_str;
	var_str.push_back(FM_);
	var_str.append(std::to_string(get_dbconn_no_or_default(connection2)));
	var_typ = VARTYP_NANSTR;

	// Cache the file so future opens return the same without reference to the database
	// Similar code in dbattach and open
	thread_file_handles[normalized_filename] = var_str;

	if (DBTRACE>1) UNLIKELY
		this->logputl("DBTR var::open-3 ");

	return true;
}

void var_db::close() const {

	THISIS("void var::close()")
	assertString(function_sig);
	/*TODO
		if (var_typ!=VARTYP_UNA) QMClose(var_int);
	*/
}

bool var_db::readf(in file, in key, const int fieldno) {

	// Read the whole record first
	if (not this->read(file, key)) UNLIKELY
		return false;

	// Extract the required field
	if (fieldno >= 0) {
		LIKELY
		var_str = this->f(fieldno).var_str;
		var_typ = VARTYP_STR;
	}

	return true;
}

bool var_db::readc(in file, in key) {

	THISIS("bool var::readc(in file, in key)")
	assertVar(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	// Check cache first, and return any cached record
	int dbconn_no = get_dbconn_no_or_default(file);
	if (not dbconn_no) UNLIKELY
		throw VarDBException("get_dbconn_no() failed");

	// Initialise the record var to unassigned
	// unless record and key are the same variable
	// in which case allow failure to read to leave the record (key) untouched
	// PICKOS leaves the variable untouched but we decide
	// to make use of a variable after unsuccessful read to be runtime error
	if (this != &key) {
		var_typ = VARTYP_UNA;
		var_str.clear();
	}

	// Attempt to get record from the cache
	auto hash64 = vardb_hash_file_and_key(file, key);
	std::string cachedrecord;
	if (thread_dbpool.getrecord(dbconn_no, hash64, cachedrecord)) {
		// Cache hit.
		if (cachedrecord.empty()) LIKELY {
			// Actual record doesnt exist.
			return false;
		} else {
			// Use record from cache.
			var_str = std::move(cachedrecord);
			var_typ = VARTYP_STR;
			return true;
		}
	}
	// Cache miss.

#ifdef EXO_TIMEBANK
	//Timer t2(247);//readc cache_miss
	Timer timer1(get_timebank_acno("bool var::readc cache_miss"));
#endif

	// Ordinary read from the database
	bool result = this->read(file, key);

	if (result) {
		// Cache a successful ordinary read
		thread_dbpool.putrecord(dbconn_no, hash64, var_str);
	} else {
		// Cache "" to falsify future reads without actual reads.
		var("").writec(file, key);
		thread_dbpool.putrecord(dbconn_no, hash64, "");
	}

	return result;
}

void var_db::writec(in file, in key) const {

	THISIS("void var::writec(in file, in key)")
	assertString(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	// Update cache
	// Virtually identical code in read and write/update/insert/delete
	int dbconn_no = get_dbconn_no_or_default(file);
	if (not dbconn_no)
		UNLIKELY
		throw VarDBException("get_dbconn_no() failed");

	auto hash64 = vardb_hash_file_and_key(file, key);
	thread_dbpool.putrecord(dbconn_no, hash64, var_str);

	return;
}

bool var_db::deletec(in key) const {

	THISIS("bool var::deletec(in key)")
	assertString(function_sig);
	ISSTRING(key)

	// Update cache
	// Virtually identical code in read and write/update/insert/delete
	int dbconn_no = get_dbconn_no_or_default(*this);
	if (not dbconn_no)
		UNLIKELY
		throw VarDBException("get_dbconn_no() failed");

	auto hash64 = vardb_hash_file_and_key(*this, key);
	return thread_dbpool.delrecord(dbconn_no, hash64);
}


////////
// XLATE - Lookup a db field given file name, key and field number.
////////

// fieldno can be "" to return the whole record (0 returns the key)
// TODO provide a version with int fieldno to handle the most frequent case
// although may also support dictid (of target file) instead of fieldno

var  var_db::xlate(in filename, in fieldno, const char* mode) const {

	THISIS("var  var::xlate(in filename, in fieldno, const char* mode) const")
	assertString(function_sig);
	ISSTRING(filename)
	// fieldnames are supported as exoprog::xlate
	// but not here in var::xlate which only supports field numbers since it has no
	// access to dictionaries
	ISNUMERIC(fieldno)

	// Use filename in case it is a file handle
	// and can give us a non-default connection
	auto file = filename;

	// We must open the file (probably cached)
	// to ensure that sql PREPARED statements are available.
	// Actually only needed during transactions
	// but checking that will increase work during transactions
	// while perhaps being only a little faster otherwise.
	if (!file.open(filename.lcase()))
		throw VarDBException("xlate: " ^ var::lasterror());

	char sep = fieldno.len() ? VM_ : RM_;

	// TODO implement multi-key read

	var response = "";
	int nmv = this->fcount(_VM);
	for (int vn = 1; vn <= nmv; ++vn) {

		//test every time instead of always appending and removing at the end
		//because the vast majority of xlate are single valued so it is faster
		if (vn > 1)
			response ^= sep;

		// read the record
		var key = this->f(1, vn);
		var record;
		if (!record.readc(file, key)) {
			// If record doesnt exist then "", or original key if mode is "C"

			// no record and mode C returns the key
			// gcc warning: comparison with string literal results in unspecified
			// behaviour if (mode=="C")
			if (*mode == 'C')
				response ^= key;

			// no record and mode X or anything else returns ""
			continue;
		}

		// extract the field or field 0 means return the whole record
		if (fieldno) {

			// numeric fieldno not zero return field
			// if (fieldno.isnum())

			// throw non-numeric error if fieldno not numeric
			response ^= record.f(fieldno);

			// non-numeric fieldno - cannot call calculate from here
			// return calculate(fieldno,filename,mode);
			continue;
		}

		// fieldno "" returns whole record
		if (!fieldno.len()) {
			response ^= record;
			continue;
		}

		// field no 0 returns key
		response ^= key;
	}
	//response.convert(FM^VM,"^]").outputl("RESPONSE=");
	return response;
}

bool var_db::read(in file, in key) {

	THISIS("bool var::read(in file, in key)")
	assertVar(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	// Initialise the record var to unassigned
	// unless record and key are the same variable
	// in which case allow failure to read to leave the record (key) untouched
	// PICKOS leaves the variable untouched but we decide
	// to make use of a variable after unsuccessful read to be runtime error
	if (this != &key) {
		var_typ = VARTYP_UNA;
		var_str.clear();
	}

	// dos or DOS file means osread/oswrite/osremove
	if (file.var_str.size() == 3 and (file.var_str == "dos" or file.var_str == "DOS")) UNLIKELY {
		//use osfilenames unnormalised so we can read and write as is
		return this->osread(key);  //.convert("\\",OSSLASH));
	}

	// TODO Lower case key if reading from dictionary

	const std::string key2 = key.normalize().var_str;

	// Asking to read DOS file! do osread using key as osfilename!
	if (file == "") UNLIKELY {
		let errmsg = "read(...) filename not specified, probably not opened.";
		throw VarDBException(errmsg);
	}

	// Reading a magic special key returns all keys in the file in natural order
	if (key == "%RECORDS%") UNLIKELY {

		var_str.clear();
		var_typ = VARTYP_STR;

		let sql =
			"SELECT key from " ^ get_normalized_filename(file) ^
			" WHERE NOT (key LIKE '%$%' AND key LIKE '%')"
			" ORDER BY key"
			" COLLATE exodus_natural"
			" LIMIT 65536";

		var response;
		if (not sqlexec(sql, response)) {
			var::loglasterror();
			return false;
		}
		if (response) {
			response.converter(_RM, _FM);
			response.cutter(4);
			var_str = std::move(response.var_str);
			return true;
		}
		return false;

	} // %RECORDS%

	// Get file specific connection or fail
	const auto pgconn = get_pgconn(file);
	if (not pgconn) UNLIKELY {
		let errmsg = "var::read() get_pgconn() failed for " ^ file;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size())};
	const int   paramFormats[] = {0}; // Text format

	DBresult dbresult;
	for (bool maybe_prepared = EXO_PREPARED;;) {

		let sql = maybe_prepared ?
			"vardb_read_" ^ get_normalized_filename(file).convert(".", "_")
		:
			"SELECT data FROM " ^ get_normalized_filename(file) ^ " WHERE key = $1";
		;
		// let sql = "EXECUTE " + prepared_code + " ($1)";
		DEBUG_LOG_SQL1

		dbresult = maybe_prepared ?
			XPQexecPrepared(
				pgconn,
				sql.var_str.c_str(), 1,       // One param (key)
				nullptr,                      // Dummy
				paramValues, paramLengths,
				nullptr,	                  // Text arguments
				0                             // Text results
			)
		:
			XPQexecParams(
				pgconn,
				sql.var_str.c_str(),
				1,                            // One param (key)
//				static_cast<const Oid[]>(25), // Text (Doesnt compile pre 24.04)
//				(const Oid[]){25},            // Text (Warning)
				nullptr,                      // Let the backend deduce param type
				paramValues, paramLengths,
				paramFormats,	              // Text arguments
				0                             // Text results
			)
		;

		// Handle serious errors
		if (PQresultStatus(dbresult) != PGRES_TUPLES_OK) UNLIKELY {
			let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
			var errmsg =
				"read(" ^ file.convert("." _FM, "_^").replace("dict_","dict.").quote() ^ ", " ^ key.quote() ^ ")";
			if (sqlstate == "42P01")
				errmsg ^= " File doesnt exist.";
			else if (sqlstate == "26000") {
				if (not pgconn.dbconn->in_transaction_) {
					maybe_prepared = false;
					continue;
				}
				errmsg ^= " file has not been opened/prepared and transaction is active.";
			} else
				errmsg ^= " " ^ (var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n") ^ ". sqlstate:" ^ sqlstate;

			var::setlasterror(errmsg);
			throw VarDBException(errmsg);
		}
		break;
	}

	if (PQntuples(dbresult) < 1) UNLIKELY {

		// Leave unassigned if not read

		var::setlasterror("ERROR: vardb: read(" ^ file.convert("." _FM, "_^").replace("dict_","dict.").quote() ^ ") record does not exist " ^
					key.quote());
		return false;
	}

	// A serious error
	if (PQntuples(dbresult) > 1) UNLIKELY {
		let errmsg = "ERROR: vardb: read() SELECT returned more than one record";
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	*this = getpgresultcell(dbresult, 0, 0);

	return true;
}

var  var_stg::hash(const std::uint64_t modulus) const {

	THISIS("var  var_stg::hash() const")
	assertVar(function_sig);
	assertString(function_sig);

	// not normalizing for speed and allow different non-normalised keys

	std::uint64_t hash64 = vardb_hash_stdstr(this->var_str);

	if (modulus)
		return var_int = hash64 % modulus;
	else
		return var_int = hash64;
}

// file doesnt not have to exist since all locks are actually numerical hashes
//
// Returns
// 0  - Failure
// 1  - Success
// "" - Failure - already locked and not in a transaction
// 2  - Success - already locked and in a transaction
var  var_db::lock(in key) const {

	// on postgres, repeated locks for the same thing (from the same connection) succeed and
	// stack up they need the same number of unlocks (from the same connection) before other
	// connections can take the lock unlock returns true if a lock (your lock) was released and
	// false if you dont have the lock NB return "" if ALREADY locked on this connection

	THISIS("var  var::lock(in key) const")
	assertVar(function_sig);
	ISSTRING(key)

	auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::lock() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	auto dbconn = get_dbconn(*this);

	// TODO consider preventing begintrans if lock cache not empty
	auto hash64 = vardb_hash_file_and_key(*this, key);

	// if already in lock cache
	//
	// then OUTSIDE transaction then FAIL with "" to indicate already locked
	//
	// then INSIDE transaction then SUCCEED with 2 to indicate already locked
	//
	// postgres allows and stacks up multiple locks whereas multivalue databases dont
	if (dbconn->locks_.contains(hash64)) {
		if (dbconn->in_transaction_)
			return 2; //SUCCESS TYPE ALREADY CACHED
		else
			return ""; //FAILURE TYPE ALREADY CACHED
	}

	// Parameter array
	const char* paramValues[] = {reinterpret_cast<char*>(&hash64)};
	const int paramLengths[] = {sizeof(std::uint64_t)};
	int paramFormats[] = {1};  // binary

	// Locks outside transactions remain for the duration of the connection and can be unlocked/relocked or unlockall'd
	// Locks inside transactions are unlockable and automatically unlocked on commit/rollback
	const char* sql = (dbconn->in_transaction_) ? "SELECT PG_TRY_ADVISORY_XACT_LOCK($1)" : "SELECT PG_TRY_ADVISORY_LOCK($1)";

	// Debugging
	if (DBTRACE) {
		if (DBTRACE>1)
			(this->assigned() ? *this : "").logputl("SQLL");
		var(sql).replace("$1)", ") file:" ^ *this ^ " key:" ^ key).convert("\t\n", "  ").trim().logputl("SQLL ");
	}

	// Call postgres
	const DBresult dbresult = XPQexecParams(
		pgconn,
		// TODO: parameterise filename
		sql, 1,                                     /* one param */
		nullptr,                                    /* let the backend deduce param type */
		paramValues, paramLengths, paramFormats, 1) /* ask for binary dbresults */
	;

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_TUPLES_OK or PQntuples(dbresult) != 1) UNLIKELY {
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		let errmsg = "lock(" ^ *this ^ ", " ^ key ^ ")\n" ^
			(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n") ^ ". sqlstate" ^ sqlstate ^ " PQresultStatus=" ^
			var(PQresStatus(PQresultStatus(dbresult))) ^ ", PQntuples=" ^
			var(PQntuples(dbresult));
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Add to lock cache if successful
	if (*PQgetvalue(dbresult, 0, 0) != 0) {
//		std::pair<const std::uint64_t, int> lock(hash64, 0);
//		dbconn->locks_.insert(lock);
		dbconn->locks_.insert(hash64);
		return 1;
	}

	// Otherwise indicate failure
	return 0;

}

bool var_db::unlock(in key) const {


	THISIS("void var::unlock(in key) const")
	assertVar(function_sig);
	ISSTRING(key)

	auto hash64 = vardb_hash_file_and_key(*this, key);

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::unlock() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	auto dbconn = get_dbconn(*this);

	// Unlock inside transaction has no effect
	if (dbconn->in_transaction_)
		return false;

	// If not in lock cache then return false
	if (not dbconn->locks_.contains(hash64))
		return false;

	// Remove from lock cache
	dbconn->locks_.erase(hash64);

	// Parameter array
	const char* paramValues[] = {reinterpret_cast<char*>(&hash64)};
	const int paramLengths[] = {sizeof(std::uint64_t)};
	int paramFormats[] = {1};//binary

	// $1=hashed filename and key
	const char* sql = "SELECT PG_ADVISORY_UNLOCK($1)";

	if (DBTRACE) {
		if (DBTRACE>1)
			(this->assigned() ? *this : "").logputl("SQLL ");
		var(sql).replace("$1)", ") file:" ^ *this ^ " key:" ^ key).convert("\t\n", "  ").trim().logputl("SQLU ");
	}

	// Call postgres
	const DBresult dbresult = XPQexecParams(pgconn,
											sql, 1,										/* one param */
											nullptr,									/* let the backend deduce param type */
											paramValues, paramLengths, paramFormats, 1); /* ask for binary results */

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_TUPLES_OK) UNLIKELY {
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		let errmsg = "unlock(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ ")\n" ^
				(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n") ^ "\nsqlstate" ^ sqlstate ^ " PQresultStatus=" ^
				var(PQresStatus(PQresultStatus(dbresult))) ^ ", PQntuples=" ^
				var(PQntuples(dbresult));
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Should return true
	return PQntuples(dbresult) == 1;
}

bool var_db::unlockall() const {

	THISIS("void var::unlockall() const")
	assertVar(function_sig);

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::unlockall() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	auto dbconn = get_dbconn(*this);

	// Locks in transactions cannot be cleared
	if (dbconn->in_transaction_)
		return false;

	// If lock cache is empty already then return true
	if (dbconn->locks_.empty())
		return true;

	// Clear the lock cache
	dbconn->locks_.clear();

	// Should return true
	return this->sqlexec("SELECT PG_ADVISORY_UNLOCK_ALL()");

}

// returns only success or failure and any response is logged and saved for future var::lasterror() call
bool var_db::sqlexec(in sql) const {
	var response = -1;	//no response required
	if (not this->sqlexec(sql, response)) UNLIKELY {
		var::setlasterror(response);
		return false;
	}
	return true;
}

// Returns success or failure
// response [out] data or errmsg
// response [in] can be preset to max number of tuples)
bool var_db::sqlexec(in sqlcmd, io response) const {

	THISIS("bool var::sqlexec(in sqlcmd, io response) const")
	ISSTRING(sqlcmd)

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		let errmsg = "var::sqlexec() get_pgconn() failed. " ^ (this->assigned() ? *this : "");
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Log the sql command
	if (DBTRACE) {
		if (DBTRACE>1)
			(this->assigned() ? *this : "").trim("\n").logputl("SQLE ");
//		sqlcmd.convert("\t\n", "  ").trim().logputl("SQLE ");
	    std::clog << pgconn.dbconn->dbconn_no_ << ": SQLE " << sqlcmd.convert("\n\t", "  ").trim() << "\n";
	}

	// will contain any dbresult IF successful

	// NB PQexec cannot be told to return binary results
	// but it can execute multiple commands
	// whereas XPQexecParams is the opposite
	DBresult dbresult = XPQexec(pgconn, sqlcmd.var_str.c_str());

	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK and
		PQresultStatus(dbresult) != PGRES_TUPLES_OK) UNLIKELY {
//		if (!dbresult) {
//			response = dbresult.pqerrmsg;
//			return false;
//		}
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		// sql state 42P03 = duplicate_cursor
		response = "var::sqlexec: " ^ (var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n") ^ ". sqlstate:" ^ sqlstate ^ " " ^ sqlcmd;
		return false;
	}

	//errmsg = var(PQntuples(dbresult));

	// Quit if no rows/columns provided or no response required (integer<=0)
	int nrows = PQntuples(dbresult);
	int ncols = PQnfields(dbresult);
	if (nrows == 0 or ncols == 0 or (response.assigned() and ((response.var_typ & VARTYP_INT) and response <= 0))) {
		response = "";
		return true;
	}

	// Option to limit number of rows returned
	if (response.assigned() and response.isnum() and response < nrows and response)
		nrows = response;

	response = "";

	// First row is the column/field names
	for (int coln = 0; coln < ncols; ++coln) {
		response.var_str.append(PQfname(dbresult, coln));
		response.var_str.push_back(FM_);
	}
	response.var_str.pop_back();

	// Output the rows/columns as records separated by RM and fields separated by FM
	for (int rown = 0; rown < nrows; rown++) {
		response.var_str.push_back(RM_);
		for (int coln = 0; coln < ncols; ++coln) {
			response.var_str.append(PQgetvalue(dbresult, rown, coln));
			response.var_str.push_back(FM_);
		}
		response.var_str.pop_back();
	}

	return true;
}

// writef writes a specific field number in a record
void var_db::writef(in file, in key, const int fieldno) const {

	// Just write the whole record if field no isnt positive
	if (fieldno <= 0) {
		UNLIKELY
		write(file, key);
		return;
	}

	THISIS("void var::writef(in file, in key, const int fieldno) const")
	assertString(function_sig);
//	// will be duplicated in read and write but do here to present the correct function name on
//	// error
//	ISSTRING(file)
//	ISSTRING(key)

	// Get the old record or use ""
	var record;
	if (not record.read(file, key))
		record = "";

	// Replace the field
	record(fieldno) = var_str;

	// Write it back
	record.write(file, key);

	return;
}

/* "prepared statement" version doesnt seem to make much difference approx -10% - possibly because
two field file is so simple void var::write(in file, in key) const {}
*/

//"update if present or insert if not" is handled in postgres using ON CONFLICT clause
void var_db::write(in file, in key) const {

	THISIS("void var::write(in file, in key) const")
	assertString(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	const std::string key2 = key.normalize().var_str;
	const std::string data2 = this->normalize().var_str;

	// file dos or DOS means osread/oswrite/osremove
	if (file.var_str.size() == 3 and (file.var_str == "dos" or file.var_str == "DOS")) UNLIKELY {
		//this->oswrite(key2); //.convert("\\",OSSLASH));
		//use osfilenames unnormalised so we can read and write as is
		if (not this->oswrite(key)) {
			throw VarError("Could not oswrite " ^ key.squote() ^ ". " ^ var::lasterror());
		}
		return;
	}

	// Clear any cache when writing actual records
	// to prevent any future readc() getting known obsolete records
	file.deletec(key2);

	const auto pgconn = get_pgconn(file);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::write() get_pgconn() failed. ";
		errmsg ^= file ^ ", " ^ key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data(),
	                              data2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size()),
	                              static_cast<int>(data2.size())};

	for (bool maybe_prepared = EXO_PREPARED;;) {

		let sql = maybe_prepared ?
			"vardb_write_" ^ get_normalized_filename(file).convert(".", "_")
		:
			"INSERT INTO " ^ get_normalized_filename(file) ^ " (key,data) values( $1 , $2)"
			" ON CONFLICT (key)"
			" DO UPDATE SET data = $2"
		;
		// let sql = "EXECUTE " + prepared_code + " ($1)";
		DEBUG_LOG_SQL1

		const DBresult dbresult = maybe_prepared ?
			XPQexecPrepared(
				pgconn,
				sql.var_str.c_str(),
				2,                         // Two params (key and data)
				nullptr,                   // Dummy
				paramValues, paramLengths,
				nullptr,	               // Text arguments
				0                          // Text results
			)
		:
			XPQexecParams(
				pgconn,
				sql.var_str.c_str(),
				2,                          // two params (key and data)
				nullptr,                    // let the backend deduce param type
				paramValues, paramLengths,
				nullptr,                    // text arguments
				0                           // text results
			)
		;

		// Handle serious errors
		if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) UNLIKELY {
			let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
			var errmsg = "ERROR: vardb: write(" ^ file.convert(_FM, "^") ^
						", " ^ key ^ ")";
			if (sqlstate == "26000") {
				if (not pgconn.dbconn->in_transaction_) {
					maybe_prepared = false;
					continue;
				}
				errmsg ^= " file has not been opened/prepared and transaction is active.";
			} else {
				errmsg ^= " failed: sqlstate" ^ sqlstate ^ " PQresultStatus=" ^
						var(PQresStatus(PQresultStatus(dbresult))) ^ " " ^
						(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n");
			}
			// ERROR: vardb: write(definitions^1, LAST_SYNCDATE_TIME*DAT) failed: sqlstate25P02
			// PQresultStatus=PGRES_FATAL_ERROR ERROR:  current transaction is aborted, commands ignored until end of transaction block

			throw VarDBException(errmsg);
		}
		break;
	}

	// success if inserted or updated 1 record
	//return std::strcmp(PQcmdTuples(dbresult), "1") != 0;

	return;
}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var_db::updaterecord(in file, in key) const {

	THISIS("bool var::updaterecord(in file, in key) const")
	assertString(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	// Clear any cache when updating actual records
	// to prevent any future readc getting known obsolete records
	file.deletec(key);

	const std::string key2 = key.normalize().var_str;
	const std::string data2 = this->normalize().var_str;

	const auto pgconn = get_pgconn(file);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::updaterecord() get_pgconn() failed. ";
		errmsg ^= file ^ ", " ^ key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data(),
	                              data2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size()),
	                              static_cast<int>(data2.size())};

	// not EXO_PREPARED
	let sql = "UPDATE "  ^ get_normalized_filename(file) ^ " SET data = $2 WHERE key = $1";
	DEBUG_LOG_SQL1

	const DBresult dbresult = XPQexecParams(
		pgconn,
		sql.var_str.c_str(),
		2,        // two params (key and data)
		nullptr,  // let the backend deduce param type
		paramValues, paramLengths,
		nullptr,  // text arguments
		0         // text results
	);

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) UNLIKELY {
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		let errmsg = "ERROR: vardb: update(" ^ file.convert(_FM, "^") ^
				", " ^ key ^ ") sqlstate " ^ sqlstate ^ " Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
				(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n");
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// If not updated 1 then fail
	if (std::strcmp(PQcmdTuples(dbresult), "1") != 0) UNLIKELY {
		var(
			"ERROR: vardb: updaterecord(" ^ file.convert(_FM, "^") ^ ", " ^ key ^ ") Failed: " ^
			var(PQntuples(dbresult)) ^ " " ^
			(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n")
		).errputl();
		return false;
	}

	return true;
}

//"updatekey" is non-standard for pick - but allows changing record keys without rewriting records.

bool var_db::updatekey(in key, in newkey) const {

	THISIS("bool var::updatekey(in key, in newkey) const")
	assertString(function_sig);
	ISSTRING(key)
	ISSTRING(newkey)

	// Clear any cache when updating actual records
	// to prevent any future readc getting known obsolete records
	this->deletec(key);
	this->deletec(newkey);

	const std::string key2    = key.normalize().var_str;
	const std::string newkey2 = newkey.normalize().var_str;

	const auto pgconn = get_pgconn(*this);
	if (!pgconn) UNLIKELY {
		let errmsg =
			"ERROR: vardb: updatekey(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ " -> " ^ newkey ^ ") "
			"get_pgconn() failed. ";
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data(),
	                              newkey2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size()),
	                              static_cast<int>(newkey2.size())};

	// not EXO_PREPARED
	let sql = "UPDATE "  ^ get_normalized_filename(*this) ^ " SET key = $2 WHERE key = $1";
	DEBUG_LOG_SQL1

	const DBresult dbresult = XPQexecParams(
		pgconn,
		sql.var_str.c_str(),
		2,        // two params (key and newkey)
		nullptr,  // let the backend deduce param type
		paramValues, paramLengths,
		nullptr,  // text arguments
		0         // text results
	);

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) UNLIKELY {
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));

		let errmsg =
			"ERROR: vardb: updatekey(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ " -> " ^ newkey ^ ") "
			"sqlstate " ^ sqlstate ^ " Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
			(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n");
		var::setlasterror(errmsg);

		// Duplicate key is a normal error. Do not throw
		if (sqlstate == 23505) {
			// sqlstate 23505 Failed: 0 ERROR:  duplicate key value violates unique constraint "xo_test_db_deleterecord_temp_pkey"
			return false;
		}

		throw VarDBException(errmsg);
	}

	// if not updated 1 then fail
	if (std::strcmp(PQcmdTuples(dbresult), "1") != 0) UNLIKELY {
		var(
			"ERROR: vardb: updatekey(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ " -> " ^ newkey ^ ") " ^
			var(PQntuples(dbresult)) ^ " " ^
			(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n")
		).errputl();
		return false;
	}

	return true;
}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt
// already exist" logic

bool var_db::insertrecord(in file, in key) const {

	THISIS("bool var::insertrecord(in file, in key) const")
	assertString(function_sig);
	ISSTRING(file)
	ISSTRING(key)

	// Clear any cache when inserting actual records
	// although there should be none when inserting new records
	// to prevent any future readc getting known obsolete records
	file.deletec(key);

	const std::string key2 = key.normalize().var_str;
	const std::string data2 = this->normalize().var_str;

	const auto pgconn = get_pgconn(file);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::insertrecord() get_pgconn() failed. ";
		errmsg ^= file ^ ", " ^ key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data(),
	                              data2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size()),
	                              static_cast<int>(data2.size())};

	// no EXO_PREPARED
	let sql =
		"INSERT INTO " ^ get_normalized_filename(file) ^ " (key,data) values( $1 , $2)";
	DEBUG_LOG_SQL1

	const DBresult dbresult = XPQexecParams(
		pgconn,
		sql.var_str.c_str(),
		2,       // two params (key and data)
		nullptr, // let the backend deduce param type
		paramValues, paramLengths,
		nullptr, // text arguments
		0        // text results
	);

	// Handle serious errors or ordinary duplicate key failure (which will mess us transactionsa)
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) UNLIKELY {

		// "duplicate key value violates unique constraint"
		let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		if (sqlstate == 23505) UNLIKELY
			return false;

		let errmsg = "ERROR: vardb: insertrecord(" ^
				file.convert(_FM, "^") ^ ", " ^ key ^ ") Failed: " ^
				var(PQntuples(dbresult)) ^ " sqlstate" ^ sqlstate ^ " " ^
				(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n");
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// If not updated 1 then fail
	if (std::strcmp(PQcmdTuples(dbresult), "1") != 0) UNLIKELY {
		var("ERROR: vardb: insertrecord(" ^ file.convert(_FM, "^") ^
			", " ^ key ^ ") Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
			(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n"))
			.errputl();
		return false;
	}

	return true;
}

bool var_db::deleterecord(in key) const {

	THISIS("bool var::deleterecord(in key) const")
	assertString(function_sig);
	ISSTRING(key)

	// Clear any cache when deleting actual records
	// to prevent and future readc getting known non-existent records
	this->deletec(key);

	const std::string key2 = key.normalize().var_str;

	// File dos or DOS means osread/oswrite/osremove
	if (var_str.size() == 3 and (var_str == "dos" or var_str == "DOS")) UNLIKELY {
		//use osfilenames unnormalised so we can read and write as is
		return (key.osfile() and key.osremove());
	}

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		var errmsg = "var::deleterecord() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[]  = {key2.data()};
	const int   paramLengths[] = {static_cast<int>(key2.size())};
	const int   paramFormats[] = {0}; // Text format

	DBresult dbresult;
	for (bool maybe_prepared = EXO_PREPARED;;) {

		let sql = maybe_prepared ?
			"vardb_delete_" ^ get_normalized_filename(*this).convert(".", "_")
		:
			"DELETE FROM " ^ get_normalized_filename(*this) ^ " WHERE KEY = $1"
		;
		// let sql = "EXECUTE " + prepared_code + " ($1)";
		DEBUG_LOG_SQL1

		dbresult = maybe_prepared ?
			XPQexecPrepared(
				pgconn,
				sql.var_str.c_str(), 1,       // One param (key)
				nullptr,                      // Dummy
				paramValues, paramLengths,
				nullptr,	                  // Text arguments
				0                             // Text results
			)
		:
			XPQexecParams(
				pgconn,
				sql.var_str.c_str(),
				1,                            // One param (key)
//				static_cast<const Oid[]>(25), // Text (Doesnt compile pre 24.04)
//				(const Oid[]){25},            // Text (Warning)
				nullptr,                      // Let the backend deduce param type
				paramValues, paramLengths,
				paramFormats,	              // Text arguments
				0                             // Text results
			)
		;

		// Handle serious errors
		if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) UNLIKELY {
			let sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
			var errmsg = "ERROR: vardb: deleterecord(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ ")";
			if (sqlstate == "26000") {
				if (not pgconn.dbconn->in_transaction_) {
					maybe_prepared = false;
					continue;
				}
				errmsg ^= " file has not been opened/prepared and transaction is active.";
			} else {
				errmsg ^= " sqlstate " ^ sqlstate ^ " Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
					(var(dbresult.pqerrmsg) ?: var(PQerrorMessage(pgconn))).trimlast("\n");
			}
			throw VarDBException(errmsg);
		}
		break;
	}

	// If not updated 1 then fail
	bool result;
	if (std::strcmp(PQcmdTuples(dbresult), "1") != 0) {
		if (DBTRACE>1)
			var("var::deleterecord(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ ") failed. Record does not exist")
				.errputl();
		result = false;
	} else
		result = true;

	return result;
}

void var_db::clearcache() const {

	THISIS("bool var::clearcache() const")
	assertVar(function_sig);

	int dbconn_no = get_dbconn_no_or_default(*this);
	if (not dbconn_no) UNLIKELY
//		throw VarDBException("get_dbconn_no() failed in clearcache");
		return;

	thread_dbpool.clearcache(dbconn_no);

	// Warning if any cursors have not been closed/cleaned up.
	for (auto& entry : thread_dbresults)
		var(entry.first).errputl("clearcache: WARNING: Cursor not cleaned up. Clearing it now.");

	// Clean up cursors - RAII/SBRM will call PQClear on the related PGresult* objects
	thread_dbresults.clear();

	return;
}

// Function to get transaction status as a string
var getTransactionStatus(PGconn* conn) {
    switch (PQtransactionStatus(conn)) {
        case PQTRANS_IDLE: return "IDLE";
        case PQTRANS_ACTIVE: return "ACTIVE";
        case PQTRANS_INTRANS: return "INTRANS";
        case PQTRANS_INERROR: return "INERROR";
        case PQTRANS_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

bool var_db::begintrans() const {

	THISIS("bool var::begintrans() const")
	assertVar(function_sig);

	// Clear the record cache
	this->clearcache();

	// begin a transaction
	// Read Committed is the default isolation level in PostgreSQL.
	// https://www.postgresql.org/docs/current/transaction-iso.html#XACT-READ-COMMITTED
	if (not this->sqlexec("BEGIN TRANSACTION ISOLATION LEVEL READ COMMITTED")) UNLIKELY
		return false;

	auto dbconn = get_dbconn(*this);

	let status = getTransactionStatus(dbconn->pgconn_);
	// Only active while an (async?) SQL command is in progress.
//	if (status != "INTRANS" && status != "ACTIVE") {
	if (status != "INTRANS") {
		var::setlasterror("Failed to begin transaction. Status: " ^ status);
		return false;
	}

	// Change status
	// ESSENTIAL Used to change locking type to PER TRANSACTION
	// so all locks persist until after commit i.e. cannot be specifically unlocked
	dbconn->in_transaction_ = true;

	return true;

}

bool var_db::rollbacktrans() const {

	THISIS("bool var::rollbacktrans() const")
	assertVar(function_sig);

	// Clear the record cache
	this->clearcache();

	// Rollback a transaction
	if (not this->sqlexec("ROLLBACK")) UNLIKELY
		return false;

	auto dbconn = get_dbconn(*this);

	// Change status
	dbconn->in_transaction_ = false;

	// Clear the lock cache
	dbconn->locks_.clear();

	return true;
}

bool var_db::committrans() const {

	THISIS("bool var::committrans() const")
	assertVar(function_sig);

	// Clear the record cache
	this->clearcache();

	auto dbconn = get_dbconn(*this);

	bool result = true;

	// Change status regardless
	dbconn->in_transaction_ = false;

	// Clear the lock cache regardless of status
	dbconn->locks_.clear();

	let status = getTransactionStatus(dbconn->pgconn_);

	// Rollback if in error and indicate failure
	if (status == "INERROR") {
		if (this->rollbacktrans())
			var::setlasterror("");
		else
			var::lasterror().errputl();
		var::setlasterror("COMMITTRANS: Failed and rolled back due to status:" ^ status ^ " " ^ var::lasterror());
		return false;
	}

	// Indicate failure if status incorrect.
	if (status != "INTRANS" && status != "ACTIVE" && status != "IDLE") {
		result = false;
		var::setlasterror("COMMITTRANS: Failed. ROLLBACK due to transaction status: " ^ status);
	}

	// End (commit) a transaction.
	// If transaction status is INERROR then Postgresql would rollback and return OK (rather unexpected).
	// BUT we filtered out INERROR above and return false to make testing committrans() for success easy.
	if (not this->sqlexec("END")) UNLIKELY
		result = false;

	return result;

}

bool var_db::statustrans() const {

	THISIS("bool var::statustrans() const")
	assertVar(function_sig);

	auto dbconn = get_dbconn(*this);

	return dbconn->in_transaction_;

}

// sample code
// var().dbcreate("mynewdb");//create a new database on the current thread-default connection
// var file;
// file.open("myfile");
// file.dbcreate("mynewdb");//creates a new db on the same connection as a file was opened on
// var connectionhandle;
// connectionhandle.connect("connection string pars");
// connectionhandle.dbcreate("mynewdb");

bool var_db::dbcopy(in from_dbname, in to_dbname) const {
	THISIS("bool var::dbcopy(in from_dbname, in new_dbname) const")
	assertVar(function_sig);
	ISSTRING(from_dbname)
	ISSTRING(to_dbname)

	// Ensure to_dbname is provided to avoid bugs in app code
	if (not to_dbname) {
		var::setlasterror("dbcopy() requires new_dbname");
		return false;
	}

	return this->dbcreate(to_dbname, from_dbname);
}

bool var_db::dbcreate(in new_dbname, in old_dbname /* = "" */) const {

	THISIS("bool var::dbcreate(in new_dbname, in old_dbname) const")
	assertVar(function_sig);
	ISSTRING(new_dbname)
	ISSTRING(old_dbname)

	// TODO Fail neatly if the source db doesnt exist or the target db already exists

	// Prepare the SQL
	var sql = "CREATE DATABASE " ^ new_dbname ^ " WITH";
	sql ^= " ENCODING='UTF8' ";

	// Optionally copy from an existing database
	if (old_dbname)
		sql ^= " TEMPLATE " ^ old_dbname;

	// Create the database
	if (not this->sqlexec(sql)) UNLIKELY {
		return false;
	}

	// Connect to the new db
	var newdb;
	if (not newdb.connect(new_dbname)) UNLIKELY {
		return false;
	}

	// Add dict schema to allow creation of dict files like dict.xxxxxxxx
	sql = "CREATE SCHEMA IF NOT EXISTS dict";
	var result = true;
	if (not newdb.sqlexec(sql)) UNLIKELY {
		result = false;
	}

	// Disconnect from the new database
	newdb.disconnect();

	return result;

}

bool var_db::dbdelete(in dbname) const {

	THISIS("bool var::dbdelete(in dbname)")
	assertVar(function_sig);
	ISSTRING(dbname)

	// Fail neatly if the database does not exist.
	// SQL errors during a transaction cause the whole transaction to fail.
	if (not this->clone().dblist().lower().locate(dbname)) UNLIKELY {
		var::setlasterror(dbname.quote() ^ " db does not exist.");
		return false;
	}

	return this->sqlexec("DROP DATABASE " ^ dbname);
}

bool var_db::createfile(in filename) const {

	THISIS("bool var::createfile(in filename)")
	assertVar(function_sig);
	ISSTRING(filename)

	// var tablename = "TEMP" ^ var(100000000).rnd();
	// Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but
	// has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default
	// behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON
	// COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	// Fail neatly if the file already exists.
	// SQL errors during a transaction cause the whole transaction to fail.
	if (this->clone().open(filename)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " already exists.");
		return false;
	}

	// sql
	var sql = "CREATE";
	if (filename.ends("_temp") and not filename.starts("dict.", "DICT."))
		sql ^= " TEMPORARY ";
	sql ^= " TABLE " ^ get_normalized_filename(filename);
	sql ^= " (key text primary key, data text)";

	// Create it
	//     if (not (this->assigned() ? *this : filename).sqlexec(sql))
	if (not this->sqlexec(sql))
		return false;

	// Open it as well to get prepared statements implemented by open
	if (not this->clone().open(filename, *this)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " created but cannot be opened.");
		return false;
	}

	return true;
}

bool var_db::renamefile(in filename, in newfilename) const {

	THISIS("bool var::renamefile(in filename, in newfilename)")
	assertVar(function_sig);
	ISSTRING(filename)
	ISSTRING(newfilename)

	// Fail neatly if the old file does not exist.
	// SQL errors during a transaction cause the whole transaction to fail.
	// Make sure we use the right connection
//	var file;
//	if (not file.open(filename, *this)) UNLIKELY {
	if (not this->clone().open(filename, *this)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " cannot be renamed because it does not exist.");
		return false;
	}

	// Fail neatly if the new file exists
	// SQL errors during a transaction cause the whole transaction to fail.
//	if (file.open(newfilename, *this)) UNLIKELY {
	if (this->clone().open(newfilename, *this)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " cannot be renamed because " ^ newfilename.quote() ^ " already exists.");
		return false;
	}

	// Remove from the cache of file handles
//	file.detach(filename);
	this->clone().detach(filename);

	let sql = "ALTER TABLE " ^ filename ^ " RENAME TO " ^ newfilename;

//	if (not file.sqlexec(sql)) UNLIKELY
	if (not this->sqlexec(sql)) UNLIKELY
		throw VarDBException(var::lasterror());

	return true;
}

bool var_db::deletefile(in filename) const {

	THISIS("bool var::deletefile(in filename)")
	assertVar(function_sig);
	ISSTRING(filename)

	// Fail neatly if the file does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
	// Delete the file on whatever connection it exists;
//	var file;
//	if (not file.open(filename, *this)) UNLIKELY {
	if (not this->clone().open(filename, *this)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " cannot be deleted because it does not exist.");
		return false;
	}

	// Remove from file cache regardless of success or failure to deletefile
	// Delete from cache AFTER the above open which will place it in the cache
	// Similar code in detach and deletefile
	if (thread_file_handles.erase(get_normalized_filename(filename))) {
		//filename.errputl("::deletefile ==== Connection cache DELETED = ");
	} else {
		//filename.errputl("::deletefile ==== Connection cache NONE    = ");
	}

	let sql = "DROP TABLE " ^ filename.f(1) ^ " CASCADE";

	if (not this->clone().sqlexec(sql)) UNLIKELY
		throw VarDBException(var::lasterror());

	return true;
}

bool var_db::clearfile(in filename) const {

	THISIS("bool var::clearfile(in filename)")
	assertVar(function_sig);
	ISSTRING(filename)

	// Fail neatly if the file does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
//	var file;
//	if (not file.open(filename, *this)) UNLIKELY {
	if (not this->clone().open(filename, *this)) UNLIKELY {
		var::setlasterror(filename.quote() ^ " cannot be cleared because it does not exist.");
		return false;
	}

	let sql = "DELETE FROM " ^ filename.f(1);
//	if (not file.sqlexec(sql)) UNLIKELY
	if (not this->sqlexec(sql)) UNLIKELY
		throw VarDBException(var::lasterror());

	return true;
}

static var get_dictexpression(in cursor, in mainfilename, in filename, in dictfilename, in dictfile, in fieldname0, io joins, io unnests, io selects, bool& ismv, bool& isdatetime, bool forsort) {

	//cursor is required to join any calculated fields in any second pass

	ismv = false;

	var fieldname = fieldname0.convert(".", "_");
	var actualdictfile = dictfile;

	// Open dict.xxxx or dict.voc on the default dict connection or throw an error
	if (not actualdictfile) {

		// The dictionary of all dict. files is dict.voc. Used when selecting any dict. file.
		var dictfilename;
		if (mainfilename.lcase().starts("dict.", "DICT."))
			dictfilename = "dict.voc";
		else
			dictfilename = "dict." ^ mainfilename;

		// If dict .mainfile is not available, use dict.voc
//		if (not actualdictfile.open(dictfilename)) {
		if (not actualdictfile.open(dictfilename, dictfile)) {
			dictfilename = "dict.voc";
//			if (not actualdictfile.open(dictfilename)) UNLIKELY {
			if (not actualdictfile.open(dictfilename, dictfile)) UNLIKELY {
				let errmsg = "get_dictexpression() cannot open " ^ dictfilename.quote();
				throw VarDBException(errmsg);
			}
		}
	}

	// If doing 2nd pass then calculated fields have been placed in a parallel temporary file
	// and their column names appended with a colon (:)
	var stage2_calculated = fieldname.ends(":");
	let stage2_filename = "SELECT_CURSOR_STAGE2_" ^ cursor.f(1);

	if (stage2_calculated) {
		fieldname.popper();
		// Create a pseudo look up to SELECT_CURSOR_STAGE2 temporary file created by stage 1/2 select
		// which has the fields stored in sql columns and not in the usual data column
		stage2_calculated = "/" "/@ANS=XLATE(\"" ^ stage2_filename ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
		if (not var().isterminal())
			stage2_calculated.logputl("stage2_calculated simulation --------------------->");
	}

	// Given a file and dictionary id
	// returns a postgres sql expression like (exodus.extract_text(filename.data,99,0,0))
	// using one of the exodus backend functions installed in postgres like exodus.extract_text,
	// exodus.extract_date etc.

	// Get the dictionary record from actualdictfile
	// If not found after trying all the below combinations,
	// and if actualdictfile is not dict.voc,
	// then try the same search in dict.voc.
	//
	// e.g. Abc tries Abc, abc, ABC
	// e.g. abc tries abc, ABC
	// e.g. ABC tries ABC, abc

	// Try the possibly mixed case dict record key as provided.
	var dictrec;
	if (not dictrec.read(actualdictfile, fieldname)) UNLIKELY {

		// Try lower case
		let origfieldname = fieldname;
		fieldname.lcaser();
		if (fieldname != origfieldname and dictrec.read(actualdictfile, fieldname)) {
		} else {

			// Try upper case
			fieldname.ucaser();
			if (fieldname != origfieldname and dictrec.read(actualdictfile, fieldname)) {
			} else {

				// Try in dict.voc original case
				// only if file is not already dict.voc
				fieldname = origfieldname;
				bool not_dict_voc = actualdictfile.f(1) != "dict.voc";
				if (not_dict_voc and dictrec.read("dict.voc", fieldname)) {
					actualdictfile = "dict.voc";
				} else {

					// Try in dict.voc lowercase
					fieldname.lcaser();
					if (not_dict_voc and fieldname != origfieldname and dictrec.read("dict.voc", fieldname)) {
						actualdictfile = "dict.voc";
					} else {

						// Try in dict.voc uppercase
						fieldname.ucaser();
						if (not_dict_voc and fieldname != origfieldname and dictrec.read("dict.voc", fieldname)) {
							actualdictfile = "dict.voc";
						} else {

							// The dict rec for field name ID can be synthesised
							if (fieldname == "@ID" or fieldname == "ID") {
								dictrec = "F^0^Ref^^^^^^15"_var;
							}

							// Give up
							else UNLIKELY {
								let errmsg =
									"get_dictexpression() cannot read " ^ fieldname.quote() ^
									" from " ^ actualdictfile.convert(FM, "^").quote() ^
									(not_dict_voc ? " or \"dict.voc\"" : "");
								throw VarDBException(errmsg);
							}
						}
					}
				}
			}
		}
	}

	//create a pseudo look up. to trigger JOIN logic to the table that we stored
	//Note that SELECT_TEMP has the fields stored in sql columns and not in the usual data column
	if (stage2_calculated) {
		//dictrec.r(8, stage2_calculated);
		dictrec(8) = stage2_calculated;
	}

	let dicttype = dictrec.f(1);
	let fieldno = dictrec.f(2);
	let conversion = dictrec.f(7);

	bool isinteger = conversion == "[NUMBER,0]" or dictrec.f(11) == "0N" or
					dictrec.f(11).starts("0N_");
	bool isdecimal = conversion.starts("MD") or conversion.starts("[NUMBER") or
					dictrec.f(12) == "FLOAT" or dictrec.f(11).contains("0N");
	//dont assume things that are R are numeric
	//eg period 1/19 is right justified but not numeric and sql select will crash if ::float8 is used
	//ordictrec.f(9) == "R";
	bool isnumeric = isinteger or isdecimal or dictrec.f(9) == "R";
	bool ismv1 = dictrec.f(4).starts("M");
	var fromjoin = false;

	bool isdate = conversion.starts("D") or conversion.starts("[DATE");
	bool istime = not isdate and (conversion.starts("MT") or conversion.starts("[TIME"));
	isdatetime = isdate or istime;

	var sqlexpression;
	if (dicttype == "F") {

		// Field 0 means key field
		if (not fieldno) {

			if (forsort and not isdate and not istime)
				sqlexpression =
					"exodus.extract_sort(" ^ mainfilename ^ ".key,0,0,0)";
			else
				sqlexpression = get_fileexpression(mainfilename, filename, "key");

			// Multipart key - extract relevent field based on "*" separator
			let keypartn = dictrec.f(5);
			if (keypartn) {
				sqlexpression =
					"split_part(" ^ sqlexpression ^ ", '*', " ^ keypartn ^ ")";
			}

			// Example of multipart key and date conversion
			// select date '1967-12-31' + split_part(convert_from(key, 'UTF8'),
			// '*',2)::integer from filename
			if (isdate)
				sqlexpression =
					//"date('1967-12-31') + " ^ sqlexpression ^ "::integer";
					// cannot seem to use + on dates in indexes
					//therefore
					"exodus.extract_date(" ^ sqlexpression ^ ",0,0,0)";
			else if (istime)
				sqlexpression =
					"exodus.extract_time(" ^ sqlexpression ^ ",0,0,0)";

			if (DBTRACE>1) {
				TRACE(fieldno)
				TRACE(sqlexpression)
			}

			return sqlexpression;

		} // of key field, Fieldno = 0

		let extractargs =
			get_fileexpression(mainfilename, filename, "data") ^ "," ^ fieldno ^ ", 0, 0)";

		if (conversion.starts("[DATETIME"))
			sqlexpression = "exodus.extract_datetime(" ^ extractargs;

		else if (isdate)
			sqlexpression = "exodus.extract_date(" ^ extractargs;

		else if (istime)
			sqlexpression = "exodus.extract_time(" ^ extractargs;

		// for now (until we have a extract_number/integer/float) that doesnt fail on
		// non-numeric like cast "as integer" and "as float" does note that we could use
		// exodus.extract_sort for EVERYTHING inc dates/time/numbers etc. but its large size
		// is perhaps a disadvantage
		else if (forsort)
			sqlexpression = "exodus.extract_sort(" ^ extractargs;

		else if (isnumeric)
			sqlexpression = "exodus.extract_number(" ^ extractargs;

		else
			sqlexpression = "exodus.extract_text(" ^ extractargs;

	} // of dict type F

	else if (dicttype == "S") {
		var function_src = dictrec.f(8).trim();
		while (function_src.f(1,1).contains("#include "))
			function_src.remover(1,1);
		var pgsql_pos = function_src.index(_VM "/" "*pgsql");
//		bool is_ans_xlate = (not ismv1 or stage2_calculated) and function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate(");
//TRACE(is_ans_xlate);// 0 ??
//		is_ans_xlate = ((not ismv1 or stage2_calculated) and function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate("));
//TRACE(is_ans_xlate);// 0 ??
		bool x = (not ismv1 or stage2_calculated);
		bool y = function_src.trimfirst("\t /").convert(" ","").lcase().starts("@ans=xlate(");
		bool is_ans_xlate = x and y;
		if (DBTRACE>1) {
			TRACE(pgsql_pos)
			TRACE(function_src.f(1,1))
			TRACE(is_ans_xlate)
		}

//TRACE(ismv1)
//TRACE(stage2_calculated)
//TRACE(x)
//TRACE(function_src)
//TRACE(function_src.trimfirst("\t /"))
//TRACE(function_src.trimfirst("\t /").lcase())
//TRACE(y)
//TRACE(pgsql_pos)
/*
TRACE: dicttype="S"
TRACE: ismv1="0"
TRACE: stage2_calculated="//@ANS=XLATE("SELECT_CURSOR_STAGE2_1",@ID,CPP_TEXT_calc,"X")"
TRACE: x="1"
TRACE: function_src="//@ANS=XLATE("SELECT_CURSOR_STAGE2_1",@ID,CPP_TEXT_calc,"X")"
TRACE: function_src.trimfirst("\t /")="@ANS=XLATE("SELECT_CURSOR_STAGE2_1",@ID,CPP_TEXT_calc,"X")"
TRACE: function_src.trimfirst("\t /").lcase()="@ans=xlate("select_cursor_stage2_1",@id,cpp_text_calc,"x")"
TRACE: y="1"
TRACE: is_ans_xlate="0"
TRACE: pgsql_pos="0"
TRACE: sqlexpression=""
TRACE: "QQQ"="QQQ"
*/
		// sql expression available
		///////////////////////////
		sqlexpression = dictrec.f(17);
		//TRACE(sqlexpression)
		if (sqlexpression) {
			//TRACE("has sqlexpression")
			// return sqlexpression;
		}
		// sql function available
		/////////////////////////
		// eg dict_schedules_PROGRAM_POSITION(key text, data text)
		else if (pgsql_pos) {
			//TRACE("has pgsql_pos")
			// plsql function name assumed to be like "dictfilename_FIELDNAME()"
			sqlexpression = get_normalized_filename(actualdictfile).replace("dict.", "dict_") ^ "_" ^ fieldname ^ "(";

			// function arguments are (key,data) by default

			// Extract pgsql_line1
			var delim;
			let pgsql_line1 = function_src.substr2(pgsql_pos, delim);

//			let keydictid = pgsql_line1.field(" ", 2);
//			if (keydictid) {
//				sqlexpression ^= get_dictexpression(
//					cursor, filename, filename, dictfilename, dictfile,
//					keydictid, joins, unnests, selects, ismv, forsort
//				);
//
//			} else {
//				sqlexpression ^= get_fileexpression(mainfilename, filename, "key");
//			}
//
//			// 2nd arg is FILENAME.data
//			sqlexpression ^= ", ";
//			sqlexpression ^= get_fileexpression(mainfilename, filename, "data");

			var args = pgsql_line1.field(" ",2,99);
			if (not args.len())
				args = "key data";
			else if (not args.contains(" "))
				args ^= " data";

			// The first and 2nd arguments can be SOME_DICTID otherwise
			// the 1st defaults to FILENAME.key
			// the 2nd defaults to FILENAME.data
			// e.g.
			// / *pgsql BRAND_CODE
			args.converter(" ", _FM);
			for (const var& arg : args) {
				if (arg == "key" or arg == "data") {
					sqlexpression ^= get_fileexpression(mainfilename, filename, arg);
				} else {
					bool isdatetime;
					sqlexpression ^= get_dictexpression(
						cursor, filename, filename, dictfilename, dictfile,
						arg, joins, unnests, selects, ismv, isdatetime, forsort
					);
				}
				sqlexpression ^= ", ";
			}

			sqlexpression.paster(-2, 2, ")");

		}

		// Multivalued stage2 handled below
		else if (stage2_calculated and ismv1) {
			//TRACE("has stage2_calculated and ismv1")
			sqlexpression = stage2_filename ^ "." ^ fieldname ^ "_calc";
		}

		// Simple join using XLATE command but not on multivalued unless stage2
		///////////////////////////////////////////////
		// stage2_calculated="@ANS=XLATE(\"SELECT_CURSOR_STAGE2_" ^ this->f(1) ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
		// expect things like
		//@ans=xlate('ACCOUNTS',@record<8,@mv>,21,'X')
		//@ans=xlate('ACCOUNTS',@record<2,1,2>,1,'X')
		//@ans=xlate('ADDRESSES',@id,2,'X')
		//@ans=xlate('SCHEDULES',field(@id,'.',1),'YEAR_PERIOD','C')
		//@ans=xlate('CURRENCIES',{CURRENCY_CODE},1,'X')
		//@ANS=XLATE("SELECT_CURSOR_STAGE2_1",@ID,CPP_TEXT_calc,"X")
		//else if ((not ismv1 or stage2_calculated) and function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate(")) {
		else if (is_ans_xlate) {

			// Expect things like:
			// "\t// @ans=xlate('SUPPLIERS',@record<22>,1,'X')"
//			TRACE("function_src starts @ans=")

			// Remove leading white space and slashes and remove all other spaces
			function_src = function_src.f(1, 1).trimfirst("\t /").convert(" ", "");

			// Remove "@ans=xlate("
			function_src.cutter(11);

			// Hide comma in arg3 like <1,@mv>
			function_src.replacer(",@mv", "|@mv");

			// Hide commas in arg2 like field(@id,'*',x)
			function_src.replacer(",'*',", "|'*'|");

			// arg1 = filename
			var xlatetargetfilename = function_src.field(",", 1).trim().convert(".", "_");
			unquoter_inline(xlatetargetfilename);

			// arg2 = key
			var xlatefromfieldname = function_src.field(",", 2).trim();

			// arg3 = target field number/name
			let xlatetargetfieldname = function_src.field(",", 3).trim().unquote();

			// arg4 = mode X or C
			let xlatemode = function_src.field(",", 4).trim().convert("'\" )", "");

			// Error if fourth field is not "X", "C" or 'C'
			if (xlatemode != "X" and xlatemode != "C") {
				// not xlate X or C
				goto exodus_call;
//				TRACE("goto exodus_call")
			}
			if (DBTRACE>1) {
				TRACE(function_src)
				TRACE(xlatetargetfilename)
				TRACE(xlatefromfieldname)
				TRACE(xlatetargetfieldname)
				TRACE(xlatemode)
			}

//			TRACE("doing a join")
			// Assume we have a good simple xlate function_src and can convert to a JOIN
			// Determine the expression in the xlate target file
			// io todictexpression=sqlexpression;
			if (xlatetargetfieldname.isnum()) {
				sqlexpression =
					"exodus.extract_text(" ^
					get_fileexpression(mainfilename, xlatetargetfilename,
							"data") ^
					", " ^ xlatetargetfieldname ^ ", 0, 0)";

			} else if (stage2_calculated) {
				sqlexpression = xlatetargetfieldname;

			} else {
				// var dictxlatetofile=xlatetargetfilename;
				// if (not dictxlatetofile.open("DICT",xlatetargetfilename))
				//	throw VarDBException("get_dictexpression() DICT" ^
				// xlatetargetfilename ^ " file cannot be opened"); var
				// ismv;
				let xlatetargetdictfilename = "dict." ^ xlatetargetfilename;
				var xlatetargetdictfile;
//				if (not xlatetargetdictfile.open(xlatetargetdictfilename)) UNLIKELY {
				if (not xlatetargetdictfile.open(xlatetargetdictfilename, dictfile)) UNLIKELY {
					let errmsg = xlatetargetdictfilename ^ " cannot be opened for " ^ function_src;
					throw VarDBException(errmsg);
				}
				bool isdatetime;
				sqlexpression = get_dictexpression(cursor,
					filename, xlatetargetfilename, xlatetargetdictfilename,
					xlatetargetdictfile, xlatetargetfieldname, joins, unnests,
					selects, ismv, isdatetime, forsort);
			}

			// determine the join details
			var xlatekeyexpression = "";
			//xlatefromfieldname.logputl("xlatefromfieldname=");
			if (xlatefromfieldname.trim().lcase().starts("@record<")) {
				xlatekeyexpression = "exodus.extract_text(";
				xlatekeyexpression ^= filename ^ ".data";
				xlatekeyexpression ^= ", " ^ xlatefromfieldname.cut(8);
				xlatekeyexpression.popper();
				xlatekeyexpression ^=
					var(", 0").str(3 - xlatekeyexpression.count(",")) ^ ")";
			} else if (xlatefromfieldname.trim().lcase().starts("field(@id|")) {
				xlatekeyexpression = "split_part(";
				xlatekeyexpression ^= filename ^ ".key,'*',";
				xlatekeyexpression ^= xlatefromfieldname.field("|", 3).field(")", 1) ^ ")";
			}

			// TODO if
			// (xlatefromfieldname.starts(FIELD(@ID))
			else if (xlatefromfieldname.starts("{")) {
				xlatefromfieldname.cutter(1);
				xlatefromfieldname.popper();
				bool isdatetime;
				xlatekeyexpression = get_dictexpression(cursor,
					filename, filename, dictfilename, dictfile,
					xlatefromfieldname, joins, unnests, selects, ismv, isdatetime, forsort);

			} else if (xlatefromfieldname.lcase() == "@id") {
				xlatekeyexpression = filename ^ ".key";

			} else UNLIKELY {
				// throw  VarDBException("get_dictexpression() " ^
				// filename.quote() ^ " " ^ fieldname.quote() ^ " - INVALID
				// DICTIONARY EXPRESSION - " ^ dictrec.f(8).quote());
				var("ERROR: vardb: get_dictexpression() " ^
					filename.quote() ^ " " ^ fieldname.quote() ^
					" - INVALID DICTIONARY EXPRESSION - " ^
					dictrec.f(8).quote())
					.errputl();
				return "";
			}

			//if the xlate key expression is stage2_calculated then
			//indicate that the whole dictid expression is stage2_calculated
			//and do not do any join
			if (xlatekeyexpression.contains("exodus_call")) UNLIKELY {
				sqlexpression = "exodus_call(";
				if (DBTRACE>1)
					TRACE(sqlexpression)

				return sqlexpression;
			}

			fromjoin = true;

			// joins needs to follow "FROM mainfilename" clause
			// except for joins based on mv fields which need to follow the
			// unnest function
			let joinsectionn = ismv ? 2 : 1;

			// add the join
			///similar code above/below
			//main file is on the left
			//secondary file is on the right
			//normally we want all records on the left (main file) and any secondary file records that exist ... LEFT JOIN
			//if joining to stage2_calculated field file then we want only records that exist in the stage2_calculated fields file ... RIGHT JOIN (could be INNER JOIN)
			//RIGHT JOIN MUST BE IDENTICAL ELSE WHERE TO PREVENT DUPLICATION
			var join_part1 = stage2_calculated ? "RIGHT" : "LEFT";
			join_part1 ^= " JOIN " ^ xlatetargetfilename ^ " ON ";

			let join_part2 =
				xlatetargetfilename ^ ".key = " ^ xlatekeyexpression;
			// only allow one join per file for now.
			// TODO allow multiple joins to the same file via different keys
			if (not joins.f(joinsectionn).contains(join_part1))
				//joins.r(joinsectionn, -1, join_part1 ^ join_part2);
				joins(joinsectionn, -1) = join_part1 ^ join_part2;

			if (DBTRACE>1) UNLIKELY
				TRACE(sqlexpression)

			return sqlexpression;

		}

		else {
exodus_call:
			// FOLLOWING IS CURRENTLY DISABLED since postgres has no way to call exodus
			// if we get here then we were unable to work out any sql expression or function
			// so originally we instructed postgres to CALL EXODUS VIA IPC to run exodus
			// subroutines in the context of the calling program. exodus vardb.cpp setup
			// a separate listening thread with a separate pgconnection before calling postgres.
			// BUT exodus subroutines cannot make request to the db while it is handling a
			// request FROM the db - unless it were to setup ANOTHER threada and pgconnection to
			// handle it. this is also perhaps SLOW since it has to copy the whole RECORD and ID
			// etc to exodus via IPC for every call!
			// sqlexpression = "'" ^ fieldname ^ "'";
			int environmentn = 1;  // getenvironmentn()
			sqlexpression = "exodus_call('exodusservice-" ^ var().ospid() ^ "." ^
							environmentn ^ "', '" ^ dictfilename.lcase() ^ "', '" ^
							fieldname.lcase() ^ "', " ^ filename ^ ".key, " ^ filename ^
							".data,0,0)";
			//sqlexpression.logputl("sqlexpression=");
			// TODO apply naturalorder conversion by passing forsort
			// option to exodus_call

			if (DBTRACE>1) UNLIKELY
				TRACE(sqlexpression)

			return sqlexpression;
		}
	} // of dict type S

	else UNLIKELY {
		// throw  filename ^ " " ^ fieldname ^ " - INVALID DICTIONARY ITEM";
		// throw  VarDBException("get_dictexpression(" ^ filename.quote() ^ ", " ^
		// fieldname.quote() ^ ") invalid dictionary type " ^ dicttype.quote());
		var("ERROR: vardb: get_dictexpression(" ^ filename.quote() ^ ", " ^
			fieldname.quote() ^ ") invalid dictionary type " ^
			dicttype.quote())
			.errputl();
		return "";
	}

	// Multivalued or xref fields need special handling
	///////////////////////////////////////////////////

	ismv = ismv1;

	// vector (for GIN or indexing/filtering multivalue fields)
	//if ((ismv1 and not forsort) or fieldname.ucase().ends("_XREF")) {
	if ((ismv1 and not forsort) or fieldname.ucase().ends("XREF")) {
		//this is the sole creation of to_tsvector in vardb.cpp
		//it will be used like to_tsvector(...) @@ to_tsquery(...)
		if (fieldname.ucase().ends("XREF"))
			sqlexpression = "immutable_unaccent(" ^ sqlexpression ^ ")";
		sqlexpression = "to_tsvector('simple', " ^ sqlexpression ^ ")";
		//sqlexpression = "to_tsvector('simple', " ^ sqlexpression ^ ")";
		//sqlexpression = "to_tsvector('english'," ^ sqlexpression ^ ")";
		//sqlexpression = "string_to_array(" ^ sqlexpression ^ ",chr(29),'')";

		//multivalued prestage2_calculated field DUPLICATE CODE
		if (fieldname0.ends(":")) {
			let joinsectionn = 1;
			let join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
			//if (not joins.f(joinsectionn).contains(join))
			if (not joins.contains(join))
				//joins.r(joinsectionn, -1, join);
				joins(joinsectionn, -1) = join;
		}

	}

	// unnest multivalued fields into multiple output rows
	else if (ismv1) {

		//ismv = true;

		// var from="string_to_array(" ^ sqlexpression ^ ",'" ^ VM ^ "'";
		// exodus_extract_date_array, exodus_extract_time_array
		if (sqlexpression.starts("exodus.extract_date(") or sqlexpression.starts("exodus.extract_time("))
			sqlexpression.paster(20, "_array");
		else {
			sqlexpression.replacer("exodus.extract_sort\\("_rex, "exodus.extract_text(");
			sqlexpression = "string_to_array(" ^ sqlexpression ^ ", chr(29),'')";

			// Note 3rd argument '' means convert empty multivalues to nullptr in the array
			// otherwise conversion to float will fail
			if (isnumeric)
				sqlexpression ^= "::float8[]";
		}

		//now do this for all fields including WHERE and ORDER BY
		//eg SELECT BOOKING_ORDERS WITH YEAR_PERIOD "21.02" AND WITH IS_LATEST AND WITH CLIENT_CODE "MIC" AND WITH @ID NOT "%]" BY ORDER_NO
		//if (forsort)
		{

			// use the fieldname as a sql column name

			// if a mv field requires a join then add it to the SELECT clause
			// since not known currently how to to do mv joins in the FROM clause
			// Note the join clause should already have been added to the JOINS for the FROM
			// clause
			if (fromjoin) {

				// this code should never execute as joined mv fields now return the plain
				// sql expression. we assume they are used in WHERE and ORDER BY clauses

				// unnest in select clause
				// from="unnest(" ^ from ^ ")";
				// as FIELDNAME
				sqlexpression ^= " as " ^ fieldname;

				// dont include more than once, in case order by and filter on the same
				// field
				if (not selects.f(1).contains(sqlexpression))
					selects ^= ", " ^ sqlexpression;
			} else {

				//multivalued prestage2_calculated field DUPLICATE CODE
				if (fieldname0.ends(":")) {
					let joinsectionn = 1;
					let join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
					//if (not joins.f(joinsectionn).contains(join))
					if (not joins.contains(join))
						//joins.r(joinsectionn, -1, join);
						joins(joinsectionn, -1) = join;
				}

				// insert with SMs since expression can contain VMs
				if (not unnests.f(2).locate(fieldname)) {
					//unnests.r(2, -1, fieldname);
					//unnests.r(3, -1, sqlexpression);
					unnests(2, -1) = fieldname;
					unnests(3, -1) = sqlexpression;
				}
			}

			sqlexpression = fieldname;
		}
	}

	if (DBTRACE>1) UNLIKELY
		TRACE(sqlexpression)

	return sqlexpression;
}

// var getword(io remainingwords, in joinvalues=false)
static var getword(io remainingwords, io ucword) {

	// Gets the next word out of a sselect command
	// or a series of words separated by FM while they are numbers or quoted strings)
	// converts to sql quoted strings
	// and clips them from the input string

	bool joinvalues = true;

	var word1 = remainingwords.field(" ", 1);
	remainingwords = remainingwords.field(" ", 2, 99999);

	//separate out leading or trailing parens () but not both
	if (word1.len() > 1) {
		if (word1.starts("(") and not word1.ends(")")) {
			//put remaining word back on the pending words
			remainingwords.prefixer(word1.cut(1) ^ " ");
			//return single leading paren (
			word1 = "(";
		} else if (word1.ends(")")) {
			//put single closing paren back on the pending words
			remainingwords.prefixer(") ");
			//return word without trailing paren )
			word1.popper();
		}
	}

	// Join words within quote marks into one quoted phrase
	let char1 = word1.first();
	if ((char1 == DQ or char1 == SQ)) {
		while (not word1.ends(char1) or word1.len() <= 1) {
			if (remainingwords.len()) {
				word1 ^= " " ^ remainingwords.field(" ", 1);
				remainingwords = remainingwords.field(" ", 2, 99999);
			} else {
				word1 ^= char1;
				break;
			}
		}
	}

	tosqlstring(word1);

	// Grab multiple values (numbers or quoted words) into one list, separated by FM
	// value chars are " ' 0-9 . + -
	if (remainingwords and joinvalues and valuechars.contains(word1.first())) {
		word1 = SQ ^ word1.unquote().replace("'", "''") ^ SQ;

		var nextword = remainingwords.field(" ", 1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		// duplicated above/below
		if (nextword == "and") {
			let nextword2 = remainingwords;
			if (valuechars.contains(nextword2.first())) {
				nextword = nextword2;
				remainingwords = remainingwords.field(" ", 2, 99999);
			}
		}

		/*
		while (nextword and valuechars.contains(nextword[1])) {
			tosqlstring(nextword);
			if (word1 != "")
				word1 ^= FM_;
			word1 ^= SQ ^ nextword.unquote() ^ SQ;

			remainingwords = remainingwords.field(" ", 2, 99999);
			nextword = remainingwords.field(" ", 1);

			//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
			// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
			// duplicated above/below
			if (nextword == "and") {
				let nextword2 = remainingwords;
				if (valuechars.contains(nextword2[1])) {
					nextword = nextword2;
					remainingwords = remainingwords.field(" ", 2, 99999);
				}
			}
		}
		*/
		nextword = getword(remainingwords, ucword);
		if (nextword and valuechars.contains(nextword.first())) {
			tosqlstring(nextword);
			if (word1 != "")
				word1 ^= FM_;
			word1 ^= SQ ^ nextword.unquote() ^ SQ;
		}
		else {
			//push the nextword back if not a value word
			remainingwords = nextword ^ " " ^ remainingwords;
		}

	} else {
		// word1.ucaser();
	}

	ucword = word1.ucase();
	if (DBTRACE>1) UNLIKELY {
		TRACE(word1)
		TRACE(remainingwords)
	}
	return word1;
}

bool var_db::select(in sortselectclause) {

	THISIS("bool var::select(in sortselectclause) const")
	assertVar(function_sig);
	ISSTRING(sortselectclause)
	auto started = var().ostime();
	bool result;
	if (not sortselectclause or sortselectclause.ends("R)"))
		result = this->selectx("key, mv::integer, data", sortselectclause);
	else
		result = this->selectx("key, mv::integer", sortselectclause);
	auto elapsed = var().ostime() - started;
	if (elapsed > 1)
		elapsed.errputl(" select: secs:");
	return result;
}

// Currently only called from select, selectrecord and getlist
// TODO merge into plain select()?
bool var_db::selectx(in fieldnames, in sortselectclause) {

//	// private - and arguments are left unchecked for speed
//	//?allow unconstructed usage like var xyz=xyz.select();
//	if (var_typ & VARTYP_MASK) {
//		// throw VarUnconstructed("selectx()");
//		var_str.clear();
//		var_typ = VARTYP_STR;
//	}

	// fieldnames.logputl("fieldnames=");
	// sortselectclause.logputl("sortselectclause=");

	// Default to ""
	if (not (var_typ & VARTYP_STR)) {
		if (not var_typ) {
			var_str.clear();
			var_typ = VARTYP_STR;
		} else
			this->createString();
	}

	if (DBTRACE>1) {
		TRACE("SELECTX")
		TRACE(*this)
		TRACE(sortselectclause.toString())
	}

	var actualfilename = get_normalized_filename(*this);
	var dictfilename = actualfilename;
	auto actualfieldnames = fieldnames;
	var dictfile = "";
	var keycodes = "";
	bool bykey = false;
	let wordn;
	var distinctfieldnames = "";

	var whereclause = "";
	bool orwith = false;
	var orderclause = "";
	var joins = "";
	var unnests = "";
	var selects = "";
	bool ismv = false;
	bool isdatetime = false;

	var maxnrecs = "";
	var xx; // throwaway return value

	// Prepare to save calculated fields that cannot be calculated by postgresql for secondary processing
	var calc_fields = "";
	this->updater(10, "");

	// Catch bad FM character
	if (sortselectclause.var_str.find(FM_) != std::string::npos) UNLIKELY {
		let errmsg = "Illegal FM character in " ^ sortselectclause;
		throw VarDBException(errmsg);
	}

	// sortselect clause can be a file in which case we extract the filename from field1
	// omitted if filename.select() or file.select()
	// cursor.select(...) where ...
	// SELECT (or SSELECT) nnn filename with .... and with ... by ... by
	// filename can be omitted if calling like filename.select(...) or file.select(...)
	// nnn is optional limit to number of records returned
	// TODO only convert \t\r\n outside single and double quotes
	//var remaining = sortselectclause.f(1).convert("\t\r\n", "   ").trim();
	var remaining = sortselectclause.convert("\t\r\n", "   ").trim();

	// remaining.logputl("remaining=");

	// Remove trailing options eg (S) or {S}
	let lastword = remaining.field(" ", -1);
	if ((lastword.starts("(") and lastword.ends(")")) or
		(lastword.starts("{") and lastword.ends("}"))) {
		remaining.cutter(-lastword.len() - 1);
	}

	var firstucword = remaining.field(" ", 1).ucase();

	// sortselectclause may start with {SELECT|SSELECT {maxnrecs} filename}
	if (firstucword == "SELECT" or firstucword == "SSELECT") {
		if (firstucword == "SSELECT")
			bykey = true;

		// Remove it
		var yy;
		let xx = getword(remaining, yy);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// The second word can be a number to limit the number of records selected
	if (firstucword.len() and firstucword.isnum()) {
		maxnrecs = firstucword;

		// Remove it
		var yy;
		let xx = getword(remaining, yy);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// The next word can be the filename if not one of the select clause words
	// override any filename in the cursor variable
	//if (firstucword and not var("BY BY-DSND WITH WITHOUT ( { USING DISTINCT").locateusing(" ", firstucword)) {
	if (firstucword and not firstucword.listed("BY,BY-DSND,WITH,WITHOUT,(,{,USING,DISTINCT")) {
		actualfilename = firstucword;
		dictfilename = actualfilename;
		// remove it
		let xx = getword(remaining, firstucword);
	}

	if (not actualfilename) UNLIKELY {
		let errmsg = "filename missing from select statement:" ^ sortselectclause;
		throw VarDBException(errmsg);
	}

	while (remaining.len()) {

		// remaining.logputl("remaining=");
		// whereclause.logputl("whereclause=");
		// orderclause.logputl("orderclause=");

		var ucword;
		var word1 = getword(remaining, ucword);

		// Skip options (last word and surrounded by brackets)
		// (S) etc
		// options - last word enclosed in () or {}
		if (not remaining.len() and
			((word1.starts("(") and word1.ends(")")) or
			(word1.starts("{") and word1.ends("}")))) {
			// word1.logputl("skipping last word in () options ");
			continue;
		}

		// 1. numbers or strings without leading clauses like with ... mean record keys
		// 2. value chars are " ' 0-9 . + -
		// 3. values are ignored after any with/by statements to skip the following
		//    e.g. JUSTLEN "T#20" or HEADING "..."
		else if (valuechars.contains(word1.first())) {
			if (not whereclause and not orderclause) {
				if (keycodes)
					keycodes ^= FM;
				keycodes ^= word1;
			}
			if (DBTRACE_SELECT)
				TRACE(keycodes)
			continue;
		}

		// USING filename
		else if (ucword == "USING" and remaining) {
			dictfilename = getword(remaining, xx);
//			if (not dictfile.open("dict." ^ dictfilename)) UNLIKELY {
			if (not dictfile.open("dict." ^ dictfilename)) UNLIKELY {
				let errmsg = "select() dict_" ^ dictfilename ^ " file cannot be opened";
				throw VarDBException(errmsg);
			}
			if (DBTRACE_SELECT)
				TRACE(dictfilename)
			continue;
		}

		// DISTINCT fieldname (returns a field instead of the key)
		else if (ucword == "DISTINCT" and remaining) UNLIKELY {

			let distinctfieldname = getword(remaining, xx);
			let distinctexpression = get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, distinctfieldname, joins, unnests, selects, ismv, isdatetime, false);
			let naturalsort_distinctexpression = get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, distinctfieldname, joins, unnests, selects, ismv, isdatetime, true);

			if ((true)) {
				// This produces the right values but in random order
				// It uses any index on the distinct field so it works on large
				// indexed files select distinct is really only useful on INDEXED
				// fields unless the file is small
				distinctfieldnames = "DISTINCT " ^ (unnests ? distinctfieldname : distinctexpression);
			} else {
				// This produces the right results in the right order
				// BUT DOES IS USE INDEXES AND ACT VERY FAST??
				distinctfieldnames = "DISTINCT ON (" ^
								naturalsort_distinctexpression ^ ") " ^
								distinctexpression;
				orderclause ^= ", " ^ naturalsort_distinctexpression;
			}
			if (DBTRACE_SELECT)
				TRACE(orderclause)
			continue;
		}

		// BY or BY-DSND
		else if (ucword == "BY" or ucword == "BY-DSND") {
			// Next word must be dictid
			let dictid = getword(remaining, xx);
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, dictid, joins, unnests, selects, ismv, isdatetime, true);

			// dictexpression.logputl("dictexpression=");
			// orderclause.logputl("orderclause=");

			// No filtering in database on calculated items
			// save then for secondary filtering
			if (dictexpression.contains("exodus_call"))
			//if (dictexpression == "true")
			{
				if (not calc_fields.f(1).locate(dictid)) {
					//++ncalc_fields;
					//calc_fields.r(1, -1, dictid);
					calc_fields(1, -1) = dictid;
				}
				continue;
			}

			// Use postgres collation instead of exodus.extract_sort
			if (dictexpression.contains("exodus.extract_sort")) {
				dictexpression.replacer("exodus.extract_sort", "exodus.extract_text");
				dictexpression ^= " COLLATE exodus_natural";
			}

			orderclause ^= ",\n " ^ dictexpression;

			if (ucword == "BY-DSND")
				orderclause ^= " DESC";

			if (DBTRACE_SELECT)
				TRACE(orderclause)
			continue;
		}

		// Subexpression combination
		else if (ucword == "AND" or ucword == "OR") {
			// Dont start with AND or OR
			if (whereclause)
				whereclause ^= "\n " ^ ucword;
			if (ucword == "OR") {
				orwith = true;
			}
			if (DBTRACE_SELECT)
				TRACE(whereclause)
			continue;
		}

		// Subexpression grouping
		else if (ucword == "(" or ucword == ")") {
			// Default to or between WITH clauses
			if (whereclause.ends(")") and ucword == "(")
				whereclause ^= "\nor";
			whereclause ^= "\n " ^ ucword;
			if (DBTRACE_SELECT)
				TRACE(whereclause)
			continue;
		}

		// WITH dictid eq/starting/ending/containing/like 1 2 3
		// WITH dictid 1 2 3
		// WITH dictid between x and y
		else if (ucword == "WITH" or ucword == "WITHOUT") {

			/////////////////////////////////////////////////////////
			// Filter Stage 1 - Decide if positive or negative filter
			/////////////////////////////////////////////////////////

			var negative = ucword == "WITHOUT";

			// Next word must be the NOT/NO or the dictionary id
			word1 = getword(remaining, ucword);

			// Can negate before (and after) dictionary word
			// e.g WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" or ucword == "NO") {
				negative = not negative;
				// word1=getword(remaining,true);
				// remove NOT or NO
				word1 = getword(remaining, ucword);
			}

			//////////////////////////////////////////////////////////
			// Filter Stage 2 - Acquire column function to be filtered
			//////////////////////////////////////////////////////////

			// Skip AUTHORISED for now since too complicated to calculate in database
			// ATM if (word1.ucase()=="AUTHORISED") { if
			//(whereclause.ends(" AND"))
			//whereclause.paster(-4,4,""); continue;
			//}

			// Process the dictionary id
			let forsort =
				false;	// because indexes are NOT created sortable (exodus_sort()
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename,
							dictfile, word1, joins, unnests, selects, ismv, isdatetime, forsort);
			// var usingnaturalorder = dictexpression.contains("exodus.extract_sort") or dictexpression.contains("exodus_natural");
			let dictid = word1;

			// var dictexpression_isarray=dictexpression.contains("string_to_array(");
			let dictexpression_isarray = dictexpression.contains("_array(");
			var dictexpression_isvector = dictexpression.contains("to_tsvector(");
			// var dictexpression_isfulltext = dictid.ucase(),ends("_XREF");
			let dictexpression_isfulltext = dictid.ucase().ends("XREF");

			// Add the dictid expression
			//if (dictexpression.contains("exodus_call"))
			//	dictexpression = "true";

			//whereclause ^= " " ^ dictexpression;

			// The words after the dictid can be NOT/NO or values
			// word1=getword(remaining, true);
			word1 = getword(remaining, ucword);

			///////////////////////////////////////////////////////////////////////
			// Filter Stage 3 - 2nd chance to decide if positive or negative filter
			///////////////////////////////////////////////////////////////////////

			// Can negate before (and after) dictionary word
			// e.g. WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" or ucword == "NO") {
				negative = not negative;
				// word1=getword(remaining,true);
				// Remove NOT/NO and acquire any values
				word1 = getword(remaining, ucword);
			}

			/////////////////////////////////////////////////
			// Filter Stage 4 - SIMPLE BETWEEN or FROM clause
			/////////////////////////////////////////////////

			// BETWEEN x AND y
			// FROM x TO y

			if (ucword == "BETWEEN" or ucword == "FROM") {

				// Prevent BETWEEN being used on fields
				if (dictexpression_isvector) UNLIKELY {
					let errmsg = sortselectclause ^ " 'BETWEEN x AND y' and 'FROM x TO y' ... are not currently supported for mv or xref columns";
					throw VarDBException(errmsg);
				}

				// Get and append first value
				word1 = getword(remaining, ucword);

				// Get and append second value
				var word2 = getword(remaining, xx);

				// Discard any optional intermediate "AND"
				if (word2.ucase() == "AND" or word2.ucase() == "TO") {
					word2 = getword(remaining, xx);
				}

				// Check we have two values (in word1 and word2)
				if (not valuechars.contains(word1.first()) or not valuechars.contains(word2.first())) UNLIKELY {
					let errmsg = sortselectclause ^ "BETWEEN x AND y/FROM x TO y must be followed by two values (x AND/TO y)";
					throw VarDBException(errmsg);
				}

				// Replaced by COLLATE
				//if (usingnaturalorder) {
				//	word1 = naturalorder(word1.var_str);
				//	word2 = naturalorder(word2.var_str);
				//}

				// No filtering in database on calculated items
				// save then for secondary filtering
				if (dictexpression.contains("exodus_call")) {
					let opid = negative ? ">!<" : "><";

					// Almost identical code for exodus_call above/below
					var calc_fieldn;
					if (not calc_fields.locate(dictid, calc_fieldn, 1)) {
						calc_fields(1, calc_fieldn) = dictid;
					}

					// Prevent WITH XXX appearing twice in the same sort/select clause
					// unless and until implemented
					if (calc_fields.f(2, calc_fieldn)) UNLIKELY {
						let errmsg = "WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote();
						throw VarDBException(errmsg);
					}

					calc_fields(2, calc_fieldn) = opid;
					word1.lowerer();
					calc_fields(3, calc_fieldn) = word1;
					calc_fields(4, calc_fieldn) = word2;

					// Place holder for stage 1 of stage2 select
					whereclause ^= " true";
					continue;
				}

				// Select numrange(100,150,'[]')  @> any(string_to_array('1,2,150',',','')::numeric[]);
				if (dictexpression_isarray) {
					var date_time_numeric;
					if (dictexpression.contains("date_array(")) {
						whereclause ^= " daterange(";
						date_time_numeric = "date";
					} else if (dictexpression.contains("time_array(")) {
						whereclause ^= " tsrange(";
						//date_time_numeric = "time";
						date_time_numeric = "interval";
					} else {
						whereclause ^= " numrange(";
						date_time_numeric = "numeric";
					}
					whereclause ^= word1 ^ "," ^ word2 ^ ",'[]') ";
					whereclause ^= " @> ";
					whereclause ^= " any( " ^ dictexpression ^ "::" ^ date_time_numeric ^ "[])";
					continue;
				}

				whereclause ^= " " ^ dictexpression;

				if (negative)
					whereclause ^= " NOT ";

				if (whereclause)
					whereclause ^= " BETWEEN " ^ word1 ^ " AND " ^ word2;

				continue;
			}

			////////////////////////////////////
			// Filter Stage 5 - ACQUIRE OPERATOR
			////////////////////////////////////

			// 1. currently using regular expression instead of SQL LIKE
			// 2. use "BETWEEN X and XZZZZZZZ" instead of "LIKE X%"
			//    to ensure that any postgresql btree index is used if present

			var op = "";
			var prefix = "";
			var postfix = "";

			// Using regular expression logic for CONTAINING/STARTING/ENDING
			// will be converted to tsvector logic if dictexpression_isvector
			if (ucword == "CONTAINING" or ucword == "[]") {
				prefix = ".*";
				postfix = ".*";
				op = "=";
				word1 = getword(remaining, ucword);
			} else if (ucword == "STARTING" or ucword == "]") {

				// Identical code above/below
				if (dictexpression_isvector) {
					prefix = "^";
					postfix = ".*";
				}
				// NOT using regular expression logic for single valued fields and STARTING
				else {
					op = "]";
				}

				word1 = getword(remaining, ucword);
			} else if (ucword == "ENDING" or ucword == "[") {
				prefix = ".*";
				postfix = "$";
				op = "=";
				word1 = getword(remaining, ucword);
			}

			// "normal" comparative filtering
			// 1) Acquire operator - or empty if not present

			// Convert PICK/AREV relational operators to standard SQL relational operators
			// IS/ISNT/NOT -> EQ/NE/NE
			var aliasno;
//			if (var("IS EQ NE NOT ISNT GT LT GE LE").locateusing(" ", ucword, aliasno)) {
			if (ucword.listed("IS,EQ,NE,NOT,ISNT,GT,LT,GE,LE", aliasno)) {
				word1 = var("= = <> <> <> > < >= <=").field(" ", aliasno);
				ucword = word1;
			}

			// Capture operator is any
//			if (var("= <> > < >= <= ~ ~* !~ !~*").locateusing(" ", ucword)) {
			if (ucword.listed("=,<>,>,<,>=,<=,~,~*,!~,!~*")) {
				// Is an operator
				op = ucword;
				// Get another word (or words)
				word1 = getword(remaining, ucword);
			}

			////////////////////////////////////
			// Filter Stage 6 - ACQUIRE VALUE(S)
			////////////////////////////////////
			//TRACE(word1)

			// determine Pick/AREV values like "[xxx" "xxx]" and "[xxx]"
			// TODO
			if (word1.starts("'")) {

				if (word1.at(2) == "[") {
					word1.paster(2, 1, "");
					prefix = ".*";

					// CONTAINING
					if (word1.at(-2) == "]") {
						word1.paster(-2, 1, "");
						postfix = ".*";
					}
					// ENDING
					else {
						postfix = "$";
					}

					// STARTING
				} else if (word1.at(-2) == "]") {
					word1.paster(-2, 1, "");

					// Identical code above/below
					if (dictexpression_isvector) {
						prefix = "^";
						postfix = ".*";
					}
					// NOT using regular expression logic for single valued fields and STARTING
					// this should trigger 'COLLATE "C" BETWEEN x AND y" below to ensure postgres indexes are used
					else {
						if (op == "<>")
							negative = not negative;
						op = "]";
					}
				}
				ucword = word1.ucase();
			}

			// select WITH ..._XREF uses postgres full text searching
			// which has its own prefix and postfix rules. see below
			if (dictexpression_isfulltext) {
				prefix = "";
				postfix = "";
			}

			/*implement using posix regular string matching
				~ Matches regular expression, case sensitive
					'thomas' ~ '.*thomas.*'
				~* Matches regular expression, case insensitive
					'thomas' ~* '.*Thomas.*'
				!~ Does not match regular expression, case sensitive
					'thomas' !~ '.*Thomas.*'
				!~* Does not match regular expression, case insensitive
					'thomas' !~* '.*vadim.*'
			*/

			else if (not dictexpression_isvector and (prefix or postfix)) {

				// Postgres match matches anything in the string unless ^ and/or $ are present
				// so .* is not necessary in prefix and postfix
				if (prefix == ".*")
					prefix = "";
				if (postfix == ".*")
					postfix = "";

				// Escape any posix special characters;
				// [\^$.|?*+()
				// If present in the search criteria, they need to be escaped with
				// TWO backslashes.
				word1.replacer("\\", "\\\\");
				let special = "[^$.|?*+()";
				for (int ii = special.len(); ii > 0; --ii) {
					if (special.contains(word1.at(ii)))
						word1.paster(ii, "\\");
				}
				word1.replacer("'" _FM "'", postfix ^ "'" _FM "'" ^ prefix);
				word1.paster(-1, postfix);
				word1.paster(2, prefix);

				// Only ops <> and != are supported when using the regular expression operator (starting/ending/containing)
				if (op == "<>")
					negative = not negative;
				else if (op != "=" and op != "") UNLIKELY {
					let errmsg = "SELECT ... WITH " ^ op ^ " " ^ word1 ^ " is not supported. " ^ prefix.quote() ^ " " ^ postfix.quote();
					throw VarDBException(errmsg);
				}

				// Use regular expression operator
				op = "~";
				ucword = word1;
			}

			// word1 at this point may be empty, contain a value or be the first word of an unrelated clause
			// if non-value word1 unrelated to current phrase
			if (ucword.len() and not valuechars.contains(ucword.first())) {

				// Push back and treat as missing value
				remaining.prefixer(ucword ^ " ");

				// Simulate no given value .. so a boolean filter like "WITH APPROVED"
				word1 = "";
				ucword = "";
			}

			var value = word1;

			// Change 'WITH SOMEMVFIELD = ""' to just 'WITH SOMEMVFIELD' to avoid ts_vector searching for nothing
			if (value == "''") {

				// Remove multivalue handling - duplicate code elsewhere
				if (dictexpression.contains("to_tsvector(")) {
					// Dont create exodus.tobool(to_tsvector(...
					dictexpression.replacer("to_tsvector('simple',","");
					dictexpression.popper();
					dictexpression_isvector = false;
				}

			}

			/////////////////////////////////////////////////////////////////////
			// Filter Stage 7 - SAVE INFO FOR CALCULATED FIELDS IN STAGE 2 SELECT
			/////////////////////////////////////////////////////////////////////

			// "Calculated fields" are exodus/c++ functions that cannot be run by postgres
			// and are done in exodus in exoprog.cpp two pass select

			// No filtering in database on calculated items
			// save then for secondary filtering
			if (dictexpression.contains("exodus_call"))
			//if (dictexpression == "true")
			{
				// No op or value means test for Pick/AREV true (zero and '' are false)
				if (op == "" and value == "")
					op = "!!";

				// Missing op presumed to be =
				else if (op == "")
					op = "=";

				// Invert comparison if "WITHOUT" or "NOT" for calculated fields
				//if (negative and
				//	var("= <> > < >= <= ~ ~* !~ !~* !! ! ]").locateusing(" ", op, aliasno)) {
				if (negative and op.listed("=,<>,>,<,>=,<=,~,~*,!~,!~*,!!,!,]", aliasno)) {
					// op.logputl("op entered:");
					negative = false;
					op = var("<> = <= >= < > !~ !~* ~ ~* ! !! !]").field(" ", aliasno);
					// op.logputl("op reversed:");
				}

				// Almost identical code for exodus_call above/below
				var calc_fieldn;
				if (not calc_fields.locate(dictid, calc_fieldn, 1)) {
					calc_fields(1, calc_fieldn) = dictid;
				}
				if (calc_fields.f(2, calc_fieldn)) UNLIKELY {
					let errmsg = "WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote();
					throw VarDBException(errmsg);
				}

				// Save the op
				calc_fields(2, calc_fieldn) = op;

				// Save the value(s) after removing quotes and using SM to separate values instead of FM
				calc_fields(3, calc_fieldn) = value.unquote().replace("'" _FM "'", FM).convert(FM, SM);

				// Place holder to be removed before issuing actual sql command
	            if (DBTRACE_SELECT)
					TRACE(whereclause)
				whereclause ^= " true";

				continue;
			}

			///////////////////////////////////////////////////////////
			// Filter Stage 8 - DUMMY OP AND VALUE SAVE IF NOT PROVIDED
			///////////////////////////////////////////////////////////

			// Missing op and value means NOT '' or NOT 0 or NOT nullptr
			// WITH CLIENT_TYPE
			if (op == "" and value == "") {
				//op = "<>";
				//value = "''";

				let origdictexpression = dictexpression;
				// Remove conversion to date/number etc
				// i.e for non-symbolic dicts i.e exodus.extract_date() and not dict_clients_stopped2()
				to_extract_text(dictexpression);
				let replacedbyextracttext = dictexpression != origdictexpression;

				// Remove conversion to array
				// e.g. string_to_array(exodus.extract_text(JOBS.data,6, 0, 0), chr(29),'')
				if (dictexpression.starts("string_to_array(")) {
					dictexpression.cutter(16);
					dictexpression.cutter(-13);
				}

				// Remove multivalue handling - duplicate code elsewhere
				if (dictexpression.contains("to_tsvector(")) {
					// Dont create exodus.tobool(to_tsvector(...
					dictexpression.replacer("to_tsvector('simple',","");
					dictexpression.popper();
					//TRACE(dictexpression)
					dictexpression_isvector = false;
				}

				// Currently tobool requires only text input
				// TODO some way to detect DATE SYMBOLIC FIELDS and not hack special dict words!
				// doesnt work on multivalued fields - results in:
				// exodus.tobool(SELECT_CURSOR_STAGE2_19397_37442_012029.TOT_SUPPINV_AMOUNT_BASE_calc, chr(29),)
				// TODO work out better way of determining DATE/TIME that must be tested versus null
				// if (isdatetime or dictexpression.contains("FULLY_") or (not dictexpression.contains("exodus.extract") and dictexpression.contains("_DATE")))
				//if (isdatetime)
				if (isdatetime and not replacedbyextracttext)
					dictexpression ^= " is not null";
				else
					dictexpression = "exodus.tobool(" ^ dictexpression ^ ")";
			}

			// Missing op means =
			// e.g. WITH CLIENT_TYPE "X" .. assume missing "=" sign
			else if (op == "") {
				op = "=";
			}

			///////////////////////////////////////////////////////////
			// Filter Stage 9 - PROCESS DICTEXPRESSION, OP AND VALUE(S)
			///////////////////////////////////////////////////////////

			// op and value(s) are now set

			// Replaced by COLLATE
			// natural order value(s)
			//if (usingnaturalorder)
			//	value = naturalorder(value.var_str);

			// without xxx = "abc"
			// with xxx not = "abc"

			// notword.logputl("notword=");
			// ucword.logputl("ucword=");

			// Allow searching for text with * characters embedded
			// otherwise interpreted as glob character?
			if (dictexpression_isvector) {
				value.replacer("*", "\\*");
			}

			// STARTING
			// Special processing for STARTING]
			// convert "STARTING 'ABC'"  to "BETWEEN 'X' AND 'XZZZZZZ'
			// so that any btree index if present will be used. "LIKE" or REGULAR EXPRESSIONS will not use indexes
			if (op == "]") {

				var expression = "";
				// value is a FM separated list here so "subvalue" is a field
				for (var subvalue : value) {
					/* ordinary UTF8 collation strangely doesnt sort single punctuation characters along with phrases starting with the same
					so we will use C collation which does. All so that we can use BETWEEN instead of LIKE to support STARTING WITH syntax

					Example WITHOUT collation showing % sorted in different places

					test_test=# select * from test1 order by key;
					key        |   data
					-----------+-----------
					%         | %
					+         | +
					1         | 1
					10        | 10
					2         | 2
					20        | 20
					A         | A
					B         | B
					%RECORDS% | RECORDS
					+RECORDS+ | +RECORDS+
					*/

					dictexpression.replacer("^exodus.extract_number\\("_rex, "exodus.extract_text(");
					if (not subvalue.starts("'"))
						subvalue.squoter();

					// .. with somefield starting 'abc'
					// -> WHERE ( exodus.extract_text(somefile.data,999, 0, 0) COLLATE "C" BETWEEN 'abc' AND 'abcZZZZZZ' )
					expression ^= dictexpression ^ " COLLATE \"C\"";
					expression ^= " BETWEEN " ^ subvalue ^ " AND " ^ subvalue.paste(-1, 0, "ZZZZZZ") ^ FM;
				}
				expression.popper();
				expression.replacer(FM, " OR ");
				value = expression;

				// Indicate that the dictexpression is included in the value(s)
				op = "(";

			}

			// Single value data with multiple values filter
			else if (value.contains(FM) and not dictexpression_isvector) {

				// WARNING ", " is replaced in exoprog.cpp ::select()
				// so change there if changed here
				value.replacer(_FM, ", ");

				// No processing for arrays (why?)
				if (dictexpression_isarray) {
				}

				// lhs is an array ("multivalues" in postgres)
				// dont convert rhs to in() or any()
				else if (op == "=") {
					to_extract_text(dictexpression);
					op = "in";
					value = "( " ^ value ^ " )";
				}

				// not any of
				else if (op == "<>") {
					to_extract_text(dictexpression);
					op = "not in";
					value = "( " ^ value ^ " )";
				}

				// any of
				else {
					to_extract_text(dictexpression);
					value = "ANY(ARRAY[" ^ value ^ "])";
				}
			}

			// Full text search OR mv data search
			if (dictexpression_isvector) {

				// See note on isxref in "multiple values" section above
				op = "@@";

				// & separates multiple required
				// | separates multiples alternatives
				// 'fat & (rat | cat)'::tsquery;
				//
				// :* means "ending in" to postgres tsquery. see:
				// https://www.postgresql.org/docs/10/datatype-textsearch.html
				// "lexemes in a tsquery can be labeled with * to specify prefix matching:"
				//
				// Spaces are NOT allowed in values unless quoted
				// Single quotes must be doubled '' (not ")
				// since the whole query string will be single quoted

				// In full text query on multiple words,
				// we implement that words all are required
				// all values and words separated by spaced are used as "word stems"

				// Using to_tsquery to search multivalued data
				if (not dictexpression_isfulltext) {

					// Double the single quotes so the whole thing can be wrapped in single quotes
					// and because to_tsquery generates a syntax error in case of spaces inside values unless quotedd
					value.replacer("'","''");

					// Wrap everything in single quotes for sql
					value.squoter();

					// Multiple with options become alternatives using to_tsquery | divider
					value.replacer(_FM, "|");

				}

				// If full text search
				if (dictexpression_isfulltext) {

					//https://www.postgresql.org/docs/current/textsearch-controls.html
					//and
					//https://www.postgresql.org/docs/current/datatype-textsearch.html#DATATYPE-TSQUERY

					// Construct according to ts_query syntax using & | ( )
					// e.g. trying to find records containing either ADIDAS or KIA MOTORS where \036 is VM
					// value 'ADID\036KIA&MOT' -> '(ADID:*)|(KIA:*&MOT:*)

					// xxx:* searches for words starting with xxx

					// Multivalues are searched using "OR" which is the | pipe character in ts_query syntax
					// words separated by spaces (or & characters) are searched for uing "AND" which is & in ts_query syntax
					var values="";
					value.unquoter();
					value.converter(VM,FM);
					for (var partvalue : value) {

						// Replace all single quotes in search term for pairs of single quotes as per postgres syntax
						partvalue.replacer("'","''");

						// Append postfix :* to every search word
						// so STEV:* also finds STEVE and STEVEN

						// Spaces should have been converted to & before selection
						// Spaces imply &
						// partvalue.replacer(" ", "&");
						// partvalue.paster(-1, ":*");

						// Treat entered colons as &
						partvalue.replacer(":", "&");

						// Respect any user entered AND or OR operators
						// search for all words STARTING with user defined words
						partvalue.replacer("&", ":*&");
						partvalue.replacer("|", ":*|");
						partvalue.replacer("!", ":*!");

						partvalue ^= ":*";

						values ^= "(" ^ partvalue ^ ")";
						values ^= FM;
					}
					values.popper();
					values.replacer(FM, "|");
					value = values.squote();
				}
				// Select multivalues starting "XYZ" by selecting "XYZ]"
				else if (postfix) {
					value.replacer("]''", "'':*");
					value.replacer("]", ":*");
					//value.replacer("|", ":*|");
					value.paster(-1, ":*");
				}

				value.replacer("]''", "'':*");
				value.replacer("]", ":*");
				//value.paster(-1, ":*");

				// Use "simple" dictionary (ie none) to allow searching for words starting with 'a'
				// Use "english" dictionary for stemming (or "simple" dictionary for none)
				// MUST use the SAME in both to_tsvector AND to_tsquery
				//https://www.postgresql.org/docs/10/textsearch-dictionaries.html
				// this is the sole occurrence of to_tsquery in vardb.cpp
				// it will be used like to_tsvector(...) @@ to_tsquery(...)
				if (value) {
					if (dictexpression_isfulltext)
						value = "unaccent(" ^ value ^ ")";
					value = "to_tsquery('simple', " ^ value ^ ")";
				}
					//value = "to_tsquery('simple', " ^ value ^ ")";
				//value = "to_tsquery('english'," ^ value ^ ")";

				/* creating a "none" stop list?
				printf "" > /usr/share/postgresql/10/tsearch_data/none.stop
				CREATE TEXT SEARCH DICTIONARY public.simple_dict (
					TEMPLATE = pg_catalog.simple,
					STOPWORDS = none
				);
				in psql default_text_search_config(pgcatalog.simple_dict)
				*/
			}

			// Testing for "" may become testing for null
			// for date and time which are returned as null for empty string
			else if (value == "''") UNLIKELY {
				if (dictexpression.contains("extract_date") or
					dictexpression.contains("extract_datetime") or
					dictexpression.contains("extract_time")) {
					//if (op == "=")
					//	op = "is";
					//else
					//	op = "is not";
					//value = "null";
					dictexpression.replacer("extract_date(","extract_text(");
					dictexpression.replacer("extract_datetime(","extract_text(");
					dictexpression.replacer("extract_time(","extract_text(");
				}
				// Currently number returns 0 for empty string
				//or dictexpression.contains("extract_number")
				else if (dictexpression.contains("extract_number")) {
					//value = "'0'";
					dictexpression.replacer("extract_number(","extract_text(");
				}
				// Horrible hack to allow filtering calculated date fields versus ""
				// TODO detect FULLY_BOOKED and FULLY_APPROVED as dates automatically
				// else if (isdatetime or dictexpression.contains("FULLY_")) {
				else if (isdatetime) {
					if (op == "=")
						op = "is";
					else
						op = "is not";
					value = "null";
				}
			}

			// If selecting a mv array then convert right hand side to array
			// (can only handle = operator at the moment)
			if (dictexpression_isarray and (op == "=" or op == "<>")) {
				if (op == "<>") {
					negative = not negative;
					op = "=";
				}

				if (value == "''") {
					value = "'{}'";
				} else {
					op = "&&";	//postgres array overlap operator
					value.replacer("'", "\"");
					//convert to postrgesql array syntax
					value = "'{" ^ value ^ "}'";
				}
			}

			//////////////////////////////////////////////
			// Filter Stage 10 - COMBINE INTO WHERE CLAUSE
			//////////////////////////////////////////////

			if (DBTRACE_SELECT)
				TRACE(whereclause)
			// Default to OR between with clauses
			if (whereclause) {
				let lastpart = whereclause.field(" ", -1);
				//if (not var("OR AND (").locateusing(" ", lastpart))
				if (not lastpart.listed("OR,AND,("))
					whereclause ^= " or ";
				if (DBTRACE_SELECT)
					TRACE(whereclause)
			}

			// Negate
			if (negative)
				whereclause ^= " not";

			if (op == "(")
				whereclause ^= " ( " ^ value ^ " )";
			else
				whereclause ^= " " ^ dictexpression ^ " " ^ op ^ " " ^ value;

			if (DBTRACE_SELECT)
				TRACE(whereclause)

		}  // WITH/WITHOUT phrase

	}  // getword loop

	if (calc_fields and orwith) {
		//		throw VarDBException("OR not allowed with sort/select calculated fields");
	}

	// Prefix specified keys into where clause
	if (keycodes) {
		//if (keycodes.count(FM))
		{
			keycodes = actualfilename ^ ".key IN ( " ^ keycodes.replace(FM, ", ") ^ " )";

			if (whereclause)
				//whereclause ^= "\n AND ( " ^ keycodes ^ " ) ";
				whereclause = keycodes ^ "\n AND " ^ whereclause;
			else
				whereclause = keycodes;
		}
	}
	//TRACE(actualfilename)
	// sselect add by key on the end of any specific order bys
	if (bykey)
		orderclause ^= ", " ^ actualfilename ^ ".key";

	// If calculated fields then secondary sort/select is going to use data column, so add the data column if missing
	if (calc_fields and not actualfieldnames.ends(", data"))
		actualfieldnames ^= ", data";

	// Remove mv::integer if no unnesting (sort on mv fields)
	if (not unnests) {
		// sql ^= ", 0 as mv";
		if (actualfieldnames.contains("mv::integer, data")) {
			// Replace the mv column with zero if selecting record
			actualfieldnames.replacer("mv::integer, data", "0::integer, data");
		} else
			actualfieldnames.replacer(", mv::integer", "");
	}

	// If any active select, convert to a file and use as an additional filter on key
	// or correctly named savelistfilename exists from getselect or selectkeys
	let listname = "";
	// See also listname below
	//	if (this->hasnext()) {
	//		listname=this->f(1) ^ "_" ^ ospid() ^ "_tempx";
	//		this->savelist(listname);
	//		let savelistfilename="savelist_" ^ listname;
	//		joins ^= " \nINNER JOIN\n " ^ savelistfilename ^ " ON " ^ actualfilename ^
	//".key = " ^ savelistfilename ^ ".key";
	//	}

	// Disambiguate from any INNER JOIN key
	actualfieldnames.replacer(R"(\b(key|data)\b)"_rex, actualfilename ^ ".$&");

	// DISTINCT has special fieldnames
	if (distinctfieldnames)
		actualfieldnames = distinctfieldnames;

	// Remove redundant clauses
	whereclause.replacer("\n AND true", "");
	whereclause.replacer("true\n AND ", "");

	// Determine the connection from the filename
	// could be an attached on a non-default connection
	// selecting dict files would trigger this
	//TRACE(*this)
	//TRACE(actualfilename)
	if (not this->f(2) or actualfilename.starts("dict.", "DICT.")) {
		var actualfile;
//		if (actualfile.open(actualfilename, *this))
		if (actualfile.open(actualfilename))
			this->updater(2, actualfile.f(2));
		//TRACE(actualfile)
	}
	//TRACE(*this)
	// Save any active selection in a temporary table and INNER JOIN to it to avoid complete selection of primary file
	if (this->hasnext()) {

		// Create a temporary sql table to hold the preselected keys
		let temptablename = "SELECT_CURSOR_" ^ this->f(1);
		var createtablesql = "CREATE TEMPORARY TABLE IF NOT EXISTS " ^ temptablename ^ "\n";
		createtablesql ^= " (KEY TEXT)\n";
		createtablesql ^= ";DELETE FROM " ^ temptablename ^ "\n";
		var errmsg;
		if (not this->sqlexec(createtablesql, errmsg)) UNLIKELY {
			throw VarDBException(errmsg);
		}

		// Readnext the keys into a temporary table
		var key;
		while (this->readnext(key)) {
			//std::clog<<key<<std::endl;
			if (not this->sqlexec("INSERT INTO " ^ temptablename ^ "(KEY) VALUES('" ^ key.replace("'", "''") ^ "')")) UNLIKELY
				throw VarDBException(var::lasterror());
		}

		if (this->f(3)) UNLIKELY {
			throw VarDBException("selectx: Error. this->f(3) must be empty");
		}
		//must be empty!

		joins.inserter(1, 1, "\n RIGHT JOIN " ^ temptablename ^ " ON " ^ temptablename ^ ".key = " ^ actualfilename ^ ".key");
	}

	// Check file exists on the same connection
	var file;
	if (not file.open(actualfilename, *this)) {
		throw VarDBException(var::lasterror());
	}

	//////////////////////////////////////////
	// Assemble the full sql select statement:
	//////////////////////////////////////////

	// SQL DECLARE CURSOR
	//  WITH HOLD is a very significant addition
	//  var sql="DECLARE cursor1_" ^ *this ^ " CURSOR WITH HOLD FOR SELECT " ^ actualfieldnames
	//  ^ " FROM ";
	//TRACE(*this);
	var sql = "DECLARE\n cursor1_" ^ this->f(1).convert(".", "_") ^ " SCROLL CURSOR WITH HOLD FOR";
	// Not using WITH HOLD because such cursors are not automatically closed at the end of transactions
	// and hang around in the server until closed. Worse, in postgres 14, multiple selects
	// to the same cursor name within a transaction, but not outside a transaction, seem to cause postgres
	// to hang. testsort hangs on declaring a cursor.
	// TODO why was WITH HOLD added in the first place?
	//var sql = "DECLARE\n cursor1_" ^ this->f(1).convert(".", "_") ^ " SCROLL CURSOR FOR";

	//SQL SELECT - field/column names
	sql ^= " \nSELECT\n " ^ actualfieldnames;
	if (selects)
		sql ^= selects;

	//SQL FROM - filename and any specially related files
	sql ^= " \nFROM\n " ^ actualfilename;

	//SQL JOIN - (1)?
	if (joins.f(1))
		sql ^= " " ^ joins.f(1).convert(VM, "\n");

	//SQL UNNEST - mv fields
	//mv fields get added to the FROM clause like "unnest() as xyz" allowing the use of xyz in WHERE/ORDER BY
	//should only be one unnest (parallel mvs if more than one) since it is not clear how sselect by mv by mv2 should work if they are not in parallel
	if (unnests) {
		// "unnest"
		sql ^= ",\n unnest(\n  " ^ unnests.f(3).replace(VM, ",\n  ") ^ "\n )";
		// as fake tablename
		sql ^= " with ordinality as mvtable1";
		// Brackets allow providing column names for use elsewhere
		// and renaming of automatic column "ORDINAL" to "mv" for use in SELECT key,mv ...
		// sql statement
		sql ^= "( " ^ unnests.f(2).replace(VM, ", ") ^ ", mv)";
	}

	// SQL JOIN - related files
	if (joins.f(2))
		sql ^= " " ^ joins.f(2).convert(VM, "");

	// SQL WHERE - excludes calculated fields if doing stage 1 of a two stage sort/select
	//TODO when doing stage2, skip "WITH/WITHOUT xxx" of stage1 fields
	if (whereclause)
		sql ^= " \nWHERE \n" ^ whereclause;

	// SQL ORDER - suppressed if doing stage 1 of a two stage sort/select
	if (orderclause and not calc_fields)
		sql ^= " \nORDER BY \n" ^ orderclause.cut(2);

	//LIMIT - number of records returned
	// no limit initially if any calculated items - limit will be done in secondary sort/select
	if (maxnrecs and not calc_fields)
		sql ^= " \nLIMIT\n " ^ maxnrecs;

	// Final catch of obsolete function that was replaced by COLLATE keyword
	sql.replacer("exodus.extract_sort\\("_rex, "exodus.extract_text(");

	//sql.logputl("sql=");

	// DEBUG_LOG_SQL0
	// if (DBTRACE>1)
	//	exo::logputl(sql);

	// First close any existing cursor with the same name, otherwise cannot create  new cursor
	// Avoid generating sql errors since they abort transactions
	if (this->dbcursorexists()) {
		var sql2 = "";
		sql2 ^= "CLOSE cursor1_";

		if (this->assigned()) {
			let cursorcode = this->f(1).convert(".", "_");
			sql2 ^= cursorcode;
			let cursorid = this->f(2) ^ "_" ^ cursorcode;
			thread_dbresults.erase(cursorid);
		}

		var errmsg;
		if (not this->sqlexec(sql2, errmsg)) UNLIKELY {

			if (errmsg)
				errmsg.errputl("::selectx on handle(" ^ *this ^ ") " ^ sql2 ^ "\n");
			// return false;
		}
	}

	///////////////////////
	// FINALLY EXEC THE SQL
	///////////////////////

	var errmsg;
	if (not this->sqlexec(sql, errmsg)) UNLIKELY {

		// Always "" in above code
		if (listname)
			this->deletelist(listname);

		// TODO handle duplicate_cursor sqlstate 42P03
		sql.logputl("sql=");

		throw VarDBException(errmsg);
	}

	// Sort/select on calculated items may be done in exoprog::calculate
	// which can call calculate() and has access to mv.RECORD, mv.ID etc
	if (calc_fields) {
		calc_fields(5) = dictfilename.lower();
		calc_fields(6) = maxnrecs;
//		(*this)(10) = calc_fields.lower();
		this->updater(10, calc_fields.lower());
	}

	return true;
}

void var_db::clearselect() {

	if (DBTRACE>1)
		var("clearselect");

	// THISIS("void var::clearselect() const")
	// assertString(function_sig);

	// The default cursor is ""
	this->defaulter("");

	/// if readnext through string
	//3/4/5/6 setup in selectkeys. cleared in clearselect
	//if (this->f(3) == "%SELECTKEYS%")
	{
		this->updater(6, "");
		this->updater(5, "");
		this->updater(4, "");
		this->updater(3, "");
	}

	let listname = this->f(1) ^ "_" ^ ospid() ^ "_tempx";

	// if (DBTRACE>1)
	//	exo::logputl("DBTRACE: ::clearselect() for " ^ listname);

	// Dont close cursor unless it exists otherwise sql error aborts any transaction
	// Avoid generating sql errors since they abort transactions
	// if (not this->dbcursorexists())
	if (not this->dbcursorexists()) UNLIKELY
		return;

	// Clear any select list
	this->deletelist(listname);

	var errors;

	let cursorcode = this->f(1).convert(".", "_");
	let cursorid = this->f(2) ^ "_" ^ cursorcode;

	// Clean up cursor cache
	thread_dbresults.erase(cursorid);

	var sql = "";
	// sql^="DECLARE BEGIN ";
	sql ^= "CLOSE cursor1_" ^ cursorcode;
	// sql^="\nEXCEPTION WHEN\n invalid_cursor_name\n THEN";
	// sql^="\nEND";

	//sql.output();

	if (not this->sqlexec(sql, errors)) UNLIKELY {
		if (errors)
			errors.errputl("::clearselect on handle(" ^ *this ^ ") ");
		return;
	}

	return;
}

// NB global not member function
//To make it var:: private member -> pollute mv.h with PGresultptr :(
// bool readnextx(const std::string& cursor, PGresultptr& dbresult)
// called by readnext (and perhaps hasnext/select to implement LISTACTIVE)
static bool readnextx(in cursor, DBconn_ptr pgconn, int direction, PGresult*& pgresult, int* rown) {

	let cursorcode = cursor.f(1).convert(".", "_");
	let cursorid = cursor.f(2) ^ "_" ^ cursorcode;

	DBresult* dbresult = nullptr;
	auto entry = thread_dbresults.find(cursorid);
	if (entry != thread_dbresults.end()) {

		// Extract the current pgresult and rown of the cursor
		dbresult = &entry->second;
		*rown = dbresult->rown_;

		// If backwards
		// (should only be done after going forwards)
		if (direction < 0) UNLIKELY {

			// rown is unlikely to be used after requesting backwards
			(*rown)--;

			dbresult->rown_ = *rown;

			return true;
		}

		// Increment the rown counter
		(*rown)++;

		// If forwards
		// Increment the current rown index into the pgresult
		// return success and the rown if within bounds
		if (*rown < PQntuples(*dbresult)) LIKELY {

			// Save the rown for the next iteration
			dbresult->rown_ = *rown;

			// Return the pgresult array
			pgresult = dbresult->pgresult_;

			// Indicate success. true = found a new key/record
			return true;
		}

	}

	let fetch_nrecs=64;
	var sql = "FETCH FORWARD " ^ fetch_nrecs ^ " in cursor1_" ^ cursorcode;

	// Execute the sql
	// Cant use sqlexec() here because it returns data
	var ok;
//	const DBresult dbresult2 = get_dbresult(sql, pgconn, ok);
	// Cant be const because we are going to return the pgresult inside dbresult2 to the caller
	DBresult dbresult2 = get_dbresult(sql, pgconn, ok);
	if (not ok) {
		if (entry != thread_dbresults.end())
			thread_dbresults.erase(entry);

		var errmsg = var(PQresultErrorMessage(dbresult2));
		var sqlstate = "";
		if (PQresultErrorField(dbresult2, PG_DIAG_SQLSTATE)) {
			sqlstate = var(PQresultErrorField(dbresult2, PG_DIAG_SQLSTATE));
		}
		// dbresult2 is NULLPTR if if get_dbresult failed but since the dbresult is needed by
		// the caller, it will be cleared by called if not NULLPTR PQclear(dbresult2);

		// If cursor simply doesnt exist then see if a savelist one is available and enable
		// it 34000 - "ERROR:  cursor "cursor1_" does not exist"
		if (direction >= 0 and sqlstate == "34000") UNLIKELY {
			return false;

			/**
			// If the standard select list file is available then select it, i.e. create
			a CURSOR, so FETCH has something to work on var listfilename="savelist_" ^
			cursor ^ "_" ^ ospid() ^ "_tempx"; if (not var().open(listfilename, *this))
				return false;
			// TODO should add BY LISTITEMNO
			if (not cursor.select("select " ^ listfilename))
				return false;
			if (DBTRACE>1)
				exo::logputl("DBTRACE: readnextx(...) found standard selectfile "
			^ listfilename);

			return readnextx(cursor, dbresult, pgconn, clearselect_onfail, forwards);
			**/
		}

		// Any other error
		if (errmsg) UNLIKELY {
			errmsg ^= " sqlstate= " ^ sqlstate.quote() ^ " in SQL " ^ sql;
			throw VarDBException(errmsg);
		}

		return false;
	}

	// If no rows returned
	//if (not PQntuples(*dbresult))
	//	return false;

	// 1. Do NOT clear the cursor even if forward since we may be testing it
	// 2. DO NOT clear since the dbresult2 is needed by the caller

	// Increment the rown counter from -1 to 0
	//dbresult2.rown_++;

	// Save rown for the next iteration
	*rown = dbresult2.rown_;

//TRACE(dbresult2.rown_)

	// Take ownership of the pgresult currently owned by dbresult2
//	pgresult = std::move(dbresult2);
//	pgresult = std::move(dbresult2.pgresult_);
	std::swap(pgresult, dbresult2.pgresult_);
	// Relinquish ownership of pgresult to prevent its destructor here freeing it on close of this func
	// done by std::move above.
//	dbresult2.pgresult_ = nullptr;

	// Transfer the probably multi-row result into the thread_dbresults cache
	// for subsequent readnextx
	if (entry != thread_dbresults.end())
		entry->second = pgresult;
	else
		thread_dbresults[cursorid] = pgresult;

	// Indicate success. true = found a new key/record
	// out pgresult contains the meat
	return true;
}

bool var_db::deletelist(SV listname) const {

	THISIS("bool var::deletelist(SV listname) const")
	assertVar(function_sig);

	if (DBTRACE>1)
		this->logputl("DBTR var::deletelist(" ^ var(listname) ^ ") ");

	// Open the lists file on the same connection
	var lists;
	if (not lists.open("LISTS", var_db(*this))) UNLIKELY
		// Skip this error for now because maybe no LISTS on some databases
		return false;
		//throw VarDBException("deletelist() LISTS file cannot be opened");

	// Initial block of keys are stored with no suffix (i.e. no *1)
	bool result = lists.deleterecord(listname);

	// Supplementary blocks of keys are stored with suffix *2, *3 etc)
	for (int listno = 2;; ++listno) {
		const var listname2 = listname;
		var xx;
		if (not xx.read(lists, listname2 ^ "*" ^ listno))
			break;
		lists.deleterecord(listname2 ^ "*" ^ listno);
	}

	return result;
}

bool var_db::savelist(SV listname) {

	THISIS("bool var::savelist(SV listname)")
	assertVar(function_sig);

	if (DBTRACE>1)
		this->logputl("DBTR var::savelist(" ^ var(listname) ^ ") ");

	// Open the lists file on the same connection
	var lists;
	if (not lists.open("LISTS", var_db(*this))) UNLIKELY {
		let errmsg = "savelist() LISTS file cannot be opened";
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	var listno = 1;
	var listkey = listname;
	int nkeys = 0;
	const var listname2 = listname;
	var list = "";
	// Limit maximum number of keys in one block to 1Mb
	constexpr int maxlistsize = 1024 * 1024;

	// Function to write list of keys.
	// Called in readnext loop if list gets too large
	// and after the loop to save any unsaved keys
	auto write_list = [&] () {

		if (not list.var_str.size()) UNLIKELY
			return;

		// Delete any prior list with the same name
		if (listno == 1) {
			// this should not throw if the list does not exist
			this->deletelist(listname);
		}

		// Remove trailing FM
		list.var_str.pop_back();

		// Save a block
		list.write(lists, listkey);

		// Prepare the next block
		// first list block is listno 1 but has no suffix
		// 2nd list block is listno 2 and has suffice *2
		listno++;
		listkey = listname2 ^ "*" ^ listno;
		list = "";
	};

	var key;
	var mv;
	while (this->readnext(key, mv)) {

		nkeys++;

		// Append the key to the list
		list.var_str.append(key.var_str);

		// Append SM + mvno if mvno present
		if (mv) {
			list.var_str.push_back(VM_);
			list.var_str.append(mv.var_str);
		}

		// Save one list of keys if more than a certain size (1MB)
		if (list.len() > maxlistsize) {

			write_list();

			continue;
		}

		// Append an FM separator since lists use FM
		list.var_str.push_back(FM_);
	}

	// Write any pending list
	write_list();

	var::setlasterror(nkeys);

	return listno > 1;
}

bool var_db::getlist(SV listname) {

	THISIS("bool var::getlist(SV listname) const")
	assertVar(function_sig);

	if (DBTRACE>1)
		this->logputl("DBTR var::getlist(" ^ var(listname) ^ ") ");

	let key;
	let mv;
	const var listname2 = listname;
	var listfilename = "savelist_" ^ listname2.field(" ", 1);
	listfilename.converter("-.*/", "____");

	// Open the lists file on the same connection
	var lists;
	if (not lists.open("LISTS", var_db(*this))) UNLIKELY {
		let errmsg = "getlist() LISTS file cannot be opened";
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	var keys;
	if (not keys.read(lists, listname)) UNLIKELY
		// throw VarDBException(listname.quote() ^ " list does not exist.");
		return false;

	// Provide first block of keys for readnext
	this->updater(3, listname);

	// List number for readnext to get next block of keys from lists file
	// suffix for first block is nothing (not *1) and then *2, *3 etc
	this->updater(4, 1);

	// Key pointer for readnext to remove next key from the block of keys
	this->updater(5, 0);

	// Keys separated by vm. each key may be followed by a sm and the mv no for readnext
	keys.lowerer();
	this->updater(6, keys);

	return true;
}

bool var_db::selectkeys(in keys) {

	THISIS("bool var::selectkeys(in keys)")
	assertVar(function_sig);
	ISSTRING(keys)

	if (DBTRACE>1)
		this->logputl("DBTR var::selectkeys(" ^ keys.field(_FM, 1, 3).quote() ^ "...) ");

	this->clearselect();

	// Keys must be provided
	if (keys.empty()) {
		const var errmsg = "selectkeys() keys cannot be empty";
		var::setlasterror(errmsg);
		return false;
	}

	// Provide a block of keys for readnext
	//3/4/5/6 setup in selectkeys. cleared in clearselect

	// listid in the lists file must be set for readnext to work, but not exist in the file
	// readnext will look for %SELECTKEYS%*2 in the lists file when it reaches the end of the
	// block of keys provided and must not find it
	this->updater(3, "%SELECTKEYS%");

	// List number for readnext to get next block of keys from lists file
	// suffix for first block is nothing (not *1) and then *2, *3 etc
	this->updater(4, 1);

	// Key pointer for readnext to find next key from the block of keys
	this->updater(5, 0);

	// Keys separated by vm. each key may be followed by a sm and the mv no for readnext
	this->updater(6, keys.lower());

	return true;
}

//bool var::hasnext() const {
bool var_db::hasnext() {

	THISIS("bool var::hasnext() const")
	// assertString(function_sig);

	// default cursor is ""
	this->defaulter("");

	// Readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->f(3);
	if (listid) {
		var keyno = this->f(5);
		keyno++;

		let key_and_mv = this->f(6, keyno);

		// True if we have another key
		if (key_and_mv.len()) LIKELY
			return true;

		if (DBTRACE>1) UNLIKELY
			this->logputl("DBTR var::hasnext(" ^ listid ^ ") ");

		// Otherwise try and get another block
		var lists;
		if (not lists.open("LISTS", var_db(*this))) UNLIKELY {
			let errmsg = "var::hasnext(): LISTS file cannot be opened";
			var::setlasterror(errmsg);
			throw VarDBException(errmsg);
		}

		var listno = this->f(4);
		listno++;
		listid.fieldstorer("*", 2, 1, listno);

		// If no more blocks of keys then return false
		var block;
		if (not block.read(lists, listid)) {

			// Clear the listid
			this->updater(3, "");

			return false;
		}

		// Might as well cache the next block for the next readnext
		this->updater(4, listno);
		this->updater(5, 0);
		block.lowerer();
		this->updater(6, block);

		return true;
	}

	// TODO avoid this trip to the database somehow?
	// Avoid generating sql errors since they abort transactions
	if (not this->dbcursorexists()) UNLIKELY
		return false;

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		let errmsg = "var::hasnext() get_pgconn() failed for " ^ this->quote();
		var::setlasterror(errmsg);
		throw VarDBException(errmsg);
	}
	// The following pair of db requests is rather slow

	// Try to move the cursor forward
	PGresult* pgresult = nullptr;
	int rown;
	if (not readnextx(*this, pgconn, /*direction=*/1, pgresult, &rown)) UNLIKELY
		return false;

	//////////////////////////////
	// Restore the cursor back one
	//////////////////////////////

	readnextx(*this, pgconn, /*direction=*/-1, pgresult, &rown);

	return true;
}

bool var_db::readnext(io key) {
	var valueno;
	return this->readnext(key, valueno);
}

bool var_db::readnext(io key, io valueno) {

	THISIS("bool var::readnext(io key, io valueno) const")
	assertVar(function_sig);

	// Default cursor is ""
	this->defaulter("");

	var record;
	return this->readnext(record, key, valueno);
}

bool var_db::readnext(io record, io key, io valueno) {

	THISIS("bool var::readnext(io record, io key, io valueno) const")
	assertVar(function_sig);
	ISVAR(key)
	ISVAR(record)

	// Default cursor is ""
	this->defaulter("");

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->f(3);
	if (listid) {

		if (DBTRACE>1)
			this->logputl("DBTR var::readnext() ");

		record = "";
		while (true) {
			var keyno = this->f(5);
			keyno++;

			let key_and_mv = this->f(6, keyno);

			// If no more keys, try to get next block of keys, otherwise return false
			if (key_and_mv.len() == 0) {

				// selectkeys provides one block of keys and nothing in the lists file
				if (listid == "%SELECTKEYS%") UNLIKELY {
					this->updater(3, "");
					this->updater(4, "");
					this->updater(5, "");
					this->updater(6, "");
					return false;
				}

				var lists;
				if (not lists.open("LISTS", var_db(*this))) UNLIKELY {
					let errmsg = "readnext() LISTS file cannot be opened";
			        var::setlasterror(errmsg);
					throw VarDBException(errmsg);
				}

				var listno = this->f(4);
				listno++;
				listid.fieldstorer("*", 2, 1, listno);

				var block;
				if (not block.read(lists, listid)) {

					// Clear the listid
					this->updater(3, "");

					return false;
				}

				this->updater(4, listno);
				this->updater(5, 0);
				block.lowerer();
				this->updater(6, block);
				continue;
			}

			// Bump up the key no pointer
			this->updater(5, keyno);

			// Extract and return the key (and mv if present)
			key = key_and_mv.f(1, 1, 1);
			valueno = key_and_mv.f(1, 1, 2);
			return true;
		}
	}

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY
		return false;

	// Avoid generating sql errors since they abort transactions
	if (not this->dbcursorexists()) UNLIKELY
		return false;

	//const DBresult dbresult;
	PGresult* pgresult = nullptr;
	int rown;
	if (not readnextx(*this, pgconn, /*direction=*/1, pgresult, &rown)) UNLIKELY {
		this->clearselect();
		return false;
	}

	//dump_pgresult(pgresult);

	// Key is first column
	key = getpgresultcell(pgresult, rown, 0);
	// TODO return zero if no mv in select because no by mv column

	// Recursive call to skip any meta data with keys starting and ending %
	// e.g. keys like "%RECORDS%" (without the quotes)
	// similar code in readnext()
	if (key.starts("%") and key.ends("%")) UNLIKELY {
		return readnext(record, key, valueno);
	}

	// vn is second column
	// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(dbresult, 0, 1)));
	// vn is second column
	// record is third column
	if (PQnfields(pgresult) > 1)
		// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(dbresult, 0, 1)));
		valueno = getpgresultcell(pgresult, rown, 1);
	else
		valueno = 0;

	// Record may be in the third column
	if (PQnfields(pgresult) < 3) {
		// Dont throw an error, just return empty record. why?
		record = "";
	} else {
		record = getpgresultcell(pgresult, rown, 2);
	}

	return true;
}

bool var_db::createindex(in fieldname0, in dictfile) const {

	THISIS("bool var::createindex(in fieldname, in dictfile) const")
	assertString(function_sig);
	ISSTRING(fieldname0)
	ISSTRING(dictfile)

	let filename = get_normalized_filename(*this);
	let fieldname = fieldname0.convert(".", "_");

	// Actual dictfile to use is either given or defaults to that of the filename
	var actualdictfile;
	if (dictfile.assigned() and dictfile != "")
		actualdictfile = dictfile;
	else
		actualdictfile = "dict." ^ filename;

	// Example sql
	// create index ads__brand_code on ads (exodus.extract_text(data,3,0,0));

	// Throws if cannot find dict file or record
	var joins = "";   // Throw away - cant index on joined fields at the moment
	var unnests = ""; // unnests are only created for ORDER BY, not indexing or selecting
	var selects = "";
	bool ismv;
	bool isdatetime;
	let forsort = false;
	let dictexpression = get_dictexpression(*this, filename, filename, actualdictfile, actualdictfile,
									fieldname, joins, unnests, selects, ismv, isdatetime, forsort);
	// dictexpression.logputl("dictexp=");stop();

	// mv fields return in unnests, not dictexpression
	//if (unnests)
	//{
	//	//dictexpression = unnests.f(3);
	//	unnests.convert(FM,"^").logputl("unnests=");
	//}

	var sql;

	// Index on calculated columns causes an additional column to be created
	if (dictexpression.contains("exodus_call")) UNLIKELY {
		("ERROR: Cannot create index on " ^ filename ^ " for calculated field " ^ fieldname).errputl();
		return false;

		/*

		// add a manually calculated index field
		let index_fieldname = "index_" ^ fieldname;
		sql = "alter table " ^ filename ^ " add " ^ index_fieldname ^ " text";
		if (not var().sqlexec(sql))
		{
			sql.logputl("sql failed ");
			return false;
		}

		// update the new index field for all records
		sql = "update " ^ filename ^ " set " ^ index_fieldname ^ " = " ^ dictexpression;
		sql.logputl("sql=");
		if (not var().sqlexec(sql))
		{
			sql.logputl("sql failed ");
			return false;
		}
		dictexpression = index_fieldname;
		*/

	}

	// Fail neatly if the index already exists
	// SQL errors during a transaction cause the whole transaction to fail.
	if (this->listindex(filename, fieldname)) UNLIKELY {
		var::setlasterror(filename.quote() ^ ", " ^ fieldname.quote() ^ " index already exists.");
		return false;
	}

	// Create postgres index
	sql = "CREATE INDEX index__" ^ filename ^ "__" ^ fieldname ^ " ON " ^ filename;
	//if (ismv or fieldname.lcase().ends("_xref"))
	if (dictexpression.contains("to_tsvector("))
		sql ^= " USING GIN";
	sql ^= " (";
	// unaccent requires "CREATE EXTENSION unaccent" in postgres
	sql ^= dictexpression;
	sql ^= ")";

	var response = "";
	if (not this->sqlexec(sql, response)) UNLIKELY {
		//ERROR:  cannot create index on foreign table "clients"
		//sqlstate:42809 CREATE INDEX index__suppliers__SEQUENCE_XREF ON suppliers USING GIN (to_tsvector('simple',dict_suppliers_SEQUENCE_XREF(suppliers.key, suppliers.data)))
		if (not response.contains("sqlstate:42809"))
			response.errputl();
		return false;
	}

	return true;
}

bool var_db::deleteindex(in fieldname0) const {

	THISIS("bool var::deleteindex(in fieldname) const")
	assertString(function_sig);
	ISSTRING(fieldname0)

	let filename = get_normalized_filename(*this);
	let fieldname = fieldname0.convert(".", "_");

	// Delete the index field (actually only present on calculated field indexes so ignore
	// result) deleting the index field automatically deletes the index
	//var index_fieldname = "index_" ^ fieldname;
	//if (var().sqlexec("alter table " ^ filename ^ " drop " ^ index_fieldname))
	//	return true;

	// Fail neatly if the index does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
	if (not this->listindex(filename, fieldname)) UNLIKELY {
		var::setlasterror(filename.quote() ^ ", " ^ fieldname.quote() ^ " index does not exist.");
		return false;
	}

	// Delete the index.
	let sql = "drop index index__" ^ filename ^ "__" ^ fieldname;
	return this->sqlexec(sql);
}

var  var_db::listfiles() const {

	THISIS("var  var::listfiles() const")
	assertVar(function_sig);

	// from http://www.alberton.info/postgresql_meta_info.html

	let schemafilter = " NOT IN ('pg_catalog', 'information_schema') ";

	var sql =
		"SELECT table_name, table_schema FROM information_schema.tables"
		//" WHERE table_type = 'BASE TABLE' AND";
		" WHERE";
	sql ^= " table_schema " ^ schemafilter;

	sql ^= " UNION SELECT matviewname as table_name, schemaname as table_schema from pg_matviews";
	sql ^= " WHERE schemaname " ^ schemafilter;

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY
		return "";

//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY
//		return "";
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok)
		return "";

	var filenames = "";
	int nfiles = PQntuples(dbresult);
	for (int filen = 0; filen < nfiles; filen++) {
		if (not PQgetisnull(dbresult, filen, 0)) {
			let filename = getpgresultcell(dbresult, filen, 0);
			let schema = getpgresultcell(dbresult, filen, 1);

			// if (schema == "public")
			// Perhaps should ignore all schemas in postgresql's schema search_path
			// Early versions of exodus created all files in public.
			// Now created in default (first element of search path $user, public) i.e. exodus
			// See postgresql.conf search_path = $user, public
			if (schema == "public" or schema == "exodus")
				filenames ^= filename;
			else
				filenames ^= schema ^ "." ^ filename;
			filenames.var_str.push_back(FM_);
		}
	}
	filenames.var_str.pop_back();

	return filenames;
}

var  var_db::dblist() const {

	THISIS("var  var::dblist() const")
	assertVar(function_sig);

	let sql = "SELECT datname FROM pg_database";

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY
		return "";

//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY
//		return "";
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok)
		return "";

	let dbnames = "";
	auto ndbs = PQntuples(dbresult);
	for (auto dbn = 0; dbn < ndbs; dbn++) {
		if (not PQgetisnull(dbresult, dbn, 0)) {
			const std::string dbname = getpgresultcell(dbresult, dbn, 0);
			if (not dbname.starts_with("template") and not dbname.starts_with("postgres")) {
				dbnames.var_str.append(dbname);
				dbnames.var_str.push_back(FM_);
			}
		}
	}
	dbnames.var_str.pop_back();

	return dbnames.sort();
}

bool var_db::dbcursorexists() {

	THISIS("bool var::dbcursorexists()")
	assertVar(function_sig);

	// We MUST avoid generating sql errors because any error aborts transactions

	// Default cursor is ""
	this->defaulter("");

	let cursorcode = this->f(1).convert(".", "_");
	let cursorid = this->f(2) ^ "_" ^ cursorcode;

	// True if exists in cursor cache
	if (thread_dbresults.find(cursorid) != thread_dbresults.end()) LIKELY
		return true;

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY
		return false;

	// from http://www.alberton.info/postgresql_meta_info.html
	let sql = "SELECT name from pg_cursors where name = 'cursor1_" ^ cursorcode ^ "'";

//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY
//		return false;
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok)
		return false;

	return PQntuples(dbresult) > 0;
}

var  var_db::listindex(in filename0, in fieldname0) const {

	THISIS("var  var::listindex(in filename, in fieldname) const")
	assertVar(function_sig);
	ISSTRING(filename0)
	ISSTRING(fieldname0)

	let filename = filename0.f(1);
	let fieldname = fieldname0.convert(".", "_");

	// TODO for some reason doesnt return the exodus index_file__fieldname records
	// perhaps you have to be connected with sufficient postgres rights
	var sql =
		"SELECT relname"
		" FROM pg_class"
		" WHERE oid IN ("
		" SELECT indexrelid"
		" FROM pg_index, pg_class"
		" WHERE";
	if (filename)
		sql ^= " relname = '" ^ filename.lcase() ^ "' AND ";
	// if (fieldname)
	//	sql^=" ???relname = '" ^ fieldname.lcase() ^  "' AND ";
	sql ^=
		" pg_class.oid=pg_index.indrelid"
		" AND indisunique != 't'"
		" AND indisprimary != 't'"
		");";

	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY
		return "";

	// Execute command or return empty string
//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY
//		return "";
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok)
		return "";

	std::string indexnames = "";
	int nindexes = PQntuples(dbresult);
	let lc_fieldname = fieldname.lcase();
	for (int indexn = 0; indexn < nindexes; indexn++) {
		if (not PQgetisnull(dbresult, indexn, 0)) {
			var indexname = getpgresultcell(dbresult, indexn, 0);
			if (indexname.starts("index_")) {
				if (indexname.contains("__")) {
					indexname.substrer(8, 999999);
					indexname.replacer("__", VM);
					if (fieldname and indexname.f(1, 2) != lc_fieldname)
						continue;

					indexnames += indexname.var_str;
					indexnames.push_back(FM_);
				}
			}
		}
	}

	var result = "";
	if (not indexnames.empty()) {
		result.var_str = std::move(indexnames);
		result.var_str.pop_back();
		result.sorter();
	}

	return result;
}

var  var_db::reccount(in filename0) const {

	THISIS("var  var::reccount(in file_or_filename) const")
	assertVar(function_sig);
	ISSTRING(filename0)

	// Use var if filename not provided.
	const auto filename = filename0 ?: (*this);

	// Vacuum first otherwise the count is unreliable
	// Sadly doesnt work in transactions
	if (not this->flushindex(filename))
		var::lasterror().logputl();

	let sql = "SELECT reltuples::integer FROM pg_class WHERE relname = '" ^ filename.f(1).lcase() ^ "';";

	// Get a connection
	const auto pgconn = get_pgconn(filename);
	if (not pgconn) UNLIKELY
		return "";

	// Execute command or return empty string
//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY
//		return "";
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok)
		return "";

	var nrvo = "";
	if (PQntuples(dbresult) and PQnfields(dbresult) > 0 and not PQgetisnull(dbresult, 0, 0)) {
		// nrvo=var((int)ntohl(*(uint32_t*)PQgetvalue(dbresult, 0, 0)));
		nrvo = getpgresultcell(dbresult, 0, 0);
		// Ensure numeric
		nrvo += 0;
	}

	// Might return -1 if unknown
	return nrvo;
}

bool var_db::flushindex(in filename) const {

	THISIS("bool var::flushindex(in filename) const")
	assertVar(function_sig);
	ISSTRING(filename)

	// Skip if in a transaction
	if (this->statustrans()) {
		var::setlasterror("Error: var::flushindex cannot vacuum while transaction is active");
		return false;
	}

	let sql = "VACUUM " ^ filename.f(1).lcase();

	// TODO perhaps should get connection from file if passed a file
	const auto pgconn = get_pgconn(*this);
	if (not pgconn) UNLIKELY {
		var::setlasterror("Error: var::flushindex cannot get a connection " ^ filename);
		return false;
	}

	// Execute command or return false
//	DBresult dbresult;
//	if (not get_dbresult(sql, dbresult, pgconn)) UNLIKELY {
//		var::setlasterror("Error: var::flushindex failed to get a result.");
//		return false;
//	}
	var ok; const DBresult dbresult = get_dbresult(sql, pgconn, ok);
	if (not ok) {
		var::setlasterror("Error: var::flushindex failed to get a result.");
		return false;
	}

	// What is this?
	if (PQntuples(dbresult)) {
		TRACE(PQntuples(dbresult))
	}

	return true;
}

}  // namespace exo
