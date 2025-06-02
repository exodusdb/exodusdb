#ifndef TASK_MUTEX_HPP
#define TASK_MUTEX_HPP

#include <memory> // For std::unique_ptr

namespace exo {

// No Boost headers included here to avoid slow compilation times.
class TaskMutex {
public:
	TaskMutex();
	~TaskMutex();

	// Mutexes are non-copyable.
	// Delete copy and assign ctors.
	TaskMutex(const TaskMutex&) = delete;
	TaskMutex& operator=(const TaskMutex&) = delete;

	// Move operations are allowed since we are only a ptr
	// Unique_ptr provides the machinery.
	TaskMutex(TaskMutex&&) noexcept = default;
	TaskMutex& operator=(TaskMutex&&) noexcept = default;

	// Allow usage like an actual mutex.
	void lock();
	bool try_lock();
	void unlock();

private:
	class Impl;
	std::unique_ptr<Impl> pimpl_;
};

} // namespace exo

#endif // TASK_MUTEX_HPP