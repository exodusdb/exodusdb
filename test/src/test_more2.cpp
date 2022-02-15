#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
//#include <functional>
//#include <limits>

#include <exodus/program.h>

programinit()

function main() {
	printl("test_more says 'Hello World!'");

	{
		var x=123.456;
		assert(double(x) == 123.456);
		assert(x.toDouble() == 123.456);
	}
	{
		var x=123;
		assert(int(x) == 123);
		assert(x.toInt() == 123);
	}

	//move constructor
	{
		var x=std::move(var("xyz"));
		assert(x == "xyz");
	}

	//move assign
	{
		var x;
		x=std::move(var("xyz"));
		assert(x == "xyz");
	}

	//++
	{
		var v=1;
		v++;
		assert(v.outputl("v=") == 2);
		v = 1;
		assert(v++ == 1);
		v = 1;
		assert(++v == 2);
	}
	{
		var v=1.1;
		v++;
		assert(v.outputl("v=") == 2.1);
		v = 1.1;
		assert(v++ == 1.1);
		v = 1.1;
		assert(++v == 2.1);
	}
	{
		var v="1";
		v++;
		assert(v.outputl("v=") == 2);
		v = "1";
		assert(v++ == 1);
		v = "1";
		assert(++v == 2);
	}
	{
		var v="1.1";
		v++;
		assert(v.outputl("v=") == 2.1);
		v = "1.1";
		assert(v++ == 1.1);
		v = "1.1";
		assert(++v == 2.1);
	}

	//v--
	{
		var v=3;
		v--;
		assert(v.outputl("v=") == 2);
		v = 3;
		assert(v-- == 3);
		v = 3;
		assert(--v == 2);
	}
	{
		var v=3.1;
		v--;
		assert(v.outputl("v=") == 2.1);
		v = 3.1;
		assert(v-- == 3.1);
		v = 3.1;
		assert(--v == 2.1);
	}
	{
		var v="3";
		v--;
		assert(v.outputl("v=") == 2);
		v = "3";
		assert(v-- == 3);
		v = "3";
		assert(--v == 2);
	}
	{
		var v="3.1";
		v--;
		assert(v.outputl("v=") == 2.1);
		v = "3.1";
		assert(v-- == 3.1);
		v = "3.1";
		assert(--v == 2.1);
	}

	{
		var x;

		//INCREMENT

		x=1;
		assert(x++ == 1);
		assert(x == 2);

		//assert(x.a(1)++ == 1);//does not compile
		//assert((x.a(1))++ == 1);//does not compile
		//assert(x == 2);

		x = 1;
		assert(++x == 2);
		assert(x == 2);

		x = 1;
		//assert(++x.a(1) == 2);//does not compile
		assert((++x).a(1) == 2);
		assert(x == 2);

		//DECREMENT

		x=1;
		assert(x-- == 1);
		assert(x == 0);

		//assert(x.a(1)-- == 1);//does not compile
		//assert((x.a(1))-- == 1);//does not compile
		assert(x == 0);

		x = 1;
		assert(--x == 0);
		assert(x == 0);

		x = 1;
		//assert(--x.a(1) == 2);//does not compile
		assert((--x).a(1) == 0);
		assert(x == 0);
	}

	//+=
	{
		var v = 3;
		v += 1;
		assert(v.outputl("v=") == 4);
		v = 3;
		v += 1.1;
		assert(v.outputl("v=") == 4.1);
		v = 3;
		v += "1.1";
		assert(v.outputl("v=") == 4.1);
		v = 3;
		v += '1';
		assert(v.outputl("v=") == 4);
		v = 3;
		v += true;
		assert(v.outputl("v=") == 4);
		v = 3;
		v += false;
		assert(v.outputl("v=") == 3);

		//
		v = 3;
		v += var(1);
		assert(v.outputl("v=") == 4);
		v = 3;
		v += var(1.1);
		assert(v.outputl("v=") == 4.1);
		v = 3;
		v += var("1.1");
		assert(v.outputl("v=") == 4.1);
	}
	{
		var v = 3.1;
		v += 1;
		assert(v.outputl("v=") == 4.1);
		v = 3.1;
		v += 1.1;
		assert(v.outputl("v=") == 4.2);
		v = 3.1;
		v += "1.1";
		assert(v.outputl("v=") == 4.2);
		v = 3;
		v += '1';
		assert(v.outputl("v=") == 4);
		v = 3.1;
		v += true;
		assert(v.outputl("v=") == 4.1);
		v = 3.1;
		v += false;
		assert(v.outputl("v=") == 3.1);

		//
		v = 3.1;
		v += var(1);
		assert(v.outputl("v=") == 4.1);
		v = 3.1;
		v += var(1.1);
		assert(v.outputl("v=") == 4.2);
		v = 3.1;
		v += var("1.1");
		assert(v.outputl("v=") == 4.2);
	}
	{
		var v = "3.1";
		v += 1;
		assert(v.outputl("v=") == 4.1);
		v = "3.1";
		v += 1.1;
		assert(v.outputl("v=") == 4.2);
		v = "3.1";
		v += "1.1";
		assert(v.outputl("v=") == 4.2);

		//
		v = "3.1";
		v += var(1);
		assert(v.outputl("v=") == 4.1);
		v = "3.1";
		v += var(1.1);
		assert(v.outputl("v=") == 4.2);
		v = "3.1";
		v += var("1.1");
		assert(v.outputl("v=") == 4.2);
	}

	//-=
	{
		var v = 3;
		v -= 1;
		assert(v.outputl("v=") == 2);
		v = 3;
		v -= 1.1;
		assert(v.outputl("v=") == 1.9);
		v = 3;
		v -= "1.1";
		assert(v.outputl("v=") == 1.9);
		v = 3;
		v -= '1';
		assert(v.outputl("v=") == 2);
		v = 3;
		v -= true;
		assert(v.outputl("v=") == 2);
		v = 3;
		v -= false;
		assert(v.outputl("v=") == 3);

		//
		v = 3;
		v -= var(1);
		assert(v.outputl("v=") == 2);
		v = 3;
		v -= var(1.1);
		assert(v.outputl("v=") == 1.9);
		v = 3;
		v -= var("1.1");
		assert(v.outputl("v=") == 1.9);
	}
	{
		var v = 3.1;
		v -= 1;
		assert(v.outputl("v=") == 2.1);
		v = 3.1;
		v -= 1.1;
		assert(v.outputl("v=") == 2.0);
		v = 3.1;
		v -= "1.1";
		assert(v.outputl("v=") == 2.0);
		v = 3.1;
		v -= '1';
		assert(v.outputl("v=") == 2.1);
		v = 3.1;
		v -= true;
		assert(v.outputl("v=") == 2.1);
		v = 3.1;
		v -= false;
		assert(v.outputl("v=") == 3.1);

		//
		v = 3.1;
		v -= var(1);
		assert(v.outputl("v=") == 2.1);
		v = 3.1;
		v -= var(1.1);
		assert(v.outputl("v=") == 2.0);
		v = 3.1;
		v -= var("1.1");
		assert(v.outputl("v=") == 2.0);
	}
	{
		var v = "3.1";
		v -= 1;
		assert(v.outputl("v=") == 2.1);
		v = "3.1";
		v -= 1.1;
		assert(v.outputl("v=") == 2.0);
		v = "3.1";
		v -= "1.1";
		assert(v.outputl("v=") == 2.0);
		v = "3.1";
		v += '1';
		assert(v.outputl("v=") == 4.1);
		v = "3.1";
		v += true;
		assert(v.outputl("v=") == 4.1);
		v = "3.1";
		v += false;
		assert(v.outputl("v=") == 3.1);

		//
		v = "3.1";
		v -= var(1);
		assert(v.outputl("v=") == 2.1);
		v = "3.1";
		v -= var(1.1);
		assert(v.outputl("v=") == 2.0);
		v = "3.1";
		v -= var("1.1");
		assert(v.outputl("v=") == 2.0);
	}


	//note that all the following call exactly the same constructor and construct in place with no copy or move
	{
		var x="x";
		var y("x");
		var z{"x"};
	}

	//constructing var from cstr (char*) including the tricky char 0
	// char 0 CANNOT be used. it and any following characters will be ignored
	{
		var x="88\099";
		//assert(x.len()==5);
		//assert(x.oconv("HEX") == "3838003939");
		//assert(x == "88\099");
		assert(x.len()==2);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "3838");
		assert(x == "88");
	}
	{
		var x("88\099");
		//assert(x.len()==5);
		//assert(x.oconv("HEX") == "3838003939");
		//assert(x == "88\099");
		assert(x.len()==2);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "3838");
		assert(x == "88");
	}
	{
		var x{"88\099"};
		//assert(x.len()==5);
		//assert(x.oconv("HEX") == "3838003939");
		//assert(x == "88\099");
		assert(x.len()==2);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "3838");
		assert(x == "88");
	}

	//constructing var from char including the tricky char 0
	// char 0 CAN be used.
	{
		var x='\0';
		assert(x.len()==1);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "00");
		assert(x == '\0');
		assert(x != "\0");//cannot include char 0 in cstr
	}
	{
		var x('\0');
		assert(x.len()==1);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "00");
		assert(x == '\0');
		assert(x != "\0");//cannot include char 0 in cstr
	}
	{
		var x{'\0'};
		assert(x.len()==1);
		TRACE(x.oconv("HEX"));
		assert(x.oconv("HEX") == "00");
		assert(x == '\0');
		assert(x != "\0");//cannot include char 0 in cstr
	}

	//std strings variables (lvalues)
	{
		std::string s = "xyz";
		var x = s;
		assert(x == "xyz");
	}
	{
		std::string s = "xyz";
		var x;
		x = s;
		assert(x == "xyz");
	}

	//std string temporaries (rvalues)
	{
		var x = std::string("xyz");
		assert(x == "xyz");
	}
	{
		var x;
		x = std::string("xyz");
		assert(x == "xyz");
	}

	//construct from memory block
	{
		var x("abcdef",3);
		assert(x=="abc");
	}

	{
		var x;
		x = 123;
		assert(x == 123);
		x = 123.456;
		assert(x == 123.456);
		x = "abc";
		assert(x == "abc");
		x = 'a';
		assert(x == 'a');
	}

	//^=
	{
		var x = "00";
		x ^= 123;
		assert(x == 123);
		x = "00";
		x ^= 123.456;
		assert(x == 123.456);
		x = "xy";
		x ^= "abc";
		assert(x == "xyabc");
		x = "xy";
		x ^= 'a';
		assert(x == "xya");
		x = "xy";
		x ^= std::string("a");
		assert(x == "xya");
	}

	//==
	{
		assert(var(1) == var(1));
		assert(var(1.1) == var(1.1));
		assert(1 == var(1));
		assert(1.1 == var(1.1));
		assert(var(1) == 1);
		assert(var(1.1) == 1.1);

		assert(var(1) == var("01"));
		assert(var(1.1) == var("01.1"));
		assert(1 == var("01"));
		assert(1.1 == var("01.1"));
		assert(var(1) == "01");
		assert(var(1.1) == "01.1");

		assert(var("01") == var(1));
		assert(var("01.1") == var(1.1));
		assert("01" == var(1));
		assert("01.1" == var(1.1));
		assert(var("01") == 1);
		assert(var("01.1") == 1.1);

		assert(var("1") == var("01"));
		assert(var("1.1") == var("01.1"));
		assert("1" == var("01"));
		assert("1.1" == var("01.1"));
		assert(var("1") == "01");
		assert(var("1.1") == "01.1");
		//
		assert(var(1) != var("01x"));
		assert(var(1.1) != var("01.1x"));
		assert(1 != var("01x"));
		assert(1.1 != var("01.1x"));
		assert(var(1) != "01x");
		assert(var(1.1) != "01.1x");

		assert(var("01x") != var(1));
		assert(var("01.1x") != var(1.1));
		assert("01x" != var(1));
		assert("01.1x" != var(1.1));
		assert(var("01x") != 1);
		assert(var("01.1x") != 1.1);

		assert(var("1x") != var("01x"));
		assert(var("1.1x") != var("01.1x"));
		assert("1x" != var("01x"));
		assert("1.1x" != var("01.1x"));
		assert(var("1x") != "01x");
		assert(var("1.1x") != "01.1x");

	}

	//<
	{
		assert(var(1) < var(2));
		assert(var(1.1) < var(2.1));
		assert(1 < var(2));
		assert(1.1 < var(2.1));
		assert(var(1) < 2);
		assert(var(1.1) < 2.1);

		assert(var(1) < var("02"));
		assert(var(1.1) < var("02.1"));
		assert(1 < var("02"));
		assert(1.1 < var("02.1"));
		assert(var(1) < "02");
		assert(var(1.1) < "02.1");

		assert(var("01") < var(2));
		assert(var("01.1") < var(2.1));
		assert("01" < var(2));
		assert("01.1" < var(2.1));
		assert(var("01") < 2);
		assert(var("01.1") < 2.1);

		assert(var("1") < var("02"));
		assert(var("1.1") < var("02.1"));
		assert("1" < var("02"));
		assert("1.1" < var("02.1"));
		assert(var("1") < "02");
		assert(var("1.1") < "02.1");
		//
		assert(var(1) < var("2x"));
		assert(var(1.1) < var("2.1x"));
		assert(1 < var("2x"));
		assert(1.1 < var("2.1x"));
		assert(var(1) < "2x");
		assert(var(1.1) < "2.1x");

		assert(var("01x") < var(2));
		assert(var("01.1x") < var(2.1));
		assert("01x" < var(2));
		assert("01.1x" < var(2.1));
		assert(var("01x") < 2);
		assert(var("01.1x") < 2.1);

		assert(var("1x") < var("2x"));
		assert(var("1.1x") < var("2.1x"));
		assert("1x" < var("2x"));
		assert("1.1x" < var("2.1x"));
		assert(var("1x") < "2x");
		assert(var("1.1x") < "2.1x");

	}

	{
		assert(!(var("22.1x") < 211.1));
	}

	assert(+var(123)    == 123);
	assert(+var(1.23)   == 1.23);
	assert(+var("1.23") == 1.23);

	assert(123  == +var(123)   );
	assert(1.23 == +var(1.23)  );
	assert(1.23 == +var("1.23"));

	assert(-var(123)    == -123);
	assert(-var(1.23)   == -1.23);
	assert(-var("1.23") == -1.23);
	assert(-123  == - var(123));
	assert(-1.23 == - var(1.23));
	assert(-1.23 == - var("1.23"));

	assert(var("") == var(""));
	assert(var("0") == var("0"));
	assert(var("00") == var("00"));
	assert(var("0") == var("00"));
	assert(var("0") == var("00"));
	assert(var("00") == var("0"));
	assert(var("0") == var("0.0"));
	assert(var("0.0") == var("0"));

	assert(var("") != var(0));
	assert(var(0) != var(""));

	assert(var("") < var(0));
	assert(var(0) > var(""));

	assert(!(var("") > var(0)));
	assert(!(var(0) < var("")));

	assert("xxx"_var == "xxx");
	assert(""_var == "");
	assert(123_var == "0123.0");
	assert(123.456_var == "0123.4560");

	var v1 = 1;
	var v2 = 1;

	// == eq
	assert(v1 == v2);
	assert(v1 == "1.0");
	assert(v1 == '1');
	assert(v1 == 1);
	assert(v1 == 1.0);
	assert(v1 == true);
	assert("1.0" == v1);
	assert('1' == v2);
	assert(1 == v2);
	assert(1.0 == v2);
	assert(true == v2);

	// != ne
	assert(!(v1 != v2));
	assert(!(v1 != "1.0"));
	assert(!(v1 != '1'));
	assert(!(v1 != 1));
	assert(!(v1 != 1.0));
	assert(!(v1 != true));
	assert(!("1.0" != v2));
	assert(!('1' != v2));
	assert(!(1 != v2));
	assert(!(1.0 != v2));
	assert(!(true != v2));

	// < lt
	assert(!(v1 < v2));
	assert(!(v1 < "1.0"));
	assert(!(v1 < '1'));
	assert(!(v1 < 1));
	assert(!(v1 < 1.0));
	assert(!(v1 < true));
	assert(!("1.0" < v2));
	assert(!('1' < v2));
	assert(!(1 < v2));
	assert(!(1.0 < v2));
	assert(!(true < v2));

	// <= le
	assert((v1 <= v2));
	assert((v1 <= "1.0"));
	assert((v1 <= '1'));
	assert((v1 <= 1));
	assert((v1 <= 1.0));
	assert((v1 <= true));
	assert(("1.0" <= v2));
	assert(('1' <= v2));
	assert((1 <= v2));
	assert((1.0 <= v2));
	assert((true <= v2));

	/// > lt
	assert(!(v1 > v2));
	assert(!(v1 > "1.0"));
	assert(!(v1 > '1'));
	assert(!(v1 > 1));
	assert(!(v1 > 1.0));
	assert(!(v1 > true));
	assert(!("1.0" > v2));
	assert(!('1' > v2));
	assert(!(1 > v2));
	assert(!(1.0 > v2));
	assert(!(true > v2));

	// >= ge
	assert((v1 >= v2));
	assert((v1 >= "1.0"));
	assert((v1 >= '1'));
	assert((v1 >= 1));
	assert((v1 >= 1.0));
	assert((v1 >= true));
	assert(("1.0" >= v2));
	assert(('1' >= v2));
	assert((1 >= v2));
	assert((1.0 >= v2));
	assert((true >= v2));

	{
		var v1 = 10;
		v1 += '1';
		assert(v1 eq 11);
		v1 += "1";
		assert(v1 eq 12);
		v1 -= '1';
		assert(v1 eq 11);
		v1 -= "1";
		assert(v1 eq 10);
//		v1 *= '2';
//		assert(v1 eq 20);
//		v1 /= '4';
//		assert(v1 eq 5);
	}

	// + add
	assert((v1 + v2) == 2);
	assert((v1 + "1.0") == 2);
	assert((v1 + '1') == 2);
	assert((v1 + 1) == 2);
	assert((v1 + 1.0) == 2);
	assert((v1 + true) == 2);
	assert(("1.0" + v2) == 2);
	assert(('1' + v2) == 2);
	assert((1 + v2) == 2);
	assert((1.0 + v2) == 2);
	assert((true + v2) == 2);

	// - subtract
	assert((v1 - v2) == 0);
	assert((v1 - "1.0") == 0);
	assert((v1 - '1') == 0);
	assert((v1 - 1) == 0);
	assert((v1 - 1.0) == 0);
	assert((v1 - true) == 0);
	assert(("1.0" - v2) == 0);
	assert(('1' - v2) == 0);
	assert((1 - v2) == 0);
	assert((1.0 - v2) == 0);
	assert((true - v2) == 0);

	v1 = 2;
	v2 = 2;

	// * multiply
	assert((v1 * v2) == 4);
	assert((v1 * "2.0") == 4);
	assert((v1 * '2') == 4);
	assert((v1 * 2) == 4);
	assert((v1 * 2.0) == 4);
	assert((v1 * true) == 2);
	assert(("2.0" * v2) == 4);
	assert(('2' * v2) == 4);
	assert((2 * v2) == 4);
	assert((2.0 * v2) == 4);
	assert((true * v2) == 2);

	// / divide
	assert((v1 / v2) == 1);
	assert((v1 / "2.0") == 1);
	assert((v1 / '2') == 1);
	assert((v1 / 2) == 1);
	assert((v1 / 2.0) == 1);
	assert((v1 / true) == 2);
	assert(("2.0" / v2) == 1);
	assert(('2' / v2) == 1);
	assert((2 / v2) == 1);
	assert((2.0 / v2) == 1);
	assert((true / v2) == 0.5);

	// % modulo
	assert((v1 % v2) == 0);
	assert((v1 % "2.0") == 0);
	assert((v1 % '2') == 0);
	assert((v1 % 2) == 0);
	assert((v1 % 2.0) == 0);
	assert((v1 % true) == 0);
	assert(("2.0" % v2) == 0);
	assert(('2' % v2) == 0);
	assert((2 % v2) == 0);
	assert((2.0 % v2) == 0);
	assert((true % v2) == 1);

    //CONCATENATE
    // NB do *NOT* support concatenate with bool or vice versa!!!
    // to avoid compiler doing wrong precendence issue between ^ and logical operators

	// : concat
	assert((v1 ^ v2) == "22");
	assert((v1 ^ "2.0") == "22.0");
	assert((v1 ^ '2') == "22");
	assert((v1 ^ 2) == "22");
	assert((v1 ^ 2.0) == "22");
	assert((v1 ^ true) == "21");
	assert(("2.0" ^ v2) == "2.02");
	assert(('2' ^ v2) == "22");
	assert((2 ^ v2) == "22");
	assert((2.0 ^ v2) == "22");
	assert((true ^ v2) == "12");

	// unary + - !
	assert((+v1) == 2);
	assert((-v1) == -2);
	assert((!v1) == 0);
	assert((!!v1) == 1);

	// ++ -- prefix and postfix
	assert(v1++ == 2);
	assert(v1 == 3);
	assert(++v1 == 4);
	assert(v1-- == 4);
	assert(v1 == 3);
	assert(--v1 == 2);

	//REPEAT with different v1 and v2

	v1 = 2;
	v2 = 3;

	// == eq
	assert(!(v1 == v2));
	assert(v1 == "2.0");
	assert(v1 == '2');
	assert(v1 == 2);
	assert(v1 == 2.0);
	assert((v1 == true));//v1.toBool()
	assert(!("2.0" == v2));
	assert(!('2' == v2));
	assert(!(2 == v2));
	assert(!(2.0 == v2));
	assert(!(true != v2));//v1.toBool()

	// != ne
	assert(v1 != v2);
	assert(v1 != "3.0");
	assert(v1 != '3');
	assert(v1 != 3);
	assert(v1 != 3.0);
	assert(v1 == true);;//v1.toBool()
	assert("2.0" != v2);
	assert('2' != v2);
	assert(2 != v2);
	assert(2.0 != v2);
	assert(true == v2);;

	// < lt
	assert((v1 < v2));
	assert((v1 < "3.0"));
	assert((v1 < '3'));
	assert((v1 < 3));
	assert((v1 < 3.0));
	assert(!(v1 < true));
	assert(("2.0" < v2));
	assert(('2' < v2));
	assert((2 < v2));
	assert((2.0 < v2));
	assert(!(true < v2));

	// <= le
	assert((v1 <= v2));
	assert((v1 <= "3.0"));
	assert((v1 <= '3'));
	assert((v1 <= 3));
	assert((v1 <= 3.0));
	assert((v1 <= true));
	assert(("2.0" <= v2));
	assert(('2' <= v2));
	assert((2 <= v2));
	assert((2.0 <= v2));
	assert((true <= v2));

	/// > lt
	assert(!(v1 > v2));
	assert(!(v1 > "3.0"));
	assert(!(v1 > '3'));
	assert(!(v1 > 3));
	assert(!(v1 > 3.0));
	assert(!(v1 > true));
	assert(!("2.0" > v2));
	assert(!('2' > v2));
	assert(!(2 > v2));
	assert(!(2.0 > v2));
	assert(!(true > v2));

	// >= ge
	assert(!(v1 >= v2));
	assert(!(v1 >= "3.0"));
	assert(!(v1 >= '3'));
	assert(!(v1 >= 3));
	assert(!(v1 >= 3.0));
	assert((v1 >= true));
	assert(!("2.0" >= v2));
	assert(!('2' >= v2));
	assert(!(2 >= v2));
	assert(!(2.0 >= v2));
	assert((true >= v2));

	// + add
	assert((v1 + v2) == 5);
	assert((v1 + "3.0") == 5);
	assert((v1 + '3') == 5);
	assert((v1 + 3) == 5);
	assert((v1 + 3.0) == 5);
	assert((v1 + true) == 3);
	assert(("2.0" + v2) == 5);
	assert(('2' + v2) == 5);
	assert((2 + v2) == 5);
	assert((2.0 + v2) == 5);
	assert((true + v2) == 4);

	// - subtract
	assert((v1 - v2) == -1);
	assert((v1 - "3.0") == -1);
	assert((v1 - '3') == -1);
	assert((v1 - 3) == -1);
	assert((v1 - 3.0) == -1);
	assert((v1 - true) == 1);
	assert(("2.0" - v2) == -1);
	assert(('2' - v2) == -1);
	assert((2 - v2) == -1);
	assert((2.0 - v2) == -1);
	assert((true - v2) == -2);

	// * multiply
	assert((v1 * v2) == 6);
	assert((v1 * "3.0") == 6);
	assert((v1 * '3') == 6);
	assert((v1 * 3) == 6);
	assert((v1 * 3.0) == 6);
	assert((v1 * true) == 2);
	assert(("2.0" * v2) == 6);
	assert(('2' * v2) == 6);
	assert((2 * v2) == 6);
	assert((2.0 * v2) == 6);
	assert((true * v2) == 3);

	// / divide
	TRACE((v1 / v2));
	assert((v1 / v2) == 0.6666);//both are right because exodus compares floating point numbers only within 0.0001
	assert((v1 / v2) == 0.6667);
	TRACE((v1 / "3.0"));
	assert((v1 / "3.0") == 0.6667);
	assert((v1 / '3') == 0.6667);
	assert((v1 / 3) == 0.6667);
	assert((v1 / 3.0) == 0.6667);
	assert((v1 / true) == 2);
	assert(("2.0" / v2) == 0.6667);
	assert(('2' / v2) == 0.6667);
	assert((2 / v2) == 0.6667);
	assert((2.0 / v2) == 0.6667);
	assert((true / v2) == 0.3333);

	// % modulo
	assert((v1 % v2) == 2);
	assert((v1 % "3.0") == 2);
	assert((v1 % '3') == 2);
	assert((v1 % 3) == 2);
	assert((v1 % 3.0) == 2);
	assert((v1 % true) == 0);
	assert(("2.0" % v2) == 2);
	assert(('2' % v2) == 2);
	assert((2 % v2) == 2);
	assert((2.0 % v2) == 2);
	assert((true % v2) == 1);

    //CONCATENATE
    // NB do *NOT* support concatenate with bool or vice versa!!!
    // to avoid compiler doing wrong precendence issue between ^ and logical operators

	// : concat
	assert((v1 ^ v2) == "23");
	assert((v1 ^ "3.0") == "23.0");
	assert((v1 ^ '3') == "23");
	assert((v1 ^ 3) == "23");
	assert((v1 ^ 3.0) == "23");
	assert((v1 ^ true) == "21");
	assert(("2.0" ^ v2) == "2.03");
	assert(('2' ^ v2) == "23");
	assert((2 ^ v2) == "23");
	assert((2.0 ^ v2) == "23");
	assert((true ^ v2) == "13");

	// unary + - !
	assert((+v1) == 2);
	assert((-v1) == -2);
	assert((!v1) == 0);
	assert((!!v1) == 1);

	// ++ -- prefix and postfix
	assert(v1++ == 2);
	assert(v1 == 3);
	assert(++v1 == 4);
	assert(v1-- == 4);
	assert(v1 == 3);
	assert(--v1 == 2);

	var i = 3;
	var d = 7.5;
	var z = "";
	var si = "8";
	var sd = "7.7";
	var sn = "xyz";

	//compare all different kinds of lhs with rhs double

	{
		//empty string is never equal to any double
		assert(var("") != 3.3);
		assert(var("") != 0.0);

		var xi = "";
		TRACE(xi.toInt())
		xi.dump("xi");
		assert(xi != 0.0);

		var xd = "";
		TRACE(xd.toDouble())
		xd.dump("xd");
		assert(xd != 0.0);

		assert(var(0.0) == 0.0);
		assert(var(3.3) == 3.3);
		assert(var(-3.3) == -3.3);
		assert(var(3.4) != 3.3);

		assert(var("0.0") == 0.0);
		assert(var("3.3") == 3.3);
		assert(var("-3.3") == -3.3);
		assert(var("3.4") != 3.3);

		assert(var(0) == 0.0);
		assert(var(0) != 3.3);
		assert(var(4) != 3.3);
		assert(var(3) == 3.0);
		assert(var(-3) == -3.0);

		assert(var("0") == 0.0);
		assert(var("0") != 3.3);
		assert(var("4") != 3.3);
		assert(var("3") == 3.0);
		assert(var("-3") == -3.0);

		assert(var("xxxx") != 3.0);

	}
	{
		//empty string is never equal to any double
		assert(var("") != 3);
		assert(var("") != 0);

		var xi = "";
		TRACE(xi.toInt())
		xi.dump("xi");
		assert(xi != 0);

		var xd = "";
		TRACE(xd.toDouble())
		xd.dump("xd");
		assert(xd != 0);

		assert(var(0) == 0);
		assert(var(0.) == 0);
		assert(var(3.) == 3);
		assert(var(-3.) == -3);
		assert(var(4.) != 3);

		assert(var("0.0") == 0);
		assert(var("3.") == 3);
		assert(var("-3.") == -3);
		assert(var("3.0") != 0);

		assert(var(0) == 0);
		assert(var(0) == -0);
		assert(var(0) != 3);
		assert(var(4) != 3);
		assert(var(3) == 3);
		assert(var(-3) == -3);

		assert(var("0") == 0);
		assert(var("-0") == 0);
		assert(var("0") != 3);
		assert(var("4") != 3);
		assert(var("3") == 3);
		assert(var("-3") == -3);

		assert(var("xxxx") != 3);
		assert(var("xxxx") != 0);
		assert(var("xxxx") != -0);

	}

	// Test comparison with bool true/false
	{
		assert(var("") == false);
		assert(var("0") == false);
		assert(var("0.") == false);
		assert(var("0.0") == false);

		assert(var("x") == true);
		assert(var("10") == true);
		assert(var("10.") == true);
		assert(var("10.0") == true);
	}
	{
		assert(false == var(""));
		assert(false == var("0"));
		assert(false == var("0."));
		assert(false == var("0.0"));

		assert(true == var("x"));
		assert(true == var("10"));
		assert(true == var("10."));
		assert(true == var("10.0"));
	}
	{
		assert(var("") != true);
		assert(var("0") != true);
		assert(var("0.") != true);
		assert(var("0.0") != true);

		assert(var("x") != false);
		assert(var("10") != false);
		assert(var("10.") != false);
		assert(var("10.0") != false);
	}
	{
		assert(true != var(""));
		assert(true != var("0"));
		assert(true != var("0."));
		assert(true != var("0.0"));

		assert(false != var("x"));
		assert(false != var("10"));
		assert(false != var("10."));
		assert(false != var("10.0"));
	}

	{
		var x=1;

		//////////
		//positive
		//////////

		//lvalue

		printl(x++,1);
		printl(x++, 2);
		printl(x, 3);
		printl(++ ++x, 4);
		printl(x, 4);

		x=1;
		assert(x++ == 1);
		assert(x == 2);
		assert(++x ==3);
		assert(x ==3);

		//rvalue

		x=1;
		//assert(x.a(1)++ == 1);//pointless. should not compile
		assert(x.a(1) == 1);
		//assert(++x.a(1) == 2);//will not compile because "." priority > "++"
		assert((++x).a(1) == 2);
		assert(x.a(1) == 2);

		//////////
		//negative
		//////////

		//lvalue

		x=1;
		assert(x-- == 1);
		assert(x == 0);
		assert(--x == -1);
		assert(x == -1);

		//rvalue

		x=1;
		//assert(x.a(1)-- == 1);//pointless. should not compile
		assert(x.a(1) == 1);
		//assert(--x.a(1) == 2);//will not compile because "." priority > "--"
		assert((--x).a(1) == 0);
		assert(x.a(1) == 0);

	}

	{
		assert('1' + var(1.1) == 2.1);
		assert("1" + var(1.1) == 2.1);
		assert("1.1" + var(1.3) == 2.4);
		assert(1 + var(2.1) == 3.1);
		assert(1.1 + var(2.1) == 3.2);

		assert(var(1.1) + '1' == 2.1);
		assert(var(1.1) + "1" == 2.1);
		assert(var(1.3) + "1.1" == 2.4);
		assert(var(2.1) + 1 == 3.1);
		assert(var(2.1) + 1.1 == 3.2);

		assert('1' - var(1.1) == -0.1);
		assert("1" - var(1.1) == -0.1);
		assert("1.1" - var(1.3) == -0.2);
		assert(1 - var(2.1) == -1.1);
		assert(1.1 - var(2.1) == -1.0 );

		assert(var(1.1) - '1' == 0.1);
		assert(var(1.1) - "1" == 0.1);
		assert(var(1.3) - "1.1" == 0.2);
		assert(var(2.1) - 1 == 1.1);
		assert(var(2.1) - 1.1 == 1.0);

	}

	printl("Test passed");

	return 0;
}

programexit()

