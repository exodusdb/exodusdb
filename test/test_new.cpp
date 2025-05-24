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
		logputl("Test async Job and queuing");
		Job j1("testecho");
		j1.input_queue->push("123");
		// It should timeout if echo is running and waiting
		if (j1.wait_for(100)) {
			j1.get(); // Trigger any exception.
			// Should not get here.
			assert(false);
		}
		j1.output_queue->wait_and_pop(DATA);
		assert(DATA = 123);
		// Close echo
		j1.input_queue->push("");
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
	}

	{
		let randomfilename1 = "xo_test_new1_temp";
		assert(createfile(randomfilename1));

		let randomfilename2 = "xo_test_new2_temp";
		assert(createfile(randomfilename2));
		assert(open(randomfilename2));

		ID = "ABC";

		// Test read from an unopened file.

		// Recovers and succeeds if outside transactions.
		write("123", randomfilename1, ID);
		assert(read(RECORD, randomfilename1, ID));
		assert(deleterecord(randomfilename1, ID));
		assert(not read(RECORD, randomfilename1, ID));

		// Throws if in transaction because using PREPARED postgresql statements.
		// which are only done on opening and any errors terminate transactions.

		assert(begintrans());

		try {
			// Opened file should be OK
			write("123", randomfilename2, ID);

			// Unopened file
			write("123", randomfilename1, ID);
			assert(read(RECORD, randomfilename1, ID));
			assert(deleterecord(randomfilename1, ID));

			assert(false && "Read/write/delete on unopened file should THROW inside transaction.");

		} catch (VarError e) {
			// Verify throwm
			TRACE(e.message);
		}

		// In trans (but errored)
		assert(statustrans());

		// Commit trans with errors should rollback and return false.
		assert(not committrans());

		// Rollback should have occurred
		assert(not statustrans());
		assert(not read(RECORD, randomfilename2, ID));
		assert(not read(RECORD, randomfilename1, ID));
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

auto func_returning_three_vars(in v1, in delim) -> std::array<var,3> {
	return v1.unpack<3>(delim);
}

}; // programexit()
