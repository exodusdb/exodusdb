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

#include <cstring>	//for strcmp strlen

#include <string>
#include <string_view>
#include <array>

// Using map for caches instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// Perhaps switch to this https://youtu.be/M2fKMP47slQ?t=476
//#include <unordered_map>
#include <map>
#include <utility> //for pair

#if defined _MSC_VER  // || defined __CYGWIN__ || defined __MINGW32__
#define WIN32_LEAN_AND_MEAN
#include <DelayImp.h>
#include <windows.h>
#else
#endif

#include <iostream>


// To see Postgres PQlib calls
//////////////////////////////
//
// All calls
//
//  grep -P '\bPQ[\w]+' vardb.cpp --color=always|less
//
// or without transaction related calls
//
//  grep -P '\bPQ[\w]+' *.cpp --color=always|grep -vP 'errorMessage|resultStatus|ntuples|getisnull|cmdTuples|PQexec|getvalue|getlength|nfields|PQfname|resultError'
//
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#include <libpq-fe.h>  //in postgres/include
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
#	include "murmurhash2_64.h" // it has included in vardbconn.h (uint64_t defined)

#else
	// c++ std Hash functions are only required to produce the same result for the same input within a single execution of a program;
	// therefore different processes would not be able to perform coordinated record locking
	// therefore we will use one of the above hashing functions which hash the same on different platforms (hopefully)
	// C++ std hash functions are designed for use in c++ containers, nothing else.
#endif

#include <exodus/varimpl.h>
#include <exodus/vardbconn.h>  // placed as last include, causes boost header compiler errors
//#include <exodus/exoenv.h>
//#include <exodus/mvutf.h>
#include <exodus/dim.h>

#ifdef EXODUS_TIMEBANK
#include "timebank.h"
#endif

#define DBTRACE_SELECT DBTRACE
#define DBTRACE_CONN 0

