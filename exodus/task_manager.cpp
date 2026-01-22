#if EXO_MODULE > 1
	import std;
#	include <memory> // work around bug in std module that disallows "using std::make_shared;"
#else
#	include <vector>
#	include <mutex>
#endif

#if EXO_MODULE
	import var;
	import exoprog; // for ExoExit
#else
#	include <var/var.h>
#	include <var/task_scheduler.h>
#	include <exodus/exoprog.h>
#endif

#include "task_manager.h"
#include "task_manager_impl.h"

#include <boost/fiber/all.hpp>

#define LOG if (0) std::cerr
//#define LOG std::cerr

namespace exo {

//TaskManager_Impl() : result_channel(128) {}
TaskManager::TaskManager() : impl_(std::make_unique<TaskManager_Impl>()) {}

TaskManager::~TaskManager() {
	impl_->result_channel.close();
	for (auto& fiber : impl_->fibers) {
		if (fiber.joinable()) {
			fiber.join();
		}
	}
}

void TaskManager::async_impl(std::function<void()> fn) {
	std::lock_guard<std::mutex> lock(impl_->mutex);
	impl_->fibers.emplace_back([fn = std::move(fn), this]() {
		LOG << "Running fiber." << std::endl;
		try {
			fn();
			LOG << "Task completed." << std::endl;

			// Require fibers to push their own results
			// impl_->result_channel.push(result_ptr);

		} catch (const ExoStop& e) {

//			// Save exception for rethrowing in main fiber
//			err_ptr = std::current_exception();
//
			// Queue a result
			auto result_ptr = std::make_shared<AsyncResult>();
			result_ptr->message = "Error: Task: " ^ e.message;
			LOG << result_ptr->message << std::endl;
			impl_->result_channel.push(result_ptr);

//			std::rethrow_exception(err_ptr);
//			var(e.stack()).convert(FM, "\n").errputl();

//		} catch (const ExoAbort& e) {
		} catch (const ExoExit& e) {

			// Save exception for rethrowing in main fiber
			err_ptr = std::current_exception();

			// Queue a result
			auto result_ptr = std::make_shared<AsyncResult>();
			result_ptr->message = "ExoAbort: Task: " ^ e.message;
			LOG << result_ptr->message << std::endl;
			impl_->result_channel.push(result_ptr);

//			std::rethrow_exception(err_ptr);
//			var(e.stack()).convert(FM, "\n").errputl();

		} catch (const VarError& e) {

			// Save exception for rethrowing in main fiber
			err_ptr = std::current_exception();

			// Queue a result
			auto result_ptr = std::make_shared<AsyncResult>();
			result_ptr->message = "Error: Task: " + e.message;
			LOG << result_ptr->message << std::endl;
			impl_->result_channel.push(result_ptr);

//			std::rethrow_exception(err_ptr);
//			var(e.stack()).convert(FM, "\n").errputl();

		} catch (const std::exception& e) {

			// Save exception for main fiber to handle
			err_ptr = std::current_exception();

			// Queue a result
			auto result_ptr = std::make_shared<AsyncResult>();
			result_ptr->message = "Error: Task: " + std::string(e.what());
			std::cerr << result_ptr->message << std::endl;
			impl_->result_channel.push(result_ptr);

		} catch (...) {

			// Save exception for main fiber to handle
			err_ptr = std::current_exception();

			// Queue a result
			auto result_ptr = std::make_shared<AsyncResult>();
			result_ptr->message = "Error: Task: Unknown.";
			std::cerr << result_ptr->message << std::endl;
			impl_->result_channel.push(result_ptr);

			// Access the scheduler
//			auto& sched = boost::fibers::context::active()->get_scheduler()->algorithm();
//			auto* custom_sched = dynamic_cast<task_scheduler*>(&*sched);
		}

		boost::this_fiber::yield();
	});
}

void TaskManager::yield() const {
	boost::this_fiber::yield();
}

void TaskManager::set_async_result(var data, var message) const {
	LOG << "set_async_result: " << std::endl;
	auto result_ptr = std::make_shared<AsyncResult>();
	result_ptr->data = std::move(data);
	result_ptr->message = std::move(message);
	impl_->result_channel.push(result_ptr);
}

#ifdef EXO_GENERATOR
auto TaskManager::async_results() -> std::generator<AsyncResult&> {
	LOG << "Expecting " << async_count_ << " results" << std::endl;
	while (async_count_) {
		async_count_--;
		std::shared_ptr<AsyncResult> result_ptr;
		if (impl_->result_channel.pop(result_ptr) == boost::fibers::channel_op_status::success) {
			LOG << "Popped result: data=" << result_ptr->data << ", message=" << result_ptr->message << std::endl;
			if (err_ptr) {
				std::rethrow_exception(err_ptr);
			}
			co_yield *result_ptr;
		} else {
			LOG << "Channel pop failed" << std::endl;
		}
	}
}
#else
auto TaskManager::async_results() -> ResultRange<TaskManager, AsyncResult> {
	return ResultRange<TaskManager, AsyncResult>(this);
}
#endif

} // namespace exo
