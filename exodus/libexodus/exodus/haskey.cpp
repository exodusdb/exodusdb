#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace exodus {

//static sig_atomic_t end = 0;

//static void sighandler(int signo)
//{
//	end = 1;
//}

bool haskey(void)
{
	struct termios oldtio, curtio;
//	struct sigaction sa;

	/* Save stdin terminal attributes */
	tcgetattr(0, &oldtio);

	/* Make sure we exit cleanly */
	//memset(&sa, 0, sizeof(struct sigaction));
	//sa.sa_handler = sighandler;
	//sigaction(SIGINT, &sa, NULL);
	//sigaction(SIGQUIT, &sa, NULL);
	//sigaction(SIGTERM, &sa, NULL);

	/* This is needed to be able to tcsetattr() after a hangup (Ctrl-C)
	* see tcsetattr() on POSIX
	*/
	//memset(&sa, 0, sizeof(struct sigaction));
	//sa.sa_handler = SIG_IGN;
	//sigaction(SIGTTOU, &sa, NULL);

	/* Set non-canonical no-echo for stdin */
	tcgetattr(0, &curtio);
	curtio.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &curtio);

	/* main loop */
	//while (!end) {

	struct pollfd pfds[1];
	int ret;
	char c;

	/* See if there is data available */
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	ret = poll(pfds, 1, 0);

	/* Consume data */
	bool hasdata;
	if (ret > 0) {
		//printf("Data available\n");
		//read(0, &c, 1);
		hasdata=true;
	} else {
		//printf("Data NOT available\n");
		hasdata=false;
	}

	//}

	/* restore terminal attributes */
	tcsetattr(0, TCSANOW, &oldtio);

	return hasdata;
}

}//namespace exodus
