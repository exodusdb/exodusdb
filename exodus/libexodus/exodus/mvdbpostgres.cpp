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

//in case of migration to db2 or mysql here is a very detailed comparison in 2006
//http://www.ibm.com/developerworks/data/library/techarticle/dm-0606khatri/

//http://www.postgresql.org/docs/8.3/static/libpq-envars.html
//PGHOST/PGHOSTADDR
//PGPORT
//PGDATABASE
//PGUSER
//PGPASSWORD
//PGPASSFILE defaults to ~/.pgpass
//PGSERVICE in pg_service.conf in PGSYSCONFDIR

//TODO sql quoting of all parameters like dbname filename etc to prevent sql injection.

//0=silent, 1=errors, 2=warnings, 3=results, 4=tracing, 5=debugging
//0=silent, 1=errors, 2=warnings, 3=failures, 4=successes, 5=debugging ?

//MSVC requires exception handling (eg compile with /EHsc or EHa?) for delayed dll loading detection

////ALN:TODO: REFACTORING NOTES
// Proposed to split content of mvdbpostres.cpp into 3 layers (classic approach):
//	mvdbfuncs.cpp - api things, like mvglobalfuncs.cpp
//	mvdbdrv.cpp - base abstract class mv_db_drv or MvDbDrv to define db access operations (db driver interface);
//	mvdbdrvpostgres.cpp - subclass of mv_db_drv, specific to PostgreSQL things like PGconn and PQfinish;
//	mvdbdrvmsde.cpp - possible subclass of mv_db_drv, specific to MSDE (AKA MSSQL Express);
//	mvdblogic.cpp - intermediate processing (most of group 2) functions.
// Proposed refactoring would:
//		- improve modularity of the Exodus platform;
//		- allow easy expanding to other DB engines.

#ifndef DEBUG
# define TRACING 1
#else
# define TRACING 2
#endif

#if defined _MSC_VER // || defined __CYGWIN__ || defined __MINGW32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <DelayImp.h>
 //doesnt seem to work, add them to visual studio project delayload section using semicolons or special linker options
	//#pragma comment(linker, "/DelayLoad:libpq.dll")
#else
//# define __try try
//# define __except catch
#endif

#pragma warning (disable: 4150)
//warning C4150: deletion of pointer to incomplete type 'pg_conn'; no destructor called
//see declaration of 'pg_conn'

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <iostream>
#include <cstring>//for strcmp strlen
#include <boost/locale.hpp>

#include <boost/thread/tss.hpp>
//http://beta.boost.org/doc/libs/1_41_0/doc/html/unordered.html

//see exports.txt for all PQ functions
//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h>//in postgres/include

#include <arpa/inet.h>//for ntohl()
#define MV_NO_NARROW

//#include "MurmurHash2_64.h"			// it has included in mvdbconns.h (uint64_t defined)

#include <exodus/mvdbconns.h>		// placed as last include, causes boost header compiler errors
#include <exodus/mvimpl.h>
#include <exodus/mv.h>
//#include <exodus/mvenvironment.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus {

bool startipc();

// Deleter function to close connection and connection cache object
static void connection_DELETER_AND_DESTROYER(CACHED_CONNECTION con_)
{
	PGconn * pgp = (PGconn *) con_;
	PQfinish(pgp);		// AFAIK, it destroys the object by pointer
//	delete pgp;
}
static MvConnectionsCache mv_connections_cache(connection_DELETER_AND_DESTROYER);

//DBTRACE is set in exodus_main (console programs) but not when used as a plain library
//so initialise it on the fly. assume that it will usually be less than one for not tracing
#define GETDBTRACE (DBTRACE>=0&&getdbtrace())
bool getdbtrace()
{
	if (DBTRACE==0)
		DBTRACE=var().osgetenv(L"EXO_DBTRACE")?1:-1;
	return DBTRACE>0;
}

//#if TRACING >= 5
#define DEBUG_LOG_SQL if (GETDBTRACE) {exodus::logputl(L"SQL:" ^ var(sql));}
#define DEBUG_LOG_SQL1 if (GETDBTRACE) {exodus::logputl(L"SQL:" ^ var(sql).swap(L"$1",L"'"^var(paramValues[0])^L"'"));}
//#else
//#define DEBUG_LOG_SQL
//#define DEBUG_LOG_SQL1
//#endif

//this front end C interface is based on postgres
//http://www.postgresql.org/docs/8.2/static/libpq-exec.html
//doc/postgres/libpq-example.html
//src/test/examples/testlibpq.c

//(backend pg functions extract and dateextract are based
// on samples in src/tutorial/funcs_new.c)

//SQL EXAMPLES
//create or replace view testview as select exodus_extract_bytea(data,1,0,0) as field1 from test;
//create index testfield1 on test (exodus_extract_bytea(data,1,0,0));
//select * from testview where field1  > 'aaaaaaaaa';
//analyse;
//explain select * from testview where field1  > 'aaaaaaaaa';
//explain analyse select * from testview where field1  > 'aaaaaaaaa';e

//#include <pg_type.h>
#define BYTEAOID 17;
#define TEXTOID 25;

//this is not threadsafe
//PGconn	 *thread_pgconn;
//but this is ...
boost::thread_specific_ptr<int> tss_pgconnids;
boost::thread_specific_ptr<var> tss_pgconnparams;
boost::thread_specific_ptr<var> tss_pglasterror;
boost::thread_specific_ptr<bool> tss_ipcstarted;


std::wstring getresult(PGresult* pgresult, int rown, int coln) {
	char* starting=PQgetvalue(pgresult,rown,coln);
	return boost::locale::conv::utf_to_utf<wchar_t>(starting,starting+PQgetlength(pgresult,rown,coln));
}

class Resultclearer {

//Save pgresultptr in this class and to guarantee that it will be PQClear'ed on function exit
//Resultclearer clearer(pgresult);

public:

Resultclearer(PGresult* pgresult) : pgresult_(pgresult) {
	//var(L"Got pgresult ... ").output();
}

~Resultclearer(){
	//var(L"Cleared pgresult").outputl();
	if (pgresult_ != nullptr)
		PQclear(pgresult_);
}

private:
	PGresult* pgresult_=nullptr;
};

void var::setlasterror(const var& msg) const
{
	//no checking for speed
	//THISIS(L"void var::setlasterror(const var& msg")
	//ISSTRING(msg)

	//tcache_get (tc_idx=12) at malloc.c:2943
	//2943    malloc.c: No such file or directory.
	//You have heap corruption somewhere -- someone is running off the end of an array or dereferencing an invalid pointer or using some object after it has been freed.
	//EVADE error for now by commenting next line

	tss_pglasterror.reset(new var(msg));
}

void var::setlasterror() const
{
	tss_pglasterror.reset();
}

var var::getlasterror() const
{
	if (tss_pglasterror.get())
		return *tss_pglasterror.get();
	else
		return L"";
}

typedef PGresult* PGresultptr;
static bool getpgresult(const var& sql, PGresultptr& pgresult, PGconn * thread_pgconn);

bool var::sqlexec(const var& sql) const
{
	var errmsg;
	bool ok = this->sqlexec(sql, errmsg);
	if (not ok) {
		this->setlasterror(errmsg);
		if (errmsg.index(L"syntax") || GETDBTRACE)
			errmsg.outputl();
	}
	return ok;
}

#if defined _MSC_VER //|| defined __CYGWIN__ || defined __MINGW32__
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

   return(lDisposition);
}

//msvc uses a special mode to catch failure to load a delay loaded dll that is incompatible with the normal try/catch and
//needs to be put in simple global function with no complex objects (that require standard c++ try/catch stack unwinding?)
//maybe it would be easier to manually load it using dlopen/dlsym implemented in var as var::load and var::call
//http://msdn.microsoft.com/en-us/library/5skw957f(vs.80).aspx
bool msvc_PQconnectdb(PGconn** pgconn, const std::string& conninfo)
{
	//connect or fail
	__try
	{
		*pgconn=PQconnectdb(conninfo.c_str());
	}
	__except(DelayLoadDllExceptionFilter(GetExceptionInformation()))
	{
		return false;
	}
	return true;
}

#endif

//the idea is for exodus to have access to one standard database without secret password
static var defaultconninfo= L"host=127.0.0.1 port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10";

var var::build_conn_info(const var& conninfo) const
{
	//priority is
	//1) given parameters or last connection parameters
	//2) individual environment parameters
	//3) environment connection string
	//4) config file parameters
	//5) default parameters

	var result(conninfo);
	//if no conninfo details provided then use last connection details if any
	if (!conninfo && tss_pgconnparams.get())
		result = *tss_pgconnparams.get();

	//otherwise search for details from exodus config file
	//if incomplete connection parameters provided
	if (not result.index(L"host=") or
		not result.index(L"port=") or
		not result.index(L"dbname=") or
		not result.index(L"user=") or
		not result.index(L"password=")
		)
	{

		//discover any configuration file parameters
		//TODO parse config properly instead of just changing \n\r to spaces!
		var configfilename=L"";
		var home=L"";
		if (home.osgetenv(L"HOME"))
			configfilename=home^SLASH^L".exodus";
		else if (home.osgetenv(L"USERPROFILE"))
			configfilename^=home^SLASH^L"Exodus\\.exodus";
		var configconn=L"";
		if (!configconn.osread(configfilename))
			configconn.osread(".exodus");

		//discover any configuration in the environment
		var envconn=L"";
		var temp;
		if (temp.osgetenv(L"EXO_CONNECTION"))
			envconn^=L" "^temp;

		//specific variable are appended ie override
		if (temp.osgetenv(L"EXO_HOST"))
			envconn^=L" host="^temp;

		if (temp.osgetenv(L"EXO_PORT"))
			envconn^=L" port="^temp;

		if (temp.osgetenv(L"EXO_USER"))
			envconn^=L" user="^temp;

		if (temp.osgetenv(L"EXO_DBNAME"))
			envconn^=L" dbname="^temp;

		if (temp.osgetenv(L"EXO_PASSWORD"))
			envconn^=L" password="^temp;

		if (temp.osgetenv(L"EXO_TIMEOUT"))
			envconn^=L" connect_timeout="^temp;

		result = defaultconninfo^L" "^configconn^L" "^envconn^L" "^result;
	}
	return result;
}

