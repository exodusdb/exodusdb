//
// Exodus -
//
// mvdbconn.cpp - keep table of connections in the way, that connection is linked to 'filename'
//		variable (that is linked to SQL TABLE name - in mvint field)
//
// NOTE_OSX: Changing order of member construction could cause warning under MacOSX, like:
//	mvdbconns.h: In constructor 'exodus::MvConnectionsCache::MvConnectionsCache(void (*)(PGconn*))':
//	mvdbconns.h:57: warning: 'exodus::MvConnectionsCache::connection_id' will be initialized after
//	mvdbconns.h:56: warning:   'void (* exodus::MvConnectionsCache::del)(PGconn*)'
//	mvdbconns.cpp:15: warning:   when initialized here
//
#include "mv.h"
//#define INSIDE_MVDBCONNS_CPP
#include "mvdbconns.h"

namespace exodus {

MvConnectionsCache::MvConnectionsCache(DELETER_AND_DESTROYER del_)
	// see important NOTE_OSX in header
	: del(del_)
	, connection_id(0)
	, mvconnections_mutex()
	, conntbl()
{}

int MvConnectionsCache::add_connection(CACHED_CONNECTION conn_to_cache)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);

	connection_id++;
	conntbl[connection_id] = MvConnectionEntry(conn_to_cache, new LockTable, new RecordCache);
	return connection_id;
}

CACHED_CONNECTION MvConnectionsCache::get_connection(int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (CACHED_CONNECTION)(iter == conntbl.end() ? 0 : iter->second.connection);
}

UNORDERED_SET_FOR_LOCKTABLE * MvConnectionsCache::get_lock_table(int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (UNORDERED_SET_FOR_LOCKTABLE *)(iter == conntbl.end() ? 0 : iter->second.plock_table);
}

RecordCache* MvConnectionsCache::get_recordcache(int index) const
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::const_iterator iter = conntbl.find(index);
	return (RecordCache *)(iter == conntbl.end() ? 0 : iter->second.precordcache);
}

std::wstring MvConnectionsCache::readrecord(const int connid, const std::wstring filename, const std::wstring key) const
{
	auto precordcache=get_recordcache(connid);
	std::wstring filenameandkey=filename + L"|" + key;
	auto cacheentry=precordcache->find(filenameandkey);
	if (cacheentry==precordcache->end())
		return L"";

	return precordcache->at(filenameandkey);
}

void MvConnectionsCache::writerecord(const int connid, const std::wstring filename, const std::wstring key, const std::wstring record)
{
	auto precordcache=get_recordcache(connid);
	std::wstring filenameandkey=filename + L"|" + key;
	(*precordcache)[filenameandkey]=record;
	return;
}

void MvConnectionsCache::clearrecordcache(const int connid)
{
	auto precordcache=get_recordcache(connid);
	precordcache->clear();
	return;
}

void MvConnectionsCache::del_connection(int index)
{
	boost::mutex::scoped_lock lock(mvconnections_mutex);
	CONN_MAP::iterator iter = conntbl.find(index);
	if (iter != conntbl.end())
	{
//	CACHED_CONNECTION p /*std::pair<int, void*> p*/ = ;
		del((CACHED_CONNECTION) iter/*conntbl.find(index)*/->second.connection);
		delete /*conntbl.find(index)*/iter->second.plock_table;
		conntbl.erase(index);
	}
}

MvConnectionsCache::~MvConnectionsCache()
{
	//no need
	//boost::mutex::scoped_lock lock(mvconnections_mutex);

	CONN_MAP::iterator ix;
	for(ix = conntbl.begin(); ix != conntbl.end(); ix ++)
	{
//		del((CACHED_CONNECTION) ix->second.connection);
		delete ix->second.plock_table;
		delete ix->second.precordcache;
	}

	//not sure what this comment means .. scoped lock is designed to autounlock
	// Release mutex, on closing brace it will be destroyed, and MUST be unlocked on ~mutex()
	//lock.unlock();
}

}	// namespace

