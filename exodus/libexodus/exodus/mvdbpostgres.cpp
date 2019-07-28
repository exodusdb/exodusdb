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
//	mvdbfuncs.cpp - api things, like mvglobalfuncs.cpp
//	mvdbdrv.cpp - base abstract class mv_db_drv or MvDbDrv to define db access operations (db
// driver interface); 	mvdbdrvpostgres.cpp - subclass of mv_db_drv, specific to PostgreSQL things
// like
// PGconn and PQfinish; 	mvdbdrvmsde.cpp - possible subclass of mv_db_drv, specific to MSDE (AKA
// MSSQL Express); 	mvdblogic.cpp - intermediate processing (most of group 2) functions.
// Proposed refactoring would:
//		- improve modularity of the Exodus platform;
//		- allow easy expanding to other DB engines.

#ifndef DEBUG
#define TRACING 1
//#define TRACING 5
#else
#define TRACING 2
#endif

#if defined _MSC_VER // || defined __CYGWIN__ || defined __MINGW32__
#define WIN32_LEAN_AND_MEAN
#include <DelayImp.h>
#include <windows.h>
// doesnt seem to work, add them to visual studio project delayload section using semicolons or
// special linker options #pragma comment(linker, "/DelayLoad:libpq.dll")
#else
//# define __try try
//# define __except catch
#endif

#pragma warning(disable : 4150)
// warning C4150: deletion of pointer to incomplete type 'pg_conn'; no destructor called
// see declaration of 'pg_conn'

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <cstring> //for strcmp strlen
#include <iostream>

#include <boost/thread/tss.hpp>
// http://beta.boost.org/doc/libs/1_41_0/doc/html/unordered.html

// see exports.txt for all PQ functions
//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h> //in postgres/include

//#include <arpa/inet.h>//for ntohl()
#define MV_NO_NARROW

#include "MurmurHash2_64.h" // it has included in mvdbconns.h (uint64_t defined)

