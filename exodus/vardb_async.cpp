#ifdef EXO_MODULE
	import std;
#else
#	include <iostream>
#	include <iomanip>
#	include <chrono>
#	include <string>
#endif

// A fiber uses a boost::fiber:promise/future/get to talk DIRECT to io_context
// using a boost::asio::posix::stream_descriptor/socket
// This disables the fiber until OS io replies and our custom fiber scheduler
// calls io_context run_one? to let i0_context enable our fiber again.

// fibers talk directly to io_context using fiber aware promises and futures
// io_context is just the io buffer interface with OS containing queues etc.
// fiber scheduler (FS) is hidden threadlocal controlled by static functions.
// FS seems to create a fiber (stack in heap) for itself.
// The initial thread and its large stack is treated as an ordinary fiber.
// fibers can be in state enabled or not enabled. If waiting for IO they are not enabled.
// Custom FS calling io_context.run_one() suspends it until an OS/IO event occurs changing io_context's state.
// The fiber promise call back sent to iocontext is used by iocontext to enable the fiber.
// A fiber is a stack plus a set of cpu registers stored in process memory and is not OS related.
// Calling ordinary OS poll/select in a fiber puts it, and therefore the whole thread, to sleep.
// FS is not directly involved in io except to call io_context to handle event(s) using poll/run/one.
// Generally speaking we try to avoid conflating FS with io_context.

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/fiber/all.hpp>

#include <libpq-fe.h>

#include "DBresult.h"
#include "DBconn.h"
#include "fiber_scheduler.h"

// Logging macro with relative timestamp from program start
//#define ENABLE_LOGGING
#ifdef ENABLE_LOGGING
	// Static variable to capture initial time (initialized once at program start)
	inline static const auto start_time = std::chrono::system_clock::now();
#	define LOG std::cerr \
					<< "[" << std::fixed << std::setprecision(3) \
					<< (std::chrono::duration_cast<std::chrono::microseconds>( \
						std::chrono::system_clock::now().time_since_epoch()).count() - \
						std::chrono::duration_cast<std::chrono::microseconds>( \
						start_time.time_since_epoch()).count()) / 1000.0 \
					<< "ms, T" << std::this_thread::get_id() << "] AE "
//					<< "ms, T" << (start_time%1000000) << "] "
#else
	#define LOG if (0) std::cerr // No-op, optimized out
#endif

