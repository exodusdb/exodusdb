/*
severely hacked to get around problem building in VS2005
http://www.mail-archive.com/pgsql-general@postgresql.org/msg116145.html
remove w64 compatibility compilation option
and change
static const Pg_magic_struct
to
static Pg_magic_struct
*/

/*

* PostgreSQL example C functions.
*
* This file must be built as a shared library or dll and
* placed into the PostgreSQL `lib' directory. On Windows
* it must link to postgres.lib .
*
* postgresql/include/server must be on your header search path.
* With MSVC++ on win32 so must postgresql/include/server/port/win32_msvc .
* With MinGW use postgresql/include/server/port/win32 .
*/

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifndef _USE_32BIT_TIME_T
#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif
//to avoid the following warnings that should be removed in postgres 9.1 or 9.2
//c:\program files (x86)\postgresql\9.0\include\server\pg_config_os.h(106): warning C4005: 'EIDRM' : macro redefinition
//          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\errno.h(103) : see previous definition of 'EIDRM'
#pragma warning (disable: 4005)
#endif

//to avoid the following errors in postgres.h below
//error C2011: 'timezone' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	188	
//error C2011: 'itimerval' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	197	
#ifndef WIN32
#define WIN32
#endif
#endif

/* BUILDING_DLL causes the declarations in Pg's headers to be declared
* __declspec(dllexport) which will break DLL linkage. */
#ifdef BUILDING_DLL
#error Do not define BUILDING_DLL when building extension libraries
#endif

/* Ensure that Pg_module_function and friends are declared __declspec(dllexport) */
#ifndef BUILDING_MODULE
#define BUILDING_MODULE
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

#include "postgres.h"
#include "fmgr.h"
/*#include "utils/geo_decls.h"*/
#include <utils/timestamp.h>
int4 pg_atoi(char*,int4,int4);

/*backward compatible to pre 8.3 with no SET_VARSIZE*/
#ifndef SET_VARSIZE
#define SET_VARSIZE(ret,size) VARATT_SIZEP((ret) ) = (size)
#endif

#ifdef _MSC_VER

/*--------------- BEGIN REDEFINITION OF PG MACROS -------------------
*
* These rewritten versions of PG_MODULE_MAGIC and PG_FUNCTION_INFO_V1
* declare the module functions as __declspec(dllexport) when building
* a module. They also provide PGMODULEEXPORT for exporting functions
* in user DLLs.
*/
#undef PG_MODULE_MAGIC
#undef PG_FUNCTION_INFO_V1

/* This might want to go somewhere other than fmgr.h, like
* pg_config_os.h alongside the definition of PGDLLIMPORT
*/
#if defined(_MSC_VER) || defined(__MINGW32__)
#if defined(BUILDING_MODULE)
#define PGMODULEEXPORT __declspec (dllexport)
#else
// Never actually used
#define PGMODULEEXPORT __declspec (dllimport)
#endif
#else
#define PGMODULEEXPORT
#endif

#define PG_MODULE_MAGIC \
PGMODULEEXPORT Pg_magic_struct * \
PG_MAGIC_FUNCTION_NAME(void) \
{ \
static Pg_magic_struct Pg_magic_data = PG_MODULE_MAGIC_DATA; \
return &Pg_magic_data; \
} \
extern int no_such_variable

#define PG_FUNCTION_INFO_V1(funcname) \
PGMODULEEXPORT const Pg_finfo_record * \
CppConcat(pg_finfo_,funcname) (void) \
{ \
static const Pg_finfo_record my_finfo = { 1 }; \
return &my_finfo; \
} \
extern int no_such_variable
#endif
/*--------------- END REDEFINITION OF PG MACROS -------------------*/


//TODO check all pallocs for success

//see http://www.postgresql.org/docs/8.3/interactive/xfunc-c.html

//text_extract2 is like text_extract but returns NULL for empty strings

/* POSTGRES INTERFACE
	PG_ARGISNULL(0) tells you if passed a null
	PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	VARDATA() gives you a pointer to the data region of a struct.
	VARSIZE() gives you the total size of the structure
	VARHDRSZ
	//elog(WARNING, "sizeof picktime8: %d",sizeof(picktime8));
	elog(ERROR, "mvExtractDate cannot convert more than 20 characters to an integer");
	PG_RETURN_NULL();
*/

