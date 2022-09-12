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
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v = unass;
			assert(false && "copy assign from unassigned lvalue");
		} catch (VarUnassigned e) {
		};

		//move ALLOWS use of unassigned without throwing for performance since temporaries are unlikeley to be unassigned
		//		try {var v = var()        ;assert(false && "move constructor from unassigned rvalue");}    catch (VarUnassigned e){};
		//		try {var v;v = var()      ;assert(false && "move assign from unassigned rvalue");}    catch (VarUnassigned e){};
		try {
			var v = var();
		} catch (VarUnassigned e) {
			assert(false && "move constructor from unassigned rvalue");
		};
		try {
			var v;
			v = var();
		} catch (VarUnassigned e) {
			assert(false && "move assign from unassigned rvalue");
		};

		var uncaught = "";

		try {
			var v	 = var("x") + 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		// Divide by zero

		try {
			var v	 = 1;
			v		 = v / var(0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};
		try {
			var v	 = 1;
			v		 = v / 0;
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};
		try {
			var v	 = 1;
			v		 = v / "0";
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};
		try {
			var v	 = 1;
			v		 = v / 0.0;
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};
		try {
			var v	 = 1;
			v		 = v / "0.0";
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};

		try {
			var v = 1;
			v %= var(0.0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};
		try {
			var v = 1;
			v %= var(0);
			uncaught = "VarDivideByZero";
		} catch (VarDivideByZero e) {
		};

		// Unitary minus

		try {
			var v;
			v		 = -v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v	 = "x";
			v		 = -v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		// Increment/Decrement

		try {
			var v;
			v++;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v--;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			++v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			--v;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};

		try {
			var v = "x";
			v++;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v--;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			++v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			--v;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		// Self assign

		try {
			var v;
			v += 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v -= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v *= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v /= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v %= 1.1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};

		try {
			var v = "x";
			v += 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v -= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v *= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v /= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v %= 1.1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		try {
			var v;
			v += 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v -= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v *= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v /= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v %= 1;
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};

		try {
			var v = "x";
			v += 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v -= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v *= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v /= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v %= 1;
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		try {
			var v;
			v += "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v -= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v *= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v /= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};
		try {
			var v;
			v %= "1";
			uncaught = "VarUnassigned";
		} catch (VarUnassigned e) {
		};

		try {
			var v = "x";
			v += "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v -= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v *= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v /= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};
		try {
			var v = "x";
			v %= "1";
			uncaught = "VarNonNumeric";
		} catch (VarNonNumeric e) {
		};

		// Overflow on inc/dec

		try {
			var x	 = "xyz"_var.locateby("XX", "xyz", MV);
			uncaught = "VarError locateby";
		} catch (VarError e) {
		};

		try {
			throw VarError("test");
			uncaught = "VarError";
		} catch (VarError e) {
		};

		try {
			--minint;
			uncaught = "VarIntUnderflow";
		} catch (VarIntUnderflow e) {
		};
		try {
			++maxint;
			uncaught = "VarIntUnderflow";
		} catch (VarIntOverflow e) {
		};
		try {
			minint--;
			uncaught = "VarIntUnderflow";
		} catch (VarIntUnderflow e) {
		};
		try {
			maxint++;
			uncaught = "VarIntUnderflow";
		} catch (VarIntOverflow e) {
		};

		try {
			throw VarOutOfMemory("Error: x");
		} catch (VarOutOfMemory e) {
		};
		try {
			throw VarInvalidPointer("Error: x");
		} catch (VarInvalidPointer e) {
		};

			// We cannot test this because the output of the word "Exception" triggers testing to fail
			//***Failed  Error regular expression found in output. Regex=[(Exception|Test failed)]  9.78 sec
			//try { throw VarDBException("Error: x"); } catch (VarDBException e){};

#ifdef VAR_SAFE_DESTRUCTOR
		try {
			var v	 = v + 1;
			uncaught = "VarUndefined";
		} catch (VarUndefined e) {
		} catch (VarUnassigned e) {
		};
#endif
		//unfortunately throw causes cmake test to fail so we skip them
		//try {throw VarDBException("test");uncaught="VarDBException";}  catch (VarDBException e){};
		//try {throw VarDebug("test");uncaught="VarDebug";}              catch (VarDebug e){};

		try {
			var v	 = var("x").oconv("QQQ");
			uncaught = "VarNotImplemented";
		} catch (VarNotImplemented e) {
		};

		// dim

		//try {dim x(0)             ;uncaught="DimDimensionedZero";} catch (DimDimensionedZero e){};
		try {
			dim x(10);
			printl(x(11));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds e) {
		};
		try {
			dim x(10);
			printl(x(1, 2));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds e) {
		};
		try {
			const dim x(10);
			printl(x(11));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds e) {
		};
		try {
			const dim x(10);
			printl(x(1, 2));
			uncaught = "DimIndexOutofBounds";
		} catch (DimIndexOutOfBounds e) {
		};

		try {
			dim x;
			var y	 = x.join();
			uncaught = "DimNotDimensioned 1";
		} catch (DimNotDimensioned e) {
		};
		try {
			dim x;
			dim y	 = x;
			uncaught = "DimNotDimensioned 2";
		} catch (DimNotDimensioned e) {
		};
		try {
			dim x;
			x		 = dim();
			uncaught = "DimNotDimensioned 3";
		} catch (DimNotDimensioned e) {
		};
		try {
			dim x;
			var y	 = x.cols();
			uncaught = "DimNotDimensioned 4";
		} catch (DimNotDimensioned e) {
		};
		try {
			dim x;
			var y	 = x.rows();
			uncaught = "DimNotDimensioned 5";
		} catch (DimNotDimensioned e) {
		};
		try {
			dim x;
			x		 = "q";
			uncaught = "DimNotDimensioned 5";
		} catch (DimNotDimensioned e) {
		};

		try {
			stop();
			uncaught = "MVStop";
		} catch (MVStop& e) {
			printl("Caught MVStop");
		};
		try {
			abort(99);
			uncaught = "MVAbort";
		} catch (MVAbort e) {
			printl("Caught MVAbort");
		};
		try {
			abortall();
			uncaught = "MVAbortAll";
		} catch (MVAbortAll e) {
			printl("Caught MVAbortAll");
		};
		try {
			logoff();
			uncaught = "MVLogoff";
		} catch (MVLogoff e) {
			printl("Caught MVlogoff");
		};

		if (uncaught) {
			printl(uncaught);
			assert(uncaught eq "");
		}
		//	try {throw Var("test");} catch (Var e){};
	}

	{
		var bt = backtrace();
		printl(bt);
		assert(bt.match(R"__(\d+:\s+test_except.cpp:\d+: var bt = backtrace\(\);)__", ""));
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
