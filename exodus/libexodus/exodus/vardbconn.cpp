//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//variable (that is linked to SQL TABLE name - in field 2 of file handle)
//
//#define INSIDE_MVDBCONNS_CPP
#include <exodus/vardbconn.h>

namespace exodus {

DBConnector::DBConnector(PGCONN_DELETER del)
	// see important NOTE_OSX in header
	: del_(del), dbconn_no_(0), dbconns_() {
}

int DBConnector::add_dbconn(PGconn* conn_to_cache, const std::string conninfo) {
	// no longer need locking since dbconns_ is thread_local
	//std::lock_guard lock(dbconns_mutex);

	dbconn_no_++;
	dbconns_[dbconn_no_] = DBConn(conn_to_cache, conninfo);
	return dbconn_no_;
}

PGconn* DBConnector::get_pgconn(int index) const {

	//std::lock_guard lock(dbconns_mutex); 

	//for (auto pair : dbconns) {
	//	std::clog << pair.first << ". " << (pair.second.pgconn_) <<std::endl;
	//}

	//std::lock_guard lock(dbconns_mutex);
	const auto iter = dbconns_.find(index);
	return reinterpret_cast<PGconn*>(iter == dbconns_.end() ? 0 : iter->second.pgconn_);
}

DBConn* DBConnector::get_dbconn(int index) const {
	//std::lock_guard lock(dbconns_mutex);
	const auto iter = dbconns_.find(index);
	return (DBConn*)(iter == dbconns_.end() ? 0 : &iter->second);
	//return reinterpret_cast<DBConn*>(iter == dbconns_.end() ? 0 : &iter->second);
}

//ConnectionLocks* DBConnector::get_lock_table(int index) const {
//	//std::lock_guard lock(dbconns_mutex);
//	const auto iter = dbconns_.find(index);
//	return (ConnectionLocks*)(iter == dbconns_.end() ? 0 : iter->second.locks__);
//}

DBCache* DBConnector::get_dbcache(int index) const {
	//std::lock_guard lock(dbconns_mutex);
	const auto iter = dbconns_.find(index);
	return (DBCache*)(iter == dbconns_.end() ? 0 : &iter->second.dbcache_);
	//return reinterpret_cast<DBCache*>(iter == dbconns_.end() ? 0 : &iter->second.dbcache_);
}

// pass filename and key by value relying on short string optimisation for performance
std::string DBConnector::getrecord(const int connid, uint64_t file_and_key) const {
	auto dbcache = get_dbcache(connid);
	auto cacheentry = dbcache->find(file_and_key);
	if (cacheentry == dbcache->end())
		return "";

	return dbcache->at(file_and_key);
}

// pass filename and key by value relying on short string optimisation for performance
void DBConnector::putrecord(const int connid, uint64_t file_and_key, const std::string& record) {
	auto dbcache = get_dbcache(connid);
	dbcache->insert_or_assign(file_and_key, record);
	return;
}

// delrecord is currently setting record to "" to counter c++ unordered map reputed performance issues
// pass filename and key by value relying on short string optimisation for performance
void DBConnector::delrecord(const int connid, uint64_t file_and_key) {
	auto dbcache = get_dbcache(connid);
	//(*dbcache)[filenameandkey] = "";
	//dbcache->erase(filenameandkey);
	dbcache->insert_or_assign(file_and_key, "");
	return;
}

void DBConnector::cleardbcache(const int connid) {
	auto dbcache = get_dbcache(connid);
	dbcache->clear();
	return;
}

void DBConnector::del_dbconn(int index) {
	//std::lock_guard lock(dbconns_mutex);

	auto iter = dbconns_.find(index);
	if (iter != dbconns_.end()) {
		//	PGconn* p /*std::pair<int, void*> p*/ = ;
		del_(reinterpret_cast<PGconn*>(iter /*dbconns_.find(index)*/->second.pgconn_));
		//delete /*dbconns_.find(index)*/ iter->second.locks__;
		//delete /*dbconns_.find(index)*/ iter->second.dbcache;
		dbconns_.erase(index);
	}
}

void DBConnector::del_dbconns(int from_index) {
	//std::lock_guard lock(dbconns_mutex);

	auto ix = dbconns_.begin();
	while (ix != dbconns_.end()) {
		if (ix->first >= from_index) {
			//TRACE(ix->first)
			del_(reinterpret_cast<PGconn*>(ix->second.pgconn_));
			//delete ix->second.locks__;
			//delete ix->second.dbcache;
			ix = dbconns_.erase(ix);
		}
		else {
			ix++;
		}
	}
	dbconn_no_ = from_index -= 1;
}

DBConnector::~DBConnector() {
	// std::lock_guard lock(dbconns_mutex);

	auto ix = dbconns_.begin();
	for (;ix != dbconns_.end(); ix++) {
		del_(reinterpret_cast<PGconn*>(ix->second.pgconn_));
		//delete ix->second.locks__;
		//delete ix->second.dbcache;
	}

	// not sure what this comment means .. scoped lock is designed to autounlock
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	// lock.unlock();
}

}  // namespace exodus