namespace exodus {

// the idea is for exodus to have access to one standard database without secret password
static const var defaultconninfo =
	"host=127.0.0.1 port=5432 dbname=exodus user=exodus "
	"password=somesillysecret connect_timeout=10";

// Deleter function to close connection and connection cache object
// this is also called in the destructor of DBConnectorCache
// MAKE POSTGRES CONNECTIONS ARE CLOSED PROPERLY OTHERWISE MIGHT RUN OUT OF CONNECTIONS!!!
// TODO so make sure that we dont use exit(n) anywhere in the programs!
static void PGconn_DELETER(PGconn* pgconn) {
	auto pgconn2 = pgconn;
    // at this point we have good new connection to database
    if (DBTRACE) {
        var("").logput("DBTR PQFinish");
		std::clog << pgconn << std::endl;
	}
	//var("========================== deleting connection ==============================").errputl();
	PQfinish(pgconn2);	// AFAIK, it destroys the object by pointer
					//	delete pgp;
}

#define DEBUG_LOG_SQL if (DBTRACE) sql.squote().logputl("SQL0 ");

#define DEBUG_LOG_SQL1 if (DBTRACE) {((this->assigned() ? (this->first(50)) : "") ^ " | " ^ sql.replace("$1", var(paramValues[0]).first(50).squote())).logputl("SQL1 ");}

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
static thread_local int thread_default_data_dbconn_no = 0;
static thread_local int thread_default_dict_dbconn_no = 0;
//thread_local var thread_connparams = "";
static thread_local var thread_lasterror = "";
static thread_local DBConnector thread_dbconnector(PGconn_DELETER);
static thread_local std::map<std::string, std::string> thread_file_handles;

//very few entries so map will be much faster than unordered_map
//thread_local std::unordered_map<std::string, DBresult> thread_dbresults;
class DBresult;
static thread_local std::map<std::string, DBresult> thread_dbresults;

//std::string getpgresultcell(PGresult* pgresult, int rown, int coln) {
//	return std::string(PQgetvalue(pgresult, rown, coln), PQgetlength(pgresult, rown, coln));
//}

static var getpgresultcell(PGresult* pgresult, int rown, int coln) {
	return var(PQgetvalue(pgresult, rown, coln), PQgetlength(pgresult, rown, coln));
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
static var get_normal_filename(CVR filename_or_handle) {
	//return filename_or_handle.f(1).normalize().lcase().convert(".", "_").replace("dict_","dict.");
	// No longer convert . in filenames to _
	return filename_or_handle.f(1).normalize().lcase();
}

// Detect sselect command words that are values like quoted words or plain numbers.
// eg "xxx" 'xxx' 123 .123 +123 -123
static const var valuechars("\"'.0123456789-+");

// hash64 a std::string
///////////////////////
static uint64_t mvdbpostgres_hash_stdstr(std::string str1) {

#if defined(USE_WYHASH)
	return wyhash(str1.data(), str1.size(), 0, _wyp);

#elif defined(USE_MURMURHASH3)

//	uint64_t u128[2];
//	MurmurHash3_x64_128(str1.data(), static_cast<int>(str1.size()), murmurhash_seed, u128);
//
//	static_cast<uint64_t*>(u128)[0] ^= static_cast<uint64_t*>(u128)[1];
//	return static_cast<uint64_t*>(u128)[0];

	// Pass two uint64_t to MurmurHash3
	std::array<uint64_t, 2> u128;

	// MurmurHash3
	MurmurHash3_x64_128(str1.data(), static_cast<int>(str1.size()), murmurhash_seed, &u128[0]);

	// From the MurmurHash3 function's finalizer.
	//
	//  ((uint64_t*)out)[0] = h1;
	//  ((uint64_t*)out)[1] = h2;

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
static uint64_t mvdbpostgres_hash_file_and_key(CVR filehandle, CVR key) {

	// Use the pure filename and disregard any connection number
	std::string fileandkey = get_normal_filename(filehandle);

	// Separate the filename from the key to avoid alternative interpretations
	// eg "FILENAMEKEY" could be FILE, NAMEKEY or FILENAME, KEY
	// but "FILENAME KEY" can only be one thing
	fileandkey.push_back(' ');

	// Append the key
	// Normalise all alternative utf8 encodings of the same unicode points so they hash identically
	fileandkey.append(key.normalize().toString());

	return mvdbpostgres_hash_stdstr(fileandkey);

}

// DBresult is a RAII/SBRM container of a PGresult pointer and calls PQclear on destruction
// Safe and automatic clearing of heap data generated by the postgresql C language PQlib api
class DBresult {

   public:

	// Owns the PGresult object on the stack. Initially none.
	PGresult* pgresult_ = nullptr;

	// Pointer into pgresult if there are many rows
	// e.g. in readnextx after FETCH nn
	int rown_ = 0;

	// Default constructor results in a nullptr
	DBresult() = default;

	// Allow construction from a PGresult*
	//DBresult(PGresult* pgresult)
	//explicit DBresult(PGresult* pgresult)
	DBresult(PGresult* pgresult)
	:
	pgresult_(pgresult) {

		if (DBTRACE) {
			var("DBresult (c) own ").logput();
			std::clog << pgresult_ << std::endl;
		}
	}

	// Prevent copy
	DBresult(const DBresult&) = delete;

	// Move constructor transfers ownership of the PGresult
	DBresult(DBresult&& dbresult)
	:
	rown_(dbresult.rown_) {

		//if already assigned to a different PGresult then clear the old one first
		if (pgresult_ && pgresult_ != dbresult.pgresult_) {
			if (DBTRACE) {
				var("DBresult (m) PQC ").logput();
				std::clog << pgresult_ << std::endl;
			}
			PQclear(pgresult_);
			pgresult_ = nullptr;
		}

		pgresult_=dbresult.pgresult_;
		dbresult.pgresult_ = nullptr;

		if (DBTRACE) {
			var("DBresult (m) own ").logput();
			std::clog << pgresult_ << std::endl;
		}
	}

	// Allow assignment from a PGresult*
	void operator=(PGresult* pgresult) {

		rown_ = 0;

		//if already assigned to a different PGresult then clear the old one first
		if (pgresult_ && pgresult_ != pgresult) {
			if (DBTRACE) {
				var("DBresult (=) PQC ").logput();
				std::clog << pgresult_ << std::endl;
			}
			PQclear(pgresult_);
		}

		pgresult_ = pgresult;
		if (DBTRACE) {
			var("DBresult (=) own ").logput();
			std::clog << pgresult_ << std::endl;
		}
	}

	// Allow implicit conversion to a PGresult*
	// No tiresome .get() required as in unique_ptr
	operator PGresult*() const {
		return pgresult_;
	}

	// Destructor calls PQClear
	// This the whole point of having DBresult.
	~DBresult() {
		if (pgresult_) {
			if (DBTRACE) {
				var("DBresult (~) PQC ").logput();
				std::clog << pgresult_ << std::endl;
			}
			PQclear(pgresult_);
		}
	}
};

static int get_dbconn_no(CVR dbhandle) {

	if (!dbhandle.assigned()) {
		// var("get_dbconn_no() returning 0 - unassigned").logputl();
		return 0;
	}
	var dbconn_no = dbhandle.f(2);
	if (dbconn_no.isnum()) {
		/// var("get_dbconn_no() returning " ^ dbconn_no).logputl();
		return dbconn_no;
	}

	// var("get_dbconn_no() returning 0 - not numeric").logputl();

	return 0;
}

static int get_dbconn_no_or_default(CVR dbhandle) {

	int dbconn_no = get_dbconn_no(dbhandle);
	if (dbconn_no)
		return dbconn_no;

	// otherwise get the default connection
	if (DBTRACE_CONN >= 3 and dbhandle.assigned())
		TRACE(dbhandle)

	//dbhandle MUST always arrive in lower case to detect if "dict."
	bool isdict = dbhandle.unassigned() ? false : dbhandle.starts("dict.");
	//bool isdict = dbhandle.unassigned() ? false : (dbhandle.starts("dict.") || dbhandle.starts("DICT."));
	//bool isdict = false;

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

	// otherwise try the default connection
	if (!dbconn_no) {

		var defaultdb;

		//look for dicts in the following order
		//1. $EXO_DICT if defines
		//2. db "dict" if present
		//3. the default db connection
		if (isdict) {
			if (not defaultdb.osgetenv("EXO_DICT") or not defaultdb)
				//must be the same in mvdbpostgres.cpp and dict2sql
				defaultdb="exodus";
		} else {
			defaultdb = "";
		}

		if (DBTRACE_CONN >= 3)
			TRACE(defaultdb)

		//try to connect
		if (defaultdb.connect()) {
			dbconn_no = get_dbconn_no(defaultdb);

			if (DBTRACE_CONN >= 3 )
				TRACE("defaultdb connected")

		//if cannot connect then for dictionaries look on default connection
		} else if (isdict) {

			//attempt a default connection if not already done
			if (!thread_default_data_dbconn_no) {
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

		//save default dict/data connections
		if (isdict) {
			thread_default_dict_dbconn_no = dbconn_no;
			if (DBTRACE or DBTRACE_CONN) {
				var(dbconn_no).logputl("DBTR NEW DEFAULT DICT CONN ");
			}
		}
		else {
			thread_default_data_dbconn_no = dbconn_no;
			if (DBTRACE or DBTRACE_CONN) {
				var(dbconn_no).logputl("DBTR NEW DEFAULT DATA CONN ");
			}
		}
	}

	//save the connection number in the dbhandle
	//if (dbconn_no) {
	//	dbhandle.default_to("");
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
static PGconn* get_pgconn(CVR dbhandle) {

	// var("--- connection ---").logputl();
	// get the connection associated with *this
	int dbconn_no = get_dbconn_no_or_default(dbhandle);
	// var(dbconn_no).logputl("dbconn_no1=");

	// otherwise fail
	if (!dbconn_no) [[unlikely]] 
//		throw VarDBException("pgconnection() requested when not connected.");
		return nullptr;

	if (DBTRACE) {
		std::cout << std::endl;
		PGconn* pgconn=thread_dbconnector.get_pgconn(dbconn_no);
		std::clog << "CONN " << dbconn_no << " " << pgconn << std::endl;
	}

	// return the relevent pg_connection structure
	auto pgconn = thread_dbconnector.get_pgconn(dbconn_no);
	//TODO error abort if zero
	return pgconn;

}

// gets lock_table, associated with connection, associated with this object
static DBConn* get_dbconn(CVR dbhandle) {
	int dbconn_no = get_dbconn_no_or_default(dbhandle);
	if (!dbconn_no) [[unlikely]] {
		var errmsg = "get_dbconn() attempted when not connected";
		throw VarDBException(errmsg);
	}
	return thread_dbconnector.get_dbconn(dbconn_no);
}

//static bool get_dbresult(CVR sql, DBresult& dbresult, PGconn* pgconn);
// used for sql commands that require no parameters
// returns 1 for success
// returns 0 for failure
// dbresult is returned to caller to extract any data and call PQclear(dbresult) in destructor of DBresult
static bool get_dbresult(CVR sql, DBresult& dbresult, PGconn* pgconn) {
	DEBUG_LOG_SQL

	/* dont use PQexec because is cannot be told to return binary results
	and use PQexecParams with zero parameters instead
	//execute the command
	dbresult = get_dbresult(pgconn, sql.var_str.c_str());
	dbresult = dbresult;
	*/

	// Parameter array with no parameters
	const char* paramValues[1];
	int paramLengths[1];

	// will contain any dbresult IF successful
	dbresult = PQexecParams(pgconn, sql.toString().c_str(), 0, /* zero params */
							nullptr,								/* let the backend deduce param type */
							paramValues, paramLengths,
							nullptr,  // text arguments
							0);	// text results

	// Handle serious error. Why not throw?
	if (!dbresult) {
		var("ERROR: mvdbpostgres PQexec command failed, no error code: ").errputl();
		return false;
	}

	switch (PQresultStatus(dbresult)) {

		case PGRES_COMMAND_OK:
			return true;

		case PGRES_TUPLES_OK:
			return PQntuples(dbresult) > 0;

		case PGRES_NONFATAL_ERROR:

			var("ERROR: mvdbpostgres SQL non-fatal error code " ^
				var(PQresStatus(PQresultStatus(dbresult))) ^ ", " ^
				var(PQresultErrorMessage(dbresult)))
				.errputl();

			return true;

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

			var("ERROR: mvdbpostgres pqexec " ^ var(sql)).errputl();
			var("ERROR: mvdbpostgres pqexec " ^
				var(PQresStatus(PQresultStatus(dbresult))) ^ ": " ^
				var(PQresultErrorMessage(dbresult)))
				.errputl();

			return false;
#pragma clang diagnostic push
	}

	// should never get here
	//std::unreachable
	//return false;

}

#if defined _MSC_VER  //|| defined __CYGWIN__ || defined __MINGW32__
LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pExcPointers) {
	LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;

	PDelayLoadInfo pDelayLoadInfo =
		PDelayLoadInfo(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

	switch (pExcPointers->ExceptionRecord->ExceptionCode) {
		case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
			printf("mvdbpostgres: %s was not found\n", pDelayLoadInfo->szDll);
			break;
			/*
			case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
			if (pdli->dlp.fImportByName) {
				printf("Function %s was not found in %sn",
				pDelayLoadInfo->dlp.szProcName, pDelayLoadInfo->szDll);
			} else {
			printf("Function ordinal %d was not found in %sn",
				pDelayLoadInfo->dlp.dwOrdinal, pDelayLoadInfo->szDll);
			}
			break;
		*/
		default:
			// Exception is not related to delay loading
			printf("Unknown problem %s\n", pDelayLoadInfo->szDll);
			lDisposition = EXCEPTION_CONTINUE_SEARCH;
			break;
	}

	return (lDisposition);
}

// msvc uses a special mode to catch failure to load a delay loaded dll that is incompatible with
// the normal try/catch and needs to be put in simple global function with no complex objects (that
// require standard c++ try/catch stack unwinding?) maybe it would be easier to manually load it
// using dlopen/dlsym implemented in var as var::load and var::call
// http://msdn.microsoft.com/en-us/library/5skw957f(vs.80).aspx
static bool msvc_PQconnectdb(PGconn** pgconn, const std::string& conninfo) {
	// connect or fail
	__try {
		*pgconn = PQconnectdb(conninfo.c_str());
	} __except (DelayLoadDllExceptionFilter(GetExceptionInformation())) {
		return false;
	}
	return true;
}

#endif

static var build_conn_info(CVR conninfo) {
	// priority is
	// 1) given parameters //or last connection parameters
	// 2) individual environment parameters
	// 3) environment connection string
	// 4) config file parameters
	// 5) hard coded default parameters

	var result(conninfo);
	result.trimmer();

	// if no conninfo details provided then use last connection details if any
	//if (!conninfo)
	//	result = thread_connparams;

	// Remove spaces around = to enable parsing using space
	result.regex_replacer("\\s+=\\s+", "=");

	// otherwise search for details from exodus config file
	// if incomplete connection parameters provided
	if (not result.contains("host=") or not result.contains("port=") or not result.contains("dbname=") or
		not result.contains("user=") or not result.contains("password=")) {

		// discover any configuration file parameters
		// TODO parse config properly instead of just changing \r\n to spaces!
		var configfilename = "";
		var home = "";
		if (home.osgetenv("HOME"))
			configfilename = home ^ "/.config/exodus/exodus.cfg";
		else if (home.osgetenv("USERPROFILE"))
			configfilename ^= home ^ "\\Exodus\\.exodus";
		var configconn = "";
		if (!configconn.osread(configfilename) and !configconn.osread("exodus.cfg"))
			configconn = "";
		//postgres ignores after \n?
		configconn.converter("\r\n","  ");

		// discover any configuration in the environment
		var envconn = "";
		var temp;
		if (temp.osgetenv("EXO_CONN") && temp)
			envconn ^= " " ^ temp;

		// specific variable are appended ie override
		if (temp.osgetenv("EXO_HOST") && temp)
			envconn ^= " host=" ^ temp;

		if (temp.osgetenv("EXO_PORT") && temp)
			envconn ^= " port=" ^ temp;

		if (temp.osgetenv("EXO_USER") && temp)
			envconn ^= " user=" ^ temp;

		if (temp.osgetenv("EXO_DATA") && temp) {
			envconn.regex_replacer(R"(dbname\s*=\s*\w*)", "");
			envconn ^= " dbname=" ^ temp;
		}

		if (temp.osgetenv("EXO_PASS") && temp)
			envconn ^= " password=" ^ temp;

		if (temp.osgetenv("EXO_TIME") && temp)
			envconn ^= " connect_timeout=" ^ temp;

		result = defaultconninfo ^ " " ^ configconn ^ " " ^ envconn ^ " " ^ result;
	}

	// Remove excess spaces. Especially around = to enable parsing using space
	result.regex_replacer("\\s+=\\s+", "=");
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

static inline void unquoter_inline(VARREF iovar) {
	// remove "", '' and {}
	static var quotecharacters("\"'{");
	if (quotecharacters.contains(iovar.first()))
		//string = string.b(2, string.len() - 2);
		//iovar = iovar.cut(1).popper();
		iovar.cutter(1).popper();
}

static void tosqlstring(VARREF string1) {

	// if double quoted then convert to sql style single quoted strings
	// double up any internal single quotes
	if (string1.starts("\"")) {
	//if (string1.var_str.front() == '"') {
		string1.replacer("'", "''");
		string1.paster(1, 1, "'");
		string1.paster(-1, 1, "'");
		//string1.var_str.front() = "'";
		//string1.var_str.back() = "'";
	}
}

static var get_fileexpression([[maybe_unused]] CVR mainfilename, CVR filename, CVR keyordata) {

	// if (filename == mainfilename)
	//	return keyordata;
	// else
	//return filename.convert(".", "_") ^ "." ^ keyordata;
	return get_normal_filename(filename) ^ "." ^ keyordata;

	// if you dont use STRICT in the postgres function declaration/definitions then nullptr
	// parameters do not abort functions

	// use COALESCE function in case this is a joined but missing record (and therefore null)
	// in MYSQL this is the ISNULL expression?
	// xlatekeyexpression="exodus.extract_text(coalesce(" ^ filename ^ ".data,''::text), " ^
	// xlatefromfieldname.cut(8); if (filename==mainfilename) return expression; return
	// "coalesce(" ^ expression ^", ''::text)";
}

// Used in var::selectx
static void to_extract_text(VARREF dictexpression) {
				dictexpression.regex_replacer("^exodus.extract_number\\(", "exodus.extract_text\\(");
				dictexpression.regex_replacer("^exodus.extract_sort\\(", "exodus.extract_text\\(");
				dictexpression.regex_replacer("^exodus.extract_date\\(", "exodus.extract_text\\(");
				dictexpression.regex_replacer("^exodus.extract_time\\(", "exodus.extract_text\\(");
				dictexpression.regex_replacer("^exodus.extract_datetime\\(", "exodus.extract_text\\(");
}

bool var::connect(CVR conninfo) {

	THISIS("bool var::connect(CVR conninfo")
	// nb dont log/trace or otherwise output the full connection info without HIDING the
	// password
	assertDefined(function_sig);
	ISSTRING(conninfo)

	if (DBTRACE or DBTRACE_CONN) {
		TRACE(__PRETTY_FUNCTION__)
		TRACE(conninfo)
	}
	var fullconninfo = conninfo.trimboth();

	//use *this if conninfo not specified;
	bool isdefault = false;
	if (!fullconninfo) {
		if (this->assigned())
			fullconninfo = *this;
		isdefault = !fullconninfo;
		if (DBTRACE_CONN) {
			TRACE(fullconninfo)
			TRACE(isdefault)
			TRACE(thread_default_data_dbconn_no)
		}
		if (isdefault && thread_default_data_dbconn_no) {
			return thread_default_data_dbconn_no;
		}
	}

	//add dbname= if missing
	if (fullconninfo && !fullconninfo.contains("="))
		fullconninfo = "dbname=" ^ fullconninfo.lcase();

	fullconninfo = build_conn_info(fullconninfo);

	if (DBTRACE_CONN) {
		//fullconninfo.replace(R"(password\s*=\s*\w*)", "password=**********").logputl("var::connect( ) ");
		fullconninfo.regex_replace(R"(password\s*=\s*\w*)", "password=**********").logputl("\nvar::connect( ) ");
	}

	PGconn* pgconn;
	for (;;) {

#if defined _MSC_VER  //|| defined __CYGWIN__ || defined __MINGW32__
		if (not msvc_PQconnectdb(&pgconn, fullconninfo.var_str)) {
			var libname = "libpq.dl";
			// var libname="libpq.so";
			var errmsg="ERROR: mvdbpostgres connect() Cannot load shared library " ^ libname ^
				". Verify configuration PATH contains postgres's \\bin.";
			this->setlasterror(errmsg);
			return false;
		}
#else
		//connect
		pgconn = PQconnectdb(fullconninfo.var_str.c_str());
#endif

		//connected OK
		if (PQstatus(pgconn) == CONNECTION_OK || fullconninfo)
			break;

		// required even if connect fails according to docs
		PQfinish(pgconn);

		// try again with default conninfo
		fullconninfo = defaultconninfo;

	}

	// failed to connect so return false
	if (PQstatus(pgconn) != CONNECTION_OK) {

		var errmsg = "ERROR: mvdbpostgres connect() Connection to database failed: " ^ var(PQerrorMessage(pgconn));

		this->setlasterror(errmsg);

		// required even if connect fails according to docs
		PQfinish(pgconn);

		return false;
	}

// abort if multithreading and it is not supported
#ifdef PQisthreadsafe
	if (!PQisthreadsafe()) [[unlikely]] {
		// TODO only abort if environmentn>0
		var errmsg = "connect(): Postgres PQ library is not threadsafe";
		throw VarDBException(errmsg);

	}
#endif

	// at this point we have good new connection to database

	// cache the new connection handle
	int dbconn_no = thread_dbconnector.add_dbconn(pgconn, fullconninfo.var_str);
	if (DBTRACE_CONN) {
		TRACE(thread_default_data_dbconn_no)
		TRACE(thread_default_dict_dbconn_no)
	}
	//(*this) = conninfo ^ FM ^ conn_no;
	if (!this->assigned())
		(*this) = "";
	if (not this->f(1))
		//this->r(1,fullconninfo.field(" ",1));
		this->r(1,fullconninfo.field2("dbname=", -1).field(" ", 1));
	(*this)(2) = dbconn_no;
	(*this)(3) = dbconn_no;

	if (DBTRACE_CONN) {
		fullconninfo.regex_replace(R"(password\s*=\s*\w*)", "password=**********").logputl("var::connect() OK ");
		this->logput("var::connect() OK ");
		std::clog << " " << pgconn << std::endl;
	}

	// this->outputl("new connection=");

	// set default connection - ONLY IF THERE ISNT ONE ALREADY
	if (isdefault && !thread_default_data_dbconn_no) {
		thread_default_data_dbconn_no = dbconn_no;
		if (DBTRACE_CONN) {
			this->logputl("NEW DEFAULT DATA CONN " ^ var(dbconn_no) ^ " on ");
		}

	}

	// save last connection string (used in startipc())
	//thread_connparams = fullconninfo;

	// doesnt work
	// need to set PQnoticeReceiver to suppress NOTICES like when creating files
	// PQsetErrorVerbosity(pgconn, PQERRORS_TERSE);
	// but this does
	// this turns off the notice when creating tables with a primary key
	// DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, LOG, NOTICE, WARNING, ERROR, FATAL, and PANIC
	//this->sqlexec(var("SET client_min_messages = ") ^ (DBTRACE ? "LOG" : "WARNING"));
	if (not this->sqlexec(var("SET client_min_messages = ") ^ (DBTRACE ? "LOG" : "WARNING")))
		this->loglasterror();

	return true;
}

// conn1.attach("filename1^filename2...");
bool var::attach(CVR filenames) {

	THISIS("bool var::attach(CVR filenames")
	assertDefined(function_sig);
	ISSTRING(filenames)

	//option to attach all dict files
	var filenames2;
	if (filenames == "dict") {
		filenames2 = "";
		var allfilenames = this->listfiles();
		for (var filename : allfilenames) {
			if (filename.starts("dict.")) {
				filenames2 ^= filename ^ FM;
			}
		}
		filenames2.popper();
	}
	else {
		filenames2 = filenames;
	}

	// cache file handles in thread_file_handles
	var notattached_filenames = "";
	for (var filename : filenames2) {
		const var normal_filename = get_normal_filename(filename);
		var file;
		if (file.open(normal_filename,*this)) {
			// Similar code in dbattach and open
			thread_file_handles[normal_filename] = file.var_str;
			if (DBTRACE)
				file.logputl("DBTR var::attach() ");
		}
		else {
			notattached_filenames ^= normal_filename ^ " ";
		}
	}

	//fail if anything not attached
	if (notattached_filenames) {
		var errmsg = "ERROR: mvdbpostgres/attach: " ^ notattached_filenames ^ "cannot be attached on connection " ^ this->f(1).quote();
		this->setlasterror(errmsg);
		return false;
	}

	return true;
}

// conn1.detach("filename1^filename2...");
void var::detach(CVR filenames) {

	THISIS("bool var::detach(CVR filenames")
	assertDefined(function_sig);
	ISSTRING(filenames)

	for (var filename : filenames) {
		// Similar code in detach and deletefile
		thread_file_handles.erase(get_normal_filename(filename));
	}
	return;
}

// if this->obj contains connection_id, then such connection is disconnected with this-> becomes UNA
// Otherwise, default connection is disconnected
void var::disconnect() {

	THISIS("bool var::disconnect()")
	assertDefined(function_sig);

	if (DBTRACE or DBTRACE_CONN)
		(this->assigned() ? *this : var("")).logputl("DBTR var::disconnect() ");

	// Disconnect the specified connection or default data connection
	var dbconn_no = get_dbconn_no(*this);
	if (!dbconn_no)
		dbconn_no = thread_default_data_dbconn_no;

	// Quit if no connection
	if (not dbconn_no)
		return;

	//disconnect
	// note singular form of dbconn
	thread_dbconnector.del_dbconn(dbconn_no);
	var_typ = VARTYP_UNA;

	// if we happen to be disconnecting the same connection as the default connection
	// then reset the default connection so that it will be reconnected to the next
	// connect this is rather too smart but will probably do what people expect
	if (dbconn_no == thread_default_data_dbconn_no) {
		thread_default_data_dbconn_no = 0;
		if (DBTRACE or DBTRACE_CONN) {
			var(dbconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DATA ");
		}
	}

	// if we happen to be disconnecting the same connection as the default connection FOR DICT
	// then reset the default connection so that it will be reconnected to the next
	// connect this is rather too smart but will probably do what people expect
	if (dbconn_no == thread_default_dict_dbconn_no) {
		thread_default_dict_dbconn_no = 0;
		if (DBTRACE or DBTRACE_CONN) {
			var(dbconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DICT ");
		}
	}

	// Remove all cached files handles referring to the disconnected connection
	std::erase_if(
		thread_file_handles,
		[dbconn_no](auto iter){
			var filehandle = iter.second;
			// Don't erase if not the desired dbconn_no
			if (filehandle.f(2) != dbconn_no)
				return false;
			if (DBTRACE_CONN >= 3) {
				var(iter.second).logputl("var::disconnect() remove cached filehandle ");
			}
			// Do erase this file handle cache entry
			return true;
		}
	);

	return;
}

void var::disconnectall() {

	THISIS("bool var::disconnectall()")
	assertDefined(function_sig);

	var dbconn_no = get_dbconn_no(*this);
	if (!dbconn_no)
		dbconn_no = 2;

	if (DBTRACE)
		dbconn_no.logputl("DBTR var::disconnectall() >= ");

//	// Note the plural for dbconn"S" to delete all starting from
//	thread_dbconnector.del_dbconns(dbconn_no);
//
//	if (thread_default_data_dbconn_no >= dbconn_no) {
//		thread_default_data_dbconn_no = 0;
//		if (DBTRACE) {
//			var(dbconn_no).logputl("DBTR var::disconnectall() DEFAULT CONN FOR DATA ");
//		}
//	}
//
//	if (thread_default_dict_dbconn_no >= dbconn_no) {
//		thread_default_dict_dbconn_no = 0;
//		if (DBTRACE) {
//			var(dbconn_no).logputl("DBTR var::disconnectall() DEFAULT CONN FOR DICT ");
//		}
//	}

	// Disconnect all connections >= dbconn_no
	auto max_dbconn_no = thread_dbconnector.max_dbconn_no();
	for (auto dbconn_no2 = dbconn_no; dbconn_no2 <= max_dbconn_no; ++dbconn_no2) {
		var connection2 = FM ^ dbconn_no2;
		connection2.disconnect();
	}

	// Make sure the max dbconn_no is reduced
	// although the connections will have already been disconnected
	// Note the PLURAL form of dbconn"S" to delete all starting from
	thread_dbconnector.del_dbconns(dbconn_no);

	return;
}

// open filehandle given a filename on current thread-default connection
// we are using strict filename/filehandle syntax (even though we could use one variable for both!)
// we store the filename in the filehandle since that is what we need to generate read/write sql
// later usage example:
// var file;
// var filename="customers";
// if (not file.open(filename)) ...

// connection is optional and default connection may be used instead
bool var::open(CVR filename, CVR connection /*DEFAULTNULL*/) {

	THISIS("bool var::open(CVR filename, CVR connection)")
	assertDefined(function_sig);
	ISSTRING(filename)

	const var normal_filename = get_normal_filename(filename);

	// filename dos or DOS  means osread/oswrite/osremove
	if (normal_filename.var_str.size() == 3 && normal_filename.var_str == "dos") {
		//(*this) = "dos";
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
		auto entry = thread_file_handles.find(normal_filename);
		if (entry != thread_file_handles.end()) {

			//(*this) = thread_file_handles.at(normal_filename);
			auto cached_file_handle = entry->second;

			// Make sure the connection is still valid otherwise redo the open
			auto pgconn = get_pgconn(cached_file_handle);
			if (! pgconn) {
				thread_file_handles.erase(normal_filename);
				//var(cached_file_handle).errputl("==== Connection cache INVALID = ");
			} else {
				//var(cached_file_handle).errputl("==== Connection cache VALID   = ");
				var_str = cached_file_handle;
				var_typ = VARTYP_NANSTR;
				if (DBTRACE)
					this->logputl("DBTR open() cached or attached ");
				return true;
			}
		}

		// Or determine connection from filename in case filename is a handle
		//use default data or dict connection
		connection2 = normal_filename;

	}

#ifdef EXODUS_TIMEBANK
	//Timer t2(245);//open cache_miss
	Timer timer1(get_timeacno("bool var::open cache_miss"));
	//if (DBTRACE) {
	//	connection2.logputl("DBTR var::open-1 ");
	//}
#endif

	var tablename;
	var schema;
	var and_schema_clause;
	if (normal_filename.contains(".")) {
		schema = normal_filename.field(".",1);
		tablename = normal_filename.field(".",2,999);
		and_schema_clause = " AND table_schema = " ^ schema.squote();
	} else {
		schema = "public";
		tablename = normal_filename;
		//no schema filter allows opening temporary files with exist in various pg_temp_xxxx schemata
		//and_schema_clause = "";
		and_schema_clause = " AND table_schema != 'dict'";
	}
	// 1. look in information_schema.tables
	var sql =
		"\
		SELECT\
		EXISTS	(\
				SELECT table_name\
				FROM   information_schema.tables\
				WHERE  table_name = " ^ tablename.squote() ^ and_schema_clause ^ "\
				)";
	var result;
	if (not connection2.sqlexec(sql, result))
		[[unlikely]]
		//throw VarDBException(this->lasterror());
		throw VarDBException(result);
	//result.convert(RM,"|").logputl("result=");

	//if (DBTRACE) {
	//	connection2.logputl("DBTR var::open-2 ");
	//}

	// 2. look in materialised views
	// select matviewname from pg_matviews where matviewname = '...';
	if (result.last() != "t") {
		sql =
			"\
			SELECT\
			EXISTS	(\
				SELECT  matviewname as table_name\
				FROM    pg_matviews\
				WHERE\
						schemaname = '" ^ schema ^ "'\
						and matviewname = '" ^ tablename ^	"'\
					)\
		";
		if (not connection2.sqlexec(sql, result))
			[[unlikely]]
			throw VarDBException(lasterror());
	}

	//failure if not found
	if (result.last() != "t") {
		var errmsg = "ERROR: mvdbpostgres 2 open(" ^ filename.quote() ^
					") file does not exist.";
		this->setlasterror(errmsg);
		return false;
	}
	// */

	//this->setlasterror();
	// var becomes a filehandle containing the filename and connection no
//	(*this) = normal_filename ^ FM ^ get_dbconn_no_or_default(connection2);
//	normal_filename.var_str.push_back(FM_);
//	normal_filename.var_str.append(std::to_string(get_dbconn_no_or_default(connection2)));
//	var_str = normal_filename.var_str;
//	var_typ = VARTYP_NANSTR;
	var_str = normal_filename.var_str;
	var_str.push_back(FM_);
	var_str.append(std::to_string(get_dbconn_no_or_default(connection2)));
	var_typ = VARTYP_NANSTR;

	// Cache the filehandle so future opens return the same
	// Similar code in dbattach and open
	thread_file_handles[normal_filename] = var_str;

	if (DBTRACE)
		this->logputl("DBTR var::open-3 ");

	return true;
}

void var::close() {

	THISIS("void var::close()")
	assertString(function_sig);
	/*TODO
		if (var_typ!=VARTYP_UNA) QMClose(var_int);
	*/
}

bool var::readf(CVR filehandle, CVR key, const int fieldno) {
	//THISIS("bool var::readf(CVR filehandle,CVR key,const int fieldno)")
	//assertDefined(function_sig);
	//ISSTRING(filehandle)
	//ISSTRING(key)

	if (!this->read(filehandle, key))
		return false;

	var_str = this->f(fieldno).var_str;
	var_typ = VARTYP_STR;

	return true;
}

bool var::readc(CVR filehandle, CVR key) {

	THISIS("bool var::readc(CVR filehandle,CVR key)")
	assertDefined(function_sig);
	//ISSTRING(filehandle)
	//ISSTRING(key)
	ISSTRING(filehandle)
	ISSTRING(key)

	// check cache first, and return any cached record
	int dbconn_no = get_dbconn_no_or_default(filehandle);
	if (!dbconn_no)
		[[unlikely]]
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
	// TODO cache non-existent records as empty
	auto hash64 = mvdbpostgres_hash_file_and_key(filehandle, key);
	//TRACE("readc " ^ filehandle ^ " " ^ key ^ " " ^ var(hash64 % 1'000'000'000)) //modulo to bring the uint64 into range that var can handle without throwing overflow
	std::string cachedrecord;
	if (thread_dbconnector.getrecord(dbconn_no, hash64, cachedrecord)) {
		if (cachedrecord.empty()) {
			//TRACE("cache hit-" ^ key);
			return false;
		} else {
			//TRACE("cache hit+" ^ key);
			var_str = std::move(cachedrecord);
			var_typ = VARTYP_STR;
			return true;
		}
	}
	//TRACE("cache miss" ^ key);

#ifdef EXODUS_TIMEBANK
	//Timer t2(247);//readc cache_miss
	Timer timer1(get_timeacno("bool var::reado cache_miss"));
#endif
	// Ordinary read from the database
	bool result = this->read(filehandle, key);

	if (result) {
		// Cache the ordinary read
		//this->writec(filehandle, key);
		//TRACE("cache add+ " ^ key);
		thread_dbconnector.putrecord(dbconn_no, hash64, var_str);
	} else {
		// Empty if failure
		var("").writec(filehandle, key);
		//TRACE("cache add-" ^ key);
		thread_dbconnector.putrecord(dbconn_no, hash64, "");

	}

	return result;
}

bool var::writec(CVR filehandle, CVR key) const {

	THISIS("void var::writec(CVR filehandle,CVR key)")
	assertString(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int dbconn_no = get_dbconn_no_or_default(filehandle);
	if (!dbconn_no)
		[[unlikely]]
		throw VarDBException("get_dbconn_no() failed");

	auto hash64 = mvdbpostgres_hash_file_and_key(filehandle, key);
	//TRACE("writec " ^ filehandle ^ " " ^ key ^ " " ^ var(hash64 % 1'000'000'000)) //modulo to bring the uint64 into range that var can handle without throwing overflow
	thread_dbconnector.putrecord(dbconn_no, hash64, var_str);

	return true;
}

bool var::deletec(CVR key) const {

	THISIS("bool var::deletec(CVR key)")
	assertString(function_sig);
	ISSTRING(key)

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int dbconn_no = get_dbconn_no_or_default(*this);
	if (!dbconn_no)
		[[unlikely]]
		throw VarDBException("get_dbconn_no() failed");

	auto hash64 = mvdbpostgres_hash_file_and_key(*this, key);
	//TRACE("deletec " ^ *this ^ " " ^ key ^ " " ^ var(hash64 % 1'000'000'000)) //modulo to bring the uint64 into range that var can handle without throwing overflow
	return thread_dbconnector.delrecord(dbconn_no, hash64);
}

bool var::read(CVR filehandle, CVR key) {

	THISIS("bool var::read(CVR filehandle,CVR key)")
	assertDefined(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	//amending var_str invalidates all flags
	//var_typ = VARTYP_STR;
	//var_typ = VARTYP_UNA;
	//var_str.resize(0);

	// Initialise the record var to unassigned
	// unless record and key are the same variable
	// in which case allow failure to read to leave the record (key) untouched
	// PICKOS leaves the variable untouched but we decide
	// to make use of a variable after unsuccessful read to be runtime error
	if (this != &key) {
		var_typ = VARTYP_UNA;
		var_str.clear();
	}

	// lower case key if reading from dictionary
	// std::string key2;
	// if (filehandle.lcase().starts("dict.)")
	//	key2=key.lcase().var_str;
	// else
	//	key2=key.var_str;
	std::string key2 = key.normalize().var_str;

	// dos or DOS filehandle means osread/oswrite/osremove
	if (filehandle.var_str.size() == 3 && (filehandle.var_str == "dos" || filehandle.var_str == "DOS")) {
		//return this->osread(key2);  //.convert("\\",OSSLASH));
		//use osfilenames unnormalised so we can read and write as is
		return this->osread(key);  //.convert("\\",OSSLASH));
	}

	// asking to read DOS file! do osread using key as osfilename!
	if (filehandle == "") [[unlikely]] {
		var errmsg = "read(...) filename not specified, probably not opened.";
		this->setlasterror(errmsg);
		throw VarDBException(errmsg);
	}

	// reading a magic special key returns all keys in the file in natural order
	if (key == "%RECORDS%") {
		var sql = "SELECT key from " ^ get_normal_filename(filehandle) ^ ";";

		auto pgconn = get_pgconn(filehandle);
		if (! pgconn)
			return false;

		DBresult dbresult;
		if (!get_dbresult(sql, dbresult, pgconn))
			return false;

		// *this = "";
		var_str.clear();
		var_typ = VARTYP_STR;

		var keyn;
		int ntuples = PQntuples(dbresult);
		for (int tuplen = 0; tuplen < ntuples; tuplen++) {
			if (!PQgetisnull(dbresult, tuplen, 0)) {
				var key = getpgresultcell(dbresult, tuplen, 0);
				//skip metadata keys starting and ending in % eg "%RECORDS%"
				if (key.first() != "%" && key.last() != "%") {
					if (this->len() <= 65535) {
						if (!this->locatebyusing("AR", _FM, key, keyn))
							this->inserter(keyn, key);
					} else {
						var_str.append(key.var_str);
						var_str.push_back(FM_);
					}
				}
			}
		}

		//remove any trailing FM
		if (var_str.back() == FM_)
			var_str.pop_back();

		//this->setlasterror();

		return true;
	}

	// get filehandle specific connection or fail
	auto pgconn = get_pgconn(filehandle);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::read() get_pgconn() failed for " ^ filehandle;
		this->setlasterror(errmsg);
		// // this->loglasterror(errmsg);
		throw VarDBException(errmsg);
	}
	// Parameter array
	const char* paramValues[] = {key2.data()};
	int paramLengths[] = {static_cast<int>(key2.size())};

	var sql = "SELECT data FROM " ^ get_normal_filename(filehandle) ^ " WHERE key = $1";

	DEBUG_LOG_SQL1
	//DBresult dbresult = PQexecParams(pgconn,
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(), 1, /* one param */
											nullptr,					/* let the backend deduce param type */
											paramValues, paramLengths,
											nullptr,	// text arguments
											0);	// text results

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_TUPLES_OK) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg =
			"read(" ^ filehandle.convert("." _FM, "_^").replace("dict_","dict.").quote() ^ ", " ^ key.quote() ^ ")";
		if (sqlstate == "42P01")
			errmsg ^= " File doesnt exist";
		else
			errmsg ^= var(PQerrorMessage(pgconn)) ^ " SQLERROR:" ^ sqlstate;

		this->setlasterror(errmsg);
		// // this->loglasterror();
		throw VarDBException(errmsg);
	}

	if (PQntuples(dbresult) < 1) {

		// Leave unassigned if not read
		//leave record (this) untouched if record cannot be read
		// *this = "";

		this->setlasterror("ERROR: mvdbpostgres read() record does not exist " ^
					key.quote());
		return false;
	}

	// A serious error
	if (PQntuples(dbresult) > 1) [[unlikely]] {
		var errmsg = "ERROR: mvdbpostgres read() SELECT returned more than one record";
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	*this = getpgresultcell(dbresult, 0, 0);

	return true;
}

var var::hash(const uint64_t modulus) const {

	THISIS("var var::hash() const")
	assertDefined(function_sig);
	assertString(function_sig);
	// ISNUMERIC(modulus)

	// not normalizing for speed and allow different non-normalised keys
	// std::string tempstr=this->normalize();

//
//	// uint64_t
//	// hash64=MurmurHash64((wchar_t*)fileandkey.data(),static_cast<int>(fileandkey.size()*sizeof(wchar_t)),0);
//
//#if defined(USE_WYHASH)
//	uint64_t hash64 = wyhash(var_str.data(), var_str.size(), 0, _wyp);
//#elif defined(USE_MURMURHASH)
//	uint64_t hash64 = MurmurHash64(var_str.data(), static_cast<int>(var_str.size()), 0);
//#else
//	uint64_t hash64 = std::hash<std::string>{}(var_str);
//#endif
//
	uint64_t hash64 = mvdbpostgres_hash_stdstr(this->var_str);

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
var var::lock(CVR key) const {

	// on postgres, repeated locks for the same thing (from the same connection) succeed and
	// stack up they need the same number of unlocks (from the same connection) before other
	// connections can take the lock unlock returns true if a lock (your lock) was released and
	// false if you dont have the lock NB return "" if ALREADY locked on this connection


	THISIS("var var::lock(CVR key) const")
	assertDefined(function_sig);
	ISSTRING(key)

	PGconn* pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::lock() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	auto dbconn = get_dbconn(*this);

	// TODO consider preventing begintrans if lock cache not empty
	auto hash64 = mvdbpostgres_hash_file_and_key(*this, key);

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
	int paramLengths[] = {sizeof(uint64_t)};
	int paramFormats[] = {1};  // binary

	// Locks outside transactions remain for the duration of the connection and can be unlocked/relocked or unlockall'd
	// Locks inside transactions are unlockable and automatically unlocked on commit/rollback
	const char* sql = (dbconn->in_transaction_) ? "SELECT PG_TRY_ADVISORY_XACT_LOCK($1)" : "SELECT PG_TRY_ADVISORY_LOCK($1)";

	// Debugging
	if (DBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ var(sql).replace("$1)", /*var(hash64) ^*/ ") file:" ^ *this ^ " key:" ^ key)).logputl("SQLL ");

	// Call postgres
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql, 1,                                      /* one param */
											nullptr,                                     /* let the backend deduce param type */
											paramValues, paramLengths, paramFormats, 1); /* ask for binary dbresults */

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_TUPLES_OK || PQntuples(dbresult) != 1) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg = "lock(" ^ *this ^ ", " ^ key ^ ")\n" ^
			var(PQerrorMessage(pgconn)) ^ "\nSQLERROR:" ^ sqlstate ^ " PQresultStatus=" ^
			var(PQresStatus(PQresultStatus(dbresult))) ^ ", PQntuples=" ^
			var(PQntuples(dbresult));
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// Add to lock cache if successful
	if (*PQgetvalue(dbresult, 0, 0) != 0) {
		std::pair<const uint64_t, int> lock(hash64, 0);
		dbconn->locks_.insert(lock);
		return 1;
	}

	// Otherwise indicate failure
	return 0;

}

bool var::unlock(CVR key) const {


	THISIS("void var::unlock(CVR key) const")
	assertDefined(function_sig);
	ISSTRING(key)

	auto hash64 = mvdbpostgres_hash_file_and_key(*this, key);

	auto pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::unlock() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	auto dbconn = get_dbconn(*this);

	// Unlock inside transaction has no effect
	if (dbconn->in_transaction_)
		return false;

	// If not in lock cache then return false
	if (!dbconn->locks_.contains(hash64))
		return false;

	// Remove from lock cache
	dbconn->locks_.erase(hash64);

	// Parameter array
	const char* paramValues[] = {reinterpret_cast<char*>(&hash64)};
	int paramLengths[] = {sizeof(uint64_t)};
	int paramFormats[] = {1};//binary

	// $1=hashed filename and key
	const char* sql = "SELECT PG_ADVISORY_UNLOCK($1)";

	if (DBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ var(sql).replace("$1)", /*var(hash64) ^*/ ") file:" ^ *this ^ " key:" ^ key)).logputl("SQLL ");

	// Call postgres
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql, 1,										/* one param */
											nullptr,									/* let the backend deduce param type */
											paramValues, paramLengths, paramFormats, 1); /* ask for binary results */

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_TUPLES_OK) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg = "unlock(" ^ this->convert(_FM, "^") ^ ", " ^ key ^ ")\n" ^
				var(PQerrorMessage(pgconn)) ^ "\nSQLERROR:" ^ sqlstate ^ " PQresultStatus=" ^
				var(PQresStatus(PQresultStatus(dbresult))) ^ ", PQntuples=" ^
				var(PQntuples(dbresult));
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// Should return true
	return PQntuples(dbresult) == 1;
}

bool var::unlockall() const {

	THISIS("void var::unlockall() const")
	assertDefined(function_sig);

	auto pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::unlockall() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this;
		this->setlasterror(errmsg);
		// this->loglasterror();
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

// returns only success or failure and any response is logged and saved for future lasterror() call
bool var::sqlexec(CVR sql) const {
	var response = -1;	//no response required
	if (!this->sqlexec(sql, response)) {
		this->setlasterror(response);
		return false;
	}
	return true;
}

// returns success or failure, and response = data or errmsg (response can be preset to max number of tuples)
bool var::sqlexec(CVR sqlcmd, VARREF response) const {

	THISIS("bool var::sqlexec(CVR sqlcmd, VARREF response) const")
	ISSTRING(sqlcmd)

	auto pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::sqlexec() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// log the sql command
	if (DBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ sqlcmd.convert("\t"," ").trim()).logputl("SQLE ");

	// will contain any dbresult IF successful

	// NB PQexec cannot be told to return binary results
	// but it can execute multiple commands
	// whereas PQexecParams is the opposite
	DBresult dbresult = PQexec(pgconn, sqlcmd.var_str.c_str());

	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK &&
		PQresultStatus(dbresult) != PGRES_TUPLES_OK) {
		//int xx = PQresultStatus(dbresult);
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		// sql state 42P03 = duplicate_cursor
		response = var(PQerrorMessage(pgconn)) ^ " sqlstate:" ^ sqlstate ^ " " ^ sqlcmd;
		return false;
	}

	//errmsg = var(PQntuples(dbresult));

	//quit if no rows/columns provided or no response required (integer<=0)
	int nrows = PQntuples(dbresult);
	int ncols = PQnfields(dbresult);
	if (nrows == 0 or ncols == 0 || (response.assigned() && ((response.var_typ & VARTYP_INT) && response <= 0))) {
		response = "";
		return true;
	}

	//option to limit number of rows returned
	if (response.assigned() && response.isnum() && response < nrows && response)
		nrows = response;

	response = "";

	//first row is the column/field names
	for (int coln = 0; coln < ncols; ++coln) {
		response.var_str.append(PQfname(dbresult, coln));
		response.var_str.push_back(FM_);
	}
	response.var_str.pop_back();

	//output the rows/columns as records separated by RM and fields separated by FM
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
//(why it is "writef" instead of "writef" isnt known!
bool var::writef(CVR filehandle, CVR key, const int fieldno) const {
	if (fieldno <= 0)
		return write(filehandle, key);

	THISIS("bool var::writef(CVR filehandle,CVR key,const int fieldno) const")
	// will be duplicated in read and write but do here to present the correct function name on
	// error
	assertString(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	// get the old record
	var record;
	if (!record.read(filehandle, key))
		record = "";

	// replace the field
	record(fieldno) = var_str;

	// write it back
	return record.write(filehandle, key);
}

/* "prepared statement" version doesnt seem to make much difference approx -10% - possibly because
two field file is so simple bool var::write(CVR filehandle,CVR key) const {}
*/

//"update if present or insert if not" is handled in postgres using ON CONFLICT clause
bool var::write(CVR filehandle, CVR key) const {

	THISIS("bool var::write(CVR filehandle, CVR key) const")
	assertString(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	// std::string key2=key.var_str;
	// std::string data2=var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

	// clear any cache
	filehandle.deletec(key2);

	// filehandle dos or DOS means osread/oswrite/osremove
	if (filehandle.var_str.size() == 3 && (filehandle.var_str == "dos" || filehandle.var_str == "DOS")) {
		//this->oswrite(key2); //.convert("\\",OSSLASH));
		//use osfilenames unnormalised so we can read and write as is
		return this->oswrite(key);
	}

	var sql;

	// Note cannot use postgres PREPARE/EXECUTE with parameterised filename
	// but performance gain is probably not great since the sql we use to read and write is
	// quite simple (could PREPARE once per file/table)

	sql = "INSERT INTO " ^ get_normal_filename(filehandle) ^ " (key,data) values( $1 , $2)";
	sql ^= " ON CONFLICT (key)";
	sql ^= " DO UPDATE SET data = $2";

	auto pgconn = get_pgconn(filehandle);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::write() get_pgconn() failed. ";
		errmsg ^= filehandle ^ ", " ^ key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// Parameter array
	const char* paramValues[] = {key2.data(), data2.data()};
	int paramLengths[] = {static_cast<int>(key2.size()), static_cast<int>(data2.size())};

	DEBUG_LOG_SQL1
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,       // two params (key and data)
											nullptr, // let the backend deduce param type
											paramValues, paramLengths,
											nullptr,       // text arguments
											0);      // text results

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg = "ERROR: mvdbpostgres write(" ^ filehandle.convert(_FM, "^") ^
					", " ^ key ^ ") failed: SQLERROR:" ^ sqlstate ^ " PQresultStatus=" ^
					var(PQresStatus(PQresultStatus(dbresult))) ^ " " ^
					var(PQerrorMessage(pgconn));
		// ERROR: mvdbpostgres write(definitions^1, LAST_SYNCDATE_TIME*DAT) failed: SQLERROR:25P02
		// PQresultStatus=PGRES_FATAL_ERROR ERROR:  current transaction is aborted, commands ignored until end of transaction block

		//errmsg.logputl();
		//errmsg.oswrite("/tmp/x");
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// success if inserted or updated 1 record
	//return strcmp(PQcmdTuples(dbresult), "1") != 0;

	return true;
}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var::updaterecord(CVR filehandle, CVR key) const {

	THISIS("bool var::updaterecord(CVR filehandle,CVR key) const")
	assertString(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	// clear any cache
	filehandle.deletec(key);

	// std::string key2=key.var_str;
	// std::string data2=var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

	// Parameter array
	const char* paramValues[] = {key2.data(), data2.data()};
	int paramLengths[] = {static_cast<int>(key2.size()), static_cast<int>(data2.size())};

	var sql = "UPDATE "  ^ get_normal_filename(filehandle) ^ " SET data = $2 WHERE key = $1";

	auto pgconn = get_pgconn(filehandle);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::updaterecord() get_pgconn() failed. ";
		errmsg ^= filehandle ^ ", " ^ key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	DEBUG_LOG_SQL1
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,        // two params (key and data)
											nullptr,  // let the backend deduce param type
											paramValues, paramLengths,
											nullptr,        // text arguments
											0);       // text results

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg = "ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM, "^") ^
				", " ^ key ^ ") SQLERROR: " ^ sqlstate ^ " Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
				var(PQerrorMessage(pgconn));
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(dbresult), "1") != 0) {
		var("ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM, "^") ^
			", " ^ key ^ ") Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
			var(PQerrorMessage(pgconn)))
			.errputl();
		return false;
	}

	return true;
}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt
// already exist" logic

bool var::insertrecord(CVR filehandle, CVR key) const {

	THISIS("bool var::insertrecord(CVR filehandle,CVR key) const")
	assertString(function_sig);
	ISSTRING(filehandle)
	ISSTRING(key)

	// clear any cache
	filehandle.deletec(key);

	// std::string key2=key.var_str;
	// std::string data2=var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

	// Parameter array
	const char* paramValues[] = {key2.data(), data2.data()};
	int paramLengths[] = {static_cast<int>(key2.size()), static_cast<int>(data2.size())};

	var sql =
		"INSERT INTO " ^ get_normal_filename(filehandle) ^ " (key,data) values( $1 , $2)";

	auto pgconn = get_pgconn(filehandle);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::insertrecord() get_pgconn() failed. ";
		errmsg ^= filehandle ^ ", " ^ key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	DEBUG_LOG_SQL1
	DBresult dbresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,       // two params (key and data)
											nullptr, // let the backend deduce param type
											paramValues, paramLengths,
											nullptr,       // text arguments
											0);      // text results

	// Handle serious errors or ordinary duplicate key failure (which will mess us transactionsa)
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) [[unlikely]] {

		// "duplicate key value violates unique constraint"
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		if (sqlstate == 23505)
			return false;

		var errmsg = "ERROR: mvdbpostgres insertrecord(" ^
				filehandle.convert(_FM, "^") ^ ", " ^ key ^ ") Failed: " ^
				var(PQntuples(dbresult)) ^ " SQLERROR:" ^ sqlstate ^ " " ^
				var(PQerrorMessage(pgconn));
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(dbresult), "1") != 0) {
		var("ERROR: mvdbpostgres insertrecord(" ^ filehandle.convert(_FM, "^") ^
			", " ^ key ^ ") Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
			var(PQerrorMessage(pgconn)))
			.errputl();
		return false;
	}

	return true;
}

bool var::deleterecord(CVR key) const {

	THISIS("bool var::deleterecord(CVR key) const")
	assertString(function_sig);
	ISSTRING(key)

	// clear any cache
	this->deletec(key);

	// std::string key2=key.var_str;
	std::string key2 = key.normalize().var_str;

	// filehandle dos or DOS means osread/oswrite/osremove
	if (var_str.size() == 3 && (var_str == "dos" || var_str == "DOS")) {
		//return this->osremove(key2);
		//use osfilenames unnormalised so we can read and write as is
		//return key.osremove();
		return (key.osfile() and key.osremove());
	}

	// Parameter array
	const char* paramValues[] = {key2.data()};
	int paramLengths[] = {static_cast<int>(key2.size())};

	var sql = "DELETE FROM " ^ get_normal_filename(*this) ^ " WHERE KEY = $1";

	auto pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::deleterecord() get_pgconn() failed. ";
		if (this->assigned())
			errmsg ^= *this ^ ", ";
		errmsg ^= key;
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	DEBUG_LOG_SQL1
	DBresult dbresult = PQexecParams(pgconn, sql.var_str.c_str(), 1, /* two param */
											nullptr,							/* let the backend deduce param type */
											paramValues, paramLengths,
											nullptr,  // text arguments
											0); // text results

	// Handle serious errors
	if (PQresultStatus(dbresult) != PGRES_COMMAND_OK) [[unlikely]] {
		var sqlstate = var(PQresultErrorField(dbresult, PG_DIAG_SQLSTATE));
		var errmsg = "ERROR: mvdbpostgres deleterecord(" ^ this->convert(_FM, "^") ^
				", " ^ key ^ ") SQLERROR: " ^ sqlstate ^ " Failed: " ^ var(PQntuples(dbresult)) ^ " " ^
				var(PQerrorMessage(pgconn));
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	// if not updated 1 then fail
	bool result;
	if (strcmp(PQcmdTuples(dbresult), "1") != 0) {
		if (DBTRACE)
			var("var::deleterecord(" ^ this->convert(_FM, "^") ^ ", " ^ key ^
				") failed. Record does not exist")
				.errputl();
		result = false;
	} else
		result = true;

	return result;
}

void var::cleardbcache() const {

	THISIS("bool var::cleardbcache() const")
	assertDefined(function_sig);

	int dbconn_no = get_dbconn_no_or_default(*this);
	if (!dbconn_no)
//		throw VarDBException("get_dbconn_no() failed in cleardbcache");
		return;

	thread_dbconnector.cleardbcache(dbconn_no);

	// Warning if any cursors have not been closed/cleaned up.
	for (auto& entry : thread_dbresults)
		var(entry.first).errputl("clearcache: WARNING: Cursor not cleaned up. Clearing it now.");

	// Clean up cursors - RAII/SBRM will call PQClear on the related PGresult* objects
	thread_dbresults.clear();

	return;
}

bool var::begintrans() const {

	THISIS("bool var::begintrans() const")
	assertDefined(function_sig);

	// Clear the record cache
	this->cleardbcache();

	// begin a transaction
	//if (!this->sqlexec("BEGIN"))
	// Read Committed is the default isolation level in PostgreSQL.
	// https://www.postgresql.org/docs/current/transaction-iso.html#XACT-READ-COMMITTED
	if (!this->sqlexec("BEGIN TRANSACTION ISOLATION LEVEL READ COMMITTED"))
		return false;

	auto dbconn = get_dbconn(*this);

	// Change status
	// ESSENTIAL Used to change locking type to PER TRANSACTION
	// so all locks persist until after commit i.e. cannot be specifically unlocked
	dbconn->in_transaction_ = true;

	return true;

}

bool var::rollbacktrans() const {

	THISIS("bool var::rollbacktrans() const")
	assertDefined(function_sig);

	// Clear the record cache
	this->cleardbcache();

	// Rollback a transaction
	if (!this->sqlexec("ROLLBACK"))
		return false;

	auto dbconn = get_dbconn(*this);

	// Change status
	dbconn->in_transaction_ = false;

    // Clear the lock cache
    dbconn->locks_.clear();

	return true;
}

bool var::committrans() const {

	THISIS("bool var::committrans() const")
	assertDefined(function_sig);

	// Clear the record cache
	this->cleardbcache();

	// end (commit) a transaction
	if (!this->sqlexec("END"))
		return false;

	auto dbconn = get_dbconn(*this);

	// Change status
	dbconn->in_transaction_ = false;

    // Clear the lock cache
    dbconn->locks_.clear();

	return true;

}

bool var::statustrans() const {

	THISIS("bool var::statustrans() const")
	assertDefined(function_sig);

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

bool var::dbcreate(CVR dbname) const {
	return this->dbcopy(var(""), dbname);
}

bool var::dbcopy(CVR from_dbname, CVR to_dbname) const {

	THISIS("bool var::dbcreate(CVR from_dbname, CVR to_dbname)")
	assertDefined(function_sig);
	ISSTRING(from_dbname)
	ISSTRING(to_dbname)

	// TODO Fail neatly if the source db doesnt exist or the target db already exists

	// Prepare the SQL
	var sql = "CREATE DATABASE " ^ to_dbname ^ " WITH";
	sql ^= " ENCODING='UTF8' ";

	// Optionally copy from an existing database
	if (from_dbname)
		sql ^= " TEMPLATE " ^ from_dbname;

	// Create the database
	if (!this->sqlexec(sql)) {
		//newdb.lasterror().errputl();
		return false;
	}

	// Connect to the new db
	var newdb;
	if (not newdb.connect(to_dbname)) {
		//newdb.lasterror().errputl();
		return false;
	}

	// Add dict schema to allow creation of dict files like dict.xxxxxxxx
	sql = "CREATE SCHEMA IF NOT EXISTS dict";
	//sql ^= "AUTHORIZATION exodus;"
	var result = true;
	if (!newdb.sqlexec(sql)) {
		//newdb.lasterror().errputl();
		result = false;
	}

	// Disconnect from the new database
	newdb.disconnect();

	return result;

}

bool var::dbdelete(CVR dbname) const {

	THISIS("bool var::dbdelete(CVR dbname)")
	assertDefined(function_sig);
	ISSTRING(dbname)

	// Fail neatly if the database does not exist.
	// SQL errors during a transaction cause the whole transaction to fail.
	if (!this->clone().dblist().lower().locate(dbname)) {
		setlasterror(dbname.quote() ^ " db does not exist.");
		return false;
	}

	return this->sqlexec("DROP DATABASE " ^ dbname);
}

bool var::createfile(CVR filename) const {

	THISIS("bool var::createfile(CVR filename)")
	assertDefined(function_sig);
	ISSTRING(filename)

	// var tablename = "TEMP" ^ var(100000000).rnd();
	// Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but
	// has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default
	// behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON
	// COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	// Fail neatly if the file already exists.
	// SQL errors during a transaction cause the whole transaction to fail.
	if (this->clone().open(filename)) {
		setlasterror(filename.quote() ^ " already exists.");
		return false;
	}

	var sql = "CREATE";
	if (filename.ends("_temp"))
		sql ^= " TEMPORARY ";
	sql ^= " TABLE " ^ get_normal_filename(filename);
	sql ^= " (key text primary key, data text)";

	if (this->assigned())
		return this->sqlexec(sql);
	else
		return filename.sqlexec(sql);
}

bool var::renamefile(CVR filename, CVR newfilename) const {

	THISIS("bool var::renamefile(CVR filename, CVR newfilename)")
	assertDefined(function_sig);
	ISSTRING(filename)
	ISSTRING(newfilename)

	// Fail neatly if the old file does not exist.
	// SQL errors during a transaction cause the whole transaction to fail.
	// Make sure we use the right connection
	var filehandle = this->clone();
	if (!filehandle.open(filename)) {
		setlasterror(filename.quote() ^ " cannot be renamed because it does not exist.");
		return false;
	}

	// Fail neatly if the new file exists
	// SQL errors during a transaction cause the whole transaction to fail.
	if (filehandle.open(newfilename)) {
		setlasterror(filename.quote() ^ " cannot be renamed because " ^ newfilename.quote() ^ " already exists.");
		return false;
	}

	// Remove from the cache of file handles
	filehandle.detach(filename);

	var sql = "ALTER TABLE " ^ filename ^ " RENAME TO " ^ newfilename;

//	if (this->assigned())
//		return this->sqlexec(sql);
//	else
//		return filename.sqlexec(sql);
	if (!filehandle.sqlexec(sql))
		[[unlikely]]
		throw VarDBException(this->lasterror());

	return true;
}

bool var::deletefile(CVR filename) const {

	THISIS("bool var::deletefile(CVR filename)")
	assertDefined(function_sig);
	ISSTRING(filename)

	// Fail neatly if the file does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
	// Delete the file on whatever connection it exists;
	var filehandle = this->clone();
	if (!filehandle.open(filename)) {
		setlasterror(filename.quote() ^ " cannot be deleted because it does not exist.");
		return false;
	}

	// Remove from filehandle cache regardless of success or failure to deletefile
	// Delete from cache AFTER the above open which will place it in the cache
	// Similar code in detach and deletefile
	if (thread_file_handles.erase(get_normal_filename(filename))) {
		//filename.errputl("::deletefile ==== Connection cache DELETED = ");
	} else {
		//filename.errputl("::deletefile ==== Connection cache NONE    = ");
	}

	var sql = "DROP TABLE " ^ filename.f(1) ^ " CASCADE";
	//var sql = "DROP TABLE IF EXISTS " ^ filename.f(1) ^ " CASCADE";

//	if (this->assigned())
//		return this->sqlexec(sql);
//	else
//		return filename.sqlexec(sql);
	if (!filehandle.sqlexec(sql))
		[[unlikely]]
		throw VarDBException(this->lasterror());

	return true;
}

bool var::clearfile(CVR filename) const {

	THISIS("bool var::clearfile(CVR filename)")
	assertDefined(function_sig);
	ISSTRING(filename)

	// Fail neatly if the file does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
	var filehandle = this->clone();
	if (!filehandle.open(filename, *this)) {
		setlasterror(filename.quote() ^ " cannot be deleted because it does not exist.");
		return false;
	}

	var sql = "DELETE FROM " ^ filename.f(1);
//	if (this->assigned())
//		return this->sqlexec(sql);
//	else
//		return filename.sqlexec(sql);
	if (!filehandle.sqlexec(sql))
		[[unlikely]]
		throw VarDBException(this->lasterror());

	return true;
}

static var get_dictexpression(CVR cursor, CVR mainfilename, CVR filename, CVR dictfilename, CVR dictfile, CVR fieldname0, VARREF joins, VARREF unnests, VARREF selects, bool& ismv, bool& isdatetime, bool forsort) {

	//cursor is required to join any calculated fields in any second pass

	ismv = false;

	var fieldname = fieldname0.convert(".", "_");
	var actualdictfile = dictfile;

	// Open dict.xxxx or dict.voc on the default dict connection or throw an error
	if (!actualdictfile) {

		// The dictionary of all dict. files is dict.voc. Used when selecting any dict. file.
		var dictfilename;
		if (mainfilename.lcase().starts("dict."))
			dictfilename = "dict.voc";
		else
			dictfilename = "dict." ^ mainfilename;

		// If dict .mainfile is not available, use dict.voc
		if (!actualdictfile.open(dictfilename)) {
			dictfilename = "dict.voc";
			if (!actualdictfile.open(dictfilename)) [[unlikely]] {
				var errmsg = "get_dictexpression() cannot open " ^ dictfilename.quote();
				//this->setlasterror(errmsg);
				//this->loglasterror();
				throw VarDBException(errmsg);
			}
		}
	}

	//if doing 2nd pass then calculated fields have been placed in a parallel temporary file
	//and their column names appended with a colon (:)
	var stage2_calculated = fieldname.ends(":");
	var stage2_filename = "SELECT_CURSOR_STAGE2_" ^ cursor.f(1);

	if (stage2_calculated) {
		fieldname.popper();
		// Create a pseudo look up to SELECT_CURSOR_STAGE2 temporary file created by stage 1/2 select
		// which has the fields stored in sql columns and not in the usual data column
		stage2_calculated = "/" "/@ANS=XLATE(\"" ^ stage2_filename ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
		stage2_calculated.logputl("stage2_calculated simulation --------------------->");
	}

	// given a file and dictionary id
	// returns a postgres sql expression like (exodus.extract_text(filename.data,99,0,0))
	// using one of the exodus backend functions installed in postgres like exodus.extract_text,
	// exodus.extract_date etc.
	var dictrec;
	if (!dictrec.read(actualdictfile, fieldname)) {
		// try lowercase
		fieldname.lcaser();
		if (!dictrec.read(actualdictfile, fieldname)) {
			// try uppercase
			fieldname.ucaser();
			if (!dictrec.read(actualdictfile, fieldname)) {
				// try in voc lowercase
				fieldname.lcaser();
				if (dictrec.read("dict.voc", fieldname)) {
					actualdictfile = "dict.voc";
				} else {
					// try in voc uppercase
					fieldname.ucaser();
					if (dictrec.read("dict.voc", fieldname)) {
						actualdictfile = "dict.voc";
					} else {
						if (fieldname == "@ID" || fieldname == "ID")
							dictrec = "F" ^ FM ^ "0" ^ FM ^ "Ref" ^ FM ^
								FM ^ FM ^ FM ^ FM ^ FM ^ "" ^ FM ^
								15;
						else [[unlikely]] {
							var errmsg = "get_dictexpression() cannot read " ^
								fieldname.quote() ^ " from " ^
								actualdictfile.convert(FM, "^")
									.quote() ^
								" or \"dict.voc\"";
							throw VarDBException(errmsg);
							//					exodus::errputl("ERROR:
							// mvdbpostgres get_dictexpression() cannot
							// read " ^ fieldname.quote() ^ " from " ^
							// actualdictfile.quote());
							//return "";
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

	var dicttype = dictrec.f(1);
	var fieldno = dictrec.f(2);
	var conversion = dictrec.f(7);

	bool isinteger = conversion == "[NUMBER,0]" || dictrec.f(11) == "0N" ||
					dictrec.f(11).starts("0N_");
	bool isdecimal = conversion.starts("MD") || conversion.starts("[NUMBER") ||
					dictrec.f(12) == "FLOAT" || dictrec.f(11).contains("0N");
	//dont assume things that are R are numeric
	//eg period 1/19 is right justified but not numeric and sql select will crash if ::float8 is used
	//||dictrec.f(9) == "R";
	bool isnumeric = isinteger || isdecimal || dictrec.f(9) == "R";
	bool ismv1 = dictrec.f(4).starts("M");
	var fromjoin = false;

	bool isdate = conversion.starts("D") || conversion.starts("[DATE");
	bool istime = !isdate && (conversion.starts("MT") || conversion.starts("[TIME"));
	isdatetime = isdate || istime;

	var sqlexpression;
	if (dicttype == "F") {

		// Field 0 means key field
		if (!fieldno) {

			if (forsort && !isdate && !istime)
				// sqlexpression="exodus.extract_sort(" ^
				// get_fileexpression(mainfilename, filename,"key") ^ ")";
				sqlexpression =
					"exodus.extract_sort(" ^ mainfilename ^ ".key,0,0,0)";
			else
				// sqlexpression="convert_from(" ^ get_fileexpression(mainfilename,
				// filename, "key") ^ ", 'UTF8')";
				sqlexpression = get_fileexpression(mainfilename, filename, "key");

			// Multipart key - extract relevent field based on "*" separator
			var keypartn = dictrec.f(5);
			if (keypartn) {
				sqlexpression =
					"split_part(" ^ sqlexpression ^ ", '*', " ^ keypartn ^ ")";
			}

			// example of multipart key and date conversion
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

			if (DBTRACE) {
				TRACE(fieldno)
				TRACE(sqlexpression)
			}

			return sqlexpression;

		} // of key field, Fieldno = 0

		var extractargs =
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
//		bool is_ans_xlate = (!ismv1 || stage2_calculated) && function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate(");
//TRACE(is_ans_xlate);// 0 ??
//		is_ans_xlate = ((!ismv1 || stage2_calculated) && function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate("));
//TRACE(is_ans_xlate);// 0 ??
		bool x = (!ismv1 || stage2_calculated);
		bool y = function_src.trimfirst("\t /").convert(" ","").lcase().starts("@ans=xlate(");
		bool is_ans_xlate = x and y;
		if (DBTRACE) {
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
			sqlexpression = get_normal_filename(actualdictfile).replace("dict.", "dict_") ^ "_" ^ fieldname ^ "(";

			// function arguments are (key,data) by default

			// Extract pgsql_line1
			var delim;
			var pgsql_line1 = function_src.substr2(pgsql_pos, delim);

//			var keydictid = pgsql_line1.field(" ", 2);
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
		else if (stage2_calculated && ismv1) {
			//TRACE("has stage2_calculated && ismv1")
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
		//else if ((!ismv1 || stage2_calculated) && function_src.trimfirst("\t /").lcase().starts("/" "/@ans=xlate(")) {
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
			var xlatetargetfieldname = function_src.field(",", 3).trim().unquoter();

			// arg4 = mode X or C
			var xlatemode = function_src.field(",", 4).trim().convert("'\" )", "");

			// Error if fourth field is not "X", "C" or 'C'
			if (xlatemode != "X" && xlatemode != "C") {
				// not xlate X or C
				goto exodus_call;
//				TRACE("goto exodus_call")
			}
			if (DBTRACE) {
				TRACE(function_src)
				TRACE(xlatetargetfilename)
				TRACE(xlatefromfieldname)
				TRACE(xlatetargetfieldname)
				TRACE(xlatemode)
			}

//			TRACE("doing a join")
			// Assume we have a good simple xlate function_src and can convert to a JOIN
			// Determine the expression in the xlate target file
			// VARREF todictexpression=sqlexpression;
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
				// if (!dictxlatetofile.open("DICT",xlatetargetfilename))
				//	throw VarDBException("get_dictexpression() DICT" ^
				// xlatetargetfilename ^ " file cannot be opened"); var
				// ismv;
				var xlatetargetdictfilename = "dict." ^ xlatetargetfilename;
				var xlatetargetdictfile;
				if (!xlatetargetdictfile.open(xlatetargetdictfilename)) [[unlikely]] {
					var errmsg = xlatetargetdictfilename ^ " cannot be opened for " ^ function_src;
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
				xlatefromfieldname.cutter(1).popper();
				bool isdatetime;
				xlatekeyexpression = get_dictexpression(cursor,
					filename, filename, dictfilename, dictfile,
					xlatefromfieldname, joins, unnests, selects, ismv, isdatetime, forsort);

			} else if (xlatefromfieldname.lcase() == "@id") {
				xlatekeyexpression = filename ^ ".key";

			} else [[unlikely]] {
				// throw  VarDBException("get_dictexpression() " ^
				// filename.quote() ^ " " ^ fieldname.quote() ^ " - INVALID
				// DICTIONARY EXPRESSION - " ^ dictrec.f(8).quote());
				var("ERROR: mvdbpostgres get_dictexpression() " ^
					filename.quote() ^ " " ^ fieldname.quote() ^
					" - INVALID DICTIONARY EXPRESSION - " ^
					dictrec.f(8).quote())
					.errputl();
				return "";
			}

			//if the xlate key expression is stage2_calculated then
			//indicate that the whole dictid expression is stage2_calculated
			//and do not do any join
			if (xlatekeyexpression.contains("exodus_call")) {
				sqlexpression = "exodus_call(";
				if (DBTRACE)
					TRACE(sqlexpression)

				return sqlexpression;
			}

			fromjoin = true;

			// joins needs to follow "FROM mainfilename" clause
			// except for joins based on mv fields which need to follow the
			// unnest function
			var joinsectionn = ismv ? 2 : 1;

			// add the join
			///similar code above/below
			//main file is on the left
			//secondary file is on the right
			//normally we want all records on the left (main file) and any secondary file records that exist ... LEFT JOIN
			//if joining to stage2_calculated field file then we want only records that exist in the stage2_calculated fields file ... RIGHT JOIN (could be INNER JOIN)
			//RIGHT JOIN MUST BE IDENTICAL ELSE WHERE TO PREVENT DUPLICATION
			var join_part1 = stage2_calculated ? "RIGHT" : "LEFT";
			join_part1 ^= " JOIN " ^ xlatetargetfilename ^ " ON ";

			var join_part2 =
				xlatetargetfilename ^ ".key = " ^ xlatekeyexpression;
			// only allow one join per file for now.
			// TODO allow multiple joins to the same file via different keys
			if (!joins.f(joinsectionn).contains(join_part1))
				//joins.r(joinsectionn, -1, join_part1 ^ join_part2);
				joins(joinsectionn, -1) = join_part1 ^ join_part2;

			if (DBTRACE)
				TRACE(sqlexpression)

			return sqlexpression;

		}

		else {
exodus_call:
			// FOLLOWING IS CURRENTLY DISABLED since postgres has no way to call exodus
			// if we get here then we were unable to work out any sql expression or function
			// so originally we instructed postgres to CALL EXODUS VIA IPC to run exodus
			// subroutines in the context of the calling program. exodus mvdbpostgres.cpp setup
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

			if (DBTRACE)
				TRACE(sqlexpression)

			return sqlexpression;
		}
	} // of dict type S

	else [[unlikely]] {
		// throw  filename ^ " " ^ fieldname ^ " - INVALID DICTIONARY ITEM";
		// throw  VarDBException("get_dictexpression(" ^ filename.quote() ^ ", " ^
		// fieldname.quote() ^ ") invalid dictionary type " ^ dicttype.quote());
		var("ERROR: mvdbpostgres get_dictexpression(" ^ filename.quote() ^ ", " ^
			fieldname.quote() ^ ") invalid dictionary type " ^
			dicttype.quote())
			.errputl();
		return "";
	}

	// Multivalued or xref fields need special handling
	///////////////////////////////////////////////////

	ismv = ismv1;

	// vector (for GIN or indexing/filtering multivalue fields)
	//if ((ismv1 and !forsort) || fieldname.ucase().ends("_XREF")) {
	if ((ismv1 and !forsort) || fieldname.ucase().ends("XREF")) {
		//this is the sole creation of to_tsvector in mvdbpostgres.cpp
		//it will be used like to_tsvector(...) @@ to_tsquery(...)
		if (fieldname.ucase().ends("XREF"))
			sqlexpression = "immutable_unaccent(" ^ sqlexpression ^ ")";
		sqlexpression = "to_tsvector('simple', " ^ sqlexpression ^ ")";
		//sqlexpression = "to_tsvector('simple', " ^ sqlexpression ^ ")";
		//sqlexpression = "to_tsvector('english'," ^ sqlexpression ^ ")";
		//sqlexpression = "string_to_array(" ^ sqlexpression ^ ",chr(29),'')";

		//multivalued prestage2_calculated field DUPLICATE CODE
		if (fieldname0.ends(":")) {
			var joinsectionn = 1;
			var join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
			//if (!joins.f(joinsectionn).contains(join))
			if (!joins.contains(join))
				//joins.r(joinsectionn, -1, join);
				joins(joinsectionn, -1) = join;
		}

	}

	// unnest multivalued fields into multiple output rows
	else if (ismv1) {

		//ismv = true;

		// var from="string_to_array(" ^ sqlexpression ^ ",'" ^ VM ^ "'";
		// exodus_extract_date_array, exodus_extract_time_array
		if (sqlexpression.starts("exodus.extract_date(") || sqlexpression.starts("exodus.extract_time("))
			sqlexpression.paster(20, "_array");
		else {
			sqlexpression.regex_replacer("exodus.extract_sort\\(", "exodus.extract_text\\(");
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
				if (!selects.f(1).contains(sqlexpression))
					selects ^= ", " ^ sqlexpression;
			} else {

				//multivalued prestage2_calculated field DUPLICATE CODE
				if (fieldname0.ends(":")) {
					var joinsectionn = 1;
					var join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
					//if (!joins.f(joinsectionn).contains(join))
					if (!joins.contains(join))
						//joins.r(joinsectionn, -1, join);
						joins(joinsectionn, -1) = join;
				}

				// insert with SMs since expression can contain VMs
				if (!unnests.f(2).locate(fieldname)) {
					//unnests.r(2, -1, fieldname);
					//unnests.r(3, -1, sqlexpression);
					unnests(2, -1) = fieldname;
					unnests(3, -1) = sqlexpression;
				}
			}

			sqlexpression = fieldname;
		}
	}

	if (DBTRACE)
		TRACE(sqlexpression)

	return sqlexpression;
}

// var getword(VARREF remainingwords, CVR joinvalues=false)
static var getword(VARREF remainingwords, VARREF ucword) {

	// gets the next word
	// or a series of words separated by FM while they are numbers or quoted strings)
	// converts to sql quoted strings
	// and clips them from the input string

	bool joinvalues = true;

	var word1 = remainingwords.field(" ", 1);
	remainingwords = remainingwords.field(" ", 2, 99999);

	//separate out leading or trailing parens () but not both
	if (word1.len() > 1) {
		if (word1.starts("(") && word1.last() != ")") {
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

	// join words within quote marks into one quoted phrase
	var char1 = word1.first();
	if ((char1 == DQ || char1 == SQ)) {
		while (not word1.ends(char1) || word1.len() <= 1) {
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

	// grab multiple values (numbers or quoted words) into one list, separated by FM
	//value chars are " ' 0-9 . + -
	if (remainingwords && joinvalues && valuechars.contains(word1.first())) {
		word1 = SQ ^ word1.unquote().replace("'", "''") ^ SQ;

		var nextword = remainingwords.field(" ", 1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		// duplicated above/below
		if (nextword == "and") {
			var nextword2 = remainingwords;
			if (valuechars.contains(nextword2.first())) {
				nextword = nextword2;
				remainingwords = remainingwords.field(" ", 2, 99999);
			}
		}

		/*
		while (nextword && valuechars.contains(nextword[1])) {
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
				var nextword2 = remainingwords;
				if (valuechars.contains(nextword2[1])) {
					nextword = nextword2;
					remainingwords = remainingwords.field(" ", 2, 99999);
				}
			}
		}
		*/
		nextword = getword(remainingwords, ucword);
		if (nextword && valuechars.contains(nextword.first())) {
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
	if (DBTRACE) {
		TRACE(word1)
		TRACE(remainingwords)
	}
	return word1;
}

//bool var::saveselect(CVR filename) {
//
//	THISIS("bool var::saveselect(CVR filename) const")
//	//?allow undefined usage like var xyz=xyz.select();
//	assertDefined(function_sig);
//	ISSTRING(filename)
//
//	if (DBTRACE)
//		filename.logputl("DBTR var::saveselect() ");
//
//	int recn = 0;
//	var key;
//	var mv;
//
//	// save preselected keys into a file to be used with INNERJOIN on select()
//
//	// this should not throw if the select does not exist
//	this->deletefile(filename);
//
//	// clear or create any existing saveselect file with the same name
//	if (!this->createfile(filename))
//		throw VarDBException("saveselect cannot create file " ^ filename);
//
//	var file;
//	if (!file.open(filename, (*this)))
//		throw VarDBException("saveselect cannot open file " ^ filename);
//
//	while (this->readnext(key, mv)) {
//		recn++;
//
//		// save a key
//		(mv ^ FM ^ recn).write(file, key);
//	}
//
//	return recn > 0;
//}

bool var::select(CVR sortselectclause) {

	THISIS("bool var::select(CVR sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	assertDefined(function_sig);
	ISSTRING(sortselectclause)
	auto started = var().ostime();
	bool result;
	if (!sortselectclause || sortselectclause.ends("R)"))
		result = this->selectx("key, mv::integer, data", sortselectclause);
	else
		result = this->selectx("key, mv::integer", sortselectclause);
	auto elapsed = var().ostime() - started;
	if (elapsed > 1)
		elapsed.errputl(" select: secs:");
	return result;
}

// currently only called from select, selectrecord and getlist
// TODO merge into plain select()?
bool var::selectx(CVR fieldnames, CVR sortselectclause) {
	// private - and arguments are left unchecked for speed
	//?allow undefined usage like var xyz=xyz.select();
	if (var_typ & VARTYP_MASK) {
		// throw VarUndefined("selectx()");
		var_str.clear();
		var_typ = VARTYP_STR;
	}

	// fieldnames.logputl("fieldnames=");
	// sortselectclause.logputl("sortselectclause=");

	// default to ""
	if (!(var_typ & VARTYP_STR)) {
		if (!var_typ) {
			var_str.clear();
			var_typ = VARTYP_STR;
		} else
			this->createString();
	}

	if (DBTRACE)
		TRACE(sortselectclause)

	var actualfilename = get_normal_filename(*this);
	// actualfilename.logputl("actualfilename=");
	var dictfilename = actualfilename;
	var actualfieldnames = fieldnames;
	var dictfile = "";
	var keycodes = "";
	bool bykey = false;
	var wordn;
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

	//prepare to save calculated fields that cannot be calculated by postgresql for secondary processing
	var calc_fields = "";
	//var ncalc_fields=0;
	this->r(10, "");

	//catch bad FM character
	if (sortselectclause.var_str.find(FM_) != std::string::npos) [[unlikely]] {
		var errmsg = "Illegal FM character in " ^ sortselectclause;
		throw VarDBException(errmsg);
	}

	// sortselect clause can be a filehandle in which case we extract the filename from field1
	// omitted if filename.select() or filehandle.select()
	// cursor.select(...) where ...
	// SELECT (or SSELECT) nnn filename with .... and with ... by ... by
	// filename can be omitted if calling like filename.select(...) or filehandle.select(...)
	// nnn is optional limit to number of records returned
	// TODO only convert \t\r\n outside single and double quotes
	//var remaining = sortselectclause.f(1).convert("\t\r\n", "   ").trim();
	var remaining = sortselectclause.convert("\t\r\n", "   ").trim();

	// remaining.logputl("remaining=");

	// remove trailing options eg (S) or {S}
	var lastword = remaining.field2(" ", -1);
	if ((lastword.starts("(") && lastword.ends(")")) ||
		(lastword.starts("{") && lastword.ends("}"))) {
		remaining.cutter(-lastword.len() - 1);
	}

	var firstucword = remaining.field(" ", 1).ucase();

	// sortselectclause may start with {SELECT|SSELECT {maxnrecs} filename}
	if (firstucword == "SELECT" || firstucword == "SSELECT") {
		if (firstucword == "SSELECT")
			bykey = true;

		// remove it
		var xx = getword(remaining, xx);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// the second word can be a number to limit the number of records selected
	if (firstucword.len() and firstucword.isnum()) {
		maxnrecs = firstucword;

		// remove it
		var xx = getword(remaining, xx);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// the next word can be the filename if not one of the select clause words
	// override any filename in the cursor variable
	if (firstucword && not var("BY BY-DSND WITH WITHOUT ( { USING DISTINCT").locateusing(" ", firstucword)) {
		actualfilename = firstucword;
		dictfilename = actualfilename;
		// remove it
		var xx = getword(remaining, firstucword);
	}

	// actualfilename.logputl("actualfilename=");
	if (!actualfilename) [[unlikely]] {
		// this->outputl("this=");
		var errmsg = "filename missing from select statement:" ^ sortselectclause;
		throw VarDBException(errmsg);
	}

	while (remaining.len()) {

		// remaining.logputl("remaining=");
		// whereclause.logputl("whereclause=");
		// orderclause.logputl("orderclause=");

		var ucword;
		var word1 = getword(remaining, ucword);

		// skip options (last word and surrounded by brackets)
		// (S) etc
		// options - last word enclosed in () or {}
		if (!remaining.len() &&
			((word1.starts("(") && word1.ends(")")) ||
			(word1.starts("{") && word1.ends("}")))) {
			// word1.logputl("skipping last word in () options ");
			continue;
		}

		// 1. numbers or strings without leading clauses like with ... mean record keys
		// 2. value chars are " ' 0-9 . + -
		// 3. values are ignored after any with/by statements to skip the following
		//    e.g. JUSTLEN "T#20" or HEADING "..."
		else if (valuechars.contains(word1.first())) {
			if (!whereclause && !orderclause) {
				if (keycodes)
					keycodes ^= FM;
				keycodes ^= word1;
			}
			if (DBTRACE_SELECT)
				TRACE(keycodes)
			continue;
		}

		// using filename
		else if (ucword == "USING" && remaining) {
			dictfilename = getword(remaining, xx);
			if (!dictfile.open("dict." ^ dictfilename)) [[unlikely]] {
				var errmsg = "select() dict_" ^ dictfilename ^ " file cannot be opened";
				throw VarDBException(errmsg);
			}
			if (DBTRACE_SELECT)
				TRACE(dictfilename)
			continue;
		}

		// distinct fieldname (returns a field instead of the key)
		else if (ucword == "DISTINCT" && remaining) {

			var distinctfieldname = getword(remaining, xx);
			var distinctexpression = get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, distinctfieldname, joins, unnests, selects, ismv, isdatetime, false);
			var naturalsort_distinctexpression = get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, distinctfieldname, joins, unnests, selects, ismv, isdatetime, true);

			if ((true)) {
				// this produces the right values but in random order
				// it use any index on the distinct field so it works on large
				// indexed files select distinct is really only useful on INDEXED
				// fields unless the file is small
				distinctfieldnames = "DISTINCT " ^ (unnests ? distinctfieldname : distinctexpression);
			} else {
				// this produces the right results in the right order
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

		// by or by-dsnd
		else if (ucword == "BY" || ucword == "BY-DSND") {
			// next word must be dictid
			var dictid = getword(remaining, xx);
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename, dictfile, dictid, joins, unnests, selects, ismv, isdatetime, true);

			// dictexpression.logputl("dictexpression=");
			// orderclause.logputl("orderclause=");

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.contains("exodus_call"))
			//if (dictexpression == "true")
			{
				if (!calc_fields.f(1).locate(dictid)) {
					//++ncalc_fields;
					//calc_fields.r(1, -1, dictid);
					calc_fields(1, -1) = dictid;
				}
				continue;
			}

			//use postgres collation instead of exodus.extract_sort
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

		// subexpression combination
		else if (ucword == "AND" || ucword == "OR") {
			// dont start with AND or OR
			if (whereclause)
				whereclause ^= "\n " ^ ucword;
			if (ucword == "OR") {
				orwith = true;
			}
			if (DBTRACE_SELECT)
				TRACE(whereclause)
			continue;
		}

		// subexpression grouping
		else if (ucword == "(" || ucword == ")") {
			//default to or between WITH clauses
			if (whereclause.last() == ")" and ucword == "(")
				whereclause ^= "\nor";
			whereclause ^= "\n " ^ ucword;
			if (DBTRACE_SELECT)
				TRACE(whereclause)
			continue;
		}

		// with dictid eq/starting/ending/containing/like 1 2 3
		// with dictid 1 2 3
		// with dictid between x and y
		else if (ucword == "WITH" || ucword == "WITHOUT") {

			/////////////////////////////////////////////////////////
			// Filter Stage 1 - Decide if positive or negative filter
			/////////////////////////////////////////////////////////

			var negative = ucword == "WITHOUT";

			// next word must be the NOT/NO or the dictionary id
			word1 = getword(remaining, ucword);

			// can negate before (and after) dictionary word
			// eg WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" || ucword == "NO") {
				negative = !negative;
				// word1=getword(remaining,true);
				// remove NOT or NO
				word1 = getword(remaining, ucword);
			}

			//////////////////////////////////////////////////////////
			// Filter Stage 2 - Acquire column function to be filtered
			//////////////////////////////////////////////////////////

			// skip AUTHORISED for now since too complicated to calculate in database
			// ATM if (word1.ucase()=="AUTHORISED") { if
			//(whereclause.ends(" AND"))
			//whereclause.paster(-4,4,""); continue;
			//}

			// process the dictionary id
			var forsort =
				false;	// because indexes are NOT created sortable (exodus_sort()
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename,
							dictfile, word1, joins, unnests, selects, ismv, isdatetime, forsort);
			//var usingnaturalorder = dictexpression.contains("exodus.extract_sort") or dictexpression.contains("exodus_natural");
			var dictid = word1;

			//var dictexpression_isarray=dictexpression.contains("string_to_array(");
			var dictexpression_isarray = dictexpression.contains("_array(");
			var dictexpression_isvector = dictexpression.contains("to_tsvector(");
			//var dictexpression_isfulltext = dictid.ucase(),ends("_XREF");
			var dictexpression_isfulltext = dictid.ucase().ends("XREF");

			// add the dictid expression
			//if (dictexpression.contains("exodus_call"))
			//	dictexpression = "true";

			//whereclause ^= " " ^ dictexpression;

			// the words after the dictid can be NOT/NO or values
			// word1=getword(remaining, true);
			word1 = getword(remaining, ucword);

			///////////////////////////////////////////////////////////////////////
			// Filter Stage 3 - 2nd chance to decide if positive or negative filter
			///////////////////////////////////////////////////////////////////////

			// can negate before (and after) dictionary word
			// eg WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" || ucword == "NO") {
				negative = !negative;
				// word1=getword(remaining,true);
				// remove NOT/NO and acquire any values
				word1 = getword(remaining, ucword);
			}

			/////////////////////////////////////////////////
			// Filter Stage 4 - SIMPLE BETWEEN or FROM clause
			/////////////////////////////////////////////////

			// BETWEEN x AND y
			// FROM x TO y

			if (ucword == "BETWEEN" || ucword == "FROM") {

				//prevent BETWEEN being used on fields
				if (dictexpression_isvector) [[unlikely]] {
					var errmsg = sortselectclause ^ " 'BETWEEN x AND y' and 'FROM x TO y' ... are not currently supported for mv or xref columns";
					throw VarDBException(errmsg);
				}

				// get and append first value
				word1 = getword(remaining, ucword);

				// get and append second value
				var word2 = getword(remaining, xx);

				// discard any optional intermediate "AND"
				if (word2.ucase() == "AND" || word2.ucase() == "TO") {
					word2 = getword(remaining, xx);
				}

				// check we have two values (in word1 and word2)
				if (!valuechars.contains(word1.first()) || !valuechars.contains(word2.first())) [[unlikely]] {
					var errmsg = sortselectclause ^ "BETWEEN x AND y/FROM x TO y must be followed by two values (x AND/TO y)";
					throw VarDBException(errmsg);
				}

				// Replaced by COLLATE
				//if (usingnaturalorder) {
				//	word1 = naturalorder(word1.var_str);
				//	word2 = naturalorder(word2.var_str);
				//}

				// no filtering in database on calculated items
				//save then for secondary filtering
				if (dictexpression.contains("exodus_call")) {
					var opid = negative ? ">!<" : "><";

					//almost identical code for exodus_call above/below
					var calc_fieldn;
					if (!calc_fields.locate(dictid, calc_fieldn, 1)) {
						//++ncalc_fields;
						//calc_fields.r(1, calc_fieldn, dictid);
						calc_fields(1, calc_fieldn) = dictid;
					}

					//prevent WITH XXX appearing twice in the same sort/select clause
					//unless and until implemented
					if (calc_fields.f(2, calc_fieldn)) [[unlikely]] {
						var errmsg = "WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote();
						throw VarDBException(errmsg);
					}

					//calc_fields.r(2, calc_fieldn, opid);
					//calc_fields.r(3, calc_fieldn, word1.lowerer());
					//calc_fields.r(4, calc_fieldn, word2);
					calc_fields(2, calc_fieldn) = opid;
					calc_fields(3, calc_fieldn) = word1.lowerer();
					calc_fields(4, calc_fieldn) = word2;

					// Place holder for stage 1 of stage2 select
					whereclause ^= " true";
					continue;
				}
				//select numrange(100,150,'[]')  @> any(string_to_array('1,2,150',',','')::numeric[]);
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

			//using regular expression logic for CONTAINING/STARTING/ENDING
			//will be converted to tsvector logic if dictexpression_isvector
			if (ucword == "CONTAINING" or ucword == "[]") {
				prefix = ".*";
				postfix = ".*";
				op = "=";
				word1 = getword(remaining, ucword);
			} else if (ucword == "STARTING" or ucword == "]") {

				//identical code above/below
				if (dictexpression_isvector) {
					prefix = "^";
					postfix = ".*";
				}
				//NOT using regular expression logic for single valued fields and STARTING
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

			//"normal" comparative filtering
			// 1) Acquire operator - or empty if not present

			// convert PICK/AREV relational operators to standard SQL relational operators
			// IS/ISNT/NOT -> EQ/NE/NE
			var aliasno;
			if (var("IS EQ NE NOT ISNT GT LT GE LE").locateusing(" ", ucword, aliasno)) {
				word1 = var("= = <> <> <> > < >= <=").field(" ", aliasno);
				ucword = word1;
			}

			// capture operator is any
			if (var("= <> > < >= <= ~ ~* !~ !~*").locateusing(" ", ucword)) {
				// is an operator
				op = ucword;
				// get another word (or words)
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

					//CONTAINING
					if (word1.at(-2) == "]") {
						word1.paster(-2, 1, "");
						postfix = ".*";
					}
					//ENDING
					else {
						postfix = "$";
					}

					//STARTING
				} else if (word1.at(-2) == "]") {
					word1.paster(-2, 1, "");

					//identical code above/below
					if (dictexpression_isvector) {
						prefix = "^";
						postfix = ".*";
					}
					//NOT using regular expression logic for single valued fields and STARTING
					//this should trigger 'COLLATE "C" BETWEEN x AND y" below to ensure postgres indexes are used
					else {
						if (op == "<>")
							negative = !negative;
						op = "]";
					}
				}
				ucword = word1.ucase();
			}

			//select WITH ..._XREF uses postgres full text searching
			//which has its own prefix and postfix rules. see below
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

			else if (!dictexpression_isvector && (prefix || postfix)) {

				//postgres match matches anything in the string unless ^ and/or $ are present
				// so .* is not necessary in prefix and postfix
				if (prefix == ".*")
					prefix = "";
				if (postfix == ".*")
					postfix = "";

				// escape any posix special characters;
				// [\^$.|?*+()
				// if present in the search criteria, they need to be escaped with
				// TWO backslashes.
				word1.replacer("\\", "\\\\");
				var special = "[^$.|?*+()";
				for (int ii = special.len(); ii > 0; --ii) {
					if (special.contains(word1.at(ii)))
						word1.paster(ii, "\\");
				}
				word1.replacer("'" _FM "'", postfix ^ "'" _FM "'" ^ prefix);
				word1.paster(-1, postfix);
				word1.paster(2, prefix);

				//only ops <> and != are supported when using the regular expression operator (starting/ending/containing)
				if (op == "<>")
					negative = !negative;
				else if (op != "=" and op != "") [[unlikely]] {
					var errmsg = "SELECT ... WITH " ^ op ^ " " ^ word1 ^ " is not supported. " ^ prefix.quote() ^ " " ^ postfix.quote();
					throw VarDBException(errmsg);
				}

				// use regular expression operator
				op = "~";
				ucword = word1;
			}

			// word1 at this point may be empty, contain a value or be the first word of an unrelated clause
			// if non-value word1 unrelated to current phrase
			if (ucword.len() && !valuechars.contains(ucword.first())) {

				// push back and treat as missing value
				// remaining[1,0]=ucword:' '
				remaining.prefixer(ucword ^ " ");

				// simulate no given value .. so a boolean filter like "WITH APPROVED"
				word1 = "";
				ucword = "";
			}

			var value = word1;

			//change 'WITH SOMEMVFIELD = ""' to just 'WITH SOMEMVFIELD' to avoid ts_vector searching for nothing
			if (value == "''") {

				//remove multivalue handling - duplicate code elsewhere
				if (dictexpression.contains("to_tsvector(")) {
					//dont create exodus.tobool(to_tsvector(...
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

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.contains("exodus_call"))
			//if (dictexpression == "true")
			{
				//no op or value means test for Pick/AREV true (zero and '' are false)
				if (op == "" && value == "")
					op = "!!";

				//missing op presumed to be =
				else if (op == "")
					op = "=";

				// invert comparison if "without" or "not" for calculated fields
				if (negative &&
					var("= <> > < >= <= ~ ~* !~ !~* !! ! ]").locateusing(" ", op, aliasno)) {
					// op.logputl("op entered:");
					negative = false;
					op = var("<> = <= >= < > !~ !~* ~ ~* ! !! !]").field(" ", aliasno);
					// op.logputl("op reversed:");
				}

				//++ncalc_fields;
				//calc_fields.r(1,ncalc_fields,dictid);
				//calc_fields.r(2,ncalc_fields,op);
				//calc_fields.r(3,ncalc_fields,value);
				//dictid = calc_fields.f(1,n);
				//op     = calc_fields.f(2,n);
				//values  = calc_fields.f(3,n);

				//almost identical code for exodus_call above/below
				var calc_fieldn;
				if (!calc_fields.locate(dictid, calc_fieldn, 1)) {
					//++ncalc_fields;
					//calc_fields.r(1, calc_fieldn, dictid);
					calc_fields(1, calc_fieldn) = dictid;
				}
				if (calc_fields.f(2, calc_fieldn)) [[unlikely]] {
					var errmsg = "WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote();
					throw VarDBException(errmsg);
				}

				//save the op
				//calc_fields.r(2, calc_fieldn, op);
				calc_fields(2, calc_fieldn) = op;

				//save the value(s) after removing quotes and using SM to separate values instead of FM
				//calc_fields.r(3, calc_fieldn, value.unquote().replace("'" _FM "'", FM).convert(FM, SM));
				calc_fields(3, calc_fieldn) = value.unquote().replace("'" _FM "'", FM).convert(FM, SM);

				//place holder to be removed before issuing actual sql command
	            if (DBTRACE_SELECT)
					TRACE(whereclause)
				whereclause ^= " true";

				continue;
			}

			///////////////////////////////////////////////////////////
			// Filter Stage 8 - DUMMY OP AND VALUE SAVE IF NOT PROVIDED
			///////////////////////////////////////////////////////////

			// missing op and value means NOT '' or NOT 0 or NOT nullptr
			// WITH CLIENT_TYPE
			if (op == "" && value == "") {
				//op = "<>";
				//value = "''";

				var origdictexpression = dictexpression;
				//remove conversion to date/number etc
				//i.e for non-symbolic dicts i.e exodus.extract_date() and not dict_clients_stopped2()
				to_extract_text(dictexpression);
				var replacedbyextracttext = dictexpression != origdictexpression;

				//remove conversion to array
				//eg string_to_array(exodus.extract_text(JOBS.data,6, 0, 0), chr(29),'')
				if (dictexpression.starts("string_to_array(")) {
					dictexpression.cutter(16);
					dictexpression.cutter(-13);
				}

				//remove multivalue handling - duplicate code elsewhere
				if (dictexpression.contains("to_tsvector(")) {
					//dont create exodus.tobool(to_tsvector(...
					dictexpression.replacer("to_tsvector('simple',","");
					dictexpression.popper();
					//TRACE(dictexpression)
					dictexpression_isvector = false;
				}

				//currently tobool requires only text input
				//TODO some way to detect DATE SYMBOLIC FIELDS and not hack special dict words!
				//doesnt work on multivalued fields - results in:
				//exodus.tobool(SELECT_CURSOR_STAGE2_19397_37442_012029.TOT_SUPPINV_AMOUNT_BASE_calc, chr(29),)
				//TODO work out better way of determining DATE/TIME that must be tested versus null
				//if (isdatetime || dictexpression.contains("FULLY_") || (!dictexpression.contains("exodus.extract") && dictexpression.contains("_DATE")))
				//if (isdatetime)
				if (isdatetime and not replacedbyextracttext)
					dictexpression ^= " is not null";
				else
					dictexpression = "exodus.tobool(" ^ dictexpression ^ ")";
			}

			// missing op means =
			// eg WITH CLIENT_TYPE "X" .. assume missing "=" sign
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

			//allow searching for text with * characters embedded
			//otherwise interpreted as glob character?
			if (dictexpression_isvector) {
				value.replacer("*", "\\*");
			}

			// STARTING
			// special processing for STARTING]
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

					dictexpression.regex_replacer("^exodus.extract_number\\(", "exodus.extract_text\\(");
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

				// indicate that the dictexpression is included in the value(s)
				op = "(";

			}

			// single value data with multiple values filter
			else if (value.contains(FM) && !dictexpression_isvector) {

				//WARNING ", " is replaceped in exoprog.cpp ::select()
				//so change there if changed here
				value.replacer(_FM, ", ");

				// no processing for arrays (why?)
				if (dictexpression_isarray) {
				}

				//lhs is an array ("multivalues" in postgres)
				//dont convert rhs to in() or any()
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

				//any of
				else {
					to_extract_text(dictexpression);
					value = "ANY(ARRAY[" ^ value ^ "])";
				}
			}

			//full text search OR mv data search
			if (dictexpression_isvector) {

				//see note on isxref in "multiple values" section above
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

				//in full text query on multiple words,
				//we implement that words all are required
				//all values and words separated by spaced are used as "word stems"

				//using to_tsquery to search multivalued data
				if (not dictexpression_isfulltext) {

					//double the single quotes so the whole thing can be wrapped in single quotes
					//and because to_tsquery generates a syntax error in case of spaces inside values unless quotedd
					value.replacer("'","''");

					//wrap everything in single quotes for sql
					value.squoter();

					//multiple with options become alternatives using to_tsquery | divider
					value.replacer(_FM, "|");

				}

				// if full text search
				//if (dictid.last(5).ucase() == "_XREF") {
				if (dictexpression_isfulltext) {

					//https://www.postgresql.org/docs/current/textsearch-controls.html
					//and
					//https://www.postgresql.org/docs/current/datatype-textsearch.html#DATATYPE-TSQUERY

					//construct according to ts_query syntax using & | ( )
					//e.g. trying to find records containing either ADIDAS or KIA MOTORS where \036 is VM
					//value 'ADID\036KIA&MOT' -> '(ADID:*)|(KIA:*&MOT:*)

					// xxx:* searches for words starting with xxx

					//multivalues are searched using "OR" which is the | pipe character in ts_query syntax
					//words separated by spaces (or & characters) are searched for uing "AND" which is & in ts_query syntax
					var values="";
					value.unquoter().converter(VM,FM);
					for (var partvalue : value) {

						//remove all single quotes
						//partvalue.converter("'","");

						//replace all single quotes in search term for pairs of single quotes as per postgres syntax
						partvalue.replacer("'","''");

						//append postfix :* to every search word
						//so STEV:* also finds STEVE and STEVEN

						//spaces should have been converted to & before selection
						//spaces imply &
						//partvalue.replacer(" ", "&");
						//partvalue.paster(-1, ":*");

						//treat entered colons as &
						partvalue.replacer(":", "&");

						//respect any user entered AND or OR operators
						//search for all words STARTING with user defined words
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
				//select multivalues starting "XYZ" by selecting "XYZ]"
				else if (postfix) {
					value.replacer("]''", "'':*");
					value.replacer("]", ":*");
					//value.replacer("|", ":*|");
					value.paster(-1, ":*");
				}

				value.replacer("]''", "'':*");
				value.replacer("]", ":*");
				//value.paster(-1, ":*");

				//use "simple" dictionary (ie none) to allow searching for words starting with 'a'
				//use "english" dictionary for stemming (or "simple" dictionary for none)
				// MUST use the SAME in both to_tsvector AND to_tsquery
				//https://www.postgresql.org/docs/10/textsearch-dictionaries.html
				//this is the sole occurrence of to_tsquery in mvdbpostgres.cpp
				//it will be used like to_tsvector(...) @@ to_tsquery(...)
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

			// testing for "" may become testing for null
			// for date and time which are returned as null for empty string
			else if (value == "''") {
				if (dictexpression.contains("extract_date") ||
					dictexpression.contains("extract_datetime") ||
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
				// currently number returns 0 for empty string
				//|| dictexpression.contains("extract_number")
				else if (dictexpression.contains("extract_number")) {
					//value = "'0'";
					dictexpression.replacer("extract_number(","extract_text(");
				}
				//horrible hack to allow filtering calculated date fields versus ""
				//TODO detect FULLY_BOOKED and FULLY_APPROVED as dates automatically
				//else if (isdatetime || dictexpression.contains("FULLY_")) {
				else if (isdatetime) {
					if (op == "=")
						op = "is";
					else
						op = "is not";
					value = "null";
				}
			}

			//if selecting a mv array then convert right hand side to array
			//(can only handle = operator at the moment)
			if (dictexpression_isarray && (op == "=" or op == "<>")) {
				if (op == "<>") {
					negative = !negative;
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
				var lastpart = whereclause.field2(" ", -1);
				if (not var("OR AND (").locateusing(" ", lastpart))
					whereclause ^= " or ";
				if (DBTRACE_SELECT)
					TRACE(whereclause)
			}

			//negate
			if (negative)
				whereclause ^= " not";

			if (op == "(")
				whereclause ^= " ( " ^ value ^ " )";
			else
				whereclause ^= " " ^ dictexpression ^ " " ^ op ^ " " ^ value;

			if (DBTRACE_SELECT)
				TRACE(whereclause)

		}  //with/without

	}  // getword loop

	if (calc_fields && orwith) {
		//		throw VarDBException("OR not allowed with sort/select calculated fields");
	}

	// prefix specified keys into where clause
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

	//if calculated fields then secondary sort/select is going to use data column, so add the data column if missing
	if (calc_fields && not actualfieldnames.ends(", data"))
		actualfieldnames ^= ", data";

	//remove mv::integer if no unnesting (sort on mv fields)
	if (!unnests) {
		// sql ^= ", 0 as mv";
		if (actualfieldnames.contains("mv::integer, data")) {
			// replace the mv column with zero if selecting record
			actualfieldnames.replacer("mv::integer, data", "0::integer, data");
		} else
			actualfieldnames.replacer(", mv::integer", "");
	}

	// if any active select, convert to a file and use as an additional filter on key
	// or correctly named savelistfilename exists from getselect or makelist
	var listname = "";
	// see also listname below
	//	if (this->hasnext()) {
	//		listname=this->f(1) ^ "_" ^ ospid() ^ "_tempx";
	//		this->savelist(listname);
	//		var savelistfilename="savelist_" ^ listname;
	//		joins ^= " \nINNER JOIN\n " ^ savelistfilename ^ " ON " ^ actualfilename ^
	//".key = " ^ savelistfilename ^ ".key";
	//	}

	// disambiguate from any INNER JOIN key
	//actualfieldnames.logputl("actualfieldnames=");
	//actualfieldnames.replacer("key", actualfilename ^ ".key");
	//actualfieldnames.replacer("data", actualfilename ^ ".data");
	actualfieldnames.regex_replacer("\\bkey\\b", actualfilename ^ ".key");
	actualfieldnames.regex_replacer("\\bdata\\b", actualfilename ^ ".data");

	// DISTINCT has special fieldnames
	if (distinctfieldnames)
		actualfieldnames = distinctfieldnames;

	// remove redundant clauses
	whereclause.replacer("\n AND true", "");
	whereclause.replacer("true\n AND ", "");

	//determine the connection from the filename
	//could be an attached on a non-default connection
	//selecting dict files would trigger this
	//TRACE(*this)
	//TRACE(actualfilename)
	if (not this->f(2) || actualfilename.lcase().starts("dict.")) {
		var actualfile;
		if (actualfile.open(actualfilename))
			this->r(2, actualfile.f(2));
		//TRACE(actualfile)
	}
	//TRACE(*this)
	//save any active selection in a temporary table and INNER JOIN to it to avoid complete selection of primary file
	if (this->hasnext()) {

		//create a temporary sql table to hold the preselected keys
		var temptablename = "SELECT_CURSOR_" ^ this->f(1);
		//var createtablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
		//createtablesql ^= "CREATE TABLE " ^ temptablename ^ "\n";
		var createtablesql = "CREATE TEMPORARY TABLE IF NOT EXISTS " ^ temptablename ^ "\n";
		createtablesql ^= " (KEY TEXT)\n";
		createtablesql ^= ";DELETE FROM " ^ temptablename ^ "\n";
		var errmsg;
		if (!this->sqlexec(createtablesql, errmsg)) [[unlikely]] {
			throw VarDBException(errmsg);
		}

		//readnext the keys into a temporary table
		var key;
		while (this->readnext(key)) {
			//std::cout<<key<<std::endl;
			if (not this->sqlexec("INSERT INTO " ^ temptablename ^ "(KEY) VALUES('" ^ key.replace("'", "''") ^ "')"))
				[[unlikely]]
				throw VarDBException(lasterror());
		}

		if (this->f(3)) [[unlikely]] {
			throw VarDBException("selectx: Error. this->f(3) must be empty");
		}
		//must be empty!

		joins.inserter(1, 1, "\n RIGHT JOIN " ^ temptablename ^ " ON " ^ temptablename ^ ".key = " ^ actualfilename ^ ".key");
	}

	// Assemble the full sql select statement:

	// DECLARE - cursor
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

	//SELECT - field/column names
	sql ^= " \nSELECT\n " ^ actualfieldnames;
	if (selects)
		sql ^= selects;

	//FROM - filename and any specially related files
	sql ^= " \nFROM\n " ^ actualfilename;

	//JOIN - (1)?
	if (joins.f(1))
		sql ^= " " ^ joins.f(1).convert(VM, "\n");

	//UNNEST - mv fields
	//mv fields get added to the FROM clause like "unnest() as xyz" allowing the use of xyz in WHERE/ORDER BY
	//should only be one unnest (parallel mvs if more than one) since it is not clear how sselect by mv by mv2 should work if they are not in parallel
	if (unnests) {
		// unnest
		sql ^= ",\n unnest(\n  " ^ unnests.f(3).replace(VM, ",\n  ") ^ "\n )";
		// as fake tablename
		sql ^= " with ordinality as mvtable1";
		// brackets allow providing column names for use elsewhere
		// and renaming of automatic column "ORDINAL" to "mv" for use in SELECT key,mv ...
		// sql statement
		sql ^= "( " ^ unnests.f(2).replace(VM, ", ") ^ ", mv)";
	}

	//JOIN - related files
	if (joins.f(2))
		sql ^= " " ^ joins.f(2).convert(VM, "");

	//WHERE - excludes calculated fields if doing stage 1 of a two stage sort/select
	//TODO when doing stage2, skip "WITH/WITHOUT xxx" of stage1 fields
	if (whereclause)
		sql ^= " \nWHERE \n" ^ whereclause;

	//ORDER - suppressed if doing stage 1 of a two stage sort/select
	if (orderclause && !calc_fields)
		sql ^= " \nORDER BY \n" ^ orderclause.cut(2);

	//LIMIT - number of records returned
	// no limit initially if any calculated items - limit will be done in secondary sort/select
	if (maxnrecs && !calc_fields)
		sql ^= " \nLIMIT\n " ^ maxnrecs;

	// Final catch of obsolete function that was replaced by COLLATE keyword
	sql.regex_replacer("exodus.extract_sort\\(", "exodus.extract_text\\(");

	//sql.logputl("sql=");

	// DEBUG_LOG_SQL
	// if (DBTRACE)
	//	exodus::logputl(sql);

	// first close any existing cursor with the same name, otherwise cannot create  new cursor
    // Avoid generating sql errors since they abort transactions
	if (this->cursorexists()) {
		var sql2 = "";
		sql2 ^= "CLOSE cursor1_";

		if (this->assigned()) {
			var cursorcode = this->f(1).convert(".", "_");
			sql2 ^= cursorcode;
			var cursorid = this->f(2) ^ "_" ^ cursorcode;
			thread_dbresults.erase(cursorid);
		}

		var errmsg;
		if (!this->sqlexec(sql2, errmsg)) {

			if (errmsg)
				errmsg.errputl("::selectx on handle(" ^ *this ^ ") " ^ sql2 ^ "\n");
			// return false;
		}
	}

	var errmsg;
	if (!this->sqlexec(sql, errmsg)) [[unlikely]] {

		if (listname)
			this->deletelist(listname);

		// TODO handle duplicate_cursor sqlstate 42P03
		sql.logputl("sql=");

		throw VarDBException(errmsg);

		// if (autotrans)
		//	rollbacktrans();
		//std::unreachable();
		//return false;
	}

	//sort/select on calculated items may be done in exoprog::calculate
	//which can call calculate() and has access to mv.RECORD, mv.ID etc
	if (calc_fields) {
		//calc_fields.r(5, dictfilename.lower());
		//calc_fields.r(6, maxnrecs);
		//this->r(10, calc_fields.lower());
		calc_fields(5) = dictfilename.lower();
		calc_fields(6) = maxnrecs;
		(*this)(10) = calc_fields.lower();
	}

	return true;
}

void var::clearselect() {

	if (DBTRACE)
		var("clearselect");

	// THISIS("void var::clearselect() const")
	// assertString(function_sig);

	// default cursor is ""
	this->default_to("");

	/// if readnext through string
	//3/4/5/6 setup in makelist. cleared in clearselect
	//if (this->f(3) == "%MAKELIST%")
	{
		this->r(6, "");
		this->r(5, "");
		this->r(4, "");
		this->r(3, "");
		//		return;
	}

	var listname = (*this) ^ "_" ^ ospid() ^ "_tempx";

	// if (DBTRACE)
	//	exodus::logputl("DBTRACE: ::clearselect() for " ^ listname);

	// Dont close cursor unless it exists otherwise sql error aborts any transaction
    // Avoid generating sql errors since they abort transactions
	// if (not this->cursorexists())
	if (not this->cursorexists())
		return;

	// clear any select list
	this->deletelist(listname);

	var errors;

	//delete any temporary sql table created to hold preselected keys
	//if (this->assigned())
	//{
	//	var temptablename="PRESELECT_CURSOR_" ^ this->f(1);
	//	var deletetablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	//	if (!this->sqlexec(deletetablesql, errors))
	//	{
	//		if (errors)
	//			errors.logputl("::clearselect " ^ errors);
	//		return;
	//	}
	//}

	//if (this->assigned())
	var cursorcode = this->f(1).convert(".", "_");
	var cursorid = this->f(2) ^ "_" ^ cursorcode;

	// Clean up cursor cache
	thread_dbresults.erase(cursorid);

	var sql = "";
	// sql^="DECLARE BEGIN ";
	sql ^= "CLOSE cursor1_" ^ cursorcode;
	// sql^="\nEXCEPTION WHEN\n invalid_cursor_name\n THEN";
	// sql^="\nEND";

	//sql.output();

	if (!this->sqlexec(sql, errors)) {
		if (errors)
			errors.errputl("::clearselect on handle(" ^ *this ^ ") ");
		return;
	}

	return;
}

// NB global not member function
//	To make it var:: privat member -> pollute mv.h with PGresultptr :(
// bool readnextx(const std::string& cursor, PGresultptr& dbresult)
// called by readnext (and perhaps hasnext/select to implement LISTACTIVE)
//bool readnextx(CVR cursor, PGresult* pgresult, PGconn* pgconn, int  nrows, int* rown) {
static bool readnextx(CVR cursor, PGconn* pgconn, int  direction, PGresult*& pgresult, int* rown) {

	var cursorcode = cursor.f(1).convert(".", "_");
	var cursorid = cursor.f(2) ^ "_" ^ cursorcode;

	DBresult* dbresult = nullptr;
	auto entry = thread_dbresults.find(cursorid);
	if (entry != thread_dbresults.end()) {

		// Extract the current pgresult and rown of the cursor
		dbresult = &entry->second;
		*rown = dbresult->rown_;

		// If backwards
		// (should only be done after going forwards)
		if (direction < 0) {

			// rown is unlikely to be used after requesting backwards
			(*rown)--;

			dbresult->rown_ = *rown;

			return true;
		}

		//Increment the rown counter
		(*rown)++;

		// If forwards
		// Increment the current rown index into the pgresult
		// return success and the rown if within bounds
//TRACE(*rown)
//TRACE(PQntuples(*dbresult))
		if (*rown < PQntuples(*dbresult)) {

			// Save the rown for the next iteration
			dbresult->rown_ = *rown;

			// Return the pgresult array
			pgresult = dbresult->pgresult_;

//TRACE(*rown)
//TRACE(getpgresultcell(pgresult, *rown, 0))

			// Indicate success. true = found a new key/record
			return true;
		}

	}

	var fetch_nrecs=64;

	var sql;
	//if (direction >= 0)
	sql = "FETCH FORWARD " ^ fetch_nrecs ^ " in cursor1_" ^ cursorcode;
	//else
	//	sql = "FETCH BACKWARD " ^ var(-1).abs() ^ " in cursor1_" ^ cursorcode;

	// sql="BEGIN;" ^ sql ^ "; END";

	// execute the sql
	// cant use sqlexec here because it returns data
	// sqlexec();
	DBresult dbresult2;
	if (!get_dbresult(sql, dbresult2, pgconn)) {

		if (entry != thread_dbresults.end())
			thread_dbresults.erase(entry);

		var errmsg = var(PQresultErrorMessage(dbresult2));
		// errmsg.logputl("errmsg=");
		// var(dbresult2).logputl("dbresult2=");
		var sqlstate = "";
		if (PQresultErrorField(dbresult2, PG_DIAG_SQLSTATE)) {
			sqlstate = var(PQresultErrorField(dbresult2, PG_DIAG_SQLSTATE));
		}
		// dbresult2 is NULLPTR if if get_dbresult failed but since the dbresult is needed by
		// the caller, it will be cleared by called if not NULLPTR PQclear(dbresult2);

		// if cursor simply doesnt exist then see if a savelist one is available and enable
		// it 34000 - "ERROR:  cursor "cursor1_" does not exist"
		if (direction >= 0 && sqlstate == "34000") {
			return false;

			/**
			//if the standard select list file is available then select it, i.e. create
			a CURSOR, so FETCH has something to work on var listfilename="savelist_" ^
			cursor ^ "_" ^ ospid() ^ "_tempx"; if (not var().open(listfilename))
				return false;
			//TODO should add BY LISTITEMNO
			if (not cursor.select("select " ^ listfilename))
				return false;
			if (DBTRACE)
				exodus::logputl("DBTRACE: readnextx(...) found standard selectfile "
			^ listfilename);

			return readnextx(cursor, dbresult, pgconn, clearselect_onfail, forwards);
			**/
		}

		// any other error
		if (errmsg) [[unlikely]] {
			errmsg ^= " sqlstate= " ^ sqlstate.quote() ^ " in SQL " ^ sql;
			throw VarDBException(errmsg);
		}

		return false;
	}

	// If no rows returned
	//if (!PQntuples(*dbresult))
	//	return false;

	// 1. Do NOT clear the cursor even if forward since we may be testing it
	// 2. DO NOT clear since the dbresult2 is needed by the caller

	//Increment the rown counter from -1 to 0
	//dbresult2.rown_++;

	// Save rown for the next iteration
	*rown = dbresult2.rown_;

//TRACE(dbresult2.rown_)

	// Return a pointer to the pgresult
	pgresult = dbresult2;

	// Transfer the probably multi-row result into the thread_dbresults cache
	// for subsequent readnextx
	//thread_dbresults[cursorid] = pgresult;
	//thread_dbresults[cursorid] = dbresult2;
	if (entry != thread_dbresults.end())
		entry->second = pgresult;
	else
		thread_dbresults[cursorid] = pgresult;

	// Relinquish ownership of pgresult
	dbresult2.pgresult_ = nullptr;

	//dump_pgresult(pgresult);

	// Indicate success. true = found a new key/record
	return true;
}

bool var::deletelist(CVR listname) const {

	THISIS("bool var::deletelist(CVR listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	assertDefined(function_sig);
	ISSTRING(listname)

	if (DBTRACE)
		this->logputl("DBTR var::deletelist(" ^ listname ^ ") ");

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS")) [[unlikely]] 
		//skip this error for now because maybe no LISTS on some databases
		return false;
		//throw VarDBException("deletelist() LISTS file cannot be opened");

	// initial block of keys are stored with no suffix (i.e. no *1)
	lists.deleterecord(listname);

	// supplementary blocks of keys are stored with suffix *2, *3 etc)
	for (int listno = 2;; ++listno) {
		var xx;
		if (!xx.read(lists, listname ^ "*" ^ listno))
			break;
		lists.deleterecord(listname ^ "*" ^ listno);
	}

	return true;
}

bool var::savelist(CVR listname) {

	THISIS("bool var::savelist(CVR listname)")
	//?allow undefined usage like var xyz=xyz.select();
	assertDefined(function_sig);
	ISSTRING(listname)

	if (DBTRACE)
		this->logputl("DBTR var::savelist(" ^ listname ^ ") ");

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS")) [[unlikely]] {
		var errmsg = "savelist() LISTS file cannot be opened";
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	var listno = 1;
	var listkey = listname;
	var list = "";
	// Limit maximum number of keys in one block to 1Mb
	constexpr int maxlistsize = 1024 * 1024;

	// Function to write list of keys.
	// Called in readnext loop if list gets too large
	// and after the loop to save any unsaved keys
	auto write_list = [&] () {

		if (!list.var_str.size())
			return;

		// Delete any prior list with the same name
		if (listno == 1) {
			// this should not throw if the list does not exist
			this->deletelist(listname);
		}

		// Remove trailing FM
		list.var_str.pop_back();

		// save the block
		list.write(lists, listkey);

		// prepare the next block
		// first list block is listno 1 but has no suffix
		// 2nd list block is listno 2 and has suffice *2
		listno++;
		listkey = listname ^ "*" ^ listno;
		list = "";
	};

	var key;
	var mv;
	while (this->readnext(key, mv)) {

		// append the key to the list
		list.var_str.append(key.var_str);

		// append SM + mvno if mvno present
		if (mv) {
			list.var_str.push_back(VM_);
			list.var_str.append(mv.var_str);
		}

		// save one list of keys if more than a certain size (1MB)
		if (list.len() > maxlistsize) {

			write_list();

			continue;
		}

		// append a FM separator since lists use FM
		list.var_str.push_back(FM_);
	}

	// write any pending list
	write_list();

	return listno > 1;
}

bool var::getlist(CVR listname) {

	THISIS("bool var::getlist(CVR listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	assertDefined(function_sig);
	ISSTRING(listname)

	if (DBTRACE)
		listname.logputl("DBTR var::getlist(" ^ listname ^ ") ");

	//int recn = 0;
	var key;
	var mv;
	var listfilename = "savelist_" ^ listname.field(" ", 1);
	listfilename.converter("-.*/", "____");
	// return this->selectx("key, mv::integer",listfilename);

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS")) [[unlikely]] {
		var errmsg = "getlist() LISTS file cannot be opened";
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}

	var keys;
	if (!keys.read(lists, listname))
		// throw VarDBException(listname.quote() ^ " list does not exist.");
		return false;

	// provide first block of keys for readnext
	this->r(3, listname);

	// list number for readnext to get next block of keys from lists file
	// suffix for first block is nothing (not *1) and then *2, *3 etc
	this->r(4, 1);

	// key pointer for readnext to remove next key from the block of keys
	this->r(5, 0);

	// keys separated by vm. each key may be followed by a sm and the mv no for readnext
	this->r(6, keys.lowerer());

	return true;
}

//TODO make it work for multiple keys or select list
bool var::formlist(CVR keys, CVR fieldno) {

	THISIS("bool var::formlist(CVR keys, CVR fieldno)")
	//?allow undefined usage like var xyz=xyz.select();
	assertString(function_sig);
	ISSTRING(keys)
	ISNUMERIC(fieldno)

	if (DBTRACE)
		keys.logputl("DBTR var::formlist() ");

	this->clearselect();

	var record;
	if (not record.read(*this, keys)) {
		keys.errputl("formlist() cannot read on handle(" ^ *this ^ ") ");
		return false;
	}

	//optional field extract
	if (fieldno)
		record = record.f(fieldno).converter(VM, FM);

	if (not this->makelist("", record))
		[[unlikely]]
		throw VarDBException(this->lasterror());

	return true;
}

// MAKELIST would be much better called MAKESELECT
// since the most common usage is to omit listname in which case the keys will be used to simulate a
// SELECT statement Making a list can be done simply by writing the keys into the list file without
// using this function
bool var::makelist(CVR listname, CVR keys) {

	THISIS("bool var::makelist(CVR listname)")
	//?allow undefined usage like var xyz=xyz.select();
	assertDefined(function_sig);
	ISSTRING(listname)
	ISSTRING(keys)

	if (DBTRACE)
		this->logputl("DBTR var::makelist(" ^ listname ^ ") ");

	// this is not often used since can be achieved by writing keys to lists file directly
	if (listname) {
		this->deletelist(listname);

		// open the lists file on the same connection
		var lists = *this;
		if (!lists.open("LISTS")) [[unlikely]] {
			var errmsg = "makelist() LISTS file cannot be opened";
			this->setlasterror(errmsg);
			// this->loglasterror();
			throw VarDBException(errmsg);
		}

		keys.write(lists, listname);
		return true;
	}

	// provide a block of keys for readnext
	//3/4/5/6 setup in makelist. cleared in clearselect

	// listid in the lists file must be set for readnext to work, but not exist in the file
	// readnext will look for %MAKELIST%*2 in the lists file when it reaches the end of the
	// block of keys provided and must not find it
	this->r(3, "%MAKELIST%");

	// list number for readnext to get next block of keys from lists file
	// suffix for first block is nothing (not *1) and then *2, *3 etc
	this->r(4, 1);

	// key pointer for readnext to find next key from the block of keys
	this->r(5, 0);

	// keys separated by vm. each key may be followed by a sm and the mv no for readnext
	this->r(6, keys.lower());

	return true;
}

//bool var::hasnext() const {
bool var::hasnext() {

	// var xx;
	// return this->readnext(xx);

	THISIS("bool var::hasnext() const")
	// assertString(function_sig);

	// default cursor is ""
	this->default_to("");

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->f(3);
	if (listid) {
		var keyno = this->f(5);
		keyno++;

		var key_and_mv = this->f(6, keyno);

		// true if we have another key
		if (key_and_mv.len())
			return true;

		if (DBTRACE)
			this->logputl("DBTR var::hasnext(" ^ listid ^ ") ");

		// otherwise try and get another block
		var lists = *this;
		if (!lists.open("LISTS")) [[unlikely]] {
			var errmsg = "var::hasnext(): LISTS file cannot be opened";
			this->setlasterror(errmsg);
			// this->loglasterror();
			throw VarDBException(errmsg);
		}

		var listno = this->f(4);
		listno++;
		listid.fieldstorer("*", 2, 1, listno);

		// if no more blocks of keys then return false
		var block;
		if (!block.read(lists, listid)) {

			//clear the listid
			this->r(3, "");

			return false;
		}

		// might as well cache the next block for the next readnext
		this->r(4, listno);
		this->r(5, 0);
		this->r(6, block.lowerer());

		return true;
	}

	//TODO avoid this trip to the database somehow?
    // Avoid generating sql errors since they abort transactions
	if (!this->cursorexists())
		return false;

	auto pgconn = get_pgconn(*this);
	if (!pgconn) [[unlikely]] {
		var errmsg = "var::hasnext() get_pgconn() failed for " ^ this->quote();
		this->setlasterror(errmsg);
		// this->loglasterror();
		throw VarDBException(errmsg);
	}
	// The following pair of db requests is rather slow

	// Try to move the cursor forward
	PGresult* pgresult = nullptr;
	int rown;
	if (!readnextx(*this, pgconn, /*direction=*/1, pgresult, &rown))
		return false;

	//////////////////////////////
	// restore the cursor back one
	//////////////////////////////

	readnextx(*this, pgconn, /*direction=*/-1, pgresult, &rown);

	return true;
}

bool var::readnext(VARREF key) {
	var valueno;
	return this->readnext(key, valueno);
}

bool var::readnext(VARREF key, VARREF valueno) {

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ & VARTYP_MASK) {
		// throw VarUndefined("readnext()");
		var_str.clear();
		var_typ = VARTYP_STR;
	}

	// default cursor is ""
	this->default_to("");


	THISIS("bool var::readnext(VARREF key, VARREF valueno) const")
	assertString(function_sig);

	var record;
	return this->readnext(record, key, valueno);
}

bool var::readnext(VARREF record, VARREF key, VARREF valueno) {

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ & VARTYP_MASK || !var_typ) {
		// throw VarUndefined("readnext()");
		var_str.clear();
		var_typ = VARTYP_STR;
	}

	// default cursor is ""
	this->default_to("");

	THISIS("bool var::readnext(VARREF record, VARREF key, VARREF valueno) const")
	assertString(function_sig);
	ISDEFINED(key)
	ISDEFINED(record)

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->f(3);
	if (listid) {

		if (DBTRACE)
			this->logputl("DBTR var::readnext() ");

		record = "";
		while (true) {
			var keyno = this->f(5);
			keyno++;

			var key_and_mv = this->f(6, keyno);

			// if no more keys, try to get next block of keys, otherwise return false
			if (key_and_mv.len() == 0) {

				// makelist provides one block of keys and nothing in the lists file
				if (listid == "%MAKELIST%") {
					this->r(3, "");
					this->r(4, "");
					this->r(5, "");
					this->r(6, "");
					return false;
				}

				var lists = *this;
				if (!lists.open("LISTS")) [[unlikely]] {
					var errmsg = "readnext() LISTS file cannot be opened";
			        this->setlasterror(errmsg);
       				// this->loglasterror();
       				throw VarDBException(errmsg);
				}

				var listno = this->f(4);
				listno++;
				listid.fieldstorer("*", 2, 1, listno);

				var block;
				if (!block.read(lists, listid)) {

					//clear the listid
					this->r(3, "");

					return false;
				}

				this->r(4, listno);
				this->r(5, 0);
				this->r(6, block.lowerer());
				continue;
			}

			// bump up the key no pointer
			this->r(5, keyno);

			// extract and return the key (and mv if present)
			key = key_and_mv.f(1, 1, 1);
			valueno = key_and_mv.f(1, 1, 2);
			return true;
		}
	}

	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return false;

	// Avoid generating sql errors since they abort transactions
	if (!this->cursorexists())
		return false;

	//DBresult dbresult;
	PGresult* pgresult = nullptr;
	int rown;
	if (!readnextx(*this, pgconn, /*direction=*/1, pgresult, &rown)) {
		this->clearselect();
		return false;
	}

	//dump_pgresult(pgresult);

	// key is first column
	// char* data = PQgetvalue(dbresult, 0, 0);
	// int datalen = PQgetlength(dbresult, 0, 0);
	// key=std::string(data,datalen);
	key = getpgresultcell(pgresult, rown, 0);
	// TODO return zero if no mv in select because no by mv column

	//recursive call to skip any meta data with keys starting and ending %
	//eg keys like "%RECORDS%" (without the quotes)
	//similar code in readnext()
	if (key.starts("%") && key.ends("%")) {
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

	// record is third column
	if (PQnfields(pgresult) < 3) {
		//var errmsg = "readnext() must follow a select() clause with option (R)";
		//this->setlasterror(errmsg);
		//throw VarDBException(errmsg);
		// return false;

		// Dont throw an error, just return empty record. why?
		record = "";
	} else {
		record = getpgresultcell(pgresult, rown, 2);
	}

	return true;
}

bool var::createindex(CVR fieldname0, CVR dictfile) const {

	THISIS("bool var::createindex(CVR fieldname, CVR dictfile) const")
	assertString(function_sig);
	ISSTRING(fieldname0)
	ISSTRING(dictfile)

	var filename = get_normal_filename(*this);
	var fieldname = fieldname0.convert(".", "_");

	// actual dictfile to use is either given or defaults to that of the filename
	var actualdictfile;
	if (dictfile.assigned() and dictfile != "")
		actualdictfile = dictfile;
	else
		actualdictfile = "dict." ^ filename;

	// example sql
	// create index ads__brand_code on ads (exodus.extract_text(data,3,0,0));

	// throws if cannot find dict file or record
	var joins = "";   // throw away - cant index on joined fields at the moment
	var unnests = ""; // unnests are only created for ORDER BY, not indexing or selecting
	var selects = "";
	bool ismv;
	bool isdatetime;
	var forsort = false;
	var dictexpression = get_dictexpression(*this, filename, filename, actualdictfile, actualdictfile,
									fieldname, joins, unnests, selects, ismv, isdatetime, forsort);
	// dictexpression.logputl("dictexp=");stop();

	//mv fields return in unnests, not dictexpression
	//if (unnests)
	//{
	//	//dictexpression = unnests.f(3);
	//	unnests.convert(FM,"^").logputl("unnests=");
	//}

	var sql;

	// index on calculated columns causes an additional column to be created
	if (dictexpression.contains("exodus_call")) {
		("ERROR: Cannot create index on " ^ filename ^ " for calculated field " ^ fieldname).errputl();
		return false;

		/*

		// add a manually calculated index field
		var index_fieldname = "index_" ^ fieldname;
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
	if (this->listindex(filename, fieldname)) {
		setlasterror(filename.quote() ^ ", " ^ fieldname.quote() ^ " index already exists.");
		return false;
	}

	// Create postgres index
	sql = "CREATE INDEX index__" ^ filename ^ "__" ^ fieldname ^ " ON " ^ filename;
	//if (ismv || fieldname.lcase().ends("_xref"))
	if (dictexpression.contains("to_tsvector("))
		sql ^= " USING GIN";
	sql ^= " (";
	// unaccent requires "CREATE EXTENSION unaccent" in postgres
	sql ^= dictexpression;
	sql ^= ")";

	var response = "";
	if (!this->sqlexec(sql, response)) {
		//ERROR:  cannot create index on foreign table "clients"
		//sqlstate:42809 CREATE INDEX index__suppliers__SEQUENCE_XREF ON suppliers USING GIN (to_tsvector('simple',dict_suppliers_SEQUENCE_XREF(suppliers.key, suppliers.data)))
		if (!response.contains("sqlstate:42809"))
			response.errputl();
		return false;
	}

	return true;
}

bool var::deleteindex(CVR fieldname0) const {

	THISIS("bool var::deleteindex(CVR fieldname) const")
	assertString(function_sig);
	ISSTRING(fieldname0)

	var filename = get_normal_filename(*this);
	var fieldname = fieldname0.convert(".", "_");

	// delete the index field (actually only present on calculated field indexes so ignore
	// result) deleting the index field automatically deletes the index
	//var index_fieldname = "index_" ^ fieldname;
	//if (var().sqlexec("alter table " ^ filename ^ " drop " ^ index_fieldname))
	//	return true;

	// Fail neatly if the index does not exist
	// SQL errors during a transaction cause the whole transaction to fail.
	if (not this->listindex(filename, fieldname)) {
		setlasterror(filename.quote() ^ ", " ^ fieldname.quote() ^ " index does not exist.");
		return false;
	}

	// Delete the index.
	var sql = "drop index index__" ^ filename ^ "__" ^ fieldname;
	return this->sqlexec(sql);
}

var var::listfiles() const {

	THISIS("var var::listfiles() const")
	assertDefined(function_sig);

	// from http://www.alberton.info/postgresql_meta_info.html

	var schemafilter = " NOT IN ('pg_catalog', 'information_schema') ";

	var sql =
		"SELECT table_name, table_schema FROM information_schema.tables"
		//" WHERE table_type = 'BASE TABLE' AND";
		" WHERE";
	sql ^= " table_schema " ^ schemafilter;

	sql ^= " UNION SELECT matviewname as table_name, schemaname as table_schema from pg_matviews";
	sql ^= " WHERE schemaname " ^ schemafilter;

	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return "";

	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return "";

	var filenames = "";
	int nfiles = PQntuples(dbresult);
	for (int filen = 0; filen < nfiles; filen++) {
		if (!PQgetisnull(dbresult, filen, 0)) {
			var filename = getpgresultcell(dbresult, filen, 0);
			var schema = getpgresultcell(dbresult, filen, 1);
			if (schema == "public")
				filenames ^= filename;
			else
				filenames ^= schema ^ "." ^ filename;
			filenames.var_str.push_back(FM_);
		}
	}
	filenames.var_str.pop_back();

	return filenames;
}

var var::dblist() const {

	THISIS("var var::dblist() const")
	assertDefined(function_sig);

	var sql = "SELECT datname FROM pg_database";

	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return "";

	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return "";

	var dbnames = "";
	auto ndbs = PQntuples(dbresult);
	for (auto dbn = 0; dbn < ndbs; dbn++) {
		if (!PQgetisnull(dbresult, dbn, 0)) {
			std::string dbname = getpgresultcell(dbresult, dbn, 0);
			if (!dbname.starts_with("template") and !dbname.starts_with("postgres")) {
				dbnames.var_str.append(dbname);
				dbnames.var_str.push_back(FM_);
			}
		}
	}
	dbnames.var_str.pop_back();

	return dbnames.sort();
}

//TODO avoid round trip to server to check this somehow or avoid calling it all the time
bool var::cursorexists() {

	THISIS("bool var::cursorexists()")
	// could allow undefined usage since *this isnt used?
	assertDefined(function_sig);

    // Avoid generating sql errors since they abort transactions

	// default cursor is ""
	this->default_to("");

	var cursorcode = this->f(1).convert(".", "_");
	var cursorid = this->f(2) ^ "_" ^ cursorcode;

	// true if exists in cursor cache
	if (thread_dbresults.find(cursorid) != thread_dbresults.end())
		return true;

	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return false;

	// from http://www.alberton.info/postgresql_meta_info.html
	var sql = "SELECT name from pg_cursors where name = 'cursor1_" ^ cursorcode ^ "'";

	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return false;

	return PQntuples(dbresult) > 0;
}

var var::listindex(CVR filename0, CVR fieldname0) const {

	THISIS("var var::listindex(CVR filename) const")
	// could allow undefined usage since *this isnt used?
	assertDefined(function_sig);
	ISSTRING(filename0)
	ISSTRING(fieldname0)

	var filename = filename0.f(1);
	var fieldname = fieldname0.convert(".", "_");

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

	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return "";

	// execute command or return empty string
	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return "";

	std::string indexnames = "";
	int nindexes = PQntuples(dbresult);
	var lc_fieldname = fieldname.lcase();
	for (int indexn = 0; indexn < nindexes; indexn++) {
		if (!PQgetisnull(dbresult, indexn, 0)) {
			var indexname = getpgresultcell(dbresult, indexn, 0);
			if (indexname.starts("index_")) {
				if (indexname.contains("__")) {
					indexname.substrer(8, 999999).replacer("__", VM);
					if (fieldname && indexname.f(1, 2) != lc_fieldname)
						continue;

					indexnames += indexname.var_str;
					indexnames.push_back(FM_);
				}
			}
		}
	}

	var result = "";
	if (!indexnames.empty()) {
		indexnames.pop_back();
		result = std::move(indexnames);
		result.sorter();
	}
	return result;
}

var var::reccount(CVR filename0) const {

	THISIS("var var::reccount(CVR filename_or_handle_or_null) const")
	// could allow undefined usage since *this isnt used?
	assertDefined(function_sig);
	ISSTRING(filename0)

	var filename = filename0 ?: (*this);

	// vacuum otherwise unreliable
	if (!this->statustrans())
		this->flushindex(filename);

	var sql = "SELECT reltuples::integer FROM pg_class WHERE relname = '";
	sql ^= filename.f(1).lcase();
	sql ^= "';";

	auto pgconn = get_pgconn(filename);
	if (! pgconn)
		return "";

	// execute command or return empty string
	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return "";

	var reccount = "";
	if (PQntuples(dbresult) && PQnfields(dbresult) > 0 && !PQgetisnull(dbresult, 0, 0)) {
		// reccount=var((int)ntohl(*(uint32_t*)PQgetvalue(dbresult, 0, 0)));
		reccount = getpgresultcell(dbresult, 0, 0);
		reccount += 0;
	}

	return reccount;
}

var var::flushindex(CVR filename) const {

	THISIS("var var::flushindex(CVR filename=) const")
	// could allow undefined usage since *this isnt used?
	assertDefined(function_sig);
	ISSTRING(filename)

	var sql = "VACUUM";
	if (filename)
		// attribute 1 in case passed a filehandle instead of just filename
		sql ^= " " ^ filename.f(1).lcase();
	sql ^= ";";
	// sql.logputl("sql=");

	// TODO perhaps should get connection from filehandle if passed a filehandle
	auto pgconn = get_pgconn(*this);
	if (! pgconn)
		return "";

	// execute command or return empty string
	DBresult dbresult;
	if (!get_dbresult(sql, dbresult, pgconn))
		return "";

	var flushresult = "";
	if (PQntuples(dbresult)) {
		flushresult = true;
	}

	return flushresult;
}

var var::setlasterror(CVR msg) const {
	// no checking for speed
	// THISIS("void var::lasterror(CVR msg")
	// ISSTRING(msg)

	// tcache_get (tc_idx=12) at malloc.c:2943
	// 2943    malloc.c: No such file or directory.
	// You have heap corruption somewhere -- someone is running off the end of an array or
	// dereferencing an invalid pointer or using some object after it has been freed. EVADE
	// error for now by commenting next line

	thread_lasterror = msg;

	if (DBTRACE)
		TRACE(thread_lasterror)

	return lasterror();
}

var var::lasterror() const {
	//TRACE(thread_file_handles.size());
	return thread_lasterror;
}

var var::loglasterror(CVR source) const {
	return thread_lasterror.logputl(source ^ " ");
}

}  // namespace exodus
