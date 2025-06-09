#ifdef EXO_MODULE
	import std;
#else
#	include <iostream>
#	include <iomanip>
#	include <chrono>
#	include <string>
#	include <utility>
#endif

// A fiber uses a boost::fiber:promise/future/get to talk DIRECT to io_context
// using a boost::asio::posix::stream_descriptor/socket
// This disables the fiber until OS io replies and our custom fiber scheduler
// calls io_context run_one? to let i0_context call back and enable our fiber again.

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
#include "task_scheduler.h"
#include "task_mutex.h"
#include "stream_ptr.h"

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
thread_local auto async_PQexec_count = 0; // Only for logging

// async_PQ_exec_impl<>(...)
//
// Provide an async wrapper around PQexec and PQexecParams.
//
// Use a template to avoid code duplication.
//
// Signature remains identical except that we return DBresult
// which is a PGresult convertible RAII container that
// also contains an errmsg used for pre-PGresult stage errors.
//
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

	// There are only three versions of this function
	static_assert(N == 1 || N == 2 || N == 3, "N must be 1 (PQexecParams), 2 (PQexec), 3 (PQexecPrepared");

	LOG << ++async_PQexec_count << ": async_PQexec<" << N << "> started: " << query << "\n";

	// Custom fiber scheduler that sleeps when no fibers are ready
	//////////////////////////////////////////////////////////////
	// and polls/runs the io_context only enough to pick up events
	// without requiring a classic independent event loop.
	// The run_one() function blocks until one handler has been dispatched, or until the io_context has been stopped.
