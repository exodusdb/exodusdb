#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

using namespace std;

programinit()

function test(in str, in str20 = "") {

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

	var str2 = (len(str20) > 0) ? str20 : str;
	print("\t\t\t\t",quote(str)," -> ",quote(str2));
	printl((quote(str + 0) == quote(str2))? " is true" : " is false");
	assert(quote(str + 0) == quote(str2));

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

	printl("Test passed");

	return 0;
}

programexit()
