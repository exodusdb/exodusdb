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

        var unass;
		try {var v = unass        ;assert(false && "copy constructor from unassigned lvalue");}    catch (MVUnassigned e){};
		try {var v;v = unass      ;assert(false && "copy assign from unassigned lvalue");}    catch (MVUnassigned e){};

		//move ALLOWS use of unassigned without throwing for performance since temporaries are unlikeley to be unassigned
//		try {var v = var()        ;assert(false && "move constructor from unassigned rvalue");}    catch (MVUnassigned e){};
//		try {var v;v = var()      ;assert(false && "move assign from unassigned rvalue");}    catch (MVUnassigned e){};
		try {var v = var()        ;} catch (MVUnassigned e){assert(false && "move constructor from unassigned rvalue");};
		try {var v;v = var()      ;} catch (MVUnassigned e){assert(false && "move assign from unassigned rvalue");};

		var uncaught = "";

		try {var v=var("x")+1     ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};

		// Divide by zero

		try {var v=1;v=v/var(0)   ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {var v=1;v=v/0        ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {var v=1;v=v/"0"      ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {var v=1;v=v/0.0      ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {var v=1;v=v/"0.0"    ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};

		try {var v=1;v%=var(0.0)  ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};
		try {var v=1;v%=var(0)    ;uncaught="MVDivideByZero";}  catch (MVDivideByZero e){};

		// Unitary minus

		try {var v;v = -v         ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v = "x";v = -v   ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};

		// Increment/Decrement

		try {var v;v++            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v--            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;++v            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;--v            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};

		try {var v = "x";v++            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v--            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";++v            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";--v            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};

		// Self assign

		try {var v;v+=1.1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v-=1.1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v*=1.1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v/=1.1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v%=1.1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};

		try {var v = "x";v+=1.1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v-=1.1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v*=1.1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v/=1.1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v%=1.1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};


		try {var v;v+=1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v-=1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v*=1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v/=1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v%=1            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};

		try {var v = "x";v+=1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v-=1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v*=1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v/=1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v%=1            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};


		try {var v;v+="1"            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v-="1"            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v*="1"            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v/="1"            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};
		try {var v;v%="1"            ;uncaught="MVUnassigned";}    catch (MVUnassigned e){};

		try {var v = "x";v+="1"            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v-="1"            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v*="1"            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v/="1"            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};
		try {var v = "x";v%="1"            ;uncaught="MVNonNumeric";}    catch (MVNonNumeric e){};

		// Overflow on inc/dec

		try {var x = "xyz"_var.locateby("XX", "xyz", MV);uncaught="MVError locateby";} catch(MVError e){};

		try {throw MVError("test");uncaught="MVError";}         catch (MVError e){};

		try {--minint             ;uncaught="MVIntUnderflow";}  catch (MVIntUnderflow e){};
		try {++maxint             ;uncaught="MVIntUnderflow";}  catch (MVIntOverflow e){};
		try {minint--             ;uncaught="MVIntUnderflow";}  catch (MVIntUnderflow e){};
		try {maxint++             ;uncaught="MVIntUnderflow";}  catch (MVIntOverflow e){};


		try { throw MVOutOfMemory("Error: x"); } catch (MVOutOfMemory e){};
		try { throw MVInvalidPointer("Error: x"); } catch (MVInvalidPointer e){};

		// We cannot test this because the output of the word "Exception" triggers testing to fail
		//***Failed  Error regular expression found in output. Regex=[(Exception|Test failed)]  9.78 sec
		//try { throw MVDBException("Error: x"); } catch (MVDBException e){};

#ifdef VAR_SAFE_DESTRUCTOR
		try {var v = v+1          ;uncaught="MVUndefined";}     catch (MVUndefined e){} catch (MVUnassigned e){};
#endif
		//unfortunately throw causes cmake test to fail so we skip them
		//try {throw MVDBException("test");uncaught="MVDBException";}  catch (MVDBException e){};
		//try {throw MVDebug("test");uncaught="MVDebug";}              catch (MVDebug e){};

		try {var v=var("x").oconv("QQQ");uncaught="MVNotImplemented";} catch (MVNotImplemented e){};

		// dim

		//try {dim x(0)             ;uncaught="MVArrayDimensionedZero";} catch (MVArrayDimensionedZero e){};
		try {dim x(10);printl(x(11))         ;uncaught="MVArrayIndexOutofBounds";}catch (MVArrayIndexOutOfBounds e){};
		try {dim x(10);printl(x(1,2))        ;uncaught="MVArrayIndexOutofBounds";}catch (MVArrayIndexOutOfBounds e){};
		try {const dim x(10);printl(x(11))   ;uncaught="MVArrayIndexOutofBounds";}catch (MVArrayIndexOutOfBounds e){};
		try {const dim x(10);printl(x(1,2))  ;uncaught="MVArrayIndexOutofBounds";}catch (MVArrayIndexOutOfBounds e){};

		try {dim x; var y=x.join()           ;uncaught="MVArrayNotDimensioned 1";}  catch (MVArrayNotDimensioned e){};
		try {dim x; dim y = x                ;uncaught="MVArrayNotDimensioned 2";}  catch (MVArrayNotDimensioned e){};
		try {dim x; x = dim()                ;uncaught="MVArrayNotDimensioned 3";}  catch (MVArrayNotDimensioned e){};
		try {dim x; var y=x.cols()           ;uncaught="MVArrayNotDimensioned 4";}  catch (MVArrayNotDimensioned e){};
		try {dim x; var y=x.rows()           ;uncaught="MVArrayNotDimensioned 5";}  catch (MVArrayNotDimensioned e){};
		try {dim x; x = "q"                  ;uncaught="MVArrayNotDimensioned 5";}  catch (MVArrayNotDimensioned e){};

		try {stop()               ;uncaught="MVStop";}                 catch (const MVStop& e){printl("Caught MVStop");};
		try {abort(99)      ;uncaught="MVAbort";}                catch (MVAbort e){printl("Caught MVAbort");};
		//try {var().abort(99)      ;uncaught="MVAbort";}                catch (MVAbort e){printl("Caught MVAbort");};

		try {abortall()   ;uncaught="MVAbortAll";}             catch (MVAbortAll e){printl("Caught MVAbortAll");};

		try {logoff()             ;uncaught="MVLogoff";}               catch (MVLogoff e){printl("Caught MVlogoff");};

		if (uncaught) {
			printl(uncaught);
			assert(uncaught == "");
		}
		//	try {throw MV("test");} catch (MV e){};
	}

	{
		var bt = backtrace();
		printl(bt);
		assert(bt.match(R"__(\d+:\s+test_except.cpp:\d+: var bt = backtrace\(\);)__", ""));
	}

	printl("Test passed");

	return 0;
}

programexit()

