//
// DBpool.cpp - keep table of connections in the way, that connection is linked to 'filename'
// variable (that is linked to SQL TABLE name - in field 2 of file handle)
//

#if EXO_MODULE > 1
	import std;
#else
#	include <string>
#	include <cstdint>
#endif

#include "DBpool.h"

constexpr int TRACING = 0;

namespace exo {

DBpool::DBpool()
	: dbconn_no_(0), dbconns_() {
}

int DBpool::add_dbconn(PGconn* pgconn, const std::string conninfo) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	++dbconn_no_;
	dbconns_[dbconn_no_] = DBconn(pgconn, conninfo, dbconn_no_);
#pragma GCC diagnostic pop

	if (TRACING >= 3) {
		// TODO avoid casual usage of var while building var to speed compilation time
//		const var password = var(conninfo).match(R"__(password\s*=\s*\w*)__");
//		var(conninfo).replace(var(conninfo).match(R"__(password\s*=\s*\w*)__", "").errputl("DBpool::add_dbconn: " ^ var(dbconn_no_) ^ " conninfo: ");
	}

	return dbconn_no_;
}

// NOTE: We are actually returning something that can be implicit converted to a PGconn
DBconn_ptr DBpool::get_pgconn(const int index) const {
	//TimeAcc t(100);
	auto iter = dbconns_.find(index);
	DBconn_ptr dbconn_ptr;
	if (iter != dbconns_.end())
		dbconn_ptr.dbconn = &iter->second;
	return dbconn_ptr;
}

DBconn* DBpool::get_dbconn(const int index) const {
	//TimeAcc t(103);
	auto iter = dbconns_.find(index);
	return static_cast<DBconn*>(iter == dbconns_.end() ? nullptr : &iter->second);
}

DBcache* DBpool::get_dbcache(const int index) const{
	//TimeAcc t(104);
	auto iter = dbconns_.find(index);
	return static_cast<DBcache*>(iter == dbconns_.end() ? nullptr : &iter->second.dbcache_);
}

bool DBpool::getrecord(const int connid, const std::uint64_t hash64, std::string& record) const {
	//TimeAcc t(105);
	auto dbcache = get_dbcache(connid);
	auto cacheentry = dbcache->find(hash64);
	if (cacheentry == dbcache->end()) {
		return false;
	} else {
		record = cacheentry->second;
		return true;
	}
}

void DBpool::putrecord(const int connid, const std::uint64_t hash64, const std::string& record) {
	//TimeAcc t(106);
	auto dbcache = get_dbcache(connid);
	dbcache->insert_or_assign(hash64, record);
	return;
}

bool DBpool::delrecord(const int connid, const std::uint64_t hash64) {
	//TimeAcc t(106);
	auto dbcache = get_dbcache(connid);
	return dbcache->erase(hash64);
}

void DBpool::clearcache(const int connid) {
	//TimeAcc t(107);
	auto dbcache = get_dbcache(connid);

	dbcache->clear();
	return;
}

void DBpool::del_dbconn(const int index) {
	//TimeAcc t(108);
	auto iter = dbconns_.find(index);
	if (iter != dbconns_.end()) {

		// Redundant since DBconn dtor calls PQfinish
		if (iter->second.pgconn_) {
			PQfinish(iter->second.pgconn_);
			iter->second.pgconn_ = nullptr;
		}

		dbconns_.erase(index);
	}
}

void DBpool::del_dbconns(const int from_index) {
	//TimeAcc t(109);
	auto ix = dbconns_.begin();
	while (ix != dbconns_.end()) {
		if (ix->first >= from_index) {

			// Redundant since DBconn dtor calls PQfinish
			if (ix->second.pgconn_) {
				PQfinish(ix->second.pgconn_);
				ix->second.pgconn_ = nullptr;
			}

			ix = dbconns_.erase(ix);
		}
		else {
			ix++;
		}
	}
	dbconn_no_ = from_index - 1;
}

DBpool::~DBpool() {
	//TimeAcc t(110);

	// Redundant since DBconn dtor calls PQfinish
	auto ix = dbconns_.begin();
	for (;ix != dbconns_.end(); ix++) {
		if (ix->second.pgconn_) {
			PQfinish(ix->second.pgconn_);
			ix->second.pgconn_ = nullptr;
		}
	}

}

int DBpool::max_dbconn_no() {
	return dbconn_no_;
}

}  // namespace exo
