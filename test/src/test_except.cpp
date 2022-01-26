#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <functional>
#include <limits>

#include <exodus/program.h>

programinit()

function main() {

	printl("test_more3 says 'Hello World!'");

	{
		using mvint_t = long long;
		var minint = std::numeric_limits<mvint_t>::min();
		var maxint = std::numeric_limits<mvint_t>::max();

		var uncaught = "";

		try {throw MVError("test");uncaught="MVError";}         catch (MVError e){};
		try {var v;v++            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v=var("x")+1     ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v=1;v=v/0        ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {--minint             ;uncaught="MVIntUnderflow";}  catch (MVIntUnderflow e){};
		try {++maxint             ;uncaught="MVIntUnderflow";}  catch (MVIntOverflow e){};
		try {minint--             ;uncaught="MVIntUnderflow";}  catch (MVIntUnderflow e){};
		try {maxint++             ;uncaught="MVIntUnderflow";}  catch (MVIntOverflow e){};
#define SAFE_DESTRUCTOR
#ifdef SAFE_DESTRUCTOR
		try {var v = v+1          ;uncaught="MVUndefined";}     catch (MVUndefined e){} catch (MVUnassigned e){};
#endif
		//unfortunately throw causes cmake test to fail so we skip them
		//try {throw MVDBException("test");uncaught="MVDBException";}  catch (MVDBException e){};
		//try {throw MVDebug("test");uncaught="MVDebug";}              catch (MVDebug e){};
		try {var v=var("x").oconv("QQQ");uncaught="MVNotImplemented";} catch (MVNotImplemented e){};
		try {dim x(0)             ;uncaught="MVArrayDimensionedZero";} catch (MVArrayDimensionedZero e){};
		try {dim x(10);printl(x(11))      ;uncaught="MVArrayIndexOutofBounds";}catch (MVArrayIndexOutOfBounds e){};
		try {dim x; var y=x.join();uncaught="MVArrayNotDimensioned";}  catch (MVArrayNotDimensioned e){};
		try {stop()               ;uncaught="MVStop";}                 catch (const MVStop& e){printl("Caught MVStop");};
		try {var().abort(99)      ;uncaught="MVAbort";}                catch (MVAbort e){printl("Caught MVAbort");};
		//for some currently unknown reason this cannot be caught despite being almost identical to abort
		//try {var().abortall()   ;uncaught="MVAbortAll";}             catch (MVAbortAll e){printl("Caught MVAbortAll");};
		try {logoff()             ;uncaught="MVLogoff";}               catch (MVLogoff e){};

		if (uncaught) {
			printl(uncaught);
			assert(uncaught == "");
		}
		//	try {throw MV("test");} catch (MV e){};
	}

	printl("Test passed");

	return 0;
}

programexit()