/* DEBUG USING SOMETHING LIKE THIS
		elog(ERROR, "Debug point xxx");
		//elog(WARNING, "Debug point xxx");
		elog(DEBUG, "Debug point xxx");
		PG_RETURN_NULL();
*/

/* POSTGRES SQL FUNCTION DEFINITION CREATION COPY AND PASTE

NB public functions need to be listed in the .def file on windows.

"STRICT" KEYWORD MEANS THAT THE FUNCTION WILL NOT BE CALLED IF ANY OF THE ARGUMENTS ARE NULL
Note that select statements from exodus are (currently) generated with coalesce function
 to turn nulls into "" in order to retrieve non-existent join records.

LOAD 'pgexodus' without the .dll or .so to load or reload a shared library file NB single quotes
but the above will not allow simple unload or updating the DLL without stopping and restarting postgres on win32

-- All functions are defined as IMMUTABLE although exodus_call could theorectically return anything.
-- First three are not STRICT and return "" in case passed a null

-- cut and paste the following SQL to register the functions into postgres --

CREATE OR REPLACE FUNCTION exodus_call(bytea, bytea, bytea, bytea, bytea, int4, int4) RETURNS bytea
AS 'pgexodus', 'exodus_call' LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION exodus_extract_bytea(bytea, int4, int4, int4)
RETURNS bytea AS 'pgexodus', 'exodus_extract_bytea' LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION exodus_extract_text(bytea, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_text' LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION exodus_extract_sort(bytea, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_sort' LANGUAGE C IMMUTABLE;

-- Remaining functions are STRICT therefore never get called with NULLS
-- also return NULL if passed zero length strings

CREATE OR REPLACE FUNCTION exodus_extract_text2(bytea, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_text2' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_date(bytea, int4, int4, int4) RETURNS date
AS 'pgexodus', 'exodus_extract_date' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_time(bytea, int4, int4, int4) RETURNS time
AS 'pgexodus', 'exodus_extract_time' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_datetime(bytea, int4, int4, int4) RETURNS timestamp
AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;

TO REMOVE THE ABOVE DO THE FOLLOWING AS POSTGRES SUPERUSER CONNECTED TO EXODUS DATABASE

drop FUNCTION exodus_call(bytea, bytea, bytea, bytea, bytea, int4, int4) cascade;
drop FUNCTION exodus_extract_bytea(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_text(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_sort(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_text2(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_date(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_time(bytea, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_datetime(bytea, int4, int4, int4) cascade;

*/

/* evade the following warnings from GETINPUTSATRTLENGTH macro
  but it isnt clear why the warning doesnt come in all places that the macro is called
pgexodus.c: In function 'exodus_extract_sort': (GETINPUTSTARTLENGTH)
pgexodus.c:679: warning: 'input' may be used uninitialized in this function
pgexodus.c: In function 'exodus_extract_text': (GETINPUTSTARTLENGTH)
pgexodus.c:462: warning: 'input' may be used uninitialized in this function
*/

#define GETINPUTSTARTLENGTH\
	text *input;\
	int32 outstart;\
	int32 outlen;\
	int32 fieldno;\
	int32 valueno;\
	int32 subvalueno;\
	if (PG_ARGISNULL(0))\
	{\
		outstart=0;\
		outlen=0;\
		input=0; /* evade warning: 'input' may be used unitialized */\
	}\
	else\
	{\
		/*get a pointer to the first parameter (0)*/\
		input = PG_GETARG_TEXT_P(0);\
		fieldno = PG_GETARG_INT32(1);\
		valueno = PG_GETARG_INT32(2);\
		subvalueno = PG_GETARG_INT32(3);\
		extract(VARDATA(input), VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);\
	}\

//extern "C" {

//bool callexodus(const char* serverid, const char* request, const int nrequestbytes, const char* response, int& nresponsebytes);
bool callexodus(const char* serverid, const char* request, const int nrequestbytes, const char* response, int* nresponsebytes);

