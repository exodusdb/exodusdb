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
	// Exodus' asynchronous functions run sequentially not simultaneously. Their execution will be interleaved with other async functions while database i/o is pending or when they call yield(). They are implicitly thread-safe. Global state may be changed by other async functions but not simultaneously.
	// Exodus' asynchronous functions are fibers akin to threads but without parallel processing or OS management. They are extremely fast to setup and and switch between and have a normal stack. They might be called "coprocesses" by comparion with "coroutines" which do not have stacks.
	// The initial state of an asynchronous function is "ready, not running". Once the main program calls yield() then one of the "ready, not running" async functions is selected by a round robin scheduler and it runs until complete, requests database i/o or yields, and so on.
	//
	// `//func add(in a, in b) {
	//  //    set_run_result(a + b);
	//  //	return "";
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
	// Callable in an async function to provide output collectible by co_run_results().
	// See co_run for an example.
	void set_run_result(var data, var message = "") const;

	// A range-based for loop to pick up all co_run results.
	// See co_run for an example.
	auto co_run_results() -> std::generator<CoRunResult&>;

	// Get the number of co_runs pending.
	// Not required if you use co_run_results to collect results.
	// Calling co_run increases the number and co_run_results reduces the number.
	auto co_run_count() const -> var {return co_run_count_;}

//    // Calls shutdown, dtor, ctor. Useful to start a new batch of jobs followed by async collection of all run_results.
//    static auto reset_run(size_t num_threads) -> void;
//
};

} // namespace exo

#endif
