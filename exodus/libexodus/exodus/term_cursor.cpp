#include <iostream>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>

#if EXO_MODULE
	import exoprog;
#else
#	include <exodus/exoprog.h>
#endif

// Static state to track error count and disabled status
thread_local static int thread_error_count = 0;
thread_local static int thread_max_errors  = 3;

enum class CursorError {
    SUCCESS = 0,         // Valid response received
    TIMEOUT,             // No response within timeout
    READ_ERROR,          // Failed to read from terminal
    INVALID_RESPONSE,    // Response didn’t match expected format
    SETUP_ERROR,         // Failed to configure terminal
    DISABLED             // Function disabled due to too many errors
};

[[nodiscard]]
bool getCursorPos(
		int& x,
		int& y,
		double& delay_ms,
		CursorError& error,
		int timeout_ms = 3000
	) {

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Initialize outputs and variables used after goto
    x = -1;
    y = -1;
    delay_ms = 0.0;
    error = CursorError::SUCCESS;
    int i = 0;           // Moved up from read loop
    char buf[32];        // Moved up from read loop
    int poll_ret = 0;    // Moved up to avoid jump-past-initialization
    struct pollfd fds;   // Moved up, initialized later

    // Check if too many errors
    if (thread_error_count > thread_max_errors) {
        error = CursorError::DISABLED;
        clock_gettime(CLOCK_MONOTONIC, &end);
        delay_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
        return false;
    }

    // Configure terminal
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        error = CursorError::SETUP_ERROR;
        goto cleanup;
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        error = CursorError::SETUP_ERROR;
        goto cleanup;
    }

    // Send query
    std::cout << "\033[6n" << std::flush;

    // Wait for response
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;
    poll_ret = poll(&fds, 1, timeout_ms); // Assign here, declared above

    if (poll_ret < 0) {
        error = CursorError::READ_ERROR;
        goto cleanup;
    }
    if (poll_ret == 0) {
        error = CursorError::TIMEOUT;
        goto cleanup;
    }

    // Read the response
    while (i < sizeof(buf) - 1) {
        ssize_t bytes = read(STDIN_FILENO, &buf[i], 1);
        if (bytes < 0) {
            error = CursorError::READ_ERROR;
            goto cleanup;
        }
        if (bytes == 0 || buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    // Validate and parse
    int row, col;
    if (i < 4 || buf[0] != '\033' || buf[1] != '[' || sscanf(buf, "\033[%d;%dR", &row, &col) != 2 || row < 1 || col < 1) {
        error = CursorError::INVALID_RESPONSE;
        goto cleanup;
    }

    // Success
    x = col - 1;
    y = row - 1;
    thread_error_count = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    clock_gettime(CLOCK_MONOTONIC, &end);
    delay_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
    return true;

cleanup:
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	clock_gettime(CLOCK_MONOTONIC, &end);
	delay_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
	if (error != CursorError::SUCCESS) {
		thread_error_count++;
		if (thread_error_count > thread_max_errors) {
			error = CursorError::DISABLED;
		}
	}
	return false;

}

namespace exo {

var ExoProgram::getcursor() const {

	var cursor;
	if (not getcursor(cursor)) {};

	return cursor;
}

bool ExoProgram::getcursor(out cursor, int timeout_ms = 3000, int max_ignorable_errors = 0) const {

	// Set max errors if requested to do so.
	// simultaneously reset the error_count
	if (max_ignorable_errors) {
		thread_max_errors = max_ignorable_errors;
		thread_error_count = 0;
	}

	// No cursor if stdin is not a terminal
	if (not var().isterminal()) {
		cursor = "";
		return false;
	}

	// Force out any previous cursor control characters
	var().osflush();

	//TIMEOUT
	//READ_ERROR
	//INVALID_RESPONSE
	//SETUP_ERROR
	//DISABLED

	int x, y;
	double delay_ms;
	CursorError err;
	if (not getCursorPos(x, y, delay_ms, err, timeout_ms)) {};
	switch (err) {

		case CursorError::SUCCESS:
//			std::cout << "Cursor position: (" << x << ", " << y << "), Delay: " << delay_ms << " ms" << std::endl;
			cursor = x ^ FM ^ y ^ FM ^ delay_ms;
			return true;

		case CursorError::TIMEOUT:
			std::cout << "getcursor: Timeout after " << delay_ms << " ms" << std::endl;
			cursor = FM ^ FM ^ delay_ms ^ FM ^ "TIMEOUT";
			return false;

		case CursorError::READ_ERROR:
			std::cout << "getcursor: Read error after " << delay_ms << " ms" << std::endl;
			cursor = FM ^ FM ^ delay_ms ^ FM ^ "READ_ERROR";
			return false;

		case CursorError::INVALID_RESPONSE:
			std::cout << "getcursor: Invalid response after " << delay_ms << " ms" << std::endl;
			cursor = FM ^ FM ^ delay_ms ^ FM ^ "INVALID_RESPONSE";
			return false;

		case CursorError::SETUP_ERROR:
			std::cout << "getcursor: Terminal setup failed" << std::endl;
			cursor = FM ^ FM ^ delay_ms ^ FM ^ "SETUP_ERROR";
			return false;

		case CursorError::DISABLED:
			std::cout << "getcursor: Terminal disabled" << std::endl;
			cursor = FM ^ FM ^ delay_ms ^ FM ^ "DISABLED";
			return false;
	}

	cursor = FM ^ FM ^ delay_ms ^ FM ^ "UNKNOWN_ERROR";
	return false;

} // getcursor

void ExoProgram::setcursor(in cursor) const {

	AT(cursor.f(1), cursor.f(2)).output();
	cursor.osflush();

	return;
}

}  // namespace exo
