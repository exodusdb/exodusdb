
//required for postgres.h on windows .cpp
#ifdef _MSC_VER
#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

//to avoid the following errors in postgres.h below
//error C2011: 'timezone' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	188	
//error C2011: 'itimerval' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	197	
#ifndef WIN32
#define WIN32
#endif

//to avoid the following warnings that should be removed in postgres 9.1 or 9.2
//c:\program files (x86)\postgresql\9.0\include\server\pg_config_os.h(106): warning C4005: 'EIDRM' : macro redefinition
//          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\errno.h(103) : see previous definition of 'EIDRM'
#pragma warning (disable: 4005)

#endif

#include <string>

//for Datum, VARSIZE, VARDATA etc
#undef errcode
#include "postgres.h"

/*backward compatible to pre 8.3 with no SET_VARSIZE*/
#ifndef SET_VARSIZE
#define SET_VARSIZE(ret,size) VARATT_SIZEP((ret) ) = (size)
#endif

//defined in naturalorder.cpp
namespace exodus{
std::string naturalorder(const std::string&);
}

#ifdef __cplusplus
extern "C" {
#endif

//defined in pgexodus.c because palloc wont work in .cpp
void* pallocproxy(size_t sz1);

Datum
exodus_natural_order(char * instring, int inlength)
{
	std::string natural_order_string=exodus::naturalorder(std::string(instring,inlength));
	size_t resultlen=natural_order_string.length();

	//prepare a new output
	Datum output = (Datum) pallocproxy(VARHDRSZ+resultlen);

	//set the complete size of the output
	SET_VARSIZE(output,VARHDRSZ+resultlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (natural_order_string.data()),	// starting from
		   resultlen);						// how many bytes

	return output;

}

#ifdef __cplusplus
}
#endif

