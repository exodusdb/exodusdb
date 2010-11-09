/*
Copyright (c) 2009 Stephen John Bush

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

#ifndef DEBUG
# define TRACING 0
#else
# define TRACING 2
#endif

#include "MurmurHash2_64.h"

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

#include <boost/thread/tss.hpp>
//http://beta.boost.org/doc/libs/1_41_0/doc/html/unordered.html

//#define HAVE_TR1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  HAVE_CXX0X
#  include <unordered_set>
#  define UNORDEREDSET std::unordered_set<uint64_t>

//expect bug here since centos5.3 32bit has tr1 but not with an unordered set
#elif defined(HAVE_TR1)
#  include <tr1/unordered_set>
#  define UNORDEREDSET std::tr1::unordered_set<uint64_t>

//assume we have a recent boost on windows
//boost 1.33 doesnt seem to have unordered set
#elif defined(HAVE_BOOST_UNORDERED_SET) || defined(_MSC_VER)
#  include <boost/unordered_set.hpp>
#  define UNORDEREDSET boost::unordered_set<uint64_t>
#else
#  define USE_MAP_FOR_UNORDERED
#  include <map>
#  define UNORDEREDSET std::map<uint64_t,int>
#endif

//see exports.txt for a list of all PQ functions
//#include <postgresql/libpq-fe.h>//in postgres/include
#include <libpq-fe.h>//in postgres/include

#define MV_NO_NARROW

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvenvironment.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

//#if TRACING >= 5
#define DEBUG_LOG_SQL if (DBTRACE) {exodus::errputl(L"SQL:" ^ var(sql));}
#define DEBUG_LOG_SQL1 if (DBTRACE) {exodus::errputl(L"SQL:" ^ var(sql).swap(L"$1",var(paramValues[0])));}
//#else
//#define DEBUG_LOG_SQL
//#define DEBUG_LOG_SQL1
//#endif

namespace exodus {

bool startipc();

//#define PGDATAFILEPREFIX "data_"
#define PGDATAFILEPREFIX L""

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

//typedef boost::unordered_set<uint64_t> LockTable; 
//typedef UNORDEREDSET<uint64_t> LockTable;
typedef UNORDEREDSET LockTable;
//typedef tr1::unordered_set<uint64_t> LockTable;

//this is not threadsafe
//PGconn     *thread_pgconn;
//but this is ...
boost::thread_specific_ptr<PGconn> tss_pgconns;
boost::thread_specific_ptr<var> tss_pgconnparams;
boost::thread_specific_ptr<bool> tss_ipcstarted;
boost::thread_specific_ptr<LockTable> tss_locktables;

//this is not thread safe since it is at global scope
//var _STATUS;

/* not used anywhere in postgres interface
#define MV_MAX_KEY_LENGTH 8092-1
#define MV_MAX_KEY_LENGTH_EXCEEDED "MV_MAX_KEY_LENGTH_EXCEEDED"
#define MV_MAX_RECORD_LENGTH 1048576-1
#define MV_MAX_RECORD_LENGTH_EXCEEDED "MV_MAX_RECORD_LENGTH_EXCEEDED"
#define MV_MAX_FILENAME_LENGTH 8092-1
#define MV_MAX_FILENAME_LENGTH_EXCEEDED "MV_MAX_FILENAME_LENGTH_EXCEEDED"
#define MV_MAX_CONNECTPARAM_LENGTH 8092-1
#define MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED "MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED"
#define MV_BAD_FILENAME "MV_BAD_FILENAME"
*/

typedef PGresult* 	PGresultptr;
int pqexec(const var& sql, PGresultptr& pgresult);

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

bool var::connect(const var& conninfo)
{
	THISIS(L"bool var::connect(const var& conninfo)")
	//the idea is for exodus to have access to one standard database without secret password
	var defaultconninfo=L"host=127.0.0.1 port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10";
	//priority is
	//1) given parameters or last connection parameters
	//2) individual environment parameters
	//3) environment connection string
	//4) config file parameters
	//5) default parameters

	//nb dont log/trace or otherwise output the full connection info without HIDING the password
	THISISDEFINED()
	ISSTRING(conninfo)

	var_mvtyp=pimpl::MVTYPE_UNA;

	//find connection details
	var conninfo2=conninfo;

	//if no conninfo details provided then use last connection details if any
	if (!conninfo && tss_pgconnparams.get())
		conninfo2==*tss_pgconnparams.get();
	
	//otherwise search for details from exodus config file
	//if incomplete connection parameters provided
	if (not conninfo2.index(L"host=") or 
		not conninfo2.index(L"port=") or
		not conninfo2.index(L"dbname=") or
		not conninfo2.index(L"user=") or
		not conninfo2.index(L"password=")
		)
	{

		//discover any configuration file parameters
		//TODO parse config properly instead of just changing \n\r to spaces!
		var configfilename=L"";
		var home=L"";
		if (home.osgetenv(L"HOME"))
			configfilename=home^SLASH^L".exodus";
		else if (home.osgetenv(L"USERPROFILE"))
			configfilename^=home^SLASH^L".exodus";
		var configconn=L"";
		if (!configconn.osread(configfilename))
			configconn.osread(".exodus");

		//discover any configuration in the environment
		var envconn=L"";
		var temp;
		if (temp.osgetenv(L"EXODUS_CONNECTION"))
			envconn^=L" "^temp;

		//specific variable are appended ie override
		if (temp.osgetenv(L"EXODUS_HOST"))
			envconn^=L" host="^temp;

		if (temp.osgetenv(L"EXODUS_PORT"))
			envconn^=L" port="^temp;

		if (temp.osgetenv(L"EXODUS_USER"))
			envconn^=L" user="^temp;

		if (temp.osgetenv(L"EXODUS_DBNAME"))
			envconn^=L" dbname="^temp;

		if (temp.osgetenv(L"EXODUS_PASSWORD"))
			envconn^=L" password="^temp;

		if (temp.osgetenv(L"EXODUS_TIMEOUT"))
			envconn^=L" connect_timeout="^temp;

		conninfo2=defaultconninfo^L" "^configconn^L" "^envconn^L" "^conninfo2;

	}
 
	//disconnect any previous connection
	if (tss_pgconns.get())
		disconnect();

	//connect or fail
	PGconn* pgconn;
	do
	{
#if defined _MSC_VER //|| defined __CYGWIN__ || defined __MINGW32__
		if (not msvc_PQconnectdb(&pgconn,conninfo2.tostring()))
		{
			//#if TRACING >= 1
				var libname=L"libpq.dll";
				//var libname=L"libpq.so";
				exodus::errputl(L"var::connect() Cannot load shared library " ^ libname ^ L". Verify configuration PATH contains postgres's \\bin.");
			//#endif
			return false;
		};
#else
		pgconn=PQconnectdb(conninfo2.tostring().c_str());
#endif

		if (PQstatus(pgconn) == CONNECTION_OK || conninfo2)
			break;

		//required even if connect fails according to docs
		PQfinish(pgconn);

		//try again with default conninfo
		conninfo2=defaultconninfo;

	} while (true);

	if (PQstatus(pgconn) != CONNECTION_OK)
	{
//		#if TRACING >= 2
			exodus::errputl(L"var::connect() Connection to database failed: " ^ var(PQerrorMessage(pgconn)));
			//if (not conninfo2)
				exodus::errputl(L"var::connect() Postgres connection configuration missing or incorrect. Please login.");
;
//		#endif
		//required even if connect fails according to docs
		PQfinish(pgconn);

		return false;
	}
	#if TRACING >= 3
		exodus::logputl(L"var::connect() Connection to database succeeded.");
	#endif

	//abort if multithreading and it is not supported
	#ifdef PQisthreadsafe
		if (!PQisthreadsafe())
		{
			//TODO only abort if environmentn>0
			throw MVDBException(L"connect(): Postgres PQ library is not threadsafe");
		}
	#endif

	*this=1;

	//save the connection in thread specific storage
	tss_pgconns.reset(pgconn);
	tss_pgconnparams.reset(new var(conninfo2));
	tss_locktables.reset(new LockTable());

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
	var sql=L"SET client_min_messages = WARNING";

	sql.sqlexec();

	return true;

}