void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);

//#include <postgres.h>
//#include <fmgr.h>
//#include <utils/timestamp.h>
//int4 pg_atoi(char*,int4,int4);

/*
To ensure that a dynamically loaded object file is not loaded into an incompatible server, PostgreSQL checks that the file contains a "magic block" with the appropriate contents. This allows the server to detect obvious incompatibilities, such as code compiled for a different major version of PostgreSQL. A magic block is required as of PostgreSQL 8.2. To include a magic block, write this in one (and only one) of the module source files, after having included the header fmgr.h:
*/
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(exodus_call);

Datum
exodus_call(PG_FUNCTION_ARGS)
{

#define MAXREPLYBYTES 1048576

	//get a pointer to the first parameter (0)
	bytea *serverid = PG_GETARG_BYTEA_P(0);
	bytea *tablename = PG_GETARG_BYTEA_P(1);
	bytea *dictkey = PG_GETARG_BYTEA_P(2);
	bytea *datakey = PG_GETARG_BYTEA_P(3);
	bytea *data = PG_GETARG_BYTEA_P(4);
	int32 valueno = PG_GETARG_INT32(5);
	int32 subvalueno = PG_GETARG_INT32(6);

	char* prequest;
	char* appendpoint;
	int32** lengthpoint;
	char serverid2[4096];
	char* presponse;
	int32 nresponsebytes;
	bytea* output;

	//calculate length of pipe data
	//total length
	int32 nrequestbytes=sizeof(int32);
	//length and tablename
	nrequestbytes+=sizeof(int32)+VARSIZE(tablename)-VARHDRSZ;
	//length and dictkey
	nrequestbytes+=sizeof(int32)+VARSIZE(dictkey)-VARHDRSZ;
	//length and datakey
	nrequestbytes+=sizeof(int32)+VARSIZE(datakey)-VARHDRSZ;
	//length and data
	nrequestbytes+=sizeof(int32)+VARSIZE(data)-VARHDRSZ;
	//valueno and subvalueno
	nrequestbytes+=sizeof(int32)*2;

	//acquire space for pipedata
	prequest=(char*) palloc(nrequestbytes);
	//TODO: check if successful
//	delete prequest;

	//start appending at beginning
	appendpoint=prequest;

	//pointer to enable saving of an integer (will track appendpoint)
	lengthpoint=(int32**)&appendpoint;

	//length of whole thing
	**lengthpoint=nrequestbytes;
	appendpoint+=sizeof(int32);
//?	*lengthpoint++;

	//length of tablename
	// *appendpoint=(int32)(VARSIZE(tablename)-VARHDRSZ);
	**lengthpoint=(int32)(VARSIZE(tablename)-VARHDRSZ);
	appendpoint+=sizeof(int32);

	//tablename
	memcpy((void *) (appendpoint)	// destination
		   ,(void *) VARDATA(tablename)	// starting from
		   ,VARSIZE(tablename)-VARHDRSZ	// how many bytes
		   );
	appendpoint+=VARSIZE(tablename)-VARHDRSZ;

	//length of dictkey
	// *appendpoint=(int32)(VARSIZE(dictkey)-VARHDRSZ);
	**lengthpoint=(int32)(VARSIZE(dictkey)-VARHDRSZ);
	appendpoint+=sizeof(int32);

	//dictkey
	memcpy((void *) (appendpoint)	// destination
		   ,(void *) VARDATA(dictkey)	// starting from
		   ,VARSIZE(dictkey)-VARHDRSZ	// how many bytes
		   );
	appendpoint+=VARSIZE(dictkey)-VARHDRSZ;

	//length of datakey
	// *appendpoint=(int32)(VARSIZE(datakey)-VARHDRSZ);
	**lengthpoint=(int32)(VARSIZE(datakey)-VARHDRSZ);
	appendpoint+=sizeof(int32);

	//datakey
	memcpy((void *) (appendpoint)	// destination
			,(void *) VARDATA(datakey)	// starting from
			,VARSIZE(datakey)-VARHDRSZ	// how many bytes
			);
	appendpoint+=VARSIZE(datakey)-VARHDRSZ;

	//length of data
	// *appendpoint=(int32)(VARSIZE(data)-VARHDRSZ);
	**lengthpoint=(int32)(VARSIZE(data)-VARHDRSZ);
	appendpoint+=sizeof(int32);

	//data
	memcpy((void *) (appendpoint)	// destination
		   ,(void *) VARDATA(data)	// starting from
		   ,VARSIZE(data)-VARHDRSZ	// how many bytes
		   );
	appendpoint+=VARSIZE(data)-VARHDRSZ;

	//value number
	// *appendpoint=(int32)(valueno);
	**lengthpoint=(int32)(valueno);
	appendpoint+=sizeof(int32);

	//subvalue number
	// *appendpoint=(int32)(subvalueno);
	**lengthpoint=(int32)(subvalueno);
	appendpoint+=sizeof(int32);

	//failsafe check is correct total length
	if (appendpoint-prequest-nrequestbytes)
	{
		pfree(prequest);
		elog(ERROR, "pgexodus exodus_call: Incorrect request structure.");
		PG_RETURN_NULL();
	}

	//convert serverid to cstr
	memset(serverid2,0,4096);
	memcpy((void *) (serverid2)	// destination
		   ,(void *) VARDATA(serverid)	// starting from
		   ,VARSIZE(serverid)-VARHDRSZ	// how many bytes
		   );

	//allocate space for reply
	presponse = (char*) palloc(MAXREPLYBYTES);

	////elog(WARNING, "exodus_call: callexodus()");
	if (!callexodus(serverid2, prequest, nrequestbytes, presponse, &nresponsebytes))
	{
		//elog(ERROR, "pgexodus exodus_call: Failed with %s ", presponse);
		elog(ERROR, "pgexodus exodus_call failed. (%s)", presponse);
		pfree(prequest);
		pfree(presponse);
		PG_RETURN_NULL();
	}

	//extract(VARDATA(input), VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);

	//elog(WARNING, "exodus_call: palloc'ing");
	//prepare a new output
	output = (bytea*) palloc(VARHDRSZ+nresponsebytes+4);

	//set the complete size of the output
	//elog(WARNING, "exodus_call: initialising palloc'ed structure");
	SET_VARSIZE(output,VARHDRSZ+nresponsebytes);

	//copy the input to the output
	//elog(WARNING, "exodus_call: copying response to palloc'ed");
	memcpy((void *) VARDATA(output),	// destination
//		(void *) (int)(presponse),	// starting from
		(void *) (presponse),		// starting from
		nresponsebytes);		// how many bytes


	//elog(WARNING, "exodus_call: freeing temp data");
	pfree(prequest);
	pfree(presponse);

	//elog(WARNING, "exodus_call: returning response");
	PG_RETURN_BYTEA_P(output);

};

