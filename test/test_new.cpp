#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("test says 'Hello World!'");

	{	// Easy wrapping/unwrapping of vars in FM (i.e. '\0x1f') or other delimiters including (multibyte and multichar)
		// Easy to cut and paste list of variable but can easily get silently out of sync.
		var package;
		{
			var a = "a";
			var b = "b";
			var c = "c";
			// Super easy FM Only method to package at the moment
			package = {a, b, c};
		}
		{
			// Can unpack using any delimiter even multichar delimiters e.g. ||
			// NO way to pass unassigned atm.
			auto [a, b, c, d] = package.unpack<4>(FM);
			assert(a eq "a" and b eq "b" and c eq "c" and d.unassigned());
		}
	}
	{
		printl("Test auto [a, b, c] = varx.unpack<N>()");

		{	// Any delimiter
			auto [aa, bb, cc, dd] = unpack<4>("aa]bb]cc"_var, VM);
			assert(aa eq "aa" and bb eq "bb" and cc eq "cc" and dd.unassigned());
		}

		{ 	// Including Unicode
			auto [aa, bb, cc, dd] = unpack<4>("aa⭕bb⭕cc", "⭕");
			assert(aa eq "aa" and bb eq "bb" and cc eq "cc" and dd.unassigned());
		}

		{ 	// Including multichar
			auto [aa, bb, cc, dd] = unpack<4>("aa||bb||cc", "||");
			assert(aa eq "aa" and bb eq "bb" and cc eq "cc" and dd.unassigned());
		}

		var v1 = {1,2,3};
		logputl(v1);
		assert(v1 = "1^2^3"_var);
		{
			printl("Unpack 1");
			auto [a] = unpack<1>(v1);
			assert(a eq 1);
		}
		{
			printl("Unpack 2");
			auto [a,b] = unpack<2>(v1);
			assert(a eq 1 and b eq 2);
		}
		{
			printl("Unpack 3");
			auto [a, b, c] = unpack<3>(v1);
			assert(a eq 1 and b eq 2 and c eq 3);
		}
		{
			printl("Unpack 4");
			auto [a, b, c, d] = unpack<4>(v1);
			assert(a eq 1 and b eq 2 and c eq 3 and d.unassigned());
		}
		{
			printl("Unpack 5");
			auto [a, b, c, d, e] = unpack<5>(v1);
			assert(a eq 1 and b eq 2 and c eq 3 and d.unassigned() and e.unassigned());
		}
	}
	{
		printl("Test auto [a, b, c] = dimx.unpack<N>()");

		dim d1 = {1,2,3};
		logputl(d1.join());
		{
			printl("Unpack 1");
			auto [a] = unpack<1>(d1);
			assert(a eq 1);
		}
		{
			printl("Unpack 2");
			auto [a,b] = unpack<2>(d1);
			assert(a eq 1 and b eq 2);
		}
		{
			printl("Unpack 3");
			auto [a, b, c] = unpack<3>(d1);
			TRACE(a)
			TRACE(b)
			TRACE(c)
			assert(a eq 1 and b eq 2 and c eq 3);//err
			a = "x";
			TRACE(d1[1])
			TRACE(d1[2])
			TRACE(d1[3])
		}
		{
			printl("Unpack 4");
			auto [a, b, c, d] = unpack<4>(d1);
			assert(a eq 1 and b eq 2 and c eq 3 and d.unassigned());
		}
		{
			printl("Unpack 5");
			auto [a, b, c, d, e] = unpack<5>(d1);
			assert(a eq 1 and b eq 2 and c eq 3 and d.unassigned() and e.unassigned());
		}
	}

	{
		// Use a function that returns three args. Using delimiter "| |" here.
		auto [x, y, z] = func_returning_three_vars("a| |b| |c", "| |");
		assert(x eq "a" and y eq "b" and z eq "c");

	}

	{
		// multi-argument starts, ends, contains
	}

	{
		// x.listed(list) and listed(x, list)
        let v1 = "def";

        assert(v1.listed("abc,def"));
        assert(listed(v1, "abc,def"));

        assert(v1.listed("abc,def", MV) && MV.errputl() eq 2);
        assert(listed(v1, "abc,def", MV) && MV.errputl() eq 2);

        assert(not v1.listed("abc,deff", MV) && MV.errputl() eq 0);
        assert(not listed(v1, "abc,deff", MV) && MV.errputl() eq 0);
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

	{
		logputl("Test async Job and queuing");

		// Start a job using libtestecho.so
		Job j1("testecho");

		// Message the job
		j1.input_queue->push("123");

		// Check that the job is not closed.
		// It should timeout and return false
		// since echo should still be running and waiting
		if (j1.wait_for(100)) {
			j1.get(); // Trigger any exception.
			// Should not get here.
			assert(false);
		}

		// Verify that job replies correctly.
		j1.output_queue->wait_and_pop(DATA);
		assert(DATA = 123);

		// Request echo to close with a "" input.
		j1.input_queue->push("");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

auto func_returning_three_vars(in v1, in delim) -> std::array<var,3> {
	return v1.unpack<3>(delim);
}

}; // programexit()