//use void pointer to avoid need for including postgres headers in mv.h or any fancy class hierarchy
//(assumes accurate programming by system programmers in exodus mvdb routines)
void* var::connection() const
{
	THISIS(L"void* var::connection() const")

	//provides jit connection and caching

	//get the current thread connection if any
	PGconn* thread_pgconn=tss_pgconns.get();

	//autoconnect if not connected
	if (!thread_pgconn)
	{
		//cast away const otherwise we have to remove const from all the var::select() methods to reflect
		//that autoconnection could cause connect() to update the base var (this)
		//if (!(const_cast<const var>(*this)).connect())
		//dodge the issue for now
		if (!var().connect())
		{
			//handle failure to connect here to avoid error handling on every connection
			//calling process can always use connect() or try/catch
			//throw MVDBException(L"connection(): Cannot connect. Please login");
			return NULL;
		}
		//get the new connection
		thread_pgconn=tss_pgconns.get();
	}
	return (void*) thread_pgconn;
}

bool var::disconnect()
{
	THISIS(L"bool var::disconnect()")

	THISISDEFINED()

	#if TRACING >= 3
		exodus::errputl(L"var::disconnect() Closing connection");
	#endif

	PGconn* thread_pgconn=tss_pgconns.get();

	if (thread_pgconn)
		PQfinish(thread_pgconn);

	tss_pgconns.release();
	tss_locktables.release();

	var_mvstr=L"";
	var_mvint=0;

	//make unassigned()
	var_mvtyp=pimpl::MVTYPE_UNA;

	return true;

}

bool var::open(const var& filename)
{
	THISIS(L"bool var::open(const var& dictcode,const var& filename)")
	THISISDEFINED()
	ISSTRING(filename)

    //dumb version of read to see if file exists
    //should perhaps prepare pg parameters for repeated speed

    var key=L"";

    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];
//    uint32_t    binaryIntVal;

    /* Here is our out-of-line parameter value */
	std::string key2=key.tostring();
    paramValues[0] = key2.c_str();
    paramLengths[0] = int(key2.length());
    paramFormats[0] = 1;//binary

	//TODO optimise by doing something other than SELECT * where key = ""?
	var sql=L"SELECT key FROM "^filename.convert(L".",L"_")^L" WHERE key = $1";

    PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
	PGresult* result = PQexecParams(thread_pgconn,
    					//TODO: parameterise filename
                       sql.tostring().c_str(),
                       1,       /* one param */
                       NULL,    /* let the backend deduce param type */
                       paramValues,
					   paramLengths,
					   paramFormats,
                       1);      /* ask for binary results */

    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
		PQclear(result);
		#if TRACING >= 2
			var errmsg=PQerrorMessage(thread_pgconn);
			if (!errmsg.index(L"does not exist"))
		        exodus::errputl(L"OPEN failed: " ^ errmsg);
        #endif
        return false;
    }

    PQclear(result);

	//save the filename
	var_mvstr=L"";
	//var_mvstr+=filename.var_mvstr;
	var_mvstr+=filename.convert(L". ",L"__").towstring();
	var_mvtyp=pimpl::MVTYPE_STR;

	return true;

}

void var::close()
{
	THISIS(L"void var::close()")
	THISISSTRING()
/*TODO
	if (var_mvtyp!=MVTYPE_UNA) QMClose(var_mvint);
*/
}

bool var::readv(const var& filehandle, const var& key, const int fieldno)
{
	THISIS(L"bool var::readv(const var& filehandle, const var& key, const int fieldno)")

	if (!read(filehandle,key))
		return false;
	var_mvstr=extract(fieldno).var_mvstr;
	return true;
}

bool var::read(const var& filehandle,const var& key)
{
	THISIS(L"bool var::read(const var& filehandle,const var& key)")
	THISISDEFINED()
	ISSTRING(filehandle)
	ISSTRING(key)

    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];
	//uint32_t    binaryIntVal;

	std::string key2=key.tostring();

	paramValues[0]=key2.c_str();
	paramLengths[0]=int(key2.length());
	paramFormats[0]=1;

	var sql=L"SELECT data FROM " PGDATAFILEPREFIX ^ filehandle ^ L" WHERE key = $1";

    PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
	PGresult* result = PQexecParams(thread_pgconn,
    					//TODO: parameterise filename
                       sql.tostring().c_str(),
                       1,       /* one param */
                       NULL,    /* let the backend deduce param type */
                       paramValues,
					   paramLengths,
					   paramFormats,
                       1);      /* ask for binary results */

	if (PQresultStatus(result) != PGRES_TUPLES_OK)
 	{
		PQclear(result);
		throw MVException(L"read(" ^ filehandle ^ L", " ^ key ^ L") - probably file not opened or doesnt exist\n" ^ var(PQerrorMessage(thread_pgconn)));
		return false;
	}

	if (PQntuples(result) < 1)
	{
		PQclear(result);
		return false;
	}

	if (PQntuples(result) > 1)
 	{
		PQclear(result);
		exodus::errputl(L"var::read() SELECT returned more than one record");
		return false;
	}

	*this=wstringfromUTF8((UTF8*)PQgetvalue(result, 0, 0), PQgetlength(result, 0, 0));

	PQclear(result);

	return true;

}

bool var::lock(const var& key) const
{
	//on postgres, repeated locks for the same thing (from the same connection) succeed and stack up
	//they need the same number of unlocks (from the same connection) before other connections
	//can take the lock
	//unlock returns true if a lock (your lock) was released and false if you dont have the lock
	
	THISIS(L"bool var::lock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];
	//uint32_t    binaryIntVal;

	std::wstring fileandkey=var_mvstr;
	fileandkey.append(L" ");
	fileandkey.append(key.var_mvstr);

	//TODO .. provide endian identical version
	//required if and when exodus processes connect to postgres on a DIFFERENT host
	//although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the SAME host
	uint64_t hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);

	//check if already lock in current connection
	LockTable* locktable=tss_locktables.get();
	if (locktable)
	{
		//if already in local lock table then dont lock on database
		//since postgres stacks up multiple locks
		//whereas multivalue databases dont
		if (((*locktable).find(hash64))!=(*locktable).end())
			//TODO indicate in some global variable "OWN LOCK"
			return false;
		//register that it is locked
#ifdef USE_MAP_FOR_UNORDERED
		std::pair<const uint64_t,int> lock(hash64,0);
		(*locktable).insert(lock);
#else
		(*locktable).insert(hash64);
#endif
	}

	paramValues[0]=(char*)&hash64;
	paramLengths[0]=sizeof(uint64_t);
	paramFormats[0]=1;

	const char* sql="SELECT PG_TRY_ADVISORY_LOCK($1)";

    PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
	PGresult* result = PQexecParams(thread_pgconn,
    					//TODO: parameterise filename
                       sql,
                       1,       /* one param */
                       NULL,    /* let the backend deduce param type */
                       paramValues,
					   paramLengths,
					   paramFormats,
                       1);      /* ask for binary results */

	if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) != 1)
 	{
		PQclear(result);
		throw MVException(L"lock(" ^ (*this) ^ L", " ^ key ^ L")\n" ^ var(PQerrorMessage(thread_pgconn)));
		return false;
	}

	//*this=wstringfromUTF8((UTF8*)PQgetvalue(result, 0, 0), PQgetlength(result, 0, 0));

	//std::wstring temp=wstringfromUTF8((UTF8*)PQgetvalue(result, 0, 0), PQgetlength(result, 0, 0));

	bool lockedok= *PQgetvalue(result, 0, 0)!=0;

	PQclear(result);

	return lockedok;
}