// var connection;
// connection.connect2("dbname=exodusbase");
bool var::connect(const var& conninfo)
{
	THISIS(L"bool var::connect(const var& conninfo")
	//nb dont log/trace or otherwise output the full connection info without HIDING the password
	THISISDEFINED()
	ISSTRING(conninfo)

	var conninfo2 = build_conn_info(conninfo);

	if (GETDBTRACE)
		exodus::logputl(L"DBTRACE:" ^ conninfo2);

	PGconn* pgconn;
	for(;;)
	{
#		if defined _MSC_VER //|| defined __CYGWIN__ || defined __MINGW32__
			if (not msvc_PQconnectdb(&pgconn,conninfo2.toString()))
			{
#				if TRACING >= 1
					var libname=L"libpq.dll";
					//var libname=L"libpq.so";
					exodus::errputl(L"ERROR: mvdbpostgres connect() Cannot load shared library " ^ libname ^ L". Verify configuration PATH contains postgres's \\bin.");
#				endif
				return false;
			};
#		else
			pgconn=PQconnectdb(conninfo2.toString().c_str());
#		endif

		if (PQstatus(pgconn) == CONNECTION_OK || conninfo2)
			break;

		//required even if connect fails according to docs
		PQfinish(pgconn);
		//try again with default conninfo
		conninfo2=defaultconninfo;

	}

	//failed to connect so return false
	if (PQstatus(pgconn) != CONNECTION_OK)
	{
		#if TRACING >= 3
			exodus::errputl(L"ERROR: mvdbpostgres connect() Connection to database failed: " ^ var(PQerrorMessage(pgconn)));
			//if (not conninfo2)
				exodus::errputl(L"ERROR: mvdbpostgres connect() Postgres connection configuration missing or incorrect. Please login.");
		#endif

		//required even if connect fails according to docs
		PQfinish(pgconn);
		return false;
	}

	//abort if multithreading and it is not supported
	#ifdef PQisthreadsafe
		if (!PQisthreadsafe())
		{
			//TODO only abort if environmentn>0
			throw MVDBException(L"connect(): Postgres PQ library is not threadsafe");
		}
	#endif

	//at this point we have good new connection to database
	#if TRACING >= 3
		exodus::logputl(L"var::connect() Connection to database succeeded.");
	#endif

	//save a new connection handle
	var_int = mv_connections_cache.add_connection(pgconn);
	var_str = conninfo.var_str;
	var_typ = pimpl::VARTYP_NANSTR_DBCONN;

	//set default connection
	//ONLY IF THERE ISNT ONE ALREADY
	int* connid=tss_pgconnids.get();
	if (connid==NULL)
		tss_pgconnids.reset(new int((int)var_int));

	//save last connection string (used in startipc())
	tss_pgconnparams.reset(new var(conninfo2));

	//setup a thread to service callbacks from the database backend
	if (!tss_ipcstarted.get())
	{
		#if TRACING >= 3
			exodus::outputl(L"Starting IPC");
		#endif
		startipc();
	}

	//doesnt work
	//need to set PQnoticeReceiver to suppress NOTICES like when creating files
	//PQsetErrorVerbosity(pgconn, PQERRORS_TERSE);
	//but this does
	//this turns off the notice when creating tables with a primary key
	//DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, LOG, NOTICE, WARNING, ERROR, FATAL, and PANIC
	this->sqlexec(var(L"SET client_min_messages = ") ^ (GETDBTRACE  ? L"LOG" : L"WARNING"));

	return true;
}

bool var::setdefaultconnection()
{
	THISIS(L"bool var::setdefaultconnection()")
	THISISDEFINED()

	//this should be a db connection
	if (!THIS_IS_DBCONN())
		MVException(L"is not a valid connection in setdefaultconnection()");

	//save current connection handle number as thread specific handle no
	tss_pgconnids.reset(new int((int) var_int));

	return true;

}

int var::connection_id() const
{
	//first return connection id if this is a connection handle
	if (THIS_IS_DBCONN())
		return (int) var_int;

	//otherwise return thread default connection id
	int* connid=tss_pgconnids.get();
	int connid2=0;
	if (connid&&*connid!=0)
		connid2=*connid;

	//otherwise do a default connect and do setdefaultconnection
	else
	{
		var conn1;
		if (conn1.connect())
		{
			conn1.setdefaultconnection();
			connid2=(int) conn1.var_int;
		}
	}

	//turn this into a db connection (int holds the connection number)
	//leave any string in place but prevent it being used a number
	var_int = connid2;
	//var_str = L""; does it ever need initialising?
	var_typ = pimpl::VARTYP_NANSTR_DBCONN;

	return connid2;
}

//var::connection()
//1. return the associated db connection
//this could be a previously opened filevar, a previous connected connectionvar
//or any variable previously used for a default connection
//OR
//2. return the thread-default connection
//OR
//3. do a default connect if necessary
//
//NB in case 2 and 3 the connection id is recorded in the var
//use void pointer to avoid need for including postgres headers in mv.h or any fancy class hierarchy
//(assumes accurate programming by system programmers in exodus mvdb routines)
void* var::connection() const
{
	int connid = connection_id();
	return connid ? mv_connections_cache.get_connection(connid) : NULL;
}

// gets lock_table, associated with connection, associated with this object
void* var::get_lock_table() const
{
	int connid = connection_id();
	return connid ? mv_connections_cache.get_lock_table(connid) : NULL;
}


// if this->obj contains connection_id, then such connection is disconnected with this-> becomes UNA
// Otherwise, default connection is disconnected
bool var::disconnect()
{
	THISIS(L"bool var::disconnect()")
	THISISDEFINED()

#	if TRACING >= 3
		exodus::errputl(L"ERROR: mvdbpostgres disconnect() Closing connection");
#	endif

	int* default_connid=tss_pgconnids.get();

	if (THIS_IS_DBCONN())
	{
		mv_connections_cache.del_connection((int) var_int);
		var_typ=pimpl::VARTYP_UNA;
		//if we happen to be disconnecting the same connection as the default connection
		//then reset the default connection so that it will be reconnected to the next connect
		//this is rather too smart but will probably do what people expect
		if (default_connid && *default_connid==var_int)
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

//open filehandle given a filename on current thread-default connection
//we are using strict filename/filehandle syntax (even though we could use one variable for both!)
//we store the filename in the filehandle since that is what we need to generate read/write sql later
//usage example:
// var file;
// var filename="customers";
// if (not file.open(filename)) ...

//connection is optional and default connection may be used instead
bool var::open(const var& filename, const var& connection)
{
	THISIS(L"bool var::open(const var& filename, const var& connection)")
	THISISDEFINED()
	ISSTRING(filename)

	//asking to open DOS file! ok can osread/oswrite later!
	if (filename != L"DOS") {

		//$ parameter array
		const char*	paramValues[1];
		int		paramLengths[1];
		int		paramFormats[1];

		//$1=table_name
		std::string filename2=filename.lcase().convert(L".",L"_").toString();
		paramValues[0] = filename2.c_str();
		paramLengths[0] = int(filename2.length());
		paramFormats[0] = 1;//binary

		//avoid any errors because ANY errors while a transaction is in progress cause failure of the whole transaction
		//TODO should perhaps prepare pg parameters for repeated speed
		var sql=L"SELECT table_name FROM information_schema.tables WHERE table_schema='public' and table_name=$1";

		PGconn * thread_pgconn = (PGconn *) connection.connection();
		if (!thread_pgconn) {
			this->setlasterror(L"db connection not opened");
			return false;
		}

		DEBUG_LOG_SQL1
		PGresult* pgresult = PQexecParams(thread_pgconn,
			//TODO: parameterise filename
			sql.toString().c_str(),
			1,	   /* one param */
			NULL,	/* let the backend deduce param type */
			paramValues,
			paramLengths,
			paramFormats,
			1);	  /* ask for binary results */

		Resultclearer clearer(pgresult);

		if (PQresultStatus(pgresult) != PGRES_TUPLES_OK)
		{
			var errmsg=L"ERROR: mvdbpostgres 1 open(" ^ filename.quote() ^ L") failed\n" ^ var(PQerrorMessage(thread_pgconn));
//			PQclear(pgresult);
			this->setlasterror(errmsg);
#			if TRACING >= 1
			exodus::errputl(errmsg);
#			endif
			return false;
		}

		//file (table) doesnt exist
		if (PQntuples(pgresult) < 1)
		{
			var errmsg=L"ERROR: mvdbpostgres 2 open(" ^ filename.quote() ^ L") table does not exist. failed\n" ^ var(PQerrorMessage(thread_pgconn));
//			PQclear(pgresult);
			this->setlasterror(errmsg);
			return false;
		}

		if (PQntuples(pgresult) > 1)
		{
			var errmsg=L"ERROR: mvdbpostgres 3 open() SELECT returned more than one record";
//			PQclear(pgresult);
			this->setlasterror(errmsg);
#			if TRACING >= 1
			exodus::errputl(errmsg);
#			endif
			return false;
		}

//		PQclear(pgresult);
		this->setlasterror();

	}//of not DOS

	//save the filename and memorise the current connection for this file var
	var_str=filename.var_str;
	var_int = connection.var_int;
	var_typ = pimpl::VARTYP_NANSTR_DBCONN;

	return true;
}

void var::close()
{
	THISIS(L"void var::close()")
	THISISSTRING()
/*TODO
	if (var_typ!=VARTYP_UNA) QMClose(var_int);
*/
}

bool var::readv(const var& filehandle, const var& key, const int fieldno)
{
	//THISIS(L"bool var::readv(const var& filehandle, const var& key, const int fieldno)")

	if (!read(filehandle,key))
		return false;

	var_str=this->a(fieldno).var_str;
	return true;
}

bool var::read(const var& filehandle,const var& key)
{
	THISIS(L"bool var::read(const var& filehandle,const var& key)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

	//asking to read DOS file! do osread using key as osfilename!
	if (filehandle == L"DOS") {
		(*this).osread(key);//.convert(L"\\",SLASH));
		return true;
	}

	//asking to read DOS file! do osread using key as osfilename!
	if (filehandle == L"") {
		var errmsg=L"read(...) filename not specified, probably not opened.";
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		return false;
	}

	//$parameter array
	const char* paramValues[1];
	int		 paramLengths[1];
	int		 paramFormats[1];
	//uint32_t	binaryIntVal;

	//$1=key
	std::string key2=key.toString();
	paramValues[0]=key2.c_str();
	paramLengths[0]=int(key2.length());
	paramFormats[0]=1;

	var sql=L"SELECT data FROM " ^ filehandle.convert(L".",L"_") ^ L" WHERE key = $1";

	//get filehandle specific connection or fail
	PGconn* thread_pgconn = (PGconn*) filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
		//TODO: parameterise filename
		sql.toString().c_str(),
		1,	   /* one param */
		NULL,	/* let the backend deduce param type */
		paramValues,
		paramLengths,
		paramFormats,
		1);	  /* ask for binary results */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK)
 	{
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		var errmsg=L"read(" ^ filehandle.convert(L".",L"_").quote() ^ L", " ^ key.quote() ^ L")";
		if (sqlstate == L"42P01")
			errmsg ^= L" File doesnt exist";
		else
			errmsg ^= var(PQerrorMessage(thread_pgconn)) ^ L" sqlstate:" ^ sqlstate;;
		//PQclear(pgresult);
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		//return false;
	}

	if (PQntuples(pgresult) < 1)
	{
		//PQclear(pgresult);
		this->setlasterror(L"ERROR: mvdbpostgres read() record does not exist " ^ key.quote());
		return false;
	}

	if (PQntuples(pgresult) > 1)
 	{
		//PQclear(pgresult);
		var errmsg=L"ERROR: mvdbpostgres read() SELECT returned more than one record";
		exodus::errputl(errmsg);
		this->setlasterror(errmsg);
		return false;
	}

	// *this=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));
	*this=getresult(pgresult,0,0);

	//PQclear(pgresult);

	this->setlasterror();

	return true;

}

var var::hash(const unsigned long long modulus) const
{
	THISIS(L"var var::hash() const")
	THISISDEFINED()
	THISISSTRING()
	//ISNUMERIC(modulus)

	//uint64_t hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);
	uint64_t hash64=MurmurHash64((wchar_t*)var_str.data(),int(var_str.length()*sizeof(wchar_t)),0);
	if (modulus)
		return var_int=hash64 % modulus;
	else
		return var_int=hash64;
}

var var::lock(const var& key) const
{
	//on postgres, repeated locks for the same thing (from the same connection) succeed and stack up
	//they need the same number of unlocks (from the same connection) before other connections
	//can take the lock
	//unlock returns true if a lock (your lock) was released and false if you dont have the lock
	//NB return "" if ALREADY locked on this connection

	THISIS(L"var var::lock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	std::wstring fileandkey=this->convert(L".",L"_").var_str;
	fileandkey.append(L" ");
	fileandkey.append(key.var_str);

	//TODO .. provide endian identical version
	//required if and when exodus processes connect to postgres on a DIFFERENT host
	//although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the SAME host
	uint64_t hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);

	//check if already lock in current connection

//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable *) this->get_lock_table();

	if (locktable)
	{
		//if already in local lock table then dont lock on database
		//since postgres stacks up multiple locks
		//whereas multivalue databases dont
		if (((*locktable).find(hash64))!=(*locktable).end())
			return L"";
	}

	//parameter array
	const char* paramValues[1];
	int		 paramLengths[1];
	int		 paramFormats[1];

	//$1=advisory_lock
	paramValues[0]=(char*)&hash64;
	paramLengths[0]=sizeof(uint64_t);
	paramFormats[0]=1;

	const char* sql="SELECT PG_TRY_ADVISORY_LOCK($1)";

	//"this" is a filehandle - get its connection
	PGconn* thread_pgconn=(PGconn*) this->connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
						//TODO: parameterise filename
						sql,
						1,	   /* one param */
						NULL,	/* let the backend deduce param type */
						paramValues,
						paramLengths,
						paramFormats,
						1);	  /* ask for binary pgresults */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1)
 	{
		var errmsg=L"lock(" ^ (*this) ^ L", " ^ key ^ L")\n" ^ var(PQerrorMessage(thread_pgconn)) ^ L"\n"
			^ L"PQresultStatus=" ^ var(PQresultStatus(pgresult)) ^ L", PQntuples=" ^ var(PQntuples(pgresult));
		//PQclear(pgresult);//DO THIS OR SUFFER MEMORY LEAK
		exodus::errputl(errmsg);
		//throw MVException(msg);
		return false;
	}

	// *this=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));

	//std::wstring temp=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));

	bool lockedok= *PQgetvalue(pgresult, 0, 0)!=0;

	//PQclear(pgresult);

	//add it to the local lock table so we can detect double locking locally
	//since postgres will stack up repeated locks by the same process
	if (lockedok && locktable)
	{
		//register that it is locked
#		ifdef USE_MAP_FOR_UNORDERED
			std::pair<const uint64_t,int> lock(hash64,0);
			(*locktable).insert(lock);
#		else
			(*locktable).insert(hash64);
#		endif
	}

	return lockedok;
}

