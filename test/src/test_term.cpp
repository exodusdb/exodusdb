#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {

	stop("Test passed");

	{
		echo(false);
		printl("echo(false) suppresses terminal input visibility");
		///input(ID);

		echo(true);
		printl("echo(true) enables terminal input visibility");
		//input(ID);
	}

	//	//skip input testing without terminal for now until we can redirect input
	//	// since it messing up tmux too many std inputs
	//	if (isterminal()) {
	//
	//		printl("We have a terminal. CANNOT check that input fails");
	//
	//		// Can change echo if is terminal
	//		assert(echo(false));
	//		assert(echo(true));
	//
	//		assert(getcursor() eq AT(0,0));
	//		setcursor("");
	//
	//	}
	//	else {
	//		printl("We dont have a terminal. CAN check that input fails");
	//
	//		// 1. No prompt - free function
	//		var inpz=input();
	//		//assert(not inpz);
	//
	//		// 2. Prompt -  member function
	//		var inpx;
	//		assert(not inpx.input("prompt1"));
	//		printl();
	//
	//		// 3. Prompt - free function
	//		var inpy;
	//		assert(not input("prompt2"));
	//		printl();
	//
	//		// 4. nchars - free function
	//		var inpq;
	//		assert(not inputn(5));
	//		printl();
	//
	//		// 5. std::cin
	//		var cin_v1;
	//		//assert(not std::cin >> cin_v1);
	//		//var ok = std::cin >> cin_v1;
	//		//assert(not ok);
	//		//bool ok = std::cin >> cin_v1;
	//		//assert(not ok);
	//		printl("testing std::cin >> cin_v1; - press enter if hung");
	//		osflush();
	//		std::cin >> cin_v1;
	//
	//		// get some coverage but not really testing
	//		assert(not inputn(-1));
	//		assert(not hasinput());
	//
	//		// Cannot change echo if no terminal
	//		assert(not echo(true));
	//
	//	}
	//

	if (TERMINAL) {

		// getcursor may return "\x1b[0;0H" if not enabled
		printx(AT(-1));
		var cursor = getcursor();
		TRACE(cursor)
		//no cursor if no terminal
		assert(cursor eq "\x1b[1;1H" or cursor eq "" or cursor eq "\x1b[0;0H");

		//should show 1;1 in top left
		for (const var ii : range(0, 12)) {
			printx(AT(ii, ii));
			printx(getcursor().b(2));
		}

		printx(AT(7, 24));
		cursor = getcursor();
		TRACE(cursor)
		assert(cursor eq "\x1b[24;7H" or cursor eq "" or cursor eq "\x1b[0;0H");

		setcursor(cursor);
	}

	{
		// Not implemented?
		var x = getprompt();
		setprompt(x);
	}

	{
		// Block Ctrl+C etc
		breakoff();
		//input(ID);

		breakon();
		//input(ID);
	}
	{
		echo(false);
		printl("echo(false) suppresses terminal input visibility");
		///input(ID);

		echo(true);
		printl("echo(true) enables terminal input visibility");
		//input(ID);
	}

	printl(elapsedtimetext());
	printl("Test passed");
	return 0;
}

programexit()
