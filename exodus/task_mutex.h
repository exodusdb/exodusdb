#ifndef FIBER_MUTEX_HPP
#define FIBER_MUTEX_HPP

#include <memory> // For std::unique_ptr

namespace exo {

// No Boost headers included here to avoid slow compilation times.
class FiberMutex {
public:
	FiberMutex();
	~FiberMutex();

	// Mutexes are non-copyable.
	// Delete copy and assign ctors.
	FiberMutex(const FiberMutex&) = delete;
	FiberMutex& operator=(const FiberMutex&) = delete;

	// Move operations are allowed since we are only a ptr
	// Unique_ptr provides the machinery.
	FiberMutex(FiberMutex&&) noexcept = default;
	FiberMutex& operator=(FiberMutex&&) noexcept = default;

	// Allow usage like an actual mutex.
	void lock();
	bool try_lock();
	void unlock();

private:
	class Impl;
	std::unique_ptr<Impl> pimpl_;
};

} // namespace exo

#endif // FIBER_MUTEX_HPP