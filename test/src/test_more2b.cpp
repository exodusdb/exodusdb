#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
//#include <functional>
//#include <limits>

#include <exodus/program.h>

programinit()

	function main() {
	printl("test_more2b says 'Hello World!'");

	{
		var v1 = 1;
		var v2 = 1;

		// eq eq
		assert(v1    eq v2);
		assert(v1    eq "1.0");
		assert(v1    eq '1');
		assert(v1    eq 1);
		assert(v1    eq 1.0);
		assert(v1    eq true);
		assert("1.0" eq v1);
		assert('1'   eq v2);
		assert(1     eq v2);
		assert(1.0   eq v2);
		assert(true  eq v2);

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
		assert((v1 + v2)    eq 2);
		assert((v1 + "1.0") eq 2);
		assert((v1 + '1')   eq 2);
		assert((v1 + 1)     eq 2);
		assert((v1 + 1.0)   eq 2);
		assert((v1 + true)  eq 2);
		assert(("1.0" + v2) eq 2);
		assert(('1' + v2)   eq 2);
		assert((1 + v2)     eq 2);
		assert((1.0 + v2)   eq 2);
		assert((true + v2)  eq 2);

		// - subtract
		assert((v1 - v2)    eq 0);
		assert((v1 - "1.0") eq 0);
		assert((v1 - '1')   eq 0);
		assert((v1 - 1)     eq 0);
		assert((v1 - 1.0)   eq 0);
		assert((v1 - true)  eq 0);
		assert(("1.0" - v2) eq 0);
		assert(('1' - v2)   eq 0);
		assert((1 - v2)     eq 0);
		assert((1.0 - v2)   eq 0);
		assert((true - v2)  eq 0);

		v1 = 2;
		v2 = 2;

		// * multiply
		assert((v1 * v2)    eq 4);
		assert((v1 * "2.0") eq 4);
		assert((v1 * '2')   eq 4);
		assert((v1 * 2)     eq 4);
		assert((v1 * 2.0)   eq 4);
		assert((v1 * true)  eq 2);
		assert(("2.0" * v2) eq 4);
		assert(('2' * v2)   eq 4);
		assert((2 * v2)     eq 4);
		assert((2.0 * v2)   eq 4);
		assert((true * v2)  eq 2);

		// / divide
		assert((v1 / v2)    eq 1);
		assert((v1 / "2.0") eq 1);
		assert((v1 / '2')   eq 1);
		assert((v1 / 2)     eq 1);
		assert((v1 / 2.0)   eq 1);
		assert((v1 / true)  eq 2);
		assert(("2.0" / v2) eq 1);
		assert(('2' / v2)   eq 1);
		assert((2 / v2)     eq 1);
		assert((2.0 / v2)   eq 1);
		assert((true / v2)  eq 0.5);

		// % modulo
		assert((v1 % v2)    eq 0);
		assert((v1 % "2.0") eq 0);
		assert((v1 % '2')   eq 0);
		assert((v1 % 2)     eq 0);
		assert((v1 % 2.0)   eq 0);
		assert((v1 % true)  eq 0);
		assert(("2.0" % v2) eq 0);
		assert(('2' % v2)   eq 0);
		assert((2 % v2)     eq 0);
		assert((2.0 % v2)   eq 0);
		assert((true % v2)  eq 1);

		//CONCATENATE
		// NB do *NOT* support concatenate with bool or vice versa!!!
		// to avoid compiler doing wrong precendence issue between ^ and logical operators

		// : concat
		assert((v1 ^ v2)    eq "22");
		assert((v1 ^ "2.0") eq "22.0");
		assert((v1 ^ '2')   eq "22");
		assert((v1 ^ 2)     eq "22");
		assert((v1 ^ 2.0)   eq "22");
		assert((v1 ^ true)  eq "21");
		assert(("2.0" ^ v2) eq "2.02");
		assert(('2' ^ v2)   eq "22");
		assert((2 ^ v2)     eq "22");
		assert((2.0 ^ v2)   eq "22");
		assert((true ^ v2)  eq "12");

		// unary + - !
		assert((+v1)  eq 2);
		assert((-v1)  eq -2);
		assert((!v1)  eq 0);
		assert((!!v1) eq 1);

		// ++ -- prefix and postfix
		assert(v1++ eq 2);
		assert(v1   eq 3);
		assert(++v1 eq 4);
		assert(v1-- eq 4);
		assert(v1   eq 3);
		assert(--v1 eq 2);

		//REPEAT with different v1 and v2

		v1 = 2;
		v2 = 3;

		// eq eq
		assert(!(v1    eq v2));
		assert(v1      eq "2.0");
		assert(v1      eq '2');
		assert(v1      eq 2);
		assert(v1      eq 2.0);

		//assert((v1 eq true));  //v1.toBool()
		assert((v1 ne false));
		assert((v1 ne true));

		assert(!("2.0" eq v2));
		assert(!('2'   eq v2));
		assert(!(2     eq v2));
		assert(!(2.0   eq v2));

		//assert(!(true != v2));	//v1.toBool()
		assert( (true != v2));
		assert( (false != v2));

		// != ne
		assert(v1 != v2);
		assert(v1 != "3.0");
		assert(v1 != '3');
		assert(v1 != 3);
		assert(v1 != 3.0);
		assert(!(v1   eq true));
		assert(  v1   ne true);
		assert(!(v1   eq false));
		assert(  v1   ne true);
		;  //v1.toBool()
		assert("2.0" != v2);
		assert('2' != v2);
		assert(2 != v2);
		assert(2.0 != v2);
		assert(!(true eq v2));
		assert(  true ne v2);
		assert(!(false eq v2));
		assert(  false ne v2);
		;

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
		assert(true < v2);

		// <= le
		assert((v1 <= v2));
		assert((v1 <= "3.0"));
		assert((v1 <= '3'));
		assert((v1 <= 3));
		assert((v1 <= 3.0));
		assert(!(v1 <= true));
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
		assert( (v1 > true));
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
		assert(!(true >= v2));

		// + add
		assert((v1 + v2)    eq 5);
		assert((v1 + "3.0") eq 5);
		assert((v1 + '3')   eq 5);
		assert((v1 + 3)     eq 5);
		assert((v1 + 3.0)   eq 5);
		assert((v1 + true)  eq 3);
		assert(("2.0" + v2) eq 5);
		assert(('2' + v2)   eq 5);
		assert((2 + v2)     eq 5);
		assert((2.0 + v2)   eq 5);
		assert((true + v2)  eq 4);

		// - subtract
		assert((v1 - v2)    eq -1);
		assert((v1 - "3.0") eq -1);
		assert((v1 - '3')   eq -1);
		assert((v1 - 3)     eq -1);
		assert((v1 - 3.0)   eq -1);
		assert((v1 - true)  eq 1);
		assert(("2.0" - v2) eq -1);
		assert(('2' - v2)   eq -1);
		assert((2 - v2)     eq -1);
		assert((2.0 - v2)   eq -1);
		assert((true - v2)  eq -2);

		// * multiply
		assert((v1 * v2)    eq 6);
		assert((v1 * "3.0") eq 6);
		assert((v1 * '3')   eq 6);
		assert((v1 * 3)     eq 6);
		assert((v1 * 3.0)   eq 6);
		assert((v1 * true)  eq 2);
		assert(("2.0" * v2) eq 6);
		assert(('2' * v2)   eq 6);
		assert((2 * v2)     eq 6);
		assert((2.0 * v2)   eq 6);
		assert((true * v2)  eq 3);

		// / divide
		TRACE((v1 / v2));
		assert((v1 / v2) eq 0.6666);  //both are right because exodus compares floating point numbers only within 0.0001
		assert((v1 / v2)    eq 0.6667);
		TRACE((v1 / "3.0"));
		assert((v1 / "3.0") eq 0.6667);
		assert((v1 / '3')   eq 0.6667);
		assert((v1 / 3)     eq 0.6667);
		assert((v1 / 3.0)   eq 0.6667);
		assert((v1 / true)  eq 2);
		assert(("2.0" / v2) eq 0.6667);
		assert(('2' / v2)   eq 0.6667);
		assert((2 / v2)     eq 0.6667);
		assert((2.0 / v2)   eq 0.6667);
		assert((true / v2)  eq 0.3333);

		// % modulo
		assert((v1 % v2)    eq 2);
		assert((v1 % "3.0") eq 2);
		assert((v1 % '3')   eq 2);
		assert((v1 % 3)     eq 2);
		assert((v1 % 3.0)   eq 2);
		assert((v1 % true)  eq 0);
		assert(("2.0" % v2) eq 2);
		assert(('2' % v2)   eq 2);
		assert((2 % v2)     eq 2);
		assert((2.0 % v2)   eq 2);
		assert((true % v2)  eq 1);

		//CONCATENATE
		// NB do *NOT* support concatenate with bool or vice versa!!!
		// to avoid compiler doing wrong precendence issue between ^ and logical operators

		// : concat
		assert((v1 ^ v2)    eq "23");
		assert((v1 ^ "3.0") eq "23.0");
		assert((v1 ^ '3')   eq "23");
		assert((v1 ^ 3)     eq "23");
		assert((v1 ^ 3.0)   eq "23");
		assert((v1 ^ true)  eq "21");
		assert(("2.0" ^ v2) eq "2.03");
		assert(('2' ^ v2)   eq "23");
		assert((2 ^ v2)     eq "23");
		assert((2.0 ^ v2)   eq "23");
		assert((true ^ v2)  eq "13");

		// unary + - !
		assert((+v1)  eq 2);
		assert((-v1)  eq -2);
		assert((!v1)  eq 0);
		assert((!!v1) eq 1);

		// ++ -- prefix and postfix
		assert(v1++ eq 2);
		assert(v1   eq 3);
		assert(++v1 eq 4);
		assert(v1-- eq 4);
		assert(v1   eq 3);
		assert(--v1 eq 2);

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
