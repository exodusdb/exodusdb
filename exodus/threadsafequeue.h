#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#if EXO_MODULE
	import std;
#else
#	include <queue>
#	include <mutex>
#	include <condition_variable>
#	include <type_traits>
#endif

namespace exo {

template <typename T>
class ThreadSafeQueue {
public:
//	void push(const T& value);   // Push a value into the queue

    template <typename U = T, typename = std::enable_if_t<std::is_copy_constructible_v<U>>>
	void push(const U& value);

//    template <typename U = T, typename = std::enable_if_t<std::is_copy_constructible_v<U>>>
//	void push(const U& value) {
//	    std::lock_guard<std::mutex> lock(mutex_);
////	    LOG << "Pushing value to queue, size before: " << queue_.size() << std::endl;
//	    queue_.push(value);
////	    LOG << "Pushed value, size after: " << queue_.size() << ", notifying one" << std::endl;
//	    cond_var_.notify_one();
//	}

	bool empty();                // Check if empty
	bool pop(T& value);          // Try to pop a value; returns false if queue is empty
	bool try_pop(T& value);      // Try to pop a value; returns false if queue is empty
	bool wait_and_pop(T& value); // Wait until a value is available and pop it

    void push(T&& value); // Add for move-only ExoEnv
    T pop();            // Move element out of queue

private:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;
};

} // namespace exo

#endif // THREAD_SAFE_QUEUE_HPP
