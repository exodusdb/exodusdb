#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <functional>
#include <limits>

#include <exodus/program.h>

programinit()

	function main() {

	printl("test_more3 says 'Hello World!'");

	{
		using mvint_t = long long;
		var minint	  = std::numeric_limits<mvint_t>::min();
		var maxint	  = std::numeric_limits<mvint_t>::max();

		var unass;
		try {
			var v = unass;
			assert(false && "copy constructor from unassigned lvalue");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v = unass;
			assert(false && "copy assign from unassigned lvalue");
		} catch (VarUnassigned&) {
		}

		//move ALLOWS use of unassigned without throwing for performance since temporaries are unlikeley to be unassigned
		//		try {var v = var()        ;assert(false && "move constructor from unassigned rvalue");}    catch (VarUnassigned& e){};
		//		try {var v;v = var()      ;assert(false && "move assign from unassigned rvalue");}    catch (VarUnassigned& e){};
		try {
			var v = var();
		} catch (VarUnassigned&) {
			assert(false && "move constructor from unassigned rvalue");
		}
		try {
			var v;
			v = var();
		} catch (VarUnassigned&) {
			assert(false && "move assign from unassigned rvalue");
		}

		var uncaught = "";

		try {
			var v	 = var("x") + 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		// Divide by zero

		try {
			var v	 = 1;
			v		 = v / var(0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / 0;
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / "0";
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / 0.0;
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / "0.0";
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}

		try {
			var v = 1;
			v %= var(0.0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}
		try {
			var v = 1;
			v %= var(0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero&) {
		}

		// Unitary minus

		try {
			var v;
			v		 = -v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v	 = "x";
			v		 = -v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		// Increment/Decrement

		try {
			var v;
			v++;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v--;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			++v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			--v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v++;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v--;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			++v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			--v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		// Self assign

		try {
			var v;
			v += 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		try {
			var v;
			v += 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		try {
			var v;
			v += "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric&) {
		}

		// Overflow on inc/dec

		try {
			var x	 = "xyz"_var.locateby("XX", "xyz", MV);
			uncaught = "VarError locateby";
		} catch (VarError& e) {
			printl(e.description);
		}

		try {
			throw VarError("test");
			//std::unreachable();
			//uncaught = "VarError";
		} catch (VarError& e) {
			printl(e.description);
		}

		try {
			--minint;
			uncaught = "VarNumUnderflow";
		} catch (VarNumUnderflow& e) {
			printl(e.description);
		}
		try {
			++maxint;
			uncaught = "VarNumUnderflow";
		} catch (VarNumOverflow& e) {
			printl(e.description);
		}
		try {
			minint--;
			uncaught = "VarNumUnderflow";
		} catch (VarNumUnderflow& e) {
			printl(e.description);
		}
		try {
			maxint++;
			uncaught = "VarNumUnderflow";
		} catch (VarNumOverflow& e) {
			printl(e.description);
		}

		try {
			throw VarOutOfMemory("Error: x");
		} catch (VarOutOfMemory& e) {
			printl(e.description);
		}
		try {
			throw VarInvalidPointer("Error: x");
		} catch (VarInvalidPointer& e) {
			printl(e.description);
		}

			// We cannot test this because the output of the word "Exception" triggers testing to fail
			// ***Failed  Error regular expression found in output. Regex=[(Exception|Test failed)]  9.78 sec
			//try { throw VarDBException("Error: x"); } catch (VarDBException& e){};

#ifdef VAR_SAFE_DESTRUCTOR
		try {
			var v	 = v + 1;
			uncaught = "VarUndefined";
		} catch (VarUndefined& e) {
			printl(e.description);
		} catch (VarUnassigned& e) {
			printl(e.description);
		}
#endif
		//unfortunately throw causes cmake test to fail so we skip them
		//try {throw VarDBException("test");uncaught="VarDBException";}  catch (VarDBException& e){};
		//try {throw VarDebug("test");uncaught="VarDebug";}              catch (VarDebug& e){};

		try {
			var v	 = var("x").oconv("QQQ");
			uncaught = "VarOconvNotImplemented";
		} catch (VarNotImplemented& e) {
			printl(e.description);
		}

		// dim

		//try {dim x(0)             ;uncaught="DimDimensionedZero";} catch (DimDimensionedZero& e){};
		try {
			dim x(10);
			printl(x(11));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds& e) {
			printl(e.description);
		}
		try {
			dim x(10);
			printl(x(1, 2));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds& e) {
			printl(e.description);
		}
		try {
			const dim x(10);
			printl(x(11));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds& e) {
			printl(e.description);
		}
		try {
			const dim x(10);
			printl(x(1, 2));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds& e) {
			printl(e.description);
		}

		try {
			dim x;
			var y	 = x.join();
			uncaught = "DimNotDimensioned 1";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}
		try {
			dim x;
			dim y	 = x;
			uncaught = "DimNotDimensioned 2";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}
		try {
			dim x;
			x		 = dim();
			uncaught = "DimNotDimensioned 3";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}
		try {
			dim x;
			var y	 = x.cols();
			uncaught = "DimNotDimensioned 4";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}
		try {
			dim x;
			var y	 = x.rows();
			uncaught = "DimNotDimensioned 5";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}
		try {
			dim x;
			x		 = "q";
			uncaught = "DimNotDimensioned 5";
		} catch (DimNotDimensioned& e) {
			printl(e.description);
		}

		try {
			stop();
			uncaught = "MVStop";
		} catch (MVStop& e) {
			printl("Caught MVStop", e.description);
		}
		try {
			abort(99);
			uncaught = "MVAbort";
		} catch (MVAbort& e) {
			printl("Caught MVAbort", e.description);
		}
		try {
			abortall();
			uncaught = "MVAbortAll";
		} catch (MVAbortAll& e) {
			printl("Caught MVAbortAll", e.description);
		}
		try {
			logoff();
			uncaught = "MVLogoff";
		} catch (MVLogoff& e) {
			printl("Caught MVlogoff", e.description);
		}

		if (uncaught) {
			TRACE(uncaught)
			assert(uncaught eq "");
		}
		//	try {throw Var("test");} catch (Var& e){};
	}

	{
		var backtracex = backtrace();
		TRACE(backtracex)
		// Not available if built without -g option for including debugging info
		//assert(bt.match(R"__(\d+:\s+test_except.cpp:\d+: var bt = backtrace\(\);)__", ""));
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