//	boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
//	boost::fibers::use_scheduling_algorithm<boost::fibers::asio::yield>(io_context);
	thread_local bool scheduler_set = false;
	if (!scheduler_set) {
		LOG << "[Main] Setting scheduler on ioc: " << &io_context << std::endl;
		boost::fibers::use_scheduling_algorithm<task_scheduler>(io_context);
		scheduler_set = true;
	}

	// Lock the connection at least until we have obtained our PGresult.
	// Stop other fibers from using the same PGconn simultaneously.
	// TaskMutex is boost::fibers::mutex
	std::lock_guard<TaskMutex> lock(conn.dbconn->mutex_);

	DBresult dbresult;

	// Quit if connection isnt OK
	if (PQstatus(conn) != CONNECTION_OK) {
		dbresult.pqerrmsg = std::string("Connection bad before query: ") + PQerrorMessage(conn);
		LOG << dbresult.pqerrmsg << "\n";
		return dbresult;
	}

	// Set non-blocking
	LOG << "Set non-blocking:\n";
	if (PQsetnonblocking(conn, 1) != 0) {
		dbresult.pqerrmsg = std::string("Set non-blocking: ") + query;
		LOG << dbresult.pqerrmsg << "\n";
		return dbresult;
	}

	// Get posix socket
	int socket_fd = PQsocket(conn);
	if (socket_fd < 0) {
		dbresult.pqerrmsg = std::string("Failed to get socket: ") + query;
		LOG << dbresult.pqerrmsg << "\n";
		return dbresult;
	}
	LOG << "Socket FD: " << socket_fd << "\n";

	// StreamDescriptor is boost asio's handler for posix sockets
	using StreamDescriptor = boost::asio::posix::stream_descriptor;

	// Cleanup function for a StreamDescriptor
	auto cleanup = [](void* stream_ptr) {
		auto* s = static_cast<StreamDescriptor*>(stream_ptr);
		if (s && s->is_open()) {
			s->release();
		}
		delete s;
	};

	// conn has a StreamPtr. Initially with no stream.
	StreamPtr& stream_ptr = conn.dbconn->stream_ptr_;

	// Get the cached StreamDescriptor
	StreamDescriptor* stream = static_cast<StreamDescriptor*>(stream_ptr.get_stream());

	// Check if the stream exists, is open, and matches socket_fd
	if (stream && stream->is_open() && stream->native_handle() == socket_fd) {
		// Reuse the cached stream
		LOG << "Reusing cached boost asio posix stream_descriptor: \n";
	} else {
		// Stream is either not cached, closed, or has a different socket_fd
		if (stream) {
			// Clean up the existing stream
			LOG << "Cleaning up mismatched or invalid cached stream: \n";
			stream_ptr.set_stream(nullptr, nullptr); // Clear to avoid double-free
			cleanup(stream); // Call the cleanup function
		}

		// Create and cache a new stream
		LOG << "Creating new boost asio posix stream_descriptor: \n";
		stream = new StreamDescriptor(io_context);
		stream_ptr.set_stream(stream, cleanup); // Cache the new stream

		// Assign the socket_fd to the stream
		LOG << "Assigning boost asio posix stream_descriptor: \n";
		stream->assign(socket_fd);
	}

	// Send/start sending the query in one of two ways.
	int exit_status = -1;
	if constexpr (N == 1) {
		LOG << "PQsendQueryParams:" << static_cast<PGconn*>(conn) << "\n";
		exit_status = PQsendQueryParams(conn, query, nParams, paramTypes, paramValues, paramLengths, paramFormats, resultFormat);
	} else if constexpr (N == 2) {
		LOG << "PQsendQuery:" << static_cast<PGconn*>(conn) << "\n";
		exit_status = PQsendQuery(conn, query);
	} else if constexpr (N == 3) {
		LOG << "PQsendQueryPrepared:" << static_cast<PGconn*>(conn) << "\n";
		exit_status = PQsendQueryPrepared(conn, query, nParams, /*paramTypes,*/ paramValues, paramLengths, paramFormats, resultFormat);
	} else {
		// Doesnt compile pre-24.04
//		static_assert(false, "N must be 1, 2 or 3");
	}
	if (exit_status != 1) {
		std::string pqerr = PQerrorMessage(conn);
		dbresult.pqerrmsg = std::string("PQsendQuery failed: ") + std::to_string(exit_status) + " '" + pqerr + "'";
		LOG << dbresult.pqerrmsg << "\n";
		return dbresult;
	}
	LOG << "PQsend succeeded.\n";

	// Flush query data. "Query" is a misnomer since inserting, updating and numerous database manipulation commands are included."
	int flush_count = 0;
	while (PQflush(conn) == 1) {
		flush_count++;
		LOG << "PQflush iteration: " << flush_count << "\n";

		// Assumptions why not using shared_ptr<promise>:
		// 1. The loop always waits for future.get(), ensuring the stack-allocated promise
		//	remains valid until the async_wait callback runs.
		// 2. TCP/IP errors are handled by PQflush (-1) or async_wait's error_code,
		//	preventing premature loop exits.
		// 3. Task scheduler does not destroy the fiber before future.get().
		// 4. No timeouts or early loop exits (e.g., break, return) are introduced.
		// Review if: Loop cancellation (e.g., timeouts via stream->cancel()) or early
		// exits are added, as shared_ptr may be needed to ensure promise lifetime.
		// auto promise = std::make_shared<boost::fibers::promise<void>>();
		auto promise = boost::fibers::promise<void>();
		boost::fibers::future<void> future = promise.get_future();
		stream->async_wait(
			StreamDescriptor::wait_write,
			[&promise](const boost::system::error_code& ec) {
				LOG << "async_wait callback for PQflush, error: " << ec.message() << "\n";
				if (!ec) promise.set_value();
				else promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
			}
		);

// This looks like a busy loop if there is only one fiber. It was written before fiber
//		LOG << "PQflush: future.wait_for()\n";
//		while (future.wait_for(std::chrono::milliseconds(0)) != boost::fibers::future_status::ready) {
//			io_context.poll_one();
//			boost::this_fiber::yield();
//		}

		LOG << "PQflush: future.get()\n";
		future.get();

	} // PQflush loop

	if (PQflush(conn) == -1) {
		dbresult.pqerrmsg = std::string("Failed to flush query: ") + query + " " + PQerrorMessage(conn);
		LOG << dbresult.pqerrmsg << "\n";
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
//				return dbresult;
//			}

//			// Check for timeout
//			auto elapsed = std::chrono::steady_clock::now() - start;
//			if (elapsed > std::chrono::seconds(connection_timeout_seconds)) {
//				dbresult.pqerrmsg = std::string("Timeout waiting for PQisBusy: ") + std::to_string(connection_timeout_seconds) + "s " + query;
//				LOG << dbresult.pqerrmsg << "\n";
//				return dbresult;
//			}
		}

		{
			// Mandatory. DONT start waiting if already arrived and nothing else coming.

			LOG << "PQisBusy: Try consuming before waiting.\n";
			if (PQconsumeInput(conn) == 0) {
				dbresult.pqerrmsg = std::string("Failed to consume input: ") + query + " " + PQerrorMessage(conn);
				LOG << dbresult.pqerrmsg << "\n";
				return dbresult;
			}

			if (!PQisBusy(conn)) {
				LOG << "PQisBusy: Break out since query is no longer busy after consume.\n";
				break;
			}
		}

		// See note about shared_ptr<promise> in wait loops above
		// auto promise = std::make_shared<boost::fibers::promise<void>>();
		auto promise = boost::fibers::promise<void>();
		boost::fibers::future<void> future = promise.get_future();
		stream->async_wait(
			StreamDescriptor::wait_read,
			[&promise](const boost::system::error_code& ec) {
				LOG << "async_wait callback for PQisBusy, error: " << ec.message() << "\n";
				if (!ec) promise.set_value();
				else promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
			}
		);

		LOG << "PQisBusy: future.get()\n";
		future.get();

	} //PQisBusy loop
	LOG << "After PQisBusy:\n";

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

// Instantiate impl<3>
template
auto async_PQexec_impl<3>(
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