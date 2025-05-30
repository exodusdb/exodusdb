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
#	include <exception>

#	include <exodus/var.h>
#endif

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

class AsyncResult {
public:
	var data;
	var message;
};

class PUBLIC FiberManager {

	struct Impl;
	std::unique_ptr<Impl> impl_;
	size_t async_count_ = 0;
	std::exception_ptr err_ptr;

	void async_impl(std::function<void()> fn);

public:
	FiberManager();
	~FiberManager();

	// Prevent copying
	FiberManager(const FiberManager&) = delete;
	FiberManager& operator=(const FiberManager&) = delete;

	/////////////////////
	/// Async functions :
	/////////////////////

	// Templated constructor to accept any callable and its arguments.

	// Call any function asynchronously.
	// funcname: To call an Exodus program function the function name must be prefixed by "&_ExoProgram::" and followed by ", this" as follows:
	// e.g. async(&_ExoProgram::xxxxxx, this, arg1, arg2, etc)
	// Exodus' asynchronous functions run sequentially not simultaneously. Their execution will be interleaved with other async functions in the same thread of execution while database i/o is pending or when they call yield(). They are implicitly thread-safe but global state may be changed by other async functions.
	// Exodus' asynchronous functions are implemented as fibers which are akin to threads but without parallel processing or OS management. They are extremely fast to setup and and switch between and have a normal stack. They might be called "coprocesses" by comparion with "coroutines" which do not have stacks.
	// The initial state of an asynchronous function is "ready, not running". Once the main program calls yield() then one of the "ready, not running" async functions is selected by a round robin scheduler and it runs until complete, requests database i/o or yields, and so on.
	//
	// https://x.com/i/grok/share/N4zbWT3V2hb4qK71G0qT0togh
	//
	// * Concept * Aligns with the computer science concept of *cooperative multitasking*, where isolated execution contexts yield to a scheduler for interleaved execution.
	//
	// * Concurrency * Interleaved execution of multiple tasks (with full stacks) within a *single thread*, not parallelism (simultaneous multi-CPU execution).
	//
	// * Full Stacks * Independent, full execution context sharing global state but with separate call stacks, resembling process-like entities but not OS processes. Not lightweight coroutines (e.g., Python’s 'async def' or generators) due to their lack of isolation or stacks.
	//
	// * Yield * Each full stack can yield control explicitly to a scheduler, suspending its own execution and allowing another stack to run.
	//
	// Scheduler:
	//
	//  * A *minimal, non-interrupt-driven* mechanism that uses a *loop* to scan full stacks and decide which to run next.
	//  * Its sole role is to *pass control* (or "hand the baton") to the next stack, not to manage events (e.g., I/O, timers, or signals).
	//  * May optionally link to events if chosen, but this is not its primary function.
	//
	// * No OS * No OS-level process creation (e.g., no 'fork', 'spawn', or equivalents like Python’s 'multiprocessing').
	//
	// * No Threads * Multiple async functions share a *single thread*. Threads can have multiple async routines but not vice versa.
	//
	// * No Stackless Coroutines * Lightweight cooperative tasks (e.g., Python’s 'asyncio' coroutines or generators) are not considered full stacks
	//
	// * No Event Loop * The scheduler is not driven by interrupts or events (e.g., I/O polling, timers, or signals, as in 'asyncio's event loop).
	//
	// Comparable:
	//
	//  * Boost *  Boost Fibers are used to implement async in Exodus.
	//  * Go *     Goroutines have with a runtime scheduler loop (GOMAXPROCS=1), balancing scalability and ease.
	//  * Java *   Project Loom: Continuations provide stackful coroutines, but require a custom single-threaded, non-interrupt-driven scheduler.
	//  * Python * Stackless, a non-standard Python variant, has tasklets with a scheduler loop.
	//  * Lua *    Stackful coroutines with a user-defined scheduler loop, offering simplicity and control.
	//  * Ruby *   Native Fibers provide stackful coroutines with a user-defined, non-interrupt-driven scheduler loop.
	//  * Erlang * Lightweight VM processes with BEAM’s scheduler loop, providing strong isolation.
	//
	// +---------.-------------.--------------------------.-------------------------+
	// | Command | Mechanism   | Execution                | Use Case                |
	// |---------|-------------|--------------------------|-------------------------|
	// | async   | Fiber       | Cooperative, yield-based | Lightweight async tasks |
	// | run     | Thread pool | Parallel, preemptive     | Heavy parallel tasks    |
	// +---------.-------------.--------------------------.-------------------------+
	//
	// `//func add(in a, in b) {
	//  //    set_async_result(a + b);
	//  //	return "";
	//  //}
	//  for (var i : range(1, 100)) {
	//      async(&_ExoProgram::add, this, i, 2);
	//  }
	//  yield();
	//  var total = 0;
	//  for (auto result : async_results())
	//      total += result.data;
	//  assert(total eq 5250);`
	//
	template <typename F, typename... Args>
	void async(F&& funcname, Args&&... args) {

		// Create a lambda that captures the callable and arguments
		auto callable = [funcname = std::forward<F>(funcname), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
			std::apply(funcname, std::move(args));
		};

		++async_count_;
		async_impl(callable);
	}

	// Hand over execution to other fibers.
	// Execution can be suspended with full retention of stack and state.
	// A round robin scheduler resumes fibers in turn or sleeps if all are waiting for database i/o.
	//
	void yield() const;

	// Set a async result.
	// Callable in an async function to provide output collectible by async_results().
	// See async for an example.
	void set_async_result(var data, var message = "") const;

	// A range-based for loop to pick up all async results.
	// See async for an example.
	auto async_results() -> std::generator<AsyncResult&>;

	// Get the number of asyncs pending.
	// Not required if you use async_results to collect results.
	// Calling async increases the number and async_results reduces the number.
	auto async_count() const -> var {return async_count_;}

//    // Calls shutdown, dtor, ctor. Useful to start a new batch of jobs followed by async collection of all async_results.
//    static auto reset_async(size_t num_threads) -> void;
//
};

} // namespace exo

#endif
