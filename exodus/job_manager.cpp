#if EXO_MODULE > 1
	import std;
//	import var;
//#	include <memory> // work around bug in std module that disallows "using std::make_shared;"
#else
//#	include <vector>
//#	include <mutex>
//#	include <var/var.h>
#endif

#include "job_manager.h"

#define LOG if (0) std::cerr
//#define LOG std::cerr

namespace exo {

auto JobManager::setmaxthreads(std::size_t max_threads) -> void {
	threadpool1.set_max_threads(max_threads?:threadpool1.get_num_cores());
}

auto JobManager::getmaxthreads() -> var {
	return threadpool1.get_max_threads();
}

auto JobManager::getnumcores() -> var {
	return threadpool1.get_num_cores();
}

auto JobManager::run_count() -> var {
	return threadpool1.get_total_tasks_enqueued();
}

auto JobManager::decrement_run_count() -> var {
	return threadpool1.decrement_total_tasks_enqueued();
}

std::shared_ptr<ResultQueue> JobManager::result_queue() { return result_queue_; }

auto JobManager::shutdown_run() -> void {
	threadpool1.shutdown();
}

void JobManager::reset_run(std::size_t num_threads) {
	ThreadPool::reset(&threadpool1, num_threads);
}

#ifdef EXO_GENERATOR
auto JobManager::run_results() -> std::generator<ExoEnv&> {
	int rc = run_count();
	for (int i = 0; i < rc; ++i) {
		ExoEnv env;
		result_queue_->wait_and_pop(env);
		decrement_run_count();
		co_yield env;
	}
}
#else
auto JobManager::run_results() -> ResultRange<JobManager, ExoEnv> {
	return ResultRange<JobManager, ExoEnv>(this);
}
#endif

// Parallel run:
// return: Job containing a future<ExoEnv> and max_thread_no, input_queue, output_queue
// Creates new ExoEnv
Job JobManager::run(in command) {
	return Job(command, nullptr, nullptr, result_queue_);
}

} // namespace exo
