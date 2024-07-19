//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//variable (that is linked to SQL TABLE name - in field 2 of file handle)
//
//#define INSIDE_MVDBCONNS_CPP
#include <exodus/rex.h>
#include <exodus/vardbconn.h>

constexpr int TRACING = 0;

namespace exo {

DBConn::DBConn(PGconn* pgconn, std::string conninfo)
	: pgconn_(pgconn), conninfo_(conninfo) {
}

DBConnector::DBConnector(PGCONN_DELETER del)
	// see important NOTE_OSX in header
	: del_(del), dbconn_no_(0), dbconns_() {
}

int DBConnector::add_dbconn(PGconn* conn_to_cache, const std::string conninfo) {
	// no longer need locking since dbconns_ is thread_local
	//std::lock_guard lock(dbconns_mutex);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	++dbconn_no_;
	dbconns_[dbconn_no_] = DBConn(conn_to_cache, conninfo);
#pragma GCC diagnostic pop

	if (TRACING >= 3) {
		var(conninfo).replacer(R"(password\s*=\s*\w*)"_rex, "").errputl("DBConnector::add_dbconn: " ^ var(dbconn_no_) ^ " conninfo: ");
	}

	return dbconn_no_;
}

PGconn* DBConnector::get_pgconn(const int index) const {
	//TimeAcc t(100);
	//std::lock_guard lock(dbconns_mutex);

	//for (auto pair : dbconns) {
	//	std::clog << pair.first << ". " << (pair.second.pgconn_) <<std::endl;
	//}

	//std::lock_guard lock(dbconns_mutex);
	auto iter = dbconns_.find(index);
	//return (PGconn*)(iter == dbconns_.end() ? nullptr : iter->second.pgconn_);
	//return reinterpret_cast<PGconn*>(iter == dbconns_.end() ? nullptr : iter->second.pgconn_);
	return static_cast<PGconn*>(iter == dbconns_.end() ? nullptr : iter->second.pgconn_);
}

DBConn* DBConnector::get_dbconn(const int index) const {
	//TimeAcc t(103);
	//std::lock_guard lock(dbconns_mutex);
	auto iter = dbconns_.find(index);
	return static_cast<DBConn*>(iter == dbconns_.end() ? nullptr : &iter->second);
	//return reinterpret_cast<DBConn*>(iter == dbconns_.end() ? nullptr : &iter->second);
}

//ConnectionLocks* DBConnector::get_lock_table(int index) const {
//	//std::lock_guard lock(dbconns_mutex);
//	const auto iter = dbconns_.find(index);
//	return (ConnectionLocks*)(iter == dbconns_.end() ? nullptr : iter->second.locks__);
//}

DBCache* DBConnector::get_dbcache(const int index) const{
	//TimeAcc t(104);
	//std::lock_guard lock(dbconns_mutex);
	auto iter = dbconns_.find(index);
	return static_cast<DBCache*>(iter == dbconns_.end() ? nullptr : &iter->second.dbcache_);
	//return reinterpret_cast<DBCache*>(iter == dbconns_.end() ? nullptr : &iter->second.dbcache_);
}

// pass filename and key by value relying on short string optimisation for performance
bool DBConnector::getrecord(const int connid, const std::uint64_t hash64, std::string& record) const {
	//TimeAcc t(105);
	auto dbcache = get_dbcache(connid);
	auto cacheentry = dbcache->find(hash64);
	if (cacheentry == dbcache->end()) {
		return false;
	} else {
		//record = dbcache->at(hash64);
		record = cacheentry->second;
		//TRACE(record)
		return true;
	}
}

// pass filename and key by value relying on short string optimisation for performance
void DBConnector::putrecord(const int connid, const std::uint64_t hash64, const std::string& record) {
	//TimeAcc t(106);
	auto dbcache = get_dbcache(connid);
	dbcache->insert_or_assign(hash64, record);
	//TRACE("added " + record)
	return;
}

// delrecord is currently setting record to "" to counter c++ unordered map reputed performance issues
// pass filename and key by value relying on short string optimisation for performance
bool DBConnector::delrecord(const int connid, const std::uint64_t hash64) {
	//TimeAcc t(106);
	auto dbcache = get_dbcache(connid);
	//Must remove since empty entries indicate no present and could be writing a record
	//dbcache->insert_or_assign(hash64, "");
	//(*dbcache)[filenameandkey] = "";
	return dbcache->erase(hash64);
}

void DBConnector::cleardbcache(const int connid) {
	//TimeAcc t(107);
	auto dbcache = get_dbcache(connid);

//	for (const auto& iter : *dbcache)
//		var(iter.first % 1'000'000'000).errputl("Clearing cache of ");

	//if (dbcache->size())
	//	var(dbcache->size()).errputl("cleardbcache: dbcache.size was ");

	dbcache->clear();
	return;
}

void DBConnector::del_dbconn(const int index) {
	//std::lock_guard lock(dbconns_mutex);
	//TimeAcc t(108);
	auto iter = dbconns_.find(index);
	if (iter != dbconns_.end()) {
		//	PGconn* p /*std::pair<int, void*> p*/ = ;

//		if (iter->second.dbcache_.size())
//			var(iter->second.dbcache_.size()).logputl("del_dbconn: dbcache size was ");

		del_(reinterpret_cast<PGconn*>(iter /*dbconns_.find(index)*/->second.pgconn_));
		//delete /*dbconns_.find(index)*/ iter->second.locks__;
		//delete /*dbconns_.find(index)*/ iter->second.dbcache;
		dbconns_.erase(index);
	}
}

void DBConnector::del_dbconns(const int from_index) {
	//std::lock_guard lock(dbconns_mutex);
	//TimeAcc t(109);
	auto ix = dbconns_.begin();
	while (ix != dbconns_.end()) {
		if (ix->first >= from_index) {
			//TRACE(ix->first)

			if (ix->second.dbcache_.size())
				var(ix->second.dbcache_.size()).logputl("del_dbconns: []dbcache size was ");

			del_(reinterpret_cast<PGconn*>(ix->second.pgconn_));
			//delete ix->second.locks__;
			//delete ix->second.dbcache;
			ix = dbconns_.erase(ix);
		}
		else {
			ix++;
		}
	}
	//dbconn_no_ = from_index -= 1;
	dbconn_no_ = from_index - 1;
}

DBConnector::~DBConnector() {
	// std::lock_guard lock(dbconns_mutex);
	//TimeAcc t(110);

	//if (dbconns_.size())
	//	var(dbconns_.size()).errputl("~DBConnector: dbconns_.size was ");

	auto ix = dbconns_.begin();
	for (;ix != dbconns_.end(); ix++) {

		//if (ix->second.dbcache_.size())
		//	var(ix->second.dbcache_.size()).errputl("~DBConnector: []dbcache_.size was ");

		del_(reinterpret_cast<PGconn*>(ix->second.pgconn_));
		//delete ix->second.locks__;
		//delete ix->second.dbcache;
	}

	// not sure what this comment means .. scoped lock is designed to autounlock
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	// lock.unlock();
}

int DBConnector::max_dbconn_no() {
	return dbconn_no_;
}

}  // namespace exo
