#if EXO_MODULE
	import std;
#else
#	include <unistd.h>
#	include <errno.h>
#	include <stdlib.h> // for EXIT_FAILURE
#	include <cstdio> // for stderr
#endif

#include <sys/inotify.h>
#include <poll.h>
#include <termios.h>

#include <exodus/cargs.h>
#include <exodus/var.h>

//similar code in haskey.cpp and varwait.cpp

#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"

namespace exo {

using let = const var;

/* Read all available inotify events from the file descriptor 'fd'.
  wd is the table of watch descriptors for the directories in argv.
  argc is the length of wd and argv.
  argv is the list of watched directories.
  Entry 0 of wd and argv is unused. */

//return true if any relevent events
static var handle_events(int inotify_fd, int* wd, const int argc, const char* argv[]) {
	/* Some systems cannot read integer variables if they are not
      properly aligned. On other systems, incorrect alignment may
      decrease performance. Hence, the buffer used for reading from
      the inotify file descriptor should have the same alignment as
      struct inotify_event. */

	char buf[4096]
		__attribute__((aligned(__alignof__(struct inotify_event))));
	//const struct inotify_event* event;
	const inotify_event* event;
	int i;
	ssize_t len;
	char* ptr;

	//printf("Handling events\n");

	///Loop while events can be read from inotify file descriptor.
	int eventn = 0;
	var events = "";
	do {

		// Read some events
		len = read(inotify_fd, buf, sizeof buf);
		if (len == -1 && errno != EAGAIN) {
			std::perror("varwait: read");
			std::exit(EXIT_FAILURE);
		}

		// If the nonblocking read() found no events to read, then
		// it returns -1 with errno set to EAGAIN. In that case,
		// we exit the loop.
		if (len <= 0)
			break;

		//printf("Loop over all events in the buffer\n");
		for (ptr = buf; ptr < buf + len;

			 //ptr += sizeof(struct inotify_event) + event->len) {
			 ptr += sizeof(inotify_event) + event->len) {

			eventn++;

			//event = (const struct inotify_event*)ptr;
			//event = reinterpret_cast<struct inotify_event*>(ptr);
			event = reinterpret_cast<inotify_event*>(ptr);

			//printf("Print event type\n");
			if (event->mask & IN_OPEN)
				events.updater(1, "IN_OPEN");
			else if (event->mask & IN_CLOSE_WRITE)
				events.updater(1, eventn, "IN_CLOSE_WRITE");
			else if (event->mask & IN_ACCESS)
				events.updater(1, eventn, "IN_ACCESS");  //Data was read from file.");
			else if (event->mask & IN_MODIFY)
				events.updater(1, eventn, "IN_MODIFY");  //Data was written to file.");
			else if (event->mask & IN_ATTRIB)
				events.updater(1, eventn, "IN_ATTRIB");  //File attributes changed.");
			else if (event->mask & IN_CLOSE)
				events.updater(1, eventn, "IN_CLOSE");  //File was closed (read or write).");
			else if (event->mask & IN_MOVED_FROM)
				events.updater(1, eventn, "IN_MOVED_FROM");  //File was moved away from watched directory.");
			else if (event->mask & IN_MOVED_TO)
				events.updater(1, eventn, "IN_MOVED_TO");	 //File was moved into watched directory.");
			else if (event->mask & IN_MOVE)
				events.updater(1, eventn, "IN_MOVE");	 //File was moved (in or out of directory).");
			else if (event->mask & IN_CREATE)
				events.updater(1, eventn, "IN_CREATE");  //A file was created in the directory.");
			else if (event->mask & IN_DELETE)
				events.updater(1, eventn, "IN_DELETE");  //A file was deleted from the directory.");
			else if (event->mask & IN_DELETE_SELF)
				events.updater(1, eventn, "IN_DELETE_SELF");	//Directory or file under observation was deleted.");
			else if (event->mask & IN_MOVE_SELF)
				events.updater(1, eventn, "IN_MOVE_SELF");  //Directory or file under observation was moved.");
			else
				continue;

			// Multivalued if multiple events caught
			// 1 Event type code
			// 2 dirpath
			// 3 filename
			// 4 d=dir, f=file

			// The name of the watched directory
			for (i = 1; i < argc; ++i) {
				if (wd[i] == event->wd) {
					events.updater(2, eventn, argv[i]);
					break;
				}
			}

			// The name of the file
			if (event->len)
				events.updater(3, eventn, event->name);

			//The type of filesystem object
			if (event->mask & IN_ISDIR)
				events.updater(4, eventn, "d");
			else
				events.updater(4, eventn, "f");
		}

		//if (events)
		//events.outputl("events=");

		return events;

	} while (false);

	return "";
}

//given a list of directories starting in argv[1] and a timeout in ms
//waits for any changes in files or a input line to become available
//
//returns a list of events in those directories
//or "1" if a key was pressed
static var wait_main(const int argc, const char* argv[], const int wait_time_ms) {

	//c style declaration at top
	//char buf;
	int inotify_fd, i, poll_num;
	int* wd;
	nfds_t nfds;
	struct pollfd fds[2];

	//check at least one item to watch
	// or crash
	if (argc < 2) {
		fprintf(stderr, "oswait must specify at least one directory or file\n");
		exit(EXIT_FAILURE);
	}

	//printf("Press any key to terminate.\n");

	// Create the file descriptor for accessing the inotify API
	// or crash
	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd == -1) {
		std::perror("varwait: inotify_init1");
		std::exit(EXIT_FAILURE);
	}

