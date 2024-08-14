#undef NDEBUG
#include <cassert>

#include <exodus/program.h>
//#pragma clangx diagnostic ignored "-Wunsafe-buffer-usage"
programinit()

function main() {

#if EXO_MODULE
#else
    logputl("Note: libexodus was not built with cmake ... -DEXO_TIMEBANK=1 so test_timebank will not collect or output any timings");
#endif


//	// Verify catch format error
//	try {
//		var x = 12.3456;
//		var fmt = "{.:.3f}";
//		assert(xvformat(fmt, x) == "12.346");
//		assert(false && fmt ^ " should throw a format error");
//	} catch (VarError e) {
//		errputl("Caught format error: " + e.description);
//	}

	var v1 = 12.3456;
	var v2;
	var n = 1'000;
	for (auto i [[maybe_unused]] : range(1, n)) {

		{

			var x = 12.3456;

			// Poor performance?
			//
			// var  var::format(SV fmt_str, Args&&... args) const
			//
			// 34'000 ns 1st op + 400 ns/op for 1'000 ops (exodus cached)
#if EXO_FORMAT
			assert(x.format("{:.2f}") == "12.35");
#endif

//			// auto fmt::formatter::format
//			//
//			// formatting a var using a var
//			var fmt = "{:.3f}";
//			// 6000 ns 1st op + 110 ns/op for 1'000 ops (exodus cached?)
//			assert(xvformat(fmt, x) == "12.346");

			// Compile time format is very fast even for the 1st one

			// TODO should be compile time
			// auto fmt::formatter::format(...) const
			// 4148 ns 1st op + 106 ns/op for 1000 ops
//			assert(xformat("{:.4f}", x) == "12.3456");
	continue;
//
//			assert(xvformat(fmt, 12.3456).outputl() == "12.35");
//			assert(x.vformat(fmt).outputl() == "12.35");
		}
#if 0
		var v3;

		v3 = var("abc").len();
		assert(v3 == 3);

		v3 = var("abc").oconv("T#10");
		assert(v3 == "abc       ");

		v3 = var(12.3456).oconv("MD20P");
		assert(v3 == 12.35);

		v3 = var("123").oconv("L#10");
		assert(v3 == "123       ");

		v3 = str(FM,5).oconv("TX");
		var v4;
		v4 = v3.iconv("TX");
		assert(v4 == str(FM,5));

		v3 = var("ab").oconv("HEX");
		assert(v3 == "6162");

		v3 = var("25/12/2024").iconv("D2/E");
		assert(v3 == "20814");

		v3 = var(20000).oconv("D2/E");
		assert(v3 == "03/10/22");

		v3 = var("abc").oconv("MRU");
		assert(v3 == "ABC");

		v3 = osgetenv("SHELL");
		assert(v3 == "/bin/bash");

		v3 = osgetenv("HOME");
		assert(v3 == "/root");

		v3 = var("23:59:59").iconv("MTHS");
		assert(v3 == 86399);

		assert(!str("xyz",1024).starts("qq"));
		assert(!str("xyz",1024).ends("qq"));
		assert(!str("xyz",1024).contains("qq"));

		v3 = var("23:59:59").contains("P");
		assert(v3 == 0);

		v3 = var(86399).oconv("MTS");
		assert(v3 == "23:59:59");

		v3 = var("asd").quote();
		assert(v3 == "\"asd\"");

		v3 = var("asd").squote();
		assert(v3 == "'asd'");

		v3 = var("'asd'").unquote();
		assert(v3 == "asd");


		v3 ="asd";
		v3 = v3.squote();
		assert(v3 == "'asd'");

		v3 = v3.quote();
		assert(v3 == "\"'asd'\"");

		v3 = v3.unquote();
		assert(v3 == "'asd'");

		v3 = v3.unquote();
		assert(v3 == "asd");


		{
			var v3, v4;
			int n = 10000;

			// "Do nothing" ucase is 25% faster than "do nothing" lcase?

	//		v3 = str("abcdefghij", n);
			v3 = str("ABCDEFGHIJ", n);
			v3.ucaser();

	//		v3 = str("ABCDEFGHIJ", n);
			v3 = str("abcdefghij", n);
			v3.lcaser();
	//		TRACE(v3)

		}

//		printl(oconv(-20, "MX"));

		{
			int n = 10000;
			var v3b;
			v3b = str("abcdefghij", n);
			assert(v3b.lcase() == v3b);
			v3b = str("ABCDEFGHIJ", n);
			assert(v3b.ucase() == v3b);

			v3b = str("abcdefghij", n);
			assert(v3b.ucase() != v3b);

			v3b = str("ABCDEFGHIJ", n);
			assert(v3b.lcase() != v3b);
//			assert(v3b == str("ABCDEFGHIJ", n));
//			assert(v3b == str("abcdefghij", n));
//			assert(v3b == str("", 1000));
		}
#endif

	} // for loop

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
