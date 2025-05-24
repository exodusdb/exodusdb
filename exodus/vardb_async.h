#ifndef VARDB_ASYNC_H
#define VARDB_ASYNC_H

#include "DBresult.h"
#include "DBconn.h" // for DBconn_ptr

namespace exo {

// Forward declare the implementation.
// It is instantiated in the cpp file to avoid including
// all the std headers required for boost fiber and asio
// in vardb.cpp
template<int N>
auto async_PQexec_impl(
	DBconn_ptr conn,
	const char* query,
	int nParams = 0,
	const Oid* paramTypes = nullptr,
	const char* const* paramValues = nullptr,
	const int* paramLengths = nullptr,
	const int* paramFormats = nullptr,
	int resultFormat = 0
) -> DBresult;

// Forward to implementation
// async_PQexec -> async_PQexec_impl
template<int N>
auto async_PQexec(
	DBconn_ptr conn,
	const char* query,
	int nParams = 0,
	const Oid* paramTypes = nullptr,
	const char* const* paramValues = nullptr,
	const int* paramLengths = nullptr,
	const int* paramFormats = nullptr,
	int resultFormat = 0
) -> DBresult {
	static_assert(N == 1 || N == 2 || N == 3, "N must be 1 (PQexecParams), 2 (PQexec), or 3 (PQexecPrepared)");
	return async_PQexec_impl<N>(
		conn,
		query,
		nParams,
		paramTypes,
		paramValues,
		paramLengths,
		paramFormats,
		resultFormat
	);
}

} // namespace exo

#endif // VARDB_ASYNC_H