PG_FUNCTION_INFO_V1(exodus_extract_bytea);

Datum
exodus_extract_bytea(PG_FUNCTION_ARGS)
{


	/*very similar to GETINPUTSTARTLENGTH macro but bytea instead of text*/

	int32 outstart;
	int32 outlen;
	int32 fieldno;
	int32 valueno;
	int32 subvalueno;

	bytea* input;
	bytea* output;

	//get a pointer to the first parameter (0)
	input = PG_GETARG_BYTEA_P(0);

	fieldno = PG_GETARG_INT32(1);

	valueno = PG_GETARG_INT32(2);

	subvalueno = PG_GETARG_INT32(3);

	extract(VARDATA(input), VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);


	//prepare a new output
	//bytea	   *output = (bytea *) palloc(VARSIZE(input));
	output = (bytea *) palloc(VARHDRSZ+outlen);

	//set the complete size of the output
	SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes

	PG_RETURN_BYTEA_P(output);

}

PG_FUNCTION_INFO_V1(exodus_extract_text);

Datum
exodus_extract_text(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	bytea* output;

	GETINPUTSTARTLENGTH
/*
	if (PG_ARGISNULL(0))
	{
		outstart=0;
		outlen=0;
	}
	else
	{
		//get a pointer to the first parameter (0)
		input = PG_GETARG_TEXT_P(0);
		fieldno = PG_GETARG_INT32(1);
		valueno = PG_GETARG_INT32(2);
		subvalueno = PG_GETARG_INT32(3);
		extract(VARDATA(input), VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);
	}
*/
//PG_RETURN_NULL();
	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (text *) palloc(VARHDRSZ+outlen);

	//set the complete size of the output
	//SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes

	PG_RETURN_TEXT_P(output);

}

