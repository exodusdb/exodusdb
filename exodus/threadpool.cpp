// threadpool.cpp
#if EXO_MODULE// > 1
#	include <vector> // here to solve bug in building module?
	import std;
#else
#	include <vector>
#	include <functional>
#	include <queue>
#	include <mutex>
#	include <condition_variable>
#	include <thread>
#	include <atomic>
#	include <memory>

// Logging macro with relative timestamp from program start
//#define ENABLE_LOGGING
#	include <iostream> // unlike clang, g++ wants this to compile std::cerr in logging despite if (0)
#	ifdef ENABLE_LOGGING
#		include <iomanip>
#		include <chrono>
#		include <thread>
#	endif
#endif

#include "threadpool.h"

#ifdef ENABLE_LOGGING
//	// Static variable to capture initial time (initialized once at program start)
static const auto start_time = std::chrono::system_clock::now();
#	define LOG std::cerr << "[" << std::fixed << std::setprecision(3) \
					  << double(std::chrono::duration_cast<std::chrono::microseconds>( \
						  std::chrono::system_clock::now().time_since_epoch()).count() - \
						  std::chrono::duration_cast<std::chrono::microseconds>( \
						  start_time.time_since_epoch()).count()) / 1000.0 \
					  << "ms, T" << std::this_thread::get_id() << "] P "
//					  << "ms, T" << std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000000 << "] "
#else
#	define LOG if (0) std::cerr // No-op, optimized out
#endif

