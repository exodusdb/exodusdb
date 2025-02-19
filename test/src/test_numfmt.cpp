#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#if EXO_MODULE
	import std;
#endif

using namespace std;

#include <exodus/program.h>
programinit()

function test(in str, in str2o = "") {

	printx(str, "==", str.quote());
	printl((str eq str) ? " is true" : " is false", "and ");

	//test self equality via string
	assert(str eq str);						   //hard to fail this
	assert(str.toString() eq str.toString());  //hard to fail this
	assert(str            eq str.toString());
	assert(str.toString() eq str);

	//test self equality via string
	//test numerical self equality (hard to fail except if non-numeric)
	assert((str + 0) eq (str + 0));

	//in pick/arev
	// 0 ne ''
	//and
	// '' |= 0
	TRACE(str)
	TRACE(double(str))
	if (len(str) eq 0) {
		assert((str + 0) ne str);
		assert(str ne (str + 0));
		assert((str - 0) ne str);
		assert(str ne (str - 0));
		assert(double(str) ne str);
		assert(str ne double(str));
		return 0;
	}

	//test numerical equality (auto conversion to num)
	assert((str + 0)   eq str);
	assert(str         eq (str + 0));
	assert((str - 0)   eq str);
	assert(str         eq (str - 0));
	assert(double(str) eq str);
	assert(str         eq double(str));

	var str2 = (len(str2o) gt 0) ? str2o : str;
	printx("\t\t\t\t", quote(str), " -> ", quote(str2));
	printl((quote(str + 0) eq quote(str2)) ? " is true" : " is false");
	//var x=str+0;
	//var y=quote(str+0);
	//var z=quote(str2);
	//if (quote(str+0) ne quote(str2))
	//	debug();
#ifdef EXO_USE_RYU
	assert(quote(str + 0) eq quote(str2));	//RYU converts 99.9+0 (99.90000000000000006) to 99.9
#else
	assert((str + 0) eq str2);	//numerical comparison to avoid 99.9+0 = 99.90000000000000006 without using RYU
#endif
	//var rounded_str = round(str,14)+0;
	//var x = str+0;
	//printl();
	//TRACE(x)
	//if (quote(str + 0) ne quote(str2))
	//	debug();
	//var a=quote(str);
	//var b=quote(str+0);
	//var c=quote(str2);
	//printl((quote(rounded_str) eq quote(str2))? " is true" : " is false");
	//assert(quote(rounded_str) eq quote(str2));

	return 0;
}

function main() {

	test("0", "0");
	test("00", "0");
	test("0000000000", "0");
	test("-0000000000", "0");
	test("-0", "0");
	test("-00", "0");
	test("+0", "0");
	test("+00", "0");

	test("0.0", "0");
	test("-0.0", "0");
	test("0.00", "0");
	test("-0.00", "0");
	test("00.0", "0");
	test("-00.0", "0");
	test("00.00", "0");
	test("-00.00", "0");
	test("0.", "0");
	test("-0.", "0");
	test(".0", "0");
	test("-.0", "0");

	test("");
	test("-0.", "0");
	test("0");
	test("-0", "0");
	test("0.0", "0");
	test("-0.0", "0");
	test("00000.00000", "0");
	test("-0000000000", "0");
	test("-00000.00000", "0");
	test("0000000000", "0");
	test("000008.76229", "8.76229");
	test("00000876229", "876229");
	test("+.09", "0.09");
	test("+09", "9");
	test("+09.", "9");
	test("-.09", "-0.09");
	test("-09", "-9");
	test("-09.", "-9");
	test(".09", "0.09");
	test("09", "9");
	test("09.", "9");
	test("+09.09", "9.09");
	test("-09.09", "-9.09");
	test("09.09", "9.09");
	test("11111");
	test("-11111.11");
	test("11111.11");
	test("12222");
	test("+.1239", "0.1239");
	test("+2", "2");
	test("-2");
	test("2");
	test("2208.76229");
	test("-.222", "-0.222");
	test("-2.22");
	test("-222");
	test("222");
	test("22222222");
	test("-2.27770", "-2.2777");
	test("8.76");
	test("8.76229");
	test("+.9", "0.9");
	test("+9", "9");
	test("+9.", "9");
	test("-.9", "-0.9");
	test("-9");
	test("-9.", "-9");
	test(".9", "0.9");
	test("9");
	test("9.", "9");
	test(".9000000000", "0.9");
	test("+.99", "0.99");
	test("+9.9", "9.9");
	test("+99", "99");
	test("+99.", "99");
	test("-.99", "-0.99");
	test("-9.9");
	test("-99");
	test("-99.", "-99");
	test(".99", "0.99");
	test("9.9");
	test("99");
	test("99.", "99");
	test("+999", "999");
	test("-999");
	test(".999", "0.999");
	test("9.99");
	test("99.9");
	test("999");
	test("999.", "999");
	test("9999");

// // isnum returns bool true or false
//
// // num   returns the original var IF isnum is true or 0 if the var is non-numeric
//          in other words, it forces numeric and non-numeric becomes zero.
//
//	isnum is true for all numeric vars which includes all strings that can be converted to numbers (which includes "" which is 0)
//
//	num is the same as isnum but zero (and "") is FALSE for num and TRUE for isnum

//	var        isnum   num   diff
//	---        -----   ---   ----
//	""         true    false *
//	0 or "0"   true    false *
//
//  1 or "1"   true    true
//  "x"        false   false

//	"" isnum but not num
//	"0" isnum but not num
//	"1" isnum and num
//	"x" not isnum and not num

	{
		var v1 = "";
		assert(isnum(v1));
		assert(not num(v1));
	}

	{
		var v1 = "0";
		assert(isnum(v1));
		assert(not num(v1));
	}

	{
		var v1 = "1.234";
		assert(isnum(v1));
		assert(num(v1));
	}

	{
		var v1 = "abc";
		assert(not isnum(v1));
		assert(not num(v1));
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
