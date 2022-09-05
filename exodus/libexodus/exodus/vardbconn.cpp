//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// NOTE_OSX: Changing order of member construction could cause warning under MacOSX, like:
//	vardbconn.h: In constructor 'exodus::MVConnections::MVConnections(void
//(*)(PGconn*))': vardbconn.h:57: warning: 'exodus::MVConnections::connection_id' will be
// initialized after vardbconn.h:56: warning:   'void (*
// exodus::MVConnections::del)(PGconn*)' mvdbconns.cpp:15: warning:   when initialized here
//
//#define INSIDE_MVDBCONNS_CPP
#include "vardbconn.h"

namespace exodus {

MVConnections::MVConnections(DELETER_AND_DESTROYER del_)
	// see important NOTE_OSX in header
	: del(del_), connection_id(0), conntbl() {
}

int MVConnections::add_connection(PGconn* conn_to_cache, const std::string conninfo) {
	// no longer need locking since mvconnections is thread_local
	//std::lock_guard lock(mvconnections_mutex);

	connection_id++;
	conntbl[connection_id] = MVConnection(conn_to_cache, conninfo);
	return connection_id;
}

PGconn* MVConnections::get_pgconnection(int index) const {

	//std::lock_guard lock(mvconnections_mutex); 

	//for (auto pair : conntbl) {
	//	std::clog << pair.first << ". " << (pair.second.connection) <<std::endl;
	//}

	//std::lock_guard lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (PGconn*)(iter == conntbl.end() ? 0 : iter->second.connection);
}

MVConnection* MVConnections::get_mvconnection(int index) const {
	//std::lock_guard lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (MVConnection*)(iter == conntbl.end() ? 0 : &iter->second);
}

//ConnectionLocks* MVConnections::get_lock_table(int index) const {
//	//std::lock_guard lock(mvconnections_mutex);
//	CONN_MAP::const_iterator iter = conntbl.find(index);
//	return (ConnectionLocks*)(iter == conntbl.end() ? 0 : iter->second.connection_locks);
//}

ConnectionRecordCache* MVConnections::get_recordcache(int index) const {
	//std::lock_guard lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (ConnectionRecordCache*)(iter == conntbl.end() ? 0 : &iter->second.connection_readcache);
}

// pass filename and key by value relying on short string optimisation for performance
std::string MVConnections::getrecord(const int connid, uint64_t file_and_key) const {
	auto connection_readcache = get_recordcache(connid);
	auto cacheentry = connection_readcache->find(file_and_key);
	if (cacheentry == connection_readcache->end())
		return "";

	return connection_readcache->at(file_and_key);
}

// pass filename and key by value relying on short string optimisation for performance
void MVConnections::putrecord(const int connid, uint64_t file_and_key, const std::string& record) {
	auto connection_readcache = get_recordcache(connid);
	connection_readcache->insert_or_assign(file_and_key, record);
	return;
}

// delrecord is currently setting record to "" to counter c++ unordered map reputed performance issues
// pass filename and key by value relying on short string optimisation for performance
void MVConnections::delrecord(const int connid, uint64_t file_and_key) {
	auto connection_readcache = get_recordcache(connid);
	//(*connection_readcache)[filenameandkey] = "";
	//connection_readcache->erase(filenameandkey);
	connection_readcache->insert_or_assign(file_and_key, "");
	return;
}

void MVConnections::clearrecordcache(const int connid) {
	auto connection_readcache = get_recordcache(connid);
	connection_readcache->clear();
	return;
}

void MVConnections::del_connection(int index) {
	//std::lock_guard lock(mvconnections_mutex);
	CONN_MAP::iterator iter = conntbl.find(index);
	if (iter != conntbl.end()) {
		//	PGconn* p /*std::pair<int, void*> p*/ = ;
		del((PGconn*)iter /*conntbl.find(index)*/->second.connection);
		//delete /*conntbl.find(index)*/ iter->second.connection_locks;
		//delete /*conntbl.find(index)*/ iter->second.connection_readcache;
		conntbl.erase(index);
	}
}

void MVConnections::del_connections(int from_index) {
	//std::lock_guard lock(mvconnections_mutex);
	CONN_MAP::iterator ix;
	ix = conntbl.begin();
	while (ix != conntbl.end()) {
		if (ix->first >= from_index) {
			//TRACE(ix->first)
			del((PGconn*)ix->second.connection);
			//delete ix->second.connection_locks;
			//delete ix->second.connection_readcache;
			ix = conntbl.erase(ix);
		}
		else {
			ix++;
		}
	}
	connection_id = from_index -= 1;
}

MVConnections::~MVConnections() {
	// no need
	// std::lock_guard lock(mvconnections_mutex);

	CONN_MAP::iterator ix;
	for (ix = conntbl.begin(); ix != conntbl.end(); ix++) {
		del((PGconn*)ix->second.connection);
		//delete ix->second.connection_locks;
		//delete ix->second.connection_readcache;
	}

	// not sure what this comment means .. scoped lock is designed to autounlock
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	// lock.unlock();
}

}  // namespace exodus
