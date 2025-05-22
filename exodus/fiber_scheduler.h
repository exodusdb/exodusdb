#ifndef FIBER_SCHEDULER_H
#define FIBER_SCHEDULER_H

#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/fiber/all.hpp>
//#include <boost/fibers/algo/round_robin.hpp>

namespace exo {

class fiber_scheduler : public boost::fibers::algo::round_robin {
	boost::asio::io_context& io_context_;
	std::mutex mutex_;
	std::condition_variable cond_;
	bool notified_ = false;

public:
	fiber_scheduler(boost::asio::io_context& ioc);

	void awakened(boost::fibers::context* ctx) noexcept override;
	boost::fibers::context* pick_next() noexcept override;
	bool has_ready_fibers() const noexcept override;
	void suspend_until(std::chrono::steady_clock::time_point const& abs_time) noexcept override;
	void notify() noexcept override;
	~fiber_scheduler();
};

} // namespace exo

#endif // FIBER_SCHEDULER_H