#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <cmath>

#include <exodus/program.h>
programinit()

	function main() {
	printl("test_math says 'Hello World!'");

	gosub test_basic_ops();
	gosub test_mod();

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

subroutine test_mod() {

	//a	b	exodus	arev	remainder
	var moddata = R"(
-2	-2	0	0	0
-2	-1.5	-0.5	-0.5	-0.5
-2	-1	0	0	0
-2	-0.5	0	0	0
-2	0.5	0	0	0
-2	1	0	0	0
-2	1.5	1	1	-0.5
-2	2	0	0	0
-1.5	-2	-1.5	-1.5	0.5
-1.5	-1.5	0	0	0
-1.5	-1	-0.5	-0.5	0.5
-1.5	-0.5	0	0	0
-1.5	0.5	0	0	0
-1.5	1	0.5	0.5	0.5
-1.5	1.5	0	0	0
-1.5	2	0.5	0.5	0.5
-1	-2	-1	-1	-1
-1	-1.5	-1	-1	0.5
-1	-1	0	0	0
-1	-0.5	0	0	0
-1	0.5	0	0	0
-1	1	0	0	0
-1	1.5	0.5	0.5	0.5
-1	2	-1	1	-1
-0.5	-2	-0.5	-0.5	-0.5
-0.5	-1.5	-0.5	-0.5	-0.5
-0.5	-1	-0.5	-0.5	-0.5
-0.5	-0.5	0	0	0
-0.5	0.5	0	0	0
-0.5	1	0.5	0.5	-0.5
-0.5	1.5	1	1	-0.5
-0.5	2	1.5	1.5	-0.5
0	-2	0	0	0
0	-1.5	0	0	0
0	-1	0	0	0
0	-0.5	0	0	0
0	0.5	0	0	0
0	1	0	0	0
0	1.5	0	0	0
0	2	0	0	0
0.5	-2	-1.5	-1.5	0.5
0.5	-1.5	-1	-1	0.5
0.5	-1	-0.5	-0.5	0.5
0.5	-0.5	0	0	0
0.5	0.5	0	0	0
0.5	1	0.5	0.5	0.5
0.5	1.5	0.5	0.5	0.5
0.5	2	0.5	0.5	0.5
1	-2	1	-1	1
1	-1.5	-0.5	-0.5	-0.5
1	-1	0	0	0
1	-0.5	0	0	0
1	0.5	0	0	0
1	1	0	0	0
1	1.5	1	1	-0.5
1	2	1	1	1
1.5	-2	-0.5	-0.5	-0.5
1.5	-1.5	0	0	0
1.5	-1	-0.5	-0.5	-0.5
1.5	-0.5	0	0	0
1.5	0.5	0	0	0
1.5	1	0.5	0.5	-0.5
1.5	1.5	0	0	0
1.5	2	1.5	1.5	-0.5
2	-2	0	0	0
2	-1.5	-1	-1	0.5
2	-1	0	0	0
2	-0.5	0	0	0
2	0.5	0	0	0
2	1	0	0	0
2	1.5	0.5	0.5	0.5
2	2	0	0	0)";

	converter(moddata, "\n\r\t", FM ^ FM ^ VM);

	//PICKOS modulo limits the value to
	// [0 , limit) if limit is positive
	// (limit, 0] if limit is positive
	// unlike c++ which is acts more like a divisor/remainder function

#define USE_PICKOS_MODULUS
#ifdef USE_PICKOS_MODULUS
#define TARGET_COLN 4
#else
#define TARGET_COLN 3
#endif
	for (var line : moddata) {

		if (not line)
			continue;

		TRACE(line)
		line.outputl("line=");

		var a	   = line.f(1, 1);
		var b	   = line.f(1, 2);
		var target = line.f(1, TARGET_COLN);

		// var % var
		assert((a.outputl("a=") % b.outputl("b="))   eq target);
		assert(mod(a, b) eq target);

		// test symmetry around zero of limit
		assert((-a % -b)   eq -target);
		assert(mod(-a, -b) eq -target);

		if (index(b, ".")) {
			// var % double
			assert((a % double(b))   eq target);
			assert(mod(a, double(b)) eq target);

			//self assign
			{
				var c = a;
				c %= double(b);
				assert(c eq target);
			}

			// var % double symmetry
			assert((-a % -double(b))   eq -target);
			assert(mod(-a, -double(b)) eq -target);

			//self assign symmetry
			{
				var c = -a;
				c %= double(-b);
				assert(c eq -target);
			}

		} else {
			// var % int
			assert((a % int(b))   eq target);
			assert(mod(a, int(b)) eq target);

			//self assign
			{
				var c = a;
				c %= int(b);
				assert(c eq target);
			}

			// var % int symmetry
			assert((-a % -int(b))   eq -target);
			assert(mod(-a, -int(b)) eq -target);

			//self assign symmetry
			{
				var c = -a;
				c %= int(-b);
				assert(c eq -target);
			}
		}

		assert(std::remainder(line.f(1, 1).toDouble(), line.f(1, 2).toDouble()) eq line.f(1, 5));
	}

	return;
}

