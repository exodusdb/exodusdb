#ifndef JOB_MANAGER_HPP
#define JOB_MANAGER_HPP

#ifdef EXO_MODULE
	import std;
	import var;
#	define EXO_GENERATOR
//#		include <exodus/result_range.h>
#else
#	include <utility>
#   include <functional>
#   include <memory>
#   include <tuple>
//#	include <version>
//#	ifdef __cpp_lib_generator // 202207L
#	if __has_include(<generator>)
#		define EXO_GENERATOR
#   	include <generator>
#	else
#		include <exodus/result_range.h>
#	endif
#
#	include <exodus/var.h>
#endif

#include <exodus/job.h>

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

using Jobs = std::vector<Job>;
using Queue = ThreadSafeQueue<var>;
using ResultQueue = ThreadSafeQueue<ExoEnv>;

class PUBLIC JobManager {
	std::shared_ptr<ResultQueue> result_queue_ = std::make_shared<ResultQueue>();

public:
	JobManager() = default;
	~JobManager() = default;

	// Prevent copying
	JobManager(const JobManager&) = delete;
	JobManager& operator=(const JobManager&) = delete;

#ifndef EXO_GENERATOR
	friend ResultRange<JobManager, ExoEnv>;
#endif

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

	////////////////
	/// RUN THREAD :
	////////////////

	// Run another program in parallel.
	// Unlike perform and execute, run() does not block until it completes. It runs in parallel.
	// The program runs in new thread with its own copy of environment variables, RECORD, ID, ANS etc.
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

#ifdef EXO_GENERATOR

	// Process all run results in parallel as they become available.
	// return: range based for loop argument.
	// See run() for an example.
	auto run_results() -> std::generator<ExoEnv&>;

#else
/*nodoc*/	auto run_results() -> ResultRange<JobManager, ExoEnv>;
#endif

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
	static auto reset_run(std::size_t num_threads) -> void;

/*nodoc*/	auto result_queue() -> std::shared_ptr<ResultQueue>;

};

} // namespace exo

#endif