PG_FUNCTION_INFO_V1(exodus_extract_text2);

Datum
exodus_extract_text2(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	text *output;
	GETINPUTSTARTLENGTH

	//return NULL for zero length string
	if (outlen==0) PG_RETURN_NULL();

	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (text *) palloc(VARHDRSZ+outlen);

	//set the complete size of the output
	SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes

	PG_RETURN_TEXT_P(output);

}

PG_FUNCTION_INFO_V1(exodus_extract_date);

Datum
exodus_extract_date(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	int4 pickdate;

	char intstr[21]="12345";

	GETINPUTSTARTLENGTH

	//intstr="12345";
	intstr[20]='\0';

	//return NULL for zero length string
	if (outlen==0) PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		elog(ERROR, "mvExtractDate cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}

	memcpy(intstr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes
	intstr[outlen]='\0';

	//convert the c str to an int
	pickdate=outlen;
	pickdate=pg_atoi(intstr,4,'.');

	//pick date 0 is 31/12/1967
	//pg date 0 is 31/12/1999
	PG_RETURN_INT32(pickdate-11689);

}

PG_FUNCTION_INFO_V1(exodus_extract_time);

Datum
exodus_extract_time(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	char intstr[21];
	int4 picktime;
	Interval *output;

	GETINPUTSTARTLENGTH

	//return NULL for zero length string
	if (outlen==0) PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		elog(ERROR, "exodus_extract_time cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	intstr[20]='\0';
	memcpy(intstr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes
	intstr[outlen]='\0';

	//convert the c str to an int
	picktime=pg_atoi(intstr,4,'.');

    #define SIZEOFINTERVAL 12
	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (Interval *) palloc(VARHDRSZ+SIZEOFINTERVAL);

	output->month=0;
	output->day=0;
	output->time=picktime;
#	ifdef HAVE_INT64_TIMESTAMP
		output->time=output->time*1000000;
#	endif

	PG_RETURN_INTERVAL_P(output);
}

PG_FUNCTION_INFO_V1(exodus_extract_datetime);

Datum
exodus_extract_datetime(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	Timestamp pickdatetime;
	char datetimestr[21];

	GETINPUTSTARTLENGTH

	//return NULL for zero length string
	if (outlen==0) PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		elog(ERROR, "mvExtractTime cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	datetimestr[20]='\0';
	memcpy(datetimestr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   outlen);						// how many bytes
	datetimestr[outlen]='\0';

	//convert the c str to an double

#ifdef HAVE_INT64_TIMESTAMP
    //number of microseconds before or after midnight 1/1/2000?
	pickdatetime=(long long int)(atof(datetimestr)-11689)*86400000000LL;
#else
    //number of seconds before or after midnight 1/1/2000
	pickdatetime=(atof(datetimestr)-11689)*86400;
#endif

	PG_RETURN_TIMESTAMP(pickdatetime);
}

//defined in naturalorder.cpp
Datum
exodus_natural_order(char * instring, int inlength);

PG_FUNCTION_INFO_V1(exodus_extract_sort);

//avoid warning "Unused declaration of .." in above macro
int no_such_variable;

Datum
exodus_extract_sort(PG_FUNCTION_ARGS)
{
	GETINPUTSTARTLENGTH


/*
	if (!outlen)
		elog(WARNING, "outlen0");
	if (!outstart)
		elog(WARNING, "outstart0");
	PG_RETURN_NULL();

	//return NULL for zero length string
	if (outstart==0||outlen==0)
		PG_RETURN_NULL();
*/
	return exodus_natural_order(VARDATA(input)+outstart,outlen);
}

/*

PG_FUNCTION_INFO_V1(exodus_extract_sort);

Datum
exodus_extract_sort(PG_FUNCTION_ARGS)
{

	//PG_GETARG_BYTEA_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	bytea* output;
	int nextrachars;
	char* inputiter;
	char* inputlast;

	char* outputiter;

	int ndigits;
	int ninserted;
	char digit2;

	GETINPUTSTARTLENGTH

	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	//assume two extra character anyway
	nextrachars=0;

allocandcopy:

	++nextrachars;
	++nextrachars;

	output = (text *) palloc(VARHDRSZ+outlen+nextrachars);

	//set the complete size of the output
	//SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+outlen+nextrachars);

	//get a pointer to the last character of the output buffer
	outputiter=(VARDATA(output)+outlen+nextrachars);

	//copy the input to the output
	//memcpy((void *) VARDATA(output),			// destination
	//	   (void *) (VARDATA(input)+outstart),	// starting from
	//	   outlen);						// how many bytes

	//walk backwards looking for whole numbers (before the decimal point)
	//and prefix nn before them depending on the number of digits.

	//copy the input to the output
	//memcpy((void *) VARDATA(output),			// destination
	//	   (void *) (VARDATA(input)+outstart),	// starting from
	//	   outlen);						// how many bytes

	inputiter=VARDATA(input)+outstart+outlen;
	//one byte before the first input character
	inputlast=VARDATA(input)+outstart;

	ndigits=0;
	ninserted=0;
	while (inputiter>=inputlast) {

		//continuous series of digits are counted for length
		if (*inputiter<='9' && *inputiter>='0')
			++ndigits;

		//if we get to a dot then any preceeding digits are to the right of decimal point so they dont need justification
		else if (*inputiter=='.')
			ndigits=0;

		//if we get to a non-digit after a series of digits then insert nn
		//where nn is the ascii designation of the number of digits (max 99 is represented as 99)
		else

		//duplicated IN and AFTER loop (except ndigits not zeroed after loop)
		if (ndigits)
		{

			//if already ninserted then we have consumed the spare two and cannot continue without new memory
			++ninserted;
			++ninserted;
			if (ninserted>nextrachars)
			{
				pfree(output);
				//need goto because we are need to restart the loop outside the current loop
				goto allocandcopy;
			}

			//insert nn into the output
			if (ndigits>99)
				ndigits=99;
			digit2=ndigits%10;
			ndigits-=digit2;
			*outputiter--='0'+digit2;
			*outputiter--='0'+ndigits/10;
			ndigits=0;
		}

		*outputiter--=*inputiter--;

	}//of loop to copy character input to output

	//output ALWAYS has two extra prefix characters even if no insertions. this method of assuming
	//one insertion allows us to avoid a reallocation on the first insertion and allows
	//a single pass copying the input buffer to the output in the
	//most frequent cases of 1) simple number or 2) non-numeric
	//yet still sort correctly when there are no insertions

	//duplicated IN and AFTER loop (except ndigits not zeroed after loop)
	if (ndigits)
	{

		//if already ninserted then we have consumed the spare two and cannot continue without new memory
		++ninserted;
		++ninserted;
		if (ninserted>nextrachars)
		{
			pfree(output);
			//need goto because we are need to restart the loop outside the current loop
			goto allocandcopy;
		}

		//insert nn into the output
		if (ndigits>99)
			ndigits=99;
		digit2=ndigits%10;
		ndigits-=digit2;
		*outputiter--='0'+digit2;
		*outputiter--='0'+ndigits/10;
//			ndigits=0;
	}

	//we can control where the non-numeric insertions appear relative numbers by
	//inserting something other than spaces at the front. spaces cause non-numeric to sort BEFORE all numbers
	//99 would sort after some ascii punc and before other
	//      (space)!"#$%&'()*+,-./    0123456789      :;<=>?@ABCDEFGHIJKLMNO
	if (ninserted<nextrachars) {
		*outputiter--=' ';
		*outputiter=' ';
	}

	PG_RETURN_TEXT_P(output);

}
*/

//used in naturalorder because palloc doesnt link in cpp
void* pallocproxy(size_t sz1)
{
	return palloc(sz1);
}

#ifdef __cplusplus
}//of extern "C"
#endif
