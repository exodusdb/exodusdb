#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("test says 'Hello World!'");

	{
		printl("Test auto [a, b, c] = dimx.unpack<N>()");

		dim d1 = {1,2,3};
		logputl(d1.join());
		{
			printl("Unpack 1");
			auto [a] = d1.unpack<1>();
			assert(a == 1);
		}
		{
			printl("Unpack 2");
			auto [a,b] = d1.unpack<2>();
			assert(a == 1 && b == 2);
		}
		{
			printl("Unpack 3");
			auto [a, b, c] = d1.unpack<3>();
			assert(a == 1 && b == 2 && c == 3);
		}
		{
			printl("Unpack 4");
			auto [a, b, c, d] = d1.unpack<4>();
			assert(a == 1 && b == 2 && c == 3 && d == "");
		}
		{
			printl("Unpack 5");
			auto [a, b, c, d, e] = d1.unpack<5>();
			assert(a == 1 && b == 2 && c == 3 && d == "" && e == "");
		}
	}
	{
		printl("Test auto [a, b, c] = varx.unpack<N>()");

		var v1 = {1,2,3};
		logputl(v1);
		assert(v1 = "1^2^3"_var);
		{
			printl("Unpack 1");
			auto [a] = v1.unpack<1>();
			assert(a == 1);
		}
		{
			printl("Unpack 2");
			auto [a,b] = v1.unpack<2>();
			assert(a == 1 && b == 2);
		}
		{
			printl("Unpack 3");
			auto [a, b, c] = v1.unpack<3>();
			assert(a == 1 && b == 2 && c == 3);
		}
		{
			printl("Unpack 4");
			auto [a, b, c, d] = v1.unpack<4>();
			assert(a == 1 && b == 2 && c == 3 && d == "");
		}
		{
			printl("Unpack 5");
			auto [a, b, c, d, e] = v1.unpack<5>();
			assert(a == 1 && b == 2 && c == 3 && d == "" && e == "");
		}
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
}; // programexit()