#include <exodus/mv.h>
#include <exodus/mvdbconns.h> // placed as last include, causes boost header compiler errors
//#include <exodus/mvenvironment.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus
{

// bool startipc();

// Deleter function to close connection and connection cache object
static void connection_DELETER_AND_DESTROYER(CACHED_CONNECTION con_)
{
	PGconn* pgp = (PGconn*)con_;
	PQfinish(pgp); // AFAIK, it destroys the object by pointer
		       //	delete pgp;
}
static MvConnectionsCache mv_connections_cache(connection_DELETER_AND_DESTROYER);

// DBTRACE is set in exodus_main (console programs) but not when used as a plain library
// so initialise it on the fly. assume that it will usually be less than one for not tracing
#define GETDBTRACE (DBTRACE >= 0 && getdbtrace())
bool getdbtrace()
{
	if (DBTRACE == 0)
		DBTRACE = var().osgetenv("EXO_DBTRACE") ? 1 : -1;
	return DBTRACE > 0;
}

//#if TRACING >= 5
#define DEBUG_LOG_SQL                                                                              \
	if (GETDBTRACE)                                                                            \
	{                                                                                          \
		sql.logputl("SQL:");                                                               \
	}
#define DEBUG_LOG_SQL1                                                                             \
	if (GETDBTRACE)                                                                            \
	{                                                                                          \
		sql.swap("$1", var(paramValues[0]).squote()).logputl("SQL1:");               \
	}
//#else
//#define DEBUG_LOG_SQL
//#define DEBUG_LOG_SQL1
//#endif

// this front end C interface is based on postgres
// http://www.postgresql.org/docs/8.2/static/libpq-exec.html
// doc/postgres/libpq-example.html
// src/test/examples/testlibpq.c

//(backend pg functions extract and dateextract are based
// on samples in src/tutorial/funcs_new.c)

// SQL EXAMPLES
// create or replace view testview as select exodus_extract_text(data,1,0,0) as field1 from test;
// create index testfield1 on test (exodus_extract_text(data,1,0,0));
// select * from testview where field1  > 'aaaaaaaaa';
// analyse;
// explain select * from testview where field1  > 'aaaaaaaaa';
// explain analyse select * from testview where field1  > 'aaaaaaaaa';e

//#include <pg_type.h>
#define BYTEAOID 17;
#define TEXTOID 25;

// this is not threadsafe
// PGconn	 *thread_pgconn;
// but this is ...
boost::thread_specific_ptr<int> tss_pgconnids;
boost::thread_specific_ptr<var> tss_pgconnparams;
boost::thread_specific_ptr<var> tss_pglasterror;
// boost::thread_specific_ptr<bool> tss_ipcstarted;

std::string getresult(PGresult* pgresult, int rown, int coln)
{
	return std::string(PQgetvalue(pgresult, rown, coln), PQgetlength(pgresult, rown, coln));
}

// used to detect sselect command words that are values like quoted words or plain numbers. eg "xxx"
// 'xxx' 123 .123 +123 -123
static const var valuechars("\"'.0123456789-+");

class Resultclearer
{

	// Save pgresultptr in this class and to guarantee that it will be PQClear'ed on function
	// exit Resultclearer clearer(pgresult);

      public:
	Resultclearer(PGresult* pgresult) : pgresult_(pgresult)
	{
		// var("Got pgresult ... ").output();
	}

	~Resultclearer()
	{
		// var("Cleared pgresult").outputl();
		if (pgresult_ != nullptr)
			PQclear(pgresult_);
	}

      private:
	PGresult* pgresult_ = nullptr;
};

void var::setlasterror(const var& msg) const
{
	// no checking for speed
	// THISIS("void var::setlasterror(const var& msg")
	// ISSTRING(msg)

	// tcache_get (tc_idx=12) at malloc.c:2943
	// 2943    malloc.c: No such file or directory.
	// You have heap corruption somewhere -- someone is running off the end of an array or
	// dereferencing an invalid pointer or using some object after it has been freed. EVADE
	// error for now by commenting next line

	tss_pglasterror.reset(new var(msg));
}

void var::setlasterror() const { tss_pglasterror.reset(); }

var var::getlasterror() const
{
	if (tss_pglasterror.get())
		return *tss_pglasterror.get();
	else
		return "";
}

typedef PGresult* PGresultptr;
static bool getpgresult(const var& sql, PGresultptr& pgresult, PGconn* thread_pgconn);

bool var::sqlexec(const var& sql) const
{
	var errmsg;
	bool ok = this->sqlexec(sql, errmsg);
	if (not ok)
	{
		this->setlasterror(errmsg);
		if (errmsg.index("syntax") || GETDBTRACE)
			errmsg.outputl();
	}
	return ok;
}

#if defined _MSC_VER //|| defined __CYGWIN__ || defined __MINGW32__
LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pExcPointers)
{
	LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;

	PDelayLoadInfo pDelayLoadInfo =
	    PDelayLoadInfo(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

	switch (pExcPointers->ExceptionRecord->ExceptionCode)
	{
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
bool msvc_PQconnectdb(PGconn** pgconn, const std::string& conninfo)
{
	// connect or fail
	__try
	{
		*pgconn = PQconnectdb(conninfo.c_str());
	}
	__except (DelayLoadDllExceptionFilter(GetExceptionInformation()))
	{
		return false;
	}
	return true;
}

#endif

// the idea is for exodus to have access to one standard database without secret password
static var defaultconninfo = "host=127.0.0.1 port=5432 dbname=exodus user=exodus "
			     "password=somesillysecret connect_timeout=10";

var var::build_conn_info(const var& conninfo) const
{
	// priority is
	// 1) given parameters or last connection parameters
	// 2) individual environment parameters
	// 3) environment connection string
	// 4) config file parameters
	// 5) default parameters

	var result(conninfo);
	// if no conninfo details provided then use last connection details if any
	if (!conninfo && tss_pgconnparams.get())
		result = *tss_pgconnparams.get();

	// otherwise search for details from exodus config file
	// if incomplete connection parameters provided
	if (not result.index("host=") or not result.index("port=") or not result.index("dbname=") or
	    not result.index("user=") or not result.index("password="))
	{

		// discover any configuration file parameters
		// TODO parse config properly instead of just changing \n\r to spaces!
		var configfilename = "";
		var home = "";
		if (home.osgetenv("HOME"))
			configfilename = home ^ SLASH ^ ".exodus";
		else if (home.osgetenv("USERPROFILE"))
			configfilename ^= home ^ SLASH ^ "Exodus\\.exodus";
		var configconn = "";
		if (!configconn.osread(configfilename))
			configconn.osread(".exodus");

		// discover any configuration in the environment
		var envconn = "";
		var temp;
		if (temp.osgetenv("EXO_CONNECTION"))
			envconn ^= " " ^ temp;

		// specific variable are appended ie override
		if (temp.osgetenv("EXO_HOST"))
			envconn ^= " host=" ^ temp;

		if (temp.osgetenv("EXO_PORT"))
			envconn ^= " port=" ^ temp;

		if (temp.osgetenv("EXO_USER"))
			envconn ^= " user=" ^ temp;

		if (temp.osgetenv("EXO_DBNAME"))
			envconn ^= " dbname=" ^ temp;

		if (temp.osgetenv("EXO_PASSWORD"))
			envconn ^= " password=" ^ temp;

		if (temp.osgetenv("EXO_TIMEOUT"))
			envconn ^= " connect_timeout=" ^ temp;

		result = defaultconninfo ^ " " ^ configconn ^ " " ^ envconn ^ " " ^ result;
	}
	return result;
}

// var connection;
// connection.connect2("dbname=exodusbase");
bool var::connect(const var& conninfo)
{
	THISIS("bool var::connect(const var& conninfo")
	// nb dont log/trace or otherwise output the full connection info without HIDING the
	// password
	THISISDEFINED()
	ISSTRING(conninfo)

	var conninfo2 = build_conn_info(conninfo);

	if (GETDBTRACE)
		conninfo2.logputl("DBTRACE:");

	PGconn* pgconn;
	for (;;)
	{
#if defined _MSC_VER //|| defined __CYGWIN__ || defined __MINGW32__
		if (not msvc_PQconnectdb(&pgconn, conninfo2.toString()))
		{
#if TRACING >= 1
			var libname = "libpq.dl";
			// var libname="libpq.so";
			var(
			    "ERROR: mvdbpostgres connect() Cannot load shared library " ^ libname ^
			    ". Verify configuration PATH contains postgres's \\bin.").errputl();
#endif
			return false;
		};
#else
		pgconn = PQconnectdb(conninfo2.toString().c_str());
#endif

		if (PQstatus(pgconn) == CONNECTION_OK || conninfo2)
			break;

		// required even if connect fails according to docs
		PQfinish(pgconn);
		// try again with default conninfo
		conninfo2 = defaultconninfo;
	}

	// failed to connect so return false
	if (PQstatus(pgconn) != CONNECTION_OK)
	{
#if TRACING >= 3
		var("ERROR: mvdbpostgres connect() Connection to database failed: " ^
				var(PQerrorMessage(pgconn))).errputl();
		// if (not conninfo2)
		var("ERROR: mvdbpostgres connect() Postgres connection configuration "
				"missing or incorrect. Please login.").errputl();
#endif

		// required even if connect fails according to docs
		PQfinish(pgconn);
		return false;
	}

// abort if multithreading and it is not supported
#ifdef PQisthreadsafe
	if (!PQisthreadsafe())
	{
		// TODO only abort if environmentn>0
		throw MVDBException("connect(): Postgres PQ library is not threadsafe");
	}
#endif

// at this point we have good new connection to database
#if TRACING >= 3
	var("var::connect() Connection to database succeeded.").logputl();
#endif

	// save a new connection handle
	int conn_no = mv_connections_cache.add_connection(pgconn);
	(*this) = conninfo ^ FM ^ conn_no;

	// this->outputl("new connection=");

	// set default connection
	// ONLY IF THERE ISNT ONE ALREADY
	int* connid = tss_pgconnids.get();
	if (connid == NULL)
		tss_pgconnids.reset(new int((int)conn_no));

	// save last connection string (used in startipc())
	tss_pgconnparams.reset(new var(conninfo2));

	// setup a thread to service callbacks from the database backend
	// if (!tss_ipcstarted.get())
	//{
	//	//	#if TRACING >= 3
	//		exodus::outputl("Starting IPC");
	//	//	#endif
	//	startipc();
	//}

	// doesnt work
	// need to set PQnoticeReceiver to suppress NOTICES like when creating files
	// PQsetErrorVerbosity(pgconn, PQERRORS_TERSE);
	// but this does
	// this turns off the notice when creating tables with a primary key
	// DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, LOG, NOTICE, WARNING, ERROR, FATAL, and PANIC
	this->sqlexec(var("SET client_min_messages = ") ^ (GETDBTRACE ? "LOG" : "WARNING"));

	return true;
}

int var::getdefaultconnectionid() const
{
	// otherwise return thread default connection id
	int* connid = tss_pgconnids.get();
	if (connid && *connid != 0)
	{
		//(var("getdefaultconnection found default thread connection id ") ^
		// *connid).outputl();
		return *connid;
	}
	else
	{
		// var("getdefaultconnection returning 0").outputl();
		return 0;
	}
}

bool var::setdefaultconnectionid() const
{
	THISIS("bool var::setdefaultconnectionid()")
	THISISDEFINED()

	// this should be a db connection
	// if (!THIS_IS_DBCONN())
	//	MVDBException("is not a valid connection in setdefaultconnectionid()");

	// connection number should be in field 2
	int connid = this->getconnectionid();
	if (!connid)
		MVDBException("setdefaultconnectionid() when not connected");

	if (!mv_connections_cache.get_connection(connid))
		MVDBException("is not a valid connection in setdefaultconnectionid()");

	// save current connection handle number as thread specific handle no
	tss_pgconnids.reset(new int((int)connid));

	return true;
}

int var::getconnectionid() const
{
	THISIS("bool var::getconnectionid()")
	THISISDEFINED();

	if (!this->assigned())
	{
		// var("getconnectionid() returning 0 - unassigned").outputl();
		return 0;
	}
	var connid = this->a(2);
	if (connid.isnum())
	{
		/// var("getconnectionid() returning " ^ connid).outputl();
		return connid;
	}

	// var("getconnectionid() returning 0 - not numeric").outputl();

	return 0;
}

int var::getconnectionid_ordefault() const
{
	// first return connection id if this is a connection handle
	//	if (THIS_IS_DBCONN())
	//		return (int) var_int;

	int connid2 = this->getconnectionid();
	if (connid2)
		return connid2;

	// otherwise return thread default connection id
	int* connid = tss_pgconnids.get();
	connid2 = 0;
	if (connid && *connid != 0)
	{
		connid2 = *connid;
		//(var("getconnectionid_ordefault found default thread connection id ") ^
		// connid2).outputl();
	}

	// otherwise do a default connect and do setdefaultconnection
	else
	{
		// exodus::outputl("getconnectionid_ordefault didnt find default thread connection
		// id");
		var conn1;
		if (conn1.connect())
		{
			conn1.setdefaultconnectionid();
			// connid2=(int) conn1.var_int;
			connid2 = conn1.getconnectionid();
		}
	}

	// turn this into a db connection (int holds the connection number)
	// leave any string in place but prevent it being used as a number
	// var_int = connid2;
	////var_str = ""; does it ever need initialising?
	// var_typ = VARTYP_NANSTR_DBCONN;

	// save the connection id
	// this->r(2,connid2);

	return connid2;
}

// var::connection()
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
void* var::connection() const
{

	// var("--- connection ---").outputl();
	// get the connection associated with *this
	int connid = this->getconnectionid();
	// var(connid).outputl("connid1=");

	// otherwise get the default connection
	if (!connid)
		connid = this->getdefaultconnectionid();

	// var(connid).outputl("connid2=");

	// otherwise try the default connection
	if (!connid)
	{
		var temp = "";
		if (temp.connect())
			connid = temp.getconnectionid();
	}
	// var(connid).outputl("connid3=");

	// otherwise error
	if (!connid)
		throw MVDBException("connection() attempted when not connected");

	// return a pg_connection structure
	return mv_connections_cache.get_connection(connid);
}

// gets lock_table, associated with connection, associated with this object
void* var::get_lock_table() const
{
	int connid = this->getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("get_lock_table() attempted when not connected");
	// return connid ? mv_connections_cache.get_lock_table(connid) : NULL;
	return mv_connections_cache.get_lock_table(connid);
}

// if this->obj contains connection_id, then such connection is disconnected with this-> becomes UNA
// Otherwise, default connection is disconnected
bool var::disconnect()
{
	THISIS("bool var::disconnect()")
	THISISDEFINED()

#if TRACING >= 3
	var("ERROR: mvdbpostgres disconnect() Closing connection").errputl();
#endif

	int* default_connid = tss_pgconnids.get();

	//	if (THIS_IS_DBCONN())
	//	{
	var connid = this->getconnectionid();
	if (connid)
	{
		mv_connections_cache.del_connection((int)connid);
		var_typ = VARTYP_UNA;
		// if we happen to be disconnecting the same connection as the default connection
		// then reset the default connection so that it will be reconnected to the next
		// connect this is rather too smart but will probably do what people expect
		if (default_connid && *default_connid == connid)
			tss_pgconnids.reset();
	}
	else
	{
		if (default_connid && *default_connid)
		{
			mv_connections_cache.del_connection(*default_connid);
			tss_pgconnids.reset();
		}
	}
	return true;
}

// open filehandle given a filename on current thread-default connection
// we are using strict filename/filehandle syntax (even though we could use one variable for both!)
// we store the filename in the filehandle since that is what we need to generate read/write sql
// later usage example:
// var file;
// var filename="customers";
// if (not file.open(filename)) ...

// connection is optional and default connection may be used instead
bool var::open(const var& filename, const var& connection /*DEFAULTNULL*/)
{
	THISIS("bool var::open(const var& filename, const var& connection)")
	THISISDEFINED()
	ISSTRING(filename)

	// asking to open DOS file! ok can osread/oswrite later!
	if (filename == "DOS")
		return true;

	std::string filename2 = filename.a(1).normalize().lcase().convert(".", "_").toString();

/*
	//$ parameter array
	const char* paramValues[1];
	int paramLengths[1];
	// int		paramFormats[1];

	// filename.outputl("filename=");
	//$1=table_name
	paramValues[0] = filename2.c_str();
	paramLengths[0] = int(filename2.length());
	// paramFormats[0] = 1;//binary

	// avoid any errors because ANY errors while a transaction is in progress cause
	// failure of the whole transaction
	// TODO should perhaps prepare pg parameters for repeated speed
	var sql = "SELECT table_name FROM information_schema.tables WHERE "
		  "table_schema='public' and table_name=$1";

	PGconn* thread_pgconn = (PGconn*)connection.connection();
	if (!thread_pgconn)
	{
		this->setlasterror("db connection not opened");
		return false;
	}

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
					  // TODO: parameterise filename
					  sql.toString().c_str(), 1, // one param
					  NULL, // let the backend deduce param type
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,
	// 1);	  // ask for binary results

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK)
	{
		var errmsg = "ERROR: mvdbpostgres 1 open(" ^ filename.quote() ^
			     ") failed\n" ^ var(PQerrorMessage(thread_pgconn));
		//			PQclear(pgresult);
		this->setlasterror(errmsg);
#if TRACING >= 1
		var(errmsg).errputl();
#endif
		return false;
	}

	// file (table) doesnt exist
	if (PQntuples(pgresult) < 1)
	{
		var errmsg = "ERROR: mvdbpostgres 2 open(" ^ filename.quote() ^
			     ") table does not exist. failed\n" ^
			     var(PQerrorMessage(thread_pgconn));
		//			PQclear(pgresult);
		this->setlasterror(errmsg);
		return false;
	}

	if (PQntuples(pgresult) > 1)
	{
		var errmsg =
		    "ERROR: mvdbpostgres 3 open() SELECT returned more than one record";
		//			PQclear(pgresult);
		this->setlasterror(errmsg);
#if TRACING >= 1
		errmsg.errputl();
#endif
		return false;
	}
*/
// *
	//check filename2 is a valid table or view etc.
	var sql="select '" ^ filename2 ^ "'::regclass";
	if (! connection.sqlexec(sql))
	{
		var errmsg = "ERROR: mvdbpostgres 2 open(" ^ filename.quote() ^
			     ") table does not exist.";
		this->setlasterror(errmsg);
		return false;
	}
// */
	this->setlasterror();

	// save the filename and connection no
	// memorise the current connection for this file var
	(*this) = filename2 ^ FM ^ connection.getconnectionid();

	// outputl("opened filehandle");

	return true;
}

void var::close()
{
	THISIS("void var::close()")
	THISISSTRING()
	/*TODO
		if (var_typ!=VARTYP_UNA) QMClose(var_int);
	*/
}

bool var::readv(const var& filehandle, const var& key, const int fieldno)
{
	if (!read(filehandle, key))
		return false;

	var_str = this->a(fieldno).var_str;
	return true;
}

bool var::reado(const var& filehandle, const var& key)
{
	// check cache first, and return any cached record
	int connid = filehandle.getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed");
	std::string cachedrecord =
	    mv_connections_cache.readrecord(connid, filehandle.a(1).var_str, key.var_str);
	if (!cachedrecord.empty())
	{
		// key.outputl("cache read " ^ filehandle.a(1) ^ "=");
		//(*this) = cachedrecord;
		var_str = cachedrecord;
		var_typ = VARTYP_STR;

		this->setlasterror();
		return true;
	}

	// ordinary read
	if (!read(filehandle, key))
		return false;

	// save in cache only when allowing read from cache ie only in reado
	// and NOT in read() or write()
	// so that ordinary reads dont cause caching - for safety
	mv_connections_cache.writerecord(connid, filehandle.a(1).var_str, key.var_str, var_str);

	return true;
}

bool var::read(const var& filehandle, const var& key)
{
	THISIS("bool var::read(const var& filehandle,const var& key)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

	//amending var_str invalidates all flags
	var_typ = VARTYP_STR;

	// asking to read DOS file! do osread using key as osfilename!
	if (filehandle == "DOS")
	{
		(*this).osread(key); //.convert("\\",SLASH));
		return true;
	}

	// asking to read DOS file! do osread using key as osfilename!
	if (filehandle == "")
	{
		var errmsg = "read(...) filename not specified, probably not opened.";
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		return false;
	}

	// reading a magic special key returns all keys in the file in natural order
	if (key == "%RECORDS%")
	{
		var sql = "SELECT key from " ^ filehandle.a(1).convert(".", "_") ^ ";";

		PGconn* pgconn = (PGconn*)filehandle.connection();
		if (pgconn == NULL)
			return "";

		PGresultptr pgresult;
		bool ok = getpgresult(sql, pgresult, pgconn);

		Resultclearer clearer(pgresult);

		if (!ok)
			return "";

		var_str = "";
		var keyn;
		int ntuples = PQntuples(pgresult);
		for (int tuplen = 0; tuplen < ntuples; tuplen++)
		{
			if (!PQgetisnull(pgresult, tuplen, 0))
			{
				var key = getresult(pgresult, tuplen, 0);
				if (this->length() <= 65535)
				{
					if (!this->locatebyusing("AR", FM_, key, keyn))
						this->inserter(keyn, key);
				}
				else
				{
					this->var_str.push_back(FM_);
					this->var_str.append(key.var_str);
				}
			}
		}
		this->setlasterror();
		return true;
	}

	// get filehandle specific connection or fail
	PGconn* thread_pgconn = (PGconn*)filehandle.connection();
	if (!thread_pgconn)
		return false;

	//$parameter array
	const char* paramValues[1];
	int paramLengths[1];
	// int		paramFormats[1];
	// uint32_t	binaryIntVal;

	// lower case key if reading from dictionary
	// std::string key2;
	// if (filehandle.substr(1,5).lcase()=="dict_")
	//	key2=key.lcase().toString();
	// else
	//	key2=key.toString();
	std::string key2 = key.normalize();

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0]=1;

	var sql = "SELECT data FROM " ^ filehandle.a(1).convert(".", "_") ^ " WHERE key = $1";

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
					  // TODO: parameterise filename
					  sql.toString().c_str(), 1, /* one param */
					  NULL, /* let the backend deduce param type */
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,
	// 1);	  /* ask for binary results */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK)
	{
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		var errmsg =
		    "read(" ^ filehandle.convert("." _FM_, "_^").quote() ^ ", " ^ key.quote() ^ ")";
		if (sqlstate == "42P01")
			errmsg ^= " File doesnt exist";
		else
			errmsg ^= var(PQerrorMessage(thread_pgconn)) ^ " sqlstate:" ^ sqlstate;
		;
		// PQclear(pgresult);
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		// return false;
	}

	if (PQntuples(pgresult) < 1)
	{
		// PQclear(pgresult);
		this->setlasterror("ERROR: mvdbpostgres read() record does not exist " ^
				   key.quote());
		return false;
	}

	if (PQntuples(pgresult) > 1)
	{
		// PQclear(pgresult);
		var errmsg = "ERROR: mvdbpostgres read() SELECT returned more than one record";
		errmsg.errputl();
		this->setlasterror(errmsg);
		return false;
	}

	*this = getresult(pgresult, 0, 0);

	this->setlasterror();

	return true;
}

var var::hash(const unsigned long long modulus) const
{
	THISIS("var var::hash() const")
	THISISDEFINED()
	THISISSTRING()
	// ISNUMERIC(modulus)

	// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed

	// not normalizing for speed
	// std::string tempstr=this->normalize();

	// uint64_t
	// hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);
	uint64_t hash64 =
	    MurmurHash64((char*)var_str.data(), int(var_str.length() * sizeof(char)), 0);
	if (modulus)
		return var_int = hash64 % modulus;
	else
		return var_int = hash64;
}

var var::lock(const var& key) const
{
	// on postgres, repeated locks for the same thing (from the same connection) succeed and
	// stack up they need the same number of unlocks (from the same connection) before other
	// connections can take the lock unlock returns true if a lock (your lock) was released and
	// false if you dont have the lock NB return "" if ALREADY locked on this connection

	THISIS("var var::lock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	std::string fileandkey = this->a(1).convert(".", "_").var_str;
	fileandkey.append(" ");
	fileandkey.append(key.normalize().var_str);

	// TODO .. provide endian identical version
	// required if and when exodus processes connect to postgres on a DIFFERENT host
	// although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the
	// SAME host
	uint64_t hash64 =
	    MurmurHash64((char*)fileandkey.data(), int(fileandkey.length() * sizeof(char)), 0);

	// check if already lock in current connection

	//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable*)this->get_lock_table();

	if (locktable)
	{
		// if already in local lock table then dont lock on database
		// since postgres stacks up multiple locks
		// whereas multivalue databases dont
		if (((*locktable).find(hash64)) != (*locktable).end())
			return "";
	}

	// parameter array
	const char* paramValues[1];
	int paramLengths[1];
	int paramFormats[1];

	//$1=advisory_lock
	paramValues[0] = (char*)&hash64;
	paramLengths[0] = sizeof(uint64_t);
	paramFormats[0] = 1; // binary

	const char* sql = "SELECT PG_TRY_ADVISORY_LOCK($1)";

	//"this" is a filehandle - get its connection
	PGconn* thread_pgconn = (PGconn*)this->connection();
	if (!thread_pgconn)
		return false;

	// DEBUG_LOG_SQL1
	if (GETDBTRACE)
		(var(sql) ^ " " ^ fileandkey).logputl("SQL:");

	PGresult* pgresult =
	    PQexecParams(thread_pgconn,
			 // TODO: parameterise filename
			 sql, 1, /* one param */
			 NULL,   /* let the backend deduce param type */
			 paramValues, paramLengths, paramFormats, 1); /* ask for binary pgresults */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1)
	{
		var errmsg = "lock(" ^ (*this) ^ ", " ^ key ^ ")\n" ^
			     var(PQerrorMessage(thread_pgconn)) ^ "\n" ^ "PQresultStatus=" ^
			     var(PQresultStatus(pgresult)) ^ ", PQntuples=" ^
			     var(PQntuples(pgresult));
		// PQclear(pgresult);//DO THIS OR SUFFER MEMORY LEAK
		errmsg.errputl();
		// throw MVException(msg);
		return false;
	}

	bool lockedok = *PQgetvalue(pgresult, 0, 0) != 0;

	// add it to the local lock table so we can detect double locking locally
	// since postgres will stack up repeated locks by the same process
	if (lockedok && locktable)
	{
		// register that it is locked
#ifdef USE_MAP_FOR_UNORDERED
		std::pair<const uint64_t, int> lock(hash64, 0);
		(*locktable).insert(lock);
#else
		(*locktable).insert(hash64);
#endif
	}

	return lockedok;
}

bool var::unlock(const var& key) const
{

	THISIS("void var::unlock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	std::string fileandkey = this->a(1).var_str;
	fileandkey.append(" ");
	fileandkey.append(key.normalize().var_str);

	// TODO .. provide endian identical version
	// required if and when exodus processes connect to postgres on a DIFFERENT host
	// although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the
	// SAME host
	uint64_t hash64 =
	    MurmurHash64((char*)fileandkey.data(), int(fileandkey.length() * sizeof(char)), 0);

	// remove from local current connection locktable
	//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable*)this->get_lock_table();
	if (locktable)
	{
		// if not in local locktable then no need to unlock on database
		if (((*locktable).find(hash64)) == (*locktable).end())
			return true;
		// register that it is unlocked
		(*locktable).erase(hash64);
	}

	// parameter array
	const char* paramValues[1];
	int paramLengths[1];
	int paramFormats[1];

	//$1=advisory_lock
	paramValues[0] = (char*)&hash64;
	paramLengths[0] = sizeof(uint64_t);
	paramFormats[0] = 1;

	const char* sql = "SELECT PG_ADVISORY_UNLOCK($1)";

	//"this" is a filehandle - get its connection
	PGconn* thread_pgconn = (PGconn*)this->connection();
	if (!thread_pgconn)
		return false;

	// DEBUG_LOG_SQL
	if (GETDBTRACE)
		(var(sql) ^ " " ^ fileandkey).logputl("SQL:");

	PGresult* pgresult =
	    PQexecParams(thread_pgconn,
			 // TODO: parameterise filename
			 sql, 1, /* one param */
			 NULL,   /* let the backend deduce param type */
			 paramValues, paramLengths, paramFormats, 1); /* ask for binary results */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1)
	{
		var errmsg = "unlock(" ^ (*this).convert(_FM_, "^") ^ ", " ^ key ^ ")\n" ^
			     var(PQerrorMessage(thread_pgconn)) ^ "\n" ^ "PQresultStatus=" ^
			     var(PQresultStatus(pgresult)) ^ ", PQntuples=" ^
			     var(PQntuples(pgresult));
		// PQclear(pgresult);//DO THIS OR SUFFER MEMORY LEAK
		errmsg.errputl();
		// throw MVException(msg);
		return false;
	}

	// bool unlockedok= *PQgetvalue(pgresult, 0, 0)!=0;

	// PQclear(pgresult);

	return true;
}

bool var::unlockall() const
{
	// THISIS("void var::unlockall() const")

	// check if any locks
	//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable*)this->get_lock_table();
	if (locktable)
	{
		// if local lock table is empty then dont unlock all database
		if ((*locktable).begin() == (*locktable).end())
			// TODO indicate in some global variable "OWN LOCK"
			return true;
		// register that it is locked
		(*locktable).clear();
	}

	return this->sqlexec("SELECT PG_ADVISORY_UNLOCK_ALL()");
}

// returns success or failure but no data
bool var::sqlexec(const var& sqlcmd, var& errmsg) const
{
	THISIS("bool var::sqlexec(const var& sqlcmd, var& errmsg) const")
	ISSTRING(sqlcmd)

	PGconn* thread_pgconn = (PGconn*)this->connection();
	if (!thread_pgconn)
	{
		errmsg = "Error: sqlexec cannot find thread database connection";
		return false;
	}

	// log the sql command
	if (GETDBTRACE)
	{
		//		exodus::logputl("SQL:" ^ *this);
		var temp("SQL:");
		if (this->assigned())
			temp ^= this->convert(_FM_, "^") ^ " ";
		temp ^= sqlcmd;
		temp.logputl();
	}

	// will contain any pgresult IF successful
	// MUST do PQclear(pgresult) after using it;

	// NB PQexec cannot be told to return binary results
	// but it can execute multiple commands
	// whereas PQexecParams is the opposite
	PGresult* pgresult = PQexec(thread_pgconn, sqlcmd.toString().c_str());

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK &&
	    PQresultStatus(pgresult) != PGRES_TUPLES_OK)
	{
		//int xx = PQresultStatus(pgresult);
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		// PQclear(pgresult);//essential
		// sql state 42P03 = duplicate_cursor
		errmsg = var(PQerrorMessage(thread_pgconn)) ^ " sqlstate:" ^ sqlstate;
		return false;
	}

	errmsg = var(PQntuples(pgresult));
	// PQclear(pgresult);//essential
	return true;
}

// writev writes a specific field number in a record
//(why it is "writev" instead of "writef" isnt known!
bool var::writev(const var& filehandle, const var& key, const int fieldno) const
{
	if (fieldno <= 0)
		return write(filehandle, key);

	THISIS("bool var::writev(const var& filehandle,const var& key,const int fieldno) const")
	// will be duplicated in read and write but do here to present the correct function name on
	// error
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// get the old record
	var record;
	if (!record.read(filehandle, key))
		record = "";

	// replace the field
	record.r(fieldno, var_str);

	// write it back
	record.write(filehandle, key);

	return true;
}

/* "prepared statement" version doesnt seem to make much difference approx -10% - possibly because
two field file is so simple bool var::write(const var& filehandle,const var& key) const {}
*/

//"update if present or insert if not" is handled in postgres using ON CONFLICT clause
bool var::write(const var& filehandle, const var& key) const
{
	THISIS("bool var::write(const var& filehandle, const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// asking to write DOS file! do osread!
	if (filehandle == "DOS")
	{
		this->oswrite(key); //.convert("\\",SLASH));
		return true;
	}

	// std::string key2=key.toString();
	// std::string data2=(*this).toString();
	std::string key2 = key.normalize();
	std::string data2 = (*this).normalize();

	// a 2 parameter array
	const char* paramValues[2];
	int paramLengths[2];
	// int		paramFormats[2];

	//$1 key
	// paramValues[0] = key2.data();
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0] = 1;//binary

	//$2 data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	// paramFormats[1] = 1;//binary

	var sql;

	// Note cannot use postgres PREPARE/EXECUTE with parameterised filename
	// but performance gain is probably not great since the sql we use to read and write is
	// quite simple (could PREPARE once per file/table)

	sql = "INSERT INTO " ^ filehandle.a(1).convert(".", "_") ^ " (key,data) values( $1 , $2)";
	sql ^= " ON CONFLICT (key)";
	sql ^= " DO UPDATE SET data = $2";

	PGconn* thread_pgconn = (PGconn*)filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
					  // TODO: parameterise filename
					  sql.toString().c_str(),
					  2,    // two params (key and data)
					  NULL, // let the backend deduce param type
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,
	// 1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#if TRACING >= 1
		var("ERROR: mvdbpostgres write(" ^ filehandle.convert(_FM_, "^") ^
				", " ^ key ^ ") failed: PQresultStatus=" ^
				var(PQresultStatus(pgresult)) ^ " " ^
				var(PQerrorMessage(thread_pgconn))).errputl();
#endif
		// PQclear(pgresult);
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0)
	{
		// PQclear(pgresult);
		return false;
	}

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int connid = filehandle.getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed");
	mv_connections_cache.writerecord(connid, filehandle.a(1).var_str, key.var_str, var_str);

	// PQclear(pgresult);
	return true;
}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var::updaterecord(const var& filehandle, const var& key) const
{
	THISIS("bool var::updaterecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// std::string key2=key.toString();
	// std::string data2=(*this).toString();
	std::string key2 = key.normalize();
	std::string data2 = (*this).normalize();

	// a 2 parameter array
	const char* paramValues[2];
	int paramLengths[2];
	// int		 paramFormats[2];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0] = 1;//binary

	//$2=data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	// paramFormats[1] = 1;//binary

	var sql = "UPDATE " ^ filehandle.a(1).convert(".", "_") ^ " SET data = $2 WHERE key = $1";

	PGconn* thread_pgconn = (PGconn*)filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
					  // TODO: parameterise filename
					  sql.toString().c_str(),
					  2,    // two params (key and data)
					  NULL, // let the backend deduce param type
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,	// bytea
	// 1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#if TRACING >= 1
		var("ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM_, "^") ^
				", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
				var(PQerrorMessage(thread_pgconn))).errputl();
#endif
		// PQclear(pgresult);
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0)
	{
#if TRACING >= 3
		var("ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM_, "^") ^
				", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
				var(PQerrorMessage(thread_pgconn))).errputl();
#endif
		// PQclear(pgresult);
		return false;
	}

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int connid = filehandle.getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed");
	mv_connections_cache.writerecord(connid, filehandle.a(1).var_str, key.var_str, var_str);

	// PQclear(pgresult);
	return true;
}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt
// already exist" logic

bool var::insertrecord(const var& filehandle, const var& key) const
{
	THISIS("bool var::insertrecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// std::string key2=key.toString();
	// std::string data2=(*this).toString();
	std::string key2 = key.normalize();
	std::string data2 = (*this).normalize();

	// a 2 parameter array
	const char* paramValues[2];
	int paramLengths[2];
	// int		 paramFormats[2];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0] = 1;//binary

	//$2=data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	// paramFormats[1] = 1;//binary

	var sql =
	    "INSERT INTO " ^ filehandle.a(1).convert(".", "_") ^ " (key,data) values( $1 , $2)";

	PGconn* thread_pgconn = (PGconn*)filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
					  // TODO: parameterise filename
					  sql.toString().c_str(),
					  2,    // two params (key and data)
					  NULL, // let the backend deduce param type
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,	// bytea
	// 1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#if TRACING >= 3
		var("ERROR: mvdbpostgres insertrecord(" ^
				filehandle.convert(_FM_, "^") ^ ", " ^ key ^ ") Failed: " ^
				var(PQntuples(pgresult)) ^ " " ^
				var(PQerrorMessage(thread_pgconn))).errputl();
#endif
		// PQclear(pgresult);
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0)
	{
		// PQclear(pgresult);
		return false;
	}

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int connid = filehandle.getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed");
	mv_connections_cache.writerecord(connid, filehandle.a(1).var_str, key.var_str, var_str);

	// PQclear(pgresult);
	return true;
}

bool var::deleterecord(const var& key) const
{
	THISIS("bool var::deleterecord(const var& key) const")
	THISISSTRING()
	ISSTRING(key)

	// std::string key2=key.toString();
	std::string key2 = key.normalize();

	// a one parameter array
	const char* paramValues[1];
	int paramLengths[1];
	// int		 paramFormats[1];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0] = 1;//binary

	var sql = "DELETE FROM " ^ this->a(1) ^ " WHERE KEY = $1";

	PGconn* thread_pgconn = (PGconn*)this->connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn, sql.toString().c_str(), 1, /* two param */
					  NULL, /* let the backend deduce param type */
					  paramValues, paramLengths,
					  0,  // text arguments
					  0); // text results
	// paramFormats,
	// 1);	  /* ask for binary results */
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#if TRACING >= 1
		var("ERROR: mvdbpostgres deleterecord(" ^ this->convert(_FM_, "^") ^
				", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
				var(PQerrorMessage(thread_pgconn))).errputl();
#endif
		// PQclear(pgresult);
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0)
	{
		// PQclear(pgresult);
#if TRACING >= 3
		var("var::deleterecord(" ^ this->convert(_FM_, "^") ^ ", " ^ key ^
				") failed. Record does not exist").errputl();
#endif
		return false;
	}

	// PQclear(pgresult);

	// update cache (set to "" for delete)
	// virtually identical code in read and write/update/insert/delete
	int connid = this->getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed");
	mv_connections_cache.writerecord(connid, this->a(1).var_str, key.var_str, "");

	return true;
}

