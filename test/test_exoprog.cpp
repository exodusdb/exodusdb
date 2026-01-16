#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

// Member functions on temporaries can std:move(*this) to return a temporary object for RVO instead of var&& which is subtley different
//struct x {
//
//x func1() & {
//	std::cout << "&" << std::endl;
//	return *this;
//}
////x&& func1() && {
//x func1() && { // better
//	std::cout << "&&" << std::endl;
//	return std::move(*this);
//}
//};

// Passing by pointer allows access to completely fake arrays
//void funcx(int* i1) {
//	i1[3] = 123;
//	return;
//}

func main() {

	{
		// testing oconv(x, "[NUMBER]")

		auto test = [&](in number, in format, in output, in input, in comment = "") {

			printl();
			printt(number, format, "oconv:", squote(output), "iconv:", squote(input), "(expected)", comment);
			printl();

			let output2 = oconv(number, format);
			let input2 = iconv(number, format);

			printt(number, format, "oconv:", squote(output2), "iconv:", squote(input2), "(actual)");
			printl();

			if (output2.squote() != output.squote()) return false;	// oconv
			if (input2.squote() != input.squote()) return false; 	// iconv

			return true;
		};

		assert(test("GARBAGE"		, "[NUMBER,2]"		, "0.00GARBAGE"		, "0.00GARBAGE"		, "iconv ignores non numerics"));

		assert(test(12345.6789	, "[NUMBER]"		, "12,345.6789"	, 12345.68		, "oconv defaults to no change in decimals. iconv defaults to 2 decimals"));
		assert(test(12345.6789	, "[NUMBER,3]"		, "12,345.679"	, "12345.679"	, ""));
		assert(test(12345.6		, "[NUMBER,3]"		, "12,345.600"	, "12345.600"	, ""));
		TRACE(BASEFMT)
		assert(test(12345.6789	, "[NUMBER,BASE]"	, "12,345.68"	, 12345.68		, ""));

		assert(test("12345.6789ABC"	, "[NUMBER,2]"	, "12,345.68ABC", "12345.68ABC", ""));

		assert(test("12,345.6789ABC"	, "[NUMBER]"	, "12,345.6789ABC"	, "12345.68ABC"	, "oconv cant handle commas, iconv removes commas"));

	// TODO
	//	assert(test("1,2345.6789ABC"	, "[NUMBER]"	, "12,345.6789ABC"	, "12345.6789ABC"	, "oconv cant handle commas, iconv removes commas"));
	//	assert(test("12,345.6789ABC"	, "[NUMBER,2]"	, "12,345.6789ABC"	, "12345.68ABC"		, "iconv doesnt change decimals if currency code"));

		assert(test("GARBAGE"			, "[NUMBER]"	, "0GARBAGE"		, "0.00GARBAGE"	, "oconv adds 0, iconv ndecs = 2"));

		assert(test(0					, "[NUMBER]"	, "0"				, "0.00"	, "iconv defaults to 2 decimals"));
		assert(test(0					, "[NUMBER,Z]"	, ""				, "0.00"	, "iconv doesnt respect the Z flag"));
		assert(test(""					, "[NUMBER,Z]"	, ""				, ""		, "iconv doesnt respect the Z flag"));
		assert(test("0.0000"			, "[NUMBER,0]"	, "0"				, "0"		, ""));
		assert(test("0.0000"			, "[NUMBER,2]"	, "0.00"			, "0.00"	, ""));

		// X or * = retain decimals on iconv
		assert(test("0.0000"			, "[NUMBER,X]"	, "0.0000"			, "0.0000"	, "X or * for iconv = retain decimals"));
		assert(test("0.000"				, "[NUMBER,X]"	, "0.000"			, "0.000"	, ""));
		assert(test("0.000ABC"			, "[NUMBER,X]"	, "0.000ABC"		, "0.000ABC", ""));
		assert(test("123,456.789ABC"	, "[NUMBER,X]"	, "123,456.789ABC"	, "123456.789ABC", ""));
		assert(test("123,456.789ABC"	, "[NUMBER,*]"	, "123,456.789ABC"	, "123456.789ABC", ""));

		// oconv not fixing commas on output
		assert(test("12,3456.789ABC"	, "[NUMBER,*]"	, "123,456.789ABC", "123456.789ABC", "iconv removes commas on input"));

		// reciprocals on input
		// reciprocals on output are weird TODO
		assert(test("/100"	, "[NUMBER,2]"	, "/100", "0.01", "oconv cant convert. iconv reciprocal"));
		assert(test("1/100"	, "[NUMBER,2]"	, "1/100", "0.01", "oconv cant convert. iconv reciprocal"));
		assert(test(".01"	, "[NUMBER,3]"	, "0.010", "0.010", "iconv reciprocal"));
		assert(test("1/100"	, "[NUMBER,3]"	, "1/100", "0.010", "oconv cant convert. iconv reciprocal."));
		assert(test("1/100"	, "[NUMBER]"	, "1/100", "0.01", "iconv reciprocal"));
	//	assert(test("1/100ABC"	, "[NUMBER,2]"	, "0.01ABC", "0.01ABC", "iconv reciprocal"));
		assert(test("1/100ABC"	, "[NUMBER,2]"	, "1/100ABC", "0.01ABC", "oconv cant convert. iconv reciprocal"));

	}

	{
		printl("Server and user on UTC");
		TRACE(TZ)
		TRACE(DATEFMT)
		printl(timedate2());

		printl();
		printl("User is ahead of UTC by 60 minutes");
		printl("Server is on UTC");
		TZ = "3600"_var;
		TRACE(TZ)
		printl(timedate2());

		printl();
		printl("User is on UTC");
		printl("Server is ahead of UTC by 60 minutes");
		TZ = "0^3600"_var;
		TRACE(TZ)
		printl(timedate2());

		printl();
		printl("Both User and Server are ahead of UTC by 60 minutes");
		TZ = "3600^3600"_var;
		TRACE(TZ)
		printl(timedate2());

	}

	{
		if (connect()) {
			TRACE(CURSOR)

			// VarDBException:read("dict.xo_clients^2", "name_and_type") File doesnt exist - Aborting.
			// 4: vardb.cpp:2708: if (!dictrec.read(actualdictfile, fieldname)) UNLIKELY {
			// 7: test_exoprog.cpp:133: if (not CURSOR.select("xo_clients by name_and_type"))

			// Unfortunately libdict_xo_clients.so is installed in /usr/local/lib not ~/lib so it is not detectable by libinfo
			// and anyway it may not be usable if compiling a new binary incompatible version of exodus
			//if (libinfo("dict_xo_clients")) {

			try {
				if (not CURSOR.select("xo_clients by name_and_type"))
					abort(lasterror());

				TRACE(CURSOR)
				var v1, v2, v3, v4;
				pushselect(v2);
				TRACE(CURSOR)
				popselect(v2);
				TRACE(CURSOR)

				while(CURSOR.readnext(ID))
					printl(ID);
			} catch (VarDBException e) {
				loglasterror();
			}
			//}

		}
	}

	{
		var cursor = getcursor();

		// print a box of numbers at col 5, row 10
		var row = 10, col = 40;
		printl();
		printx(AT(col, row), "+-----+-----+");
		for (var i : range(1, 5))
			printx(AT(col, row+i), "|12345|67890|");
		printx(AT(col, row + 6), "+-----+-----+");

		// Unfortunately returns to row 0 (top)
		setcursor(cursor);

	}
	if (isterminal()) {

		// Default getcursor to be deprecated
		var cursor = getcursor(); // Save the current cursor position.
		TRACE(cursor)             // Show the saved cursor position.
		if (cursor.f(1).empty())
			abort();
		assert(cursor.f(1).match("\\d+").len());    //x
		assert(cursor.f(2).match("\\d+").len());    //y
		assert(cursor.f(3).match("[\\d.]+").len()); //ms response time

		// New ND bool getcursor
		var cursor2;
		assert(getcursor(cursor2));
		TRACE(cursor2)
		if (cursor2.f(1).empty())
			abort();
		assert(cursor2.f(1).match("\\d+").len());    //x
		assert(cursor2.f(2).match("\\d+").len());    //y
		assert(cursor2.f(3).match("[\\d.]+").len()); //ms response time

		// Disable getcursor by setting max errors to -1
		var cursor3;
		assert(not getcursor(cursor3, 3000, -1));
		TRACE(cursor3)
		// Check disabled
		assert(cursor3.f(4) == "DISABLED");

		// Check it is disabled
		var cursor4;
		assert(not getcursor(cursor4));
		assert(cursor4.f(4) == "DISABLED");
		TRACE(cursor4)

		// Reenable getcursor by setting max errors to 3
		var cursor5;
		assert(getcursor(cursor5, 3000, 3));
		assert(cursor5.f(4) == "");
		TRACE(cursor5)

		// Check it is enabled
		var cursor6;
		assert(getcursor(cursor6));
		assert(cursor6.f(4) == "");
		TRACE(cursor6)

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
