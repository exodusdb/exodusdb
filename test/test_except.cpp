#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#if EXO_MODULE
	import std;
#else
#	include <functional>
#	include <limits>
#endif

#include <exodus/program.h>
programinit()

func main() {

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

		//var uncaught = "";

		try {
			var v	 = var("x") + 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		// Divide by zero

		try {
			var v	 = 1;
			v		 = v / var(0);
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / 0;
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / "0";
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / 0.0;
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}
		try {
			var v	 = 1;
			v		 = v / "0.0";
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}

		try {
			var v = 1;
			v %= var(0.0);
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}
		try {
			var v = 1;
			v %= var(0);
			throw VarError("Uncaught error VarDivideByZero");
		} catch (VarDivideByZero&) {
		}

		// Unitary minus

		try {
			var v;
			v		 = -v;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v	 = "x";
			v		 = -v;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		// Increment/Decrement

		try {
			var v;
			v++;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v--;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			++v;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			--v;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v++;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v--;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			++v;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			--v;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		// Self assign

		try {
			var v;
			v += 1.1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= 1.1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= 1.1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= 1.1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= 1.1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += 1.1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= 1.1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= 1.1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= 1.1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= 1.1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		try {
			var v;
			v += 1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= 1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= 1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= 1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= 1;
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= 1;
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		try {
			var v;
			v += "1";
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v -= "1";
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v *= "1";
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v /= "1";
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}
		try {
			var v;
			v %= "1";
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned&) {
		}

		try {
			var v = "x";
			v += "1";
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v -= "1";
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v *= "1";
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v /= "1";
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}
		try {
			var v = "x";
			v %= "1";
			throw VarError("Uncaught error VarNonNumeric");
		} catch (VarNonNumeric&) {
		}

		// Overflow on conversion from signed
		try {
			printl("std::numeric_limits<unsigned long long>::max() = ", std::numeric_limits<unsigned long long>::max());
//			assert(![] () {var x(std::numeric_limits<unsigned long long>::max());return x;}());
			var x = std::numeric_limits<unsigned long long>::max();
			throw VarError("Uncaught error VarNumOverflow unsigned long long");
		} catch (VarNumOverflow& e) {
			printl(e.message);
		}

//		// Underflow on conversion from signed
//		try {
//			printl("std::numeric_limits<signed long long>::min() = ", std::numeric_limits<signed long long>::min());
////			assert(![] () {var x(std::numeric_limits<unsigned long long>::max());return x;}());
//			var x = std::numeric_limits<signed long long>::min();
//			throw VarError("Uncaught error VarNumUnderflow unsigned long long");
//		} catch (VarNumOverflow& e) {
//			printl(e.message);
//		}

		// Overflow on inc/dec

		try {
			var x	 = "xyz"_var.locateby("XX", "xyz", MV);
			throw VarError("Uncaught error VarError locateby");
		} catch (VarError& e) {
			printl(e.message);
		}

		try {
			throw VarError("test");
			//std::unreachable();
			//throw VarError("Uncaught error VarError");
		} catch (VarError& e) {
			printl(e.message);
		}

		try {
			--minint;
			throw VarError("Uncaught error VarNumUnderflow");
		} catch (VarNumUnderflow& e) {
			printl(e.message);
		}
		try {
			minint--;
			throw VarError("Uncaught error VarNumUnderflow");
		} catch (VarNumUnderflow& e) {
			printl(e.message);
		}
		try {
			maxint++;
			throw VarError("Uncaught error VarNumOverflow");
		} catch (VarNumOverflow& e) {
			printl(e.message);
		}

		try {
			throw VarOutOfMemory("Error: x");
		} catch (VarOutOfMemory& e) {
			printl(e.message);
		}
		try {
			throw VarInvalidPointer("Error: x");
		} catch (VarInvalidPointer& e) {
			printl(e.message);
		}

			// We cannot test this because the output of the word "Exception" triggers testing to fail
			// ***Failed  Error regular expression found in output. Regex=[(Exception|Test failed)]  9.78 sec
			//try { throw VarDBException("Error: x"); } catch (VarDBException& e){};

#ifdef VAR_SAFE_DESTRUCTOR
		try {
			var v1	 = v1 + 1;
			var v2	 = v2 + 1;
			var v3	 = v3 + 1;
			var v4	 = v4 + 1;
			var v5	 = v5 + 1;
			var v6	 = v6 + 1;
			var v7	 = v7 + 1;
			throw VarError("Uncaught error VarUnconstructed");
		} catch (VarUnconstructed& e) {
			printl(e.message);
		} catch (VarUnassigned& e) {
			printl(e.message);
		}
#endif
		//unfortunately throw causes cmake test to fail so we skip them
		//try {throw VarDBException("test");uncaught="VarDBException";}  catch (VarDBException& e){};
		//try {throw VarDebug("test");uncaught="VarDebug";}              catch (VarDebug& e){};

		try {
			var v	 = var("x").oconv("QQQ");
			throw VarError("Uncaught error VarOconvNotImplemented");
		} catch (VarNotImplemented& e) {
			printl(e.message);
		}

		// dim

		//try {dim x(0)             ;uncaught="DimDimensionedZero";} catch (DimDimensionedZero& e){};
		try {
			dim x(10);
			printl(x[11]);
			throw VarError("DimIndexOutofBounds");
		} catch (DimIndexOutOfBounds& e) {
			printl(e.message);
		}

		// Exceeding 
		try {
			dim x(10);
			printl(x.at(1, 2));

#define FLEXIBLE_ROW_COL 0
#if FLEXIBLE_ROW_COL
			throw VarError("Uncaught error VarUnassigned");
		} catch (VarUnassigned& e) {
			printl(e.message);
		}
#else
			throw VarError("Uncaught error VarDimIndexOutofBounds");
		} catch (DimIndexOutOfBounds& e) {
			printl(e.message);
		}
#endif
		catch (VarError e) {
			throw VarError("Unexpected error using dim x(1,2)" ^ var(e.message));
		}

		try {
			const dim x(10);
			printl(x[11]);
			throw VarError("Uncaught error DimIndexOutofBounds");
		} catch (DimIndexOutOfBounds& e) {
			printl(e.message);
		}

		try {
			const dim x(10);
			printl(x.at(1, 2));
			throw VarError("Uncaught error DimIndexOutofBounds");
		} catch (DimIndexOutOfBounds& e) {
			printl(e.message);
		}

		try {
			dim x;
			var y	 = x.join();
			throw VarError("Uncaught error - DimIndexOutofBounds");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}
		try {
			dim x;
			dim y	 = x;
			throw VarError("Uncaught error DimUndimensioned 2");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}
		try {
			dim x;
			x		 = dim();
//			throw VarError("Uncaught error DimUndimensioned 3");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}
		try {
			dim x;
			var y	 = x.cols();
//			throw VarError("Uncaught error DimUndimensioned 4");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}
		try {
			dim x;
			var y	 = x.join();
			throw VarError("Uncaught error DimUndimensioned 5");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}
		try {
			dim x;
			x		 = "q";
			throw VarError("Uncaught error DimUndimensioned 5");
		} catch (DimUndimensioned& e) {
			printl(e.message);
		}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
		try {
			stop();
			throw VarError("Uncaught error ExoStop");
			//std::unreachable();
		} catch (ExoStop& e) {
			printl("Caught ExoStop", e.message);
		}
		try {
			abort(99);
			throw VarError("Uncaught error ExoAbort");
			//std::unreachable();
		} catch (ExoAbort& e) {
			printl("Caught ExoAbort", e.message);
		}
		try {
			abortall();
			throw VarError("Uncaught error ExoAbortAll");
			//std::unreachable();
		} catch (ExoAbortAll& e) {
			printl("Caught ExoAbortAll", e.message);
		}
//		try {
//			logoff();
//			throw VarError("Uncaught error ExoStop");
//			//std::unreachable();
//		} catch (ExoStop& e) {
//			printl("Caught MVlogoff", e.message);
//		}
#pragma GCC diagnostic pop

//		if (uncaught) {
//			TRACE(uncaught)
//			assert(uncaught eq "");
//		}
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

}; // programexit()