bool var::unlock(const var& key) const
{

	THISIS(L"void var::unlock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

	std::wstring fileandkey=var_str;
	fileandkey.append(L" ");
	fileandkey.append(key.var_str);

	//TODO .. provide endian identical version
	//required if and when exodus processes connect to postgres on a DIFFERENT host
	//although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the SAME host
	uint64_t hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);

	//remove from local current connection locktable
//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable *) this->get_lock_table();
	if (locktable)
	{
		//if not in local locktable then no need to unlock on database
		if (((*locktable).find(hash64))==(*locktable).end())
			return true;
		//register that it is unlocked
		(*locktable).erase(hash64);
	}

	//parameter array
	const char*	paramValues[1];
	int		paramLengths[1];
	int		paramFormats[1];

	//$1=advisory_lock
	paramValues[0]=(char*)&hash64;
	paramLengths[0]=sizeof(uint64_t);
	paramFormats[0]=1;

	const char* sql="SELECT PG_ADVISORY_UNLOCK($1)";

	//"this" is a filehandle - get its connection
	PGconn* thread_pgconn=(PGconn*) this->connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
						//TODO: parameterise filename
					   sql,
					   1,	   /* one param */
					   NULL,	/* let the backend deduce param type */
					   paramValues,
					   paramLengths,
					   paramFormats,
					   1);	  /* ask for binary results */

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_TUPLES_OK || PQntuples(pgresult) != 1)
 	{
		var errmsg=L"unlock(" ^ (*this) ^ L", " ^ key ^ L")\n" ^ var(PQerrorMessage(thread_pgconn)) ^ L"\n"
			^ L"PQresultStatus=" ^ var(PQresultStatus(pgresult)) ^ L", PQntuples=" ^ var(PQntuples(pgresult));
		//PQclear(pgresult);//DO THIS OR SUFFER MEMORY LEAK
		exodus::errputl(errmsg);
		//throw MVException(msg);
		return false;
	}

	//bool unlockedok= *PQgetvalue(pgresult, 0, 0)!=0;

	//PQclear(pgresult);

	return true;
}

bool var::unlockall() const
{
	//THISIS(L"void var::unlockall() const")

	//check if any locks
//	LockTable* locktable=tss_locktables.get();
	LockTable* locktable = (LockTable *) this->get_lock_table();
	if (locktable)
	{
		//if local lock table is empty then dont unlock all database
		if ((*locktable).begin()==(*locktable).end())
			//TODO indicate in some global variable "OWN LOCK"
			return true;
		//register that it is locked
		(*locktable).clear();
	}

	return this->sqlexec(L"SELECT PG_ADVISORY_UNLOCK_ALL()");
}

//returns success or failure but no data
bool var::sqlexec(const var& sqlcmd, var& errmsg) const
{
	THISIS(L"bool var::sqlexec(const var& sqlcmd, var& errmsg) const")
	ISSTRING(sqlcmd)

	PGconn * thread_pgconn = (PGconn *) connection();
	if (!thread_pgconn)
	{
		errmsg=L"Error: sqlexec cannot find thread database connection";
		return false;
	}

	//log the sql command
	if (GETDBTRACE)
	{
//		exodus::logputl(L"SQL:" ^ *this);
		var temp(L"SQL:");
		if (this->assigned())
			temp ^= *this ^ L" ";
        	temp ^= sqlcmd;
		exodus::logputl(temp);
	}


	//will contain any pgresult IF successful
	//MUST do PQclear(pgresult) after using it;

	//NB PQexec cannot be told to return binary results
	//but it can execute multiple commands
	//whereas PQexecParams is the opposite
	PGresult* pgresult = PQexec(thread_pgconn, sqlcmd.toString().c_str());

	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK && PQresultStatus(pgresult) != PGRES_TUPLES_OK) {
		int xx=PQresultStatus(pgresult);
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));
		//PQclear(pgresult);//essential
		//sql state 42P03 = duplicate_cursor
		errmsg=var(PQerrorMessage(thread_pgconn)) ^ L" sqlstate:" ^ sqlstate;
		return false;
	}

	errmsg=var(PQntuples(pgresult));
	//PQclear(pgresult);//essential
	return true;

}

//writev writes a specific field number in a record
//(why it is "writev" instead of "writef" isnt known!
bool var::writev(const var& filehandle,const var& key,const int fieldno) const
{
	if (fieldno<=0)
		return write(filehandle, key);

	THISIS(L"bool var::writev(const var& filehandle,const var& key,const int fieldno) const")
	//will be duplicated in read and write but do here to present the correct function name on error
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	//get the old record
	var record;
	if (!record.read(filehandle,key))
		record=L"";

	//replace the field
	record.r(fieldno,var_str);

	//write it back
	record.write(filehandle,key);

	return true;

}

/* "prepared statement" version doesnt seem to make much difference approx -10% - possibly because two field file is so simple
bool var::write(const var& filehandle,const var& key) const {}
*/

