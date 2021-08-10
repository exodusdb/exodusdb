#include <cassert>

#include <exodus/program.h>
programinit()

	var exodus_test = "exodus_test";
	var dict_test = "dict_test";
	var default_conn;
	var exodus_test_conn;

function main() {

	// default connection first thing before anything else to ensure clean
	assert(default_conn.connect());

	gosub cleanup();

	//createa specific database for dicts
	assert(createdb(exodus_test));

	//test with EXO_DICTDBNAME
	if (true) {

		//connect to the specific database
		assert(exodus_test_conn.connect(exodus_test));

		//say that dicts are on the specific database
		assert(ossetenv("EXO_DICTDBNAME",exodus_test));
		assert(var(getenv("EXO_DICTDBNAME")) == exodus_test);

		//create a dict on the specific database implicitly
		assert(createfile(dict_test));

		//ensure new dict IS on the right database implicitly
		var dicttest;
		assert(dicttest.open(dict_test));

		//ensure new dict IS on the right database specifically
		var dicttest2;
		assert(dicttest2.open(dict_test, exodus_test_conn));

		//ensure dict IS NOT on the default connection
		assert(not dicttest.open(dict_test, default_conn));

		//delete the dict on the specific database implicitly
		assert(deletefile(dict_test));

		//ensure new dict IS NO LONGER on the right database implicitly
		var dicttest3;
		assert(not dicttest3.open(dict_test));

		//ensure new dict IS NO LONGER on the specific database
		var dicttest4;
		assert(not dicttest4.open(dict_test, exodus_test_conn));

		//disconnect specific connection
		exodus_test_conn.disconnect();

	}

	//test without EXO_DICTDBNAME
	if (true) {

		//connect to the specific database (although not used for dict here)

		//say that dicts are on the default database
		assert(ossetenv("EXO_DICTDBNAME",""));
		assert(var(getenv("EXO_DICTDBNAME")) == "");

		//needed to remove default dict connection
		disconnectall();

		assert(exodus_test_conn.connect(exodus_test));

		//create a dict on the default database implicitly
		assert(createfile(dict_test));


		//ensure new dict IS on the right database implicitly
		var dicttest;
		assert(dicttest.open(dict_test));

		//ensure new dict IS on the right database specifically
		var dicttest2;
		assert(dicttest2.open(dict_test, default_conn));

		//ensure dict IS NOT on the test database
		var dicttest3;
		assert(not dicttest3.open(dict_test, exodus_test_conn));


		//delete the dict on the default database implicitly
		assert(deletefile(dict_test));


		//ensure new dict IS NO LONGER on the default database implicitly
		var dicttest4;
		assert(not dicttest4.open(dict_test));

		//ensure new dict IS NO LONGER on the default database specifically
		var dicttest5;
		assert(not dicttest5.open(dict_test, default_conn));

		//ensure new dict IS NO LONGER on the special  database
		var dicttest6;
		assert(not dicttest6.open(dict_test, exodus_test_conn));


		//disconnect specific connection
		exodus_test_conn.disconnect();

	}

	gosub cleanup();

	printl("Test passed");

	return 0;
}

subroutine cleanup() {

	deletedb(exodus_test);

}
programexit()

