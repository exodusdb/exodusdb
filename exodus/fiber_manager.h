#ifndef FIBER_MANAGER_HPP
#define FIBER_MANAGER_HPP

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

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

class CoRunResult {
public:
	var data;
	var message;
};

class PUBLIC FiberManager {

	struct Impl;
	std::unique_ptr<Impl> impl_;
	size_t co_run_count_ = 0;

	void do_run(std::function<void()> fn);

public:
	FiberManager();
	~FiberManager();

	// Prevent copying
	FiberManager(const FiberManager&) = delete;
	FiberManager& operator=(const FiberManager&) = delete;

	/////////////////////
	/// Async functions :
	/////////////////////

	// Templated constructor to accept any callable and its arguments

	// Call any function asynchronously.
	// Exodus' asynchronous functions run to completion, or they request database i/o, or they call yield(), and their execution is interleaved with other async functions. Since they are not threaded, they are implicitly thread-safe and do not need to use locking when reading or updating shared data.
	// Exodus' asynchronous functions are fibers akin to threads but without parallel processing. They are extremely fast to setup and switch with complete stack and state.
	// The initial state of asynchronous functions is "ready, not running". Once the main program calls yield() then one is selected by a round robin scheduler and they begin running and yielding to each other.
	//
	// `//func add(in a, in b) {
	//  //    set_run_result(a + b);
	//	//	return "";
	//  //}
	//  for (var i : range(1, 100)) {
	//      co_run(&_ExoProgram::add, this, i, 2);
	//  }
	//  yield();
	//  var total = 0;
	//  for (auto result : co_run_results())
	//      total += result.data;
	//  assert(total eq 5250);`
	//
	template <typename F, typename... Args>
	void co_run(F&& f, Args&&... args) {

		// Create a lambda that captures the callable and arguments
		auto callable = [f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
			std::apply(f, std::move(args));
		};

		++co_run_count_;
		do_run(callable);
	}

	// Hand over execution to other fibers.
	// Execution can be suspended with full retention of stack and state.
	// A round robin scheduler resumes fibers in turn or sleeps if all are waiting for database i/o.
	//
	void yield() const;

	// Set a co_run result.
	// See co_run for an example.
	void set_run_result(var data, var message = "") const;

	// A range that can be used in range-based for loops to pick up all co_run results.
	// See co_run for an example.
	auto co_run_results() -> std::generator<CoRunResult&>;

	// Get the number of co_runs started.
	auto co_run_count() const -> var {return co_run_count_;}

//    // Calls shutdown, dtor, ctor. Useful to start a new batch of jobs followed by async collection of all run_results.
//    static auto reset_run(size_t num_threads) -> void;
//
};

} // namespace exo

#endif
