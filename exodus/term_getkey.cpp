// https://stackoverflow.com/questions/14322430/how-to-do-console-input-like-in-the-top-linux-command
//#if EXO_MODULE
//	import std;
//#endif
#include <cstdio>		 //fileno(), stdin
#include <unistd.h>		 //read()
#include <fcntl.h>		 //fcntl()
#include <sys/select.h>	 //select()
#include <termios.h>	 //termios
//#include <sys/ioctl.h>
//#include <sys/time.h>
//#include <sys/types.h>

//------------------------------------------------------------------------------
// term_getkey() returns the next char in the stdin buffer if available, otherwise
//          it returns -1 immediately.
//
// it turns ECHO off and turns ECHO ON afterwards
//

namespace exo {

char term_getkey(void) {
	char ch = 0;
	int error;
	struct termios oldAttr, newAttr;
	int oldFlags;
	//int newFlags;
	struct timeval tv;
	int fd = fileno(stdin);

	tcgetattr(fd, &oldAttr);
	newAttr = oldAttr;
	oldFlags = fcntl(fd, F_GETFL, 0);

	newAttr.c_iflag = 0;		/* input mode */
	newAttr.c_oflag = 0;		/* output mode */
	newAttr.c_lflag &= ~ICANON; /* line settings */
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	newAttr.c_cc[VMIN] = 1;		/* minimum chars to wait for */
	newAttr.c_cc[VTIME] = 1;	/* minimum wait time */
#pragma GCC diagnostic pop

	// Set stdin to nonblocking, noncanonical input
	fcntl(fd, F_SETFL, O_NONBLOCK);
	error = tcsetattr(fd, TCSANOW, &newAttr);

	tv.tv_sec = 0;
	tv.tv_usec = 10000;	 // small 0.01 msec delay
	select(1, nullptr, nullptr, nullptr, &tv);

	if (error == 0)
		error = (read(fd, &ch, 1) != 1);  // get char from stdin

	// Restore original settings
	error |= tcsetattr(fd, TCSANOW, &oldAttr);
	fcntl(fd, F_SETFL, oldFlags);

	return (error ? -1 : ch);
}

/*
int main()
{
    int c,n=0;
    printf("Hello, world!\nPress any key to exit. I'll wait for 4 keypresses.\n\n");
    while (n<4)
    {
	//printf("."); // uncomment this to print a dot on each loop iteration
	c = getkey();
	if (c >= 0)
	{
	    printf("You pressed '%c'\n", c);
	    ++n;
	}
    }
}
*/

}  // namespace exo
