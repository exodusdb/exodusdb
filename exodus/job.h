// job.h
#ifndef EXO_JOB_H
#define EXO_JOB_H

#ifdef EXO_MODULE
	import std;
#else
//	#include <future>
	#include <memory>
namespace std {
	template<typename> class future; // Forward declaration of std::future
	enum class future_status; // Forward declaration of std::future_status
	class condition_variable; // Forward declaration of std::condition_variable
	class mutex; // Forward declaration of std::mutex
}
#endif

enum class TimeUnit { Milliseconds, Seconds };

class TimePoint; // Opaque type for wait_until

#include <exodus/exoenv.h>
#include <exodus/threadsafequeue.h>

namespace exo {

class PUBLIC Job {
public:
	std::future<ExoEnv> future;
	std::shared_ptr<ThreadSafeQueue<var>> input_queue;
	std::shared_ptr<ThreadSafeQueue<var>> output_queue;

	// ctor
	Job(std::future<ExoEnv>&& fut,
		std::shared_ptr<ThreadSafeQueue<var>> in_q,
		std::shared_ptr<ThreadSafeQueue<var>> out_q);

	// noexcept move ctor
	Job(Job&& other) noexcept;
	Job& operator=(Job&& other) noexcept;
	// No copy ctor
	Job(const Job&) = delete;
	Job& operator=(const Job&) = delete;
	// Default dtor
	~Job() = default;

    // New constructor to replicate ExoProgram::run
	Job(
        var command,
        std::shared_ptr<ThreadSafeQueue<var>> input_queue = nullptr,
        std::shared_ptr<ThreadSafeQueue<var>> output_queue = nullptr,
        std::shared_ptr<ThreadSafeQueue<ExoEnv>> result_queue = nullptr
    );

	// Forwarded std::future<ExoEnv> non-special member functions
	ExoEnv get();
	bool valid() const noexcept;
	void wait() const;

	// wait_for() overloads for common durations
	bool wait_for(int duration) const; // Default: milliseconds
	auto wait_for(int duration, TimeUnit unit) const ->	std::future_status;

	// wait_until with opaque TimePoint
//	auto wait_until(const TimePoint& abs_time) const -> std::future_status;

//	// Wait with internal condition variable, using a predicate
//	template<typename Predicate>
//	auto wait_with_condition(Predicate pred) const -> std::future_status;
//
	// Access condition variable for lambda to notify
	auto get_condition_variable() const -> std::condition_variable&;

private:
//	mutable std::mutex mutex; // Mutable for const wait_with_condition
	mutable std::condition_variable cv; // Mutable for const get_condition_variable
};

} // namespace exo

#endif // EXO_JOB_H
