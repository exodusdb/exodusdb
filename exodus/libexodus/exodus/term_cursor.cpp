#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <exodus/var.h>
#include <exoprog.h>

namespace exodus {


bool var::isterminal() const {
	return isatty(fileno(stdin));
}

var ExodusProgramBase::getcursor() const {

	//no cursor if stdin is not a terminal
	if (not var().isterminal())
		return "";

	//force out any previous cursor control characters
	var().osflush();

	// quit because hangs sometimes
	return AT(0,0);

	// Output
	/////////

	//save the current input config or quit (not terminal)
	struct termios save;
	if (tcgetattr(STDIN_FILENO, &save)) {
		perror("tcgetattr(save) error");
		return false;
	}
	std::clog << "got save" << std::endl;

	//switch input into raw io
	struct termios raw;
	cfmakeraw(&raw);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
		perror("tcsetattr(raw) error");
		return false;
	}

	//output magic request
	//char cmd[] = "echo -e '\033[6n'";
	char cmd[] = "\x1b[6n";
	int err = ::write(1, cmd, sizeof(cmd));

	//failure to write
	if (false && err) {
		std::cerr << "Error: " << err << " getcursor could not output" << std::endl;
		//restore the current input config
		tcsetattr(STDIN_FILENO, TCSANOW, &save);

		//restore the current output config
		//tcsetattr(STDOUT_FILENO, TCSANOW,&save1);

		return false;
	}

	// Input
	////////

	//input magic response
	char buf[16];
	err = ::read(STDIN_FILENO, buf, sizeof(buf));

	//restore the current input config
	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) != 0) {
		perror("tcsetattr(save) error");
		return false;
	}

	//restore the current output config
	//tcsetattr(STDOUT_FILENO, TCSANOW,&save1);

	//failure to read
	if (false && err) {
		std::cerr << "getcursor could not read response" << std::endl;
		return false;
	}

	// Parse response
	/////////////////

	//response could be ESC[54;10R for x=54, y=10
	var response(buf, sizeof(buf));
	var y = response.field(";", 1).cut(2);
	var x = response.field(";", 2).field("R");

	//x.outputl("x=");
	//y.outputl("y=");
	//x.oconv("HEX").outputl("x=");
	//y.oconv("HEX").outputl("y=");

	return AT(x, y);

}

void ExodusProgramBase::setcursor(CVR cursor) const {
	std::cout << cursor << std::flush;
	return;
}

var ExodusProgramBase::getprompt() const {
	std::cout << "ExodusProgramBase::getprompt() not implemented yet " << std::endl;
	return "";
}

void ExodusProgramBase::setprompt(CVR /*prompt*/) const {
	std::cout << "ExodusProgramBase::setprompt() not implemented yet " << std::endl;
	return;
}

}  // namespace exodus
