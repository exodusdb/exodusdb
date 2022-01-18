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

    printl();
	printl("Test passed");

    return 0;
}

programexit()

