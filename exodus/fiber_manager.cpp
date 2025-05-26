#ifdef EXO_MODULE
	import std;
	import var;
#	include <memory> // work around bug in std module that disallows "using std::make_shared;"
#else
#	include <vector>
#	include <mutex>
#	include <exodus/var.h>
#endif

#include "fiber_manager.h"

#include <boost/fiber/all.hpp>

#define LOG if (0) std::cerr
//#define LOG std::cerr

namespace exo {

struct FiberManager::Impl {
	std::vector<boost::fibers::fiber> fibers;
	boost::fibers::buffered_channel<std::shared_ptr<CoRunResult>> result_channel;
	std::mutex mutex;

	Impl() : result_channel(128) {
		// Set round-robin scheduler for cooperative execution
		// DO NOT override the custom fiber scheduler set in vardb_async.
//		boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
	}
};

FiberManager::FiberManager() : impl_(std::make_unique<Impl>()) {}

FiberManager::~FiberManager() {
	impl_->result_channel.close();
	for (auto& fiber : impl_->fibers) {
		if (fiber.joinable()) {
			fiber.join();
		}
	}
}

void FiberManager::do_run(std::function<void()> fn) {
	std::lock_guard<std::mutex> lock(impl_->mutex);
	impl_->fibers.emplace_back([fn = std::move(fn), this]() {
		LOG << "Running fiber." << std::endl;
		try {
			fn();
			LOG << "Fiber completed." << std::endl;
//			impl_->result_channel.push(result_ptr);
		} catch (const VarError& e) {
			auto result_ptr = std::make_shared<CoRunResult>();
			result_ptr->message = "Error: " + e.message;
			LOG << "Fiber error: " << e.message << std::endl;
			impl_->result_channel.push(result_ptr);
		} catch (const std::exception& e) {
			auto result_ptr = std::make_shared<CoRunResult>();
			result_ptr->message = "Error: " + std::string(e.what());
			LOG << "Fiber error: " << e.what() << std::endl;
			impl_->result_channel.push(result_ptr);
		}
		boost::this_fiber::yield();
	});
}

void FiberManager::yield() const {
	boost::this_fiber::yield();
}

void FiberManager::set_run_result(var data, var message) const {
	LOG << "set_run_result: " << std::endl;
	auto result_ptr = std::make_shared<CoRunResult>();
	result_ptr->data = std::move(data);
	result_ptr->message = std::move(message);
	impl_->result_channel.push(result_ptr);
}

auto FiberManager::co_run_results() -> std::generator<CoRunResult&> {
	LOG << "Expecting " << co_run_count_ << " results" << std::endl;
	while (co_run_count_) {
		co_run_count_--;
		std::shared_ptr<CoRunResult> result_ptr;
		if (impl_->result_channel.pop(result_ptr) == boost::fibers::channel_op_status::success) {
			LOG << "Popped result: data=" << result_ptr->data << ", message=" << result_ptr->message << std::endl;
			co_yield *result_ptr;
		} else {
			LOG << "Channel pop failed" << std::endl;
		}
	}
}

} // namespace exo
