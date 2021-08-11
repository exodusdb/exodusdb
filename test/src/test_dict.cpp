#include <cassert>

#include <exodus/program.h>
programinit()

	var xo_dict = "xo_dict";
	var dict_xo_test = "dict_xo_test";
	var default_conn;
	var xo_dict_conn;

function main() {

	// default connection first thing before anything else to ensure clean
	assert(default_conn.connect());

	gosub cleanup();

	//createa specific database for dicts
	assert(createdb(xo_dict));

	//test with EXO_DICTDBNAME
	if (true) {

		//connect to the specific database
		assert(xo_dict_conn.connect(xo_dict));

		//say that dicts are on the specific database
		assert(ossetenv("EXO_DICTDBNAME",xo_dict));
		assert(var(getenv("EXO_DICTDBNAME")) == xo_dict);

		//create a dict on the specific database implicitly
		assert(createfile(dict_xo_test));

		//ensure new dict IS on the right database implicitly
		var dicttest;
		assert(dicttest.open(dict_xo_test));

		//ensure new dict IS on the right database specifically
		var dicttest2;
		assert(dicttest2.open(dict_xo_test, xo_dict_conn));

		//ensure dict IS NOT on the default connection
		assert(not dicttest.open(dict_xo_test, default_conn));

		//delete the dict on the specific database implicitly
		assert(deletefile(dict_xo_test));

		//ensure new dict IS NO LONGER on the right database implicitly
		var dicttest3;
		assert(not dicttest3.open(dict_xo_test));

		//ensure new dict IS NO LONGER on the specific database
		var dicttest4;
		assert(not dicttest4.open(dict_xo_test, xo_dict_conn));

		//disconnect specific connection
		xo_dict_conn.disconnect();

	}

	//test without EXO_DICTDBNAME
	if (true) {

		//connect to the specific database (although not used for dict here)

		//say that dicts are on the default database
		assert(ossetenv("EXO_DICTDBNAME",""));
		assert(var(getenv("EXO_DICTDBNAME")) == "");

		//needed to remove default dict connection
		disconnectall();

		assert(xo_dict_conn.connect(xo_dict));

		//create a dict on the default database implicitly
		assert(createfile(dict_xo_test));


		//ensure new dict IS on the right database implicitly
		var dicttest;
		assert(dicttest.open(dict_xo_test));

		//ensure new dict IS on the right database specifically
		var dicttest2;
		assert(dicttest2.open(dict_xo_test, default_conn));

		//ensure dict IS NOT on the test database
		var dicttest3;
		assert(not dicttest3.open(dict_xo_test, xo_dict_conn));


		//delete the dict on the default database implicitly
		assert(deletefile(dict_xo_test));


		//ensure new dict IS NO LONGER on the default database implicitly
		var dicttest4;
		assert(not dicttest4.open(dict_xo_test));

		//ensure new dict IS NO LONGER on the default database specifically
		var dicttest5;
		assert(not dicttest5.open(dict_xo_test, default_conn));

		//ensure new dict IS NO LONGER on the special  database
		var dicttest6;
		assert(not dicttest6.open(dict_xo_test, xo_dict_conn));


		//disconnect specific connection
		xo_dict_conn.disconnect();

	}

	gosub cleanup();

	printl("Test passed");

	return 0;
}

subroutine cleanup() {

	deletedb(xo_dict);

}
programexit()