	//Allocate memory for watch descriptors
	// or crash
	wd = reinterpret_cast<int*>(calloc(argc, sizeof(int)));
	if (wd == nullptr) {
		std::perror("varwait: calloc");
		std::exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++) {

		//Mark directories and files watch
		//int inotify_rm_watch(int fd, int wd);
		wd[i] = inotify_add_watch(inotify_fd, argv[i],
								  //IN_OPEN | IN_CLOSE
								  IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE | IN_DELETE_SELF | IN_MOVE_SELF
								  //IN_ALL_EVENTS
		);
		// or crash
		if (wd[i] == -1) {
			fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
			std::perror("varwait: inotify_add_watch");
			std::exit(EXIT_FAILURE);
		}
	}

	nfds = 1;

	//do this before preparing the polling array
	//otherwise will fail with "too many inotify_init1"

	// Save stdin terminal attributes
	// Probably not available if running as a service
	struct termios oldtio, curtio;
	if (tcgetattr(STDIN_FILENO, &oldtio) < 0) {
		//EBADF - The filedes argument is not a valid file descriptor.
		//ENOTTY - The filedes is not associated with a terminal.
		//		var("no std input").outputl();
		//		return false;
	} else {
		nfds = 2;

		//Make sure we exit cleanly
		// memset(&sa, 0, sizeof(struct sigaction));
		// sa.sa_handler = sighandler;
		// sigaction(SIGINT, &sa, nullptr);
		// sigaction(SIGQUIT, &sa, nullptr);
		// sigaction(SIGTERM, &sa, nullptr);

		//This is needed to be able to tcsetattr() after a hangup (Ctrl-C)
		//see tcsetattr() on POSIX

		// memset(&sa, 0, sizeof(struct sigaction));
		// sa.sa_handler = SIG_IGN;
		// sigaction(SIGTTOU, &sa, nullptr);

		// Set stdin mode
		// a) non-canonical (i.e. characterwise not linewise input)
		// b) no-echo
		// https://man7.org/linux/man-pages/man3/termios.3.html
		//
		tcgetattr(STDIN_FILENO, &curtio);
		curtio.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &curtio);
	}

	/* Prepare for polling */

	/* Inotify input */
	fds[0].fd = inotify_fd;
	fds[0].events = POLLIN;

	// console input only if terminal available
	if (nfds == 2) {
		fds[1].fd = STDIN_FILENO;
		fds[1].events = POLLIN;
	}

	// Wait for events and/or terminal input
	// printf("Listening for events.\n");
	var events = "";

	for (;;) {

		//poll events and input until timeout or crash
		poll_num = poll(fds, nfds, wait_time_ms);
		if (poll_num == -1) {
			if (errno == EINTR) {
//				fprintf(stderr, "EINTR\n");
//				events = "C";
//				break;
				continue;
			}
			std::perror("varwait: poll");
			std::exit(EXIT_FAILURE);
		}

		if (poll_num > 0) {

			//inotify events occurred
			if (fds[0].revents & POLLIN) {

				/* Inotify events are available */
				//quit if any relevent events
				events = handle_events(inotify_fd, wd, argc, argv);
				if (events)
					break;
			}

			//input available
			if (nfds == 2 && fds[1].revents & POLLIN) {

				events = 1;
				/* Console input is available. Empty stdin and quit */

				//while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
				//    continue;
				break;
			}

			//timeout
		} else if (poll_num == 0) {
			//printf("Timeout listening for events.\n");
			break;
		}
	}

	//printf("Listening for events stopped.\n");

	//remove watches or crash (is this necessary since we close the fd next)
	for (i = 1; i < argc; i++) {
		if (inotify_rm_watch(inotify_fd, wd[i]) < 0) {
			std::perror("varwait: inotify_rm_watch");
			std::exit(EXIT_FAILURE);
		}
	}

	// Close inotify file descriptor
	close(inotify_fd);

	free(wd);

	// restore terminal attributes (probably linewise input and echo)
	if (nfds == 2) {
		tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
	}

	//exit(EXIT_SUCCESS);
	//events.outputl("oswait events=");
//TRACE(events);
	return events;
}

var  var::oswait(const int milliseconds) const {

	THISIS("var  var::oswait(const int milliseconds) const")
	assertString(function_sig);

	// Create a cstr structure from the FM list of files and or dirs provided.
	Cargs cargs(FM ^ var_str);

	return wait_main(cargs.argc(), cargs.argv(), milliseconds);
}

}  // namespace exo