void var::unlock(const var& key) const
{

	THISIS(L"void var::unlock(const var& key) const")
	THISISDEFINED()
	ISSTRING(key)

    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];

	std::wstring fileandkey=var_mvstr;
	fileandkey.append(L" ");
	fileandkey.append(key.var_mvstr);

	//TODO .. provide endian identical version
	//required if and when exodus processes connect to postgres on a DIFFERENT host
	//although currently (Sep2010) use of symbolic dictionaries requires exodus to be on the SAME host
	uint64_t hash64=MurmurHash64((wchar_t*)fileandkey.data(),int(fileandkey.length()*sizeof(wchar_t)),0);

	//remove from local current connection locktable
	LockTable* locktable=tss_locktables.get();
	if (locktable)
	{
		//if not in local locktable then no need to unlock on database
		if (((*locktable).find(hash64))==(*locktable).end())
			return;
		//register that it is unlocked
		(*locktable).erase(hash64);
	}

	paramValues[0]=(char*)&hash64;
	paramLengths[0]=sizeof(uint64_t);
	paramFormats[0]=1;

	const char* sql="SELECT PG_ADVISORY_UNLOCK($1)";

    PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return;
	DEBUG_LOG_SQL1
	PGresult* result = PQexecParams(thread_pgconn,
    					//TODO: parameterise filename
                       sql,
                       1,       /* one param */
                       NULL,    /* let the backend deduce param type */
                       paramValues,
					   paramLengths,
					   paramFormats,
                       1);      /* ask for binary results */

	if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) != 1)
 	{
		PQclear(result);
		throw MVException(L"unlock(" ^ (*this) ^ L", " ^ key ^ L")\n" ^ var(PQerrorMessage(thread_pgconn)));
		return;
	}

	//bool unlockedok= *PQgetvalue(result, 0, 0)!=0;

	PQclear(result);

	return;
}

void var::unlockall() const
{
	THISIS(L"void var::unlockall() const")

	//check if any locks
	LockTable* locktable=tss_locktables.get();
	if (locktable)
	{
		//if local lock table is empty then dont unlock all database
		if ((*locktable).begin()==(*locktable).end())
			//TODO indicate in some global variable "OWN LOCK"
			return;
		//register that it is locked
		(*locktable).clear();
	}

	var sql=L"SELECT PG_ADVISORY_UNLOCK_ALL()";

	sql.sqlexec();

}

//returns success or failure but no data
bool var::sqlexec() const
{
	var errmsg;
	return sqlexec(errmsg);
}

//returns success or failure but no data
bool var::sqlexec(var& errmsg) const
{
	THISIS(L"bool var::sqlexec(var& errmsg) const")
	THISISSTRING()

	//check/establish connection
	if (!connection())
	{
		errmsg=L"Error: sqlexec failed to connect to database";
		return false;
	}

	/* dont use pqexec here since it cannot execute multiple commands
	PGresultptr result;
	if (!pqexec(*this,result)) {
		PGconn* thread_pgconn=(PGconn*) connection();
		errormsg=var(PQerrorMessage(thread_pgconn));
		return false;
	}
	PQclear(result);
	return true;
	*/

	PGconn* thread_pgconn=tss_pgconns.get();
	if (!thread_pgconn)
	{
		errmsg=L"Error: sqlexec cannot find thread database connection";
		return 0;
	}

	//DEBUG_LOG_SQL
	if (DBTRACE) {exodus::errputl(L"SQL:" ^ *this);}

	//will contain any result IF successful
	//MUST do PQclear(local_result) after using it;
	PGresult* pgresult;

	//NB PQexec cannot be told to return binary results
	//but it can execute multiple commands
	//whereas PQexecParams is the opposite
	pgresult = PQexec(thread_pgconn, (*this).tostring().c_str());
	if (!pgresult) {
		errmsg=var(PQerrorMessage(thread_pgconn));
		return false;
	}
	if (PQresultStatus(pgresult) != PGRES_COMMAND_OK)
    {
        errmsg=var(PQerrorMessage(thread_pgconn));
		//std::wcerr<<errmsg<<std::endl;
		PQclear(pgresult);
        return false;
    }
	errmsg=var(PQntuples(pgresult));
	PQclear(pgresult);
	return true;

}

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
	record.replacer(fieldno,0,0,var_mvstr);

	//write it back
	record.write(filehandle,key);

	return true;

}

/* prepared statement version doesnt seem to make much difference approx -10% - possibly because two field file is so simple
bool var::write(const var& filehandle,const var& key) const
{

	if (!var_mvtyp) throw MVUnassigned(L"write()");
	if (!filehandle.var_mvtyp) throw MVUnassigned(L"write(filehandle)");
	if (!key.var_mvtyp) throw MVUnassigned(L"write(key)");

	if (key==0)
	{

//	PGresult *PQprepare(PGconn *thread_pgconn,
//		const wchar_t *stmtName,
//		const wchar_t *query,
//		int nParams,
//		const Oid *paramTypes);

	Oid paramTypes[3];
	paramTypes[0]=BYTEAOID;
	paramTypes[1]=TEXTOID;
	paramTypes[2]=TEXTOID;

	PGresult* res1;
	res1 = PQprepare(thread_pgconn,
						"PREPAREDINSERT",
						"INSERT INTO TEST (key, data) values($2, $3)",
						3,
						paramTypes);

	if (PQresultStatus(res1) != PGRES_COMMAND_OK)
    {
        cerr<<L"var::write() failed: Prepare: "<<PQerrorMessage(thread_pgconn)<<endl;
        PQclear(res1);
        return false;
	}

	}

    // Here is our out-of-line parameter value
    const char* paramValues[3];
    int         paramLengths[3];
    int         paramFormats[3];

	std::string=filehandle2 filehandle.tostring();
	std::string=key2 key.tostring();
	std::string=data2 data.tostring();

	paramValues[0] = filehandle2.data();//filename
    paramValues[1] = key2.data();//key
    paramValues[2] = data2.data();//data

    paramLengths[0] = filehandle2.length();//filename
    paramLengths[1] = key2.length();//key
    paramLengths[2] = data2.length();//data

    paramFormats[0] = 1;//binary
    paramFormats[1] = 1;//binary
    paramFormats[2] = 1;//binary

//	PGresult *PQexecPrepared(PGconn *thread_pgconn,
//                         const wchar_t *stmtName,
//                         int nParams,
//                         const wchar_t * const *paramValues,
//                         const int *paramLengths,
//                         const int *paramFormats,
//                         int resultFormat);
	PGresult* result;
	result = PQexecPrepared(thread_pgconn,
						"PREPAREDINSERT",
						3,
						paramValues,
						paramLengths,
						paramFormats,
						1);      // ask for binary results

//    if (PQntuples(result) != 1)
	if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        cerr<<L"var::write() failed: "<<PQntuples(result)<<L" "<<PQerrorMessage(thread_pgconn)<<endl;
        PQclear(result);
        return false;
    }

//    if (PQntuples(result) != 1)
//    {
//        cerr<<L"var::write() failed: "<<PQntuples(result)<<L" rows returned"<<endl;
//        PQclear(result);
//        return false;
//    }

    PQclear(result);

	return true;

}
*/

