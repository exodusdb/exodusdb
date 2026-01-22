#include <termios.h>
#include <unistd.h>    // isatty + STD*_FILENO constants

#include <var/var.h>

namespace exo {

bool var_os::isterminal(const int in_out_err) const {

	// 0 stdin, 1 stdout, 2 stderr/stdlog
	int filen;
	switch (in_out_err) {
		case 0: filen = STDIN_FILENO; break;
		case 1: filen = STDOUT_FILENO; break;
		case 2: filen = STDERR_FILENO; break;
		default: filen = in_out_err;
	}

	return isatty(filen);
}

//echo on/off true/false
bool var_os::echo(const bool on_off) const {

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
	if (on_off)
		curtio.c_lflag |= ECHO;
	else
		curtio.c_lflag ^= ECHO;

	tcsetattr(STDIN_FILENO, TCSANOW, &curtio);

	// Optional
	if (tcsetattr(STDIN_FILENO, TCSANOW, &curtio) < 0) return false;

	return true;
}

}  // namespace exo

