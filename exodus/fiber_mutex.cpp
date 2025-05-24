#include "fiber_mutex.h"
#include <boost/fiber/mutex.hpp>

namespace exo {

class FiberMutex::Impl {
public:
	boost::fibers::mutex mutex_;
};

FiberMutex::FiberMutex() : pimpl_(std::make_unique<Impl>()) {}
FiberMutex::~FiberMutex() {}

void FiberMutex::lock() { pimpl_->mutex_.lock(); }
bool FiberMutex::try_lock() { return pimpl_->mutex_.try_lock(); }
void FiberMutex::unlock() { pimpl_->mutex_.unlock(); }

} // namespace exo