//there is no "update if present or insert if not" command in postgres
//sql2003 "merge" http://en.wikipedia.org/wiki/Upsert
bool var::write(const var& filehandle, const var& key) const
{
	THISIS(L"bool var::write(const var& filehandle, const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	//asking to write DOS file! do osread!
	if (filehandle == L"DOS") {
		this->oswrite(key);//.convert(L"\\",SLASH));
		return true;
	}

	std::string key2=key.toString();
	std::string data2=(*this).toString();

	//a 2 parameter array
	const char* paramValues[2];
	int		 paramLengths[2];
	int		 paramFormats[2];

	//$1 key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	paramFormats[0] = 1;//binary

	//$2 data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	paramFormats[1] = 1;//binary

	var sql;

	sql = L" INSERT INTO " ^ filehandle.convert(L".",L"_") ^ L" (key,data) values( $1 , $2)";
	sql ^= L" ON CONFLICT (key)";
	sql ^= L" DO UPDATE SET data = $2";

	PGconn * thread_pgconn = (PGconn *) filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
						//TODO: parameterise filename
					   sql.toString().c_str(),
					   2,				// two params (key and data)
					   NULL,			// let the backend deduce param type
					   paramValues,
					   paramLengths,
					   paramFormats,
					   1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#if TRACING >= 1
		exodus::errputl(L"ERROR: mvdbpostgres write() failed: PQresultStatus=" ^ var(PQresultStatus(pgresult)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
#endif
		//PQclear(pgresult);
		return false;
	}

	//if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult),"1") != 0)
	{
		//PQclear(pgresult);
 		return false;
	}

	//PQclear(pgresult);
	return true;

}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var::updaterecord(const var& filehandle,const var& key) const
{
	THISIS(L"bool var::updaterecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	std::string key2=key.toString();
	std::string data2=(*this).toString();

	//a 2 parameter array
	const char* paramValues[2];
	int		 paramLengths[2];
	int		 paramFormats[2];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	paramFormats[0] = 1;//binary

	//$2=data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	paramFormats[1] = 1;//binary

	var sql = L"UPDATE " ^ filehandle.convert(L".",L"_") ^ L" SET data = $2 WHERE key = $1";

	PGconn* thread_pgconn=(PGconn*) filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
		//TODO: parameterise filename
							  sql.toString().c_str(),
		2,				// two params (key and data)
		NULL,			// let the backend deduce param type
		paramValues,
		paramLengths,
		paramFormats,	// bytea
		1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#		if TRACING >= 1
			exodus::errputl(L"ERROR: mvdbpostgres update() Failed: "
				^ var(PQntuples(pgresult)) ^ L" "
				^ var(PQerrorMessage(thread_pgconn)));
#		endif
		//PQclear(pgresult);
		return false;
	}

	//if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult),"1") != 0)
	{
#		if TRACING >= 3
			exodus::errputl(L"ERROR: mvdbpostgres update() Failed: "
				^ var(PQntuples(pgresult)) ^ L" "
				^ var(PQerrorMessage(thread_pgconn)));
#		endif
		//PQclear(pgresult);
 		return false;
	}

	//PQclear(pgresult);
	return true;

}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt already exist" logic

bool var::insertrecord(const var& filehandle,const var& key) const
{
	THISIS(L"bool var::insertrecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	std::string key2=key.toString();
	std::string data2=(*this).toString();

	//a 2 parameter array
	const char* paramValues[2];
	int		 paramLengths[2];
	int		 paramFormats[2];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	paramFormats[0] = 1;//binary

	//$2=data
	paramValues[1] = data2.data();
	paramLengths[1] = int(data2.length());
	paramFormats[1] = 1;//binary

	var sql = L"INSERT INTO " ^ filehandle.convert(L".",L"_") ^ L" (key,data) values( $1 , $2)";

	PGconn* thread_pgconn=(PGconn*) filehandle.connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
		//TODO: parameterise filename
		sql.toString().c_str(),
		2,				// two params (key and data)
		NULL,			// let the backend deduce param type
		paramValues,
		paramLengths,
		paramFormats,	// bytea
		1);				// ask for binary results
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#		if TRACING >= 3
			exodus::errputl(L"ERROR: mvdbpostgres insertrecord() Failed: "
				^ var(PQntuples(pgresult)) ^ L" "
				^ var(PQerrorMessage(thread_pgconn)));
#		endif
		//PQclear(pgresult);
		return false;
	}

	//if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult),"1") != 0)
	{
		//PQclear(pgresult);
 		return false;
	}

	//PQclear(pgresult);
	return true;

}

bool var::deleterecord(const var& key) const
{
	THISIS(L"bool var::deleterecord(const var& key) const")
	THISISSTRING()
	ISSTRING(key)

	std::string key2=key.toString();

	//a one parameter array
	const char* paramValues[1];
	int		 paramLengths[1];
	int		 paramFormats[1];

	//$1=key
	paramValues[0] = key2.data();
	paramLengths[0] = int(key2.length());
	paramFormats[0] = 1;//binary

	var sql=L"DELETE FROM " ^ var_str ^ L" WHERE KEY = $1";

	PGconn* thread_pgconn=(PGconn*) this->connection();
	if (!thread_pgconn)
		return false;

	DEBUG_LOG_SQL1
	PGresult* pgresult = PQexecParams(thread_pgconn,
		sql.toString().c_str(),
		1,	   /* two param */
		NULL,	/* let the backend deduce param type */
		paramValues,
		paramLengths,
		paramFormats,
		1);	  /* ask for binary results */
	Resultclearer clearer(pgresult);

	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
	{
#		if TRACING >= 1
			exodus::errputl(L"ERROR: mvdbpostgres deleterecord() Failed: "
				^ var(PQntuples(pgresult)) ^ L" "
				^ var(PQerrorMessage(thread_pgconn)));
#		endif
		//PQclear(pgresult);
		return false;
	}

	//if not updated 1 then fail
	if (strcmp(PQcmdTuples(pgresult),"1") != 0)
	{
		//PQclear(pgresult);
#		if TRACING >= 3
			exodus::logputl(L"var::deleterecord failed. Record does not exist " ^ var_str ^ L" " ^ key);
#		endif
		return false;
	}

	//PQclear(pgresult);

	return true;
}

// If this is opened SQL connection, pass connection ID to sqlexec
bool var::begintrans() const
{
	THISIS(L"bool var::begintrans() const")
	THISISDEFINED()

	//begin a transaction
	return this->sqlexec(L"BEGIN");
}

bool var::rollbacktrans() const
{
	THISIS(L"bool var::rollbacktrans() const")
	THISISDEFINED()

	// Rollback a transaction
	return this->sqlexec(L"ROLLBACK");
}

bool var::committrans() const
{
	THISIS(L"bool var::committrans() const")
	THISISDEFINED()

	//end (commit) a transaction
	return this->sqlexec(L"END");
}

bool var::statustrans() const
{
	THISIS(L"bool var::statustrans() const")
	THISISDEFINED()

	PGconn * thread_pgconn = (PGconn *) connection();
	if (!thread_pgconn) {
		this->setlasterror(L"db connection not opened");
		return false;
	}
	this->setlasterror();

	//only idle is considered to be not in a transaction
	return (PQtransactionStatus(thread_pgconn)!=PQTRANS_IDLE);
}

bool var::createdb(const var& dbname) const
{
	var errmsg;
	return createdb(dbname, errmsg);
}

bool var::deletedb(const var& dbname) const
{
	var errmsg;
	return deletedb(dbname,errmsg);
}

//sample code
//var().createdb("mynewdb");//create a new database on the current thread-default connection
//var file;
//file.open("myfile");
//file.createdb("mynewdb");//creates a new db on the same connection as a file was opened on
//var connectionhandle;
//connectionhandle.connect("connection string pars");
//connectionhandle.createdb("mynewdb");

bool var::createdb(const var& dbname, var& errmsg) const
{
	THISIS(L"bool var::createdb(const var& dbname, var& errmsg)")
	THISISDEFINED()
	ISSTRING(dbname)

	//var sql = L"CREATE DATABASE "^dbname.convert(L". ",L"__");
	var sql = L"CREATE DATABASE "^dbname;
	sql^=L" WITH ENCODING='UTF8' ";
	//sql^=" OWNER=exodus";

	//TODO this shouldnt only be for default connection
	return this->sqlexec(sql,errmsg);

}

bool var::deletedb(const var& dbname, var& errmsg) const
{
	THISIS(L"bool var::deletedb(const var& dbname, var& errmsg)")
	THISISDEFINED()
	ISSTRING(dbname)

	return this->sqlexec(L"DROP DATABASE "^dbname, errmsg);
}

bool var::createfile(const var& filename) const
{
	THISIS(L"bool var::createfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	//var tablename = L"TEMP" ^ var(100000000).rnd();
	//Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	var sql = L"CREATE";
	//if (options.ucase().index(L"TEMPORARY")) sql ^= L" TEMPORARY";
	//sql ^= L" TABLE " ^ filename.convert(L".",L"_");
	if (filename.substr(-5,5) ==L"_temp")
		sql ^= L" TEMP ";
	sql ^= L" TABLE " ^ filename;
	sql ^= L" (key bytea primary key, data bytea)";

	return this->sqlexec(sql);
}

bool var::renamefile(const var& filename, const var& newfilename) const
{
	THISIS(L"bool var::renamefile(const var& filename, const var& newfilename)")
	THISISDEFINED()
	ISSTRING(filename)
	ISSTRING(newfilename)

	return this->sqlexec(L"ALTER TABLE " ^ filename ^ L" RENAME TO " ^ newfilename);
}

bool var::deletefile(const var& filename) const
{
	THISIS(L"bool var::deletefile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	return this->sqlexec(L"DROP TABLE " ^ filename);
}

bool var::clearfile(const var& filename) const
{
	THISIS(L"bool var::clearfile(const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

	return this->sqlexec(L"DELETE FROM " ^ filename);
}

inline void unquoter_inline(var& string)
{
		//remove "", '' and {}
		static var quotecharacters(L"\"'{");
		if (quotecharacters.index(string[1]))
				string=string.substr(2,string.length()-2);
}

inline void tosqlstring(var& string1)
{
	//convert to sql style strings
	//use single quotes and double up any internal single quotes
	if (string1[1]==L"\"")
	{
		string1.swapper(L"'",L"''");
		string1.splicer(1,1,L"'");
		string1.splicer(-1,1,L"'");
	}
}

inline var fileexpression(const var& mainfilename, const var& filename, const var& keyordata)
{
	//evade warning: unused parameter mainfilename
	if (false && mainfilename) {}

	var expression=filename ^ L"." ^ keyordata;
	return expression;

	//if you dont use STRICT in the postgres function declaration/definitions then NULL parameters do not abort functions

	//use COALESCE function in case this is a joined but missing record (and therefore null)
	//in MYSQL this is the ISNULL expression?
	//fromdictexpression=L"exodus_extract_bytea(coalesce(L" ^ filename ^ L".data,''::bytea), " ^ xlatefromfieldname.substr(9);
	//if (filename==mainfilename) return expression;
	//return "coalesce(L" ^ expression ^", ''::bytea)";
}

var var::getdictexpression(const var& mainfilename, const var& filename, const var& dictfilename, const var& dictfile, const var& fieldname, var& joins, var& ismv, bool forsort_or_select_or_index) const
{

	var actualdictfile=dictfile;
	if (!actualdictfile)
	{
		var dictfilename;
		if (mainfilename.substr(1,5).lcase()== L"dict_")
			dictfilename=L"dict_voc";
		else
			dictfilename=L"dict_"^mainfilename;

		// we should open it through the same connection, as this->was opened, not any default connection
		int connid = 0;
		if (THIS_IS_DBCONN())
			connid = (int) var_int;

		if (!actualdictfile.open(dictfilename, connid))
		{
			dictfilename=L"dict_voc";
			if (!actualdictfile.open(dictfilename, connid))
			{

				throw MVDBException(L"getdictexpression() cannot open " ^ dictfilename.quote());
	#if TRACING >= 1
				exodus::errputl(L"ERROR: mvdbpostgres getdictexpression() cannot open " ^ dictfilename.quote());
	#endif
				return L"";
			}
		}
	}

	//given a file and dictionary id
	//returns a postgres sql expression like (texta(filename.data,99,0,0))
	//using one of the neosys backend functions installed in postgres like textextract, dateextract etc.
	var dictrec;
	if (!dictrec.read(actualdictfile,fieldname))
	{
		if (!dictrec.read(actualdictfile,fieldname.lcase()))
		{
			if (!dictrec.read(actualdictfile,fieldname.ucase()))
			{
				if (not dictrec.read(L"dict_voc", fieldname))
				{
					if (fieldname.ucase()==L"@ID")
						dictrec = L"F" ^ FM ^ L"0" ^ FM ^ L"Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ L"L" ^ FM ^ 15;
					else
					{
						throw MVDBException(L"getdictexpression() cannot read " ^ fieldname.quote() ^ L" from " ^ actualdictfile.quote());
		//				exodus::errputl(L"ERROR: mvdbpostgres getdictexpression() cannot read " ^ fieldname.quote() ^ L" from " ^ actualdictfile.quote());
						return L"";
					}
				}
			}
		}
	}
	var sqlexpression;
	var dicttype=dictrec.a(1);
	if (dicttype==L"F")
	{
		var conversion=dictrec.a(7);
		var fieldno=dictrec.a(2);
		var params;
		var ismv1=dictrec.a(4)[1] == L"M";
		if (fieldno)
			if (ismv1)
			{
				ismv=true;
				var sqlexpression=L"mv_field_" ^ fieldno;
				var phrase=L"unnest(regexp_split_to_array(split_part(convert_from(data,'UTF8'),'" ^ FM ^ L"'," ^ fieldno ^ L"),'"^VM^L"'))"
					//L" with ordinality as t( " ^ sqlexpression ^ L", pg_ordinal_mv)";
					L" with ordinality as t( " ^ sqlexpression ^ L", mv)";
				joins ^= L", " ^ phrase;
				return sqlexpression;
			}
			else
			{
				params=fileexpression(mainfilename, filename, L"data") ^ L"," ^ fieldno ^ L", 0, 0)";
			}
		else
		{
			//example of multipart key and date conversion
			//select date '1967-12-31' + split_part(convert_from(key, 'UTF8'), '*',2)::integer from filename

            if (forsort_or_select_or_index)
                //sqlexpression=L"exodus_extract_sort(" ^  fileexpression(mainfilename, filename,L"key") ^ L")";
                sqlexpression=L"exodus_extract_sort(" ^ mainfilename ^ L".key,0,0,0)";
            else
                sqlexpression=L"convert_from(" ^ fileexpression(mainfilename, filename, L"key") ^ L", 'UTF8')";

			var keypartn=dictrec.a(5);
			if (keypartn)
			{
				sqlexpression=L"split_part(" ^ sqlexpression ^ L", '*', " ^ keypartn ^ L")";
			}

			if (conversion[1]==L"D" || conversion.substr(1,5)==L"[DATE")
				sqlexpression=L"date '1967-12-31' + " ^ sqlexpression ^ L"::integer";

			return sqlexpression;
		}

		if (conversion.substr(1,9)==L"[DATETIME")
			sqlexpression=L"exodus_extract_datetime(" ^ params;

		else if (conversion[1]==L"D" || conversion.substr(1,5)==L"[DATE")
			sqlexpression=L"exodus_extract_date(" ^ params;

		else if (conversion.substr(1,2)==L"MT" || conversion.substr(1,5)==L"[TIME")
			sqlexpression=L"exodus_extract_time(" ^  params;

		//for now (until we have a extract_number/integer/float) that doesnt fail on non-numeric like cast "as integer" and "as float" does
		//note that we could use exodus_extract_sort for EVERYTHING inc dates/time/numbers etc.
		//but its large size is perhaps a disadvantage
		else if (forsort_or_select_or_index)
			sqlexpression=L"exodus_extract_sort(" ^  params;

		else if (conversion==L"[NUMBER,0]" || dictrec.a(11)==L"0N" || dictrec.a(11).substr(1,3)==L"0N_")
			sqlexpression=L"cast( exodus_extract_text(" ^ params ^ L" as integer)";

		else if (conversion.substr(1,2)==L"MD" || conversion.substr(1,7)==L"[NUMBER" || dictrec.a(12)==L"FLOAT" || dictrec.a(11).index(L"0N"))
				sqlexpression=L"cast( exodus_extract_text(" ^ params ^ L" as float)";

		else
			sqlexpression=L"exodus_extract_text(" ^ params;

	}
	else if (dicttype==L"S")
	{
		var functionx=dictrec.a(8).trim();
		if (functionx.substr(1,11).ucase()==L"@ANS=XLATE(")
		{
			functionx.splicer(1,11,L"");
			var xlatetofilename=functionx.field(L",",1).trim();
			unquoter_inline(xlatetofilename);
			var xlatefromfieldname=functionx.field(L",",2).trim();
			var xlatetofieldname=functionx.field(L",",3).trim();
			var xlatemode=functionx.field(L",",4).trim().convert(L"'\" )",L"");
			//if the fourth field is 'X', L"X", L"C" or 'C' then
			//assume we have a good simple xlate functionx and can convert to a JOIN
			if (xlatemode==L"X" || xlatemode==L"C")
			{


				//determine the expression in the xlate file
				var& todictexpression=sqlexpression;
				if (xlatetofieldname.isnum())
				{
					todictexpression=L"exodus_extract_text(" ^ fileexpression(mainfilename, xlatetofilename, L"data") ^ L", " ^ xlatetofieldname ^ L", 0, 0)";
				}
				else
				{
					var dictxlatetofile=xlatetofilename;
					//if (!dictxlatetofile.open(L"DICT",xlatetofilename))
					//	throw MVDBException(L"getdictexpression() DICT" ^ xlatetofilename ^ L" file cannot be opened");
					//var ismv;
					todictexpression=getdictexpression(filename,xlatetofilename, dictxlatetofile, dictxlatetofile, xlatetofieldname, joins, ismv, forsort_or_select_or_index);
				}

				//determine the join details
				var fromdictexpression;
				if (xlatefromfieldname.substr(1,8)==L"@RECORD<")
				{
					fromdictexpression=L"exodus_extract_bytea(";
					fromdictexpression ^= fileexpression(mainfilename, filename,L"data");
					fromdictexpression ^= L", " ^ xlatefromfieldname.substr(9);
					fromdictexpression.splicer(-1,1,L"");
					fromdictexpression ^= var(L", 0").str(3-fromdictexpression.count(',')) ^ L")";
				}
//TODO				if (xlatefromfieldname.substr(1,8)==L"FIELD(@ID)
				else if (xlatefromfieldname[1]==L"{")
				{
					xlatefromfieldname=xlatefromfieldname.substr(2).splicer(-1,1,L"");
					fromdictexpression=getdictexpression(filename, filename, dictfilename, dictfile, xlatefromfieldname, joins, ismv);
				}
				else
				{
					//throw  MVDBException(L"getdictexpression() " ^ filename.quote() ^ L" " ^ fieldname.quote() ^ L" - INVALID DICTIONARY EXPRESSION - " ^ dictrec.a(8).quote());
#if TRACING >= 1
					exodus::errputl(L"ERROR: mvdbpostgres getdictexpression() " ^ filename.quote() ^ L" " ^ fieldname.quote() ^ L" - INVALID DICTIONARY EXPRESSION - " ^ dictrec.a(8).quote());
#endif
					return L"";
				}

				//add the join
				var join=L"LEFT JOIN " ^ xlatetofilename ^ L" ON " ^ fromdictexpression ^ L" = " ^ xlatetofilename ^ L".key";
				if (!joins.index(join)) {
					if (joins)
						joins^=L", ";
					joins^=join;
				}

			} else {
				//not xlate X or C
				goto exodus_call;
			}
		}
		else
		{
exodus_call:
			sqlexpression=L"'" ^ fieldname ^ L"'";
			sqlexpression=L"exodus_call('exodusservice-" ^ getprocessn() ^ L"." ^ getenvironmentn() ^ L"'::bytea, '" ^ dictfilename.lcase() ^ L"'::bytea, '" ^ fieldname.lcase() ^ L"'::bytea, "^ filename ^ L".key, " ^ filename ^ L".data,0,0)";
			//TODO apply naturalorder conversion by passing forsort_or_select_or_index option to exodus_call
		}
	}
	else
	{
		//throw  filename ^ L" " ^ fieldname ^ L" - INVALID DICTIONARY ITEM";
		//throw  MVDBException(L"getdictexpression(" ^ filename.quote() ^ L", " ^ fieldname.quote() ^ L") invalid dictionary type " ^ dicttype.quote());
#if TRACING >= 1
		exodus::errputl(L"ERROR: mvdbpostgres getdictexpression(" ^ filename.quote() ^ L", " ^ fieldname.quote() ^ L") invalid dictionary type " ^ dicttype.quote());
#endif
		return L"";
	}
	return sqlexpression;
}

var getword(var& remainingwords, const var& joinvalues=false)
{

	//gets the next word (or series of words separated by FM while they are numbers or quoted strings)
	//converts to sql quoted strings
	//and clips them from the input string

	static const var valuechars(L"\"'.0123456789-+");

	var word1=remainingwords.field(L" ",1);
	remainingwords=remainingwords.field(L" ",2,99999);

	//join quoted words together
	var char1=word1[1];
	if ((char1==DQ||char1==SQ))
	{
		while (word1[-1]!=char1)
		{
			if (remainingwords.length())
			{
				word1^=L" "^remainingwords.field(L" ",1);
				remainingwords=remainingwords.field(L" ",2,99999);
			}
			else
			{
				word1^=char1;
				break;
			}
		}
	}

	tosqlstring(word1);

	//grab multiple values (numbers or strings) separated by FM
	if (joinvalues && valuechars.index(word1[1]))
	{
		word1 = SQ ^ word1.unquote() ^ SQ;

		var nextword=remainingwords.field(L" ",1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		//to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		//duplicated above/below
		if (nextword==L"and")
		{
			var nextword2=remainingwords;
			if (valuechars.index(nextword2[1]))
			{
				nextword=nextword2;
				remainingwords=remainingwords.field(L" ",2,99999);
			}
		}

		while (nextword && valuechars.index(nextword[1]))
		{
			tosqlstring(nextword);
			if (word1!=L"")
				word1^=FM;
			word1 ^= SQ ^ nextword.unquote() ^ SQ;

			remainingwords=remainingwords.field(L" ",2,99999);
			nextword=remainingwords.field(L" ",1);

			//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
			//to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
			//duplicated above/below
			if (nextword==L"and")
			{
				var nextword2=remainingwords;
				if (valuechars.index(nextword2[1]))
				{
					nextword=nextword2;
					remainingwords=remainingwords.field(L" ",2,99999);
				}
			}

		}
	}else{
		//word1.ucaser();
	}

	return word1;
}

bool var::deletelist(const var& listname) const
{
	THISIS(L"bool var::deletelist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	//THISISDEFINED()
	ISSTRING(listname)

	var listfilename=L"savelist_";
	if (listname)
		listfilename ^= listname;
	else
		listfilename ^= (*this) ^ L"_" ^ getprocessn() ^ L"_tempx";

	if (var().open(listfilename)) {
		//listfilename.outputl(L"Deleted ");
		if (GETDBTRACE)
			exodus::logputl(L"DBTRACE: ::deletelist(" ^ listname ^ L")");
		return this->deletefile(listfilename);
	}
	return true;//true if not existing
}

bool var::savelist(const var& listname) const
{
	THISIS(L"bool var::savelist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	//THISISDEFINED()
	ISSTRING(listname)

	int recn=0;
	var key;
	var mv;
	var listfilename=L"savelist_"^listname;
	var listfilename2=listfilename^L"_saving";

	//save preselected keys into a file to be used with INNERJOIN on select()
	if (this->selectpending()) {

		this->deletelist(listname);

		if (var(*this).open(listfilename2))
			var(*this).deletefile(listfilename2);

		if (GETDBTRACE)
			exodus::logputl(L"DBTRACE: ::savelist(" ^ listname ^ L")");

		while (this->readnext(key,mv)) {
			recn++;

			//delete/create file on first record
			if (recn==1) {
				if (!this->createfile(listfilename2))
					return false;
				//listfilename2.outputl(L"Created ");
			}

			//save a key
			(mv ^ FM ^ recn).write(listfilename2,key);
			//key.outputl(L"Written=");
		}
		if (recn)
			this->renamefile(listfilename2, listfilename);
	}

	return recn>0;
}

bool var::makelist(const var& listname, const var& keys) const
{
	THISIS(L"bool var::makelist(const var& listname) const")
	//?allow undefined usage like var xyz=xyz.select();
	THISISSTRING()
	ISSTRING(listname)
	ISSTRING(keys)

	this->deletelist(listname);

	int recn=0;
	var key;
	var mv;
	var listfilename=L"savelist_";
	if (listname)
		listfilename^=listname;
	else {
		var listname2=(*this) ^ L"_" ^ getprocessn() ^ L"_tempx";
		listfilename^=listname2;
	}

	//save preselected keys into a file to be used with INNERJOIN on select()
	int nn=keys.dcount(FM);
	//var(nn).outputl(L"nn=");
	for (int fn1=1;fn1<nn;fn1++) {
		var key=keys.a(fn1);
		if (key.length()) {

			//key can be key]mv
			var mv=key.a(1,2);
			if (mv.length())
				key=key.a(1,1);

			recn++;
			//create file on first record
			if (recn==1) {
				if (!this->createfile(listfilename)) {
					listfilename.errputl(L"Cannot create ");
					return false;
				}
			}
			//save a key
			(mv ^ FM ^ recn).write(listfilename,key);
		}
	}

	//listfilename.outputl(L"makelist recs:" ^ var(recn) ^ L", listfilename=");
	return recn>0;
}

bool var::selectrecord(const var& sortselectclause) const
{
	THISIS(L"bool var::selectrecord(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	//THISISDEFINED()
	ISSTRING(sortselectclause)

	//return const_cast<const var&>(*this).selectx(L"key, mv::integer, data",sortselectclause);
	return this->selectx(L"key, mv::integer, data",sortselectclause);
}

bool var::select(const var& sortselectclause) const
{
	THISIS(L"bool var::select(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(sortselectclause)

	return this->selectx(L"key, mv::integer",sortselectclause);
}

//currently only called from select and selectrecord
bool var::selectx(const var& fieldnames, const var& sortselectclause) const
{
	//private - and arguments are left unchecked for speed

	//?allow undefined usage like var xyz=xyz.select();
	if (var_typ&mvtypemask)
	{
		//throw MVUndefined(L"selectx()");
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
	}

	//default to ""
	if (!(var_typ&pimpl::VARTYP_STR))
	{
		if (!var_typ)
		{
			var_str=L"";
			var_typ=pimpl::VARTYP_STR;
		}
		else
			this->createString();
	}

	// Note that VARTYP_DBCONN bit is still preserved in var_typ

	//TODO only do this if cursor already exists
	//clearselect();

	//DEBUG_LOG_SQL
	if (GETDBTRACE)
		exodus::logputl(sortselectclause);

	var actualfilename=L"";
	var dictfilename=L"";
	var actualfieldnames=fieldnames;
	var dictfile=L"";
	var keycodes=L"";
	var bykey=0;
	var wordn;
	var distinctfieldnames=L"";

	var whereclause=L"";//exodus_call('NAME', "^ filename ^ L".data, " ^ filename ^ L".key,0,0) <> '' AND ";
	var orderclause=L"";
	var joins=L"";
	var ismv=false;

	var maxnrecs=L"";

	var remainingsortselectclause=sortselectclause;
//remainingsortselectclause.outputl(L"remainingsortselectclause=");

	//sortselectclause may start with {SELECT|SSELECT {maxnrecs} filename}
	var word1=remainingsortselectclause.field(L" ", 1);
	var ucword=word1.ucase();
	if (ucword==L"SELECT"||ucword==L"SSELECT")
	{
		remainingsortselectclause.substrer(ucword.length()+2);

		if (ucword==L"SSELECT")
			bykey=1;

		//discard it and get the second word which is either max number of records or filename
		actualfilename=getword(remainingsortselectclause);

		//the second word can be a limiting number of records
		if (actualfilename.length() and actualfilename.isnum())
		{
			maxnrecs=actualfilename;
			actualfilename=getword(remainingsortselectclause);
		}

		dictfilename=actualfilename;

	}

	//optionally get filename from the current var
	if (!actualfilename)
	{
		if (!assigned())
			throw MVUnassigned(L"select() unassigned filehandle and sort/select clause doesnt start \"SELECT filename\"");
		actualfilename=*this;
		dictfilename=*this;

		//assume sortselectclause is a simple filename
		if (!actualfilename) {
			actualfilename=getword(remainingsortselectclause);
			dictfilename=actualfilename;
		}

		//optionally get filename from the current var
		if (!actualfilename) {
			throw MVDBException(L"select() filehandle is missing and sort/select clause doesnt start \"SELECT filename\"");
		}
	}

	static const var valuechars(L"\"'.0123456789-+");

//remainingsortselectclause.outputl(L"remainingsortselectclause=");

	while (remainingsortselectclause.length())
	{

		word1=getword(remainingsortselectclause);
		ucword=word1.ucase();

		//options - last word enclosed in () or {}
		//ignore options (last word and surrounded by brackets)
		if (!remainingsortselectclause.length() && (word1[1]==L"(" && word1[-1]==L")") || (word1[1] == L"{" && word1[-1] == L"}"))
		{
//		word1.outputl(L"skipping last word in () options ");
			continue;
		}

		//numbers or strings without leading clauses like with ... mean record keys
		if (valuechars.index(word1[1]))
		{
			if (keycodes)
				keycodes ^= FM;
			keycodes ^= word1;
			continue;
		}

		//using filename
		if (ucword==L"USING" && remainingsortselectclause)
		{
			dictfilename=getword(remainingsortselectclause);
			if (!dictfile.open(L"dict_"^dictfilename))
			{
				//throw MVDBException(L"select() dict_" ^ dictfilename ^ L" file cannot be opened");
				exodus::errputl(L"ERROR: mvdbpostgres select() dict_" ^ dictfilename ^ L" file cannot be opened");
				return L"";
			}
			continue;
		}

		//distinct fieldname (returns a field instead of the key)
		if (ucword==L"DISTINCT" && remainingsortselectclause)
		{

			var distinctfieldname=getword(remainingsortselectclause);
			var distinctexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,distinctfieldname,joins,ismv,false);
			var naturalsort_distinctexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,distinctfieldname,joins,ismv,true);

			if (true) {
				//this produces the right values but in random order
				//it use any index on the distinct field so it works on large indexed files
				//select distinct is really only useful on INDEXED fields unless the file is small
				distinctfieldnames=L"DISTINCT " ^ distinctexpression;
			} else {
				//this produces the right results in the right order
				//BUT DOES IS USE INDEXES AND ACT VERY FAST??
				distinctfieldnames=L"DISTINCT ON (" ^ naturalsort_distinctexpression ^ L") " ^ distinctexpression;
				orderclause^=L", " ^ naturalsort_distinctexpression;
			}
			continue;
		}

		//by or by-dsnd
		if (ucword==L"BY" || ucword==L"BY-DSND")
		{
			var dictexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,getword(remainingsortselectclause),joins,ismv,true);

//dictexpression.outputl(L"dictexpression=");
//orderclause.outputl(L"orderclause=");

			orderclause ^= L", " ^ dictexpression;

			if (ucword==L"BY-DSND")
				orderclause^=L" DESC";

			continue;
		}

		//subexpressions (,),and,or
		if (var(L"( ) AND OR").locateusing(ucword,L" "))
		{
			whereclause ^= L" " ^ ucword;
			continue;
		}

		//with dictid eq/starting/ending/containing/like 1 2 3
		//with dictid 1 2 3
		//with dictid between x and y
		//TODO:without (same as "with xxx not a and b and c")
		if (ucword==L"WITH")
		{

			//add the dictionary id
			word1=getword(remainingsortselectclause);
			ucword=word1.ucase();
			var sortable=false;//because indexes are NOT created sortable (exodus_sort()
			var dictexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,word1,joins,ismv,false);
			var usingnaturalorder=dictexpression.index(L"exodus_extract_sort");
			whereclause ^= L" " ^ dictexpression;

			//get the word/values after the dictid
			word1=getword(remainingsortselectclause, true);
			ucword=word1.ucase();

			if (ucword==L"NOT")
			{
				whereclause ^= L" NOT ";
				ucword=getword(remainingsortselectclause);
			}
			if (ucword==L"BETWEEN")
			{
				//get and append "from" value
				ucword=getword(remainingsortselectclause);
				if (usingnaturalorder)
					ucword=naturalorder(ucword.toString());
				whereclause ^= L" BETWEEN " ^ ucword;

				//get, check, discard "and"
				ucword=getword(remainingsortselectclause).ucase();
				if (ucword != L"AND")
				{
					exodus::errputl(L"ERROR: mvdbpostgres SELECT STATEMENT SYNTAX IS 'between x *and* y'");
					return L"";
				}

				//get and append "to" value
				ucword=getword(remainingsortselectclause);

				whereclause ^= L" AND " ^ ucword;

				continue;
			}

			//starting, ending, containing, like
			var startingpercent=L"";
			var endingpercent=L"";
			if (ucword==L"CONTAINING")
			{
				ucword=L"LIKE";
				startingpercent=L"%";
				endingpercent=L"%";
			}
			else if (ucword==L"STARTING")
			{
				ucword=L"LIKE";
				endingpercent=L"%";
			}
			else if (ucword==L"ENDING")
			{
				ucword=L"LIKE";
				startingpercent=L"%";
			}
			if (ucword==L"LIKE")
			{

				word1=getword(remainingsortselectclause);
				// _ and % are like ? and * in globbing in sql LIKE
				//if present in the search criteria, they need to be escaped with TWO backslashes.
				word1.swapper(L"_",L"\\\\_");
				word1.swapper(L"%",L"\\\\%");
				if (endingpercent)
				{
					word1.swapper(L"'" ^ FM, L"%'" ^ FM);
					word1.splicer(-1,0,L"%");
				}
				if (startingpercent)
				{
					word1.swapper(FM ^ L"'", FM ^ L"'%");
					word1.splicer(2,0,L"%");
				}

				whereclause ^= L" LIKE " ^ word1;

				continue;
			}

			//comparison operators
			//convert neosys relational operators to standard relational operators
			var aliasno;
			if (var(L"EQ NE NOT GT LT GE LE").locateusing(ucword,L" ",aliasno))
			{
				ucword=var(L"= <> <> > < >= <=").field(L" ",aliasno);
			}
			if (var(L"= <> > < >= <=").locateusing(ucword,L" ",aliasno))
			{
				whereclause ^= L" " ^ ucword ^ L" ";
				ucword=getword(remainingsortselectclause, true);
			}
			else
			{
				//if value follows dictionary id without a relational operator then insert =
				if (ucword[1]==L"'")
					whereclause ^= L" = ";
			}

			//append value(s)
			if (usingnaturalorder)
				ucword=naturalorder(ucword.toString());
			whereclause ^= ucword;

		}

	}//getword loop

	//prefix specified keys into where clause

	if (keycodes)
	{
		if (keycodes.count(FM))
		{
			keycodes=actualfilename ^ L".key IN ( " ^ keycodes.swap(FM,L", ") ^ L" )";

			if (whereclause)
				whereclause=L" AND ( " ^ whereclause ^ L" ) ";
			else
				whereclause=keycodes;
		}
	}

	//sselect add by key on the end of any specific order bys
	if (bykey)
	{
		orderclause^=L", " ^ actualfilename ^ L".key";
	}

	if (!ismv)
	{
		//sql ^= L", 0 as mv";
		if (actualfieldnames.index(L"mv::integer, data"))
		{
			//replace the mv column with zero if selecting record
			actualfieldnames.swapper(L"mv::integer, data",L"0::integer, data");
		}
		else
		{
			actualfieldnames.swapper(L", mv::integer",L"");
		}
	}

	//if any active select, convert to a file and use as an additional filter on key
	//or correctly named savelistfilename exists from getselect or makelist
	var listname=(*this) ^ L"_" ^ getprocessn() ^ L"_tempx";
	var savelistfilename=L"savelist_" ^ listname;
	if (this->savelist(listname) || (savelistfilename != actualfilename && var().open(savelistfilename))) {
		if (joins)
			joins^=L", ";
		joins ^= L" INNER JOIN " ^ savelistfilename ^ L" ON " ^ actualfilename ^ L".key = " ^ savelistfilename ^ L".key";
	}

	//disambiguate from any INNER JOIN key
	actualfieldnames.swapper(L"key",actualfilename ^ L".key");

	//DISTINCT has special fieldnames
	if (distinctfieldnames) {
		actualfieldnames=distinctfieldnames;
	}

	//assemble the full sql select statement:	//ALN:TODO: optimize with stringbuffer

	//WITH HOLD is a very significant addition
	//var sql=L"DECLARE cursor1_" ^ (*this) ^ L" CURSOR WITH HOLD FOR SELECT " ^ actualfieldnames ^ L" FROM ";
	var sql=L"DECLARE cursor1_" ^ (*this) ^ L" SCROLL CURSOR WITH HOLD FOR SELECT " ^ actualfieldnames ^ L" FROM ";
	sql ^= actualfilename;
	if (joins)
		sql ^= L" " ^ joins;
	if (whereclause)
		sql ^= L" WHERE " ^ whereclause;
	if (orderclause)
		sql ^= L" ORDER BY " ^ orderclause.substr(3);
	if (maxnrecs)
		sql ^= L" LIMIT " ^ maxnrecs;

//sql.outputl(L"sql=");

	//DEBUG_LOG_SQL
	//if (GETDBTRACE)
	//	exodus::logputl(sql);

	//Start a transaction block if none already - because postgres requires select to cursor to be inside one
	//var autotrans=!statustrans();
    //if (autotrans && !begintrans()) {
	//	return false;
	//}

	//not required if WITH HOLD is used in ::select()'s DECLARE CURSOR
//	if (!this->statustrans())
//		throw MVDBException(L"select() must be preceeded by begintrans()");

//	if (!sql.sqlexec())
	var errmsg;
	if (! this->sqlexec(sql,errmsg)) {

		this->deletelist(listname);

		//TODO handle duplicate_cursor sqlstate 42P03
		sql.outputl(L"sql=");

		throw MVDBException(errmsg);

		//if (autotrans)
		//	rollbacktrans();
		return false;
	}

	return true;
}

var& var::setifunassigned(const var& defaultvalue/*=""*/) {

	THISIS(L"var& var::setifunassigned(const var& defaultvalue) const")
	THISISDEFINED()
	ISASSIGNED(defaultvalue)

	//?allow undefined usage like var xyz=xyz.readnext();
	//if (var_typ&mvtypemask)
	if (this->unassigned())
	{
		//throw MVUndefined(L"setifunassigned( ^ defaultvalue ^L")");
		//var_str=L"";
		//var_typ=pimpl::VARTYP_STR;
		*this=defaultvalue;
	}
	return *this;
}
void var::clearselect() const
{
	//THISIS(L"void var::clearselect() const")
	//THISISSTRING()

	//default cursor is ""
	const_cast<var&>(*this).setifunassigned(L"");

        ///if readnext through string
        if ((*this)[-1]==FM) {
                var_str=L"";
                var_typ=pimpl::VARTYP_STR;
		return;
        }

	//dont close cursor unless it exists otherwise sql error aborts any transaction
	if (not this->selectpending())
		return;

	var listname=(*this) ^ L"_" ^ getprocessn() ^ L"_tempx";

	if (GETDBTRACE)
		exodus::logputl(L"DBTRACE: ::clearselect() for " ^ listname);

	//clear any select list
	this->deletelist(listname);

	var sql=L"";
	//var sql^=L"BEGIN ;";
	sql^=L"CLOSE cursor1_";
	if (this->assigned())
		sql ^= *this;
	//sql^=L"; EXCEPTION WHEN invalid_cursor_name THEN";
	//sql^=L"; END";

	var errors;
	if (! this->sqlexec(sql,errors)) {
		if (errors)
			errors.outputl(L"::clearselect " ^ errors);
		return;
	}

	return;
}

//NB global not member function
//	To make it var:: privat member -> pollute mv.h with PGresultptr :(
//bool readnextx(const std::wstring& cursor, PGresultptr& pgresult)
//NB caller MUST ALWAYS do PQclear(pgresult) even bool false
//called by readnext/readnextrecord (and perhaps hasnext/select to implement LISTACTIVE)
bool readnextx(const var& cursor, PGresultptr& pgresult, PGconn* pgconn, bool clearselect_onfail, bool forwards)
{
	var sql;
	if (forwards)
		sql=L"FETCH NEXT in cursor1_" ^ cursor;
	else
		sql=L"FETCH BACKWARD in cursor1_" ^ cursor;

	//sql=L"BEGIN;" ^ sql ^ L"; END";

	//execute the sql
	//cant use sqlexec here because it returns data
	//sqlexec();
	if (!getpgresult(sql,pgresult, pgconn)) {

		//var errmsg=var(PQresultErrorMessage(pgresult));
		//PQclear(pgresult);
		//throw MVDBException(errmsg);
		//cursor.clearselect();
		//return false;

		var errmsg=var(PQresultErrorMessage(pgresult));
		var sqlstate = var(PQresultErrorField(pgresult, PG_DIAG_SQLSTATE));

		//PQclear(pgresult);

		if (clearselect_onfail) {
			cursor.clearselect();
		}

		//if cursor simply doesnt exist then see if a savelist one is available and enable it
		//34000 - "ERROR:  cursor "cursor1_" does not exist"
		if (forwards && sqlstate == L"34000") {
			return false;

			/**
			//if the standard select list file is available then select it, i.e. create a CURSOR, so FETCH has something to work on
			var listfilename=L"savelist_" ^ cursor ^ L"_" ^ getprocessn() ^ L"_tempx";
			if (not var().open(listfilename))
				return false;
			//TODO should add BY LISTITEMNO
			if (not cursor.select(L"select " ^ listfilename))
				return false;
			if (GETDBTRACE)
				exodus::logputl(L"DBTRACE: readnextx(...) found standard selectfile " ^ listfilename);

			return readnextx(cursor, pgresult, pgconn, clearselect_onfail, forwards);
			**/
		}

		//any other error
		throw MVDBException(errmsg ^ L" sqlstate= " ^ sqlstate.quote());

		//return false;
	}

	//false and optionally close cursor if no more
	if (forwards && (PQntuples(pgresult) < 1))
	{
		//PQclear(pgresult);
		if (clearselect_onfail) {
			cursor.clearselect();
		}
		return false;
	}

	//DO NOT clear since the pgresult is needed by the caller
	//PQclear(pgresult);

	//true = found a new key/record
	return true;
}

bool var::hasnext() const
{

	//var xx;
	//return this->readnext(xx);

	//THISIS(L"bool var::hasnext() const")
	//THISISSTRING()

	//default cursor is ""
	const_cast<var&>(*this).setifunassigned(L"");

	//readnext through string of keys if provided
	if ((*this)[-1]==FM) {
		return true;
	}

	//if the standard select list file is available then select it, i.e. create a CURSOR, so FETCH has something to work on
	//if (not this->selectpending()) {
	//	var listfilename=L"savelist_" ^ (*this) ^ L"_" ^ getprocessn() ^ L"_tempx";
	//	if (var().open(listfilename)) {
	//		true;
	//	}
	//}

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL) {
		//this->clearselect();
		return false;
	}

	PGresultptr pgresult;
	bool ok=readnextx(*this, pgresult, pgconn, false, true);

	Resultclearer clearer(pgresult);

	if (!ok)
		return false;

	//PQclear(pgresult);

	/////////////////////////////////
	//now restore the cursor back one
	/////////////////////////////////

	PGresultptr pgresult2;
	ok=readnextx(*this, pgresult2, pgconn, false, false);

	Resultclearer clearer2(pgresult2);

	if (!ok) {
		//should always be able to move the cursor backwards if could move it forwards in the code above
		//hasnext=false
		return false;
	}

	//PQclear(pgresult2);

	//hasnext=true
	return true;
}

bool var::readnext(var& key) const
{
	var valueno;
	return readnext(key, valueno);
}

bool var::readnext(var& key, var& valueno) const
{

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_typ&mvtypemask)
	{
		//throw MVUndefined(L"readnext()");
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
	}

	//default cursor is ""
	const_cast<var&>(*this).setifunassigned(L"");

	if (GETDBTRACE)
		exodus::logputl(L"DBTRACE: ::readnext(key,valueno)");

	THISIS(L"bool var::readnext(var& key, var& valueno) const")
	THISISSTRING()

	//readnext through string of keys if provided
	if ((*this)[-1]==FM) {

		//initialise readnext pointer to 0 (using var_int as pointer)
		if ((var_typ&pimpl::VARTYP_INT) == 0) {
			var_int=0;
			var_typ=var_typ|pimpl::VARTYP_INT;
		}

		//increment readnext pointer
		var_int++;

		//extract next key
		key=a(var_int);

		//key may be key]valueno
		valueno=key.a(1,2);
		key=key.a(1,1);

		//fail if no key
		return key.length()>0;
	}

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL) {
		this->clearselect();
		return false;
	}

	PGresultptr pgresult;
	bool ok=readnextx(*this, pgresult, pgconn, true, true);

	Resultclearer clearer(pgresult);

	if (not ok) {
		// end the transaction and quit
		// no more
		//committrans();

		//already done in readnextx
		//this->clearselect();

		return false;
	}

	//MUST call PQclear(pgresult) on all paths below

	/* abortive code to handle unescaping returned hex/escape data	//avoid the need for this by calling pqexecparams flagged for binary
	//even in the case where there are no parameters and pqexec could be used.

	//eg 90001 is 9.0.1
	int pgserverversion=PQserverVersion(thread_pgconn);
	if (pgserverversion>=90001) {
		var(pgserverversion).outputl();
		//unsigned char *PQunescapeBytea(const unsigned char *from, size_t *to_length);
		size_t to_length;
		unsigned char* unescaped = PQunescapeBytea((const unsigned char*) PQgetvalue(pgresult, 0, 0),
			&to_length);
		if (*unescaped)
			key=wstringfromUTF8((UTF8*)unescaped, to_length);
		PQfreemem(unescaped);
		PQclear(pgresult);
		return true;
	}
*/
	//get the key from the first column
	//char* data = PQgetvalue(pgresult, 0, 0);
	//int datalen = PQgetlength(pgresult, 0, 0);
	//key=std::string(data,datalen);
	//key=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));
	key=getresult(pgresult,0,0);
//key.output(L"key=").len().outputl(L" len=");

  //vn is second column	
	//record is third column
	if (PQnfields(pgresult)>1)
		valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));
	else
		valueno=0;

	//PQclear(pgresult);

	return true;

/*how to access multiple records and fields*/
#if 0
	/* first, print out the attribute names */
	int nFields = PQnfields(pgresult);
	for (i = 0; i < nFields; i++)
		wprintf(L"%-15s", PQfname(pgresult, i));
	wprintf(L"\n\n");

	/* next, print out the rows */
	for (i = 0; i < PQntuples(pgresult); i++)
	{
		for (j = 0; j < nFields; j++)
			wprintf(L"%-15s", PQgetvalue(pgresult, i, j));
		wprintf(L"\n");
	}
#endif

}