subroutine test_basic_ops() {

	//test all combinations of doubles and integers
	test("102.5", "5.7");
	test("102", "5.7");
	test("102.5", "5");
	test("102", "5");
	return;
}

subroutine test(in as, in bs) {

	//test all combinations of negatives
	test2(as, bs);
	test2("-" ^ as, bs);
	test2(as, "-" ^ bs);
	test2("-" ^ as, "-" ^ bs);

	return;
}

subroutine test2(in as, in bs) {

	var z;
	var ai = as;
	var bi = bs;
	printl(as, ai.toInt(), round(as, 0));
	//debug();
	printl(bs, bi.toInt(), round(bs, 0));
	//assert(ai.toInt() eq round(as,0));
	//assert(bi.toInt() eq round(bs,0));
	ai.toInt();
	bi.toInt();

	var va, vb, vc;

	//plus
	va = ai;
	vb = bi;
	vc = va + vb;
	z  = as + bs;
	printl(va, "+", vb, "=", vc, "add should be", z, vc eq z ? "ok" : "FAIL");
	assert(vc                                           eq z);

	//multiply
	va = ai;
	vb = bi;
	vc = va * vb;
	z  = as * bs;
	printl(va, "*", vb, "=", vc, "mul should be", z, vc eq z ? "ok" : "FAIL");
	assert(vc                                           eq z);

	//minus
	va = ai;
	vb = bi;
	vc = va - vb;
	z  = as - bs;
	printl(va, "-", vb, "=", vc, "sub should be", z, vc eq z ? "ok" : "FAIL");
	assert(vc                                           eq z);

	//divide
	va = ai;
	vb = bi;
	vc = va / vb;
	z  = as / bs;
	printl(va, "/", vb, "=", vc, "div be", z, vc eq z ? "ok" : "FAIL");
	assert(vc                                    eq z);

	//modulus
	va = ai;
	vb = bi;
	vc = va % vb;
	z  = as % bs;
	printl(va, "%", vb, "=", vc, "mod be", z, vc eq z ? "ok" : "FAIL");
	assert(vc                                    eq z);

	{
		//check cannot divide var decimal y var decimal 0
		try {
			var zero = 0.0;
			var x	 = 100.0;
			x /= zero;
			assert(false);
			//std::unreachable();
		} catch (VarDivideByZero&) {
		}
	}
	{
		//check cannot divide var decimal by var int 0
		try {
			var zero = 0;
			var x	 = 100.0;
			x /= zero;
			assert(false);
			//std::unreachable();
		} catch (VarDivideByZero&) {
		}
	}

	{
		//check cannot divide var int by var decimal 0
		try {
			var zero = 0.0;
			var x	 = 100;
			x /= zero;
			assert(false);
			//std::unreachable();
		} catch (VarDivideByZero&) {
		}
	}
	{
		//check cannot divide var int by var int 0
		try {
			var zero = 0;
			var x	 = 100;
			x /= zero;
			assert(false);
			//std::unreachable();
		} catch (VarDivideByZero&) {
		}
	}
	{
		//Check rhs conversion from string;
		var x = 100;
		x += var("1.12");
		x -= var("1.12");
		x *= var("1.12");
		x /= var("1.12");
		x %= var("1.12");
	}
	{
		//Check lhs conversion from string;
		var x = 100;
		x	  = "1.23";
		x += var("1.12");
		assert(x eq 1.23 + 1.12);
		x = "1.23";
		x -= var("1.12");
		assert(x eq 1.23 - 1.12);
		x = "1.23";
		x *= var("1.12");
		assert(x eq 1.23 * 1.12);
		x = "1.23";
		x /= var("1.12");
		assert(x eq 1.23 / 1.12);
		x = "1.23";
		x %= var("1.12");
		assert(x eq fmod(1.23, 1.12));

		x = "1.23";
		x += 1.12;
		assert(x eq 1.23 + 1.12);
		x = "1.23";
		x -= 1.12;
		assert(x eq 1.23 - 1.12);
		x = "1.23";
		x *= 1.12;
		assert(x eq 1.23 * 1.12);
		x = "1.23";
		x /= 1.12;
		assert(x eq 1.23 / 1.12);
		x = "1.23";
		x %= 1.12;
		assert(x eq fmod(1.23, 1.12));

		x = "1.23";
		x += 112;
		assert(x eq 1.23 + 112);
		x = "1.23";
		x -= 112;
		assert(x eq 1.23 - 112);
		x = "1.23";
		x *= 112;
		assert(x eq 1.23 * 112);
		x = "1.23";
		x /= 112;
		assert(x eq 1.23 / 112);
		x = "1.23";
		x %= 112;
		assert(x eq fmod(1.23, 112));
	}
	{

		//math.h seems to have been included in one of the boost or other special headers
		//in this main.cpp file and that causes confusion between math.h and exodus.h sin() and other functions.
		//we resolved the issue here by being specific about the namespace

		//trig on integers
		TRACE(exodus::sin(30))
		TRACE(exodus::cos(30))
		TRACE(exodus::tan(30))
		TRACE(exodus::atan(1))
		assert(exodus::sin(30).round(8)   eq 0.5);
		assert(exodus::cos(60).round(8)   eq 0.5);
		assert(exodus::tan(45).round(8)   eq 1);
		assert(exodus::atan(1).round(6)   eq 45);
		//trig on doubles
		assert(exodus::sin(30.0).round(8) eq 0.5);
		assert(exodus::cos(60.0).round(8) eq 0.5);
		assert(exodus::tan(45.0).round(8) eq 1);
		assert(exodus::atan(1.0).round(6) eq 45);

		// abs positives
		assert(exodus::abs(0)      eq 0);
		assert(exodus::abs(30)     eq 30);
		assert(exodus::abs(30.00)  eq 30);
		assert(exodus::abs(30.10)  eq 30.1);
		assert(exodus::abs(30.90)  eq 30.9);

		assert(exodus::abs(0.01) eq 0.01);
		assert(exodus::abs(0.001) eq 0.001);
		assert(exodus::abs(0.0001) eq 0.0001);
		assert(exodus::abs(0.00001) eq 0.00001);
		assert(exodus::abs(0.000001) eq 0.000001);
		assert(exodus::abs(0.0000001) eq 0.0000001);
		assert(exodus::abs(0.00000001) eq 0.00000001);
		assert(exodus::abs(0.000000001) eq 0.000000001);

		//abs negatives
		assert(exodus::abs(-0)     eq 0);
		assert(exodus::abs(-30)    eq 30);
		assert(exodus::abs(-30.00) eq 30);
		assert(exodus::abs(-30.10) eq 30.1);
		assert(exodus::abs(-30.90) eq 30.9);

		assert(exodus::abs(-0.01) eq 0.01);
		assert(exodus::abs(-0.001) eq 0.001);
		assert(exodus::abs(-0.0001) eq 0.0001);
		assert(exodus::abs(-0.00001) eq 0.00001);
		assert(exodus::abs(-0.000001) eq 0.000001);
		assert(exodus::abs(-0.0000001) eq 0.0000001);
		assert(exodus::abs(-0.00000001) eq 0.00000001);
		assert(exodus::abs(-0.000000001) eq 0.000000001);

		// exponents and roots on integers and doubls
		assert(exodus::exp(1).round(9)            eq 2.718281828);
		assert(exodus::exp(1.0).round(9)          eq 2.718281828);
		assert(exodus::loge(1)                    eq 0);
		assert(exodus::loge(2.718281828).round(9) eq 1);
		assert(exodus::loge(10).round(9)          eq 2.302585093);
		assert(exodus::sqrt(100)                  eq 10);
		assert(exodus::sqrt(100.0)                eq 10);

		var dividend = 100;
		TRACE(mod(dividend, 30))

		assert(mod(-4, 3) eq 2);
		assert(mod(-3, 3) eq 0);
		assert(mod(-2, 3) eq 1);
		assert(mod(-1, 3) eq 2);
		assert(mod(-0, 3) eq 0);
		assert(mod(0, 3)  eq 0);
		assert(mod(1, 3)  eq 1);
		assert(mod(2, 3)  eq 2);
		assert(mod(3, 3)  eq 0);
		assert(mod(4, 3)  eq 1);

		assert(mod(-4.0, 3) eq 2);
		assert(mod(-3.0, 3) eq 0);
		assert(mod(-2.0, 3) eq 1);
		assert(mod(-1.0, 3) eq 2);
		assert(mod(-0.0, 3) eq 0);
		assert(mod(0.0, 3)  eq 0);
		assert(mod(1.0, 3)  eq 1);
		assert(mod(2.0, 3)  eq 2);
		assert(mod(3.0, 3)  eq 0);
		assert(mod(4.0, 3)  eq 1);

		assert(mod(-4.0, 3.0) eq 2);
		assert(mod(-3.0, 3.0) eq 0);
		assert(mod(-2.0, 3.0) eq 1);
		assert(mod(-1.0, 3.0) eq 2);
		assert(mod(-0.0, 3.0) eq 0);
		assert(mod(0.0, 3.0)  eq 0);
		assert(mod(1.0, 3.0)  eq 1);
		assert(mod(2.0, 3.0)  eq 2);
		assert(mod(3.0, 3.0)  eq 0);
		assert(mod(4.0, 3.0)  eq 1);

		assert(mod(-4, 3.0) eq 2);
		assert(mod(-3, 3.0) eq 0);
		assert(mod(-2, 3.0) eq 1);
		assert(mod(-1, 3.0) eq 2);
		assert(mod(-0, 3.0) eq 0);
		assert(mod(0, 3.0)  eq 0);
		assert(mod(1, 3.0)  eq 1);
		assert(mod(2, 3.0)  eq 2);
		assert(mod(3, 3.0)  eq 0);
		assert(mod(4, 3.0)  eq 1);

		//negative dividend
		/*
		assert(mod(-4,-3)      eq -1);
		assert(mod(-3,-3)      eq 0);
		assert(mod(-2,-3)      eq -2);
		assert(mod(-1,-3)      eq -1);
		assert(mod(-0,-3)      eq 0);
		assert(mod(0,-3)       eq 0);
		TRACE(mod(1,-3));
		assert(mod(1,-3)       eq -2);
		assert(mod(2,-3)       eq -1);
		assert(mod(3,-3)       eq 0);
		assert(mod(4,-3)       eq -2);
	*/
		assert(mod(-4.0, -3.0) eq -1);
		assert(mod(-3.0, -3.0) eq 0);
		assert(mod(-2.0, -3.0) eq -2);
		assert(mod(-1.0, -3.0) eq -1);
		assert(mod(-0.0, -3.0) eq 0);
		assert(mod(0.0, -3.0)  eq 0);
		TRACE(mod(1.0, -3.0))
		TRACE(fmod(1.0, -3.0))
		TRACE(1 % -3)
		TRACE(-1 % 3)
		assert(mod(1.0, -3.0)  eq -2);
		assert(mod(2.0, -3.0)  eq -1);
		assert(mod(3.0, -3.0)  eq 0);
		assert(mod(4.0, -3.0)  eq -2);

		//check floating point modulo
		TRACE(mod(2.3, var(1.499)).round(3))
		assert(mod(2.3, var(1.499)).round(3)   eq 0.801);
		TRACE(mod(-2.3, var(-1.499)).round(3))
		assert(mod(-2.3, var(-1.499)).round(3) eq - 0.801);
		TRACE(mod(-2.3, var(1.499)).round(3))
		assert(mod(-2.3, var(1.499)).round(3)  eq 0.698);
		TRACE(mod(2.3, var(-1.499)).round(3))
		assert(mod(2.3, var(-1.499)).round(3)  eq - 0.698);
	}

	{

		// C++ double to int is symmetrical about 0
		// exodus is the same

		// c++ double to int
		assert(int(2.9)  eq 2);
		assert(int(-2.9) eq -2);

		// Using exodus' var::toInt
		assert(var(2.9).toInt()  eq 2);
		assert(var(-2.9).toInt() eq -2);

		// Using c++ int()
		assert(int(var(2.9))  eq 2);
		assert(int(var(-2.9)) eq -2);

		// Using exodus' integer()
		assert(integer(var(2.9))  eq 2);
		assert(integer(var(-2.9)) eq -2);

		//		// BUT pickos int()
		//		// is the floor function which is *not* symmetrical
		//		// float -> nearest integer towards negative infinity
		//		// 1.999 -> 1 and -1.001 -> -2
		//		assert(var(1.9999).toInt() eq 1);
		//		assert(var(-1.9999).toInt() eq -2);
		//
		//		// exodus round function is symmetrical
		//		// and rounds 0.5 up and -0.5 down
		//		// which is
		//		assert(var(1.5).toInt() eq 1);
		//		assert(var(-1.5).toInt() eq -2);

		// floor is not symmetrical
		assert(var(1.9999).floor()  eq 1);
		assert(var(-1.9999).floor() eq -2);

		// round function is symmetrical
		assert(var(1.5).round()  eq 2);
		assert(var(-1.5).round() eq -2);

		printl("More tests that var double to int is same as standard c++ static_cast i.e. truncate towards zero");

		printl("c++ static_cast double to int is truncate towards zero");
		assert(static_cast<int>(2.0) == 2);
		assert(static_cast<int>(-2.0) == -2);
		assert(static_cast<int>(2.1) == 2);
		assert(static_cast<int>(2.0) == 2);
		assert(static_cast<int>(2.1) == 2);
		assert(static_cast<int>(-2.1) == -2);
		assert(static_cast<int>(2.5) == 2);
		assert(static_cast<int>(-2.5) == -2);
		assert(static_cast<int>(2.9) == 2);
		assert(static_cast<int>(-2.9) == -2);
		assert(static_cast<int>(3.0) == 3);
		assert(static_cast<int>(-3.0) == -3);

		printl("var double to int is c++ truncate towards zero");
		assert(var(2.0).toInt() == 2);
		assert(var(-2.0).toInt() == -2);
		assert(var(2.1).toInt() == 2);
		assert(var(-2.1).toInt() == -2);
		assert(var(2.5).toInt() == 2);
		assert(var(-2.5).toInt() == -2);
		assert(var(2.9).toInt() == 2);
		assert(var(-2.9).toInt() == -2);
		assert(var(3.0).toInt() == 3);
		assert(var(-3.0).toInt() == -3);

		printl("implicit conversion to int(var double) is c++ truncate towards zero");
		assert(int(var(2.0)) == 2);
		assert(int(var(2.1)) == 2);
		assert(int(var(-2.1)) == -2);
		assert(int(var(2.5)) == 2);
		assert(int(var(-2.5)) == -2);
		assert(int(var(2.9)) == 2);
		assert(int(var(-2.9)) == -2);
		assert(int(var(3.0)) == 3);
		assert(int(var(-3.0)) == -3);
	}

	{
		//floor() (=== arev int())

		assert(floor(var(-3.5)) eq -4);
		assert(floor(var(-3))   eq -3);
		assert(floor(var(-2.5)) eq -3);
		assert(floor(var(-2))   eq -2);
		assert(floor(var(-1.5)) eq -2);
		assert(floor(var(-1))   eq -1);
		assert(floor(var(-.5))  eq -1);
		assert(floor(var(0))    eq 0);
		assert(floor(var(.5))   eq 0);
		assert(floor(var(1))    eq 1);
		assert(floor(var(1.5))  eq 1);
		assert(floor(var(2))    eq 2);
		assert(floor(var(2.5))  eq 2);
		assert(floor(var(3))    eq 3);
		assert(floor(var(3.5))  eq 3);

		//toInt truncates like c++ int(double)
		assert((var(-3.5)).toInt() eq -3);
		assert((var(-3)).toInt()   eq -3);
		assert((var(-2.5)).toInt() eq -2);
		assert((var(-2)).toInt()   eq -2);
		assert((var(-1.5)).toInt() eq -1);
		assert((var(-1)).toInt()   eq -1);
		assert((var(-.5)).toInt()  eq -0);
		assert((var(0)).toInt()    eq 0);
		assert((var(.5)).toInt()   eq 0);
		assert((var(1)).toInt()    eq 1);
		assert((var(1.5)).toInt()  eq 1);
		assert((var(2)).toInt()    eq 2);
		assert((var(2.5)).toInt()  eq 2);
		assert((var(3)).toInt()    eq 3);
		assert((var(3.5)).toInt()  eq 3);
	}
	{
		// Check that double is used in preference to int for lt/gt
		var v1 = 1.9;
		v1.toInt();
		printl(v1);
		printl(v1.dump());
		assert(v1 < 2);
		assert(v1 > 1);
		assert(2 > v1);
		assert(1 < v1);
	}
	{
		// Check that double is used in preference to int for lt/gt
		var v1 = -1.9;
		v1.toInt();
		printl(v1);
		printl(v1.dump());
		assert(v1 <  1);
		assert(v1 > -2);
		assert(1 > v1);
		assert(-2 < v1);
	}

	// Check that test for equality uses double by preference if available
	{
		var v1=1.8;
		printl(int(v1));
		TRACE(v1.dump())
		assert(v1 ne 1);
		assert(1 ne v1);
	}
	{
		var v1=-1.8;
		printl(int(v1));
		TRACE(v1.dump())
		assert(v1 ne 1);
		assert(1 ne v1);
	}

	return;
}

programexit()
