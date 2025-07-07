#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>
#include <errno.h>
#include <string>
#include <vector>
#include <poll.h>
#include <ctime>
#include <sys/time.h>
#include <cctype>

namespace exo {

//#define PUBLIC __attribute__((visibility("default")))
//#define LOGGING

void log(const std::string& msg) {
#ifdef LOGGING
	std::time_t now = std::time(nullptr);
	char* time_str = std::ctime(&now);
	time_str[strlen(time_str) - 1] = '\0';
	std::cout << time_str << " - " << msg << std::endl;
#endif
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) / 1000000.0;
}

std::vector<std::string> parse_command_line(const std::string& cmd) {
	std::vector<std::string> args;
	std::string current_arg;
	bool in_quotes = false;
	char quote_type = '\0';
	bool escaped = false;

	for (size_t i = 0; i < cmd.length(); ++i) {
		char c = cmd[i];

		if (escaped) {
			current_arg += c;
			escaped = false;
			continue;
		}

		if (c == '\\') {
			escaped = true;
			continue;
		}

		if (in_quotes) {
			if (c == quote_type) {
				in_quotes = false;
			} else {
				current_arg += c;
			}
		} else {
			if (c == '"' || c == '\'') {
				in_quotes = true;
				quote_type = c;
			} else if (std::isspace(static_cast<unsigned char>(c))) {
				if (!current_arg.empty()) {
					args.push_back(current_arg);
					current_arg.clear();
				}
			} else {
				current_arg += c;
			}
		}
	}

	if (!current_arg.empty()) {
		if (in_quotes) {
			std::cerr << "Warning: Unclosed quote in command: " << cmd << std::endl;
		}
		args.push_back(current_arg);
	}

	return args;
}

/*
exit_status if false
not zero from client or
User createable:
127: No args. Treated as "no program provided."
127: Program not found. Child exits with 127.
-1: Timeout. Internal error when we terminate the child.
Internal:
-2: Pipe fail.
-3: Fork fail.
-4: Rare internal failure (poll fail, abnormal termination).
*/

// Implementation of var::osprocess