bool var::readnextrecord(var& record, var& key) const
{
	var valueno=0;
	return readnextrecord(record, key, valueno);
}

bool var::readnextrecord(var& record, var& key, var& valueno) const
{

	//?allow undefined usage like var xyz=xyz.readnextrecord();
	if (var_typ&mvtypemask || !var_typ)
	{
		//throw MVUndefined(L"readnextrecord()");
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
	}

	//default cursor is ""
	const_cast<var&>(*this).setifunassigned(L"");

	THISIS(L"bool var::readnextrecord(var& record, var& key) const")
	THISISSTRING()
	ISDEFINED(key)
	ISDEFINED(record)

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	PGresultptr pgresult;
	bool ok=readnextx(*this, pgresult, pgconn, true, true);

	Resultclearer clearer(pgresult);

	if (!ok) {
		// end the transaction
		// no more
		//committrans();
		return false;
	}

	//MUST call PQclear(pgresult) on all paths below

	//key is first column
	//char* data = PQgetvalue(pgresult, 0, 0);
	//int datalen = PQgetlength(pgresult, 0, 0);
	//key=std::string(data,datalen);
	//key=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));
	key=getresult(pgresult,0,0);
//TODO return zero if no mv in select because no by mv column
	//vn is second column
	valueno=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 1)));

	//record is third column
	if (PQnfields(pgresult)<3) {
		PQclear(pgresult);
		var errmsg=L"readnextrecord() must follow selectrecord(), not select()";
		this->setlasterror(errmsg);
		throw MVException(errmsg);
		//return false;
	}
	//record=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 2), PQgetlength(pgresult, 0, 2));
	record=getresult(pgresult,0,2);

	//PQclear(pgresult);

	return true;

}

