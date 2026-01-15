#ifndef EXO_TASK_MANAGER_IMPL_H
#define EXO_TASK_MANAGER_IMPL_H

#if EXO_MODULE > 1
	import std;
//	import var;
#else
#	include <vector>
#	include <mutex>
#endif

#include <boost/fiber/all.hpp>

namespace exo {

struct TaskManager_Impl {
	std::vector<boost::fibers::fiber> fibers;
	boost::fibers::buffered_channel<std::shared_ptr<AsyncResult>> result_channel;
	std::mutex mutex;
	TaskManager_Impl() : result_channel(128) {}
};

} // namespace exo

#endif // EXO_TASK_MANAGER_IMPL_H
