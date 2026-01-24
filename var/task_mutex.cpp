#include <boost/fiber/mutex.hpp>
#include "task_mutex.h"

namespace exo {

class TaskMutex::Impl {
public:
	boost::fibers::mutex mutex_;
};

TaskMutex::TaskMutex() : pimpl_(std::make_unique<Impl>()) {}
TaskMutex::~TaskMutex() {}

void TaskMutex::lock() { pimpl_->mutex_.lock(); }
bool TaskMutex::try_lock() { return pimpl_->mutex_.try_lock(); }
void TaskMutex::unlock() { pimpl_->mutex_.unlock(); }

} // namespace exo