#ifndef JOB_MANAGER_HPP
#define JOB_MANAGER_HPP

#ifdef EXO_MODULE
	import std;
	import var;
#else
#	include <utility>
#   include <functional>
#   include <memory>
#   include <generator>
#   include <tuple>
#	include <exodus/var.h>
#endif

#include <exodus/job.h>

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

using Jobs = std::vector<Job>;
using Queue = ThreadSafeQueue<var>;
using ResultQueue = ThreadSafeQueue<ExoEnv>;

class PUBLIC JobManager {
public:
	JobManager() = default;
	~JobManager() = default;

//	std::shared_ptr<ResultQueue> result_queue_ = std::make_shared<ResultQueue>();

//	// Templated constructor to accept any callable and its arguments
//	template <typename F, typename... Args>
//	void co_run(F&& f, Args&&... args) {
//
//		// Create a lambda that captures the callable and arguments
//		auto callable = [f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
//			std::apply(f, std::move(args));
//		};
//
//		++co_run_count_;
//		do_run(callable);
//	}
//
//	// Get the number of fibers started
//	size_t co_run_count() const { return co_run_count_; }
//
//	// Generator for completed co_run results
//	std::generator<CoRunResult&> co_run_results();
//
//	// function to queue a result
//	void set_run_result(var data, var message);
//
//	// function to yield
//	void yield();

	// Prevent copying
	JobManager(const JobManager&) = delete;
	JobManager& operator=(const JobManager&) = delete;

	////////////////
	/// RUN THREAD :
	////////////////

	// Run another program in parallel.
	// Unlike perform and execute, run() does not block until it completes. It runs in parallel.
	// command: See perform.
	// return: A "job" object that you can use to manage the running job.
	// * Check if it has finished.
	// * Pause and wait for it to finish with optional timeout.
	// * When it has finished, access all its environment variables.
	// See also:
	// * run_results() * For use in a range based for-loops to simplify the asynchronous collection of output from all commands as they complete. Programs inner environments are retained for post processing so any suitable environment variable e.g. DATA, ANS or RECORD can be used to return data.
	// * shutdown_run() * Wait for all jobs to finish before continuing, otherwise, if the parent program exits, they will be aborted.
	//
	// `run("testlib aa");
	//  run("testlib bb");
	//  run("testlib cc");
	//  var results = "";
	//  // Process results asynchronously.
	//  for (auto& env : run_results())
	//      results ^= env.DATA.f(2) ^ FM;`
	//
	Job  run(in command);

	// Set the threadpool size
	// Initial configuration is 4.
	// max_threads: The default is 0 which sets the threadpool size to the number of processors in the system.
	auto setmaxthreads(std::size_t max_threads = 0) -> void;

	// Get the current threadpool size.
	auto getmaxthreads() -> var;

	// Get the number of processor cores available.
	auto getnumcores() -> var;

	// Process all run results asynchronously.
	// for (auto& env : run_results()) {
	//     ...
	// }
	auto run_results() -> std::generator<ExoEnv&>;

	// Get the number of jobs run and pending results.
	auto run_count() -> var;

	// Decrease the number of jobs pending results.
	auto decrement_run_count() -> var;

	// Wait for all jobs to finish.
	// Both pending and running jobs will be completed.
	// On program termination all jobs will be aborted.
	static auto shutdown_run() -> void;

	// Start a new batch of jobs.
	// Calls shutdown_run() first.
	static auto reset_run(size_t num_threads) -> void;

private:
//	void do_run(std::function<void()> fn);
//
//	struct Impl;
//	std::unique_ptr<Impl> impl_;
//	size_t co_run_count_ = 0;
	std::shared_ptr<ResultQueue> result_queue_ = std::make_shared<ResultQueue>();

};

} // namespace exo

#endif
