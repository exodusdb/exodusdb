// threadpool.h
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

// Evade bug in std module
///root/exodus/exodus/thread_pool.cpp:15:20: error: invalid operands to binary expression ('iterator' (aka '__normal_iterator<std::thread *, std::vector<std::thread, std::allocator<std::t
//hread>>>') and 'iterator')
//   15 |		 for (auto& worker : workers_) {
//	  |						   ^
///usr/lib/gcc/x86_64-linux-gnu/99/../../../../include/c++/99/bits/allocator.h:216:7: note: candidate function not viable: no known conversion from 'iterator' (aka '__normal_iterator<std:
//:thread *, std::vector<std::thread, std::allocator<std::thread>>>') to 'const allocator<thread>' for 1st argument
//  216 |	   operator==(const allocator&, const allocator&) _GLIBCXX_NOTHROW
//	  |	   ^		  ~~~~~~~~~~~~~~~~
// <plus of similar note:s

#if EXO_MODULE
	import std;
#else
#	include <functional> // std::function
//#	include <queue>
//#	include <mutex>
//#	include <condition_variable>
//#	include <thread>
#	include <vector>
//#	include <atomic>
#endif

// Forward declarations
namespace std {
	class mutex;
	class condition_variable;
	class thread;
	template<typename T> class atomic;
}

namespace exo {

// Opaque type for task queue (defined in .cpp)
struct TaskQueue;

// ThreadPool: A thread pool for executing tasks using a dynamic number of worker threads.
// Workers are threads that execute tasks from a queue. The pool supports increasing or
// decreasing the number of live workers and stopping all workers.
class ThreadPool {
public:
	// Constructs a thread pool with an initial maximum number of workers.
	// @param max_threads The initial number of workers to allow.
	ThreadPool(std::size_t max_threads);

	// Destroys the thread pool, stopping all workers and joining their threads.
	~ThreadPool();

	// Enqueues a task for execution by a worker.
	// Creates a new worker if the number of running tasks plus queued tasks exceeds the
	// number of live workers and capacity exists.
	// Throws std::runtime_error if the pool is stopped.
	// @param task The task (function) to execute.
	void enqueue(std::function<void()> task);

	std::size_t get_total_tasks_enqueued() const;
	std::size_t decrement_total_tasks_enqueued() const;

	// Sets the desired number of live workers, increasing or decreasing the pool.
	// Workers with IDs >= max_worker_id exit when idle to reduce the pool size.
	// Setting max_threads to 0 stops all workers.
	// @param max_threads The desired number of live workers.
	auto set_max_threads(std::size_t max_threads) -> void;
	auto get_max_threads() -> std::size_t;
	auto get_num_cores()   -> std::size_t;

	// Blocks until all workers exit, processing all pending tasks
	void shutdown();

	// Reset in place
	static void reset(ThreadPool* ptr, std::size_t num_threads);

private:

	std::unique_ptr<std::atomic<std::size_t>> total_tasks_enqueued;

	// Maximum worker ID threshold. Workers with worker_id >= max_worker_id exit when idle.
	// Adjusted by set_max_threads to control the number of live workers.
	std::unique_ptr<std::atomic<std::size_t>> max_worker_id_;

	// Count of live workers (running, either executing tasks or idle).
	// Incremented when a worker is created, decremented when a worker exits.
	// Note: live_worker_count is NOT the same as workers.size(). live_worker_count
	// tracks currently running workers, while workers.size() is the total number of worker
	// threads ever created, including dead (exited) ones that remain in workers until
	// destruction.
	std::unique_ptr<std::atomic<std::size_t>> live_worker_count;

	// Count of workers currently executing tasks (running tasks).
	// Incremented before a task is executed, decremented after.
	std::unique_ptr<std::atomic<std::size_t>> running_task_count;

	// Queue of tasks awaiting execution by workers.
	std::unique_ptr<TaskQueue> pending_tasks;

	// Mutex for synchronizing access to pending_tasks, worker creation, and condition variable.
	std::unique_ptr<std::mutex> mutex;

	// Condition variable for notifying workers of new tasks or changes to max_worker_id.
	std::unique_ptr<std::condition_variable> condition;

	// Collection of worker threads (live and dead), each executing tasks from the queue.
	// Size increases when new workers are created but does not decrease until destruction.
	std::vector<std::thread> workers;
};

} // namespace exo

#endif // THREAD_POOL_H_
