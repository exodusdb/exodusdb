#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("test_new_db says 'Hello World!'");

	// Quit if no default database connection
	if (not connect()) {
		//Pass if allowed
		if (osgetenv("EXO_NODATA") or true) {
			printx("Test passed. Not really. ");
		}
		printl("No default db connection to perform db testing.");
		return 0;
	}

	{
		printl("Check that we can read/write/delete files");
		// without opening them after new logic in vardb::read/write/delete
		// to always use sql PREPARED statements and fall back to normal sql on error.

		// Fallback on error doesnt work in transactions
		// because ANY error causes transaction failure.

//		let xo_test_new1 = "xo_test_new1_temp";
		let xo_test_new1 = "xo_test_new1";
		if (not deletefile(xo_test_new1)) {};
		assert(createfile(xo_test_new1));

		// Remove prepared statements created by createfile
		disconnect();
		assert(connect());

		let xo_test_new2_temp = "xo_test_new2_temp";
		assert(createfile(xo_test_new2_temp));
		// createfile auto opens/prepares files
//		assert(open(xo_test_new2_temp));

		ID = "ABC";

		printl("Test write/read/delete from an unopened file. Recovers and succeeds if outside transactions.");
		errputl("write");
//		try {
			write("123", xo_test_new1, ID);
//			abort("aaaaaaaaaaaaaaaaaaaaaaaa");
//		} catch (VarError e) {
//			abort("xxxxxxxxxxxx");
//		}
		errputl("read");
		assert(read(RECORD, xo_test_new1, ID));
		errputl("delete");
		assert(deleterecord(xo_test_new1, ID));
		errputl("not read");
		assert(not read(RECORD, xo_test_new1, ID));
		errputl("not read ok");

		// Throws if in transaction because using PREPARED postgresql statements.
		// which are only done on opening and any errors terminate transactions.

		// TRANSACTION
		assert(begintrans());

		try {
			printl("Check creation also prepares the file.");
			write("123", xo_test_new2_temp, ID);
		}
		catch (...) {
			assert(false and "Write on created not opened file SHOULD be ok.");
		}

		try {
			printl("Check cannot use unopened (unprepared) file.");
			// It was created before connection and not opened after so it is unprepared.
			errputl("write");
			write("123", xo_test_new1, ID);

			errputl("read");
			assert(read(RECORD, xo_test_new1, ID));

			errputl("delete");
			assert(deleterecord(xo_test_new1, ID));
			errputl("deleted");

			assert(false && "Read/write/delete on unopened file should THROW inside transaction.");
		}
		catch (VarError e) {
			// Verify throwm
//			TRACE(e.message);
			errputl("OK - cannot use.");
		}

		printl("Check in trans (but errored)");
		assert(statustrans());

		printl("Commit trans with errors should rollback");
		// Exodus returns FALSE if trans was in error and commit has resulted in ROLLBACK
		// (pg returns OK "because it rolled back successfully")
		assert(not committrans());

		printl("Rollback should have occurred");
		assert(not statustrans());
		assert(not read(RECORD, xo_test_new2_temp, ID));
		assert(not read(RECORD, xo_test_new1, ID));

		printl("Clean up non-temporary file");
		if (deletefile(xo_test_new1)) {};

	}

	{
		// Check delete and recreate file with the same name properly "prepares" the new file (for read/write/delete)
		// postgres OID is stored in prepared statements so filename is not sufficient.

		// BEGIN
		assert(begintrans());
		let filename = "xo_test_delete_create_temp";
		if (not deletefile(filename)) {}

		// Check createfile auto opens the file.
		// i.e. prepares the file on the connection.
		assert(createfile(filename));
		write("x" on filename, "x");

		// delete and recreate the file
		assert(deletefile(filename));
		assert(createfile(filename));

		// Check createfile auto opens (and prepares) the file even after delete and recreate.
		write("x" on filename, "x");

		// ROLLBACK
		assert(rollbacktrans());

	}

	printl(elapsedtimetext());
	printl("Test passed.");

	return 0;
}

}; // programexit()
