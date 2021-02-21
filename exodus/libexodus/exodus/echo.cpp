#include <unistd.h>
#include <termios.h>

#include <mv.h>

namespace exodus {

//echo on=1 off=0
bool var::echo(const int on_off) const
{

	// Probably not available if running as a service or in a pipe
	struct termios curtio;
	if (tcgetattr(STDIN_FILENO, &curtio) < 0) {
		//EBADF - The filedes argument is not a valid file descriptor.
		//ENOTTY - The filedes is not associated with a terminal.
		return false;
	}

	// Set stdin mode
	// a) non-canonical (i.e. characterwise not linewise input)
	// b) no-echo
	// https://man7.org/linux/man-pages/man3/termios.3.html
	//
	//curtio.c_lflag &= ~(ICANON | ECHO);
	if (on_off == 1)
		curtio.c_lflag |= ECHO;
	else
		curtio.c_lflag ^= ECHO;

	tcsetattr(STDIN_FILENO, TCSANOW, &curtio);

	return true;
}

}//namespace
