// job.h
#ifndef EXO_JOB_H
#define EXO_JOB_H

#if EXO_MODULE > 1
	import std;
#else
	#include <future>
//	#include <memory>
namespace std {
	template<typename> class future; // Forward declaration of std::future
	enum class future_status; // Forward declaration of std::future_status
//	class condition_variable; // Forward declaration of std::condition_variable
//	class mutex; // Forward declaration of std::mutex
}
#endif

enum class TimeUnit { Milliseconds, Seconds };

class TimePoint; // Opaque type for wait_until

#include <exodus/exoenv.h>
#include <exodus/threadsafequeue.h>

namespace exo {

class PUBLIC Job {
public:
	// async result
	std::future<ExoEnv> future;

	// async i/o
	std::shared_ptr<ThreadSafeQueue<var>> input_queue;
	std::shared_ptr<ThreadSafeQueue<var>> output_queue;

public:
	// ctors for move/move assign
	Job(Job&& other) noexcept;
	Job& operator=(Job&& other) noexcept;

	// Suppress copy and assign ctors
	Job(const Job&) = delete;
	Job& operator=(const Job&) = delete;

	// Default dtor
	~Job() = default;

	// ctor 1. for async result and i/o
	Job(std::future<ExoEnv>&& fut,
		std::shared_ptr<ThreadSafeQueue<var>> in_q,
		std::shared_ptr<ThreadSafeQueue<var>> out_q);

	// ctor 2. for ExoProgram::run and run_results
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

};

} // namespace exo

#endif // EXO_JOB_H
