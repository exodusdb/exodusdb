#ifndef VARDBCONN_H_
#define VARDBCONN_H_

#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#	include <cstdint>
#endif

// For postgresql's PGconn connection handle type
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
//#include <libpq-fe.h>
#include "libpq-fe.h"
#pragma clang diagnostic push

#include "DBconn.h"

namespace exo {

// "final" to avoid declaring the destructor as virtual as a precaution
class DBpool final {

	// Data members
	///////////////

	// Number of db connections
	int dbconn_no_;

	// Container
	mutable DBconns dbconns_;

   public:

	// ctors/dtors
	//////////////

	explicit DBpool();

	//class marked as final so no need for virtual
	//virtual ~DBpool();
	~DBpool();

	// Manipulators
	///////////////

	int add_dbconn(PGconn* connection_with_file, const std::string conninfo);

	// Singular version deletes one
	void del_dbconn(const int dbconn_no);

	// Plural version deletes many
	void del_dbconns(const int from_dbconn_no);

	// Observers
	////////////

	int max_dbconn_no();

	// We are actually passing something that can be implicit converted to a PGconn
	auto get_pgconn(const int dbconn_no) const -> DBconn_ptr;

	auto get_dbconn(const int dbconn_no) const -> DBconn*;
	auto get_dbcache(const int dbconn_no) const -> DBcache*;

	bool getrecord(const int dbconn_no, const std::uint64_t hash64, std::string& record) const;
	void putrecord(const int dbconn_no, const std::uint64_t hash64, const std::string& record);
	bool delrecord(const int dbconn_no, const std::uint64_t hash64);
	void clearcache(const int dbconn_no);

};

}  // namespace exo

#endif	//VARDBCONN_H_
