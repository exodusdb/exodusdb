#if EXO_MODULE
	import std;
#else
#	include <chrono>
#	include <iostream>
#	include <utility>
#endif

// Motivation:
// 1. Allow a thread to act as its own io event loop whether it has fibers or not.
// 2. Allow multiple fibers within a thread to use async io independently without blocking all fibers in the thread as would be the case if they rely on ordinary threadwise async io like poll/select.
// 3. If no fibers are enabled (all are waiting for i/o) then the whole thread should be in suspense until any i/o occurs.

#include "task_scheduler.h"

#define LOG if (0) std::cerr
//#define LOG std::cerr << "[Task Scheduler T" << std::this_thread::get_id() << "] "

namespace exo {

task_scheduler::task_scheduler(boost::asio::io_context& ioc) : io_context_(ioc) {
	LOG << "Initialized with io_context" << std::endl;
}

void task_scheduler::awakened(boost::fibers::context* ctx) noexcept {
	LOG << "Awakened (enabled) fiber: " << ctx << std::endl;
	boost::fibers::algo::round_robin::awakened(ctx);
}

boost::fibers::context* task_scheduler::pick_next() noexcept {
	auto ctx = boost::fibers::algo::round_robin::pick_next();
	if (!ctx && !io_context_.stopped()) {
		LOG << "No ready fibers, polling io_context" << std::endl;
		// Only one event at time to keep things smooth.
		// Assuming that there is little point in enabling many fibers together.
		size_t events = io_context_.poll_one();
		if (events) {
			LOG << "Polled " << events << " io_context event(s)" << std::endl;
			ctx = boost::fibers::algo::round_robin::pick_next();
		}
	}
	if (ctx) {
		LOG << "Picked ready fiber: " << ctx << std::endl;
	} else {
		LOG << "No ready fibers after poll, stopped: " << io_context_.stopped() << std::endl;
	}
	return ctx;
}

bool task_scheduler::has_ready_fibers() const noexcept {
	bool ready = boost::fibers::algo::round_robin::has_ready_fibers();
	LOG << "Has ready fibers: " << ready << std::endl;
	return ready;
}

void task_scheduler::suspend_until(std::chrono::steady_clock::time_point const& abs_time) noexcept {
	std::unique_lock<std::mutex> lock(mutex_);
	LOG << "Entering suspend_until: " << abs_time.time_since_epoch().count() << std::endl;

	while (!has_ready_fibers() && !io_context_.stopped() && !notified_) {
		if (io_context_.stopped()) {
			io_context_.restart();
		}

		auto now = std::chrono::steady_clock::now();
		boost::asio::steady_timer timer(io_context_, std::chrono::milliseconds(0));
		bool timed_out = false;
		bool io_completed = false;

		if (abs_time != std::chrono::steady_clock::time_point::max()) {
			auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(abs_time - now).count();
			if (timeout_ms > 0) {
				timer.expires_after(std::chrono::milliseconds(timeout_ms));
				timer.async_wait([&](const boost::system::error_code& ec) {
					if (!ec) {
						timed_out = true;
					}
				});
			}
		}

		size_t events = io_context_.run_one();
		if (events) {
			io_completed = true;
			LOG << "Processed " << events << " io_context event(s)" << std::endl;
		}

		if (!timed_out && abs_time != std::chrono::steady_clock::time_point::max()) {
			timer.cancel();
		}

		if (has_ready_fibers()) {
			LOG << "Ready fibers detected after io_context run" << std::endl;
			break;
		}

		if (!io_completed && !timed_out && !notified_) {
			if (abs_time == std::chrono::steady_clock::time_point::max()) {
				cond_.wait(lock);
			} else if (now < abs_time) {
				cond_.wait_until(lock, abs_time);
			}
		}

		notified_ = false;
	}

	auto ready_fiber_count = has_ready_fibers();
	LOG << "Exiting suspend_until, ready fibers: " << ready_fiber_count
		<< ", io_context stopped: " << io_context_.stopped() << std::endl;
}

void task_scheduler::notify() noexcept {
	LOG << "Notified, posting to io_context" << std::endl;
//	io_context_.post([this] {
// Suggested by grok to cater for long deprecated .post function finally removed in Boost 1.87
// https://x.com/i/grok/share/RYVdpebhVk1WkN2Ah22BQoMWx
	boost::asio::post(io_context_.get_executor(), [this] {
		std::unique_lock<std::mutex> lock(mutex_);
		notified_ = true;
		cond_.notify_all();
	});
}

task_scheduler::~task_scheduler() {
//	stopped_ = true;
	LOG << "Shutdown" << std::endl;
//	io_context_.stop(); // Prevent further handler execution
	// Drain remaining handlers
//	while (io_context_.run_one()) {
//		std::cout << "Drained handler\n";
//	}
//	boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
}

} // namespace exo