bool run_piped_process_with_timeout(const std::string& input, std::string& stdout_output, std::string& stderr_output, int& exit_status, int timeout_seconds, const std::string& command) {
    int parent_to_child[2];
    int child_to_parent[2];
    int child_to_parent_err[2];

    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1 || pipe(child_to_parent_err) == -1) {
        perror("pipe failed");
        exit_status = -2; // Pipe fail (throw)
        return false;     // Will throw in caller if needed
    }

    std::vector<std::string> args = parse_command_line(command);
    if (args.empty()) {
        std::cerr << "No command provided" << std::endl;
        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);
        close(child_to_parent_err[0]);
        close(child_to_parent_err[1]);
        exit_status = 127; // No args (document)
        return false;
    }

    std::vector<const char*> exec_args;
    for (const auto& arg : args) {
        exec_args.push_back(arg.c_str());
    }
    exec_args.push_back(nullptr);

    std::string arg_log = "Executing: ";
    for (const auto* arg : exec_args) {
        if (arg) arg_log += std::string(arg) + " ";
    }
    log(arg_log);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);
        close(child_to_parent_err[0]);
        close(child_to_parent_err[1]);
        exit_status = -3; // Fork fail (throw)
        return false;     // Will throw in caller if needed
    }

    if (pid == 0) { // Child
        dup2(parent_to_child[0], STDIN_FILENO);
        close(parent_to_child[0]);
        close(parent_to_child[1]);

        dup2(child_to_parent[1], STDOUT_FILENO);
        close(child_to_parent[0]);
        close(child_to_parent[1]);

        dup2(child_to_parent_err[1], STDERR_FILENO);
        close(child_to_parent_err[0]);
        close(child_to_parent_err[1]);

        execvp(exec_args[0], const_cast<char* const*>(exec_args.data()));
        perror("execvp failed");
        exit(127); // Program not found
    }

    log("Parent PID: " + std::to_string(getpid()) + ", Child PID: " + std::to_string(pid));

    close(parent_to_child[0]);
    close(child_to_parent[1]);
    close(child_to_parent_err[1]);

    fcntl(parent_to_child[1], F_SETFL, O_NONBLOCK);
    fcntl(child_to_parent[0], F_SETFL, O_NONBLOCK);
    fcntl(child_to_parent_err[0], F_SETFL, O_NONBLOCK);

    struct pollfd fds[3];
    fds[0].fd = parent_to_child[1];
    fds[0].events = POLLOUT;
    fds[1].fd = child_to_parent[0];
    fds[1].events = POLLIN;
    fds[2].fd = child_to_parent_err[0];
    fds[2].events = POLLIN;

    size_t written = 0;
    stdout_output.clear();
    stderr_output.clear();
    const size_t chunk_size = 4096;
    char buffer[chunk_size];
    bool input_done = false;
    bool stdout_done = false;
    bool stderr_done = false;
    bool child_reaped = false;

    double last_read_time = get_time();
    const double timeout = static_cast<double>(timeout_seconds);

    log("Starting I/O loop with child PID " + std::to_string(pid));
    while ((!input_done || !stdout_done || !stderr_done) && !child_reaped) {
        double current_time = get_time();
        double elapsed_since_last_read = current_time - last_read_time;

        if (timeout_seconds > 0 && elapsed_since_last_read >= timeout) {
            log("Timeout reached (no read activity for " + std::to_string(timeout_seconds) + "s), killing child " + std::to_string(pid));
            if (kill(pid, SIGTERM) == -1) {
                perror("kill failed");
            }
            close(parent_to_child[1]);
            close(child_to_parent[0]);
            close(child_to_parent_err[0]);
            int status = 0;
            pid_t wait_result = waitpid(pid, &status, 0);
            if (wait_result == -1) {
                perror("waitpid failed after timeout");
                exit_status = -1; // Timeout (document)
            } else {
                exit_status = -1; // Timeout, we sent SIGTERM
                log("Child terminated due to timeout");
            }
            return false;
        }

        int status = 0;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid) {
            log("Child exited during loop");
            child_reaped = true;
            if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
                log("Child exited with status " + std::to_string(exit_status));
                if (exit_status == 127) {
                    log("Child failed to execute program");
                    exit_status = 127; // Program not found (document)
                    return false;
                }
            } else if (WIFSIGNALED(status)) {
                exit_status = 128 + WTERMSIG(status); // POSIX standard: 128 + signal
                log("Child terminated by signal " + std::to_string(WTERMSIG(status)));
            } else {
                exit_status = -4; // Rare internal failure (throw)
                log("Child terminated abnormally");
                return false; // Will throw in caller if needed
            }
            break;
        }

        int poll_result = poll(fds, 3, 100);
        if (poll_result == -1) {
            if (errno == EINTR) continue;
            perror("poll failed");
            int status = 0;
            pid_t wait_result = waitpid(pid, &status, WNOHANG);
            if (wait_result == pid) {
                log("Child exited during poll failure");
                child_reaped = true;
                if (WIFEXITED(status)) {
                    exit_status = WEXITSTATUS(status);
                    log("Child exited with status " + std::to_string(exit_status));
                    if (exit_status == 127) {
                        log("Child failed to execute program");
                        exit_status = 127; // Program not found (document)
                        return false;
                    }
                } else if (WIFSIGNALED(status)) {
                    exit_status = 128 + WTERMSIG(status); // POSIX standard: 128 + signal
                    log("Child terminated by signal " + std::to_string(WTERMSIG(status)));
                } else {
                    exit_status = -4; // Rare internal failure (throw)
                    log("Child terminated abnormally");
                    return false; // Will throw in caller if needed
                }
                break;
            }
            exit_status = -4; // Rare internal failure (throw)
            return false;     // Will throw in caller if needed
        }

        if (poll_result == 0) continue;

        if (!input_done && (fds[0].revents & POLLOUT)) {
            size_t to_write = std::min(chunk_size, input.size() - written);
            if (to_write > 0) {
                ssize_t bytes_written = write(parent_to_child[1], input.data() + written, to_write);
                if (bytes_written > 0) {
                    written += static_cast<size_t>(bytes_written);
                    log("Wrote " + std::to_string(bytes_written) + " bytes, total " + std::to_string(written));
                } else if (bytes_written == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("write failed");
                    break;
                }
            }
            if (written >= input.size()) {
                log("Finished writing input, closing write pipe");
                close(parent_to_child[1]);
                input_done = true;
                fds[0].fd = -1;
            }
        }

        if (!stdout_done && (fds[1].revents & POLLIN)) {
            ssize_t bytes_read = read(child_to_parent[0], buffer, chunk_size);
            if (bytes_read > 0) {
                stdout_output.append(buffer, static_cast<size_t>(bytes_read));
                log("Read stdout " + std::to_string(bytes_read) + " bytes, total " + std::to_string(stdout_output.size()));
                last_read_time = get_time();
            } else if (bytes_read == 0) {
                log("Child closed stdout, closing read pipe");
                close(child_to_parent[0]);
                stdout_done = true;
            } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read stdout failed");
                break;
            }
        }

        if (!stderr_done && (fds[2].revents & POLLIN)) {
            ssize_t bytes_read = read(child_to_parent_err[0], buffer, chunk_size);
            if (bytes_read > 0) {
                stderr_output.append(buffer, static_cast<size_t>(bytes_read));
                log("Read stderr " + std::to_string(bytes_read) + " bytes, total " + std::to_string(stderr_output.size()));
                last_read_time = get_time();
            } else if (bytes_read == 0) {
                log("Child closed stderr, closing read pipe");
                close(child_to_parent_err[0]);
                stderr_done = true;
            } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read stderr failed");
                break;
            }
        }
    }

    if (!child_reaped) {
        log("Child not reaped yet, forcing wait");
        int status = 0;
        pid_t wait_result = waitpid(pid, &status, 0);
        if (wait_result == pid) {
            log("Child reaped after loop");
            child_reaped = true;
            if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
                log("Child exited with status " + std::to_string(exit_status));
                if (exit_status == 127) {
                    log("Child failed to execute program");
                    exit_status = 127; // Program not found (document)
                    return false;
                }
            } else if (WIFSIGNALED(status)) {
                exit_status = 128 + WTERMSIG(status); // POSIX standard: 128 + signal
                log("Child terminated by signal " + std::to_string(WTERMSIG(status)));
            } else {
                exit_status = -4; // Rare internal failure (throw)
                log("Child terminated abnormally");
                return false; // Will throw in caller if needed
            }
        } else if (wait_result == -1) {
            perror("Final waitpid failed");
            exit_status = -4; // Rare internal failure (throw)
            return false;     // Will throw in caller if needed
        }
    }

    if (child_reaped) {
        log("Draining remaining stdout/stderr");
        while (!stdout_done || !stderr_done) {
            if (!stdout_done) {
                ssize_t bytes_read = read(child_to_parent[0], buffer, chunk_size);
                if (bytes_read > 0) {
                    stdout_output.append(buffer, static_cast<size_t>(bytes_read));
                    log("Post-exit read stdout " + std::to_string(bytes_read) + " bytes, total " + std::to_string(stdout_output.size()));
                } else if (bytes_read == 0) {
                    log("Child closed stdout (post-exit), closing read pipe");
                    close(child_to_parent[0]);
                    stdout_done = true;
                } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("post-exit read stdout failed");
                    stdout_done = true;
                }
            }

            if (!stderr_done) {
                ssize_t bytes_read = read(child_to_parent_err[0], buffer, chunk_size);
                if (bytes_read > 0) {
                    stderr_output.append(buffer, static_cast<size_t>(bytes_read));
                    log("Post-exit read stderr " + std::to_string(bytes_read) + " bytes, total " + std::to_string(stderr_output.size()));
                } else if (bytes_read == 0) {
                    log("Child closed stderr (post-exit), closing read pipe");
                    close(child_to_parent_err[0]);
                    stderr_done = true;
                } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("post-exit read stderr failed");
                    stderr_done = true;
                }
            }

            if (!stdout_done || !stderr_done) {
                usleep(1000); // 1ms sleep to avoid tight loop
            } else {
                break;
            }
        }
        log("Drain complete");
    }

    if (!input_done) close(parent_to_child[1]);
    if (!stdout_done) close(child_to_parent[0]);
    if (!stderr_done) close(child_to_parent_err[0]);

    return child_reaped;
}

} // namespace exo







