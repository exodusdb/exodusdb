#include <limits>
#include <iomanip>

#include <exodus/program.h>
#include <cassert>
using namespace std;

programinit()
function main() {
	printl("\n============================");
	{

		printl("\nASSUMING precision(14) in");
		printl("mv.cpp std::string dblToString(double double1)");

		print("\nMaximum precision is ");
		cout << numeric_limits<double>::digits10 + 1 << endl;

		cout<< "\nsize of double " << sizeof(double) << " size of long double" << sizeof(long double) << endl;

		//cout << setprecision(16) << fixed << 1234567890123456.789012345678901234567890 << endl;
		//cout << setprecision(16) << fixed << 999999999999999.9 << endl;

		printl("\ntoo decimal points get truncated");
		var d1 = 0.00005678901234567890;
		var d2 = "0.00005678901234567890";
		var s1 = d1^"x";
		var s2 = "0.00005678901235x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\ntoo many digits of precision get truncated");
		d1 = 1234567890.00005678;
		d2 = "1234567890.00005678";
		s1 = d1 ^ "x";
		s2 = "1234567890.0001x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\nsmall numbers are ok");
		d1 = 0.00005678;
		d2 = "0.00005678";
		s1 = d1^"x";
		s2 = "0.00005678x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\nsmallest decimal number");
		d1=0.0000000000001;
		d2="0.0000000000001";
		s1 = d1^"x";
		s2="0.0000000000001x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\nless than smallest decimal number becomes 0.0");
		d1=0.000000000000009;
		d2="0.000000000000009";
		s1 = d1^"x";
		s2="0.0x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\ncheck largest number (14x9) works without scientific format");
		d1= 9999999999999.9;
		d2="9999999999999.9";
		s1=d1^"x";
		s2="9999999999999.9x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

		printl("\ncheck excessive number convert to scientific");
		d1=99999999999999.9;
		d2="99999999999999.9";
		s1=d1^"x";
		s2="1e+14x";
		printl("Raw   : ", d2);
		printl("Print : ", d1);
		printl("Result: ", s1);
		printl("Target: ", s2);
		assert(s1 == s2);

	}

	return 0;
}

programexit()