bool var::createindex(const var& fieldname, const var& dictfile) const
{
	THISIS(L"bool var::createindex(const var& fieldname, const var& dictfile) const")
	THISISSTRING()
	ISSTRING(fieldname)
	ISSTRING(dictfile)

	var filename=*this;

	//actual dictfile to use is either given or defaults to that of the filename
	var actualdictfile;
	if (dictfile.assigned() and dictfile != L"")
		actualdictfile=dictfile;
	else
		actualdictfile=L"dict_" ^ filename;

	//example sql
	//create index ads__brand_code on ads (exodus_extract_text(data,3,0,0));

	//throws if cannot find dict file or record
	var joins=L"";//throw away - cant index on joined fields at the moment
	var ismv;
	var sortable=false;
	var dictexpression=getdictexpression(filename,filename,actualdictfile,actualdictfile,fieldname,joins,ismv,false);
//dictexpression.outputl(L"dictexp=");stop();
	var sql=L"create index index__" ^ filename ^ L"__" ^ fieldname ^ L" on " ^ filename;
	sql^=L" (";
	sql^=dictexpression;
	sql^=L")";

	return this->sqlexec(sql);
}

bool var::deleteindex(const var& fieldname) const
{
	THISIS(L"bool var::deleteindex(const var& fieldname) const")
	THISISSTRING()
	ISSTRING(fieldname)

	//var filename=*this;
	var sql=L"drop index index__" ^ *this ^ L"__" ^ fieldname;

	return this->sqlexec(sql);
}