namespace exo {

// Define the task queue type
struct TaskQueue {
	std::queue<std::function<void()>> queue;
};

// Initialize the thread pool with a maximum number of workers
ThreadPool::ThreadPool(size_t max_threads)
	:
	total_tasks_enqueued(std::make_unique<std::atomic<size_t>>(0)),
	max_worker_id_(std::make_unique<std::atomic<size_t>>(max_threads)),
	live_worker_count(std::make_unique<std::atomic<size_t>>(0)),
	running_task_count(std::make_unique<std::atomic<size_t>>(0)),
	pending_tasks(std::make_unique<TaskQueue>()),
	mutex(std::make_unique<std::mutex>()),
	condition(std::make_unique<std::condition_variable>()) {}

// Destroy the thread pool, stopping all workers and joining their threads
ThreadPool::~ThreadPool() {
	{
		LOG << "~Threadpool: Acquiring mutex." << std::endl;
		std::unique_lock<std::mutex> lock(*mutex);

		while (!pending_tasks->queue.empty()) {
			LOG << "~Threadpool: Popping pending task." << std::endl;
			pending_tasks->queue.pop();
		}
		LOG << "~Threadpool: Signal shutdown." << std::endl;
		(*max_worker_id_).store(0);
		LOG << "~Threadpool: Wake all workers to check exit condition" << std::endl;
		condition->notify_all();
	}
	// Join all worker threads
	for (auto& worker : workers) {
		if (worker.joinable()) {
			LOG << "~Threadpool: Detaching worker." << std::endl;
			// compile with many threads usually errors out with 
			//malloc_consolidate(): unaligned fastbin chunk detected
			//./compall: line 52: 1107964 Segmentation fault      (core dumped) compile alt exo srv dic {S${COMPILER_OPTIONS}}
			//make: *** [Makefile:2: comp] Error 139
//			worker.detach();
			worker.join();
			LOG << "~Threadpool: Detached  worker." << std::endl;
		}
	}
}

// Enqueue a task for execution by a worker
void ThreadPool::enqueue(std::function<void()> task) {
	{
		std::unique_lock<std::mutex> lock(*mutex);
		// Prevent enqueuing tasks when the pool is stopped
		if ((*max_worker_id_).load() == 0 && (*live_worker_count).load() == 0) {
			throw std::runtime_error("Enqueue on stopped ThreadPool");
		}
		pending_tasks->queue.push(std::move(task));

		(*total_tasks_enqueued).fetch_add(1); // Increment total tasks enqueued

		// Create a new worker if running tasks plus queued tasks exceed live workers and capacity exists
		if ((*running_task_count).load() + pending_tasks->queue.size() > (*live_worker_count).load() && (*live_worker_count).load() < (*max_worker_id_).load()) {
			size_t worker_id = workers.size() + 1; // Assign worker_id based on current size (Start at 1 so setting max to 0 can stop it)
			(*live_worker_count).fetch_add(1); // Increment count of live workers
			workers.emplace_back([this, worker_id] {
				while (true) {
					std::function<void()> task;
					{
						LOG << "Worker " << worker_id << ": Acquiring mutex." << std::endl;
						std::unique_lock<std::mutex> lock(*mutex);
						LOG << "Worker " << worker_id << ": Entering wait." << std::endl;
						condition->wait(lock, [this, worker_id] {
							LOG << "Worker " << worker_id << ": Evaluating predicate (queue_empty=" \
									  << pending_tasks->queue.empty() << ", max_worker_id=" \
									  << (*max_worker_id_).load() << ", worker_id=" << worker_id << ")." << std::endl;
							return !pending_tasks->queue.empty() || (*max_worker_id_).load() == 0 || worker_id >= (*max_worker_id_).load();
						});
						LOG << "Worker " << worker_id << ": Woke from wait." << std::endl;
						LOG << "Worker " << worker_id << ": Checking exit condition (queue_empty=" \
								  << pending_tasks->queue.empty() << ", max_worker_id=" \
								  << (*max_worker_id_).load() << ", worker_id=" << worker_id << ")." << std::endl;
						if (pending_tasks->queue.empty() && ((*max_worker_id_).load() == 0 || worker_id >= (*max_worker_id_).load())) {
							LOG << "Worker " << worker_id << ": Exiting." << std::endl;
							(*live_worker_count).fetch_sub(1);
							return;
						}
						LOG << "Worker " << worker_id << ": Accessing queue front." << std::endl;
						task = std::move(pending_tasks->queue.front());
						LOG << "Worker " << worker_id << ": Popping queue." << std::endl;
						pending_tasks->queue.pop();
						LOG << "Worker " << worker_id << ": Task acquired." << std::endl;
					}
					LOG << "Worker " << worker_id << ": Incrementing running_task_count." << std::endl;
					(*running_task_count).fetch_add(1);
					LOG << "Worker " << worker_id << ": Executing task." << std::endl;
					task();
					LOG << "Worker " << worker_id << ": Task completed." << std::endl;
					LOG << "Worker " << worker_id << ": Decrementing running_task_count." << std::endl;
					(*running_task_count).fetch_sub(1);
				}
			});

		}
	}
	condition->notify_one(); // Notify one idle worker to process the task
}

// New method to get the total number of tasks enqueued
size_t ThreadPool::get_total_tasks_enqueued() const {
	return (*total_tasks_enqueued).load();
}

// Decrement e.g. when consuming result queue.
size_t ThreadPool::decrement_total_tasks_enqueued() const {
size_t current = total_tasks_enqueued->load();
while (current > 0) {
    if (total_tasks_enqueued->compare_exchange_strong(current, current - 1)) {
        break; // Successfully decremented
    }
    // Optional: Add a pause or yield to reduce contention
//    std::this_thread::yield(); // Helps in high-contention scenarios
}
	return current;
}

// Set the desired number of live workers
void ThreadPool::set_max_threads(size_t max_threads) {
	{
		std::unique_lock<std::mutex> lock(*mutex);
		size_t live_count = (*live_worker_count).load(); // Current count of live workers
		size_t new_max_worker_id;
		if (max_threads == 0) {
			new_max_worker_id = 0; // Stop all workers
		} else {
			// Adjust max_worker_id to achieve the desired number of workers
			// Formula: max_worker_id = workers.size() + (max_threads - live_count)
			new_max_worker_id = workers.size() + (max_threads - live_count);
		}
		(*max_worker_id_).store(new_max_worker_id);
		condition->notify_all(); // Notify all workers to check for excess or new capacity
	}
}
auto ThreadPool::get_max_threads() -> std::size_t {
	return max_worker_id_->load();
}

auto ThreadPool::get_num_cores() -> std::size_t {
	unsigned int num_cores = std::thread::hardware_concurrency();
	return num_cores;
}

void ThreadPool::shutdown() {
	{
		std::unique_lock<std::mutex> lock(*mutex);
		(*max_worker_id_).store(0);
		LOG << "ThreadPool shutdown: Setting max_worker_id=0." << std::endl;
		condition->notify_all();
	}
	for (auto& worker : workers) {
		if (worker.joinable()) {
			LOG << "ThreadPool shutdown: Joining worker." << std::endl;
			worker.join();
			LOG << "ThreadPool shutdown: Joined worker." << std::endl;
		}
	}
}
// Static function to reset a ThreadPool object in place
void ThreadPool::reset(ThreadPool* ptr, size_t num_threads) {
	if (!ptr) {
//		std::cerr << "Invalid ThreadPool pointer\n";
		return;
	}

	LOG << "ThreadPool resetting.\n";
	// Ensure the thread pool is shut down cleanly
	ptr->shutdown();

	ptr->~ThreadPool(); // Call destructor to clean up

	// Reconstruct in place with placement new
	new (ptr) ThreadPool(num_threads); // Initialize new thread pool

	LOG << "ThreadPool reset complete\n";
}

} // namespace exo
