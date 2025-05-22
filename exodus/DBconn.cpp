#ifdef EXO_MODULE
	import std;
#else
#	include <string>
#	include <utility> // for move
#endif

#include "DBconn.h"

#include <libpq-fe.h> // for PGconn and PQfinish

namespace exo {

// Default constructor
DBconn::DBconn() = default;

// Constructor with PGconn* and conninfo
DBconn::DBconn(PGconn* pgconn, std::string conninfo)
	: pgconn_(pgconn), conninfo_(std::move(conninfo)) {}

// Destructor for RAII cleanup
DBconn::~DBconn() {
	if (pgconn_) {
		PQfinish(pgconn_);
	}
}

// Move constructor
DBconn::DBconn(DBconn&& other) noexcept
	: pgconn_(other.pgconn_),
	  locks_(std::move(other.locks_)),
	  dbcache_(std::move(other.dbcache_)),
	  conninfo_(std::move(other.conninfo_)),
	  in_transaction_(other.in_transaction_) {
	other.pgconn_ = nullptr; // Prevent double-free
}

// Move assignment operator
DBconn& DBconn::operator=(DBconn&& other) noexcept {
	if (this != &other) {
		// Clean up current resource
		if (pgconn_) {
			PQfinish(pgconn_);
		}
		// Move members
		pgconn_ = other.pgconn_;
		locks_ = std::move(other.locks_);
		dbcache_ = std::move(other.dbcache_);
		conninfo_ = std::move(other.conninfo_);
		in_transaction_ = other.in_transaction_;
		// Nullify other's pgconn to prevent double-free
		other.pgconn_ = nullptr;
	}
	return *this;
}

} // namespace exo