#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

//derived from https://stackoverflow.com/questions/3962263/how-to-check-if-a-key-was-pressed-in-linux

namespace exodus
{

// static sig_atomic_t end = 0;

// static void sighandler(int signo)
//{
//	end = 1;
//}

bool haskey(void)
{
	struct termios oldtio, curtio;
	//	struct sigaction sa;

	/* Save stdin terminal attributes */
	/* Probably not available if running as a service */
	if (tcgetattr(0, &oldtio)<0) {
		//EBADF - The filedes argument is not a valid file descriptor.
		//ENOTTY - The filedes is not associated with a terminal.
		return false;
	}

	/* Make sure we exit cleanly */
	// memset(&sa, 0, sizeof(struct sigaction));
	// sa.sa_handler = sighandler;
	// sigaction(SIGINT, &sa, NULL);
	// sigaction(SIGQUIT, &sa, NULL);
	// sigaction(SIGTERM, &sa, NULL);

	/* This is needed to be able to tcsetattr() after a hangup (Ctrl-C)
	 * see tcsetattr() on POSIX
	 */
	// memset(&sa, 0, sizeof(struct sigaction));
	// sa.sa_handler = SIG_IGN;
	// sigaction(SIGTTOU, &sa, NULL);

	/* Set stdin mode
	a) non-canonical (i.e. characterwise not linewise input)
	b) no-echo
	https://man7.org/linux/man-pages/man3/termios.3.html
	*/
	tcgetattr(0, &curtio);
	curtio.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &curtio);

	/* main loop */
	// while (!end) {

	struct pollfd pfds[1];
	//char c;

	/* See if there is data available */
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	bool hasdata = poll(pfds, 1, 0) > 0;

	//}

	/* restore terminal attributes */
	tcsetattr(0, TCSANOW, &oldtio);

	return hasdata;
}

} // namespace exodus
