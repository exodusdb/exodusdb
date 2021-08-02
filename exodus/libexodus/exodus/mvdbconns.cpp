//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// NOTE_OSX: Changing order of member construction could cause warning under MacOSX, like:
//	mvdbconns.h: In constructor 'exodus::MVConnections::MVConnections(void
//(*)(PGconn*))': 	mvdbconns.h:57: warning: 'exodus::MVConnections::connection_id' will be
// initialized after 	mvdbconns.h:56: warning:   'void (*
// exodus::MVConnections::del)(PGconn*)' 	mvdbconns.cpp:15: warning:   when initialized here
//
#include "mv.h"
//#define INSIDE_MVDBCONNS_CPP
#include "mvdbconns.h"

namespace exodus {

MVConnections::MVConnections(DELETER_AND_DESTROYER del_)
	// see important NOTE_OSX in header
	: del(del_), connection_id(0), conntbl(), mvconnections_mutex() {
}

int MVConnections::add_connection(CACHED_CONNECTION conn_to_cache) {
	boost::mutex::scoped_lock lock(mvconnections_mutex);

	connection_id++;
	conntbl[connection_id] = MVConnection(conn_to_cache, new LockTable, new RecordCache);
	return connection_id;
}

CACHED_CONNECTION MVConnections::get_connection(int index) const {
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (CACHED_CONNECTION)(iter == conntbl.end() ? 0 : iter->second.connection);
}

LockTable* MVConnections::get_lock_table(int index) const {
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (LockTable*)(iter == conntbl.end() ? 0 : iter->second.plock_table);
}

RecordCache* MVConnections::get_recordcache(int index) const {
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (RecordCache*)(iter == conntbl.end() ? 0 : iter->second.precordcache);
}

// pass filename and key by value relying on short string optimisation for performance
std::string MVConnections::getrecord(const int connid, const std::string filename, const std::string key) const {
	auto precordcache = get_recordcache(connid);
	std::string filenameandkey = filename + "|" + key;
	auto cacheentry = precordcache->find(filenameandkey);
	if (cacheentry == precordcache->end())
		return "";

	return precordcache->at(filenameandkey);
}

// pass filename and key by value relying on short string optimisation for performance
void MVConnections::putrecord(const int connid, const std::string filename, const std::string key, const std::string& record) {
	auto precordcache = get_recordcache(connid);
	std::string filenameandkey = filename + "|" + key;
	//(*precordcache)[filenameandkey] = record;
	precordcache->insert_or_assign(filenameandkey, record);
	return;
}

// delrecord is currently setting record to "" to counter c++ unordered map reputed performance issues
// pass filename and key by value relying on short string optimisation for performance
void MVConnections::delrecord(const int connid, const std::string filename, const std::string key) {
	auto precordcache = get_recordcache(connid);
	std::string filenameandkey = filename + "|" + key;
	//(*precordcache)[filenameandkey] = "";
	//precordcache->erase(filenameandkey);
	precordcache->insert_or_assign(filenameandkey, "");
	return;
}

void MVConnections::clearrecordcache(const int connid) {
	auto precordcache = get_recordcache(connid);
	precordcache->clear();
	return;
}

void MVConnections::del_connection(int index) {
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator iter = conntbl.find(index);
	if (iter != conntbl.end()) {
		//	CACHED_CONNECTION p /*std::pair<int, void*> p*/ = ;
		del((CACHED_CONNECTION)iter /*conntbl.find(index)*/->second.connection);
		delete /*conntbl.find(index)*/ iter->second.plock_table;
		delete /*conntbl.find(index)*/ iter->second.precordcache;
		conntbl.erase(index);
	}
}

void MVConnections::del_connections(int from_index) {
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator ix;
	ix = conntbl.begin();
	while (ix != conntbl.end()) {
		if (ix->first >= from_index) {
			//TRACE(ix->first)
			del((CACHED_CONNECTION)ix->second.connection);
			delete ix->second.plock_table;
			delete ix->second.precordcache;
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
	// boost::mutex::scoped_lock lock(mvconnections_mutex);

	CONN_MAP::iterator ix;
	for (ix = conntbl.begin(); ix != conntbl.end(); ix++) {
		del((CACHED_CONNECTION)ix->second.connection);
		delete ix->second.plock_table;
		delete ix->second.precordcache;
	}

	// not sure what this comment means .. scoped lock is designed to autounlock
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	// lock.unlock();
}

}  // namespace exodus
