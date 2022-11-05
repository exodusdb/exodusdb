#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <boost/range/irange.hpp>
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {

	// Quit if running under make since there are no tests
	if (osgetenv("MAKELEVEL")) {
		printl();
		printl("Test passed - skipped because MAKELEVEL is set");
		return 0;
	}

	printl();

	int nn = 1'000'000;
	var setup_time;

	bool   the_truth_hurts = false;
	double some_dbl		   = 1.1;
	char   some_char	   = ' ';

	printl("Exp: 7 ns Time creation+destruction of unassigned var + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var().assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}

	printl();

	printl("Exp: 15  ns - Creation+destruction of string var '12345.67' + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var("12345.67").assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}
	printl();

	printl("Exp: 65 ns - Conversion of string var '12345.67' to double (Using fast_float::from_chars on Ubuntu 20.04/g++v9.3 and Ubuntu 22.04/g++v11.2)");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			some_dbl = var("12345.67").toDouble();
			//var("12345.67");
		}
		printl("Act:", round((ostime() - started - setup_time) / nn * 1E9), "ns");
	}

	printl();

	printl("Exp: 6 ns - Creation+destruction of double var 12345.67 + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var(12367).assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}
	printl();

	printl("Exp: 650  ns - Conversion of double var 12345.67 to string and return length (Using RYU D2S on Ubuntu 20.04/g++v9.3 and std::to_chars on Ubuntu 22.04/g++v11.2)");
	{
		var nn		= 100'000;
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			some_char = var(12345.67).toChar();
		}
		printl("Act:", round((ostime() - started - setup_time) / nn * 1E9), "ns");
	}

	// Test integer loops
	{
		printl();
		printl("Testing simple integer for loops");
		printl("--------------------------------");

		int firstcasen = COMMAND.f(2);
		if (not firstcasen)
			firstcasen = 0;

		int ncases = 20;

		for (int casen = firstcasen; casen le ncases; casen++) {

			printl();
			for (int repeatn = 0; repeatn lt 3; repeatn++) {

				int nn = 1'000'000;

				int i1 = 1;
				//			double d1 = 0;
				//			char c1 = '\0';
				//			char* cp1 = nullptr;
				bool		b1	= true;
				var			v1	= 1;
				std::string ss1 = "x";

				var started = ostime();

				switch (casen) {

					break;
					case 0:
						if (repeatn eq 0)
							printl("Exp: 35     ns - old var method - for (var;;)");
						nn = 1'000'000;
						for (var v2 = 0; v2 le nn; v2++) {
							i1 = v2;
						};

						break;
					case 1:
						if (repeatn eq 0)
							printl("Exp: 15     ns - new var method - for (var:range)");
						for (const var v2 : range(0 to nn)) {
							i1 = v2;
						};

						break;
					case 2:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - old int method - for (int;;)");
						for (int i2 = 0; i2 le nn; i2++) {
							i1 = i2;
						};

						break;
					case 3:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - new int method - for (int:range)");
						for (int i2 : range(0 to nn)) {
							i1 = i2;
						};

						break;
					case 4:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - new int method using boost - for (int:range)");
						for (int i2 : boost::irange(0, nn)) {
							i1 = i2;
						};

						break;
					case 5:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct empty var + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var x;
							b1 = x.assigned();
						};

						break;
					case 6:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from int + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var i = 123456;
							b1	  = i.assigned();
						};

						break;
					case 7:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from bool + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var b = true;
							b1	  = b.assigned();
						};

						break;
					case 8:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from double + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var d = 123.456;
							b1	  = d.assigned();
						};

						break;
					case 9:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from cstr + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = "1";
							b1	  = s.assigned();
						};

						break;
					case 10:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from char + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = '1';
							b1	  = s.assigned();
						};

						break;
					case 11:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from std::string + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = ss1;
							b1	  = s.assigned();
						};

						break;
					case 12:
						if (repeatn eq 0)
							printl("Exp: 4.5   ns - pure test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = v1.assigned();
						};

						break;
					case 13:
						if (repeatn eq 0)
							printl("Exp: 0.9   ns - assign from int");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = 123456;
						};

						break;
					case 14:
						if (repeatn eq 0)
							printl("Exp: 0.9   ns - assign from bool");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = true;
						};

						break;
					case 15:
						if (repeatn eq 0)
							printl("Exp: 1.3   ns - assign from double");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = 123.456;
						};

						break;
					case 16:
						if (repeatn eq 0)
							printl("Exp: 13    ns - assign from cstr");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = "1";
						};

						break;
					case 17:
						if (repeatn eq 0)
							printl("Exp: 11    ns - assign from char");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = '1';
						};

						break;
					case 18:
						if (repeatn eq 0)
							printl("Exp: 11    ns - assign from std::string");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = ss1;
						};

						break;
					case 19:
						if (repeatn eq 0)
							printl("Exp: 22    ns - var + int + test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = (v1 + i2).assigned();
						};

						break;
					case 20:
						if (repeatn eq 0)
							printl("Exp: 22    ns - var + 0.1 + test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = (v1 + 0.1).assigned();
						};
				}
				var ended = ostime();
				if (i1 or b1 or v1)
					printl("Act:", ((ended - started) / int(nn) * 1e9).round(3), "ns");
			}
		}
	}

	// Ensure optimiser doesnt see as unused
	printl(the_truth_hurts, some_dbl, some_char);

	printl();
	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
