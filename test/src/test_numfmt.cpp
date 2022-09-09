#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

// 1. TO_CHARS from Ubuntu 22.04
#if __GNUC__ >= 11
#define USE_TO_CHARS_G

// 2. RYU
#elif __has_include(<ryu/ryu.h>)
#define USE_RYU

// 3. STRINGSTREAM
#else
#endif

#include <exodus/program.h>

using namespace std;

programinit()

function test(in str, in str2o = "") {

	print(str,"==",str.quote());
	printl((str == str)? " is true" : " is false","and ");

	//test self equality via string
	assert( str               ==  str);				//hard to fail this
	assert( str.toString()    ==  str.toString());	//hard to fail this
	assert( str               ==  str.toString());
	assert( str.toString()    ==  str);

	//test self equality via string
	//test numerical self equality (hard to fail except if non-numeric)
	assert((str+0) == (str + 0));

	//in pick/arev
	// 0 != ''
	//and
	// '' |= 0
	TRACE(str)
	TRACE(double(str))
	if (len(str) == 0) {
		assert((str + 0)   !=  str);
		assert( str        != (str + 0));
		assert((str - 0)   !=  str);
		assert( str        != (str - 0));
		assert(double(str) !=  str);
		assert( str        != double(str));
		return 0;
	}

	//test numerical equality (auto conversion to num)
	assert((str+0) ==  str);
	assert( str    == (str + 0));
	assert((str-0) ==  str);
	assert( str    == (str - 0));
	assert(double(str) ==  str);
	assert( str    == double(str));

	var str2 = (len(str2o) > 0) ? str2o : str;
	print("\t\t\t\t",quote(str)," -> ",quote(str2));
	printl((quote(str+0) == quote(str2))? " is true" : " is false");
	//var x=str+0;
	//var y=quote(str+0);
	//var z=quote(str2);
	//if (quote(str+0) != quote(str2))
	//	debug();
#ifdef USE_RYU
	assert(quote(str+0) == quote(str2));//RYU converts 99.9+0 (99.90000000000000006) to 99.9
#else
	assert((str+0) == str2);//numerical comparison to avoid 99.9+0 = 99.90000000000000006 without using RYU
#endif
	//var rounded_str = round(str,14)+0;
	//var x = str+0;
	//printl();
	//TRACE(x)
	//if (quote(str + 0) != quote(str2))
	//	debug();
	//var a=quote(str);
	//var b=quote(str+0);
	//var c=quote(str2);
	//printl((quote(rounded_str) == quote(str2))? " is true" : " is false");
	//assert(quote(rounded_str) == quote(str2));

	return 0;
}

function main() {


	test("0","0");
	test("00","0");
	test("0000000000","0");
	test("-0000000000","0");
	test("-0","0");
	test("-00","0");
	test("+0","0");
	test("+00","0");

	test("0.0","0");
	test("-0.0","0");
	test("0.00","0");
	test("-0.00","0");
	test("00.0","0");
	test("-00.0","0");
	test("00.00","0");
	test("-00.00","0");
	test("0.","0");
	test("-0.","0");
	test(".0","0");
	test("-.0","0");

	test("");
	test("-0.",			"0");
	test("0");
	test("-0", 			"0");
	test("0.0", 		"0");
	test("-0.0", 		"0");
	test("00000.00000",	"0");
	test("-0000000000",	"0");
	test("-00000.00000","0");
	test("0000000000",	"0");
	test("000008.76229","8.76229");
	test("00000876229", "876229");
	test("+.09",	"0.09");
	test("+09",		"9");
	test("+09.",	"9");
	test("-.09",	"-0.09");
	test("-09",		"-9");
	test("-09.",	"-9");
	test(".09",		"0.09");
	test("09",		"9");
	test("09.",		"9");
	test("+09.09",	"9.09");
	test("-09.09",	"-9.09");
	test("09.09",	"9.09");
	test("11111");
	test("-11111.11");
	test("11111.11");
	test("12222");
	test("+.1239",	"0.1239");
	test("+2",		"2");
	test("-2");
	test("2");
	test("2208.76229");
	test("-.222",	"-0.222");
	test("-2.22");
	test("-222");
	test("222");
	test("22222222");
	test("-2.27770","-2.2777");
	test("8.76");
	test("8.76229");
	test("+.9",		"0.9");
	test("+9",		"9");
	test("+9.",		"9");
	test("-.9",		"-0.9");
	test("-9");
	test("-9.",		"-9");
	test(".9",		"0.9");
	test("9");
	test("9.",		"9");
	test(".9000000000", "0.9");
	test("+.99",	"0.99");
	test("+9.9",	"9.9");
	test("+99",		"99");
	test("+99.",	"99");
	test("-.99",	"-0.99");
	test("-9.9");
	test("-99");
	test("-99.",	"-99");
	test(".99",		"0.99");
	test("9.9");
	test("99");
	test("99.",		"99");
	test("+999",	"999");
	test("-999");
	test(".999",	"0.999");
	test("9.99");
	test("99.9");
	test("999");
	test("999.",	"999");
	test("9999");

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