/*
http://www.petefreitag.com/item/666.cfm
information_schema is am SQL-92 standard for accessinging information about the tables etc in a database
	* Microsoft SQL Server - Supported in Version 7 and up
	* MySQL - Supported in Version 5 and up
	* PostgreSQL - Supported in Version 7.4 and up

	* Oracle - Does not appear to be supported
	* Apache Derby - NOT Supported As of Version 10.3
	* DB2 - NOT supported?
*/

var var::listfiles() const
{
	THISIS(L"var var::listfiles() const")
	//could allow undefined usage since *this isnt used?
	THISISDEFINED()

	//from http://www.alberton.info/postgresql_meta_info.html

	var sql=L"SELECT table_name FROM information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema NOT IN ('pg_catalog', 'information_schema'); ";

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	PGresultptr pgresult;
	bool ok=getpgresult(sql,pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return L"";

	var filenames=L"";
	int nfiles=PQntuples(pgresult);
	for (int filen=0; filen<nfiles; filen++) {
		if (!PQgetisnull(pgresult, filen, 0))
			//filenames^= FM ^ wstringfromUTF8((UTF8*)PQgetvalue(pgresult, filen, 0), PQgetlength(pgresult, filen, 0));
			filenames^= FM ^ getresult(pgresult, filen, 0);
	}
	filenames.splicer(1,1,L"");

	//PQclear(pgresult);

	return filenames;
}

bool var::selectpending() const
{
	//THISIS(L"var var::selectpending() const")
	//could allow undefined usage since *this isnt used?
	//THISISSTRING()

	//default cursor is ""
	const_cast<var&>(*this).setifunassigned(L"");

	//from http://www.alberton.info/postgresql_meta_info.html

	var sql=L"SELECT name from pg_cursors where name = 'cursor1_" ^ (*this) ^ SQ;
	//var sql=L"SELECT name from pg_cursors";

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	PGresultptr pgresult;
	bool ok=getpgresult(sql,pgresult, pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return L"";

	//MUST PQclear(pgresult) on path below

	ok = PQntuples(pgresult)>0;

	if (GETDBTRACE)
		exodus::logputl(L"DBTRACE: ::selectpending() is " ^ var(ok));
	/*
        var names=L"";
        int nn=PQntuples(pgresult);
        for (var ii=0; ii<nn; ii++)
        {
                //if (!PQgetisnull(pgresult, ii, 0))
                        names^= FM ^ getresult(pgresult, ii, 0);
        }
        //names.splicer(1,1,L"");
	names ^= nn;
	names.outputl(L"names=");
	*/

	//PQclear(pgresult);

	return ok;
}

var var::listindexes(const var& filename, const var& fieldname) const
{
	THISIS(L"var var::listindexes(const var& filename) const")
	//could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename)

	//TODO for some reason doesnt return the exodus index_file__fieldname records
	//perhaps you have to be connected with sufficient postgres rights
	var sql=
		L"SELECT relname"
		L" FROM pg_class"
		L" WHERE oid IN ("
			L" SELECT indexrelid"
			L" FROM pg_index, pg_class"
			L" WHERE";
	if (filename)
		sql^=L" relname = '" ^ filename.lcase() ^  L"' AND ";
	//if (fieldname)
	//	sql^=L" ???relname = '" ^ fieldname.lcase() ^  L"' AND ";
	sql^=L" pg_class.oid=pg_index.indrelid"
		 L" AND indisunique != 't'"
		 L" AND indisprimary != 't'"
		 L");";

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	//execute command or return empty string
	PGresultptr pgresult;
	bool ok=getpgresult(sql,pgresult,pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return L"";

	//MUST PQclear(pgresult) below

	var tt;
	var indexname;
	var indexnames=L"";
	int nindexes=PQntuples(pgresult);
	var lc_fieldname=fieldname.lcase();
	for (int indexn=0; indexn<nindexes; indexn++)
	{
		if	(!PQgetisnull(pgresult, indexn, 0))
		{
			//indexname=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, indexn, 0), PQgetlength(pgresult, indexn, 0));
			indexname=getresult(pgresult, indexn, 0);
			if (indexname.substr(1,6)==L"index_")
			{
				tt=indexname.index(L"__");
				if (tt)
				{
					indexname.substrer(8,999999).swapper(L"__",VM);
					if (fieldname && indexname.a(1,2) != lc_fieldname)
						continue;

					//indexnames^=FM^indexname;
					var fn;
					if (not indexnames.locateby(indexname,L"AL",fn,0))
						indexnames.inserter(fn,indexname);
				}
			}
		}
	}
	//indexnames.splicer(1,1,L"");

	//PQclear(pgresult);

	return indexnames;
}

var var::reccount(const var& filename) const
{
	THISIS(L"var var::reccount(const var& filename) const")
	//could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename)

	//vacuum otherwise unreliable
	this->flushindex(filename);

	var sql=L"SELECT reltuples::integer FROM pg_class WHERE relname = '" ^ filename.lcase() ^  L"';";

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	//execute command or return empty string
	PGresultptr pgresult;
	bool ok=getpgresult(sql,pgresult,pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return L"";

	//MUST PQclear(pgresult) below

	var reccount=L"";
        if (PQntuples(pgresult) && PQnfields(pgresult)>0 && !PQgetisnull(pgresult, 0, 0))
        {
                reccount=var((int)ntohl(*(uint32_t*)PQgetvalue(pgresult, 0, 0)));
        }

	//PQclear(pgresult);

	return reccount;
}

var var::flushindex(const var& filename) const
{
	THISIS(L"var var::flushindex(const var& filename="") const")
	//could allow undefined usage since *this isnt used?
	THISISDEFINED()
	ISSTRING(filename)

	var sql=L"VACUUM";
	if (filename)
		sql^=L" "^filename.lcase();
	sql^=L";";
	//sql.outputl(L"sql=");

	PGconn* pgconn=(PGconn*) connection();
	if (pgconn==NULL)
		return L"";

	//execute command or return empty string
	PGresultptr pgresult;
	bool ok=getpgresult(sql,pgresult,pgconn);

	Resultclearer clearer(pgresult);

	if (!ok)
		return L"";

	//MUST PQclear(pgresult) below

	var flushresult=L"";
        if (PQntuples(pgresult))
        {
                flushresult=true;
        }

	//PQclear(pgresult);

	return flushresult;
}

//used for sql commands that require no parameters
//returns 1 for success
//returns 0 for failure
//NB in either case caller MUST PQclear(pgresult)
static bool getpgresult(const var& sql, PGresultptr& pgresult, PGconn * thread_pgconn)
{
	DEBUG_LOG_SQL

	/* dont use PQexec because is cannot be told to return binary results
	 and use PQexecParams with zero parameters instead
	//execute the command
	pgresult = getpgresult(thread_pgconn, sql.toString().c_str());
	pgresult = pgresult;
	*/

	//parameter array but no parameters
	const char* paramValues[1];
	int		 paramLengths[1];
	int		 paramFormats[1];

	//PGresult*
	//will contain any pgresult IF successful
	//MUST do PQclear(pgresult) after using it;
	//PGresult* local_result = PQexecParams(thread_pgconn,
	pgresult = PQexecParams(thread_pgconn,
		sql.toString().c_str(),
		0,	   /* zero params */
		NULL,	/* let the backend deduce param type */
		paramValues,
		paramLengths,
		paramFormats,
		1);	  /* ask for binary results */

	//NO! pgresult is returned to caller to extract any data AND call PQclear(pgresult)
	//Resultclearer clearer(pgresult);

	if (!pgresult) {

		#if TRACING >=1
			exodus::errputl(L"ERROR: mvdbpostgres PQexec command failed, no error code: ");
		#endif

		//PQclear(pgresult);
		return false;

	} else {

		switch (PQresultStatus(pgresult)) {
		case PGRES_COMMAND_OK:
			#if TRACING >= 3
				const char *str_res;
				str_res = PQcmdTuples(pgresult);
				if (strlen(str_res) > 0) {
					exodus::logputl(L"Command executed OK, " ^ var(str_res) ^ L" rows.");
				} else {
					exodus::logputl(L"Command executed OK, 0 rows.");
				}
			#endif

			return true;

		case PGRES_TUPLES_OK:

#if TRACING >= 3
			exodus::logputl(L"Select executed OK, " ^ var(PQntuples(pgresult)) ^ L" rows found.");
#endif

			return true;

		case PGRES_NONFATAL_ERROR:

#if TRACING >= 1
				exodus::errputl(L"ERROR: mvdbpostgres SQL non-fatal error code " ^ var(PQresStatus(PQresultStatus(pgresult))) ^ L", " ^ var(PQresultErrorMessage(pgresult)));
#endif

			return true;

		default:

#if TRACING >= 1
			if (sql.field(L" ",1,2) !="FETCH NEXT") {
				exodus::errputl(L"ERROR: mvdbpostgres pqexec "^var(sql));
				exodus::errputl(L"ERROR: mvdbpostgres pqexec "^var(PQresStatus(PQresultStatus(pgresult))) ^ L": " ^ var(PQresultErrorMessage(pgresult)));
			}
#endif

			//this is defaulted above for safety
			//retcode=0;
			//PQclear(pgresult);
			return false;

		}

	//should never get here
	//PQclear(pgresult);

	}

	//should never get here
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

		//sql:=crlf:'CREATE OR REPLACE VIEW ':viewprefix:sqlfilename:viewsuffix:' AS SELECT '
		sql ^= crlf ^ "CREATE VIEW " ^ viewprefix ^ sqlfilename ^ viewsuffix ^ " AS SELECT ";

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

		//sql:=crlf:'CREATE OR REPLACE VIEW ':viewprefix:sqlfilename:'_LINES':viewsuffix:' AS SELECT '
		sql2 ^= crlf ^ "CREATE VIEW " ^ viewprefix ^ sqlfilename ^ "_LINES" ^ viewsuffix ^ " AS SELECT ";

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
			expression = "exodus_extract_text(data," ^ dict.a(2) ^ "," ^ extractvarno ^ ",0)";
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

		}else if (conversion == "[NUMBER,0]" or dict.a(11) == "0N" or (dict.a(11)).substr(1, 3) == "0N_") {
			expression.splicer(1, 0, "cast(");
			expression ^= " as integer)";
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}else if (conversion.substr(1, 2) == "MD" or conversion.substr(1, 7) == "[NUMBER" or (dict.a(11)).substr(1, 7) == "[NUMBER" or dict.a(12) == "FLOAT" or (dict.a(11)).index("0N", 1)) {
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
		sql2 ^= " ON " ^ datafileprefix ^ sqlfilename ^ "_LINES.KEY = " ^ datafileprefix ^ sqlfilename ^ ".key";
		sql2 ^= crlf ^ ";";

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


}//of namespace exodus
