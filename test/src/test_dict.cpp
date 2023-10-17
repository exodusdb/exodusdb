#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	var xo_dict	 = "xo_dict";
var dict_xo_test = "dict.xo_test";
var default_conn;
var xo_dict_conn;

function main() {

	printl("\n   --- clear any existing EXO_DATA/DICT env ---\n");
	ossetenv("EXO_DATA", "");
	ossetenv("EXO_DICT", "");

	printl("\n   --- default connection first thing before anything else to ensure clean ---\n");
	printl("\n   --- quit (pass test) if no default database connection ---\n");
	if (not default_conn.connect()) {
		printl("\n   --- No default db connection to perform db testing.\n");
		printl("Test passed. Not really. No default db connection.");
		return 0;
	}
	TRACE(default_conn)

	//Skip if fast testing required
	if (osgetenv("EXO_FAST_TEST")) {
		printl("Test passed. Not really. EXO_FAST_TEST - skipping");
		return 0;
	}

	gosub cleanup();

	printl("\n   --- create a specific database for dicts ---\n");
	assert(dbcreate(xo_dict));

	// This no longer works as is.
	// It is unclear how to change the DEFAULT database for dicts
	if ((false)) {

		printl("\n   --- test with EXO_DICT ---\n");

		printl("\n   --- connect to the specific database ---\n");
		assert(xo_dict_conn.connect(xo_dict));

		printl("\n   --- say that dicts are on the specific database " ^ xo_dict ^ " ---\n");
		ossetenv("EXO_DICT",xo_dict);
		assert(var(getenv("EXO_DICT")) eq xo_dict);

		printl("\n   --- create a dict on the specific database implicitly ---\n");
		assert(createfile(dict_xo_test));

		printl("\n   --- ensure new dict IS on the right database implicitly ---\n");
		var dicttest;
		assert(dicttest.open(dict_xo_test));

		printl("\n   --- ensure new dict IS on the right database specifically ---\n");
		var dicttest2;
		assert(dicttest2.open(dict_xo_test, xo_dict_conn));

		printl("\n   --- ensure dict IS NOT on the default connection ---\n");
		assert(not dicttest.open(dict_xo_test, default_conn));

		printl("\n   --- delete the dict on the specific database implicitly ---\n");
		assert(deletefile(dict_xo_test));

		printl("\n   --- ensure new dict IS NO LONGER on the right database implicitly ---\n");
		var dicttest3;
		assert(not dicttest3.open(dict_xo_test));

		printl("\n   --- ensure new dict IS NO LONGER on the specific database ---\n");
		var dicttest4;
		assert(not dicttest4.open(dict_xo_test, xo_dict_conn));

		printl("\n   --- disconnect specific connection ---\n");
		xo_dict_conn.disconnect();

	}

	printl("\n   --- test without EXO_DICT ---\n");
	if (true) {

		printl("\n   --- connect to the specific database (although not used for dict here) ---\n");

		printl("\n   --- say that dicts are on the default database ---\n");
		ossetenv("EXO_DICT", "");
		assert(var(osgetenv("EXO_DICT")) eq "");

		printl("\n   --- needed to remove default dict connection ---\n");
		disconnectall();

		printl("\n   --- specific connection to the dic database ---\n");
		assert(xo_dict_conn.connect(xo_dict));

		printl("\n   --- create a dict on the default database implicitly ---\n");
		assert(createfile(dict_xo_test));

		printl("\n   --- ensure new dict IS on the right database implicitly ---\n");
		var dicttest;
		assert(dicttest.open(dict_xo_test));

		printl("\n   --- ensure new dict IS on the right database specifically ---\n");
		var dicttest2;
		assert(dicttest2.open(dict_xo_test, default_conn));

		printl("\n   --- ensure dict IS NOT on the test database ---\n");
		var dicttest3;
		assert(not dicttest3.open(dict_xo_test, xo_dict_conn));

		printl("\n   --- delete the dict on the default database implicitly ---\n");
		assert(deletefile(dict_xo_test));

		printl("\n   --- ensure new dict IS NO LONGER on the default database implicitly ---\n");
		var dicttest4;
		assert(not dicttest4.open(dict_xo_test));

		printl("\n   --- ensure new dict IS NO LONGER on the default database specifically ---\n");
		var dicttest5;
		assert(not dicttest5.open(dict_xo_test, default_conn));

		printl("\n   --- ensure new dict IS NO LONGER on the special  database ---\n");
		var dicttest6;
		assert(not dicttest6.open(dict_xo_test, xo_dict_conn));

		printl("\n   --- disconnect specific connection ---\n");
		xo_dict_conn.disconnect();
	}

	gosub cleanup();

	printl(elapsedtimetext());
	printl("\nTest passed");

	return 0;
}

subroutine cleanup() {

	if (not dbdelete(xo_dict))
		loglasterror();

	printl("\n   --- delete the dict file from the default connection just in case, to be clean ---\n");
	if (not default_conn.deletefile(dict_xo_test))
		loglasterror();
}
programexit()