void var::clearcache() const
{
	THISIS("bool var::clearcache() const")
	THISISDEFINED()

	int connid = this->getconnectionid_ordefault();
	if (!connid)
		throw MVDBException("getconnectionid() failed in clearcache");
	mv_connections_cache.clearrecordcache(connid);
	return;
}

// If this is opened SQL connection, pass connection ID to sqlexec
bool var::begintrans() const
{
	THISIS("bool var::begintrans() const")
	THISISDEFINED()

	this->clearcache();

	// begin a transaction
	return this->sqlexec("BEGIN");
}

bool var::rollbacktrans() const
{
	THISIS("bool var::rollbacktrans() const")
	THISISDEFINED()

	this->clearcache();

	// Rollback a transaction
	return this->sqlexec("ROLLBACK");
}

bool var::committrans() const
{
	THISIS("bool var::committrans() const")
	THISISDEFINED()

	this->clearcache();

	// end (commit) a transaction
	return this->sqlexec("END");
}

bool var::statustrans() const
{
	THISIS("bool var::statustrans() const")
	THISISDEFINED()

	PGconn* thread_pgconn = (PGconn*)this->connection();
	if (!thread_pgconn)
	{
		this->setlasterror("db connection " ^ var(this->getconnectionid()) ^ "not opened");
		return false;
	}
	this->setlasterror();

	// only idle is considered to be not in a transaction
	return (PQtransactionStatus(thread_pgconn) != PQTRANS_IDLE);
}

bool var::createdb(const var& dbname) const
{
	var errmsg;
	return createdb(dbname, errmsg);
}

bool var::deletedb(const var& dbname) const
{
	var errmsg;
	return deletedb(dbname, errmsg);
}

// sample code
// var().createdb("mynewdb");//create a new database on the current thread-default connection
// var file;
// file.open("myfile");
// file.createdb("mynewdb");//creates a new db on the same connection as a file was opened on
// var connectionhandle;
// connectionhandle.connect("connection string pars");
// connectionhandle.createdb("mynewdb");

bool var::createdb(const var& dbname, var& errmsg) const
{
	THISIS("bool var::createdb(const var& dbname, var& errmsg)")
	THISISDEFINED()
	ISSTRING(dbname)

	// var sql = "CREATE DATABASE "^dbname.convert(". ","__");
	var sql = "CREATE DATABASE " ^ dbname;
	sql ^= " WITH ENCODING='UTF8' ";
	// sql^=" OWNER=exodus";

	// TODO this shouldnt only be for default connection
	return this->sqlexec(sql, errmsg);
}

bool var::deletedb(const var& dbname, var& errmsg) const
{
	THISIS("bool var::deletedb(const var& dbname, var& errmsg)")
	THISISDEFINED()
	ISSTRING(dbname)

	return this->sqlexec("DROP DATABASE " ^ dbname, errmsg);
}