namespace exo {

// Thread data
thread_local boost::asio::io_context io_context;
thread_local auto work_guard = boost::asio::make_work_guard(io_context);
thread_local auto async_PQexec_count = 0; // Logging

// TODO one lock per DBconn
boost::fibers::mutex mtx;

// async_PQ_exec_impl<>(...)
//
// Provide an async wrapper around PQexec and PQexecParams.
//
// Use a template to avoid code duplication.
//
// Signature remains identical but we return a smarter
// C++ RAII container of PGresult called DBresult.
template<int N>
auto async_PQexec_impl(
	DBconn_ptr conn,
	const char* query,
	int nParams = 0,
	const Oid* paramTypes = nullptr,
	const char* const* paramValues = nullptr,
	const int* paramLengths = nullptr,
	const int* paramFormats = nullptr,
	int resultFormat = 0
) -> DBresult {
	static_assert(N == 1 || N == 2, "N must be 1 (PQexecParams) or 2 (PQexec)");

	LOG << ++async_PQexec_count << ": async_PQexec<" << N << "> started: " << query << "\n";

	// Custom fiber scheduler that sleeps when no fibers are ready
	// and runs the io_context just enough to pick up events
	// without requiring a classic independent event loop.
	thread_local bool scheduler_set = false;
	if (!scheduler_set) {
		LOG << "[Main] Setting scheduler on ioc: " << &io_context << std::endl;
		boost::fibers::use_scheduling_algorithm<fiber_scheduler>(io_context);
		scheduler_set = true;
	}
//	boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
//	boost::fibers::use_scheduling_algorithm<boost::fibers::asio::yield>(io_context);

	// Lock the mutex using std::lock_guard
	std::lock_guard<boost::fibers::mutex> lock(mtx);

	DBresult dbresult;

	// Check connection state
	if (PQstatus(conn) != CONNECTION_OK) {
		dbresult.pqerrmsg = std::string("Connection bad before query: ") + PQerrorMessage(conn);
		LOG << dbresult.pqerrmsg << "\n";
//		PQfinish(conn);
		return dbresult;
	}

	// Set non-blocking
	LOG << "Set non-blocking:\n";
	if (PQsetnonblocking(conn, 1) != 0) {
		dbresult.pqerrmsg = std::string("Set non-blocking: ") + query;
		LOG << dbresult.pqerrmsg << "\n";
//		PQfinish(conn);
		return dbresult;
	}

	// boost asio has a posix socket wrapper
	using StreamDescriptor = boost::asio::posix::stream_descriptor;

	// A boost asio socket releaser/delete
	auto releaser = [](StreamDescriptor* socket) {
		if (socket && socket->is_open()) socket->release();
		delete socket;
	};

	// A boost asio socket (StreamDescriptor)
	std::unique_ptr<StreamDescriptor, decltype(releaser)> socket(
		new StreamDescriptor(io_context),
		releaser
	);

	// Get posix socket
	int sock_fd = PQsocket(conn);
	if (sock_fd < 0) {
		dbresult.pqerrmsg = std::string("Failed to get socket: ") + query;
		LOG << dbresult.pqerrmsg << "\n";
//		PQfinish(conn);
		return dbresult;
	}
	LOG << "Socket FD: " << sock_fd << "\n";

	// Assign socket to boost asio socket
	LOG << "Assigning boost asio posix stream_descriptor socket: \n";
	socket->assign(sock_fd);

	// Send/start sending the query
	int exit_status = -1;
	if constexpr (N == 1) {
		LOG << "PQsendQueryParams:" << static_cast<PGconn*>(conn) << "\n";
		exit_status = PQsendQueryParams(conn, query, nParams, paramTypes, paramValues, paramLengths, paramFormats, resultFormat);
	} else {
		LOG << "PQsendQuery:" << static_cast<PGconn*>(conn) << "\n";
		exit_status = PQsendQuery(conn, query);
	}
	if (exit_status != 1) {
		std::string pqerr = PQerrorMessage(conn);
		dbresult.pqerrmsg = std::string("PQsendQuery failed: ") + std::to_string(exit_status) + " '" + pqerr + "'";
		LOG << dbresult.pqerrmsg << "\n";
//		socket->release();
		return dbresult;
	}
	LOG << "PQsend succeeded.\n";

	// Flush query data
	int flush_count = 0;
	while (PQflush(conn) == 1) {
		flush_count++;
		LOG << "PQflush iteration: " << flush_count << "\n";
//		boost::fibers::promise<void> promise;
		auto promise = std::make_shared<boost::fibers::promise<void>>();
		boost::fibers::future<void> future = promise->get_future();
		socket->async_wait(
			StreamDescriptor::wait_write,
			[promise](const boost::system::error_code& ec) {
				LOG << "async_wait callback for PQflush, error: " << ec.message() << "\n";
				if (!ec) promise->set_value();
				else promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
			}
		);
		LOG << "PQflush: future.wait_for()\n";
		while (future.wait_for(std::chrono::milliseconds(0)) != boost::fibers::future_status::ready) {
			io_context.poll_one();
			boost::this_fiber::yield();
		}
		LOG << "PQflush: future.get()\n";
		future.get();

	} // PQflush loop

	if (PQflush(conn) == -1) {
		dbresult.pqerrmsg = std::string("Failed to flush query: ") + query + " " + PQerrorMessage(conn);
		LOG << dbresult.pqerrmsg << "\n";
//		socket->release();
		return dbresult;
	}
	LOG << "After PQflush loop\n";

	// Receive response with timeout
//	auto start = std::chrono::steady_clock::now();
	auto busy_count = 0;
	while (PQisBusy(conn)) {
		LOG << "PQisBusy: Loop started. " << ++busy_count << "\n";

		{
			// Monitor connection status.
			// Seems to cause random hangs in future.get.

//			// Check connection state
//			if (PQstatus(conn) != CONNECTION_OK) {
//				dbresult.pqerrmsg = std::string("Connection lost: ") + query + " " + PQerrorMessage(conn);
//				LOG << dbresult.pqerrmsg << "\n";
//				socket->release();
//				return dbresult;
//			}

//			// Check for timeout
//			auto elapsed = std::chrono::steady_clock::now() - start;
//			if (elapsed > std::chrono::seconds(connection_timeout_seconds)) {
//				dbresult.pqerrmsg = std::string("Timeout waiting for PQisBusy: ") + std::to_string(connection_timeout_seconds) + "s " + query;
//				LOG << dbresult.pqerrmsg << "\n";
//				socket->release();
//				return dbresult;
//			}
		}

		{
			// Mandatory. DONT start waiting if already arrived and nothing else coming.

			LOG << "PQisBusy: Try consuming before waiting.\n";
			if (PQconsumeInput(conn) == 0) {
				dbresult.pqerrmsg = std::string("Failed to consume input: ") + query + " " + PQerrorMessage(conn);
				LOG << dbresult.pqerrmsg << "\n";
//				socket->release();
				return dbresult;
			}

			if (!PQisBusy(conn)) {
				LOG << "PQisBusy: Break out since query is no longer busy after consume.\n";
				break;
			}
		}

		short events = POLLIN; // Always wait for read after consuming input. Really? Why?
		auto promise = std::make_shared<boost::fibers::promise<void>>();
		boost::fibers::future<void> future = promise->get_future();

		socket->async_wait(
			StreamDescriptor::wait_read,
			[promise](const boost::system::error_code& ec) {
				LOG << "async_wait callback for PQisBusy, error: " << ec.message() << "\n";
				if (!ec) promise->set_value();
				else promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
			}
		);

		LOG << "PQisBusy: future.get()\n";
		future.get();

	} //PQisBusy loop
	LOG << "After PQisBusy:\n";

//	socket->release();

	// Get the first error result or the last ok result
	// but always flush all results
	bool error = false;
	DBresult res;
	while ((res = PQgetResult(conn))) {
		ExecStatusType status = PQresultStatus(res);
		if (!error)
			dbresult = std::move(res);
		if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
			error = true;
	}

	if (!dbresult)
		dbresult.pqerrmsg = std::string("No PQresult returned. ") + query;
	LOG << async_PQexec_count << ": exit: " << dbresult.pqerrmsg << "\n";

	return dbresult;
}

// Instantiate impl<1>
template
auto async_PQexec_impl<1>(
	DBconn_ptr conn,
	const char* query,
	int nParams = 0,
	const Oid* paramTypes = nullptr,
	const char* const* paramValues = nullptr,
	const int* paramLengths = nullptr,
	const int* paramFormats = nullptr,
	int resultFormat = 0
) -> DBresult;

// Instantiate impl<2>
template
auto async_PQexec_impl<2>(
	DBconn_ptr conn,
	const char* query,
	int nParams = 0,
	const Oid* paramTypes = nullptr,
	const char* const* paramValues = nullptr,
	const int* paramLengths = nullptr,
	const int* paramFormats = nullptr,
	int resultFormat = 0
) -> DBresult;

} // namespace exo