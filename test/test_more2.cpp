#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#if EXO_MODULE
	import std;
#else
#	include <string>
#endif

#include <exodus/program.h>
programinit()

func main() {
	printl("test_more2 says 'Hello World!'");

	{
		var x = 123.456;
		assert(double(x)    eq 123.456);
		assert(x.toDouble() eq 123.456);
	}
	{
		var x = 123;
		assert(int(x)       eq 123);
		assert(x.toInt()    eq 123);
	}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpessimizing-move"
// GCC warning started in g++13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpessimizing-move"
	// Test move constructor
	{
		//warning: moving a temporary object prevents copy elision [-Wpessimizing-move]
//		var x = std::move(var("xyz"));
		var x = var("xyz").move();
		assert(x eq "xyz");
	}
	// Test move assign
	{
		var x;
		//warning: moving a temporary object prevents copy elision [-Wpessimizing-move]
//		x = std::move(var("xyz"));
		x = var("xyz").move();
		assert(x eq "xyz");
	}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

	//++
	{
		var v = 1;
		v++;
		assert(v.outputl("v=") eq 2);
		v = 1;
		assert(v++             eq 1);
		v = 1;
		assert(++v             eq 2);
	}
	{
		var v = 1.1;
		v++;
		assert(v.outputl("v=") eq 2.1);
		v = 1.1;
		assert(v++             eq 1.1);
		v = 1.1;
		assert(++v             eq 2.1);
	}
	{
		var v = "1";
		v++;
		assert(v.outputl("v=") eq 2);
		v = "1";
		assert(v++             eq 1);
		v = "1";
		assert(++v             eq 2);
	}
	{
		var v = "1.1";
		v++;
		assert(v.outputl("v=") eq 2.1);
		v = "1.1";
		assert(v++             eq 1.1);
		v = "1.1";
		assert(++v             eq 2.1);
	}

	//v--
	{
		var v = 3;
		v--;
		assert(v.outputl("v=") eq 2);
		v = 3;
		assert(v--             eq 3);
		v = 3;
		assert(--v             eq 2);
	}
	{
		var v = 3.1;
		v--;
		assert(v.outputl("v=") eq 2.1);
		v = 3.1;
		assert(v--             eq 3.1);
		v = 3.1;
		assert(--v             eq 2.1);
	}
	{
		var v = "3";
		v--;
		assert(v.outputl("v=") eq 2);
		v = "3";
		assert(v--             eq 3);
		v = "3";
		assert(--v             eq 2);
	}
	{
		var v = "3.1";
		v--;
		assert(v.outputl("v=") eq 2.1);
		v = "3.1";
		assert(v--             eq 3.1);
		v = "3.1";
		assert(--v             eq 2.1);
	}

	{
		var x;

		//INCREMENT

		x = 1;
		assert(x++ eq 1);
		assert(x   eq 2);

		assert(x.f(1).outputl() eq 2);

		//assert(x.f(1)++.outputl() eq 3); // should not compile
		//assert((x.f(1))++.outputl() eq 3); // should not compile
		//assert(x.f(1)++ eq 3);

		x = 1;
		assert(++x eq 2);
		assert(x   eq 2);

		x = 1;
		//assert(++x.f(1) eq 2);//does not compile
		//assert((++x).f(1) eq 2); // SADLY WILL NOT COMPILE because ++x produces a var& and ++ is implemented in var_base ATM
		//assert(x          eq 2);

		//DECREMENT

		x = 1;
		assert(x-- eq 1);
		assert(x   eq 0);

		//assert(x.f(1)-- eq 1);//should not compile
		//assert((x.f(1))-- eq 1);//should not compile
		assert(x eq 0);

		x = 1;
		assert(--x eq 0);
		assert(x   eq 0);

		x = 1;
		//assert(--x.f(1) eq 2);//does not compile
		//assert((--x).f(1) eq 0); // SADLY WILL NOT COMPILE because ++x produces a var& and ++ is implemented in var_base ATM
		//assert(x          eq 0);
	}

	//+=
	{
		var v = 3;
		v += 1;
		assert(v.outputl("v=") eq 4);
		v = 3;
		v += 1.1;
		assert(v.outputl("v=") eq 4.1);
		v = 3;
		v += "1.1";
		assert(v.outputl("v=") eq 4.1);
		v = 3;
		v += '1';
		assert(v.outputl("v=") eq 4);
		v = 3;
		v += true;
		assert(v.outputl("v=") eq 4);
		v = 3;
		v += false;
		assert(v.outputl("v=") eq 3);

		//
		v = 3;
		v += var(1);
		assert(v.outputl("v=") eq 4);
		v = 3;
		v += var(1.1);
		assert(v.outputl("v=") eq 4.1);
		v = 3;
		v += var("1.1");
		assert(v.outputl("v=") eq 4.1);
	}
	{
		var v = 3.1;
		v += 1;
		assert(v.outputl("v=") eq 4.1);
		v = 3.1;
		v += 1.1;
		assert(v.outputl("v=") eq 4.2);
		v = 3.1;
		v += "1.1";
		assert(v.outputl("v=") eq 4.2);
		v = 3;
		v += '1';
		assert(v.outputl("v=") eq 4);
		v = 3.1;
		v += true;
		assert(v.outputl("v=") eq 4.1);
		v = 3.1;
		v += false;
		assert(v.outputl("v=") eq 3.1);

		//
		v = 3.1;
		v += var(1);
		assert(v.outputl("v=") eq 4.1);
		v = 3.1;
		v += var(1.1);
		assert(v.outputl("v=") eq 4.2);
		v = 3.1;
		v += var("1.1");
		assert(v.outputl("v=") eq 4.2);
	}
	{
		var v = "3.1";
		v += 1;
		assert(v.outputl("v=") eq 4.1);
		v = "3.1";
		v += 1.1;
		assert(v.outputl("v=") eq 4.2);
		v = "3.1";
		v += "1.1";
		assert(v.outputl("v=") eq 4.2);

		//
		v = "3.1";
		v += var(1);
		assert(v.outputl("v=") eq 4.1);
		v = "3.1";
		v += var(1.1);
		assert(v.outputl("v=") eq 4.2);
		v = "3.1";
		v += var("1.1");
		assert(v.outputl("v=") eq 4.2);
	}

	//-=
	{
		var v = 3;
		v -= 1;
		assert(v.outputl("v=") eq 2);
		v = 3;
		v -= 1.1;
		assert(v.outputl("v=") eq 1.9);
		v = 3;
		v -= "1.1";
		assert(v.outputl("v=") eq 1.9);
		v = 3;
		v -= '1';
		assert(v.outputl("v=") eq 2);
		v = 3;
		v -= true;
		assert(v.outputl("v=") eq 2);
		v = 3;
		v -= false;
		assert(v.outputl("v=") eq 3);

		//
		v = 3;
		v -= var(1);
		assert(v.outputl("v=") eq 2);
		v = 3;
		v -= var(1.1);
		assert(v.outputl("v=") eq 1.9);
		v = 3;
		v -= var("1.1");
		assert(v.outputl("v=") eq 1.9);
	}
	{
		var v = 3.1;
		v -= 1;
		assert(v.outputl("v=") eq 2.1);
		v = 3.1;
		v -= 1.1;
		assert(v.outputl("v=") eq 2.0);
		v = 3.1;
		v -= "1.1";
		assert(v.outputl("v=") eq 2.0);
		v = 3.1;
		v -= '1';
		assert(v.outputl("v=") eq 2.1);
		v = 3.1;
		v -= true;
		assert(v.outputl("v=") eq 2.1);
		v = 3.1;
		v -= false;
		assert(v.outputl("v=") eq 3.1);

		//
		v = 3.1;
		v -= var(1);
		assert(v.outputl("v=") eq 2.1);
		v = 3.1;
		v -= var(1.1);
		assert(v.outputl("v=") eq 2.0);
		v = 3.1;
		v -= var("1.1");
		assert(v.outputl("v=") eq 2.0);
	}
	{
		var v = "3.1";
		v -= 1;
		assert(v.outputl("v=") eq 2.1);
		v = "3.1";
		v -= 1.1;
		assert(v.outputl("v=") eq 2.0);
		v = "3.1";
		v -= "1.1";
		assert(v.outputl("v=") eq 2.0);
		v = "3.1";
		v += '1';
		assert(v.outputl("v=") eq 4.1);
		v = "3.1";
		v += true;
		assert(v.outputl("v=") eq 4.1);
		v = "3.1";
		v += false;
		assert(v.outputl("v=") eq 3.1);

		//
		v = "3.1";
		v -= var(1);
		assert(v.outputl("v=") eq 2.1);
		v = "3.1";
		v -= var(1.1);
		assert(v.outputl("v=") eq 2.0);
		v = "3.1";
		v -= var("1.1");
		assert(v.outputl("v=") eq 2.0);
	}

	//note that all the following call exactly the same constructor and construct in place with no copy or move
	{
		var x = "x";
		var y("x");
		var z{"x"};
	}

	//constructing var from cstr (char*) including the tricky char 0
	// char 0 CANNOT be used. it and any following characters will be ignored
	{
		var x = "88\099";
		//assert(x.len()==5);
		//assert(x.oconv("HEX") eq "3838003939");
		//assert(x eq "88\099");
		assert(x.len()        eq 2);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "3838");
		assert(x              eq "88");
	}
	{
		var x("88\099");
		//assert(x.len()==5);
		//assert(x.oconv("HEX") eq "3838003939");
		//assert(x eq "88\099");
		assert(x.len()        eq 2);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "3838");
		assert(x              eq "88");
	}
	{
		var x{"88\099"};
		//assert(x.len()==5);
		//assert(x.oconv("HEX") eq "3838003939");
		//assert(x eq "88\099");
		assert(x.len()        eq 2);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "3838");
		assert(x              eq "88");
	}

	//constructing var from char including the tricky char 0
	// char 0 CAN be used.
	{
		var x = '\0';
		assert(x.len()        eq 1);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "00");
		assert(x              eq '\0');
		assert(x ne "\0");	//cannot include char 0 in cstr
	}
	{
		var x('\0');
		assert(x.len()        eq 1);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "00");
		assert(x              eq '\0');
		assert(x ne "\0");	//cannot include char 0 in cstr
	}
	{
		var x{'\0'};
		assert(x.len()        eq 1);
		TRACE(x.oconv("HEX"))
		assert(x.oconv("HEX") eq "00");
		assert(x              eq '\0');
		assert(x ne "\0");	//cannot include char 0 in cstr
	}

	//std strings variables (lvalues)
	{
		std::string s = "xyz";
		var			x = s;
		assert(x eq "xyz");
	}
	{
		std::string s = "xyz";
		var			x;
		x = s;
		assert(x eq "xyz");
	}

	//std string temporaries (rvalues)
	{
		var x = std::string("xyz");
		assert(x eq "xyz");
	}
	{
		var x;
		x = std::string("xyz");
		assert(x eq "xyz");
	}

	//construct from memory block
	{
		var x("abcdef", 3);
		assert(x eq "abc");
	}

	{
		var x;
		x = 123;
		assert(x eq 123);
		x = 123.456;
		assert(x eq 123.456);
		x = "abc";
		assert(x eq "abc");
		x = 'a';
		assert(x eq 'a');
	}

	//^=
	{
		var x = "00";
		x ^= 123;
		assert(x eq 123);
		x = "00";
		x ^= 123.456;
		assert(x eq 123.456);
		x = "xy";
		x ^= "abc";
		assert(x eq "xyabc");
		x = "xy";
		x ^= 'a';
		assert(x eq "xya");
		x = "xy";
		x ^= std::string("a");
		assert(x eq "xya");
	}

	//==
	{
		assert(var(1)   eq var(1));
		assert(var(1.1) eq var(1.1));
		assert(1        eq var(1));
		assert(1.1      eq var(1.1));
		assert(var(1)   eq 1);
		assert(var(1.1) eq 1.1);

		assert(var(1)   eq var("01"));
		assert(var(1.1) eq var("01.1"));
		assert(1        eq var("01"));
		assert(1.1      eq var("01.1"));
		assert(var(1)   eq "01");
		assert(var(1.1) eq "01.1");

		assert(var("01")   eq var(1));
		assert(var("01.1") eq var(1.1));
		assert("01"        eq var(1));
		assert("01.1"      eq var(1.1));
		assert(var("01")   eq 1);
		assert(var("01.1") eq 1.1);

		assert(var("1")   eq var("01"));
		assert(var("1.1") eq var("01.1"));
		assert("1"        eq var("01"));
		assert("1.1"      eq var("01.1"));
		assert(var("1")   eq "01");
		assert(var("1.1") eq "01.1");
		//
		assert(var(1) ne var("01x"));
		assert(var(1.1) ne var("01.1x"));
		assert(1 ne var("01x"));
		assert(1.1 ne var("01.1x"));
		assert(var(1) ne "01x");
		assert(var(1.1) ne "01.1x");

		assert(var("01x") ne var(1));
		assert(var("01.1x") ne var(1.1));
		assert("01x" ne var(1));
		assert("01.1x" ne var(1.1));
		assert(var("01x") ne 1);
		assert(var("01.1x") ne 1.1);

		assert(var("1x") ne var("01x"));
		assert(var("1.1x") ne var("01.1x"));
		assert("1x" ne var("01x"));
		assert("1.1x" ne var("01.1x"));
		assert(var("1x") ne "01x");
		assert(var("1.1x") ne "01.1x");
	}

	{
		assert(oconv('\x00', "HEX") eq "00");
		assert(oconv('\x01', "HEX") eq "01");
		assert(oconv('\x02', "HEX") eq "02");
		assert(oconv('\x03', "HEX") eq "03");
		assert(oconv('\x04', "HEX") eq "04");
		assert(oconv('\x05', "HEX") eq "05");
		assert(oconv('\x06', "HEX") eq "06");
		assert(oconv('\x07', "HEX") eq "07");
		assert(oconv('\x08', "HEX") eq "08");
		assert(oconv('\x09', "HEX") eq "09");
		assert(oconv('\x0a', "HEX") eq "0A");
		assert(oconv('\x0b', "HEX") eq "0B");
		assert(oconv('\x0c', "HEX") eq "0C");
		assert(oconv('\x0d', "HEX") eq "0D");
		assert(oconv('\x0e', "HEX") eq "0E");
		assert(oconv('\x0f', "HEX") eq "0F");

		assert(oconv('\x10', "HEX") eq "10");
		assert(oconv('\x11', "HEX") eq "11");
		assert(oconv('\x12', "HEX") eq "12");
		assert(oconv('\x13', "HEX") eq "13");
		assert(oconv('\x14', "HEX") eq "14");
		assert(oconv('\x15', "HEX") eq "15");
		assert(oconv('\x16', "HEX") eq "16");
		assert(oconv('\x17', "HEX") eq "17");
		assert(oconv('\x18', "HEX") eq "18");
		assert(oconv('\x19', "HEX") eq "19");
		//TRACE( oconv('\x1a', "HEX")        );
		assert(oconv('\x1a', "HEX") eq "1A");
		assert(oconv('\x1b', "HEX") eq "1B");
		assert(oconv('\x1c', "HEX") eq "1C");
		assert(oconv('\x1d', "HEX") eq "1D");
		assert(oconv('\x1e', "HEX") eq "1E");
		assert(oconv('\x1f', "HEX") eq "1F");

		assert(oconv('\x20', "HEX") eq "20");
		assert(oconv('\x21', "HEX") eq "21");
		assert(oconv('\x22', "HEX") eq "22");
		assert(oconv('\x23', "HEX") eq "23");
		assert(oconv('\x24', "HEX") eq "24");
		assert(oconv('\x25', "HEX") eq "25");
		assert(oconv('\x26', "HEX") eq "26");
		assert(oconv('\x27', "HEX") eq "27");
		assert(oconv('\x28', "HEX") eq "28");
		assert(oconv('\x29', "HEX") eq "29");
		assert(oconv('\x2a', "HEX") eq "2A");
		assert(oconv('\x2b', "HEX") eq "2B");
		assert(oconv('\x2c', "HEX") eq "2C");
		assert(oconv('\x2d', "HEX") eq "2D");
		assert(oconv('\x2e', "HEX") eq "2E");
		assert(oconv('\x2f', "HEX") eq "2F");

		assert(oconv('\xf0', "HEX") eq "F0");
		assert(oconv('\xf1', "HEX") eq "F1");
		assert(oconv('\xf2', "HEX") eq "F2");
		assert(oconv('\xf3', "HEX") eq "F3");
		assert(oconv('\xf4', "HEX") eq "F4");
		assert(oconv('\xf5', "HEX") eq "F5");
		assert(oconv('\xf6', "HEX") eq "F6");
		assert(oconv('\xf7', "HEX") eq "F7");
		assert(oconv('\xf8', "HEX") eq "F8");
		assert(oconv('\xf9', "HEX") eq "F9");
		assert(oconv('\xfa', "HEX") eq "FA");
		assert(oconv('\xfb', "HEX") eq "FB");
		assert(oconv('\xfc', "HEX") eq "FC");
		assert(oconv('\xfd', "HEX") eq "FD");
		assert(oconv('\xfe', "HEX") eq "FE");
		assert(oconv('\xff', "HEX") eq "FF");
	}

	// <
	{
		assert(var(1) lt var(2));
		assert(var(1.1) lt var(2.1));
		assert(1 lt var(2));
		assert(1.1 lt var(2.1));
		assert(var(1) lt 2);
		assert(var(1.1) lt 2.1);

		assert(var(1) lt var("02"));
		assert(var(1.1) lt var("02.1"));
		assert(1 lt var("02"));
		assert(1.1 lt var("02.1"));
		assert(var(1) lt "02");
		assert(var(1.1) lt "02.1");

		assert(var("01") lt var(2));
		assert(var("01.1") lt var(2.1));
		assert("01" lt var(2));
		assert("01.1" lt var(2.1));
		assert(var("01") lt 2);
		assert(var("01.1") lt 2.1);

		assert(var("1") lt var("02"));
		assert(var("1.1") lt var("02.1"));
		assert("1" lt var("02"));
		assert("1.1" lt var("02.1"));
		assert(var("1") lt "02");
		assert(var("1.1") lt "02.1");
		//
		assert(var(1) lt var("2x"));
		assert(var(1.1) lt var("2.1x"));
		assert(1 lt var("2x"));
		assert(1.1 lt var("2.1x"));
		assert(var(1) lt "2x");
		assert(var(1.1) lt "2.1x");

		assert(var("01x") lt var(2));
		assert(var("01.1x") lt var(2.1));
		assert("01x" lt var(2));
		assert("01.1x" lt var(2.1));
		assert(var("01x") lt 2);
		assert(var("01.1x") lt 2.1);

		assert(var("1x") lt var("2x"));
		assert(var("1.1x") lt var("2.1x"));
		assert("1x" lt var("2x"));
		assert("1.1x" lt var("2.1x"));
		assert(var("1x") lt "2x");
		assert(var("1.1x") lt "2.1x");
	}

	{
		assert(!(var("22.1x") lt 211.1));
	}

	assert(+var(123)    eq 123);
	assert(+var(1.23)   eq 1.23);
	assert(+var("1.23") eq 1.23);

	assert(123  eq +var(123));
	assert(1.23 eq +var(1.23));
	assert(1.23 eq +var("1.23"));

	assert(-var(123)    eq -123);
	assert(-var(1.23)   eq -1.23);
	assert(-var("1.23") eq -1.23);
	assert(-123         eq -var(123));
	assert(-1.23        eq -var(1.23));
	assert(-1.23        eq -var("1.23"));

	assert(var("")    eq var(""));
	assert(var("0")   eq var("0"));
	assert(var("00")  eq var("00"));
	assert(var("0")   eq var("00"));
	assert(var("0")   eq var("00"));
	assert(var("00")  eq var("0"));
	assert(var("0")   eq var("0.0"));
	assert(var("0.0") eq var("0"));

	assert(var("") ne var(0));
	assert(var(0) ne var(""));

	assert(var("") lt var(0));
	assert(var(0) gt var(""));

	assert(!(var("") gt var(0)));
	assert(!(var(0) lt var("")));

	assert("xxx"_var   eq "xxx");
	assert(""_var      eq "");
	assert(123_var     eq "0123.0");
	assert(123.456_var eq "0123.4560");

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
