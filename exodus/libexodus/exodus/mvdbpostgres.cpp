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

#if defined _MSC_VER  // || defined __CYGWIN__ || defined __MINGW32__
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

#include <cstring>	//for strcmp strlen
#include <iostream>

// see exports.txt for all PQ functions
//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h>  //in postgres/include

//#include <arpa/inet.h>//for ntohl()

#include "MurmurHash2_64.h"	 // it has included in mvdbconns.h (uint64_t defined)

#include <exodus/mv.h>
#include <exodus/mvdbconns.h>  // placed as last include, causes boost header compiler errors
//#include <exodus/mvenvironment.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus {

// the idea is for exodus to have access to one standard database without secret password
static var defaultconninfo =
	"host=127.0.0.1 port=5432 dbname=exodus user=exodus "
	"password=somesillysecret connect_timeout=10";

// bool startipc();

// DBTRACE is set in exodus_main (console programs) but not when used as a plain library
// so initialise it on the fly. assume that it will usually be less than one for not tracing
#define GETDBTRACE (DBTRACE >= 0 && getdbtrace())
bool getdbtrace() {
	if (DBTRACE == 0)
		DBTRACE = var().osgetenv("EXO_DBTRACE") ? 1 : -1;
	return DBTRACE > 0;
}

// Deleter function to close connection and connection cache object
// this is also called in the destructor of MVConnectionsCache
// MAKE POSTGRES CONNECTIONS ARE CLOSED PROPERLY OTHERWISE MIGHT RUN OUT OF CONNECTIONS!!!
// TODO so make sure that we dont use exit(n) anywhere in the programs!
static void connection_DELETER_AND_DESTROYER(PGconn* pgconn) {
	//PGconn* pgp = (PGconn*)pgconn;
	auto pgconn2 = pgconn;
    // at this point we have good new connection to database
    if (GETDBTRACE) {
        var("").logputl("DBTR PQFinish");
	}
	//var("========================== deleting connection ==============================").errputl();
	PQfinish(pgconn2);	// AFAIK, it destroys the object by pointer
					//	delete pgp;
}

//#if TRACING >= 5
#define DEBUG_LOG_SQL                  \
	if (GETDBTRACE) {                  \
		sql.squote().logputl("SQL0 "); \
	}

