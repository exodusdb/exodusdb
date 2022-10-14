#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <thread>

#include <exodus/program.h>
programinit()

	var key        = 1;
	var max        = 5;
	var nprocesses = 10;//50;

	// Create a temporary file (ending _temp)
	var filename = "xo_test";

	var file;

	var silent = OPTIONS.count("S");

function main() {

	//Pass if no default database connection
	if (not connect()) {
	//if (true or not connect()) {
		////
		printl("No default db connection to perform test_asyncupd");
		printl("Test passed");
		return 0;
	}

	//Skip if fast testing required
	if (osgetenv("EXO_FAST_TEST")) {
		printl("EXO_FAST_TEST - skipping test.");
		printl("Test passed");
		return 0;
	}

	// Will FAIL without locking
	var use_locking = not OPTIONS.count("L");

	// Will work with and without transactions
	var use_transaction = not OPTIONS.count("T");

	if (not TERMINAL)
		silent = 1;

	if (not open(filename to file)) {
		createfile(filename);
		assert(open(filename to file));
	}

	THREADNO = COMMAND.f(2);

	// Start multiple processes, wait for them to finish, and check result as expected
	// and exit
	if (not THREADNO) {

		// Test WITH any given options
		// or test with NO options (ie with transactions and locking)
		var ok = test(OPTIONS);

		// If no given options then test WITHOUT transactions (i.e. WITH T option!)
		if (ok and not OPTIONS)
			ok = test("T");

		if (ok)
			printl("Test passed");

		return not ok;
	}

	// Perform one process updates
	if (silent < 2)
		printl("Starting", THREADNO, OPTIONS);

	var n = "";
	while (n < max and not esctoexit()) {

		// Start transaction
		if (use_transaction)
			assert(begintrans());

		if (not use_locking || lock(file, key)) {

			if (not read(RECORD from file, key))
				RECORD = "";

			if (n eq "")
				n = 1;
			else {
				var o = RECORD.f(THREADNO);
				assert(o eq n);
				n = o + 1;
			}

			RECORD(THREADNO) = n;

			write(RECORD on file, key);

			if (use_locking)
				unlock(file, key);

			if (not silent)
				printl(THREADNO, RECORD);

			// Commit transaction
			if (use_transaction)
				assert(committrans());

		} else {
			//printl("Cannot lock", THREADNO);
			if (use_transaction)
				assert(rollbacktrans());
			//errputl(THREADNO, "Sleeping");
			ossleep(100);
		}
	}

	if (silent < 2)
		printl("Finished", THREADNO, OPTIONS);

	return 0;
}

function test(in option) {

	deleterecord(file, key);

	var cmd		= "";
	var options = OPTIONS.convert("{}", "") ^ option;
	for (const var processn : range(1, nprocesses)) {
		// Hard code path so make test can find it without any installation
		cmd ^= "./test_asyncupd " ^ processn ^ " {" ^ options ^ "} & ";
	}

	//osshell("test_asyncupd 1 & test_asyncupd 2 & test_asyncupd 3 & test_asyncupd 4 & test_asyncupd 5 &");
	printl(cmd);
	assert(osshell(cmd));
	while (osshellread("pgrep test_asyncupd").count("\n") gt 1) {

		if (not silent)
			printl("Waiting for test_asyncupd processes to complete");

		ossleep(1000);
	}

	if (not read(RECORD from file, key))
		RECORD = "";

	var target = str(max ^ FM, nprocesses).pop();

	if (RECORD ne target) {
		errputl("ERROR: " ^ RECORD.quote() ^ " not equal to target " ^ target.quote());
		return false;
	}

	if (not silent)
		printl("OK: " ^ RECORD.quote() ^ " equal to target");

	return true;
}

programexit()
