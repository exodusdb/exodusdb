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
		printl();
		printl("Testing simple integer for loops");
		printl("--------------------------------");

		int ncases = 18;
		for (int casen = 0; casen <= ncases; casen++) {

			int nn = 100'000'000;

			int i1 = 0;
//			double d1 = 0;
//			char c1 = '\0';
//			char* cp1 = nullptr;
			bool b1;
			var v1;
			std::string ss1 = "x";

			printl();
			var started = ostime();

			switch (casen) {

			break; case 0:
				printl("Exp: 45     ns - old var method - for (var;;)");
				nn = 10'000'000;
				for (var v2 = 0; v2 <= nn; v2++)
					{i1 = v2;};

			break; case 1:
				printl("Exp: 20     ns - new var method - for (var:range)");
				for (var v2 : range(0 to nn))
					{i1 = v2;};

			break; case 2:
				printl("Exp: 0.840 ns - old int method - for (int;;)");
				for (int i2 = 0; i2 <= nn; i2++)
					{i1 = i2;};

			break; case 3:
				printl("Exp: 0.840 ns - new int method - for (int:range)");
				for (int i2 : range(0 to nn))
					{i1 = i2;};

			break; case 4:
				printl("Exp: 0.840 ns - new int method using boost - for (int:range)");
				for (int i2 : boost::irange(0, nn))
					{i1 = i2;};

			break; case 5:
				printl("Exp: 5.5   ns - empty var + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var x; b1 = x.unassigned();};

			break; case 6:
				printl("Exp: 6.5   ns - construct from int + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var i = 123456; b1 = i.assigned();};

			break; case 7:
				printl("Exp: 6.5   ns - construct from bool + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var b = true; b1 = b.assigned();};

			break; case 8:
				printl("Exp: 6.5   ns - construct from double + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var d = 123.456; b1 = d.assigned();};

			break; case 9:
				printl("Exp: 11    ns - construct from cstr + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var s = "1"; b1 = s.assigned();};

			break; case 10:
				printl("Exp: 11    ns - construct from char + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var s = '1'; b1 = s.assigned();};

			break; case 11:
				printl("Exp: 11    ns - construct from std::string + test");
				for (int i2 = 0; i2 <= nn; i2++)
					{var s = ss1; b1 = s.assigned();};

			break; case 12:
				printl("Exp: 4     ns - pure test");
				for (int i2 = 0; i2 <= nn; i2++)
					{b1 = v1.assigned();};

			break; case 13:
				printl("Exp: 0.84  ns - assign from int");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = 123456;};

			break; case 14:
				printl("Exp: 1     ns - assign from bool");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = true;};

			break; case 15:
				printl("Exp: 1     ns - assign from double");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = 123.456;;};

			break; case 16:
				printl("Exp: 11    ns - assign from cstr");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = "1";};

			break; case 17:
				printl("Exp: 11    ns - assign from char");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = '1';};

			break; case 18:
				printl("Exp: 11    ns - assign from std::string");
				for (int i2 = 0; i2 <= nn; i2++)
					{v1 = ss1;};

			}
			var ended = ostime();
			if (i1 or b1 or true)
				printl("Act:", ((ended - started) / int(nn) * 1e9).round(3), "ns");
		}
	}

    printl();
	printl("Test passed");

    return 0;
}

programexit()