#define DEBUG_LOG_SQL1                                                                                             \
	if (GETDBTRACE) {                                                                                              \
		((this->assigned() ? *this : "") ^ " | " ^ sql.swap("$1", var(paramValues[0]).squote())).logputl("SQL1 "); \
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

thread_local int thread_default_data_mvconn_no = 0;
thread_local int thread_default_dict_mvconn_no = 0;
//thread_local var thread_connparams = "";
thread_local var thread_dblasterror = "";
thread_local MVConnections thread_connections(connection_DELETER_AND_DESTROYER);
thread_local std::unordered_map<std::string, std::string> thread_file_handles;

std::string getresult(PGresult* pgresult, int rown, int coln) {
	return std::string(PQgetvalue(pgresult, rown, coln), PQgetlength(pgresult, rown, coln));
}

//given a file name or handle, extract filename, standardize utf8, lowercase and change . to _
std::string get_normal_filename(const var& filename_or_handle) {
	return filename_or_handle.a(1).normalize().lcase().convert(".", "_");
}

// used to detect sselect command words that are values like quoted words or plain numbers. eg "xxx"
// 'xxx' 123 .123 +123 -123
static const var valuechars("\"'.0123456789-+");

uint64_t mvdbpostgres_hash_filename_and_key(const var& filehandle, const var& key) {

	std::string fileandkey = get_normal_filename(filehandle);
	fileandkey.append(" ");
	fileandkey.append(key.normalize());

	// TODO .. provide endian identical version
	// required if and when exodus processes connect to postgres on a DIFFERENT host
	// although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the
	// SAME host
	return MurmurHash64((char*)fileandkey.data(), int(fileandkey.length() * sizeof(char)), 0);

}

class PGResult {

	// Save pgresultptr in this class and to guarantee that it will be PQClear'ed on function
	// exit PGResult clearer(pgresult);

	// owns the PGresult object on the stack
	PGresult* pgresult_ = nullptr;

   public:
	//default constructor
	PGResult() = default;

	// constructor from a PGresult*
	PGResult(PGresult* pgresult)
		: pgresult_(pgresult) {
		// var("Got pgresult ... ").output();
	}

	// assignment from a PGresult*
	void operator=(PGresult* pgresult) {
		pgresult_ = pgresult;
		// var("Got pgresult ... ").output();
	}

	// implicit conversion to a PGresult*
	operator PGresult*() const {
		return pgresult_;
	};

	// destructor calls PQClear
	~PGResult() {
		// var("Cleared pgresult").outputl();
		if (pgresult_ != nullptr)
			PQclear(pgresult_);
	}
};

int get_mvconn_no(const var& dbhandle) {

	if (!dbhandle.assigned()) {
		// var("get_mvconn_no() returning 0 - unassigned").outputl();
		return 0;
	}
	var mvconn_no = dbhandle.a(2);
	if (mvconn_no.isnum()) {
		/// var("get_mvconn_no() returning " ^ mvconn_no).outputl();
		return mvconn_no;
	}

	// var("get_mvconn_no() returning 0 - not numeric").outputl();

	return 0;
}

int get_mvconn_no_or_default(const var& dbhandle) {
	// first return connection id if this is a connection handle
	//	if (THIS_IS_DBCONN())
	//		return (int) var_int;

	int mvconn_no2 = get_mvconn_no(dbhandle);
	if (mvconn_no2)
		return mvconn_no2;

	// otherwise return thread default connection id
	int mvconn_no = thread_default_data_mvconn_no;
	mvconn_no2 = 0;
	if (mvconn_no) {
		mvconn_no2 = mvconn_no;
		//(var("get_mvconn_no_or_default found default thread connection id ") ^
		// mvconn_no2).outputl();
	}

	// otherwise do a default connect and do setdefaultconnection
	else {
		//std::cerr << "get_mvconn_no_or_default didnt find default thread connection" << std::endl;
		// id");
		var conn1;
		if (conn1.connect()) {

			//conn1.setdefaultconnectionid();
		    // connection number should be in field 2
		    mvconn_no2 = get_mvconn_no(conn1);

		    // save current connection handle number as thread specific handle no
		    thread_default_data_mvconn_no = mvconn_no;
			if (GETDBTRACE) {
				var(mvconn_no).logputl("DBTR NEW DEFAULT CONN FOR DATA ");
			}
		}
	}

	// turn this into a db connection (int holds the connection number)
	// leave any string in place but prevent it being used as a number
	// var_int = mvconn_no2;
	////var_str = ""; does it ever need initialising?
	// var_typ = VARTYP_NANSTR_DBCONN;

	// save the connection id
	// this->r(2,mvconn_no2);

	return mvconn_no2;
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
PGconn* get_pgconnection(const var& dbhandle) {

	// var("--- connection ---").outputl();
	// get the connection associated with *this
	int mvconn_no = get_mvconn_no(dbhandle);
	// var(mvconn_no).outputl("mvconn_no1=");

	// otherwise get the default connection
	if (!mvconn_no) {

		bool isdict = dbhandle.unassigned() ? false : dbhandle.starts("dict_");

		if (isdict)
			mvconn_no = thread_default_dict_mvconn_no;
		else
			mvconn_no = thread_default_data_mvconn_no;

		// var(mvconn_no).outputl("mvconn_no2=");

		// otherwise try the default connection
		if (!mvconn_no) {

			var defaultdb;

			//look for dicts in the following order
			//1. $EXO_DICTDBNAME if defines
			//2. db "dict" if present
			//3. the default db connection
			if (isdict) {
				defaultdb.osgetenv("EXO_DICTDBNAME");
				if (!defaultdb)
					defaultdb="exodus_dict";
			} else {
				defaultdb = "";
			}
			//TRACE(defaultdb)

			//try to connect
			if (defaultdb.connect())
				mvconn_no = get_mvconn_no(defaultdb);

			//if cannot connect then for dictionaries look on default connection
			else if (isdict) {

				//attempt a default connection if not already done
				if (!thread_default_data_mvconn_no) {
					defaultdb = "";
					defaultdb.connect();
				}

				mvconn_no = thread_default_data_mvconn_no;
			}

			//save default dict/data connections
			if (isdict) {
				thread_default_dict_mvconn_no = mvconn_no;
				if (GETDBTRACE) {
					var(mvconn_no).logputl("DBTR NEW DEFAULT CONN FOR DICT ");
				}
			}
			else {
				thread_default_data_mvconn_no = mvconn_no;
				if (GETDBTRACE) {
					var(mvconn_no).logputl("DBTR NEW DEFAULT CONN FOR DATA ");
				}
			}
		}

		//save the connection number in the dbhandle
		//if (mvconn_no) {
		//	dbhandle.unassigned("");
		//	dbhandle.r(2, mvconn_no);
		//}

	}

	// otherwise error
	if (!mvconn_no)
		throw MVDBException("pgconnection() requested when not connected");

	if (GETDBTRACE) {
		std::cout << std::endl;
		PGconn* pgconn=thread_connections.get_pgconnection(mvconn_no);
		std::clog << "CONN " << mvconn_no << " " << pgconn << std::endl;
	}

	// return the relevent pg_connection structure
	return thread_connections.get_pgconnection(mvconn_no);

}

// gets lock_table, associated with connection, associated with this object
MVConnection* get_mvconnection(const var& dbhandle) {
	int mvconn_no = get_mvconn_no_or_default(dbhandle);
	if (!mvconn_no)
		throw MVDBException("get_mvconnection() attempted when not connected");
	return thread_connections.get_mvconnection(mvconn_no);
}

void var::lasterror(const var& msg) const {
	// no checking for speed
	// THISIS("void var::lasterror(const var& msg")
	// ISSTRING(msg)

	// tcache_get (tc_idx=12) at malloc.c:2943
	// 2943    malloc.c: No such file or directory.
	// You have heap corruption somewhere -- someone is running off the end of an array or
	// dereferencing an invalid pointer or using some object after it has been freed. EVADE
	// error for now by commenting next line

	thread_dblasterror = msg;
}

var var::lasterror() const {
	return thread_dblasterror ?: "";
}

static bool get_pgresult(const var& sql, PGResult& pgresult, PGconn* pgconn);

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
bool msvc_PQconnectdb(PGconn** pgconn, const std::string& conninfo) {
	// connect or fail
	__try {
		*pgconn = PQconnectdb(conninfo.c_str());
	} __except (DelayLoadDllExceptionFilter(GetExceptionInformation())) {
		return false;
	}
	return true;
}

#endif

var var::build_conn_info(const var& conninfo) const {
	// priority is
	// 1) given parameters //or last connection parameters
	// 2) individual environment parameters
	// 3) environment connection string
	// 4) config file parameters
	// 5) hard coded default parameters

	var result(conninfo);
	// if no conninfo details provided then use last connection details if any
	//if (!conninfo)
	//	result = thread_connparams;

	// otherwise search for details from exodus config file
	// if incomplete connection parameters provided
	if (not result.index("host=") or not result.index("port=") or not result.index("dbname=") or
		not result.index("user=") or not result.index("password=")) {

		// discover any configuration file parameters
		// TODO parse config properly instead of just changing \n\r to spaces!
		var configfilename = "";
		var home = "";
		if (home.osgetenv("HOME"))
			configfilename = home ^ OSSLASH ^ ".config/exodus/exodus.cfg";
		else if (home.osgetenv("USERPROFILE"))
			configfilename ^= home ^ OSSLASH ^ "Exodus\\.exodus";
		var configconn = "";
		if (!configconn.osread(configfilename))
			configconn.osread("exodus.cfg");

		// discover any configuration in the environment
		var envconn = "";
		var temp;
		if (temp.osgetenv("EXO_CONNECTION") && temp)
			envconn ^= " " ^ temp;

		// specific variable are appended ie override
		if (temp.osgetenv("EXO_HOST") && temp)
			envconn ^= " host=" ^ temp;

		if (temp.osgetenv("EXO_PORT") && temp)
			envconn ^= " port=" ^ temp;

		if (temp.osgetenv("EXO_USER") && temp)
			envconn ^= " user=" ^ temp;

		if (temp.osgetenv("EXO_DBNAME") && temp)
			envconn ^= " dbname=" ^ temp;

		if (temp.osgetenv("EXO_PASSWORD") && temp)
			envconn ^= " password=" ^ temp;

		if (temp.osgetenv("EXO_TIMEOUT") && temp)
			envconn ^= " connect_timeout=" ^ temp;

		result = defaultconninfo ^ " " ^ configconn ^ " " ^ envconn ^ " " ^ result;
	}
	return result;
}

// var connection;
// connection.connect2("dbname=exodusbase");
bool var::connect(const var& conninfo) {
	THISIS("bool var::connect(const var& conninfo")
	// nb dont log/trace or otherwise output the full connection info without HIDING the
	// password
	THISISDEFINED()
	ISSTRING(conninfo)

	var fullconninfo = conninfo.trimf().trimb();

	//use *this if conninfo not specified;
	if (!fullconninfo) {
		if (this->assigned())
			fullconninfo = *this;
	}

	//add dbname= if missing
	if (fullconninfo && !fullconninfo.index("="))
		fullconninfo = "dbname=" ^ fullconninfo;

	fullconninfo = build_conn_info(fullconninfo);

	if (GETDBTRACE)
		fullconninfo.replace(R"(password\s*=\s*\w*)", "password=**********").logputl("DBTR var::connect() ");

	PGconn* pgconn;
	for (;;) {

#if defined _MSC_VER  //|| defined __CYGWIN__ || defined __MINGW32__
		if (not msvc_PQconnectdb(&pgconn, fullconninfo.var_str)) {
#if TRACING >= 1
			var libname = "libpq.dl";
			// var libname="libpq.so";
			var errmsg="ERROR: mvdbpostgres connect() Cannot load shared library " ^ libname ^
				". Verify configuration PATH contains postgres's \\bin.";
			//errmsg.errputl();
			this->lasterror(errmsg);
#endif
			return false;
		};
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

		//#if TRACING >= 3
		var errmsg = "ERROR: mvdbpostgres connect() Connection to database failed: " ^ var(PQerrorMessage(pgconn));
		errmsg ^= " ERROR: mvdbpostgres connect() Postgres connection configuration "
			"missing or incorrect. Please login.";
		//TODO remove password=somesillysecret
		errmsg ^= " " ^ fullconninfo.replace(R"(password\s*=\s*\w*)", "password=**********");

		//#endif
		//errmsg.errputl();
		this->lasterror(errmsg);

		// required even if connect fails according to docs
		PQfinish(pgconn);
		return false;
	}

// abort if multithreading and it is not supported
#ifdef PQisthreadsafe
	if (!PQisthreadsafe()) {
		// TODO only abort if environmentn>0
		throw MVDBException("connect(): Postgres PQ library is not threadsafe");
	}
#endif

	// at this point we have good new connection to database

	// cache the new connection handle
	int mvconn_no = thread_connections.add_connection(pgconn);
	//(*this) = conninfo ^ FM ^ conn_no;
	if (!this->assigned())
		(*this) = "";
	if (not this->a(1))
		this->r(1,fullconninfo.field(" ",1));
	this->r(2, mvconn_no);
	this->r(3, mvconn_no);

	if (GETDBTRACE) {
		this->logput("DBTR var::connect() OK ");
		std::clog << " " << pgconn << std::endl;
	}

	// this->outputl("new connection=");

	// set default connection - ONLY IF THERE ISNT ONE ALREADY
	if (!thread_default_data_mvconn_no) {
		thread_default_data_mvconn_no = mvconn_no;
		if (GETDBTRACE) {
			this->logputl("DBTR NEW DEFAULT CONN FOR DATA " ^ var(mvconn_no) ^ " on ");
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
	//this->sqlexec(var("SET client_min_messages = ") ^ (GETDBTRACE ? "LOG" : "WARNING"));
	this->sqlexec(var("SET client_min_messages = ") ^ (GETDBTRACE ? "LOG" : "NOTICE"));

	return true;
}

// conn1.attach("filename1^filename2...");
bool var::attach(const var& filenames) {
	THISIS("bool var::attach(const var& filenames")
	THISISDEFINED()
	ISSTRING(filenames)

	//option to attach all dict files
	var filenames2;
	if (filenames == "dict") {
		filenames2 = "";
		var allfilenames = this->listfiles();
		for (var filename : allfilenames) {
			if (filename.substr(1, 5) == "dict_") {
				filenames2 ^= filename ^ FM;
			}
		}
		filenames2.splicer(-1, 1, "");
	}
	else {
		filenames2 = filenames;
	}

	var notattached_filenames = "";
	for (var filename : filenames2) {
		//thread_file_handles[filename] = (filename ^ FM ^ mvconn_no).toString();
		var filename2 = get_normal_filename(filename);
		var file;
		if (file.open(filename2,*this)) {
			thread_file_handles[filename2] = file.var_str;
			if (GETDBTRACE)
				file.logputl("DBTR var::attach() ");
		}
		else {
			notattached_filenames ^= filename2 ^ " ";
		}
	}

	//fail if anything not attached
	if (notattached_filenames) {
		var errmsg = "ERROR: mvdbpostgres/attach: " ^ notattached_filenames.trimb().swap(FM,", ") ^" cannot be attached on connection " ^ (*this).a(1).quote();
		this->lasterror(errmsg);
		return false;
	}

	return true;
}

// conn1.detach("filename1^filename2...");
void var::detach(const var& filenames) {
	THISIS("bool var::detach(const var& filenames")
	THISISDEFINED()
	ISSTRING(filenames)

	for (var filename : filenames) {
		std::string filename2 = get_normal_filename(filename);
		thread_file_handles.erase(filename2);
	}
	return;
}

// if this->obj contains connection_id, then such connection is disconnected with this-> becomes UNA
// Otherwise, default connection is disconnected
void var::disconnect() {
	THISIS("bool var::disconnect()")
	THISISDEFINED()

	if (GETDBTRACE)
		(this->assigned() ? *this : var("")).logputl("DBTR var::disconnect() ");

	var mvconn_no = get_mvconn_no(*this);
	if (!mvconn_no)
		mvconn_no = thread_default_data_mvconn_no;

	if (mvconn_no) {

		//disconnect
		thread_connections.del_connection((int)mvconn_no);
		var_typ = VARTYP_UNA;

		// if we happen to be disconnecting the same connection as the default connection
		// then reset the default connection so that it will be reconnected to the next
		// connect this is rather too smart but will probably do what people expect
		if (mvconn_no == thread_default_data_mvconn_no) {
			thread_default_data_mvconn_no = 0;
			if (GETDBTRACE) {
				var(mvconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DATA ");
			}
		}

		// if we happen to be disconnecting the same connection as the default connection FOR DICT
		// then reset the default connection so that it will be reconnected to the next
		// connect this is rather too smart but will probably do what people expect
		if (mvconn_no == thread_default_dict_mvconn_no) {
			thread_default_dict_mvconn_no = 0;
			if (GETDBTRACE) {
				var(mvconn_no).logputl("DBTR var::disconnect() DEFAULT CONN FOR DICT ");
			}
		}
	}
	return;
}

void var::disconnectall() {
	THISIS("bool var::disconnectall()")
	THISISDEFINED()

	var mvconn_no = get_mvconn_no(*this);
	if (!mvconn_no)
		mvconn_no = 2;

	if (GETDBTRACE)
		mvconn_no.logputl("DBTR var::disconnectall() >= ");

	thread_connections.del_connections(mvconn_no);

	if (thread_default_data_mvconn_no >= mvconn_no) {
		thread_default_data_mvconn_no = 0;
		if (GETDBTRACE) {
			var(mvconn_no).logputl("DBTR var::disconnectall() DEFAULT CONN FOR DATA ");
		}
	}

	if (thread_default_dict_mvconn_no >= mvconn_no) {
		thread_default_dict_mvconn_no = 0;
		if (GETDBTRACE) {
			var(mvconn_no).logputl("DBTR var::disconnectall() DEFAULT CONN FOR DICT ");
		}
	}

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
bool var::open(const var& filename, const var& connection /*DEFAULTNULL*/) {
	THISIS("bool var::open(const var& filename, const var& connection)")
	THISISDEFINED()
	ISSTRING(filename)

	// asking to open DOS file! ok can osread/oswrite later!
	if (filename == "DOS") {
		(*this) = filename;
		return true;
	}

	//std::string filename2 = filename.a(1).normalize().lcase().convert(".", "_").var_str;
	std::string filename2 = get_normal_filename(filename);

	//determine actual connection to use
	var connection2;
	if (connection) {
		connection2 = connection;
	}
	else {

		//return attached file handle if any
	    auto entry = thread_file_handles.find(filename2);
    	if (entry != thread_file_handles.end()) {
			//(*this) = thread_file_handles.at(filename2);
			(*this) = entry->second;
			if (GETDBTRACE)
				this->logputl("DBTR open() attached ");
			return true;
		}

		//default connection will be determined by the filename eg dict_
		connection2 = filename2;

	}

	// *

	//check filename2 is a valid table or view etc.
	//var sql="select '" ^ filename2 ^ "'::regclass";
	//if (! connection.sqlexec(sql))

	// 1. look in information_schema.tables
	var sql =
		"\
		SELECT\
		EXISTS	(\
    		SELECT 	table_name\
    		FROM 	information_schema.tables\
    		WHERE\
					table_name = '" +
		filename2 +
		"'\
				)\
	";
	var result;
	connection2.sqlexec(sql, result);
	//result.convert(RM,"|").outputl("result=");

	// 2. look in materialised views
	// select matviewname from pg_matviews where matviewname = '...';
	if (result[-1] != "t") {
		sql =
			"\
			SELECT\
			EXISTS	(\
	    		SELECT 	matviewname\
	    		FROM 	pg_matviews\
	    		WHERE\
						matviewname = '" +
			filename2 +
			"'\
					)\
		";
		connection2.sqlexec(sql, result);
	}

	//failure if not found
	if (result[-1] != "t") {
		var errmsg = "ERROR: mvdbpostgres 2 open(" ^ filename.quote() ^
					 ") table does not exist.";
		this->lasterror(errmsg);
		return false;
	}
	// */

	//this->lasterror();

	// save the filename and connection no
	// memorise the current connection for this file var
	(*this) = filename2 ^ FM ^ get_mvconn_no(connection2);

	// outputl("opened filehandle");

	return true;
}

void var::close() {
	THISIS("void var::close()")
	THISISSTRING()
	/*TODO
		if (var_typ!=VARTYP_UNA) QMClose(var_int);
	*/
}

bool var::readv(const var& filehandle, const var& key, const int fieldno) {
	THISIS("bool var::readv(const var& filehandle,const var& key,const int fieldno)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

	if (!this->read(filehandle, key))
		return false;

	this->var_str = this->a(fieldno).var_str;
	return true;
}

bool var::reado(const var& filehandle, const var& key) {
	THISIS("bool var::reado(const var& filehandle,const var& key)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

	// check cache first, and return any cached record
	int mvconn_no = get_mvconn_no_or_default(filehandle);
	if (!mvconn_no)
		throw MVDBException("get_mvconn_no() failed");
	std::string cachedrecord =
		thread_connections.getrecord(mvconn_no, filehandle.a(1).var_str, key.var_str);
	if (!cachedrecord.empty()) {
		// key.outputl("cache read " ^ filehandle.a(1) ^ "=");
		//(*this) = cachedrecord;
		this->var_str = cachedrecord;
		this->var_typ = VARTYP_STR;

		//this->lasterror();

		return this->var_str.length() > 0;
	}

	// ordinary read
	bool result = this->read(filehandle, key);

	//cache the ordinary read if successful
	if (result)
		this->writeo(filehandle, key);

	return result;
}

bool var::writeo(const var& filehandle, const var& key) const {
	THISIS("bool var::writeo(const var& filehandle,const var& key)")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int mvconn_no = get_mvconn_no_or_default(filehandle);
	if (!mvconn_no)
		throw MVDBException("get_mvconn_no() failed");
	thread_connections.putrecord(mvconn_no, filehandle.a(1).var_str, key.var_str, this->var_str);

	return true;
}

bool var::deleteo(const var& key) const {
	THISIS("bool var::deleteo(const var& key)")
	THISISSTRING()
	ISSTRING(key)

	// update cache
	// virtually identical code in read and write/update/insert/delete
	int mvconn_no = get_mvconn_no_or_default(*this);
	if (!mvconn_no)
		throw MVDBException("get_mvconn_no() failed");
	thread_connections.delrecord(mvconn_no, this->a(1).var_str, key.var_str);

	return true;
}

bool var::read(const var& filehandle, const var& key) {
	THISIS("bool var::read(const var& filehandle,const var& key)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

	//amending var_str invalidates all flags
	//var_typ = VARTYP_STR;
	//this->var_typ = VARTYP_UNA;
	//this->var_str.resize(0);

	// LEAVE RECORD UNTOUCHED UNLESS RECORD IS SUCCESSFULLY READ
	// initialise the record to unassigned (actually empty string at the moment)
	// unless record and key are the same variable
	// in which case allow failure to read to leave the record (key) untouched
	//if (this != &key) {
	//	this->var_typ = VARTYP_UNA;
	//	//this->var_typ = VARTYP_STR;
	//	this->var_str.resize(0);
	//}

	// asking to read DOS file! do osread using key as osfilename!
	if (filehandle == "DOS") {
		return this->osread(key);  //.convert("\\",OSSLASH));
	}

	// asking to read DOS file! do osread using key as osfilename!
	if (filehandle == "") {
		var errmsg = "read(...) filename not specified, probably not opened.";
		this->lasterror(errmsg);
		throw MVDBException(errmsg);
		return false;
	}

	// reading a magic special key returns all keys in the file in natural order
	if (key == "%RECORDS%") {
		var sql = "SELECT key from " ^ filehandle.a(1).convert(".", "_") ^ ";";

		//PGconn* pgconn = (PGconn*)filehandle.get_pgconnection();
		auto pgconn = get_pgconnection(filehandle);
		if (pgconn == NULL)
			return false;

		PGResult pgresult;
		bool ok = get_pgresult(sql, pgresult, pgconn);

		if (!ok)
			return false;

		// *this = "";
		this->var_str.clear();
		this->var_typ = VARTYP_STR;

		var keyn;
		int ntuples = PQntuples(pgresult);
		for (int tuplen = 0; tuplen < ntuples; tuplen++) {
			if (!PQgetisnull(pgresult, tuplen, 0)) {
				var key = getresult(pgresult, tuplen, 0);
				//skip metadata keys starting and ending in % eg "%RECORDS%"
				if (key[1] != "%" && key[-1] != "%") {
					if (this->length() <= 65535) {
						if (!this->locatebyusing("AR", FM_, key, keyn))
							this->inserter(keyn, key);
					} else {
						this->var_str.append(key.var_str);
						this->var_str.push_back(FM_);
					}
				}
			}
		}

		//remove any trailing FM
		if (this->var_str.back() == FM_)
			this->var_str.pop_back();

		//this->lasterror();

		return true;
	}

	// get filehandle specific connection or fail
	auto pgconn = get_pgconnection(filehandle);
	if (!pgconn)
		return false;

	//$parameter array
	const char* paramValues[1];
	int paramLengths[1];
	// int		paramFormats[1];
	// uint32_t	binaryIntVal;

	// lower case key if reading from dictionary
	// std::string key2;
	// if (filehandle.substr(1,5).lcase()=="dict_")
	//	key2=key.lcase().var_str;
	// else
	//	key2=key.var_str;
	std::string key2 = key.normalize().var_str;

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0]=1;

	var sql = "SELECT data FROM " ^ filehandle.a(1).convert(".", "_") ^ " WHERE key = $1";

	DEBUG_LOG_SQL1
	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(), 1, /* one param */
											NULL,					/* let the backend deduce param type */
											paramValues, paramLengths,
											0,	 // text arguments
											0);	 // text results
	// paramFormats,
	// 1);	  /* ask for binary results */

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK) {
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		var errmsg =
			"read(" ^ filehandle.convert("." _FM_, "_^").quote() ^ ", " ^ key.quote() ^ ")";
		if (sqlstate == "42P01")
			errmsg ^= " File doesnt exist";
		else
			errmsg ^= var(PQerrorMessage(pgconn)) ^ " sqlstate:" ^ sqlstate;
		;
		this->lasterror(errmsg);
		throw MVDBException(errmsg);
		// return false;
	}

	if (PQntuples(pgresult) < 1) {
		//leave record (this) untouched if record cannot be read
		// *this = "";

		this->lasterror("ERROR: mvdbpostgres read() record does not exist " ^
						   key.quote());
		return false;
	}

	if (PQntuples(pgresult) > 1) {
		var errmsg = "ERROR: mvdbpostgres read() SELECT returned more than one record";
		throw MVDBException(errmsg);
		//errmsg.errputl();
		//this->lasterror(errmsg);
		//return false;
	}

	*this = getresult(pgresult, 0, 0);

	//this->lasterror();

	return true;
}

var var::hash(const unsigned long long modulus) const {
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

var var::lock(const var& key) const {
	// on postgres, repeated locks for the same thing (from the same connection) succeed and
	// stack up they need the same number of unlocks (from the same connection) before other
	// connections can take the lock unlock returns true if a lock (your lock) was released and
	// false if you dont have the lock NB return "" if ALREADY locked on this connection

	THISIS("var var::lock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	auto hash64 = mvdbpostgres_hash_filename_and_key(*this, key);

	// check if already lock in current connection

	//	ConnectionLocks* connection_locks=tss_connection_lockss.get();
	//ConnectionLocks* connection_locks = (ConnectionLocks*)this->get_lock_table();

	auto mvconnection = get_mvconnection(*this);

	if (mvconnection) {
		// if already in local lock table then dont lock on database
		// since postgres stacks up multiple locks
		// whereas multivalue databases dont
		if (mvconnection->connection_locks.find(hash64) != mvconnection->connection_locks.end())
			return "";
	}

	// parameter array
	const char* paramValues[1];
	int paramLengths[1];
	int paramFormats[1];

	//$1=advisory_lock
	paramValues[0] = (char*)&hash64;
	paramLengths[0] = sizeof(uint64_t);
	paramFormats[0] = 1;  // binary

	const char* sql = "SELECT PG_TRY_ADVISORY_LOCK($1)";

	// DEBUG_LOG_SQL1
	if (GETDBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ var(sql).swap("$1", (*this) ^ " " ^ key)).logputl("SQLL ");

	//"this" is a filehandle - get its connection
	//PGconn* pgconn = (PGconn*)this->connection();
	PGconn* pgconn = get_pgconnection(*this);
	if (!pgconn)
		return false;

	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql, 1,										 /* one param */
											NULL,										 /* let the backend deduce param type */
											paramValues, paramLengths, paramFormats, 1); /* ask for binary pgresults */

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1) {
		var errmsg = "lock(" ^ (*this) ^ ", " ^ key ^ ")\n" ^
					 var(PQerrorMessage(pgconn)) ^ "\n" ^ "PQresultStatus=" ^
					 var(PQresultStatus(pgresult)) ^ ", PQntuples=" ^
					 var(PQntuples(pgresult));
		//errmsg.errputl();
		throw MVDBException(errmsg);
		return false;
	}

	bool lockedok = *PQgetvalue(pgresult, 0, 0) != 0;

	// add it to the local lock table so we can detect double locking locally
	// since postgres will stack up repeated locks by the same process
	if (lockedok && mvconnection) {
		// register that it is locked
//#ifdef USE_MAP_FOR_UNORDERED
		std::pair<const uint64_t, int> lock(hash64, 0);
		mvconnection->connection_locks.insert(lock);
//#else
//		mvconnection->connection_locks->insert(hash64);
//#endif
	}

	return lockedok;
}

bool var::unlock(const var& key) const {

	THISIS("void var::unlock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	auto hash64 = mvdbpostgres_hash_filename_and_key(*this, key);

	// remove from local current connection connection_locks
	//	ConnectionLocks* connection_locks=tss_connection_lockss.get();
	auto mvconnection = get_mvconnection(*this);

	if (mvconnection) {
		// if not in local connection_locks then no need to unlock on database
		if (mvconnection->connection_locks.find(hash64) == mvconnection->connection_locks.end())
			return true;
		// register that it is unlocked
		mvconnection->connection_locks.erase(hash64);
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

	// DEBUG_LOG_SQL
	if (GETDBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ var(sql).swap("$1", (*this) ^ " " ^ key)).logputl("SQLU ");

	//"this" is a filehandle - get its connection
	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (!pgconn)
		return false;

	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql, 1,										 /* one param */
											NULL,										 /* let the backend deduce param type */
											paramValues, paramLengths, paramFormats, 1); /* ask for binary results */

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1) {
		var errmsg = "unlock(" ^ this->convert(_FM_, "^") ^ ", " ^ key ^ ")\n" ^
					 var(PQerrorMessage(pgconn)) ^ "\n" ^ "PQresultStatus=" ^
					 var(PQresultStatus(pgresult)) ^ ", PQntuples=" ^
					 var(PQntuples(pgresult));
		//errmsg.errputl();
		throw MVDBException(errmsg);
		return false;
	}

	return true;
}

bool var::unlockall() const {
	// THISIS("void var::unlockall() const")

	// check if any locks
	//	ConnectionLocks* connection_locks=tss_connection_lockss.get();
	//ConnectionLocks* connection_locks = (ConnectionLocks*)this->get_lock_table();
	auto mvconnection = get_mvconnection(*this);
	if (mvconnection) {
		// if local lock table is empty then dont unlock all database
		if (mvconnection->connection_locks.begin() == mvconnection->connection_locks.end())
			// TODO indicate in some global variable "OWN LOCK"
			return true;
		// register all unlocked locked
		mvconnection->connection_locks.clear();
	}

	return this->sqlexec("SELECT PG_ADVISORY_UNLOCK_ALL()");
}

// returns only success or failure
bool var::sqlexec(const var& sql) const {
	var response = -1;	//no response required
	bool ok = this->sqlexec(sql, response);
	if (!ok) {
		this->lasterror(response);
		if (response.index("syntax") || GETDBTRACE)
			response.outputl();
	}
	return ok;
}

// returns success or failure, and response = data or errmsg (response can be preset to max number of tuples)
bool var::sqlexec(const var& sqlcmd, var& response) const {
	THISIS("bool var::sqlexec(const var& sqlcmd, var& response) const")
	ISSTRING(sqlcmd)

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (!pgconn) {
		response = "Error: sqlexec cannot find thread database connection";
		return false;
	}

	// log the sql command
	if (GETDBTRACE)
		((this->assigned() ? *this : "") ^ " | " ^ sqlcmd.convert("\t"," ").trim()).logputl("SQLE ");

	// will contain any pgresult IF successful

	// NB PQexec cannot be told to return binary results
	// but it can execute multiple commands
	// whereas PQexecParams is the opposite
	PGResult pgresult = PQexec(pgconn, sqlcmd.var_str.c_str());

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK &&
		PQresultStatus(pgresult) != PGRES_TUPLES_OK) {
		//int xx = PQresultStatus(pgresult);
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		// sql state 42P03 = duplicate_cursor
		response = var(PQerrorMessage(pgconn)) ^ " sqlstate:" ^ sqlstate;
		return false;
	}

	//errmsg = var(PQntuples(pgresult));

	//quit if no rows/columns provided or no response required (integer<=0)
	int nrows = PQntuples(pgresult);
	int ncols = PQnfields(pgresult);
	if (nrows == 0 or ncols == 0 || (response.assigned() && ((response.var_typ & VARTYP_INT) && response <= 0))) {
		response = "";
		return true;
	}

	//option to limit number of rows returned
	if (response.assigned() && response.isnum() && response < nrows && response)
		nrows = response;

	response = "";

	//first row is the column names
	for (int coln = 0; coln < ncols; ++coln) {
		response.var_str.append(PQfname(pgresult, coln));
		response.var_str.push_back(FM_);
	}
	response.var_str.pop_back();

	//output the rows
	for (int rown = 0; rown < nrows; rown++) {
		response.var_str.push_back(RM_);
		for (int coln = 0; coln < ncols; ++coln) {
			response.var_str.append(PQgetvalue(pgresult, rown, coln));
			response.var_str.push_back(FM_);
		}
		response.var_str.pop_back();
	}

	return true;
}

// writev writes a specific field number in a record
//(why it is "writev" instead of "writef" isnt known!
bool var::writev(const var& filehandle, const var& key, const int fieldno) const {
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
bool var::write(const var& filehandle, const var& key) const {
	THISIS("bool var::write(const var& filehandle, const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// clear any cache
	filehandle.deleteo(key);

	// asking to write DOS file! do osread!
	if (filehandle == "DOS") {
		this->oswrite(key);	 //.convert("\\",OSSLASH));
		return true;
	}

	// std::string key2=key.var_str;
	// std::string data2=this->var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

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

	//PGconn* pgconn = (PGconn*)filehandle.get_pgconnection();
	auto pgconn = get_pgconnection(filehandle);
	if (!pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,	   // two params (key and data)
											NULL,  // let the backend deduce param type
											paramValues, paramLengths,
											0,	 // text arguments
											0);	 // text results
	// paramFormats,
	// 1);				// ask for binary results

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK) {
		var errmsg = var("ERROR: mvdbpostgres write(" ^ filehandle.convert(_FM_, "^") ^
						 ", " ^ key ^ ") failed: PQresultStatus=" ^
						 var(PQresultStatus(pgresult)) ^ " " ^
						 var(PQerrorMessage(pgconn)));
		//errmsg.errputl();
		throw MVDBException(errmsg);
		//return false;
	}

	// if not updated 1 then fail
	//return strcmp(PQcmdTuples(pgresult), "1") != 0;
	return true;
}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var::updaterecord(const var& filehandle, const var& key) const {
	THISIS("bool var::updaterecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// clear any cache
	filehandle.deleteo(key);

	// std::string key2=key.var_str;
	// std::string data2=this->var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

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

	//PGconn* pgconn = (PGconn*)filehandle.get_pgconnection();
	auto pgconn = get_pgconnection(filehandle);
	if (!pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,	   // two params (key and data)
											NULL,  // let the backend deduce param type
											paramValues, paramLengths,
											0,	 // text arguments
											0);	 // text results
	// paramFormats,	// bytea
	// 1);				// ask for binary results

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK) {
#if TRACING >= 1
		var errmsg = "ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM_, "^") ^
					 ", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
					 var(PQerrorMessage(pgconn));
		//errmsg.errputl();
		throw MVDBException(errmsg);
#endif
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0) {
#if TRACING >= 3
		var("ERROR: mvdbpostgres update(" ^ filehandle.convert(_FM_, "^") ^
			", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
			var(PQerrorMessage(pgconn)))
			.errputl();
#endif
		return false;
	}

	return true;
}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt
// already exist" logic

bool var::insertrecord(const var& filehandle, const var& key) const {
	THISIS("bool var::insertrecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	// clear any cache
	filehandle.deleteo(key);

	// std::string key2=key.var_str;
	// std::string data2=this->var_str;
	std::string key2 = key.normalize().var_str;
	std::string data2 = this->normalize().var_str;

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

	//PGconn* pgconn = (PGconn*)filehandle.get_pgconnection();
	auto pgconn = get_pgconnection(filehandle);
	if (!pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGResult pgresult = PQexecParams(pgconn,
											// TODO: parameterise filename
											sql.var_str.c_str(),
											2,	   // two params (key and data)
											NULL,  // let the backend deduce param type
											paramValues, paramLengths,
											0,	 // text arguments
											0);	 // text results
	// paramFormats,	// bytea
	// 1);				// ask for binary results

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK) {
#if TRACING >= 3
		var errmsg = "ERROR: mvdbpostgres insertrecord(" ^
					 filehandle.convert(_FM_, "^") ^ ", " ^ key ^ ") Failed: " ^
					 var(PQntuples(pgresult)) ^ " " ^
					 var(PQerrorMessage(pgconn));
		//errmsg.errputl();
		throw MVDBException(errmsg);
#endif
		return false;
	}

	// if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult), "1") != 0) {
		return false;
	}

	return true;
}

bool var::deleterecord(const var& key) const {
	THISIS("bool var::deleterecord(const var& key) const")
	THISISSTRING()
	ISSTRING(key)

	// clear any cache
	this->deleteo(key);

	// std::string key2=key.var_str;
	std::string key2 = key.normalize().var_str;

	// a one parameter array
	const char* paramValues[1];
	int paramLengths[1];
	// int		 paramFormats[1];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	// paramFormats[0] = 1;//binary

	var sql = "DELETE FROM " ^ this->a(1) ^ " WHERE KEY = $1";

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (!pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGResult pgresult = PQexecParams(pgconn, sql.var_str.c_str(), 1, /* two param */
											NULL,								   /* let the backend deduce param type */
											paramValues, paramLengths,
											0,	 // text arguments
											0);	 // text results
	// paramFormats,
	// 1);	  /* ask for binary results */

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK) {
		//#if TRACING >= 1
		var errmsg = "ERROR: mvdbpostgres deleterecord(" ^ this->convert(_FM_, "^") ^
					 ", " ^ key ^ ") Failed: " ^ var(PQntuples(pgresult)) ^ " " ^
					 var(PQerrorMessage(pgconn));
		//errmsg.errputl();
		throw MVDBException(errmsg);
		//#endif
		return false;
	}

	// if not updated 1 then fail
	bool result;
	if (strcmp(PQcmdTuples(pgresult), "1") != 0) {
#if TRACING >= 3
		var("var::deleterecord(" ^ this->convert(_FM_, "^") ^ ", " ^ key ^
			") failed. Record does not exist")
			.errputl();
#endif
		result = false;
	} else
		result = true;

	return result;
}

void var::clearcache() const {
	THISIS("bool var::clearcache() const")
	THISISDEFINED()

	int mvconn_no = get_mvconn_no_or_default(*this);
	if (!mvconn_no)
		throw MVDBException("get_mvconn_no() failed in clearcache");
	thread_connections.clearrecordcache(mvconn_no);
	return;
}

// If this is opened SQL connection, pass connection ID to sqlexec
bool var::begintrans() const {
	THISIS("bool var::begintrans() const")
	THISISDEFINED()

	this->clearcache();

	// begin a transaction
	return this->sqlexec("BEGIN");
}

bool var::rollbacktrans() const {
	THISIS("bool var::rollbacktrans() const")
	THISISDEFINED()

	this->clearcache();

	// Rollback a transaction
	return this->sqlexec("ROLLBACK");
}

bool var::committrans() const {
	THISIS("bool var::committrans() const")
	THISISDEFINED()

	this->clearcache();

	// end (commit) a transaction
	return this->sqlexec("END");
}

bool var::statustrans() const {
	THISIS("bool var::statustrans() const")
	THISISDEFINED()

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (!pgconn) {
		this->lasterror("db connection " ^ var(get_mvconn_no(*this)) ^ "not opened");
		return false;
	}

	//this->lasterror();

	// only idle is considered to be not in a transaction
	return (PQtransactionStatus(pgconn) != PQTRANS_IDLE);
}

// sample code
// var().createdb("mynewdb");//create a new database on the current thread-default connection
// var file;
// file.open("myfile");
// file.createdb("mynewdb");//creates a new db on the same connection as a file was opened on
// var connectionhandle;
// connectionhandle.connect("connection string pars");
// connectionhandle.createdb("mynewdb");

bool var::createdb(const var& dbname) const {
	return this->copydb(var(""), dbname);
}

bool var::copydb(const var& from_dbname, const var& to_dbname) const {
	THISIS("bool var::createdb(const var& from_dbname, const var& to_dbname)")
	THISISDEFINED()
	ISSTRING(from_dbname)
	ISSTRING(to_dbname)

	var sql = "CREATE DATABASE " ^ to_dbname ^ " WITH";
	sql ^= " ENCODING='UTF8' ";
	if (from_dbname)
		sql ^= " TEMPLATE " ^ from_dbname;

	return this->sqlexec(sql);
}

bool var::deletedb(const var& dbname) const {
	THISIS("bool var::deletedb(const var& dbname)")
	THISISDEFINED()
	ISSTRING(dbname)

	return this->sqlexec("DROP DATABASE " ^ dbname);
}

bool var::createfile(const var& filename) const {
	THISIS("bool var::createfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	// var tablename = "TEMP" ^ var(100000000).rnd();
	// Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but
	// has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default
	// behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON
	// COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	//std::string filename2 = filename.a(1).normalize().lcase().convert(".", "_").var_str;
	std::string filename2 = get_normal_filename(filename);

	var sql = "CREATE";
	// if (options.ucase().index("TEMPORARY")) sql ^= " TEMPORARY";
	// sql ^= " TABLE " ^ filename.convert(".","_");
	if (filename.substr(-5, 5) == "_temp")
		sql ^= " TEMP ";
	sql ^= " TABLE " + filename2;
	// sql ^= " (key bytea primary key, data bytea)";
	sql ^= " (key text primary key, data text)";

	if (this->assigned())
		return this->sqlexec(sql);
	else
		return filename.sqlexec(sql);
}

bool var::renamefile(const var& filename, const var& newfilename) const {
	THISIS("bool var::renamefile(const var& filename, const var& newfilename)")
	THISISDEFINED()
	ISSTRING(filename)
	ISSTRING(newfilename)

	var sql = "ALTER TABLE " ^ filename ^ " RENAME TO " ^ newfilename;

	if (this->assigned())
		return this->sqlexec(sql);
	else
		return filename.sqlexec(sql);
}

bool var::deletefile(const var& filename) const {
	THISIS("bool var::deletefile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	var sql = "DROP TABLE " ^ filename.a(1) ^ " CASCADE";

	if (this->assigned())
		return this->sqlexec(sql);
	else
		return filename.sqlexec(sql);
}

bool var::clearfile(const var& filename) const {
	THISIS("bool var::clearfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	var sql = "DELETE FROM " ^ filename.a(1);
	if (this->assigned())
		return this->sqlexec(sql);
	else
		return filename.sqlexec(sql);
}

inline void unquoter_inline(var& string) {
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
		throw MVDBException(datestr ^ " cannot be recognised as a date");
}
*/

inline void tosqlstring(var& string1) {
	// convert to sql style strings
	// use single quotes and double up any internal single quotes
	if (string1[1] == "\"") {
		string1.swapper("'", "''");
		string1.splicer(1, 1, "'");
		string1.splicer(-1, 1, "'");
	}
}

inline var fileexpression(const var& mainfilename, const var& filename, const var& keyordata) {
	// evade warning: unused parameter mainfilename
	if (false && mainfilename) {
	}

	// if (filename == mainfilename)
	//	return keyordata;
	// else
	return filename.convert(".", "_") ^ "." ^ keyordata;

	// if you dont use STRICT in the postgres function declaration/definitions then NULL
	// parameters do not abort functions

	// use COALESCE function in case this is a joined but missing record (and therefore null)
	// in MYSQL this is the ISNULL expression?
	// xlatekeyexpression="exodus_extract_text(coalesce(" ^ filename ^ ".data,''::text), " ^
	// xlatefromfieldname.substr(9); if (filename==mainfilename) return expression; return
	// "coalesce(" ^ expression ^", ''::text)";
}

var get_dictexpression(const var& cursor, const var& mainfilename, const var& filename, const var& dictfilename, const var& dictfile, const var& fieldname0, var& joins, var& unnests, var& selects, var& ismv, bool forsort) {

	//cursor is required to join any calculated fields in any second pass

	ismv = false;

	var fieldname = fieldname0.convert(".", "_");
	var actualdictfile = dictfile;
	if (!actualdictfile) {
		var dictfilename;
		if (mainfilename.substr(1, 5).lcase() == "dict_")
			dictfilename = "dict_voc";
		else
			dictfilename = "dict_" ^ mainfilename;

		// we should open it through the same connection, as this->was opened, not any
		// default connection
		// int mvconn_no = 0;
		// if (THIS_IS_DBCONN())
		//	mvconn_no = (int) var_int;
		// initialise the actualdictfilename to the same connection as (*this)
		//actualdictfile = (*this);
		if (!actualdictfile.open(dictfilename)) {
			dictfilename = "dict_voc";
			if (!actualdictfile.open(dictfilename)) {

				throw MVDBException("get_dictexpression() cannot open " ^
									dictfilename.quote());
#if TRACING >= 1
				var(
					"ERROR: mvdbpostgres get_dictexpression() cannot open " ^
					dictfilename.quote())
					.errputl();
#endif
				return "";
			}
		}
	}

	//if doing 2nd pass then calculated fields have been placed in a parallel temporary file
	//and their column names appended with a colon (:)
	var stage2_calculated = fieldname[-1] == ":";
	var stage2_filename = "SELECT_STAGE2_CURSOR_" ^ cursor.a(1);

	if (stage2_calculated) {
		fieldname.splicer(-1, 1, "");
		//create a pseudo look up ... except that SELECT_TEMP_CURSOR_n has the fields stored in sql columns and not in the usual data column
		stage2_calculated = "@ANS=XLATE(\"" ^ stage2_filename ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
		stage2_calculated.logputl("stage2_calculated simulation --------------------->");
	}

	// given a file and dictionary id
	// returns a postgres sql expression like (texta(filename.data,99,0,0))
	// using one of the exodus backend functions installed in postgres like textextract,
	// dateextract etc.
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
				if (not dictrec.read("dict_voc", fieldname)) {
					// try in voc uppercase
					fieldname.ucaser();
					if (not dictrec.read("dict_voc", fieldname)) {
						if (fieldname == "@ID" || fieldname == "ID")
							dictrec = "F" ^ FM ^ "0" ^ FM ^ "Ref" ^ FM ^
									  FM ^ FM ^ FM ^ FM ^ FM ^ "" ^ FM ^
									  15;
						else {
							throw MVDBException(
								"get_dictexpression() cannot read " ^
								fieldname.quote() ^ " from " ^
								actualdictfile.convert(FM, "^")
									.quote() ^
								" or \"dict_voc\"");
							//					exodus::errputl("ERROR:
							// mvdbpostgres get_dictexpression() cannot
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
	if (stage2_calculated) {
		dictrec.r(8, stage2_calculated);
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
	var isnumeric = isinteger || isdecimal || dictrec.a(9) == "R";
	var ismv1 = dictrec.a(4)[1] == "M";
	var fromjoin = false;

	var isdate = conversion[1] == "D" || conversion.substr(1, 5) == "[DATE";
	var istime = !isdate && (conversion.substr(1,2) == "MT" || conversion.substr(1, 5) == "[TIME");

	var sqlexpression;
	if (dicttype == "F") {
		// key field
		if (!fieldno) {

			if (forsort && !isdate && !istime)
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
					"exodus_extract_date(" ^ sqlexpression ^ ",0,0,0)";
			else if (istime)
				sqlexpression =
					"exodus_extract_time(" ^ sqlexpression ^ ",0,0,0)";

			return sqlexpression;
		}

		var extractargs =
			fileexpression(mainfilename, filename, "data") ^ "," ^ fieldno ^ ", 0, 0)";

		if (conversion.substr(1, 9) == "[DATETIME")
			sqlexpression = "exodus_extract_datetime(" ^ extractargs;

		else if (isdate)
			sqlexpression = "exodus_extract_date(" ^ extractargs;

		else if (istime)
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
	} else if (dicttype == "S") {

		var functionx = dictrec.a(8).trim();

		// sql expression available
		sqlexpression = dictrec.a(17);
		if (sqlexpression) {
			// return sqlexpression;
		}

		// sql function available
		// eg dict_schedules_PROGRAM_POSITION(key text, data text)
		else if (functionx.index("/"
								 "*pgsql")) {

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

		//handle below.
		else if (stage2_calculated && ismv1) {
			sqlexpression = stage2_filename ^ "." ^ fieldname ^ "_calc";
		}

		// simple join or stage2 but not on multivalued
		// stage2_calculated="@ANS=XLATE(\"SELECT_STAGE2_CURSOR_" ^ this->a(1) ^ "\",@ID," ^ fieldname ^ "_calc,\"X\")";
		else if ((!ismv1 || stage2_calculated) && functionx.substr(1, 11).ucase() == "@ANS=XLATE(") {
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
			if (xlatemode == "X" || xlatemode == "C") {

				// determine the expression in the xlate target file
				// var& todictexpression=sqlexpression;
				if (xlatetargetfieldname.isnum()) {
					sqlexpression =
						"exodus_extract_text(" ^
						fileexpression(mainfilename, xlatetargetfilename,
									   "data") ^
						", " ^ xlatetargetfieldname ^ ", 0, 0)";
				} else if (stage2_calculated) {
					sqlexpression = xlatetargetfieldname;
				} else {
					// var dictxlatetofile=xlatetargetfilename;
					// if (!dictxlatetofile.open("DICT",xlatetargetfilename))
					//	throw MVDBException("get_dictexpression() DICT" ^
					// xlatetargetfilename ^ " file cannot be opened"); var
					// ismv;
					var xlatetargetdictfilename = "dict_" ^ xlatetargetfilename;
					var xlatetargetdictfile;
					if (!xlatetargetdictfile.open(xlatetargetdictfilename))
						throw MVDBException(xlatetargetdictfilename ^ " cannot be opened for " ^ functionx);
					sqlexpression = get_dictexpression(cursor, 
						filename, xlatetargetfilename, xlatetargetdictfilename,
						xlatetargetdictfile, xlatetargetfieldname, joins, unnests,
						selects, ismv, forsort);
				}

				// determine the join details
				var xlatekeyexpression = "";
				//xlatefromfieldname.outputl("xlatefromfieldname=");
				if (xlatefromfieldname.trim().substr(1, 8).lcase() == "@record<") {
					xlatekeyexpression = "exodus_extract_text(";
					xlatekeyexpression ^= filename ^ ".data";
					xlatekeyexpression ^= ", " ^ xlatefromfieldname.substr(9);
					xlatekeyexpression.splicer(-1, 1, "");
					xlatekeyexpression ^=
						var(", 0").str(3 - xlatekeyexpression.count(',')) ^ ")";
				} else if (xlatefromfieldname.trim().substr(1, 10).lcase() == "field(@id|") {
					xlatekeyexpression = "split_part(";
					xlatekeyexpression ^= filename ^ ".key,'*',";
					xlatekeyexpression ^= xlatefromfieldname.field("|", 3).field(")", 1) ^ ")";
				}
				// TODO				if
				// (xlatefromfieldname.substr(1,8)=="FIELD(@ID)
				else if (xlatefromfieldname[1] == "{") {
					xlatefromfieldname =
						xlatefromfieldname.substr(2).splicer(-1, 1, "");
					xlatekeyexpression = get_dictexpression(cursor,
						filename, filename, dictfilename, dictfile,
						xlatefromfieldname, joins, unnests, selects, ismv, forsort);
				} else if (xlatefromfieldname == "@ID") {
					xlatekeyexpression = filename ^ ".key";
				} else {
					// throw  MVDBException("get_dictexpression() " ^
					// filename.quote() ^ " " ^ fieldname.quote() ^ " - INVALID
					// DICTIONARY EXPRESSION - " ^ dictrec.a(8).quote());
#if TRACING >= 1
					var("ERROR: mvdbpostgres get_dictexpression() " ^
						filename.quote() ^ " " ^ fieldname.quote() ^
						" - INVALID DICTIONARY EXPRESSION - " ^
						dictrec.a(8).quote())
						.errputl();
#endif
					return "";
				}

				//if the xlate key expression is stage2_calculated then
				//indicate that the whole dictid expression is stage2_calculated
				//and do not do any join
				if (xlatekeyexpression.index("exodus_call")) {
					sqlexpression = "exodus_call(";
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
			} else {
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
		else {
exodus_call:
			sqlexpression = "'" ^ fieldname ^ "'";
			int environmentn = 1;  // getenvironmentn()
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
	} else {
		// throw  filename ^ " " ^ fieldname ^ " - INVALID DICTIONARY ITEM";
		// throw  MVDBException("get_dictexpression(" ^ filename.quote() ^ ", " ^
		// fieldname.quote() ^ ") invalid dictionary type " ^ dicttype.quote());
#if TRACING >= 1
		var("ERROR: mvdbpostgres get_dictexpression(" ^ filename.quote() ^ ", " ^
			fieldname.quote() ^ ") invalid dictionary type " ^
			dicttype.quote())
			.errputl();
#endif
		return "";
	}

	// Multivalued or xref fields need special handling
	///////////////////////////////////////////////////

	ismv = ismv1;

	// vector (for GIN or indexing/filtering multivalue fields)
	//if ((ismv1 and !forsort) || fieldname.substr(-5).ucase() == "_XREF") {
	if ((ismv1 and !forsort) || fieldname.substr(-4).ucase() == "XREF") {
		//this is the sole creation of to_tsvector in mvdbpostgres.cpp
		//it will be used like to_tsvector(...) @@ to_tsquery(...)
		sqlexpression = "to_tsvector('simple'," ^ sqlexpression ^ ")";
		//sqlexpression = "to_tsvector('english'," ^ sqlexpression ^ ")";
		//sqlexpression = "string_to_array(" ^ sqlexpression ^ ",chr(29),'')";

		//multivalued prestage2_calculated field DUPLICATE CODE
		if (fieldname0[-1] == ":") {
			var joinsectionn = 1;
			var join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
			//if (!joins.a(joinsectionn).index(join))
			if (!joins.index(join))
				joins.r(joinsectionn, -1, join);
		}

	}

	// unnest multivalued fields into multiple output rows
	else if (ismv1) {

		//ismv = true;

		// var from="string_to_array(" ^ sqlexpression ^ ",'" ^ VM ^ "'";
		if (sqlexpression.substr(1, 20) == "exodus_extract_date(" || sqlexpression.substr(1, 20) == "exodus_extract_time(")
			sqlexpression.splicer(20, 0, "_array");
		else {
			sqlexpression = "string_to_array(" ^ sqlexpression ^ ", chr(29),'')";

			// Note 3rd argument '' means convert empty multivalues to NULL in the array
			// otherwise conversion to float will fail
			if (isnumeric)
				sqlexpression ^= "::float8[]";
		}

		//now do this for all fields including WHERE and ORDER BY
		//eg SELECT BOOKING_ORDERS WITH YEAR_PERIOD "21.02" AND WITH IS_LATEST AND WITH CLIENT_CODE "MIC" AND WITH @ID NOT "%]" BY ORDER_NO
		//if (forsort)
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
			if (fromjoin) {

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
			} else {

				//multivalued prestage2_calculated field DUPLICATE CODE
				if (fieldname0[-1] == ":") {
					var joinsectionn = 1;
					var join = "RIGHT JOIN " ^ stage2_filename ^ " ON " ^ stage2_filename ^ ".key = " ^ filename ^ ".key";
					//if (!joins.a(joinsectionn).index(join))
					if (!joins.index(join))
						joins.r(joinsectionn, -1, join);
				}

				// insert with SMs since expression can contain VMs
				if (!unnests.a(2).locate(fieldname)) {
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
var getword(var& remainingwords, var& ucword) {

	// gets the next word
	// or a series of words separated by FM while they are numbers or quoted strings)
	// converts to sql quoted strings
	// and clips them from the input string

	bool joinvalues = true;

	var word1 = remainingwords.field(" ", 1);
	remainingwords = remainingwords.field(" ", 2, 99999);

	// join words within quote marks into one quoted phrase
	var char1 = word1[1];
	if ((char1 == DQ || char1 == SQ)) {
		while (word1[-1] != char1 || word1.length() <= 1) {
			if (remainingwords.length()) {
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
	if (remainingwords && joinvalues && valuechars.index(word1[1])) {
		word1 = SQ ^ word1.unquote().swap("'", "''") ^ SQ;

		var nextword = remainingwords.field(" ", 1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		// to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		// duplicated above/below
		if (nextword == "and") {
			var nextword2 = remainingwords;
			if (valuechars.index(nextword2[1])) {
				nextword = nextword2;
				remainingwords = remainingwords.field(" ", 2, 99999);
			}
		}

		/*
		while (nextword && valuechars.index(nextword[1])) {
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
				if (valuechars.index(nextword2[1])) {
					nextword = nextword2;
					remainingwords = remainingwords.field(" ", 2, 99999);
				}
			}
		}
		*/
		nextword = getword(remainingwords, ucword);
		if (nextword && valuechars.index(nextword[1])) {
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
	return word1;
}

bool var::saveselect(const var& filename) {
	THISIS("bool var::saveselect(const var& filename) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(filename)

	if (GETDBTRACE)
		filename.logputl("DBTR var::saveselect() ");

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

	while (this->readnext(key, mv)) {
		recn++;

		// save a key
		(mv ^ FM ^ recn).write(file, key);
	}

	return recn > 0;
}

void to_extract_text(var& dictexpression) {
				dictexpression.replacer("^exodus_extract_number\\(", "exodus_extract_text\\(");
				dictexpression.replacer("^exodus_extract_sort\\(", "exodus_extract_text\\(");
				dictexpression.replacer("^exodus_extract_date\\(", "exodus_extract_text\\(");
				dictexpression.replacer("^exodus_extract_datetime\\(", "exodus_extract_text\\(");
}

bool var::select(const var& sortselectclause) {
	THISIS("bool var::select(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(sortselectclause)

	if (!sortselectclause || sortselectclause.substr(-2, 2) == "R)")
		return this->selectx("key, mv::integer, data", sortselectclause);
	else
		return this->selectx("key, mv::integer", sortselectclause);
}

// currently only called from select, selectrecord and getlist
// TODO merge into plain select()
bool var::selectx(const var& fieldnames, const var& sortselectclause) {
	// private - and arguments are left unchecked for speed
	//?allow undefined usage like var xyz=xyz.select();
	if (var_typ & VARTYP_MASK) {
		// throw MVUndefined("selectx()");
		this->var_str.clear();
		this->var_typ = VARTYP_STR;
	}

	// fieldnames.outputl("fieldnames=");
	// sortselectclause.outputl("sortselectclause=");

	// default to ""
	if (!(var_typ & VARTYP_STR)) {
		if (!var_typ) {
			this->var_str.clear();
			this->var_typ = VARTYP_STR;
		} else
			this->createString();
	}

	if (GETDBTRACE)
		sortselectclause.logputl("sortselectclause=");

	var actualfilename = get_normal_filename(*this);
	// actualfilename.outputl("actualfilename=");
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
	var ismv = false;

	var maxnrecs = "";
	var xx;	 // throwaway return value

	//prepare to save calculated fields that cannot be calculated by postgresql for secondary processing
	var calc_fields = "";
	//var ncalc_fields=0;
	this->r(10, "");

	//catch bad FM character
	if (sortselectclause.index(_FM_))
		throw MVDBException("Illegal FM character in " ^ sortselectclause);

	// sortselect clause can be a filehandle in which case we extract the filename from field1
	// omitted if filename.select() or filehandle.select()
	// cursor.select(...) where ...
	// SELECT (or SSELECT) nnn filename with .... and with ... by ... by
	// filename can be omitted if calling like filename.select(...) or filehandle.select(...)
	// nnn is optional limit to number of records returned
	// TODO only convert \t\r\n outside single and double quotes
	//var remaining = sortselectclause.a(1).convert("\t\r\n", "   ").trim();
	var remaining = sortselectclause.convert("\t\r\n", "   ").trim();

	// remaining.outputl("remaining=");

	// remove trailing options eg (S) or {S}
	var lastword = remaining.field2(" ", -1);
	if ((lastword[1] == "(" && lastword[-1] == ")") ||
		(lastword[1] == "{" && lastword[-1] == "}")) {
		remaining.splicer(-lastword.length() - 1, 999, "");
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
	if (firstucword.length() and firstucword.isnum()) {
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

	// actualfilename.outputl("actualfilename=");
	if (!actualfilename) {
		// this->outputl("this=");
		throw MVDBException("filename missing from select statement:" ^ sortselectclause);
	}

	while (remaining.length()) {

		// remaining.outputl("remaining=");
		// whereclause.outputl("whereclause=");
		// orderclause.outputl("orderclause=");

		var ucword;
		var word1 = getword(remaining, ucword);

		// skip options (last word and surrounded by brackets)
		// (S) etc
		// options - last word enclosed in () or {}
		if (!remaining.length() &&
			((word1[1] == "(" && word1[-1] == ")") ||
			 (word1[1] == "{" && word1[-1] == "}"))) {
			// word1.outputl("skipping last word in () options ");
			continue;
		}

		// 1. numbers or strings without leading clauses like with ... mean record keys
		// 2. value chars are " ' 0-9 . + -
		// 3. values are ignored after any with/by statements to skip the following
		//    e.g. JUSTLEN "T#20" or HEADING "..."
		else if (valuechars.index(word1[1])) {
			if (!whereclause && !orderclause) {
				if (keycodes)
					keycodes ^= FM;
				keycodes ^= word1;
			}
			continue;
		}

		// using filename
		else if (ucword == "USING" && remaining) {
			dictfilename = getword(remaining, xx);
			if (!dictfile.open("dict_" ^ dictfilename)) {
				throw MVDBException("select() dict_" ^ dictfilename ^
									" file cannot be opened");
				// exodus::errputl("ERROR: mvdbpostgres select() dict_" ^
				// dictfilename ^ " file cannot be opened"); return "";
			}
			continue;
		}

		// distinct fieldname (returns a field instead of the key)
		else if (ucword == "DISTINCT" && remaining) {

			var distinctfieldname = getword(remaining, xx);
			var distinctexpression = get_dictexpression(*this, 
				actualfilename, actualfilename, dictfilename, dictfile,
				distinctfieldname, joins, unnests, selects, ismv, false);
			var naturalsort_distinctexpression = get_dictexpression(*this, 
				actualfilename, actualfilename, dictfilename, dictfile,
				distinctfieldname, joins, unnests, selects, ismv, true);

			if (true) {
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
			continue;
		}

		// by or by-dsnd
		else if (ucword == "BY" || ucword == "BY-DSND") {
			// next word must be dictid
			var dictid = getword(remaining, xx);
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename,
								  dictfile, dictid, joins, unnests, selects, ismv, true);

			// dictexpression.outputl("dictexpression=");
			// orderclause.outputl("orderclause=");

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.index("exodus_call"))
			//if (dictexpression == "true")
			{
				if (!calc_fields.a(1).locate(dictid)) {
					//++ncalc_fields;
					calc_fields.r(1, -1, dictid);
				}
				continue;
			}

			orderclause ^= ",\n " ^ dictexpression;

			if (ucword == "BY-DSND")
				orderclause ^= " DESC";

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
			continue;
		}

		// subexpression grouping
		else if (ucword == "(" || ucword == ")") {
			whereclause ^= "\n " ^ ucword;
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
			// ATM if (word1.ucase()=="AUTHORISED") { 	if
			//(whereclause.substr(-4,4).ucase() == " AND")
			//whereclause.splicer(-4,4,""); 	continue;
			//}

			// process the dictionary id
			var forsort =
				false;	// because indexes are NOT created sortable (exodus_sort()
			var dictexpression =
				get_dictexpression(*this, actualfilename, actualfilename, dictfilename,
								  dictfile, word1, joins, unnests, selects, ismv, forsort);
			var usingnaturalorder = dictexpression.index("exodus_extract_sort");
			var dictid = word1;

			//var dictexpression_isarray=dictexpression.index("string_to_array(");
			var dictexpression_isarray = dictexpression.index("_array(");
			var dictexpression_isvector = dictexpression.index("to_tsvector(");
			//var dictexpression_isfulltext = dictid.substr(-5).ucase() == "_XREF";
			var dictexpression_isfulltext = dictid.substr(-4).ucase() == "XREF";

			// add the dictid expression
			//if (dictexpression.index("exodus_call"))
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
				if (dictexpression_isvector) {
					throw MVDBException(
						sortselectclause ^
						" 'BETWEEN x AND y' and 'FROM x TO y' ... are not currently supported for mv or xref columns");
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
				if (!valuechars.index(word1[1]) || !valuechars.index(word2[1])) {
					throw MVDBException(
						sortselectclause ^
						"BETWEEN x AND y/FROM x TO y must be followed by two values (x AND/TO y)");
				}

				if (usingnaturalorder) {
					word1 = naturalorder(word1.var_str);
					word2 = naturalorder(word2.var_str);
				}

				// no filtering in database on calculated items
				//save then for secondary filtering
				if (dictexpression.index("exodus_call")) {
					var opid = negative ? ">!<" : "><";

					//almost identical code for exodus_call above/below
					var calc_fieldn;
					if (!calc_fields.locate(dictid, calc_fieldn, 1)) {
						//++ncalc_fields;
						calc_fields.r(1, calc_fieldn, dictid);
					}

					//prevent WITH XXX appearing twice in the same sort/select clause
					//unless and until implementeda
					if (calc_fields.a(2, calc_fieldn))
						throw MVDBException("WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote());

					calc_fields.r(2, calc_fieldn, opid);
					calc_fields.r(3, calc_fieldn, word1.lowerer());
					calc_fields.r(4, calc_fieldn, word2);

					whereclause ^= " true";
					continue;
				}
				//select numrange(100,150,'[]')  @> any(string_to_array('1,2,150',',','')::numeric[]);
				if (dictexpression_isarray) {
					var date_time_numeric;
					if (dictexpression.index("date_array(")) {
						whereclause ^= " daterange(";
						date_time_numeric = "date";
					} else if (dictexpression.index("time_array(")) {
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
			if (word1[1] == "'") {

				if (word1[2] == "[") {
					word1.splicer(2, 1, "");
					prefix = ".*";

					//CONTAINING
					if (word1[-2] == "]") {
						word1.splicer(-2, 1, "");
						postfix = ".*";
					}
					//ENDING
					else {
						postfix = "$";
					}

					//STARTING
				} else if (word1[-2] == "]") {
					word1.splicer(-2, 1, "");

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
				~ 	Matches regular expression, case sensitive
					'thomas' ~ '.*thomas.*'
				~* 	Matches regular expression, case insensitive
					'thomas' ~* '.*Thomas.*'
				!~ 	Does not match regular expression, case sensitive
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
				word1.swapper("\\", "\\\\");
				var special = "[^$.|?*+()";
				for (int ii = special.length(); ii > 0; --ii) {
					if (special.index(word1[ii]))
						word1.splicer(ii, 0, "\\");
				}
				word1.swapper("'" _FM_ "'", postfix ^ "'" _FM_ "'" ^ prefix);
				word1.splicer(-1, 0, postfix);
				word1.splicer(2, 0, prefix);

				//only ops <> and != are supported when using the regular expression operator (starting/ending/containing)
				if (op == "<>")
					negative = !negative;
				else if (op != "=" and op != "")
					throw MVDBException("SELECT ... WITH " ^ op ^ " " ^ word1 ^ " is not supported. " ^ prefix.quote() ^ " " ^ postfix.quote());

				// use regular expression operator
				op = "~";
				ucword = word1;
			}

			// word1 at this point may be empty, contain a value or be the first word of an unrelated clause
			// if non-value word1 unrelated to current phrase
			if (ucword.length() && !valuechars.index(ucword[1])) {

				// push back and treat as missing value
				// remaining[1,0]=ucword:' '
				remaining.splicer(1, 0, ucword ^ " ");

				// simulate no given value .. so a boolean filter like "WITH APPROVED"
				word1 = "";
				ucword = "";
			}

			var value = word1;

			//change 'WITH SOMEMVFIELD = ""' to just 'WITH SOMEMVFIELD' to avoid ts_vector searching for nothing
			if (value == "''") {

				//remove multivalue handling - duplicate code elsewhere
				if (dictexpression.index("to_tsvector(")) {
					//dont create exodus_tobool(to_tsvector(...
					dictexpression.swapper("to_tsvector('simple',","");
					dictexpression.splicer(-1, 1, "");
					dictexpression_isvector = false;
				}

			}

			/////////////////////////////////////////////////////////////////////
			// Filter Stage 7 - SAVE INFO FOR CALCULATED FIELDS IN STAGE 2 SELECT
			/////////////////////////////////////////////////////////////////////

			// "Calculated fields" are exodus/c++ functions that cannot be run by postgres
			// and are done in exodus in mvprogram.cpp two pass select

			// no filtering in database on calculated items
			//save then for secondary filtering
			if (dictexpression.index("exodus_call"))
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
					// op.outputl("op entered:");
					negative = false;
					op = var("<> = <= >= < > !~ !~* ~ ~* ! !! !]").field(" ", aliasno);
					// op.outputl("op reversed:");
				}

				//++ncalc_fields;
				//calc_fields.r(1,ncalc_fields,dictid);
				//calc_fields.r(2,ncalc_fields,op);
				//calc_fields.r(3,ncalc_fields,value);
				//dictid = calc_fields.a(1,n);
				//op     = calc_fields.a(2,n);
				//values  = calc_fields.a(3,n);

				//almost identical code for exodus_call above/below
				var calc_fieldn;
				if (!calc_fields.locate(dictid, calc_fieldn, 1)) {
					//++ncalc_fields;
					calc_fields.r(1, calc_fieldn, dictid);
				}
				if (calc_fields.a(2, calc_fieldn))
					throw MVDBException("WITH " ^ dictid ^ " must not appear twice in " ^ sortselectclause.quote());

				//save the op
				calc_fields.r(2, calc_fieldn, op);

				//save the value(s) after removing quotes and using SM to separate values instead of FM
				calc_fields.r(3, calc_fieldn, value.unquote().swap("'" _FM_ "'", FM).convert(FM, SM));

				//place holder to be removed before issuing actual sql command
				whereclause ^= " true";

				continue;
			}

			///////////////////////////////////////////////////////////
			// Filter Stage 8 - DUMMY OP AND VALUE SAVE IF NOT PROVIDED
			///////////////////////////////////////////////////////////

			// missing op and value means NOT '' or NOT 0 or NOT NULL
			// WITH CLIENT_TYPE
			if (op == "" && value == "") {
				//op = "<>";
				//value = "''";

				//remove conversion to date/number etc
				to_extract_text(dictexpression);

				//remove conversion to array
				//eg string_to_array(exodus_extract_text(JOBS.data,6, 0, 0), chr(29),'')
				if (dictexpression.substr(1, 16) == "string_to_array(") {
					dictexpression.splicer(1, 16, "");
					dictexpression.splicer(-13, "");
				}

				//remove multivalue handling - duplicate code elsewhere
				if (dictexpression.index("to_tsvector(")) {
					//dont create exodus_tobool(to_tsvector(...
					dictexpression.swapper("to_tsvector('simple',","");
					dictexpression.splicer(-1, 1, "");
					//TRACE(dictexpression)
					dictexpression_isvector = false;
				}

				//currently tobool requires only text input
				//TODO some way to detect DATE SYMBOLIC FIELDS and not hack special dict words!
				//doesnt work on multivalued fields - results in:
				//exodus_tobool(SELECT_STAGE2_CURSOR_19397_37442_012029.TOT_SUPPINV_AMOUNT_BASE_calc, chr(29),)
				//TODO work out better way of determining DATE/TIME that must be tested versus null
				if (dictexpression.index("FULLY_") || (!dictexpression.index("exodus_extract") && dictexpression.index("_DATE")))
					dictexpression ^= " is not null";
				else
					dictexpression = "exodus_tobool(" ^ dictexpression ^ ")";
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

			// natural order value(s)
			if (usingnaturalorder)
				value = naturalorder(value.var_str);

			// without xxx = "abc"
			// with xxx not = "abc"

			// notword.outputl("notword=");
			// ucword.outputl("ucword=");

			//allow searching for text with * characters embedded
			//otherwise interpreted as glob character?
			if (dictexpression_isvector) {
				value.swapper("*", "\\*");
			}

			// STARTING
			// special processing for STARTING]
			// convert "STARTING 'ABC'"  to "BETWEEN 'X' AND 'XZZZZZZ'
			// so that any btree index if present will be used. "LIKE" or REGULAR EXPRESSIONS will not use indexes
			if (op == "]") {

				var expression = "";
				for (var& subvalue : value) {
					/* ordinary UTF8 collation strangely doesnt sort single punctuation characters along with phrases starting with the same
					   so we will use C collation which does. All so that we can use BETWEEN instead of LIKE to support STARTING WITH syntax

					Example WITHOUT collation showing % sorted in different places

					test_test=# select * from test1 order by key;
					    key    |   data    
					-----------+---------==--
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
					dictexpression.replacer("^exodus_extract_number\\(", "exodus_extract_text\\(");
					expression ^= dictexpression ^ " COLLATE \"C\"";
					expression ^= " BETWEEN " ^ subvalue ^ " AND " ^ subvalue.splice(-1, 0, "ZZZZZZ") ^ FM;
				}
				expression.splicer(-1, "");
				expression.swapper(FM, " OR ");
				value = expression;

				// indicate that the dictexpression is included in the value(s)
				op = "(";

			}

			// single value data with multiple values filter
			else if (value.index(FM) && !dictexpression_isvector) {

				//WARNING ", " is swapped in mvprogram.cpp ::select()
				//so change there if changed here
				value.swapper(FM_, ", ");

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
					value.swapper("'","''");

					//wrap everything in single quotes for sql
					value.squoter();

					//multiple with options become alternatives using to_tsquery | divider
					value.swapper(FM_, "|");

				}

				// if full text search
				//if (dictid.substr(-5).ucase() == "_XREF") {
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

						//swap all single quotes in search term for pairs of single quotes as per postgres syntax
						partvalue.swapper("'","''");

						//append postfix :* to every search word
						//so STEV:* also finds STEVE and STEVEN

						//spaces should have been converted to & before selection
						//spaces imply &
						//partvalue.swapper(" ", "&");
						//partvalue.splicer(-1, 0, ":*");

						//treat entered colons as &
						partvalue.swapper(":", "&");

						//respect any user entered AND or OR operators
						//search for all words STARTING with user defined words
						partvalue.swapper("&", ":*&");
						partvalue.swapper("|", ":*|");
						partvalue.swapper("!", ":*!");

						partvalue ^= ":*";

						values ^= "(" ^ partvalue ^ ")";
						values ^= FM;
					}
					values.splicer(-1,1,"");
					values.swapper(FM, "|");
					value = values.squote();
				}
				//select multivalues starting "XYZ" by selecting "XYZ]"
				else if (postfix) {
					value.swapper("]''", "'':*");
					value.swapper("]", ":*");
					//value.swapper("|", ":*|");
					value.splicer(-1, 0, ":*");
				}

				value.swapper("]''", "'':*");
				value.swapper("]", ":*");
				//value.splicer(-1, 0, ":*");

				//use "simple" dictionary (ie none) to allow searching for words starting with 'a'
				//use "english" dictionary for stemming (or "simple" dictionary for none)
				// MUST use the SAME in both to_tsvector AND to_tsquery
				//https://www.postgresql.org/docs/10/textsearch-dictionaries.html
				//this is the sole occurrence of to_tsquery in mvdbpostgres.cpp
				//it will be used like to_tsvector(...) @@ to_tsquery(...)
				if (value)
					value = "to_tsquery('simple'," ^ value ^ ")";
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
				if (dictexpression.index("extract_date") ||
					dictexpression.index("extract_datetime") ||
					dictexpression.index("extract_time")) {
					//if (op == "=")
					//	op = "is";
					//else
					//	op = "is not";
					//value = "null";
					dictexpression.swapper("extract_date(","extract_text(");
					dictexpression.swapper("extract_datetime(","extract_text(");
					dictexpression.swapper("extract_time(","extract_text(");
				}
				// currently number returns 0 for empty string
				//|| dictexpression.index("extract_number")
				else if (dictexpression.index("extract_number")) {
					//value = "'0'";
					dictexpression.swapper("extract_number(","extract_text(");
				}
				//horrible hack to allow filtering calculated date fields versus ""
				//TODO detect FULLY_BOOKED and FULLY_APPROVED as dates automatically
				else if (dictexpression.index("FULLY_")) {
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
					value.swapper("'", "\"");
					//convert to postrgesql array syntax
					value = "'{" ^ value ^ "}'";
				}
			}

			//////////////////////////////////////////////
			// Filter Stage 10 - COMBINE INTO WHERE CLAUSE
			//////////////////////////////////////////////

			//negate
			if (negative)
				whereclause ^= " not";

			if (op == "(")
				whereclause ^= " ( " ^ value ^ " )";
			else
				whereclause ^= " " ^ dictexpression ^ " " ^ op ^ " " ^ value;
			// whereclause.outputl("whereclause=");

		}  //with/without

	}  // getword loop

	if (calc_fields && orwith) {
		//		throw MVDBException("OR not allowed with sort/select calculated fields");
	}

	// prefix specified keys into where clause
	if (keycodes) {
		//if (keycodes.count(FM))
		{
			keycodes = actualfilename ^ ".key IN ( " ^ keycodes.swap(FM, ", ") ^ " )";

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
	if (calc_fields && actualfieldnames.substr(-6) != ", data")
		actualfieldnames ^= ", data";

	//remove mv::integer if no unnesting (sort on mv fields)
	if (!unnests) {
		// sql ^= ", 0 as mv";
		if (actualfieldnames.index("mv::integer, data")) {
			// replace the mv column with zero if selecting record
			actualfieldnames.swapper("mv::integer, data", "0::integer, data");
		} else
			actualfieldnames.swapper(", mv::integer", "");
	}

	// if any active select, convert to a file and use as an additional filter on key
	// or correctly named savelistfilename exists from getselect or makelist
	var listname = "";
	// see also listname below
	//	if (this->hasnext()) {
	//		listname=this->a(1) ^ "_" ^ getprocessn() ^ "_tempx";
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

	//determine the connection from the filename
	//could be an attached on a non-default connection
	//selecting dict files would trigger this
	//TRACE(*this)
	//TRACE(actualfilename)
	if (not this->a(2) || actualfilename.lcase().starts("dict_")) {
		var actualfile;
		if (actualfile.open(actualfilename))
			this->r(2, actualfile.a(2));
		//TRACE(actualfile)
	}
	//TRACE(*this)
	//save any active selection in a temporary table and INNER JOIN to it to avoid complete selection of primary file
	if (this->hasnext()) {

		//create a temporary sql table to hold the preselected keys
		var temptablename = "PRESELECT_TEMP_CURSOR_" ^ this->a(1);
		var createtablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
		//createtablesql ^= "CREATE TEMPORARY TABLE " ^ temptablename ^ "\n";
		createtablesql ^= "CREATE TABLE " ^ temptablename ^ "\n";
		createtablesql ^= " (KEY TEXT)\n";
		var errmsg;
		if (!this->sqlexec(createtablesql, errmsg)) {
			throw MVDBException(errmsg);
		}

		//readnext the keys into a temporary table
		var key;
		while (this->readnext(key)) {
			//std::cout<<key<<std::endl;
			this->sqlexec("INSERT INTO " ^ temptablename ^ "(KEY) VALUES('" ^ key.swap("'", "''") ^ "')");
		}

		if (this->a(3))
			debug();
		//must be empty!

		joins.inserter(1, 1, "\n RIGHT JOIN " ^ temptablename ^ " ON " ^ temptablename ^ ".key = " ^ actualfilename ^ ".key");
	}

	// assemble the full sql select statement:

	//DECLARE - cursor
	// WITH HOLD is a very significant addition
	// var sql="DECLARE cursor1_" ^ (*this) ^ " CURSOR WITH HOLD FOR SELECT " ^ actualfieldnames
	// ^ " FROM ";
	//TRACE(*this);
	var sql = "DECLARE\n cursor1_" ^ this->a(1) ^ " SCROLL CURSOR WITH HOLD FOR";

	//SELECT - field/column names
	sql ^= " \nSELECT\n " ^ actualfieldnames;
	if (selects)
		sql ^= selects;

	//FROM - filename and any specially related files
	sql ^= " \nFROM\n " ^ actualfilename;

	//JOIN - (1)?
	if (joins.a(1))
		sql ^= " " ^ joins.a(1).convert(VM, "\n");

	//UNNEST - mv fields
	//mv fields get added to the FROM clause like "unnest() as xyz" allowing the use of xyz in WHERE/ORDER BY
	//should only be one unnest (parallel mvs if more than one) since it is not clear how sselect by mv by mv2 should work if they are not in parallel
	if (unnests) {
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
	if (orderclause && !calc_fields)
		sql ^= " \nORDER BY \n" ^ orderclause.substr(3);

	//LIMIT - number of records returned
	// no limit initially if any calculated items - limit will be done in secondary sort/select
	if (maxnrecs && !calc_fields)
		sql ^= " \nLIMIT\n " ^ maxnrecs;

	//sql.outputl("sql=");

	// DEBUG_LOG_SQL
	// if (GETDBTRACE)
	//	exodus::logputl(sql);

	// first close any existing cursor with the same name, otherwise cannot create  new cursor
	if (this->cursorexists()) {
		var sql = "";
		sql ^= "CLOSE cursor1_";
		if (this->assigned())
			sql ^= this->a(1);

		var errmsg;
		if (!this->sqlexec(sql, errmsg)) {

			if (errmsg)
				errmsg.outputl("::selectx: " ^ sql ^ "\n" ^ errmsg);
			// return false;
		}
	}

	var errmsg;
	if (!this->sqlexec(sql, errmsg)) {

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
		calc_fields.r(5, dictfilename.lower());
		calc_fields.r(6, maxnrecs);
		this->r(10, calc_fields.lower());
	}

	return true;
}

void var::clearselect() {

	// THISIS("void var::clearselect() const")
	// THISISSTRING()

	// default cursor is ""
	this->unassigned("");

	/// if readnext through string
	//3/4/5/6 setup in makelist. cleared in clearselect
	//if (this->a(3) == "%MAKELIST%")
	{
		this->r(6, "");
		this->r(5, "");
		this->r(4, "");
		this->r(3, "");
		//		return;
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

	var errors;

	//delete any temporary sql table created to hold preselected keys
	//if (this->assigned())
	//{
	//	var temptablename="PRESELECT_TEMP_CURSOR_" ^ this->a(1);
	//	var deletetablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	//	if (!this->sqlexec(deletetablesql, errors))
	//	{
	//		if (errors)
	//			errors.outputl("::clearselect " ^ errors);
	//		return;
	//	}
	//}

	var sql = "";
	// sql^="DECLARE BEGIN ";
	sql ^= "CLOSE cursor1_";
	if (this->assigned())
		sql ^= this->a(1);
	// sql^="\nEXCEPTION WHEN\n invalid_cursor_name\n THEN";
	// sql^="\nEND";

	//sql.output();

	if (!this->sqlexec(sql, errors)) {
		if (errors)
			errors.outputl("::clearselect " ^ errors);
		return;
	}

	return;
}

// NB global not member function
//	To make it var:: privat member -> pollute mv.h with PGresultptr :(
// bool readnextx(const std::string& cursor, PGresultptr& pgresult)
// called by readnext (and perhaps hasnext/select to implement LISTACTIVE)
bool readnextx(const var& cursor, PGResult& pgresult, PGconn* pgconn, bool forwards) {
	var sql;
	if (forwards)
		sql = "FETCH NEXT in cursor1_" ^ cursor.a(1);
	else
		sql = "FETCH BACKWARD in cursor1_" ^ cursor.a(1);

	// sql="BEGIN;" ^ sql ^ "; END";

	// execute the sql
	// cant use sqlexec here because it returns data
	// sqlexec();
	if (!get_pgresult(sql, pgresult, pgconn)) {

		// var errmsg=var(PQresultErrorMessage(pgresult));
		// throw MVDBException(errmsg);
		// cursor.clearselect();
		// return false;

		var errmsg = var(PQresultErrorMessage(pgresult));
		// errmsg.outputl("errmsg=");
		// var(pgresult).outputl("pgresult=");
		var sqlstate = "";
		if (PQresultErrorField(pgresult, PG_DIAG_SQLSTATE)) {
			sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		}
		// pgresult is NULLPTR if if get_pgresult failed but since the pgresult is needed by
		// the caller, it will be cleared by called if not NULLPTR PQclear(pgresult);

		// if cursor simply doesnt exist then see if a savelist one is available and enable
		// it 34000 - "ERROR:  cursor "cursor1_" does not exist"
		if (forwards && sqlstate == "34000") {
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

	// true = found a new key/record
	return true;
}

bool var::deletelist(const var& listname) const {
	THISIS("bool var::deletelist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	if (GETDBTRACE)
		this->logputl("DBTR var::deletelist(" ^ listname ^ ") ");

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		//skip this error for now because maybe no LISTS on some databases
		return false;
		//throw MVDBException("deletelist() LISTS file cannot be opened");

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

bool var::savelist(const var& listname) {

	THISIS("bool var::savelist(const var& listname)")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	if (GETDBTRACE)
		this->logputl("DBTR var::savelist(" ^ listname ^ ") ");

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		throw MVDBException("savelist() LISTS file cannot be opened");

	// this should not throw if the list does not exist
	this->deletelist(listname);

	var listno = 1;
	var listkey = listname;
	var block = "";
	static int maxblocksize = 1024 * 1024;

	var key;
	var mv;
	while (this->readnext(key, mv)) {

		// append the key
		block.var_str.append(key.var_str);

		// append SM + mvno if mvno present
		if (mv) {
			block.var_str.push_back(VM_);
			block.var_str.append(mv.var_str);
		}

		// save a block of keys if more than a certain size (1MB)
		if (block.length() > maxblocksize) {
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
	if (block.length() > 1) {
		block.write(lists, listkey);
		listno++;
	}

	return listno > 0;
}

bool var::getlist(const var& listname) {
	THISIS("bool var::getlist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	// THISISDEFINED()
	ISSTRING(listname)

	if (GETDBTRACE)
		listname.logputl("DBTR var::getlist(" ^ listname ^ ") ");

	//int recn = 0;
	var key;
	var mv;
	var listfilename = "savelist_" ^ listname.field(" ", 1);
	listfilename.converter("-.*/", "____");
	// return this->selectx("key, mv::integer",listfilename);

	// open the lists file on the same connection
	var lists = *this;
	if (!lists.open("LISTS"))
		throw MVDBException("getlist() LISTS file cannot be opened");

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

//TODO make it work for multiple keys or select list
bool var::formlist(const var& keys, const var& fieldno) {
	THISIS("bool var::formlist(const var& keys, const var& fieldno)")
	//?allow undefined usage like var xyz=xyz.select();
	THISISSTRING()
	ISSTRING(keys)
	ISNUMERIC(fieldno)

	if (GETDBTRACE)
		keys.logputl("DBTR var::formlist() ");

	this->clearselect();

	var record;
	if (not record.read(*this, keys)) {
		keys.outputl("formlist() cannot read " ^ (*this) ^ ", ");
		return false;
	}

	//optional field extract
	if (fieldno)
		record = record.a(fieldno).converter(VM, FM);

	this->makelist("", record);

	return true;
}

// MAKELIST would be much better called MAKESELECT
// since the most common usage is to omit listname in which case the keys will be used to simulate a
// SELECT statement Making a list can be done simply by writing the keys into the list file without
// using this function
bool var::makelist(const var& listname, const var& keys) {
	THISIS("bool var::makelist(const var& listname)")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(listname)
	ISSTRING(keys)

	if (GETDBTRACE)
		this->logputl("DBTR var::makelist(" ^ listname ^ ") ");

	// this is not often used since can be achieved by writing keys to lists file directly
	if (listname) {
		this->deletelist(listname);

		// open the lists file on the same connection
		var lists = *this;
		if (!lists.open("LISTS"))
			throw MVDBException("makelist() LISTS file cannot be opened");
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

bool var::hasnext() {

	// var xx;
	// return this->readnext(xx);

	// THISIS("bool var::hasnext() const")
	// THISISSTRING()

	// default cursor is ""
	this->unassigned("");

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->a(3);
	if (listid) {
		var keyno = this->a(5);
		keyno++;

		var key_and_mv = this->a(6, keyno);

		// true if we have another key
		if (key_and_mv.length())
			return true;

		if (GETDBTRACE)
			this->logputl("DBTR var::hasnext(" ^ listid ^ ") ");

		// otherwise try and get another block
		var lists = *this;
		if (!lists.open("LISTS"))
			throw MVDBException("hasnext() LISTS file cannot be opened");
		var listno = this->a(4);
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
	if (!this->cursorexists())
		return false;

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL) {
		// this->clearselect();
		return false;
	}

	//PGresultptr pgresult;
	PGResult pgresult;
	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);

	if (!ok) {
		// this->clearselect();
		return false;
	}

	/////////////////////////////////
	// now restore the cursor back one
	/////////////////////////////////

	PGResult pgresult2;
	//ok =
	readnextx(*this, pgresult2, pgconn, /*forwards=*/false);

	// Note that moving backwards on the first record fails because it returns no rows since it
	// moves the cursor to point ONE BEFORE the first row if (!ok) { 	return true;
	//}

	return true;
}

bool var::readnext(var& key) {
	var valueno;
	return this->readnext(key, valueno);
}

bool var::readnext(var& key, var& valueno) {

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ & VARTYP_MASK) {
		// throw MVUndefined("readnext()");
		this->var_str.clear();
		this->var_typ = VARTYP_STR;
	}

	// default cursor is ""
	this->unassigned("");

	THISIS("bool var::readnext(var& key, var& valueno) const")
	THISISSTRING()

	var record;
	return this->readnext(record, key, valueno);
}

//	PGconn* pgconn = (PGconn*)this->connection();
//	if (pgconn == NULL)
//	{
//		this->clearselect();
//		return false;
//	}
//
//	PGResult pgresult;
//	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);
//
//	//__asm__("int3");
//
//	if (not ok)
//	{
//		// end the transaction and quit
//		// no more
//		// committrans();
//		this->clearselect();
//		return false;
//	}
//
//	/* abortive code to handle unescaping returned hex/escape data	//avoid the need for this by
//	calling pqexecparams flagged for binary
//	//even in the case where there are no parameters and pqexec could be used.
//
//	//eg 90001 is 9.0.1
//	int pgserverversion=PQserverVersion(pgconn);
//	if (pgserverversion>=90001) {
//		var(pgserverversion).outputl();
//		//unsigned char *PQunescapeBytea(const unsigned char *from, size_t *to_length);
//		size_t to_length;
//		unsigned char* unescaped = PQunescapeBytea((const unsigned char*)
//	PQgetvalue(pgresult, 0, 0), &to_length); if (*unescaped)
//			key=stringfromUTF8((UTF8*)unescaped, to_length);
//		PQfreemem(unescaped);
//		return true;
//	}
//*/
//	// get the key from the first column
//	// char* data = PQgetvalue(pgresult, 0, 0);
//	// int datalen = PQgetlength(pgresult, 0, 0);
//	// key=std::string(data,datalen);
//	key = getresult(pgresult, 0, 0);
//	// key.output("key=").len().outputl(" len=");
//
//	//recursive call to skip any meta data with keys starting and ending %
//	//eg keys like "%RECORDS%" (without the quotes)
//	//similar code in readnext()
//    if (key[1] == "%" && key[-1] == "%") {
//		return this->readnext(key, valueno);
//	}
//
//	// vn is second column
//	// record is third column
//	if (PQnfields(pgresult) > 1)
//		// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
//		valueno = getresult(pgresult, 0, 1);
//	else
//		valueno = 0;
//
//	return true;
//}

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

bool var::readnext(var& record, var& key, var& valueno) {

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ & VARTYP_MASK || !var_typ) {
		// throw MVUndefined("readnext()");
		this->var_str.clear();
		this->var_typ = VARTYP_STR;
	}

	// default cursor is ""
	this->unassigned("");

	THISIS("bool var::readnext(var& record, var& key, var& valueno) const")
	THISISSTRING()
	ISDEFINED(key)
	ISDEFINED(record)

	// readnext through string of keys if provided
	// Note: code similarity between hasnext and readnext
	var listid = this->a(3);
	if (listid) {

		if (GETDBTRACE)
			this->logputl("DBTR var::readnext() ");

		record = "";
		while (true) {
			var keyno = this->a(5);
			keyno++;

			var key_and_mv = this->a(6, keyno);

			// if no more keys, try to get next block of keys, otherwise return false
			if (key_and_mv.length() == 0) {

				// makelist provides one block of keys and nothing in the lists file
				if (listid == "%MAKELIST%") {
					this->r(3, "");
					this->r(4, "");
					this->r(5, "");
					this->r(6, "");
					return false;
				}

				var lists = *this;
				if (!lists.open("LISTS"))
					throw MVDBException("readnext() LISTS file cannot be opened");

				var listno = this->a(4);
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
			key = key_and_mv.a(1, 1, 1);
			valueno = key_and_mv.a(1, 1, 2);
			return true;
		}
	}

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL)
		return "";

	PGResult pgresult;
	bool ok = readnextx(*this, pgresult, pgconn, /*forwards=*/true);

	if (!ok) {
		// end the transaction
		// no more
		// committrans();
		this->clearselect();
		return false;
	}

	// key is first column
	// char* data = PQgetvalue(pgresult, 0, 0);
	// int datalen = PQgetlength(pgresult, 0, 0);
	// key=std::string(data,datalen);
	key = getresult(pgresult, 0, 0);
	// TODO return zero if no mv in select because no by mv column

	//recursive call to skip any meta data with keys starting and ending %
	//eg keys like "%RECORDS%" (without the quotes)
	//similar code in readnext()
	if (key[1] == "%" && key[-1] == "%") {
		return readnext(record, key, valueno);
	}

	// vn is second column
	// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
	// vn is second column
	// record is third column
	if (PQnfields(pgresult) > 1)
		// valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
		valueno = getresult(pgresult, 0, 1);
	else
		valueno = 0;

	// record is third column
	if (PQnfields(pgresult) < 3) {
		//var errmsg = "readnext() must follow a select() clause with option (R)";
		//this->lasterror(errmsg);
		//throw MVDBException(errmsg);
		// return false;
		record = "";
	} else {
		record = getresult(pgresult, 0, 2);
	}

	return true;
}

bool var::createindex(const var& fieldname0, const var& dictfile) const {
	THISIS("bool var::createindex(const var& fieldname, const var& dictfile) const")
	THISISSTRING()
	ISSTRING(fieldname0)
	ISSTRING(dictfile)

	var filename = get_normal_filename(*this);
	var fieldname = fieldname0.convert(".", "_");

	// actual dictfile to use is either given or defaults to that of the filename
	var actualdictfile;
	if (dictfile.assigned() and dictfile != "")
		actualdictfile = dictfile;
	else
		actualdictfile = "dict_" ^ filename;

	// example sql
	// create index ads__brand_code on ads (exodus_extract_text(data,3,0,0));

	// throws if cannot find dict file or record
	var joins = "";	   // throw away - cant index on joined fields at the moment
	var unnests = "";  // unnests are only created for ORDER BY, not indexing or selecting
	var selects = "";
	var ismv;
	var forsort = false;
	var dictexpression = get_dictexpression(*this, filename, filename, actualdictfile, actualdictfile,
										   fieldname, joins, unnests, selects, ismv, forsort);
	// dictexpression.outputl("dictexp=");stop();

	//mv fields return in unnests, not dictexpression
	//if (unnests)
	//{
	//	//dictexpression = unnests.a(3);
	//	unnests.convert(FM,"^").outputl("unnests=");
	//}

	var sql;

	// index on calculated columns causes an additional column to be created
	if (dictexpression.index("exodus_call")) {
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
	//if (ismv || fieldname.substr(-5).lcase() == "_xref")
	if (dictexpression.index("to_tsvector("))
		sql ^= " USING GIN";
	sql ^= " (";
	sql ^= dictexpression;
	sql ^= ")";

	bool result = this->sqlexec(sql);
	if (!result)
		this->lasterror().errputl();
	return result;
}

bool var::deleteindex(const var& fieldname0) const {
	THISIS("bool var::deleteindex(const var& fieldname) const")
	THISISSTRING()
	ISSTRING(fieldname0)

	var filename = get_normal_filename(*this);
	var fieldname = fieldname0.convert(".", "_");

	// delete the index field (actually only present on calculated field indexes so ignore
	// result) deleting the index field automatically deletes the index
	var index_fieldname = "index_" ^ fieldname;
	if (var().sqlexec("alter table " ^ filename ^ " drop " ^ index_fieldname))
		return true;

	// delete the index.
	// var filename=*this;
	var sql = "drop index index__" ^ filename ^ "__" ^ fieldname;
	bool result = this->sqlexec(sql);
	if (!result)
		this->lasterror().errputl();
	return result;
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

var var::listfiles() const {
	THISIS("var var::listfiles() const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()

	// from http://www.alberton.info/postgresql_meta_info.html

	var sql =
		"SELECT table_name FROM information_schema.tables WHERE table_type = 'BASE "
		"TABLE' AND table_schema NOT IN ('pg_catalog', 'information_schema') ";

	sql ^= " UNION SELECT matviewname as table_name from pg_matviews;";

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL)
		return "";

	PGResult pgresult;
	bool ok = get_pgresult(sql, pgresult, pgconn);

	if (!ok)
		return "";

	var filenames = "";
	int nfiles = PQntuples(pgresult);
	for (int filen = 0; filen < nfiles; filen++) {
		if (!PQgetisnull(pgresult, filen, 0))
			filenames ^= FM ^ getresult(pgresult, filen, 0);
	}
	filenames.splicer(1, 1, "");

	return filenames;
}

bool var::cursorexists() {
	// THISIS("var var::cursorexists() const")
	// could allow undefined usage since *this isnt used?
	// THISISSTRING()

	// default cursor is ""
	this->unassigned("");

	// from http://www.alberton.info/postgresql_meta_info.html

	var sql = "SELECT name from pg_cursors where name = 'cursor1_" ^ this->a(1) ^ "'";
	// var sql="SELECT name from pg_cursors";

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL)
		return "";

	PGResult pgresult;
	bool ok = get_pgresult(sql, pgresult, pgconn);

	if (!ok)
		return false;

	ok = PQntuples(pgresult) > 0;

	// if (GETDBTRACE)
	//	exodus::logputl("DBTRACE: ::cursorexists() is " ^ var(ok));

	return ok;
}

var var::listindexes(const var& filename0, const var& fieldname0) const {
	THISIS("var var::listindexes(const var& filename) const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename0)
	ISSTRING(fieldname0)

	var filename = filename0.a(1);
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

	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGResult pgresult;
	bool ok = get_pgresult(sql, pgresult, pgconn);

	if (!ok)
		return "";

	var tt;
	var indexname;
	var indexnames = "";
	int nindexes = PQntuples(pgresult);
	var lc_fieldname = fieldname.lcase();
	for (int indexn = 0; indexn < nindexes; indexn++) {
		if (!PQgetisnull(pgresult, indexn, 0)) {
			indexname = getresult(pgresult, indexn, 0);
			if (indexname.substr(1, 6) == "index_") {
				tt = indexname.index("__");
				if (tt) {
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

	return indexnames;
}

var var::reccount(const var& filename0) const {
	THISIS("var var::reccount(const var& filename_or_handle_or_null) const")
	// could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename0)

	var filename = filename0 ?: (*this);

	// vacuum otherwise unreliable
	if (!this->statustrans())
		this->flushindex(filename);

	var sql = "SELECT reltuples::integer FROM pg_class WHERE relname = '";
	sql ^= filename.a(1).lcase();
	sql ^= "';";

	//PGconn* pgconn = (PGconn*)filename.get_pgconnection();
	auto pgconn = get_pgconnection(filename);
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGResult pgresult;
	bool ok = get_pgresult(sql, pgresult, pgconn);

	if (!ok)
		return "";

	var reccount = "";
	if (PQntuples(pgresult) && PQnfields(pgresult) > 0 && !PQgetisnull(pgresult, 0, 0)) {
		// reccount=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 0)));
		reccount = getresult(pgresult, 0, 0);
		reccount += 0;
	}

	return reccount;
}

var var::flushindex(const var& filename) const {
	THISIS(
		"var var::flushindex(const var& filename="
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
	//PGconn* pgconn = (PGconn*)this->connection();
	auto pgconn = get_pgconnection(*this);
	if (pgconn == NULL)
		return "";

	// execute command or return empty string
	PGResult pgresult;
	bool ok = get_pgresult(sql, pgresult, pgconn);

	if (!ok)
		return "";

	var flushresult = "";
	if (PQntuples(pgresult)) {
		flushresult = true;
	}

	return flushresult;
}

// used for sql commands that require no parameters
// returns 1 for success
// returns 0 for failure
// WARNING in either case caller MUST PQclear(pgresult)
//static bool get_pgresult(const var& sql, PGresultptr& pgresult, PGconn* pgconn)
static bool get_pgresult(const var& sql, PGResult& pgresult, PGconn* pgconn) {
	DEBUG_LOG_SQL

	/* dont use PQexec because is cannot be told to return binary results
	 and use PQexecParams with zero parameters instead
	//execute the command
	pgresult = get_pgresult(pgconn, sql.var_str.c_str());
	pgresult = pgresult;
	*/

	// parameter array but no parameters
	const char* paramValues[1];
	int paramLengths[1];
	// int		 paramFormats[1];

	// will contain any pgresult IF successful
	pgresult = PQexecParams(pgconn, sql.toString().c_str(), 0, /* zero params */
							NULL,									  /* let the backend deduce param type */
							paramValues, paramLengths,
							0,	 // text arguments
							0);	 // text results
	// paramFormats,
	// 1);	  /* ask for binary results */

	// NO! pgresult is returned to caller to extract any data

	if (!pgresult) {
#if TRACING >= 1
		var("ERROR: mvdbpostgres PQexec command failed, no error code: ").errputl();
#endif

		return false;
	} else {
		switch (PQresultStatus(pgresult)) {
			case PGRES_COMMAND_OK:
#if TRACING >= 3
				const char* str_res;
				str_res = PQcmdTuples(pgresult);
				if (strlen(str_res) > 0) {
					var("Command executed OK, " ^ var(str_res) ^ " rows.").logputl();
				} else {
					var("Command executed OK, 0 rows.").logputl();
				}
#endif

				return true;

			case PGRES_TUPLES_OK:

#if TRACING >= 3
				var(sql ^ "\nSelect executed OK, " ^ var(PQntuples(pgresult)) ^
					" rows found.")
					.logputl();
#endif

				return PQntuples(pgresult) > 0;

			case PGRES_NONFATAL_ERROR:

				//#if TRACING >= 1
				var("ERROR: mvdbpostgres SQL non-fatal error code " ^
					var(PQresStatus(PQresultStatus(pgresult))) ^ ", " ^
					var(PQresultErrorMessage(pgresult)))
					.errputl();
				//#endif

				return true;

			default:

				//#if TRACING >= 1
				if (sql.field(" ", 1) != "FETCH") {
					var("ERROR: mvdbpostgres pqexec " ^ var(sql)).errputl();
					var("ERROR: mvdbpostgres pqexec " ^
						var(PQresStatus(PQresultStatus(pgresult))) ^ ": " ^
						var(PQresultErrorMessage(pgresult)))
						.errputl();
				}
				//#endif

				// this is defaulted above for safety
				// retcode=0;
				return false;
		}

		// should never get here
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
				//needs pgexodus function not written as yet
				//sql:='text_field(key,"*",':dict<5>:')'
				//postgres only probably
				expression = "split_part(key,\'*\'," ^ dict.a(5) ^ ")";
			}
		}else{
			//if dict<7> then
			//needs pgexodus functions installed in server
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

}  // namespace exodus
