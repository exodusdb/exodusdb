#include <cassert>

#include <exodus/program.h>
programinit()

	var xo_dict = "xo_dict";
	var dict_xo_test = "dict.xo_test";
	var default_conn;
	var xo_dict_conn;

function main() {

	printl("\n   --- default connection first thing before anything else to ensure clean ---\n");
    printl("\n   --- quit (pass test) if no default database connection ---\n");
    if (not default_conn.connect()) {
        printl("\n   --- No default db connection to perform db testing. Test passed ---\n");
        return 0;
    }
	TRACE(default_conn)

	gosub cleanup();

	printl("\n   --- create a specific database for dicts ---\n");
	assert(dbcreate(xo_dict));

	printl("\n   --- test with EXO_DICTDB ---\n");
	if (true) {

		printl("\n   --- connect to the specific database ---\n");
		assert(xo_dict_conn.connect(xo_dict));

		printl("\n   --- say that dicts are on the specific database " ^ xo_dict ^ " ---\n");
		assert(ossetenv("EXO_DICTDB",xo_dict));
		assert(var(getenv("EXO_DICTDB")) == xo_dict);

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

	printl("\n   --- test without EXO_DICTDB ---\n");
	if (true) {

		printl("\n   --- connect to the specific database (although not used for dict here) ---\n");

		printl("\n   --- say that dicts are on the default database ---\n");
		assert(ossetenv("EXO_DICTDB",""));
		assert(var(getenv("EXO_DICTDB")) == "");

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

	printl("\n   --- Test passed ---\n");

	return 0;
}

subroutine cleanup() {

	dbdelete(xo_dict);

	printl("\n   --- delete the dict file from the default connection just in case, to be clean ---\n");
	default_conn.deletefile(dict_xo_test);

}
programexit()

