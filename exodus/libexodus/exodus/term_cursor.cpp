#include <iostream>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

#if EXO_MODULE
	import exoprog;
#else
#	include <exodus/exoprog.h>
#endif

void getCursorPos(int& x, int& y) {
    // Disable terminal echoing to avoid cluttering output
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings

    // Send the ANSI escape sequence to query cursor position
    std::cout << "\033[6n" << std::flush;

    // Read the response from the terminal
    char buf[32];
    int i = 0;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break; // Stop at the 'R' in the response
        i++;
    }
    buf[i] = '\0'; // Null-terminate the string

    // Parse the response (e.g., "\033[12;34R" -> row 12, col 34)
    int row, col;
    if (sscanf(buf, "\033[%d;%dR", &row, &col) == 2) {
        x = col - 1; // Convert to 0-based column
        y = row - 1; // Convert to 0-based row
    } else {
        x = -1; // Error case
        y = -1;
    }

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


namespace exo {

var ExoProgram::getcursor() const {

	//no cursor if stdin is not a terminal
	if (not var().isterminal())
		return "";

	//force out any previous cursor control characters
	var().osflush();

	int x, y;
    getCursorPos(x, y);

	return x ^ FM ^ y;

}

void ExoProgram::setcursor(in cursor) const {
	//std::cout << cursor << std::flush;
	AT(cursor.f(1), cursor.f(2)).output();
	cursor.osflush();
	return;
}

}  // namespace exo