//there is no "update if present or insert if not" command in postgres
//sql2003 "merge" http://en.wikipedia.org/wiki/Upsert
bool var::write(const var& filehandle, const var& key) const
{
	THISIS(L"bool var::write(const var& filehandle, const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	const wchar_t* mode=L"write()";

    const char* paramValues[2];
    int         paramLengths[2];
    int         paramFormats[2];
    //uint32_t    binaryIntVal;

	std::string key2=key.tostring();
	std::string data2=(*this).tostring();

    paramValues[0] = key2.data();
    paramValues[1] = data2.data();

    paramLengths[0] = int(key2.length());
    paramLengths[1] = int(data2.length());

    paramFormats[0] = 1;//binary
    paramFormats[1] = 1;//binary

	var sql;

	//try update first and if it fails then try insert

	sql = L"UPDATE " PGDATAFILEPREFIX ^ filehandle ^ L" SET data = $2 WHERE key = $1";

	PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
    PGresult* result = PQexecParams(thread_pgconn,
    					//TODO: parameterise filename
                       sql.tostring().c_str(),
                       2,				// two params (key and data)
                       NULL,			// let the backend deduce param type
                       paramValues,
                       paramLengths,
                       paramFormats,
                       1);				// ask for binary results
	if (PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		PQclear(result);
        exodus::errputl(L"var::write() failed: " ^ var(PQntuples(result)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
		return false;
	}

	//if updated 1 then OK because update worked and no need to try insert below
    if (strcmp(PQcmdTuples(result),"1") == 0)
    {
		PQclear(result);
 		return true;
	}

	//if update fails then try insert

    PQclear(result);
	sql = L"INSERT INTO " PGDATAFILEPREFIX ^ filehandle ^ L" (key,data) values( $1 , $2)";
	DEBUG_LOG_SQL1
	result = PQexecParams(thread_pgconn,
		sql.tostring().c_str(),
		2,				// two params (key and data)
		NULL,			// let the backend deduce param type
		paramValues,
		paramLengths,
		paramFormats,	// bytea
		1);				// ask for binary results

	if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
		PQclear(result);
        exodus::errputl(L"var::write() INSERT failed: " ^ var(PQntuples(result)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
        return false;
    }

	//if not updated 1 then fail
    if (strcmp(PQcmdTuples(result),"1") != 0)
    {
		PQclear(result);
 		return false;
	}

	PQclear(result);
	return true;

}

//"updaterecord" is non-standard for pick - but allows "write only if already exists" logic

bool var::updaterecord(const var& filehandle,const var& key) const
{
	THISIS(L"bool var::updaterecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	const wchar_t* mode=L"updaterecord()";

    const char* paramValues[2];
    int         paramLengths[2];
    int         paramFormats[2];
    //uint32_t    binaryIntVal;

	std::string key2=key.tostring();
	std::string data2=(*this).tostring();

    paramValues[0] = key2.data();
    paramValues[1] = data2.data();

    paramLengths[0] = int(key2.length());
    paramLengths[1] = int(data2.length());

    paramFormats[0] = 1;//binary
    paramFormats[1] = 1;//binary

	var sql = L"UPDATE " PGDATAFILEPREFIX ^ filehandle ^ L" SET data = $2 WHERE key = $1";

	PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
    PGresult* result = PQexecParams(thread_pgconn,
		//TODO: parameterise filename
							  sql.tostring().c_str(),
		2,				// two params (key and data)
		NULL,			// let the backend deduce param type
		paramValues,
		paramLengths,
		paramFormats,	// bytea
		1);				// ask for binary results
	if (PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		PQclear(result);
		exodus::errputl(L"var::update failed: " ^ var(PQntuples(result)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
		return false;
	}

	//if not updated 1 then fail
    if (strcmp(PQcmdTuples(result),"1") != 0)
    {
		PQclear(result);
 		return false;
	}

	PQclear(result);
	return true;

}

//"insertrecord" is non-standard for pick - but allows faster writes under "write only if doesnt already exist" logic

bool var::insertrecord(const var& filehandle,const var& key) const
{
	THISIS(L"bool var::insertrecord(const var& filehandle,const var& key) const")
	THISISSTRING()
	ISSTRING(filehandle)
	ISSTRING(key)

	const wchar_t* mode=L"insertrecord()";

	const char* paramValues[2];
    int         paramLengths[2];
    int         paramFormats[2];
    //uint32_t    binaryIntVal;

	std::string key2=key.tostring();
	std::string data2=(*this).tostring();

    paramValues[0] = key2.data();
    paramValues[1] = data2.data();

    paramLengths[0] = int(key2.length());
    paramLengths[1] = int(data2.length());

    paramFormats[0] = 1;//binary
    paramFormats[1] = 1;//binary

	var sql = L"INSERT INTO " PGDATAFILEPREFIX ^ filehandle ^ L" (key,data) values( $1 , $2)";

	PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
	PGresult* result = PQexecParams(thread_pgconn,
		//TODO: parameterise filename
		sql.tostring().c_str(),
		2,				// two params (key and data)
		NULL,			// let the backend deduce param type
		paramValues,
		paramLengths,
		paramFormats,	// bytea
		1);				// ask for binary results

	if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
		exodus::errputl(L"var::insertrecord INSERT failed: " ^ var(PQntuples(result)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
        PQclear(result);
        return false;
    }

	//if not updated 1 then fail
    if (strcmp(PQcmdTuples(result),"1") != 0)
    {
		PQclear(result);
 		return false;
	}

	PQclear(result);
	return true;

}

bool var::deleterecord(const var& key) const
{
	THISIS(L"bool var::deleterecord(const var& key) const")
	THISISSTRING()
	ISSTRING(key)

    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];
    //uint32_t    binaryIntVal;

	std::string key2=key.tostring();

    paramValues[0] = key2.data();

    paramLengths[0] = int(key2.length());

    paramFormats[0] = 1;//binary

	var sql=L"DELETE FROM " PGDATAFILEPREFIX ^ var_mvstr ^ L" WHERE KEY = $1";

	PGconn* thread_pgconn=(PGconn*) connection();
	if (!thread_pgconn)
		return false;
	DEBUG_LOG_SQL1
    PGresult* result = PQexecParams(thread_pgconn,
		sql.tostring().c_str(),
		1,       /* two param */
		NULL,    /* let the backend deduce param type */
		paramValues,
		paramLengths,
		paramFormats,
		1);      /* ask for binary results */

	if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
		exodus::errputl(L"var::deleterecord failed: " ^ var(PQntuples(result)) ^ L" " ^ var(PQerrorMessage(thread_pgconn)));
        PQclear(result);
        return false;
    }

	//if not updated 1 then fail
    if (strcmp(PQcmdTuples(result),"1") != 0)
    {
		PQclear(result);
 		return false;
	}


    PQclear(result);

	return true;
}

var var::xlate(const var& filename,const var& fieldno, const var& mode) const
{
	THISIS(L"var var::xlate(const var& filename,const var& fieldno, const var& mode) const")
	ISSTRING(mode)

	return xlate(filename,fieldno,mode.towstring().c_str());
}

//TODO provide a version with int fieldno to handle the most frequent case
// although may also support dictid (of target file) instead of fieldno

var var::xlate(const var& filename,const var& fieldno, const wchar_t* mode) const
{
	THISIS(L"var var::xlate(const var& filename,const var& fieldno, const wchar_t* mode) const")
	THISISSTRING()
	ISSTRING(filename)
	ISSTRING(fieldno)

	//open the file (skip this for now since sql doesnt need "open"
	var file;
	var record;
	//if (!file.open(filename))
	//{
	//	_STATUS=filename^L" does not exist";
	//	record=L"";
	//	return record;
	//}
	file=filename;

	//read the record
	if (!record.read(file,var_mvstr))
	{
		//if record doesnt exist then "", or original key if mode is "C"
	 if (mode==L"C")
			record=*this;
		else
			record=L"";
	}
	else
	{
		//extract the field or field 0 means return the whole record
		if (fieldno)
			record=record.extract(fieldno);
	}
	return record;
}

bool var::begintrans() const
{
	THISIS(L"bool var::begintrans() const")

	// *this is not used
	THISISDEFINED()

	//begin a transaction
	var sql=L"BEGIN";

	return sql.sqlexec();
	
}

bool var::rollbacktrans() const
{
	THISIS(L"bool var::rollbacktrans() const")

	// *this is not used
	THISISDEFINED()

	// Rollback a transaction
	var sql=L"BEGIN";

	return sql.sqlexec();

}

bool var::committrans() const
{
	THISIS(L"bool var::committrans() const")

	// *this is not used
	THISISDEFINED()

	//end (commit) a transaction
	var sql=L"END";

	return sql.sqlexec();
}

bool var::createdb(const var& dbname) const
{
	var errmsg;
	return createdb(dbname,errmsg);
}

bool var::deletedb(const var& dbname) const
{
	var errmsg;
	return deletedb(dbname,errmsg);
}

bool var::createdb(const var& dbname, var& errmsg) const
{
	THISIS(L"bool var::createdb(const var& dbname, var& errmsg)")
	// *this is not used
	THISISDEFINED()
	ISSTRING(dbname)

	var sql = L"CREATE DATABASE "^dbname.convert(L". ",L"__");
	sql^=L" WITH ENCODING='UTF8' ";
	//sql^=" OWNER=exodus";

	return sql.sqlexec(errmsg);

}

bool var::deletedb(const var& dbname, var& errmsg) const
{
	THISIS(L"bool var::deletedb(const var& dbname, var& errmsg)")
	// *this is not used
	THISISDEFINED()
	ISSTRING(dbname)

	var sql = L"DROP DATABASE "^dbname.convert(L". ",L"__");

	return sql.sqlexec(errmsg);

}

bool var::createfile(const var& filename, const var& options)
{
	THISIS(L"bool var::createfile(const var& filename, const var& options)")
	// *this is not used
	THISISDEFINED()
	ISSTRING(filename)
	ISSTRING(options)

	//var tablename = L"TEMP" ^ var(100000000).rnd();
	//Postgres The ON COMMIT clause for temporary tables also resembles the SQL standard, but has some differences. If the ON COMMIT clause is omitted, SQL specifies that the default behavior is ON COMMIT DELETE ROWS. However, the default behavior in PostgreSQL is ON COMMIT PRESERVE ROWS. The ON COMMIT DROP option does not exist in SQL.

	var sql = L"CREATE";
	//if (options.ucase().index(L"TEMPORARY")) sql ^= L" TEMPORARY";
	sql ^= L" TABLE " PGDATAFILEPREFIX ^ filename.convert(L".",L"_");
	sql ^= L" (key bytea primary key, data bytea)";

	return sql.sqlexec();

}

bool var::deletefile() const
{
	THISIS(L"bool var::deletefile() const")
	THISISSTRING()

	var sql = L"DROP TABLE " PGDATAFILEPREFIX ^ *this;

	return sql.sqlexec();
}

bool var::clearfile() const
{
	THISIS(L"bool var::clearfile() const")
	THISISSTRING()

	var sql = L"DELETE FROM " PGDATAFILEPREFIX ^ *this;

	return sql.sqlexec();
}

inline void unquoter(var& string)
{
        //remove "", '' and {}
        static var quotecharacters(L"\"'{");
        if (quotecharacters.index(string.substr(1,1)))
                string=string.substr(2,string.length()-2);
}

inline void tosqlstring(var& string1)
{
    //convert to sql style strings
    //use single quotes and double up any internal single quotes
    if (string1.substr(1,1)==L"\"")
    {
        string1.swapper(L"'",L"''");
        string1.splicer(1,1,L"'");
        string1.splicer(-1,1,L"'");
    }
}

inline var fileexpression(const var& mainfilename, const var& filename, const var& keyordata)
{
	var expression=filename ^ L"." ^ keyordata;
	return expression;

	//if you dont use STRICT in the postgres function declaration/definitions then NULL parameters do not abort functions

	//use COALESCE function in case this is a joined but missing record (and therefore null)
	//in MYSQL this is the ISNULL expression?
	//fromdictexpression=L"exodus_extract_bytea(coalesce(L" ^ filename ^ L".data,''::bytea), " ^ xlatefromfieldname.substr(9);
	//if (filename==mainfilename) return expression;
	//return "coalesce(L" ^ expression ^", ''::bytea)";
}

var getdictexpression(const var& mainfilename, const var& filename, const var& dictfilename, const var& dictfile, const var& fieldname, var& joins, bool forsort_or_select_or_index=false)
{

	var actualdictfile=dictfile;
	if (!actualdictfile)
	{
		var dictfilename;
		if (mainfilename.substr(1,5)== L"dict_")
			dictfilename=L"dict_md";
		else
			dictfilename=L"dict_"^mainfilename;
		if (!actualdictfile.open(dictfilename))
		{
			//throw MVDBException(L"getdictexpression() cannot open " ^ dictfilename.quote());
			exodus::errputl(L"getdictexpression() cannot open " ^ dictfilename.quote());
			return L"";
		}
	}
	//given a file and dictionary id
    //returns a postgres sql expression like (textextract(filename.data,99,0,0))
    //using one of the neosys backend functions installed in postgres like textextract, dateextract etc.
    var dictrec;
    if (!dictrec.read(actualdictfile,fieldname))
	{
		if (not dictrec.read(L"dict_md", fieldname))
		{
			if (fieldname==L"@ID")
				dictrec = L"F" ^ FM ^ L"0" ^ FM ^ L"Ref" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ L"L" ^ FM ^ 15;
			else
			{
				//throw MVDBException(L"getdictexpression() cannot read " ^ fieldname.quote() ^ L" from " ^ actualdictfile.quote());
				exodus::errputl(L"getdictexpression() cannot read " ^ fieldname.quote() ^ L" from " ^ actualdictfile.quote());
				return L"";
			}
		}
	}
    var sqlexpression;
	var dicttype=dictrec.extract(1);
    if (dicttype==L"F")
    {
        var conversion=dictrec.extract(7);
        var fieldno=dictrec.extract(2);
		var params;
		if (fieldno)
			params=fileexpression(mainfilename, filename, L"data") ^ L"," ^ fieldno ^ L", 0, 0)";
		else
			params=fileexpression(mainfilename, filename,L"key") ^ L",0,0,0)";

		if (conversion.substr(1,9)==L"[DATETIME")
			sqlexpression=L"exodus_extract_datetime(" ^ params;
		else if (conversion.substr(1,1)==L"D" || conversion.substr(1,5)==L"[DATE")
			sqlexpression=L"exodus_extract_date(" ^ params;
		else if (conversion.substr(1,2)==L"MT" || conversion.substr(1,5)==L"[TIME")
			sqlexpression=L"exodus_extract_time(" ^  params;

		//for now (until we have a extract_number/integer/float) that doesnt fail on non-numeric like cast "as integer" and "as float" does
		//note that we could use exodus_extract_sort for EVERYTHING inc dates/time/numbers etc.
		//but its large size is perhaps a disadvantage
		else if (forsort_or_select_or_index)
			sqlexpression=L"exodus_extract_sort(" ^  params;

		else if (conversion==L"[NUMBER,0]" || dictrec.extract(11)==L"0N" || dictrec.extract(11).substr(1,3)==L"0N_")
			sqlexpression=L"cast( exodus_extract_text(" ^ params ^ L" as integer)";
		else if (conversion.substr(1,2)==L"MD" || conversion.substr(1,7)==L"[NUMBER" || dictrec.extract(12)==L"FLOAT" || dictrec.extract(11).index(L"0N"))
				sqlexpression=L"cast( exodus_extract_text(" ^ params ^ L" as float)";
		else
			sqlexpression=L"exodus_extract_text(" ^ params;
	}
	else if (dicttype==L"S")
	{
		var functionx=dictrec.extract(8).trim().ucase();
		if (functionx.substr(1,11)==L"@ANS=XLATE(")
		{
			functionx.splicer(1,11,L"");
			var xlatetofilename=functionx.field(L",",1).trim();
			unquoter(xlatetofilename);
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
					todictexpression=getdictexpression(filename,xlatetofilename, dictxlatetofile, dictxlatetofile, xlatetofieldname, joins,forsort_or_select_or_index);
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
				else if (xlatefromfieldname.substr(1,1)==L"{")
				{
					xlatefromfieldname=xlatefromfieldname.substr(2).splicer(-1,1,L"");
					fromdictexpression=getdictexpression(filename, filename, dictfilename, dictfile, xlatefromfieldname, joins);
				}
				else
				{
					//throw  MVDBException(L"getdictexpression() " ^ filename.quote() ^ L" " ^ fieldname.quote() ^ L" - INVALID DICTIONARY EXPRESSION - " ^ dictrec.extract(8).quote());
					exodus::errputl(L"getdictexpression() " ^ filename.quote() ^ L" " ^ fieldname.quote() ^ L" - INVALID DICTIONARY EXPRESSION - " ^ dictrec.extract(8).quote());
					return L"";
				}

				//add the join
				var join=L"LEFT JOIN " ^ xlatetofilename ^ L" ON " ^ fromdictexpression ^ L" = " ^ xlatetofilename ^ L".key";
				var xx;
				if (!joins.locate(join,xx,1))
					joins.replacer(1,-1,0,join);

			}
		}
		else
		{
			sqlexpression=L"'" ^ fieldname ^ L"'";
			sqlexpression=L"exodus_call('exodusservice-" ^ getprocessn() ^ L"." ^ getenvironmentn() ^ L"'::bytea, '" ^ dictfilename ^ L"'::bytea, '" ^ fieldname ^ L"'::bytea, "^ filename ^ L".key, " ^ filename ^ L".data,0,0)";
			//TODO apply naturalorder conversion by passing forsort_or_select_or_index option to exodus_call
		}
	}
	else
	{
		//throw  filename ^ L" " ^ fieldname ^ L" - INVALID DICTIONARY ITEM";
		//throw  MVDBException(L"getdictexpression(" ^ filename.quote() ^ L", " ^ fieldname.quote() ^ L") invalid dictionary type " ^ dicttype.quote());
		exodus::errputl(L"getdictexpression(" ^ filename.quote() ^ L", " ^ fieldname.quote() ^ L") invalid dictionary type " ^ dicttype.quote());
		return L"";
	}
    return sqlexpression;
}

var getword(var& remainingwords)
{

    //gets the next word (or series of words separated by FM while they are numbers or quoted strings)
    //converts to sql quoted strings
    //and clips them from the input string

	static const var valuechars(L"\"'.0123456789-+");

	var word1=remainingwords.field(L" ",1);
	remainingwords=remainingwords.field(L" ",2,99999);

	//join quoted words together
	var char1=word1.substr(1,1);
	if ((char1==DQ||char1==SQ))
	{
		while (word1.substr(-1,1)!=char1)
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
	if (valuechars.index(word1.substr(1,1)))
	{
		word1 = SQ ^ word1.unquote() ^ SQ;

		var nextword=remainingwords.field(L" ",1);

		//'x' and 'y' and 'z' becomes 'x' 'y' 'z'
		//to cater for WITH fieldname NOT 'X' AND 'Y' AND 'Z'
		//duplicated above/below
		if (nextword==L"AND")
		{
			var nextword2=remainingwords;
			if (valuechars.index(nextword2.substr(1,1)))
			{
				nextword=nextword2;
				remainingwords=remainingwords.field(L" ",2,99999);
			}
		}

		while (nextword && valuechars.index(nextword.substr(1,1)))
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
			if (nextword==L"AND")
			{
				var nextword2=remainingwords;
				if (valuechars.index(nextword2.substr(1,1)))
				{
					nextword=nextword2;
					remainingwords=remainingwords.field(L" ",2,99999);
				}
			}

		}
	}else{
		word1.ucaser();
	}

    return word1;
}

bool var::selectrecord(const var& sortselectclause) const
{
	THISIS(L"bool var::selectrecord(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	//THISISDEFINED()
	ISSTRING(sortselectclause)

	return const_cast<const var&>(*this).selectx(L"key, data",sortselectclause);
}

bool var::select(const var& sortselectclause) const
{
	THISIS(L"bool var::select(const var& sortselectclause) const")
	//?allow undefined usage like var xyz=xyz.select();
	THISISDEFINED()
	ISSTRING(sortselectclause)

	return selectx(L"key",sortselectclause);
}

bool var::selectx(const var& fieldnames, const var& sortselectclause) const
{
	//private unchecked

	//?allow undefined usage like var xyz=xyz.select();
	if (var_mvtyp&mvtypemask)
	{
		//throw MVUndefined(L"selectx()");
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
	}

	if (!(var_mvtyp&pimpl::MVTYPE_STR))
	{
		if (!var_mvtyp)
		{
			var_mvstr=L"";
			var_mvtyp=pimpl::MVTYPE_STR;
		}
		else
			createString();
	}

	//TODO only do this if cursor already exists
	//clearselect();

	var actualfilename=L"";
	var dictfilename=L"";
	var actualfieldnames=fieldnames;
	var dictfile=L"";
	var keycodes=L"";
	var bykey=0;
	var wordn;

	var whereclause=L"";//exodus_call('NAME', "^ filename ^ L".data, " ^ filename ^ L".key,0,0) <> '' AND ";
	var orderclause=L"";
	var joins=L"";

	var maxnrecs=L"";

	var remainingsortselectclause=sortselectclause;

	//sortselectclause may start with {SELECT|SSELECT {maxnrecs} filename}
	var word=remainingsortselectclause.field(L" ",1).ucase();
    if (word==L"SELECT"||word==L"SSELECT")
    {
		if (word==L"SSELECT")
			bykey=1;

		//discard it and get the second word which is either max number of records or filename
		getword(remainingsortselectclause);
		word=getword(remainingsortselectclause);

		//the second word can be a limiting number of records
		if (word.isnum())
		{
			maxnrecs=word;
			word=getword(remainingsortselectclause);
		}

		actualfilename=word;
		dictfilename=word;

    }

	//optionally get filename from the current var
	if (!actualfilename)
	{
		if (!assigned())
			throw MVUnassigned(L"select() unassigned filehandle and sort/select clause doesnt start \"SELECT filename\"");
		actualfilename=*this;
		dictfilename=*this;
	}

    while (remainingsortselectclause.length())
    {

        var word1=getword(remainingsortselectclause);

        //initial numbers or strings mean record keys
        if (word1.substr(1,1).index(L"\"'0123456789."))
        {
            if (keycodes)
                keycodes ^= FM;
            keycodes ^= word1;
            continue;
		}
		else if (word1==L"USING")
		{
			dictfilename=getword(remainingsortselectclause);
			if (!dictfile.open(L"dict_"^dictfilename))
			{
				//throw MVDBException(L"select() dict_" ^ dictfilename ^ L" file cannot be opened");
				exodus::errputl(L"select() dict_" ^ dictfilename ^ L" file cannot be opened");
				return L"";
			}
        }
        else if (word1==L"BY" || word1==L"BY-DSND")
        {
            if (orderclause)
                orderclause^=L", ";
            var dictexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,getword(remainingsortselectclause),joins,true);
            orderclause ^= dictexpression;
            if (word1==L"BY-DSND")
                orderclause^=L" DESC";
        }
//           else if (word1==L"BETWEEN")
//           {
//               var word2=getword(remainingsortselectclause);
//               whereclause ^= word2;
//           }
		else if (word1==L"WITH")
		{

			//add the dictionary id
            var word2=getword(remainingsortselectclause);
			var dictexpression=getdictexpression(actualfilename,actualfilename,dictfilename,dictfile,word2,joins,true);
			var usingnaturalorder=dictexpression.index(L"exodus_extract_sort");
            whereclause ^= L" " ^ dictexpression;

			//handle STARTING, ENDING and CONTAINING
            word2=getword(remainingsortselectclause);
			var startingpercent=L"";
			var endingpercent=L"";
			if (word2==L"CONTAINING")
			{
				word2=L"LIKE";
				startingpercent=L"%";
				endingpercent=L"%";
			}
			else if (word2==L"STARTING")
			{
				word2=L"LIKE";
				endingpercent=L"%";
			}
			else if (word2==L"ENDING")
			{
				word2=L"LIKE";
				startingpercent=L"%";
			}
			if (word2==L"LIKE")
			{

				var word2=getword(remainingsortselectclause);
				if (endingpercent)
				{
					word2.swapper(L"'" ^ FM, L"%'" ^ FM);
					word2.splicer(-1,0,L"%");
				}
				if (startingpercent)
				{
					word2.swapper(FM ^ L"'", FM ^ L"'%");
					word2.splicer(2,0,L"%");
				}

				whereclause ^= L" LIKE " ^ word2;

				continue;
			}

            //convert neosys relational operators to standard relational operators
	        var aliasno;
		    if (var(L"EQ,NE,NOT,GT,LT,GE,LE").locateusing(word2,L",",aliasno))
			{
				word2=var(L"=,<>,<>,>,<,>=,<=").field(L",",aliasno);
			}

			//output relational operators and get the value
		    if (var(L"=,<>,>,<,>=,<=").locateusing(word2,L",",aliasno))
			{
				whereclause ^= L" " ^ word2 ^ L" ";
				word2=getword(remainingsortselectclause);
			}
			else
			{
				//if value follows dictionary id without a relational operator then insert =
				if (word2.substr(1,1)==L"'")
					whereclause ^= L" = ";
			}

			if (word2==L"BETWEEN")
			{
				//get and append "from" value
                word2=getword(remainingsortselectclause);
				if (usingnaturalorder)
					word2=naturalorder(word2.tostring());
				whereclause ^= L" BETWEEN " ^ word2;

				//get, check, discard AND
				word2=getword(remainingsortselectclause);
				if (word2 != L"AND")
				{
					//throw MVDBException(L"SELECT STATEMENT SYNTAX IS 'BETWEEN x *AND* y'");
					exodus::errputl(L"SELECT STATEMENT SYNTAX IS 'BETWEEN x *AND* y'");
					return L"";
				}

				whereclause ^= L" AND ";

				//get "to" value
				word2=getword(remainingsortselectclause);

			}

			//TODO how to get sql to understand '' and 0 as nothing without resorting to numerics
			//WITH X becomes WITH X <> ''
			if (!word2.length() && !var(L".0123456789'\"").index(word2.substr(1,1)))
			{
				//put the current word back on the pending
				if (word.length())
					remainingsortselectclause.splicer(1,0,word2 ^ L" ");
				word2=L" <> ''";
			}

			//convert to IN clause if multiple values
            else if (word2.index(FM))
			{

				//prevent  " = IN ( ... )"
				if (whereclause.substr(-3,3)==L" = ")
					whereclause.splicer(-3,3,L"");

				word2=L" IN ( " ^ word2.swap(FM, L", ") ^ L" ) ";
			}

			//append value(s)
			if (usingnaturalorder)
				word2=naturalorder(word2.tostring());
            whereclause ^= word2;

        }
        else
        {
			//todo exclude any ordinary fields included in select?

			//and or ( ) and anything else is copied to the where clause
            whereclause ^= L" " ^ word1;
        }
    }//getword loop

    if (keycodes)
    {
        if (keycodes.count(FM))
        {
            keycodes=L"key IN ( " ^ keycodes.swap(FM,L", ") ^ L" )";
            if (whereclause)
                whereclause=L" AND ( " ^ whereclause ^ L" ) ";
            whereclause=keycodes ^ whereclause;
        }
    }

	//sselect add by key on the end of any specific order bys
	if (bykey)
	{
		if (orderclause)
                    orderclause^=L", ";
		orderclause^=L"key";
	}

	//assemble the full sql select statement
    var sql=L"DECLARE CURSOR1_" ^ (*this) ^ L" CURSOR FOR SELECT " ^ actualfieldnames ^ L" FROM ";
    sql ^= PGDATAFILEPREFIX ^ actualfilename;
	if (joins)
		sql ^= L" " ^ joins.swap(VM,L" ");
	if (whereclause)
		sql ^= L" WHERE " ^ whereclause;
	if (orderclause)
		sql ^= L" ORDER BY " ^ orderclause;
	if (maxnrecs)
		sql ^= L" LIMIT " ^ maxnrecs;

//outputl(sql);

	//Start a transaction block because postgres select requires to be inside one
	if (!begintrans())
		return false;

	if (!sql.sqlexec())
		return false;

	//allow select to be an assignment where filename becomes the cursor name
	//if actual file is in the sortselectclause
	//blank doesnt result in assignment so it can be used as a default "anonymous" cursor
	if (unassigned())
	{
		//(*this)=actualfilename;
		//changed to allow select to be const so allowing file vars to be passed as const ("in") function parameters
		var_mvstr=actualfilename.var_mvstr;
		var_mvtyp=pimpl::MVTYPE_STR;
	}
	return true;
}

void var::clearselect() const
{
	THISIS(L"void var::clearselect() const")
	THISISSTRING()

    var sql=L"CLOSE CURSOR1_";
    if (var_mvtyp)
		sql ^= *this;

	if (!sql.sqlexec())
		return;

	// end the transaction
	committrans();

	return;

}

bool var::readnext(var& key) const
{
	var valueno;
	return readnext(key, valueno);
}

bool readnextx(const std::wstring& cursor, PGresultptr& pgresult)
{
    var sql=L"FETCH NEXT in CURSOR1_" ^ cursor;

	//execute the sql
	//cant use sqlexec here because it returns data
	//sqlexec();

	if (!pqexec(sql,pgresult))
		return false;

	//close cursor if no more
	if (PQntuples(pgresult) < 1)
	{
		PQclear(pgresult);

		// close the cursor - we don't bother to check for errors
		var sql=L"CLOSE CURSOR1_" ^ cursor;

		sql.sqlexec();
		return false;

	}

	return true;

}

bool var::readnext(var& key, var& valueno) const
{
	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_mvtyp&mvtypemask)
	{
		//throw MVUndefined(L"selectx()");
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
	}

	THISIS(L"bool var::readnext(var& key, var& valueno) const")
	THISISSTRING()

	PGconn* thread_pgconn=(PGconn*) connection();
	PGresultptr pgresult;
	if (!readnextx(var_mvstr, pgresult))
	{
		// end the transaction and quit
		committrans();
		return false;
	}
/* abortive code to handle unescaping returned hex/escape data
	//avoid the need for this by calling pqexecparams flagged for binary
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
	//get the key from the first and only column
	//char* data = PQgetvalue(result, 0, 0);
	//int datalen = PQgetlength(result, 0, 0);
	//key=std::string(data,datalen);
	key=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));
//key.output(L"key=").len().outputl(L" len=");
	//TODO implement order by multivalue
	valueno = 0;

	PQclear(pgresult);

	return true;

/*how to access multiple records and fields*/
#if 0
    /* first, print out the attribute names */
    int nFields = PQnfields(result);
    for (i = 0; i < nFields; i++)
        wprintf(L"%-15s", PQfname(result, i));
    wprintf(L"\n\n");

    /* next, print out the rows */
    for (i = 0; i < PQntuples(result); i++)
    {
        for (j = 0; j < nFields; j++)
            wprintf(L"%-15s", PQgetvalue(result, i, j));
        wprintf(L"\n");
    }
#endif

}

bool var::readnextrecord(var& key, var& record) const
{

	//?allow undefined usage like var xyz=xyz.readnext();
	if (var_mvtyp&mvtypemask || !var_mvtyp)
	{
		//throw MVUndefined(L"selectx()");
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
	}

	THISIS(L"bool var::readnextrecord(var& key, var& record) const")
	THISISSTRING()
	ISDEFINED(key)
	ISDEFINED(record)

	connection();
	PGresultptr pgresult;
	if (!readnextx(var_mvstr, pgresult))
	{
		// end the transaction
		committrans();
		return false;
	}

	//key is first column
	//char* data = PQgetvalue(result, 0, 0);
	//int datalen = PQgetlength(result, 0, 0);
	//key=std::string(data,datalen);
	key=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 0), PQgetlength(pgresult, 0, 0));
	
	//record is second column
	//data = PQgetvalue(result, 0, 1);
	//datalen = PQgetlength(result, 0, 1);
	//record=std::string(data,datalen);
	record=wstringfromUTF8((UTF8*)PQgetvalue(pgresult, 0, 1), PQgetlength(pgresult, 0, 1));

    PQclear(pgresult);

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
	var dictexpression=getdictexpression(filename,filename,actualdictfile,actualdictfile,fieldname,joins,true);

	var sql=L"create index index__" ^ filename ^ L"__" ^ fieldname ^ L" on " ^ filename;
	sql^=L" (";
	sql^=dictexpression;
	sql^=L")";

	return sql.sqlexec();

}

bool var::deleteindex(const var& fieldname) const
{
	THISIS(L"bool var::deleteindex(const var& fieldname) const")
	THISISSTRING()
	ISSTRING(fieldname)

	//var filename=*this;
	var sql=L"drop index index__" ^ *this ^ L"__" ^ fieldname;

	return sql.sqlexec();
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

	//execute command or return empty string
	connection();
	PGresultptr result;
	if (!pqexec(sql,result))
		return L"";

	var filenames=L"";
	int nfiles=PQntuples(result);
	for (int filen=0; filen<nfiles; filen++)
	{
		if	(!PQgetisnull(result, filen, 0))
			filenames^= FM ^ wstringfromUTF8((UTF8*)PQgetvalue(result, filen, 0), PQgetlength(result, filen, 0));
	}
	filenames.splicer(1,1,L"");

	PQclear(result);

	return filenames;
}

var var::listindexes(const var& filename) const
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
		sql^=L" relname = '" ^ filename ^  L"' AND ";
	sql^=L" pg_class.oid=pg_index.indrelid"
		 L" AND indisunique != 't'"
		 L" AND indisprimary != 't'"
		 L");";

	//execute command or return empty string
	connection();
	PGresultptr result;
	if (!pqexec(sql,result))
		return L"";

	var tt;
	var indexname;
	var indexnames=L"";
	int nindexes=PQntuples(result);
	for (int indexn=0; indexn<nindexes; indexn++)
	{
		if	(!PQgetisnull(result, indexn, 0))
		{
			indexname=wstringfromUTF8((UTF8*)PQgetvalue(result, indexn, 0), PQgetlength(result, indexn, 0));
			if (indexname.substr(1,6)==L"index_")
			{
				tt=indexname.index(L"__");
				if (tt)
				{
					indexnames^=FM^indexname.substr(7,999999).swap(L"__",VM);
				}
			}
		}
	}
	indexnames.splicer(1,1,L"");

	PQclear(result);

	return indexnames;
}

//used for sql commands that require no parameters
//returns 1 for success and PGresult points to result WHICH MUST BE PQclear(result)'ed
//returns 0 for failure
int pqexec(const var& sql, PGresultptr& pgresult)
{
	int retcode = 1;

	DEBUG_LOG_SQL

	PGconn* thread_pgconn=tss_pgconns.get();
	if (!thread_pgconn)
		return 0;

	//will contain any result IF successful
	//MUST do PQclear(local_result) after using it;
	PGresult *local_result;

	/* dont use PQexec because is cannot be told to return binary results
	 and use PQexecParams with zero parameters instead
	//execute the command
	local_result = PQexec(thread_pgconn, sql.tostring().c_str());
	pgresult = local_result;
	*/

	//no parameters
    const char* paramValues[1];
    int         paramLengths[1];
    int         paramFormats[1];

    //PGresult*
	local_result = PQexecParams(thread_pgconn,
		sql.tostring().c_str(),
		0,       /* zero params */
		NULL,    /* let the backend deduce param type */
		paramValues,
		paramLengths,
		paramFormats,
		1);      /* ask for binary results */
	pgresult=local_result;

	if (!local_result) {
		#if TRACING >=1
			exodus::errputl(L"PQexec command failed, no error code: ");
		#endif
		retcode = 0;
	} else {

		switch (PQresultStatus(local_result)) {
		case PGRES_COMMAND_OK:
			#if TRACING >= 3
				const char *str_res;
				str_res = PQcmdTuples(local_result);
				if (strlen(str_res) > 0) {
					exodus::logputl(L"Command executed OK, " ^ var(str_res) ^ L" rows.");
				} else {
					exodus::logputl(L"Command executed OK, 0 rows.");
				}
			#endif
			break;
		case PGRES_TUPLES_OK:
			#if TRACING >= 3
				exodus::logputl(L"Select executed OK, " ^ var(PQntuples(local_result)) ^ L" rows found.");
			#endif
			break;
		case PGRES_NONFATAL_ERROR:
			#if TRACING >= 2
				exodus::errputl(L"SQL non-fatal error code " ^ var(PQresStatus(PQresultStatus(local_result))) ^ L", " ^ var(PQresultErrorMessage(local_result)));
			#endif
			break;
		default:
			#if TRACING >= 1
			exodus::errputl(var(PQresStatus(PQresultStatus(local_result))) ^ L": " ^ var(PQresultErrorMessage(local_result)));
			#endif
			PQclear(local_result);
			retcode = 0;
			break;
		}

	}

	return retcode;

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
	//syntax is DICT2SQL filename,...Ývolumename

	var filename = filenames.extract(filen);
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

		if (dict.extract(1) not_eq "F")
			continue;

		//skip duplicates
		//if dict<28> then continue

		ndicts += 1;

		//sql^=crlf;

		var ismv = dict.extract(4)[1] == "M";
		if (ismv) {
			anyvar = 1;
			extractvarno = "lineno";
		}else{
			extractvarno = "0";
		}

		if (dict.extract(2) == "0") {
			if (not dict.extract(5)) {
				expression = "key";
			}else{
				//needs pgneosys function not written as yet
				//sql:='text_field(key,"*",':dict<5>:')'
				//postgres only probably
				expression = "split_part(key,\'*\'," ^ dict.extract(5) ^ ")";
			}
		}else{
			//if dict<7> then
			//needs pgneosys functions installed in server
			expression = "exodus_extract_text(data," ^ dict.extract(2) ^ "," ^ extractvarno ^ ",0)";
			//end else
			// sql:='bytea_extract(data,':dict<2>:',':extractvarno:',0)'
			// end
		}

		var conversion = dict.extract(7);
		if (conversion.substr(1, 9) == "[DATETIME") {
			expression.swapper("exodus_extract_text(", "exodus_extract_datetime(");

		}else if (conversion[1] == "D" or conversion.substr(1, 5) == "[DATE") {
			//expression[1,0]="date '1967-12-31' + cast("
			//expression:=' as integer)'
			expression.swapper("exodus_extract_text(", "exodus_extract_date(");

		}else if (conversion == "[NUMBER,0]" or dict.extract(11) == "0N" or (dict.extract(11)).substr(1, 3) == "0N_") {
			expression.splicer(1, 0, "cast(");
			expression ^= " as integer)";
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}else if (conversion.substr(1, 2) == "MD" or conversion.substr(1, 7) == "[NUMBER" or (dict.extract(11)).substr(1, 7) == "[NUMBER" or dict.extract(12) == "FLOAT" or (dict.extract(11)).index("0N", 1)) {
			expression.splicer(1, 0, "cast(");
			expression ^= " as float)";
			//zero length string to be treated as null
			expression.swapper("exodus_extract_text(", "exodus_extract_text2(");

		}else if (conversion.substr(1, 2) == "MT" or conversion.substr(1, 5) == "[TIME") {
			//expression[1,0]="cast("
			//expression:=' as interval)'
			expression.swapper("exodus_extract_text(", "exodus_extract_time(");

		}else if (dict.extract(9) == "R") {
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
