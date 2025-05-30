// job.cpp
#ifdef EXO_MODULE
	import std;
	import var;
#else
	#include <future>
	#include <memory>
	#include <chrono> // For std::chrono types
#endif

#include "exoenv.h"
#include "threadsafequeue.h"
#include "job.h"

// Define the opaque TimePoint as std::chrono::time_point<steady_clock, milliseconds>
class TimePoint {
public:
	using Clock = std::chrono::steady_clock;
	using Duration = std::chrono::milliseconds;
	using StdTimePoint = std::chrono::time_point<Clock, Duration>;

	TimePoint(StdTimePoint tp) : time_point(tp) {}
	StdTimePoint time_point;
};

namespace exo {

Job::Job(std::future<ExoEnv>&& fut,
		 std::shared_ptr<ThreadSafeQueue<var>> in_q,
		 std::shared_ptr<ThreadSafeQueue<var>> out_q)
	: future(std::move(fut)),
	  input_queue(std::move(in_q)),
	  output_queue(std::move(out_q)) {}

Job::Job(Job&& other) noexcept
	: future(std::move(other.future)),
	  input_queue(std::move(other.input_queue)),
	  output_queue(std::move(other.output_queue)) {}

Job& Job::operator=(Job&& other) noexcept {
	if (this != &other) {
		future = std::move(other.future);
		input_queue = std::move(other.input_queue);
		output_queue = std::move(other.output_queue);
	}
	return *this;
}

ExoEnv Job::get() {
	return future.get();
}

bool Job::valid() const noexcept {
	return future.valid();
}

void Job::wait() const {
	future.wait();
}

bool Job::wait_for(int duration) const {
	switch (future.wait_for(std::chrono::milliseconds(duration))) {
		case std::future_status::ready: return true;
		case std::future_status::timeout: return false;
		case std::future_status::deferred: return true;
	}
	return false;
}

std::future_status Job::wait_for(int duration, TimeUnit unit) const {
	if (unit == TimeUnit::Milliseconds) {
		return future.wait_for(std::chrono::milliseconds(duration));
	} else { // TimeUnit::Seconds
		return future.wait_for(std::chrono::seconds(duration));
	}
}

//std::future_status Job::wait_until(const TimePoint& abs_time) const {
//	return future.wait_until(abs_time.time_point);
//}
//
//// Utility to create a TimePoint
//TimePoint time_point_now_plus(int duration, TimeUnit unit) {
//	auto now = TimePoint::Clock::now();
//	if (unit == TimeUnit::Milliseconds) {
//		return TimePoint(now + std::chrono::milliseconds(duration));
//	} else { // TimeUnit::Seconds
//		return TimePoint(now + std::chrono::seconds(duration));
//	}
//}
//

Job::Job (
		var command,
		std::shared_ptr<ThreadSafeQueue<var>> input_queuex  /*= nullptr*/,
		std::shared_ptr<ThreadSafeQueue<var>> output_queuex /*= nullptr*/,
		std::shared_ptr<ThreadSafeQueue<ExoEnv>> result_queuex /*= nullptr*/
	)
	: input_queue(input_queuex   ?: std::make_shared<ThreadSafeQueue<var>>()),
	  output_queue(output_queuex ?: std::make_shared<ThreadSafeQueue<var>>())
	{

	// A future to allow async commands on jobs.
	// Finished jobs also post into a result queue
	// which is an alternative way to wait for completion.
	auto promise = std::make_shared<std::promise<ExoEnv>>();
	future = promise->get_future();

	// A global Job no starts at 0
	static std::atomic<unsigned int> global_job_counter{0};

	// Create a new environment with a counter for it's THREADNO.
	ExoEnv new_env(command, global_job_counter.fetch_add(1) + 1);

	// Assign queues to new_env.
	new_env.input_queue = input_queue;
	new_env.output_queue = output_queue;

	// Move new_env into shared_ptr.
	auto shared_env = std::make_shared<ExoEnv>(std::move(new_env));

	// Create a lambda to be queued on the threadpool's task queue.
	auto task = [promise, shared_env, command, cv = &cv, result_queuex]() mutable {
		try {

			// Prepare to load and use a shared library
			Callable callable{*shared_env};
			var libname = command.field(" ", 1);
			var funcname = "exoprogram_createdelete_";
			if (libname.contains(".")) {
				funcname ^= libname.field(".", 2);
				libname = libname.field(".", 1);
			}

			// Load a shared library and instantiate one of its objects.
			if (!callable.initsmf(*shared_env, libname.c_str(), funcname.c_str(), true))
				throw std::runtime_error("Failed to initialize shared member function");

			// Call a shared library objects member function.
			// It will handle only its ExoExit exceptions (Stop/Abort)
			// not VarError or anything else.
			shared_env->ANS = std::move(callable.callsmf());

			// Push normal result
			if (result_queuex)
				// EITHER if given a queue
				result_queuex->push(std::move(*shared_env));
			else
				// OR (Why not both since it is a shared_ptr)
				promise->set_value(std::move(*shared_env));

			// Notify Job
			cv->notify_one();

		} catch (VarError& e) {
			std::cerr << command << std::endl;
			std::cerr << e.stack() << std::endl;
			// Push abnormal result
			if (result_queuex) {
				result_queuex->push(std::move(*shared_env));
			}
			promise->set_exception(std::current_exception());
			cv->notify_one(); // Notify on exception
		} catch (...) {
			std::cerr << "Error: Job: Unknown error in " << command << std::endl;
			// Push abnormal result
			if (result_queuex) {
				result_queuex->push(std::move(*shared_env));
			}
			promise->set_exception(std::current_exception());
			cv->notify_one(); // Notify on exception
		}
//		std::clog << "Job:task lambda finished" << std::endl;

	}; // task lambda

	// Add the new job's lambda to the threadpool's task queue
	try {
		threadpool1.enqueue(std::move(task));
	} catch (const std::exception& e) {
		promise->set_exception(std::current_exception());
		cv.notify_one(); // Notify on enqueue failure
	}
}

} // namespace exo
