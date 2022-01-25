#include <boost/range/irange.hpp>
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
    printl();

    int nn = 1000000;
    var setup_time;

	printl(R"V0G0N(
============================================
WAS THE FOLLOWING ON UBUNTU 20.04
Intel(R) Xeon(R) CPU E5620  @ 2.40GHz (2010)
9 ns construct unassigned var in int loop

49 ns construct+destruct var from const char* '12345.67' in int loop
69 ns convert var string '12345.67' to double

9 ns construct+destruct var from double 12345.67 in int loop
529 ns convert var double 12345.67 to string
============================================
)V0G0N");

	printl(R"V0G0N(
============================================
WAS THE FOLLOWING ON UBUNTU 22.04
Intel(R) Xeon(R) CPU E5620  @ 2.40GHz (2010)
9 ns construct unassigned var in int loop

20 ns construct+destruct var from const char* '12345.67' in int loop
58 ns convert var string '12345.67' to double

8 ns construct+destruct var from double 12345.67 in int loop
215 ns convert var double 12345.67 to string
============================================
)V0G0N");

    var test = "Time creation+destruction of unassigned var within int loop";
    {
        var started = ostime();
        for (int ii=0;ii < nn; ii++) {
           var();
        }
        setup_time = ostime() - started;
        printl(round((setup_time) / nn * 1E9), "ns", test);
    }

    printl();

    test = "Time creation+destruction of string var '12345.67' within int loop";
    {
        var started = ostime();
        for (int ii=0;ii < nn; ii++) {
           var("12345.67");
        }
        setup_time = ostime() - started;
        printl(round((setup_time) / nn * 1E9), "ns", test);
    }

    test = "Time conversion of string var '12345.67' to double (Using fast_float::from_chars on Ubuntu 20.04/g++v9.3 and Ubuntu 22.04/g++v11.2)";
    {
        var started = ostime();
        for (int ii=0;ii < nn; ii++) {
           //var("12345.67").toDouble();
           var("12345.67").isnum();
        }
        printl(round((ostime() - started - setup_time) / nn * 1E9), "ns", test);
    }

    printl();

    test = "Time creation+destruction of double var 12345.67 within int loop";
    {
        var started = ostime();
        for (int ii=0;ii < nn; ii++) {
           var(12345.67);
        }
        setup_time = ostime() - started;
        printl(round((setup_time) / nn * 1E9), "ns", test);
    }

    test = "Time conversion of double var 12345.67 to string and return length (Using RYU D2S on Ubuntu 20.04/g++v9.3 and std::to_chars on Ubuntu 22.04/g++v11.2)";
    {
        var started = ostime();
        for (int ii=0;ii < nn; ii++) {
           var(12345.67).length();
        }
        printl(round((ostime() - started - setup_time) / nn * 1E9), "ns", test);
    }


	// Test integer loops
	{
		int nn = 100'000'000;

		printl();
		printl("Testing simple integer for loops");
		printl("--------------------------------");

		for (int s = 0; s <= 4; s++) {

			int i1 = 0;
			printl();
			var started = ostime();

			switch (s) {

			break; case 0:
				printl("Exp: 45ns - old var method - for (var;;)");
				for (var i2 = 0; i2 <= nn; i2++)
					{i1 = i2;};

			break; case 1:
				printl("Exp: 20ns - new var method - for (var:range)");
				for (var i2 : range(0 to nn))
					{i1 = i2;};

			break; case 2:
				printl("Exp: 0.84ns - old int method - for (int;;)");
				for (int i2 = 0; i2 <= nn; i2++)
					{i1 = i2;};

			break; case 3:
				printl("Exp: 0.84ns - new int method - for (int:range)");
				for (int i2 : range(0 to nn))
					{i1 = i2;};

			break; case 4:
				printl("Exp: 0.84ns - new int method using boost - for (int:range)");
				for (int i2 : boost::irange(0, nn))
					{i1 = i2;};
			}
			var ended = ostime();
			if (i1)
				printl("Act:", ((ended - started) / int(nn) * 1e9).round(3), "ns");
		}
	}

    printl();
	printl("Test passed");

    return 0;
}

programexit()