#if 0
int main() {
	bool all_tests_passed = true;

	// Test 1: Grep with various command-line cases, 2730 bytes input
	std::cout << "Test 1 - Grep with Command-Line Parsing (2/3 Block Size)" << std::endl;
	std::vector<std::string> commands = {
		"grep foo",
		"grep \"foo bar\"",
		"grep \"foo \\\"bar\\\"\"",
		"grep 'foo bar'",
		"grep foo\\ bar",
		"grep - xxxx"
	};
	std::string input_chunk = "hello foo bar\n";
	std::string input1;
	for (int i = 0; i < 195; ++i) input1 += input_chunk; // 2730 bytes (195 * 14)
	std::string expected_output_foo_bar;
	for (int i = 0; i < 195; ++i) expected_output_foo_bar += input_chunk; // 2730 bytes
	std::vector<std::string> expected_stdout_outputs = {
		expected_output_foo_bar, // All lines have "foo"
		expected_output_foo_bar, // All lines have "foo bar"
		"",					  // No "foo \"bar\"" matches
		expected_output_foo_bar, // All lines have "foo bar"
		expected_output_foo_bar, // All lines have "foo bar"
		""
	};
	std::vector<std::string> expected_stderr_outputs = {
		"",
		"",
		"",
		"",
		"",
		"grep: xxxx: No such file or directory\n"
	};
	std::vector<int> expected_exit_statuses = {
		0,
		0,
		1, // No matches for "foo \"bar\""
		0,
		0,
		2
	};

	for (size_t i = 0; i < commands.size(); ++i) {
		std::cout << "\nSubtest - Command: " << commands[i] << "\nInput size: " << input1.size() << " bytes" << std::endl;
		std::cout << "Input: " << input1.substr(0, 50) << "..." << std::endl;

		std::string stdout_output1, stderr_output1;
		int exit_status1 = -1;
		bool success = run_piped_process_with_timeout(input1, stdout_output1, stderr_output1, exit_status1, 10, commands[i]);
		if (success && stdout_output1 == expected_stdout_outputs[i] && stderr_output1 == expected_stderr_outputs[i] && exit_status1 == expected_exit_statuses[i]) {
			std::cout << "Process completed successfully" << std::endl;
			std::cout << "Stdout size: " << stdout_output1.size() << " bytes" << std::endl;
			std::cout << "Stdout: " << stdout_output1.substr(0, 50) << (stdout_output1.size() > 50 ? "..." : "") << std::endl;
			std::cout << "Stderr: " << (stderr_output1.empty() ? "(empty)" : stderr_output1) << std::endl;
			std::cout << "Exit status: " << exit_status1 << std::endl;
			std::cout << "Subtest passed" << std::endl;
		} else {
			std::cout << "Subtest failed" << std::endl;
			if (!success) {
				std::cout << "Process failed or timed out" << std::endl;
			} else {
				std::cout << "Expected stdout size: " << expected_stdout_outputs[i].size() << ", Actual size: " << stdout_output1.size() << std::endl;
				std::cout << "Expected stdout: " << expected_stdout_outputs[i].substr(0, 50) << "..." << std::endl;
				std::cout << "Actual stdout: " << stdout_output1.substr(0, 50) << "..." << std::endl;
				std::cout << "Expected stderr: " << expected_stderr_outputs[i] << std::endl;
				std::cout << "Actual stderr: " << (stderr_output1.empty() ? "(empty)" : stderr_output1) << std::endl;
				std::cout << "Expected exit status: " << expected_exit_statuses[i] << ", Actual: " << exit_status1 << std::endl;
			}
			all_tests_passed = false;
		}
	}

	// Test 2: Big data (1GB + 7 bytes)
	std::cout << "\nTest 2 - Big Data (1GB + 7 bytes)" << std::endl;
	std::string command2 = "cat";
	std::string input2(1024 * 1024 * 1024 + 7, 'a'); // 1GB + 7 bytes
	input2[input2.size() - 4] = 'E';
	input2[input2.size() - 3] = 'N';
	input2[input2.size() - 2] = 'D';
	input2[input2.size() - 1] = '\n';
	std::cout << "Command: " << command2 << "\nInput size: " << input2.size() << " bytes" << std::endl;

	std::string stdout_output2, stderr_output2;
	int exit_status2 = -1;
	bool success2 = run_piped_process_with_timeout(input2, stdout_output2, stderr_output2, exit_status2, 60, command2);
	if (success2 && stdout_output2.size() == input2.size() && stdout_output2.substr(stdout_output2.size() - 4) == "END\n" && stderr_output2.empty() && exit_status2 == 0) {
		std::cout << "Process completed successfully" << std::endl;
		std::cout << "Stdout size: " << stdout_output2.size() << " bytes" << std::endl;
		std::cout << "Last 4 chars: " << stdout_output2.substr(stdout_output2.size() - 4) << std::endl;
		std::cout << "Stderr: (empty)" << std::endl;
		std::cout << "Exit status: " << exit_status2 << std::endl;
		std::cout << "Test passed" << std::endl;
	} else {
		std::cout << "Test failed" << std::endl;
		if (!success2) {
			std::cout << "Process failed or timed out" << std::endl;
		} else {
			std::cout << "Expected size: " << input2.size() << ", Actual size: " << stdout_output2.size() << std::endl;
			if (stdout_output2.size() >= 4) {
				std::cout << "Last 4 chars: " << stdout_output2.substr(stdout_output2.size() - 4) << std::endl;
			}
			std::cout << "Stderr: " << (stderr_output2.empty() ? "(empty)" : stderr_output2) << std::endl;
			std::cout << "Exit status: " << exit_status2 << std::endl;
		}
		all_tests_passed = false;
	}

	/*
	// Test 3: Slow child (~60s)
	std::cout << "\nTest 3 - Slow Child (~60s)" << std::endl;
	std::string command3 = "./slow_child";
	std::string input3(1024 * 1024, 'x');
	size_t expected_size3 = input3.size() * 12;
	std::cout << "Command: " << command3 << "\nInput size: " << input3.size() << " bytes (child repeats 12 times)" << std::endl;

	std::string stdout_output3, stderr_output3;
	int exit_status3 = -1;
	bool success3 = run_piped_process_with_timeout(input3, stdout_output3, stderr_output3, exit_status3, 10, command3);
	std::string expected_output3_chunk(input3.size(), 'x');
	std::string expected_output3;
	for (int i = 0; i < 12; ++i) expected_output3 += expected_output3_chunk;
	if (success3 && stdout_output3.size() == expected_size3 && stdout_output3 == expected_output3 && stderr_output3.empty() && exit_status3 == 0) {
		std::cout << "Process completed successfully" << std::endl;
		std::cout << "Stdout size: " << stdout_output3.size() << " bytes" << std::endl;
		std::cout << "First 100 chars: " << stdout_output3.substr(0, 100) << "..." << std::endl;
		std::cout << "Stderr: (empty)" << std::endl;
		std::cout << "Exit status: " << exit_status3 << std::endl;
		std::cout << "Test passed" << std::endl;
	} else {
		std::cout << "Test failed" << std::endl;
		if (!success3) {
			std::cout << "Process failed or timed out" << std::endl;
		} else {
			std::cout << "Expected size: " << expected_size3 << ", Actual size: " << stdout_output3.size() << std::endl;
			std::cout << "First 100 chars: " << stdout_output3.substr(0, std::min<size_t>(100, stdout_output3.size())) << "..." << std::endl;
			std::cout << "Stderr: " << (stderr_output3.empty() ? "(empty)" : stderr_output3) << std::endl;
			std::cout << "Exit status: " << exit_status3 << std::endl;
		}
		all_tests_passed = false;
	}
	*/

	// Test 4: Large stderr output
	std::cout << "\nTest 4 - Large Stderr Output (1MB)" << std::endl;
	std::string command4 = "./large_stderr";
	std::string input4 = "";
	std::cout << "Command: " << command4 << "\nInput size: " << input4.size() << " bytes" << std::endl;

	std::string stdout_output4, stderr_output4;
	int exit_status4 = -1;
	bool success4 = run_piped_process_with_timeout(input4, stdout_output4, stderr_output4, exit_status4, 10, command4);
	std::string expected_stderr4_chunk = "Large stderr output line\n";
	std::string expected_stderr4;
	size_t target_size = 1024 * 1024;
	while (expected_stderr4.size() < target_size) {
		expected_stderr4 += expected_stderr4_chunk;
	}
	expected_stderr4 = expected_stderr4.substr(0, target_size);
	if (success4 && stdout_output4.empty() && stderr_output4 == expected_stderr4 && exit_status4 == 42) {
		std::cout << "Process completed successfully" << std::endl;
		std::cout << "Stdout size: " << stdout_output4.size() << " bytes" << std::endl;
		std::cout << "Stdout: " << (stdout_output4.empty() ? "(empty)" : stdout_output4) << std::endl;
		std::cout << "Stderr size: " << stderr_output4.size() << " bytes" << std::endl;
		std::cout << "Stderr first 100 chars: " << stderr_output4.substr(0, 100) << "..." << std::endl;
		std::cout << "Exit status: " << exit_status4 << std::endl;
		std::cout << "Test passed" << std::endl;
	} else {
		std::cout << "Test failed" << std::endl;
		if (!success4) {
			std::cout << "Process failed or timed out" << std::endl;
		} else {
			std::cout << "Expected stdout size: 0, Actual size: " << stdout_output4.size() << std::endl;
			std::cout << "Expected stderr size: " << expected_stderr4.size() << ", Actual size: " << stderr_output4.size() << std::endl;
			std::cout << "Stderr first 100 chars: " << stderr_output4.substr(0, std::min<size_t>(100, stderr_output4.size())) << "..." << std::endl;
			std::cout << "Expected exit status: 42, Actual: " << exit_status4 << std::endl;
		}
		all_tests_passed = false;
	}

	if (all_tests_passed) {
		std::cout << "\nAll tests passed!" << std::endl;
	} else {
		std::cout << "\nSome tests failed." << std::endl;
	}

	return all_tests_passed ? 0 : 1;
}

#endif

#if 0

// slowchild.c

#include <unistd.h>
#include <stdio.h>

int main() {
	char buffer[1048576]; // 1MB buffer
	ssize_t total_read = 0;
	ssize_t n;

	// Read up to 1MB from stdin
	while (total_read < sizeof(buffer) && (n = read(STDIN_FILENO, buffer + total_read, sizeof(buffer) - total_read)) > 0) {
		total_read += n;
	}

	// Write it 12 times with 5s delays
	for (int i = 0; i < 12; i++) {
		write(STDOUT_FILENO, buffer, total_read);
		sleep(5);
	}

	return 0;
}
#endif

#if 0

// large_stderr.c

#include <unistd.h>
#include <string.h>
#include <algorithm>

using namespace std;
int main() {
	const char* msg = "Large stderr output line\n";
	size_t msg_len = strlen(msg);
	size_t total_size = 1024 * 1024; // 1MB
	size_t written = 0;

	while (written < total_size) {
		size_t to_write = std::min(msg_len, total_size - written);
		write(STDERR_FILENO, msg, to_write);
		written += to_write;
	}

	return 42; // Custom exit status for testing
}

#endif