bool var::createfile(const var& filename) const
{
	THISIS("bool var::createfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	// var tablename = "TEMP" ^ var(100000000).rnd();
	// Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but
	// has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default
	// behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON
	// COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	var sql = "CREATE";
	// if (options.ucase().index("TEMPORARY")) sql ^= " TEMPORARY";
	// sql ^= " TABLE " ^ filename.convert(".","_");
	if (filename.substr(-5, 5) == "_temp")
		sql ^= " TEMP ";
	sql ^= " TABLE " ^ filename;
	// sql ^= " (key bytea primary key, data bytea)";
	sql ^= " (key text primary key, data text)";

	return this->sqlexec(sql);
}

bool var::renamefile(const var& filename, const var& newfilename) const
{
	THISIS("bool var::renamefile(const var& filename, const var& newfilename)")
	THISISDEFINED()
	ISSTRING(filename)
	ISSTRING(newfilename)

	return this->sqlexec("ALTER TABLE " ^ filename ^ " RENAME TO " ^ newfilename);
}

bool var::deletefile(const var& filename) const
{
	THISIS("bool var::deletefile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	return this->sqlexec("DROP TABLE " ^ filename);
}

bool var::clearfile(const var& filename) const
{
	THISIS("bool var::clearfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	return this->sqlexec("DELETE FROM " ^ filename);
}

inline void unquoter_inline(var& string)
{
	// remove "", '' and {}
	static var quotecharacters("\"'{");
	if (quotecharacters.index(string[1]))
		string = string.substr(2, string.length() - 2);
}

/*
inline void tosqldate(var &datestr, const var &dateformat)
{
	// TODO! create global date format accessible from select
	var idate = datestr.iconv("D/E");
	if (idate)
		datestr = idate.oconv("DJ");
	else
		throw MVException(datestr ^ " cannot be recognised as a date");
}
*/

inline void tosqlstring(var& string1)
{
	// convert to sql style strings
	// use single quotes and double up any internal single quotes
	if (string1[1] == "\"")
	{
		string1.swapper("'", "''");
		string1.splicer(1, 1, "'");
		string1.splicer(-1, 1, "'");
	}
}

inline var fileexpression(const var& mainfilename, const var& filename, const var& keyordata)
{
	// evade warning: unused parameter mainfilename
	if (false && mainfilename)
	{
	}

	// if (filename == mainfilename)
	//	return keyordata;
	// else
	return filename ^ "." ^ keyordata;

	// if you dont use STRICT in the postgres function declaration/definitions then NULL
	// parameters do not abort functions

	// use COALESCE function in case this is a joined but missing record (and therefore null)
	// in MYSQL this is the ISNULL expression?
	// xlatekeyexpression="exodus_extract_text(coalesce(" ^ filename ^ ".data,''::text), " ^
	// xlatefromfieldname.substr(9); if (filename==mainfilename) return expression; return
	// "coalesce(" ^ expression ^", ''::text)";
}

var var::getdictexpression(const var& mainfilename, const var& filename, const var& dictfilename,
			   const var& dictfile, const var& fieldname0, var& joins, var& unnests,
			   var& selects, var& ismv, bool forsort) const
{

	ismv=false;

	var fieldname = fieldname0.convert(".", "_");
	var actualdictfile = dictfile;
	if (!actualdictfile)
	{
		var dictfilename;
		if (mainfilename.substr(1, 5).lcase() == "dict_")
			dictfilename = "dict_voc";
		else
			dictfilename = "dict_" ^ mainfilename;

		// we should open it through the same connection, as this->was opened, not any
		// default connection
		// int connid = 0;
		// if (THIS_IS_DBCONN())
		//	connid = (int) var_int;
		// initialise the actualdictfilename to the same connection as (*this)
		actualdictfile = (*this);
		if (!actualdictfile.open(dictfilename))
		{
			dictfilename = "dict_voc";
			if (!actualdictfile.open(dictfilename))
			{

				throw MVDBException("getdictexpression() cannot open " ^
						    dictfilename.quote());
#if TRACING >= 1
				var(
				    "ERROR: mvdbpostgres getdictexpression() cannot open " ^
				    dictfilename.quote()).errputl();
#endif
				return "";
			}
		}
	}

	//if doing 2nd pass then calculated fields have been placed in a parallel temporary file
	//and their column names appended with a colon (:)
	var calculated=fieldname[-1]==":";
	if (calculated) {
		fieldname.splicer(-1,1,"");
		//create a pseudo look up ... except that SELECT_TEMP_CURSOR_n has the fields stored in sql columns and not in the usual data column
		calculated="@ANS=XLATE(\"SELECT_STAGE2_CURSOR_" ^ this->a(1) ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
	}

	// given a file and dictionary id
	// returns a postgres sql expression like (texta(filename.data,99,0,0))
	// using one of the neosys backend functions installed in postgres like textextract,
	// dateextract etc.
	var dictrec;
	if (!dictrec.read(actualdictfile, fieldname))
	{
		// try lowercase
		fieldname.lcaser();
		if (!dictrec.read(actualdictfile, fieldname))
		{
			// try uppercase
			fieldname.ucaser();
			if (!dictrec.read(actualdictfile, fieldname))
			{
				// try in voc lowercase
				fieldname.lcaser();
				if (not dictrec.read("dict_voc", fieldname))
				{
					// try in voc uppercase
					fieldname.ucaser();
					if (not dictrec.read("dict_voc", fieldname))
					{
						if (fieldname == "@ID" || fieldname == "ID")
							dictrec = "F" ^ FM ^ "0" ^ FM ^ "Ref" ^ FM ^
								  FM ^ FM ^ FM ^ FM ^ FM ^ "" ^ FM ^
								  15;
						else
						{
							throw MVDBException(
							    "getdictexpression() cannot read " ^
							    fieldname.quote() ^ " from " ^
							    actualdictfile.convert(FM, "^")
								.quote() ^
							    " or \"dict_voc\"");
							//					exodus::errputl("ERROR:
							// mvdbpostgres getdictexpression() cannot
							// read " ^ fieldname.quote() ^ " from " ^
							// actualdictfile.quote());
							return "";
						}
					}
				}
			}
		}
	}

	//create a pseudo look up. to trigger JOIN logic to the table that we stored
	//Note that SELECT_TEMP has the fields stored in sql columns and not in the usual data column
	if (calculated) {
		dictrec.r(8,calculated);
	}

	var dicttype = dictrec.a(1);
	var fieldno = dictrec.a(2);
	var conversion = dictrec.a(7);

	var isinteger = conversion == "[NUMBER,0]" || dictrec.a(11) == "0N" ||
			dictrec.a(11).substr(1, 3) == "0N_";
	var isdecimal = conversion.substr(1, 2) == "MD" || conversion.substr(1, 7) == "[NUMBER" ||
			dictrec.a(12) == "FLOAT" || dictrec.a(11).index("0N");
			//dont assume things that are R are numeric
			//eg period 1/19 is right justified but not numeric and sql select will crash if ::float8 is used
			//||dictrec.a(9) == "R";
	var isnumeric = isinteger || isdecimal;
	var ismv1 = dictrec.a(4)[1] == "M";
	var fromjoin = false;

	var sqlexpression;
	if (dicttype == "F")
	{
		// key field
		if (!fieldno)
		{

			var isdate = conversion[1] == "D" || conversion.substr(1, 5) == "[DATE";

			if (forsort && !isdate)
				// sqlexpression="exodus_extract_sort(" ^
				// fileexpression(mainfilename, filename,"key") ^ ")";
				sqlexpression =
				    "exodus_extract_sort(" ^ mainfilename ^ ".key,0,0,0)";
			else
				// sqlexpression="convert_from(" ^ fileexpression(mainfilename,
				// filename, "key") ^ ", 'UTF8')";
				sqlexpression = fileexpression(mainfilename, filename, "key");

			// multipart key
			var keypartn = dictrec.a(5);
			if (keypartn)
			{
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
				    "exodus_extract_date(" ^ sqlexpression ^ ",0,0,0)";

			return sqlexpression;
		}

		var extractargs =
		    fileexpression(mainfilename, filename, "data") ^ "," ^ fieldno ^ ", 0, 0)";

		if (conversion.substr(1, 9) == "[DATETIME")
			sqlexpression = "exodus_extract_datetime(" ^ extractargs;

		else if (conversion[1] == "D" || conversion.substr(1, 5) == "[DATE")
			sqlexpression = "exodus_extract_date(" ^ extractargs;

		else if (conversion.substr(1, 2) == "MT" || conversion.substr(1, 5) == "[TIME")
			sqlexpression = "exodus_extract_time(" ^ extractargs;

		// for now (until we have a extract_number/integer/float) that doesnt fail on
		// non-numeric like cast "as integer" and "as float" does note that we could use
		// exodus_extract_sort for EVERYTHING inc dates/time/numbers etc. but its large size
		// is perhaps a disadvantage
		else if (forsort)
			sqlexpression = "exodus_extract_sort(" ^ extractargs;

		else if (isnumeric)
			sqlexpression = "exodus_extract_number(" ^ extractargs;

		else
			sqlexpression = "exodus_extract_text(" ^ extractargs;
	}
	else if (dicttype == "S")
	{

		var functionx = dictrec.a(8).trim();

		// sql expression available
		sqlexpression = dictrec.a(17);
		if (sqlexpression)
		{
			// return sqlexpression;
		}

		// sql function available
		// eg dict_schedules_PROGRAM_POSITION(key text, data text)
		else if (functionx.index("/" "*pgsql"))
		{

			// plsql function name assumed to be like "dictfilename_FIELDNAME()"
			sqlexpression = actualdictfile.a(1) ^ "_" ^ fieldname ^ "(";

			// function arguments are (key,data)
			sqlexpression ^= fileexpression(mainfilename, filename, "key");
			sqlexpression ^= ", ";
			sqlexpression ^= fileexpression(mainfilename, filename, "data");
			sqlexpression ^= ")";
			// sqlexpression^="::bytea";

			// return sqlexpression;
		}

		// simple join
		else if (! ismv1 && functionx.substr(1, 11).ucase() == "@ANS=XLATE(")
		{
			functionx = functionx.a(1, 1);

			functionx.splicer(1, 11, "");

			// allow for <1,@mv> in arg3 by replacing comma with |
			functionx.swapper(",@mv", "|@mv");

			//allow for field(@id,'*',x) in arg2 by replacing commas with |
			functionx.swapper(",'*',", "|'*'|");

			// arg1 filename
			var xlatetargetfilename = functionx.field(",", 1).trim().convert(".", "_");
			unquoter_inline(xlatetargetfilename);

			// arg2 key
			var xlatefromfieldname = functionx.field(",", 2).trim();

			// arg3 target field number/name
			var xlatetargetfieldname = functionx.field(",", 3).trim().unquoter();

			// arg4 mode X or C
			var xlatemode = functionx.field(",", 4).trim().convert("'\" )", "");

			// if the fourth field is 'X', "X", "C" or 'C' then
			// assume we have a good simple xlate functionx and can convert to a JOIN
			if (xlatemode == "X" || xlatemode == "C")
			{

				// determine the expression in the xlate target file
				// var& todictexpression=sqlexpression;
				if (xlatetargetfieldname.isnum())
				{
					sqlexpression =
					    "exodus_extract_text(" ^
					    fileexpression(mainfilename, xlatetargetfilename,
							   "data") ^
					    ", " ^ xlatetargetfieldname ^ ", 0, 0)";
				}
				//calculated fields exist as sql columns in a parallel temporary table
				else if (calculated)
				{
					sqlexpression=xlatetargetfilename ^ "." ^ xlatetargetfieldname;
				}
				else
				{
					// var dictxlatetofile=xlatetargetfilename;
					// if (!dictxlatetofile.open("DICT",xlatetargetfilename))
					//	throw MVDBException("getdictexpression() DICT" ^
					// xlatetargetfilename ^ " file cannot be opened"); var
					// ismv;
					var xlatetargetdictfilename="dict_"^xlatetargetfilename;
					var xlatetargetdictfile;
					if (! xlatetargetdictfile.open(xlatetargetdictfilename))
						throw MVDBException(xlatetargetdictfilename ^ " cannot be opened for " ^ functionx);
					sqlexpression = getdictexpression(
					    filename, xlatetargetfilename, xlatetargetdictfilename,
					    xlatetargetdictfile, xlatetargetfieldname, joins, unnests,
					    selects, ismv, forsort);
				}

				// determine the join details
				var xlatekeyexpression;
				//xlatefromfieldname.outputl("xlatefromfieldname=");
				if (xlatefromfieldname.trim().substr(1, 8).lcase() == "@record<")
				{
					xlatekeyexpression = "exodus_extract_text(";
					xlatekeyexpression ^= filename ^ ".data";
					xlatekeyexpression ^= ", " ^ xlatefromfieldname.substr(9);
					xlatekeyexpression.splicer(-1, 1, "");
					xlatekeyexpression ^=
					    var(", 0").str(3 - xlatekeyexpression.count(',')) ^ ")";
				}
				else if (xlatefromfieldname.trim().substr(1, 10).lcase() == "field(@id|")
				{
					xlatekeyexpression = "split_part(";
					xlatekeyexpression ^= filename ^ ".key,'*',";
					xlatekeyexpression ^= xlatefromfieldname.field("|",3).field(")",1) ^ ")";
				}
				// TODO				if
				// (xlatefromfieldname.substr(1,8)=="FIELD(@ID)
				else if (xlatefromfieldname[1] == "{")
				{
					xlatefromfieldname =
					    xlatefromfieldname.substr(2).splicer(-1, 1, "");
					xlatekeyexpression = getdictexpression(
					    filename, filename, dictfilename, dictfile,
					    xlatefromfieldname, joins, unnests, selects, ismv);
				}
				else if (xlatefromfieldname == "@ID")
				{
					xlatekeyexpression = filename ^ ".key";
				}
				else
				{
					// throw  MVDBException("getdictexpression() " ^
					// filename.quote() ^ " " ^ fieldname.quote() ^ " - INVALID
					// DICTIONARY EXPRESSION - " ^ dictrec.a(8).quote());
#if TRACING >= 1
					var("ERROR: mvdbpostgres getdictexpression() " ^
							filename.quote() ^ " " ^ fieldname.quote() ^
							" - INVALID DICTIONARY EXPRESSION - " ^
							dictrec.a(8).quote()).errputl();
#endif
					return "";
				}

				//if the xlate key expression is calculated then
				//indicate that the whole dictid expression is calculated
				//and do not do any join
				if (xlatekeyexpression.index("exodus_call"))
				{
					sqlexpression="exodus_call(";
					return sqlexpression;
				}

				fromjoin = true;

				// joins needs to follow "FROM mainfilename" clause
				// except for joins based on mv fields which need to follow the
				// unnest function
				var joinsectionn = ismv ? 2 : 1;

				// add the join
				var join_part1 = "\n  ";
				//main file is on the left
				//secondary file is on the right
				//normally we want all records on the left (main file) and any secondary file records that exist ... LEFT JOIN
				//if joining to calculated field file then we want only records that exist in the calculated fields file ... RIGHT JOIN (could be INNER JOIN)
				join_part1 ^= calculated?"RIGHT":"LEFT";
				join_part1 ^= " JOIN\n   " ^ xlatetargetfilename ^ " ON ";
				// var join_part2=xlatekeyexpression ^ "::text = " ^
				// xlatetargetfilename ^ ".key"; var join_part2=xlatetargetfilename
				// ^
				// ".key = " ^ xlatekeyexpression ^ "::bytea";
				var join_part2 =
				    xlatetargetfilename ^ ".key = " ^ xlatekeyexpression;
				// only allow one join per file for now.
				// TODO allow multiple joins to the same file via different keys
				if (!joins.a(joinsectionn).index(join_part1))
					joins.r(joinsectionn, -1, join_part1 ^ join_part2);

				return sqlexpression;
			}
			else
			{
				// not xlate X or C
				goto exodus_call;
			}
		}

		// FOLLOWING IS CURRENTLY DISABLED
		// if we get here then we were unable to work out any sql expression or function
		// so originally we instructed postgres to CALL EXODUS VIA IPC to run exodus
		// subroutines in the context of the calling program. exodus mvdbpostgres.cpp setup
		// a separate listening thread with a separate pgconnection before calling postgres.
		// BUT exodus subroutines cannot make request to the db while it is handling a
		// request FROM the db - unless it were to setup ANOTHER threada and pgconnection to
		// handle it. this is also perhaps SLOW since it has to copy the whole RECORD and ID
		// etc to exodus via IPC for every call!
		else
		{
		exodus_call:
			sqlexpression = "'" ^ fieldname ^ "'";
			int environmentn = 1; // getenvironmentn()
			// sqlexpression="exodus_call('exodusservice-" ^ getprocessn() ^ "." ^
			// environmentn ^ "'::bytea, '" ^ dictfilename.lcase() ^ "'::bytea, '" ^
			// fieldname.lcase() ^ "'::bytea, "^ filename ^ ".key, " ^ filename ^
			// ".data,0,0)";
			sqlexpression = "exodus_call('exodusservice-" ^ getprocessn() ^ "." ^
					environmentn ^ "', '" ^ dictfilename.lcase() ^ "', '" ^
					fieldname.lcase() ^ "', " ^ filename ^ ".key, " ^ filename ^
					".data,0,0)";
			//sqlexpression.outputl("sqlexpression=");
			// TODO apply naturalorder conversion by passing forsort
			// option to exodus_call

			return sqlexpression;
		}
	}
	else
	{
		// throw  filename ^ " " ^ fieldname ^ " - INVALID DICTIONARY ITEM";
		// throw  MVDBException("getdictexpression(" ^ filename.quote() ^ ", " ^
		// fieldname.quote() ^ ") invalid dictionary type " ^ dicttype.quote());
#if TRACING >= 1
		var("ERROR: mvdbpostgres getdictexpression(" ^ filename.quote() ^ ", " ^
				fieldname.quote() ^ ") invalid dictionary type " ^
				dicttype.quote()).errputl();
#endif
		return "";
	}

	if (fieldname.substr(-5).ucase() == "_XREF")
	{
		//sqlexpression = "to_tsvector('english'," ^ sqlexpression ^ ")";
		//attempt to ensure numbers are indexed too
		// but it prevents matching similar words
		//use "english" dictionary for stemming (or "simple" dictionary for none)
		// MUST use the SAME in both to_tsvector AND to_tsquery
		//https://www.postgresql.org/docs/10/textsearch-dictionaries.html
		//sqlexpression = "to_tsvector('simple'," ^ sqlexpression ^ ")";
		sqlexpression = "to_tsvector('english'," ^ sqlexpression ^ ")";
		//sqlexpression = "string_to_array(" ^ sqlexpression ^ ",chr(29),'')";

	// unnest multivalued fields into multiple output rows
	} else if (ismv1)
	{

		ismv = true;

		// var from="string_to_array(" ^ sqlexpression ^ ",'" ^ VM ^ "'";
		if (sqlexpression.substr(1,20)=="exodus_extract_date(" || sqlexpression.substr(1,20)=="exodus_extract_time(")
			sqlexpression.splicer(20,0,"_array");
		else
			sqlexpression = "string_to_array(" ^ sqlexpression ^ ", chr(29),'')";

		// Note 3rd argument '' means convert empty multivalues to NULL in the array
		// otherwise conversion to float will fail
		if (isnumeric)
			sqlexpression ^= "::float8[]";

		if (forsort)
		{

			// use the fieldname as a sql column name

			// convert multivalues to array
			// if (from.substr(-7)=="::bytea")
			//	from.splicer(-7,7,"");

			// fieldname.outputl("fieldname=");
			// filename.outputl("filename=");
			// mainfilename.outputl("mainfilename=");

			// if a mv field requires a join then add it to the SELECT clause
			// since not known currently how to to do mv joins in the FROM clause
			// Note the join clause should already have been added to the JOINS for the FROM
			// clause
			if (fromjoin)
			{

				// this code should never execute as joined mv fields now return the plain
				// sql expression. we assume they are used in WHERE and ORDER BY clauses

				// unnest in select clause
				// from="unnest(" ^ from ^ ")";
				// as FIELDNAME
				sqlexpression ^= " as " ^ fieldname;

				// dont include more than once, in case order by and filter on the same
				// field
				if (!selects.a(1).index(sqlexpression))
					selects ^= ", " ^ sqlexpression;
			}
			else
			{

				// insert with SMs since expression can contain VMs
				if (!unnests.a(2).locate(fieldname))
				{
					unnests.r(2, -1, fieldname);
					unnests.r(3, -1, sqlexpression);
				}
			}

			sqlexpression = fieldname;
		}
	}

	return sqlexpression;
}

// var getword(var& remainingwords, const var& joinvalues=false)
var getword(var& remainingwords, var& ucword)
{

	// gets the next word
	// or a series of words separated by FM while they are numbers or quoted strings)
	// converts to sql quoted strings
	// and clips them from the input string

	bool joinvalues = true;

	var word1 = remainingwords.field(" ", 1);
	remainingwords = remainingwords.field(" ", 2, 99999);

	// join quoted words together
	var char1 = word1[1];
	if ((char1 == DQ || char1 == SQ))
	{
		while (word1[-1] != char1)
		{
			if (remainingwords.length())
			{
				word1 ^= " " ^ remainingwords.field(" ", 1);
				remainingwords = remainingwords.field(" ", 2, 99999);
			}
			else
			{
				word1 ^= char1;
				break;
			}
		}
	}

	tosqlstring(word1);

	// grab multiple values (numbers or quoted words) into one list, separated by FM
	//value chars are " ' 0-9 . + -
	if (joinvalues && valuechars.index(word1[1]))
	{
		word1 = SQ ^ word1.unquote().swap("'","''") ^ SQ;

		var nextword = remainingwords.field(" ", 1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		// duplicated above/below
		if (nextword == "and")
		{
			var nextword2 = remainingwords;
			if (valuechars.index(nextword2[1]))
			{
				nextword = nextword2;
				remainingwords = remainingwords.field(" ", 2, 99999);
			}
		}

		while (nextword && valuechars.index(nextword[1]))
		{
			tosqlstring(nextword);
			if (word1 != "")
				word1 ^= FM_;
			word1 ^= SQ ^ nextword.unquote() ^ SQ;

			remainingwords = remainingwords.field(" ", 2, 99999);
			nextword = remainingwords.field(" ", 1);

			//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
			// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
			// duplicated above/below
			if (nextword == "and")
			{
				var nextword2 = remainingwords;
				if (valuechars.index(nextword2[1]))
				{
					nextword = nextword2;
					remainingwords = remainingwords.field(" ", 2, 99999);
				}
			}
		}
	}
	else
	{
		// word1.ucaser();
	}

	ucword = word1.ucase();
	return word1;
}

bool var::saveselect(const var& filename)
{
	THISIS("bool var::saveselect(const var& filename) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(filename)

	if (GETDBTRACE)
		var("DBTRACE: ::saveselect(" ^ filename ^ ")").logputl();

	int recn = 0;
	var key;
	var mv;

	// save preselected keys into a file to be used with INNERJOIN on select()

	// this should not throw if the select does not exist
	this->deletefile(filename);

	// clear or create any existing saveselect file with the same name
	if (!this->createfile(filename))
		throw MVDBException("saveselect cannot create file " ^ filename);
	var file;
	if (!file.open(filename, (*this)))
		throw MVDBException("saveselect cannot open file " ^ filename);

	while (this->readnext(key, mv))
	{
		recn++;

		// save a key
		(mv ^ FM ^ recn).write(file, key);
	}

	return recn > 0;
}

bool var::selectrecord(const var& sortselectclause)
{
	THISIS("bool var::selectrecord(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(sortselectclause)

	// return const_cast<const var&>(*this).selectx("key, mv::integer, data",sortselectclause);
	return this->selectx("key, mv::integer, data", sortselectclause);
}

bool var::select(const var& sortselectclause)
{
	THISIS("bool var::select(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(sortselectclause)

	return this->selectx("key, mv::integer", sortselectclause);
}

// currently only called from select, selectrecord and getlist
bool var::selectx(const var& fieldnames, const var& sortselectclause)
{
	// private - and arguments are left unchecked for speed
	//?allow undefined usage like var xyz=xyz.select();
	if (var_typ & VARTYP_MASK)
	{
		// throw MVUndefined("selectx()");
		var_str = "";
		var_typ = VARTYP_STR;
	}

	// fieldnames.outputl("fieldnames=");
	// sortselectclause.outputl("sortselectclause=");

	// default to ""
	if (!(var_typ & VARTYP_STR))
	{
		if (!var_typ)
		{
			var_str = "";
			var_typ = VARTYP_STR;
		}
		else
			this->createString();
	}

	if (GETDBTRACE)
		sortselectclause.logputl("sortselectclause=");

	var actualfilename = this->a(1).convert(".","_");
	// actualfilename.outputl("actualfilename=");
	var dictfilename = actualfilename;
	var actualfieldnames = fieldnames;
	var dictfile = "";
	var keycodes = "";
	bool bykey = false;
	var wordn;
	var distinctfieldnames = "";

	var whereclause = "";
	bool orwith=false;
	var orderclause = "";
	var joins = "";
	var unnests = "";
	var selects = "";
	var ismv = false;

	var maxnrecs = "";
	var xx; // throwaway return value

	//prepare to save calculated fields that cannot be calculated by postgresql for secondary processing
	var calc_fields="";
	//var ncalc_fields=0;
	this->r(10,"");

	// sortselect clause can be a filehandle in which case we extract the filename from field1
	// omitted if filename.select() or filehandle.select()
	// cursor.select(...) where ...
	// SELECT (or SSELECT) nnn filename with .... and with ... by ... by
	// filename can be omitted if calling like filename.select(...) or filehandle.select(...)
	// nnn is optional limit to number of records returned
	// TODO only convert \t\r\n outside single and double quotes
	var remaining = sortselectclause.a(1).convert("\t\r\n", "   ").trim();
	// remaining.outputl("remaining=");

	// remove trailing options eg (S) or {S}
	var lastword = remaining.field2(" ", -1);
	if ((lastword[1] == "(" && lastword[-1] == ")") ||
	    (lastword[1] == "{" && lastword[-1] == "}"))
	{
		remaining.splicer(-lastword.length() - 1, 999, "");
	}

	var firstucword = remaining.field(" ", 1).ucase();

	// sortselectclause may start with {SELECT|SSELECT {maxnrecs} filename}
	if (firstucword == "SELECT" || firstucword == "SSELECT")
	{
		if (firstucword == "SSELECT")
			bykey = true;

		// remove it
		var xx = getword(remaining, xx);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// the second word can be a number to limit the number of records selected
	if (firstucword.length() and firstucword.isnum())
	{
		maxnrecs = firstucword;

		// remove it
		var xx = getword(remaining, xx);

		firstucword = remaining.field(" ", 1).ucase();
	}

	// the next word can be the filename if not one of the select clause words
	// override any filename in the cursor variable
	if (firstucword && not var("BY BY-DSND WITH WITHOUT ( { USING DISTINCT").locateusing(" ", firstucword))
	{
		actualfilename = firstucword;
		dictfilename = actualfilename;
		// remove it
		var xx = getword(remaining, firstucword);
	}

	// actualfilename.outputl("actualfilename=");
	if (!actualfilename)
	{
		// this->outputl("this=");
		throw MVDBException("filename missing from select statement:" ^ sortselectclause);
	}

	while (remaining.length())
	{

		// remaining.outputl("remaining=");
		// whereclause.outputl("whereclause=");
		// orderclause.outputl("orderclause=");

		var ucword;
		var word1 = getword(remaining, ucword);

		// skip options (last word and surrounded by brackets)
		// (S) etc
		// options - last word enclosed in () or {}
		if (!remaining.length()
		    &&
		    (
			(word1[1] == "(" && word1[-1] == ")")
			||
			(word1[1] == "{" && word1[-1] == "}")
		    )
		   )
		{
			// word1.outputl("skipping last word in () options ");
			continue;
		}

		// numbers or strings without leading clauses like with ... mean record keys
		//value chars are " ' 0-9 . + -
		else if (valuechars.index(word1[1]))
		{
			if (keycodes)
				keycodes ^= FM;
			keycodes ^= word1;
			continue;
		}

		// using filename
		else if (ucword == "USING" && remaining)
		{
			dictfilename = getword(remaining, xx);
			if (!dictfile.open("dict_" ^ dictfilename))
			{
				throw MVDBException("select() dict_" ^ dictfilename ^
						    " file cannot be opened");
				// exodus::errputl("ERROR: mvdbpostgres select() dict_" ^
				// dictfilename ^ " file cannot be opened"); return "";
			}
			continue;
		}

		// distinct fieldname (returns a field instead of the key)
		else if (ucword == "DISTINCT" && remaining)
		{

			var distinctfieldname = getword(remaining, xx);
			var distinctexpression = getdictexpression(
			    actualfilename, actualfilename, dictfilename, dictfile,
			    distinctfieldname, joins, unnests, selects, ismv, false);
			var naturalsort_distinctexpression = getdictexpression(
			    actualfilename, actualfilename, dictfilename, dictfile,
			    distinctfieldname, joins, unnests, selects, ismv, true);

			if (true)
			{
				// this produces the right values but in random order
				// it use any index on the distinct field so it works on large
				// indexed files select distinct is really only useful on INDEXED
				// fields unless the file is small
				distinctfieldnames = "DISTINCT " ^ distinctexpression;
			}
			else
			{
				// this produces the right results in the right order
				// BUT DOES IS USE INDEXES AND ACT VERY FAST??
				distinctfieldnames = "DISTINCT ON (" ^
						     naturalsort_distinctexpression ^ ") " ^
						     distinctexpression;
				orderclause ^= ", " ^ naturalsort_distinctexpression;
			}
			continue;
		}

		// by or by-dsnd
		else if (ucword == "BY" || ucword == "BY-DSND")
		{
			// next word must be dictid
			var dictid = getword(remaining, xx);
			var dictexpression =
			    getdictexpression(actualfilename, actualfilename, dictfilename,
					      dictfile, dictid, joins, unnests, selects, ismv, true);

			// dictexpression.outputl("dictexpression=");
			// orderclause.outputl("orderclause=");

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.index("exodus_call"))
			//if (dictexpression == "true")
			{
				if (! calc_fields.a(1).locate(dictid))
				{
					//++ncalc_fields;
					calc_fields.r(1,-1,dictid);
				}
				continue;
			}

			orderclause ^= ",\n " ^ dictexpression;

			if (ucword == "BY-DSND")
				orderclause ^= " DESC";

			continue;
		}

		// subexpression combination
		else if (ucword == "AND" || ucword == "OR")
		{
			// dont start with AND or OR
			if (whereclause)
				whereclause ^= "\n " ^ ucword;
			if (ucword=="OR")
				orwith=true;
			continue;
		}

		// subexpression grouping
		else if (ucword == "(" || ucword == ")")
		{
			whereclause ^= "\n " ^ ucword;
			continue;
		}

		// with dictid eq/starting/ending/containing/like 1 2 3
		// with dictid 1 2 3
		// with dictid between x and y
		else if (ucword == "WITH" || ucword == "WITHOUT")
		{

			var negative = ucword == "WITHOUT";

			// next word must be the NOT/NO or the dictionary id
			word1 = getword(remaining, ucword);

			// can negate before (and after) dictionary word
			// eg WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" || ucword == "NO")
			{
				negative = !negative;
				// word1=getword(remaining,true);
				// remove NOT or NO
				word1 = getword(remaining, ucword);
			}

			// skip AUTHORISED for now since too complicated to calculate in database
			// ATM if (word1.ucase()=="AUTHORISED") { 	if
			//(whereclause.substr(-4,4).ucase() == " AND")
			//whereclause.splicer(-4,4,""); 	continue;
			//}

			// process the dictionary id
			var forsort =
			    false; // because indexes are NOT created sortable (exodus_sort()
			var dictexpression =
			    getdictexpression(actualfilename, actualfilename, dictfilename,
					      dictfile, word1, joins, unnests, selects, ismv, forsort);
			var usingnaturalorder = dictexpression.index("exodus_extract_sort");
			var dictid = word1;

			//var dictexpression_isarray=dictexpression.index("string_to_array(");
			var dictexpression_isarray=dictexpression.index("_array(");
			var dictexpression_isxref=dictexpression.index("to_tsvector(");

			// add the dictid expression
			//if (dictexpression.index("exodus_call"))
			//	dictexpression = "true";

			//whereclause ^= " " ^ dictexpression;

			// the words after the dictid can be NOT/NO or values
			// word1=getword(remaining, true);
			word1 = getword(remaining, ucword);

			// can negate before (and after) dictionary word
			// eg WITH NOT/NO INVOICE_NO or WITH INVOICE_NO NOT
			if (ucword == "NOT" || ucword == "NO")
			{
				negative = !negative;
				// word1=getword(remaining,true);
				// remove NOT/NO and acquire any values
				word1 = getword(remaining, ucword);
			}

			//if (ucword=="@ID")
			//	std::cout << "@ID";

			// between x and y
			// from x to y
			/////////////////

			if (ucword == "BETWEEN" || ucword == "FROM")
			{

				//prevent BETWEEN being used on fields
				if (dictexpression_isxref)
				{
					throw MVDBException(
					    sortselectclause ^
					    "BETWEEN x AND y/FROM x TO y ... is not currently supported for XREF");
				}

				// get and append first value
				word1 = getword(remaining, ucword);

				// get and append second value
				var word2 = getword(remaining, xx);

				// discard any optional intermediate "AND"
				if (word2.ucase() == "AND" || word2.ucase() == "TO")
				{
					word2 = getword(remaining, xx);
				}

				// check we have two values (in word1 and word2)
				if (!valuechars.index(word1[1]) || !valuechars.index(word2[1]))
				{
					throw MVDBException(
					    sortselectclause ^
					    "BETWEEN x AND y/FROM x TO y must be followed by two values (x AND/TO y)");
				}

				if (usingnaturalorder)
				{
					word1 = naturalorder(word1.toString());
					word2 = naturalorder(word2.toString());
				}

				// no filtering in database on calculated items
				//save then for secondary filtering
				if (dictexpression.index("exodus_call"))
				{
					var opid=negative?">!<":"><";

					//almost identical code for exodus_call above/below
					var calc_fieldn;
					if (! calc_fields.locate(dictid,calc_fieldn,1))
					{
						//++ncalc_fields;
						calc_fields.r(1,calc_fieldn,dictid);
					}

					//prevent WITH XXX appearing twice in the same sort/select clause
					//unless and until implementeda
					if (calc_fields.a(2,calc_fieldn))
						throw MVDBException("WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote());

					calc_fields.r(2,calc_fieldn,opid);
					calc_fields.r(3,calc_fieldn,word1);
					calc_fields.r(4,calc_fieldn,word2);

					whereclause ^= " true";
					continue;
				}
				//select numrange(100,150,'[]')  @> any(string_to_array('1,2,150',',','')::numeric[]);
				if (dictexpression_isarray)
				{
					var date_time_numeric;
					if (dictexpression.index("date_array("))
					{
						whereclause ^= " daterange(";
						date_time_numeric = "date";
					}
					else if (dictexpression.index("time_array("))
					{
						whereclause ^= " tsrange(";
						date_time_numeric = "time";
					}
					else
					{
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
					whereclause ^= " not ";

				if (whereclause)
					whereclause ^= " BETWEEN " ^ word1 ^ " AND " ^ word2;

				continue;
			}

			// starting, ending, containing = like
			/////////////////////////////////////

			var prefix = "";
			var postfix = "";
			if (ucword == "CONTAINING" or ucword == "[]")
			{
				prefix = ".*";
				postfix = ".*";
			}
			else if (ucword == "STARTING" or ucword == "]")
			{
				prefix = "^";
				postfix = ".*";
			}
			else if (ucword == "ENDING" or ucword == "[")
			{
				prefix = ".*";
				postfix = "$";
			}
			if (prefix || postfix)
				word1 = getword(remaining, ucword);

			//"normal" comparative filtering
			////////////////////////////////

			// 1) Acquire operator - or empty if not present

			// convert PICK/AREV relational operators to standard SQL relational operators
			// IS/ISNT/NOT -> EQ/NE/NE
			var aliasno;
			if (var("IS EQ NE NOT ISNT GT LT GE LE").locateusing(" ", ucword, aliasno))
			{
				word1 = var("= = <> <> <> > < >= <=").field(" ", aliasno);
				ucword = word1;
			}

			// capture operator is any
			var op = "";
			if (var("= <> > < >= <= ~ ~* !~ !~*").locateusing(" ", ucword, aliasno))
			{
				// is an operator
				op = ucword;
				// get another word (or words)
				word1 = getword(remaining, ucword);
			}

			//determine Pick/AREV values like "[xxx" "xxx]" and "[xxx]"
			if (word1[1] == "'")
			{
				if (word1[2] == "[")
				{
					word1.splicer(2, 1, "");
					prefix = ".*";
					postfix = "$";
				}
				if (word1[-2] == "]")
				{
					word1.splicer(-2, 1, "");
					if (!prefix)
						prefix = "^";
					postfix = ".*";
				}
				ucword=word1.ucase();
			}

			//select WITH ..._XREF uses postgres full text searching
			//which has its own prefix and postfix rules. see below
			if (dictexpression_isxref)
			{
				prefix="";
				postfix="";
			}

			/*implement using posix regular string matching
			~ 	Matches regular expression, case sensitive 	'thomas' ~
			'.*thomas.*'
			~* 	Matches regular expression, case insensitive 	'thomas' ~*
			'.*Thomas.*'
			!~ 	Does not match regular expression, case sensitive 'thomas' !~
			'.*Thomas.*'
			!~* 	Does not match regular expression, case insensitive 'thomas' !~*
			'.*vadim.*'
			*/

			if (prefix || postfix)
			{

				//postgres match matches anything in the string unless ^ and/or $ are present
				// so .* is not necessary in prefix and postfix
				if (prefix==".*")
					prefix="";
				if (postfix==".*")
					postfix="";

				// escape any posix special characters;
				// [\^$.|?*+()
				// if present in the search criteria, they need to be escaped with
				// TWO backslashes.
				word1.swapper("\\", "\\\\");
				var special = "[^$.|?*+()";
				for (int ii = special.length(); ii > 0; --ii)
				{
					if (special.index(word1[ii]))
						word1.splicer(ii, 0, "\\");
				}
				word1.swapper("'" _FM_ "'", postfix ^ "'" _FM_ "'" ^ prefix);
				word1.splicer(-1, 0, postfix);
				word1.splicer(2, 0, prefix);

				//only ops <> and != are supported when using the regular expression operator (starting/ending/containing)
				if (op == "<>")
					negative=!negative;
				else if (op != "=" and op != "")
					throw MVDBException("SELECT ... WITH " ^ op ^ " " ^ word1 ^ " is not supported. " ^ prefix.quote() ^ " " ^ postfix.quote());

				// use regular expression operator
				op = "~";
				ucword = word1;
			}

			// 2) Acquire value(s) - or empty if not present

			// word1 at this point may be empty, contain a value or the first word of an
			// unrelated clause

			// if word1 unrelated to current phrase
			if (ucword.length() && !valuechars.index(ucword[1]))
			{
				// push back and treat as missing value
				// remaining[1,0]=ucword:' '
				remaining.splicer(1, 0, ucword ^ " ");
				// simulate no given value
				word1 = "";
				ucword = "";
			}

			var value = word1;

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.index("exodus_call"))
			//if (dictexpression == "true")
			{
				//no op or value means test for Pick/AREV true (zero and '' are false)
				if (op == "" && value == "")
					op="!!";

				//missing op presumed to be =
				else if (op == "")
					op="=";

				// invert comparison if "without" or "not" for calculated fields
				if (negative &&
				    var("= <> > < >= <= ~ ~* !~ !~* !! !").locateusing(" ", op, aliasno))
				{
					// op.outputl("op entered:");
					negative = false;
					op = var("<> = <= >= < > !~ !~* ~ ~* ! !!").field(" ", aliasno);
					// op.outputl("op reversed:");
				}

				//++ncalc_fields;
				//calc_fields.r(1,ncalc_fields,dictid);
				//calc_fields.r(2,ncalc_fields,op);
				//calc_fields.r(3,ncalc_fields,value);

				//almost identical code for exodus_call above/below
				var calc_fieldn;
				if (! calc_fields.locate(dictid,calc_fieldn,1))
				{
					//++ncalc_fields;
					calc_fields.r(1,calc_fieldn,dictid);
				}
				if (calc_fields.a(2,calc_fieldn))
					throw MVDBException("WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote());
				calc_fields.r(2,calc_fieldn,op);
				calc_fields.r(3,calc_fieldn,value);

				//place holder to be removed before issuing actual sql command
				whereclause ^= " true";

				continue;
			}

			// missing op and value mean NOT '' or NOT 0 or NOT NULL
			// WITH CLIENT_TYPE
			if (op == "" && value == "")
			{
				op = "<>";
				value = "''";
			}

			// missing op means =
			// WITH CLIENT_TYPE "X"
			if (op == "")
				op = "=";

			// missing value means error in sql
			// WITH CLIENT_TYPE =
			if (value == "")
			{
				// value="";
			}

			// op and value(s) are now set

			// natural order value(s)
			if (usingnaturalorder)
				value = naturalorder(value.toString());

			// without xxx = "abc"
			// with xxx not = "abc"

			// notword.outputl("notword=");
			// ucword.outputl("ucword=");

			// multiple values
			if (value.index(FM))
			{
				//in full text query on multiple words,
				//we implement that words all are required
				//all values and words separated by spaced are used as "word stems"
				//NB ":*" means "ending in" to postgres tsquery. see:
				//https://www.postgresql.org/docs/10/datatype-textsearch.html
				//"lexemes in a tsquery can be labeled with * to specify prefix matching:"
				if (dictexpression_isxref)
				{

					//quoted values
					value.swapper("'" _FM_ "'", ":*|");

					//unquoted numbers
					value.swapper(FM_, ":*&");

				//ordinary query values
				} else
					//WARNING ", " is swapped in mvprogram.cpp ::select()
					//so change there if changed here
					value.swapper(FM_, ", ");

				if (dictexpression_isxref)
				{
					//done below
				}
				else if (dictexpression_isarray)
				{
					//lhs is an array ("multivalues" in postgres)
					//dont convert rhs to in() or any()
				}
				else if (op == "=")
				{
					op = "in";
					value = "( " ^ value ^ " )";
				}
				else if (op == "<>")
				{
					op = "not in";
					value = "( " ^ value ^ " )";
				}
				else
				{
					value = "ANY(ARRAY[" ^ value ^ "])";
				}
			}

			//full text searching
			if (dictexpression_isxref)
			{
				//see note on isxref in "multiple values" section above
				op = "@@";

				//use spaces to indicate search words
				value.swapper(" ",":*&");

				//append postfix :* to every search word
				value.swapper("&",":*&");
				value.splicer(-1,0,":*");

				//use "english" dictionary for stemming (or "simple" dictionary for none)
				// MUST use the SAME in both to_tsvector AND to_tsquery
				//https://www.postgresql.org/docs/10/textsearch-dictionaries.html
				//value = "to_tsquery('simple'," ^ value ^ ")";
				value = "to_tsquery('english'," ^ value ^ ")";
			}

			// testing for "" may become testing for null
			// for date and time which are returned as null for empty string
			else if (value == "''")
			{
				if (dictexpression.index("extract_date") ||
				    dictexpression.index("extract_time"))
				{
					if (op == "=")
						op = "is";
					else
						op = "is not";
					value = "null";
				}
				// currently number returns 0 for empty string
				//|| dictexpression.index("extract_number")
				else if (dictexpression.index("extract_number"))
				{
					value = "'0'";
				}
			}

			//if selecting a mv array then convert right hand side to array
			//(can only handle = operator at the moment)
			if (dictexpression_isarray && (op == "=" or op == "<>"))
			{
				if (op == "<>")
				{
					negative=!negative;
					op = "=";
				}

				if (value == "''")
				{
					value = "'{}'";
				}
				else
				{
					op = "&&";//overlap
					value = "'{" ^ value.convert("'","\"") ^ "}'";
				}
			}

			//negate
			if (negative)
				whereclause ^= " not";

			whereclause ^= " " ^ dictexpression ^ " " ^ op ^ " " ^ value;
			// whereclause.outputl("whereclause=");

		}//with/without

	} // getword loop

	if (calc_fields && orwith)
	{
		throw MVDBException("OR not allowed with sort/select calculated fields");
	}

	// prefix specified keys into where clause
	if (keycodes)
	{
		if (keycodes.count(FM))
		{
			keycodes = actualfilename ^ ".key IN ( " ^ keycodes.swap(FM, ", ") ^ " )";

			if (whereclause)
				//whereclause ^= "\n AND ( " ^ keycodes ^ " ) ";
				whereclause = keycodes ^ "\n AND " ^ whereclause;
			else
				whereclause = keycodes;
		}
	}

	// sselect add by key on the end of any specific order bys
	if (bykey)
		orderclause ^= ", " ^ actualfilename ^ ".key";

	//if calculated fields then secondary sort/select is going to use readnextrecord, so add the data column if missing
	if (calc_fields && actualfieldnames.substr(-6) != ", data")
		actualfieldnames^=", data";

	//remove mv::integer if no unnesting (sort on mv fields)
	if (!unnests)
	{
		// sql ^= ", 0 as mv";
		if (actualfieldnames.index("mv::integer, data"))
		{
			// replace the mv column with zero if selecting record
			actualfieldnames.swapper("mv::integer, data", "0::integer, data");
		}
		else
			actualfieldnames.swapper(", mv::integer", "");
	}

	// if any active select, convert to a file and use as an additional filter on key
	// or correctly named savelistfilename exists from getselect or makelist
	var listname = "";
	// see also listname below
	//	if (this->hasnext()) {
	//		listname=(*this).a(1) ^ "_" ^ getprocessn() ^ "_tempx";
	//		this->savelist(listname);
	//		var savelistfilename="savelist_" ^ listname;
	//		joins ^= " \nINNER JOIN\n " ^ savelistfilename ^ " ON " ^ actualfilename ^
	//".key = " ^ savelistfilename ^ ".key";
	//	}

	// disambiguate from any INNER JOIN key
	//actualfieldnames.outputl("actualfieldnames=");
	//actualfieldnames.swapper("key", actualfilename ^ ".key");
	//actualfieldnames.swapper("data", actualfilename ^ ".data");
	actualfieldnames.replacer("\\bkey\\b", actualfilename ^ ".key");
	actualfieldnames.replacer("\\bdata\\b", actualfilename ^ ".data");

	// DISTINCT has special fieldnames
	if (distinctfieldnames)
		actualfieldnames = distinctfieldnames;

	// remove redundant clauses
	whereclause.swapper("\n AND true", "");
	whereclause.swapper("true\n AND ", "");

	//save any active selection in a temporary table and RIGHT JOIN to it to avoid complete selection of primary file
	if (this->hasnext())
	{
		//create a temporary sql table to hold the preselected keys
		var temptablename="PRESELECT_TEMP_CURSOR_" ^ this->a(1);
		var createtablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
		//createtablesql ^= "CREATE TEMPORARY TABLE " ^ temptablename ^ "\n";
		createtablesql ^= "CREATE TABLE " ^ temptablename ^ "\n";
		createtablesql ^= " (KEY TEXT)\n";
		var errmsg;
		if (! this->sqlexec(createtablesql,errmsg))
		{
			throw MVDBException(errmsg);
		}

		//readnext the keys into a temporary table
		var key;
		while (this->readnext(key))
		{
			//std::cout<<key<<std::endl;
			this->sqlexec("INSERT INTO " ^ temptablename ^ "(KEY) VALUES('" ^ key.swap("'","''") ^"')");
		}

		joins.inserter(1,1,"\n INNER JOIN "^temptablename^" ON "^temptablename^".key = "^actualfilename^".key");
	}

	// assemble the full sql select statement:

	//DECLARE - cursor
	// WITH HOLD is a very significant addition
	// var sql="DECLARE cursor1_" ^ (*this) ^ " CURSOR WITH HOLD FOR SELECT " ^ actualfieldnames
	// ^ " FROM ";
	var sql = "DECLARE\n cursor1_" ^ this->a(1) ^ " SCROLL CURSOR WITH HOLD FOR";

	//SELECT - field/column names
	sql ^= " \nSELECT\n " ^ actualfieldnames;
	if (selects)
		sql ^= selects;

	//FROM - filename and any specially related files
	sql ^= " \nFROM\n " ^ actualfilename;


	//JOIN - (1)?
	if (joins.a(1))
		sql ^= " " ^ joins.a(1).convert(VM, "");

	//UNNEST - mv fields
	//mv fields get added to the FROM clause like "unnest() as xyz" allowing the use of xyz in WHERE/ORDER BY
	//should only be one unnest (parallel mvs if more than one) since it is not clear how sselect by mv by mv2 should work if they are not in parallel
	if (unnests)
	{
		// unnest
		sql ^= ",\n unnest(\n  " ^ unnests.a(3).swap(VM, ",\n  ") ^ "\n )";
		// as fake tablename
		sql ^= " with ordinality as mvtable1";
		// brackets allow providing column names for use elsewhere
		// and renaming of automatic column "ORDINAL" to "mv" for use in SELECT key,mv ...
		// sql statement
		sql ^= "( " ^ unnests.a(2).swap(VM, ", ") ^ ", mv)";
	}

	//JOIN - related files
	if (joins.a(2))
		sql ^= " " ^ joins.a(2).convert(VM, "");

	//WHERE - excludes calculated fields if doing stage 1 of a two stage sort/select
	//TODO when doing stage2, skip "WITH/WITHOUT xxx" of stage1 fields
	if (whereclause)
		sql ^= " \nWHERE \n" ^ whereclause;

	//ORDER - suppressed if doing stage 1 of a two stage sort/select
	if (orderclause && ! calc_fields)
		sql ^= " \nORDER BY \n" ^ orderclause.substr(3);

	//LIMIT - number of records returned
	// no limit initially if any calculated items - limit will be done in secondary sort/select
	if (maxnrecs && ! calc_fields)
		sql ^= " \nLIMIT\n " ^ maxnrecs;

	//sql.outputl("sql=");

	// DEBUG_LOG_SQL
	// if (GETDBTRACE)
	//	exodus::logputl(sql);

	// first close any existing cursor with the same name, otherwise cannot create  new cursor
	if (this->cursorexists())
	{
		var sql = "";
		sql ^= "CLOSE cursor1_";
		if (this->assigned())
			sql ^= this->a(1);

		var errmsg;
		if (!this->sqlexec(sql, errmsg))
		{

			if (errmsg)
				errmsg.outputl("::selectx: " ^ sql ^ "\n" ^ errmsg);
			// return false;
		}
	}

	var errmsg;
	if (!this->sqlexec(sql, errmsg))
	{

		if (listname)
			this->deletelist(listname);

		// TODO handle duplicate_cursor sqlstate 42P03
		sql.outputl("sql=");

		throw MVDBException(errmsg);

		// if (autotrans)
		//	rollbacktrans();
		return false;
	}

	//sort/select on calculated items may be done in mvprogram::calculate
	//which can call calculate() and has access to mv.RECORD, mv.ID etc
	if (calc_fields) {
		calc_fields.r(5,dictfilename.lower());
		calc_fields.r(6,maxnrecs);
		this->r(10,calc_fields.lower());
	}

	return true;
}

void var::clearselect()
{
	// THISIS("void var::clearselect() const")
	// THISISSTRING()

	// default cursor is ""
	const_cast<var&>(*this).unassigned("");

	/// if readnext through string
	//3/4/5/6 setup in makelist. cleared in clearselect
	//if (this->a(3) == "%MAKELIST%")
	{
		this->r(6,"");
		this->r(5,"");
		this->r(4,"");
		this->r(3,"");
		return;
	}

	var listname = (*this) ^ "_" ^ getprocessn() ^ "_tempx";

	// if (GETDBTRACE)
	//	exodus::logputl("DBTRACE: ::clearselect() for " ^ listname);

	// dont close cursor unless it exists otherwise sql error aborts any transaction
	// if (not this->cursorexists())
	if (not this->cursorexists())
		return;

	// clear any select list
	this->deletelist(listname);

	var sql = "";
	// sql^="DECLARE BEGIN ";
	sql ^= "CLOSE cursor1_";
	if (this->assigned())
		sql ^= this->a(1);
	// sql^="\nEXCEPTION WHEN\n invalid_cursor_name\n THEN";
	// sql^="\nEND";

	//sql.output();

	var errors;
	if (!this->sqlexec(sql, errors))
	{
		if (errors)
			errors.outputl("::clearselect " ^ errors);
		return;
	}

	return;
}

// NB global not member function
//	To make it var:: privat member -> pollute mv.h with PGresultptr :(
// bool readnextx(const std::string& cursor, PGresultptr& pgresult)
// NB caller MUST ALWAYS do PQclear(pgresult) even bool false
// called by readnext/readnextrecord (and perhaps hasnext/select to implement LISTACTIVE)
bool readnextx(const var& cursor, PGresultptr& pgresult, PGconn* pgconn, bool forwards)
{
	var sql;
	if (forwards)
		sql = "FETCH NEXT in cursor1_" ^ cursor.a(1);
	else
		sql = "FETCH BACKWARD in cursor1_" ^ cursor.a(1);

	// sql="BEGIN;" ^ sql ^ "; END";

	// execute the sql
	// cant use sqlexec here because it returns data
	// sqlexec();
	if (!getpgresult(sql, pgresult, pgconn))
	{

		// var errmsg=var(PQresultErrorMessage(pgresult));
		// PQclear(pgresult);
		// throw MVDBException(errmsg);
		// cursor.clearselect();
		// return false;

		var errmsg = var(PQresultErrorMessage(pgresult));
		// errmsg.outputl("errmsg=");
		// var(pgresult).outputl("pgresult=");
		var sqlstate = "";
		if (PQresultErrorField(pgresult, PG_DIAG_SQLSTATE))
		{
			sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		}
		// pgresult is NULLPTR if if getpgresult failed but since the pgresult is needed by
		// the caller, it will be cleared by called if not NULLPTR PQclear(pgresult);

		// if cursor simply doesnt exist then see if a savelist one is available and enable
		// it 34000 - "ERROR:  cursor "cursor1_" does not exist"
		if (forwards && sqlstate == "34000")
		{
			return false;

			/**
			//if the standard select list file is available then select it, i.e. create
			a CURSOR, so FETCH has something to work on var listfilename="savelist_" ^
			cursor ^ "_" ^ getprocessn() ^ "_tempx"; if (not var().open(listfilename))
				return false;
			//TODO should add BY LISTITEMNO
			if (not cursor.select("select " ^ listfilename))
				return false;
			if (GETDBTRACE)
				exodus::logputl("DBTRACE: readnextx(...) found standard selectfile "
			^ listfilename);

			return readnextx(cursor, pgresult, pgconn, clearselect_onfail, forwards);
			**/
		}

		// any other error
		if (errmsg)
			throw MVDBException(errmsg ^ " sqlstate= " ^ sqlstate.quote() ^ " in SQL " ^
					    sql);

		return false;
	}

	// 1. Do NOT clear the cursor even if forward since we may be testing it
	// 2. DO NOT clear since the pgresult is needed by the caller
	// PQclear(pgresult);

	// true = found a new key/record
	return true;
}

bool var::deletelist(const var& listname) const
{
	THISIS("bool var::deletelist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		return true;
	// throw MVDBException("LISTS file cannot be opened");

	// initial block of keys are stored with no suffix (i.e. no *1)
	lists.deleterecord(listname);

	// supplementary blocks of keys are stored with suffix *2, *3 etc)
	for (int listno = 2;; ++listno)
	{
		var xx;
		if (!xx.read(lists, listname ^ "*" ^ listno))
			break;
		lists.deleterecord(listname ^ "*" ^ listno);
	}

	return true;
}

bool var::savelist(const var& listname)
{

	THISIS("bool var::savelist(const var& listname)")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	if (GETDBTRACE)
		("DBTRACE: ::savelist(" ^ listname ^ ")").logputl();

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		throw MVDBException("LISTS file cannot be opened");

	// this should not throw if the list does not exist
	this->deletelist(listname);

	var listno = 1;
	var listkey = listname;
	var block = "";
	static int maxblocksize = 1024 * 1024;

	var key;
	var mv;
	while (this->readnext(key, mv))
	{

		// append the key
		block.var_str.append(key.var_str);

		// append SM + mvno if mvno present
		if (mv)
		{
			block.var_str.push_back(VM_);
			block.var_str.append(mv.var_str);
		}

		// save a block of keys if more than a certain size (1MB)
		if (block.length() > maxblocksize)
		{
			// save the block
			block.write(lists, listkey);

			// prepare the next block
			listno++;
			listkey = listname ^ "*" ^ listno;
			block = "";
			continue;
		}

		// append a FM separator since lists use FM
		block.var_str.push_back(FM_);
	}

	// write the block
	if (block.length() > 1)
	{
		block.write(lists, listkey);
		listno++;
	}

	return listno > 0;
}

bool var::getlist(const var& listname)
{
	THISIS("bool var::getlist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	if (GETDBTRACE)
		var("DBTRACE: ::savelist(" ^ listname ^ ")").logputl();

	//int recn = 0;
	var key;
	var mv;
	var listfilename = "savelist_" ^ listname.field(" ", 1);
	listfilename.converter("-.*/", "____");
	// return this->selectx("key, mv::integer",listfilename);

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		throw MVDBException("LISTS file cannot be opened");

	var keys;
	if (!keys.read(lists, listname))
		// throw MVDBException(listname.quote() ^ " list does not exist.");
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

// MAKELIST would be much better called MAKESELECT
// since the most common usage is to omit listname in which case the keys will be used to simulate a
// SELECT statement Making a list can be done simply by writing the keys into the list file without
// using this function
bool var::makelist(const var& listname, const var& keys)
{
	THISIS("bool var::makelist(const var& listname)")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(listname)
	ISSTRING(keys)

	// this is not often used since can be achieved by writing keys to lists file directly
	if (listname)
	{
		this->deletelist(listname);

		// open the lists file on the same connection
		var lists = *this;
		if (!lists.open("LISTS"))
			throw MVDBException("LISTS file cannot be opened");
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

bool var::hasnext() const
{

	// var xx;
	// return this->readnext(xx);

	// THISIS("bool var::hasnext() const")
	// THISISSTRING()

	// default cursor is ""
	const_cast<var&>(*this).unassigned("");

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->a(3);
	if (listid)
	{
		var keyno = this->a(5);
		keyno++;

		var key_and_mv = this->a(6, keyno);

		// true if we have another key
		if (key_and_mv.length())
			return true;

		// otherwise try and get another block
		var lists = *this;
		if (!lists.open("LISTS"))
			throw MVDBException("LISTS file cannot be opened");
		var listno = this->a(4);
		listno++;
		listid.fieldstorer("*", 2, 1, listno);

		// if no more blocks of keys then return false
		var block;
		if (!block.read(lists, listid))
			return false;

		// might as well cache the next block for the next readnext
		const_cast<var&>(*this).r(4, listno);
		const_cast<var&>(*this).r(5, 0);
		const_cast<var&>(*this).r(6, block.lowerer());

		return true;
	}

	//TODO avoid this trip to the database somehow?
	if (!this->cursorexists())
		return false;

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
	{
		// this->clearselect();
		return false;
	}

	PGresultptr pgresult;
	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);

	Resultclearer clearer(pgresult);

	if (!ok)
	{
		// this->clearselect();
		return false;
	}

	/////////////////////////////////
	// now restore the cursor back one
	/////////////////////////////////

	PGresultptr pgresult2;
	//ok =
	readnextx(*this, pgresult2, pgconn, /*forwards=*/false);

	Resultclearer clearer2(pgresult2);

	// Note that moving backwards on the first record fails because it returns no rows since it
	// moves the cursor to point ONE BEFORE the first row if (!ok) { 	return true;
	//}

	return true;
}

bool var::readnext(var& key)
{
	var valueno;
	return readnext(key, valueno);
}

bool var::readnext(var& key, var& valueno)
{

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ & VARTYP_MASK)
	{
		// throw MVUndefined("readnext()");
		var_str = "";
		var_typ = VARTYP_STR;
	}

	// default cursor is ""
	const_cast<var&>(*this).unassigned("");

	THISIS("bool var::readnext(var& key, var& valueno) const")
	THISISSTRING()

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->a(3);
	if (listid)
	{
		while (true)
		{
			var keyno = this->a(5);
			keyno++;

			var key_and_mv = this->a(6, keyno);

			// if no more keys, try to get next block of keys, otherwise return false
			if (key_and_mv.length() == 0)
			{

				// makelist provides one block of keys and nothing in the lists file
				if (listid == "%MAKELIST%")
				{
					this->r(3,"");
					this->r(4,"");
					this->r(5,"");
					this->r(6,"");
					return false;
				}

				var lists = *this;
				if (!lists.open("LISTS"))
					throw MVDBException("LISTS file cannot be opened");

				var listno = this->a(4);
				listno++;
				listid.fieldstorer("*", 2, 1, listno);

				var block;
				if (!block.read(lists, listid))
					return false;

				this->r(4, listno);
				this->r(5, 0);
				this->r(6, block.lowerer());
				continue;
			}

			// bump up the key no pointer
			this->r(5, keyno);

			// extract and return the key (and mv if present)
			key = key_and_mv.a(1, 1, 1);
			valueno = key_and_mv.a(1, 1, 2);
			return true;
		}
	}

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
	{
		this->clearselect();
		return false;
	}

	PGresultptr pgresult;
	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);

	Resultclearer clearer(pgresult);

	//__asm__("int3");

	if (not ok)
	{
		// end the transaction and quit
		// no more
		// committrans();
		this->clearselect();
		return false;
	}

	// MUST call PQclear(pgresult) on all paths below

	/* abortive code to handle unescaping returned hex/escape data	//avoid the need for this by
	calling pqexecparams flagged for binary
	//even in the case where there are no parameters and pqexec could be used.

	//eg 90001 is 9.0.1
	int pgserverversion=PQserverVersion(thread_pgconn);
	if (pgserverversion>=90001) {
		var(pgserverversion).outputl();
		//unsigned char *PQunescapeBytea(const unsigned char *from, size_t *to_length);
		size_t to_length;
		unsigned char* unescaped = PQunescapeBytea((const unsigned char*)
	PQgetvalue(pgresult, 0, 0), &to_length); if (*unescaped)
			key=stringfromUTF8((UTF8*)unescaped, to_length);
		PQfreemem(unescaped);
		PQclear(pgresult);
		return true;
	}
*/
	// get the key from the first column
	// char* data = PQgetvalue(pgresult, 0, 0);
	// int datalen = PQgetlength(pgresult, 0, 0);
	// key=std::string(data,datalen);
	key = getresult(pgresult, 0, 0);
	// key.output("key=").len().outputl(" len=");

	// vn is second column
	// record is third column
	if (PQnfields(pgresult) > 1)
		// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
		valueno = getresult(pgresult, 0, 1);
	else
		valueno = 0;

	// PQclear(pgresult);

	return true;

/*how to access multiple records and fields*/
#if 0
	/* first, print out the attribute names */
	int nFields = PQnfields(pgresult);
	for (i = 0; i < nFields; i++)
		wprintf("%-15s", PQfname(pgresult, i));
	wprintf("\n\n");

	/* next, print out the rows */
	for (i = 0; i < PQntuples(pgresult); i++)
	{
		for (j = 0; j < nFields; j++)
			wprintf("%-15s", PQgetvalue(pgresult, i, j));
		wprintf("\n");
	}
#endif
}

bool var::readnextrecord(var& record, var& key)
{
	var valueno = 0;
	return readnextrecord(record, key, valueno);
}

bool var::readnextrecord(var& record, var& key, var& valueno)
{

	//?allow undefined usage like var xyz=xyz.readnextrecord();
	if (var_typ & VARTYP_MASK || !var_typ)
	{
		// throw MVUndefined("readnextrecord()");
		var_str = "";
		var_typ = VARTYP_STR;
	}

	// default cursor is ""
	const_cast<var&>(*this).unassigned("");

	THISIS("bool var::readnextrecord(var& record, var& key) const")
	THISISSTRING()
	ISDEFINED(key)
	ISDEFINED(record)

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	PGresultptr pgresult;
	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);

	Resultclearer clearer(pgresult);

	if (!ok)
	{
		// end the transaction
		// no more
		// committrans();
		this->clearselect();
		return false;
	}

	// MUST call PQclear(pgresult) on all paths below

	// key is first column
	// char* data = PQgetvalue(pgresult, 0, 0);
	// int datalen = PQgetlength(pgresult, 0, 0);
	// key=std::string(data,datalen);
	key = getresult(pgresult, 0, 0);
	// TODO return zero if no mv in select because no by mv column

	// vn is second column
	// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
	valueno = getresult(pgresult, 0, 1);

	// record is third column
	if (PQnfields(pgresult) < 3)
	{
		PQclear(pgresult);
		var errmsg = "readnextrecord() must follow selectrecord(), not select()";
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		// return false;
	}
	record = getresult(pgresult, 0, 2);

	// PQclear(pgresult);

	return true;
}

bool var::createindex(const var& fieldname0, const var& dictfile) const
{
	THISIS("bool var::createindex(const var& fieldname, const var& dictfile) const")
	THISISSTRING()
	ISSTRING(fieldname0)
	ISSTRING(dictfile)

	var filename = (*this).a(1);
	var fieldname = fieldname0.convert(".","_");

	// actual dictfile to use is either given or defaults to that of the filename
	var actualdictfile;
	if (dictfile.assigned() and dictfile != "")
		actualdictfile = dictfile;
	else
		actualdictfile = "dict_" ^ filename;

	// example sql
	// create index ads__brand_code on ads (exodus_extract_text(data,3,0,0));

	// throws if cannot find dict file or record
	var joins = ""; // throw away - cant index on joined fields at the moment
	var unnests = ""; // throw away - cant index on multi-valued fields at the moment
	var selects = "";
	var ismv;
	var forsort = false;
	var dictexpression = getdictexpression(filename, filename, actualdictfile, actualdictfile,
					       fieldname, joins, unnests, selects, ismv, forsort);
	// dictexpression.outputl("dictexp=");stop();

	//mv fields return in unnests, not dictexpression
	//if (unnests)
	//{
	//	dictexpression = unnests.a(3);
	//}

	var sql;

	// index on calculated columns causes an additional column to be created
	if (dictexpression.index("exodus_call"))
	{
		("ERROR: Cannot create index on " ^ filename ^ " for calculated field " ^ fieldname).errputl();
		return false;

	/*
		// add a new index field
		var index_fieldname = "index_" ^ fieldname;
		// sql="alter table " ^ filename ^ " add " ^ index_fieldname ^ " bytea";
		sql = "alter table " ^ filename ^ " add " ^ index_fieldname ^ " text";
		if (not var().sqlexec(sql))
		{
			sql.outputl("sql failed ");
			return false;
		}

		// update the new index field for all records
		sql = "update " ^ filename ^ " set " ^ index_fieldname ^ " = " ^ dictexpression;
		sql.outputl("sql=");
		if (not var().sqlexec(sql))
		{
			sql.outputl("sql failed ");
			return false;
		}
		dictexpression = index_fieldname;
	*/
	}

	// create postgres index
	sql = "CREATE INDEX index__" ^ filename ^ "__" ^ fieldname ^ " ON " ^ filename;
	if (ismv || fieldname.substr(-5) == "_xref")
		sql ^= " USING GIN";
	sql ^= " (";
	sql ^= dictexpression;
	sql ^= ")";

	return this->sqlexec(sql);
}

bool var::deleteindex(const var& fieldname0) const
{
	THISIS("bool var::deleteindex(const var& fieldname) const")
	THISISSTRING()
	ISSTRING(fieldname0)

	var filename = (*this).a(1);
	var fieldname = fieldname0.convert(".","_");

	// delete the index field (actually only present on calculated field indexes so ignore
	// result) deleting the index field automatically deletes the index
	var index_fieldname = "index_" ^ fieldname;
	if (var().sqlexec("alter table " ^ filename ^ " drop " ^ index_fieldname))
		return true;

	// delete the index
	// var filename=*this;
	var sql = "drop index index__" ^ filename ^ "__" ^ fieldname;
	return this->sqlexec(sql);
}

/*
http://www.petefreitag.com/item/666.cfm
information_schema is am SQL-92 standard for accessinging information about the tables etc in a
database
	* Microsoft SQL Server - Supported in Version 7 and up
	* MySQL - Supported in Version 5 and up
	* PostgreSQL - Supported in Version 7.4 and up

	* Oracle - Does not appear to be supported
	* Apache Derby - NOT Supported As of Version 10.3
	* DB2 - NOT supported?
*/

var var::listfiles() const
{
	THISIS("var var::listfiles() const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()

	// from http://www.alberton.info/postgresql_meta_info.html

	var sql = "SELECT table_name FROM information_schema.tables WHERE table_type = 'BASE "
		  "TABLE' AND table_schema NOT IN ('pg_catalog', 'information_schema'); ";

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	PGresultptr pgresult;
	bool ok = getpgresult(sql, pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return "";

	var filenames = "";
	int nfiles = PQntuples(pgresult);
	for (int filen = 0; filen < nfiles; filen++)
	{
		if (!PQgetisnull(pgresult, filen, 0))
			filenames ^= FM ^ getresult(pgresult, filen, 0);
	}
	filenames.splicer(1, 1, "");

	// PQclear(pgresult);

	return filenames;
}

bool var::cursorexists() const
{
	// THISIS("var var::cursorexists() const")
	// could allow undefined usage since *this isnt used?
	// THISISSTRING()

	// default cursor is ""
	const_cast<var&>(*this).unassigned("");

	// from http://www.alberton.info/postgresql_meta_info.html

	var sql = "SELECT name from pg_cursors where name = 'cursor1_" ^ this->a(1) ^ "'";
	// var sql="SELECT name from pg_cursors";

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	PGresultptr pgresult;
	bool ok = getpgresult(sql, pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return false;

	ok = PQntuples(pgresult) > 0;

	// if (GETDBTRACE)
	//	exodus::logputl("DBTRACE: ::cursorexists() is " ^ var(ok));

	return ok;
}

var var::listindexes(const var& filename0, const var& fieldname0) const
{
	THISIS("var var::listindexes(const var& filename) const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename0)
	ISSTRING(fieldname0)

	var filename = filename0.a(1);
	var fieldname = fieldname0.convert(".","_");

	// TODO for some reason doesnt return the exodus index_file__fieldname records
	// perhaps you have to be connected with sufficient postgres rights
	var sql = "SELECT relname"
		  " FROM pg_class"
		  " WHERE oid IN ("
		  " SELECT indexrelid"
		  " FROM pg_index, pg_class"
		  " WHERE";
	if (filename)
		sql ^= " relname = '" ^ filename.lcase() ^ "' AND ";
	// if (fieldname)
	//	sql^=" ???relname = '" ^ fieldname.lcase() ^  "' AND ";
	sql ^= " pg_class.oid=pg_index.indrelid"
	       " AND indisunique != 't'"
	       " AND indisprimary != 't'"
	       ");";

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGresultptr pgresult;
	bool ok = getpgresult(sql, pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return "";

	// MUST PQclear(pgresult) below

	var tt;
	var indexname;
	var indexnames = "";
	int nindexes = PQntuples(pgresult);
	var lc_fieldname = fieldname.lcase();
	for (int indexn = 0; indexn < nindexes; indexn++)
	{
		if (!PQgetisnull(pgresult, indexn, 0))
		{
			indexname = getresult(pgresult, indexn, 0);
			if (indexname.substr(1, 6) == "index_")
			{
				tt = indexname.index("__");
				if (tt)
				{
					indexname.substrer(8, 999999).swapper("__", VM);
					if (fieldname && indexname.a(1, 2) != lc_fieldname)
						continue;

					// indexnames^=FM^indexname;
					var fn;
					if (not indexnames.locateby("A", indexname, fn, 0))
						indexnames.inserter(fn, indexname);
				}
			}
		}
	}
	// indexnames.splicer(1,1,"");

	// PQclear(pgresult);

	return indexnames;
}

var var::reccount(const var& filename) const
{
	THISIS("var var::reccount(const var& filename) const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename)

	// vacuum otherwise unreliable
	this->flushindex(filename);

	var sql = "SELECT reltuples::integer FROM pg_class WHERE relname = '" ^
		  filename.a(1).lcase() ^ "';";

	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGresultptr pgresult;
	bool ok = getpgresult(sql, pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return "";

	// MUST PQclear(pgresult) below

	var reccount = "";
	if (PQntuples(pgresult) && PQnfields(pgresult) > 0 && !PQgetisnull(pgresult, 0, 0))
	{
		// reccount=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 0)));
		reccount = getresult(pgresult, 0, 0);
	}

	// PQclear(pgresult);

	return reccount;
}

var var::flushindex(const var& filename) const
{
	THISIS("var var::flushindex(const var& filename="
	       ") const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename)

	var sql = "VACUUM";
	if (filename)
		// attribute 1 in case passed a filehandle instead of just filename
		sql ^= " " ^ filename.a(1).lcase();
	sql ^= ";";
	// sql.outputl("sql=");

	// TODO perhaps should get connection from filehandle if passed a filehandle
	PGconn* pgconn = (PGconn*)this->connection();
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGresultptr pgresult;
	bool ok = getpgresult(sql, pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return "";

	// MUST PQclear(pgresult) below

	var flushresult = "";
	if (PQntuples(pgresult))
	{
		flushresult = true;
	}

	// PQclear(pgresult);

	return flushresult;
}

// used for sql commands that require no parameters
// returns 1 for success
// returns 0 for failure
// WARNING in either case caller MUST PQclear(pgresult)
static bool getpgresult(const var& sql, PGresultptr& pgresult, PGconn* thread_pgconn)
{
	DEBUG_LOG_SQL

	/* dont use PQexec because is cannot be told to return binary results
	 and use PQexecParams with zero parameters instead
	//execute the command
	pgresult = getpgresult(thread_pgconn, sql.toString().c_str());
	pgresult = pgresult;
	*/

	// parameter array but no parameters
	const char* paramValues[1];
	int paramLengths[1];
	// int		 paramFormats[1];

	// PGresult*
	// will contain any pgresult IF successful
	// MUST do PQclear(pgresult) after using it;
	// PGresult* local_result = PQexecParams(thread_pgconn,
	pgresult = PQexecParams(thread_pgconn, sql.toString().c_str(), 0, /* zero params */
				NULL, /* let the backend deduce param type */
				paramValues, paramLengths,
				0,  // text arguments
				0); // text results
	// paramFormats,
	// 1);	  /* ask for binary results */

	// NO! pgresult is returned to caller to extract any data AND call PQclear(pgresult)
	// Resultclearer clearer(pgresult);

	if (!pgresult)
	{
#if TRACING >= 1
		var("ERROR: mvdbpostgres PQexec command failed, no error code: ").errputl();
#endif

		// PQclear(pgresult);
		return false;
	}
	else
	{
		switch (PQresultStatus(pgresult))
		{
		case PGRES_COMMAND_OK:
#if TRACING >= 3
			const char* str_res;
			str_res = PQcmdTuples(pgresult);
			if (strlen(str_res) > 0)
			{
				var("Command executed OK, " ^ var(str_res) ^ " rows.").logputl();
			}
			else
			{
				var("Command executed OK, 0 rows.").logputl();
			}
#endif

			return true;

		case PGRES_TUPLES_OK:

#if TRACING >= 3
			var(sql ^ "\nSelect executed OK, " ^ var(PQntuples(pgresult)) ^
					" rows found.").logputl();
#endif

			return PQntuples(pgresult) > 0;

		case PGRES_NONFATAL_ERROR:

			//#if TRACING >= 1
			var("ERROR: mvdbpostgres SQL non-fatal error code " ^
					var(PQresStatus(PQresultStatus(pgresult))) ^ ", " ^
					var(PQresultErrorMessage(pgresult))).errputl();
			//#endif

			return true;

		default:

			//#if TRACING >= 1
			if (sql.field(" ", 1) != "FETCH")
			{
				var("ERROR: mvdbpostgres pqexec " ^ var(sql)).errputl();
				var("ERROR: mvdbpostgres pqexec " ^
						var(PQresStatus(PQresultStatus(pgresult))) ^ ": " ^
						var(PQresultErrorMessage(pgresult))).errputl();
			}
			//#endif

			// this is defaulted above for safety
			// retcode=0;
			// PQclear(pgresult);
			return false;
		}

		// should never get here
		// PQclear(pgresult);
	}

	// should never get here
	return false;

} /* pqexec */

/* create sql view from dict
#include <exodus/mv.h>
using namespace exodus;

int main() {

	var crlf="\r\n";

	var textfile;
	var dictfilename;
	var extractvarno;
	var expression;

	//datafileprefix='DATA_'
	var datafileprefix = "";
	//viewsuffix='_VIEW'
	var viewsuffix = "";
	var viewprefix = "VIEW_";
	//syntax is DICT2SQL filename,...�volumename

	var filename = filenames.a(filen);
	if (filename == "")
		goto exit;

	var sqlfilename = filename;
	sqlfilename.converter(".", "_");

	if (filename.substr(1, 5) == "DICT.")
		dictfilename = "DICT.ACCESSIBLE_COLUMNS";
	else
		dictfilename = "DICT." ^ filename;
	}

	if (_DICT.open(dictfilename, ""))
		var("SSELECT " ^ dictfilename ^ " BY TYPE BY FMC BY PART (S)").perform();
	else
		var().clearselect();

		sql ^= crlf ^ "-- DROP TABLE " ^ datafileprefix ^ sqlfilename;
		sql ^= crlf ^ "-- ;";

		sql ^= crlf ^ "CREATE TABLE " ^ datafileprefix ^ sqlfilename;
		sql ^= crlf ^ "(";
		sql ^= crlf ^ " key bytea primary key,";
		sql ^= crlf ^ " data bytea";
		sql ^= crlf ^ ")";
		sql ^= crlf ^ ";";

		sql ^= crlf ^ "--DROP VIEW " ^ viewprefix ^ sqlfilename ^ viewsuffix;
		sql ^= crlf ^ "--;";

		//sql:=crlf:'CREATE OR REPLACE VIEW ':viewprefix:sqlfilename:viewsuffix:' AS SELECT
' sql ^= crlf ^ "CREATE VIEW " ^ viewprefix ^ sqlfilename ^ viewsuffix ^ " AS SELECT ";

		var nsvs = 0;

		//table and view for multivalues
		var anyvar = 0;
		var nvars = 0;
		var sql2 = "";
		sql2 ^= crlf ^ "-- DROP TABLE " ^ datafileprefix ^ sqlfilename ^ "_LINES";
		sql2 ^= crlf ^ "-- ;";

		sql2 ^= crlf ^ var("CREATE TABLE ").oconv(datafileprefix) ^ sqlfilename ^ "_LINES";
		sql2 ^= crlf ^ "(";
		sql2 ^= crlf ^ " key bytea,";
		sql2 ^= crlf ^ " lineno integer,";
		sql2 ^= crlf ^ " PRIMARY KEY (key, lineno)";
		sql2 ^= crlf ^ ")";
		sql2 ^= crlf ^ ";";

		sql2 ^= crlf ^ "--DROP VIEW " ^ viewprefix ^ sqlfilename ^ "_LINES" ^ viewsuffix;
		sql2 ^= crlf ^ "--;";

		//sql:=crlf:'CREATE OR REPLACE VIEW ':viewprefix:sqlfilename:'_LINES':viewsuffix:'
AS SELECT ' sql2 ^= crlf ^ "CREATE VIEW " ^ viewprefix ^ sqlfilename ^ "_LINES" ^ viewsuffix ^ " AS
SELECT ";

		var ndicts = 0;

nextdict:
	while (true)
	{
		var dictid;
		if (!dictid.readnext())
			break;

		var dict;
		if (!(dict.read(_DICT, dictid)))
			continue;

		if (dictfilename == "DICT.ACCESSIBLE_COLUMNS") {
			if (dictid == "TABLE_NAME")
				goto continue;
			//ordinary dictionaries dictid is not multipart
			if (dictid == "COLUMN_NAME")
				dict.replacer(5, 0, 0, "");
		}

		dictitem2sql(dict,sql);

		if (dict.a(1) not_eq "F")
			continue;

		//skip duplicates
		//if dict<28> then continue

		ndicts += 1;

		//sql^=crlf;

		var ismv = dict.a(4)[1] == "M";
		if (ismv) {
			anyvar = 1;
			extractvarno = "lineno";
		}else{
			extractvarno = "0";
		}

		if (dict.a(2) == "0") {
			if (not dict.a(5)) {
				expression = "key";
			}else{
				//needs pgneosys function not written as yet
				//sql:='text_field(key,"*",':dict<5>:')'
				//postgres only probably
				expression = "split_part(key,\'*\'," ^ dict.a(5) ^ ")";
			}
		}else{
			//if dict<7> then
			//needs pgneosys functions installed in server
			expression = "exodus_extract_text(data," ^ dict.a(2) ^ "," ^ extractvarno ^
",0)";
			//end else
			// sql:='bytea_a(data,':dict<2>:',':extractvarno:',0)'
			// end
		}

		var conversion = dict.a(7);
		if (conversion.substr(1, 9) == "[DATETIME") {
			expression.swapper("exodus_extract_text(", "exodus_extract_datetime(");

		}else if (conversion[1] == "D" or conversion.substr(1, 5) == "[DATE") {
			//expression[1,0]="date '1967-12-31' + cast("
			//expression:=' as integer)'
			expression.swapper("exodus_extract_text(", "exodus_extract_date(");

		}else if (conversion == "[NUMBER,0]" or dict.a(11) == "0N" or (dict.a(11)).substr(1,
3) == "0N_") { expression.splicer(1, 0, "cast("); expression ^= " as integer)";
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}else if (conversion.substr(1, 2) == "MD" or conversion.substr(1, 7) == "[NUMBER" or
(dict.a(11)).substr(1, 7) == "[NUMBER" or dict.a(12) == "FLOAT" or (dict.a(11)).index("0N", 1)) {
			expression.splicer(1, 0, "cast(");
			expression ^= " as float)";
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}else if (conversion.substr(1, 2) == "MT" or conversion.substr(1, 5) == "[TIME") {
			//expression[1,0]="cast("
			//expression:=' as interval)'
			expression.swapper("exodus_extract_text(", "exodus_extract_time(");

		}else if (dict.a(9) == "R") {
			//expression[1,0]="cast("
			//expression:=' as numeric)'
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}

		if (ismv) {
			nvars += 1;
			sql2 ^= crlf ^ expression ^ " as " ^ dictid;
			sql2 ^= ", ";
		}else{
			nsvs += 1;
			sql ^= crlf ^ expression ^ " as " ^ dictid;
			sql ^= ", ";
		}

	}//dicts


	if (nsvs) {
		sql.trimmerb();
		if (sql[-1] == ",")
			sql.splicer(-1, 1, "");
	}else{
		sql ^= " *";
	}

	if (nvars) {
		sql2.trimmerb();
		if (sql2[-1] == ",")
			sql2.splicer(-1, 1, "");
	}

	sql ^= crlf ^ " from " ^ datafileprefix ^ sqlfilename;
	sql ^= crlf ^ ";";

	if (anyvar) {

		sql2 ^= crlf ^ " from " ^ datafileprefix ^ sqlfilename ^ "_LINES";
		sql2 ^= crlf ^ " LEFT JOIN " ^ datafileprefix ^ sqlfilename;
		sql2 ^= " ON " ^ datafileprefix ^ sqlfilename ^ "_LINES.KEY = " ^ datafileprefix ^
sqlfilename ^ ".key"; sql2 ^= crlf ^ ";";

		sql ^= crlf ^ sql2;
		sql2 = "";
	}

	sql ^= crlf ^ "COMMIT;";

	sql.swapper(_FM, crlf);

	if (!textptr)
		sql.splicer(1, 0, "BEGIN;" ^ (crlf));

	osbwritex(sql, textfile, textfilename, textptr);
	textptr += sql.length();

	sql = "";

	var().stop();
}

}

*/

} // namespace exodus
