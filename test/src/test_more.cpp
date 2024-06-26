#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <functional>
#include <limits>

#include <string>  //for ""_s suffix
using namespace std::string_literals;

#include <exodus/program.h>

template <class A, class B>
bool less(A a, B b) {
	return a lt b and a le b and b gt a and b ge a;
}

template <class A, class B>
bool equal(A a, B b) {

	{
		errputl("equal:", a, b);

		assert((var(a) eq b and var(b) eq a and var(a) le b and var(a) ge b and var(b) le a and var(b) ge a));								  //return false;
		assert((a      eq var(b) and b eq var(a) and a le var(b) and a ge var(b) and b le var(a) and b ge var(a)));								  //return false;
		assert((var(a) eq var(b) and var(b) eq var(a) and var(a) le var(b) and var(a) ge var(b) and var(b) le var(a) and var(b) ge var(a)));  //return false;

		assert((a      eq a && b eq b));						 //return false;
		assert((var(a) eq a && var(b) eq b));			 //return false;
		assert((a      eq var(a) && b eq var(b)));			 //return false;
		assert((var(a) eq var(a) && var(b) eq var(b)));	 //return false;

		assert((a ge a && b ge b));						 //return false;
		assert((var(a) ge a && var(b) ge b));			 //return false;
		assert((a ge var(a) && b ge var(b)));			 //return false;
		assert((var(a) ge var(a) && var(b) ge var(b)));	 //return false;

		assert((a le a && b le b));						 //return false;
		assert((var(a) le a && var(b) le b));			 //return false;
		assert((a le var(a) && b le var(b)));			 //return false;
		assert((var(a) le var(a) && var(b) le var(b)));	 //return false;
	}

	return true;
}

// Test how different c++ types are handled in numeric operations with vars.

template <class A, class B>
bool numtest(A a, B b, in c, in d) {

	c.outputl("c");

	d.outputl("d");

	errputl("numtest: a,b,c,d", a, b, c, d);

	var no	= 1;
	var no2 = 1;
	var add = c + d;
	add.outputl("add = c+d");

	assert(var(a + b).errput(no++ ^ ". ")           eq (add).errputl(" should be "));			   //return false;
	assert(var(var(a) + b).errput(no++ ^ ". ")      eq (add).errputl(" should be "));	   //return false;
	assert(var(a + var(b)).errput(no++ ^ ". ")      eq (add).errputl(" should be "));	   //return false;
	assert(var(var(a) + var(b)).errput(no++ ^ ". ") eq (add).errputl(" should be "));  //return false;
	assert(var(b + a).errput(no++ ^ ". ")           eq (add).errputl(" should be "));			   //return false;
	assert(var(b + var(a)).outputl("222")              eq (add).errputl(" should be ").outputl("1"));	   //return false;

	//assert((    b  + var(a)).outputl("1") ne add.outputl("2"));//return false;
	assert(var(var(b) + var(a)).errput(no++ ^ ". ").outputl("1B") eq (add).errputl(" should be ").outputl("2B"));	 //return false;

	var sub = c - d;
	assert(var(a - b).errput(no++ ^ ". ")           eq (sub).errputl(" should be "));			   //return false;
	assert(var(var(a) - b).errput(no++ ^ ". ")      eq (sub).errputl(" should be "));	   //return false;
	assert(var(a - var(b)).errput(no++ ^ ". ")      eq (sub).errputl(" should be "));	   //return false;
	assert(var(var(a) - var(b)).errput(no++ ^ ". ") eq (sub).errputl(" should be "));  //return false;

	var mul = c * d;
	assert(var(a * b).errput(no++ ^ ". ")           eq (mul).errputl(" should be "));			   //return false;
	assert(var(var(a) * b).errput(no++ ^ ". ")      eq (mul).errputl(" should be "));	   //return false;
	assert(var(a * var(b)).errput(no++ ^ ". ")      eq (mul).errputl(" should be "));	   //return false;
	assert(var(var(a) * var(b)).errput(no++ ^ ". ") eq (mul).errputl(" should be "));  //return false;

	assert(var(b * a).errput(no++ ^ ". ")           eq (mul).errputl(" should be "));			   //return false;
	assert(var(var(b) * a).errput(no++ ^ ". ")      eq (mul).errputl(" should be "));	   //return false;
	assert(var(b * var(a)).errput(no++ ^ ". ")      eq (mul).errputl(" should be "));	   //return false;
	assert(var(var(b) * var(a)).errput(no++ ^ ". ") eq (mul).errputl(" should be "));  //return false;

	var div = var(c) / var(d);
	assert(var(a / b).errput(no++ ^ ". ")                 eq div.errputl(" should be "));				   //return false;
	assert(var(var(a) / b).errput(no++ ^ ". ")            eq div.errputl(" should be "));			   //return false;
	assert(var(a / var(b)).errput(no++ ^ ". ")            eq div.errputl(" should be "));			   //return false;
	assert(var(var(a) / var(b)).errput(no++ ^ ". ")       eq div.errputl(" should be "));		   //return false;
	assert(var(1 / (b / a)).errput(no++ ^ ". ")           eq div.errputl(" should be "));			   //return false;
	assert(var(1 / (var(b) / a)).errput(no++ ^ ". ")      eq div.errputl(" should be "));	   //return false;
	assert(var(1 / (b / var(a))).errput(no++ ^ ". ")      eq div.errputl(" should be "));	   //return false;
	assert(var(1 / (var(b) / var(a))).errput(no++ ^ ". ") eq div.errputl(" should be "));  //return false;

	var mod = var(c) % var(d);
	//assert((    a  %     b)  ne mod);//return false;
	assert((var(a) % b).errput(no++ ^ ". ")      eq mod.errputl(" should be "));		  //return false;
	assert((a % var(b)).errput(no++ ^ ". ")      eq mod.errputl(" should be "));		  //return false;
	assert((var(a) % var(b)).errput(no++ ^ ". ") eq mod.errputl(" should be "));  //return false;

	return true;
}

auto func_c(const char* xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

auto func_s(std::string xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

auto func_i(const int xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

auto func_li(const long long xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

auto func_d(const double xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

auto func_ld(const long double xyz) {
	std::cout << xyz << std::endl;
	return xyz;
}

programinit()

	function main() {
	printl("test_more says 'Hello World!'");

	{
		equal('a', 'a');
		equal('a', "a");

		equal('3', '3');
		equal('3', "3");
		equal('3', 3.0);

		equal("3", '3');
		equal("3", 3);
		equal("3", 3.0);

		equal("03", '3');
		equal("03", 3);
		equal("03", 3.0);
	}

	//note
	assert(numtest(var(223), 123, 223, 123));
	assert(numtest(223, var(123), 223, 123));
	assert(numtest(223, 123.1, 223, 123.1));
	assert(numtest(223.1, 123, 223.1, 123));
	assert(numtest(223.1, 123.1, 223.1, 123.1));

	assert(numtest("223", var(123), 223, 123));
	assert(numtest("223", var(123.1), 223, 123.1));
	assert(numtest("223.1", var(123), 223.1, 123));
	assert(numtest("223.1", var(123.1), 223.1, 123.1));
	assert(numtest("223.1", var("123"), 223.1, 123));
	assert(numtest("223.1", var("123.1"), 223.1, 123.1));

	// Construct from initializer list of int double and const char*
	{
		assert(var({1, 22, -0, 333})       eq "1" _FM "22" _FM "0" _FM "333");
		assert(var({1.1, 1.2, 1000.0})     eq "1.1" _FM "1.2" _FM "1000");
		assert(var({"a", "bb", "", "ccc"}) eq "a" _FM "bb" _FM _FM "ccc");
	}

	// Construct from literal int, double and const char*
	{
		//		assert(     123456_var.errputl() eq "123456");
		//		assert(    123.456_var.errputl() eq "123.456");
		assert(123456_var                eq "123456");
		assert(123.456_var               eq "123.456");
		assert("a^bb^^ccc"_var.errputl() eq "a" _FM "bb" _FM _FM "ccc");
		assert("a^bb^^ccc"_var.errputl() eq "a" _FM "bb" _FM _FM "ccc");
		assert("`^]}|~"_var.errputl()    eq _RM _FM _VM _SM _TM _ST);

		// Check can acquire \0 characters via _var literal
		assert("abc\x00 def"_var.len() eq 8);
		assert(" \x00 "_var.oconv("HEX") eq "200020");

	}

	//Construct fixed dim array from initializer list
	{
		//check FM list from initializer and and FM list from _var agree
		var x = {1, 20, 30};
		assert(x eq "1^20^30"_var);

		//create fixed dim array using list of int
		dim d = {1, 20, 30};
		assert(d[1] eq 1);
		assert(d[3] eq 30);

		//create fixed dim array using list of double
		dim d2 = {1.0, 20.0, 30.0};
		assert(d2[1] eq 1.0);
		assert(d2[3] eq 30.0);

		//create fixed dim array using list of const char*
		dim d3 = {"1x", "20x", "30x"};
		assert(d3[1] eq "1x");
		assert(d3[3] eq "30x");

		//oswrite dim to file
		var tfilename = "t_dimoswrite.txt";
		assert(d.oswrite(tfilename));

		//check roundtrip agrees
		assert(osread(tfilename) eq(d.join("\n")));
	}

	{
		// bool -> int
		assert((var(true) ^ "x").errputl()  eq "1x");
		assert((var(false) ^ "x").errputl() eq "0x");

		// char -> str
		assert((var(char('A')) ^ "x").errputl() eq "Ax");

		// signed char -> int
		assert((var(static_cast<signed char>('A')) ^ "x").errputl() eq "65x");	 //numeric character -> int

		// unsigned char -> int
		assert((var(static_cast<unsigned char>('A')) ^ "x").errputl() eq "65x");  //numeric character -> int

		// std string suffix -> str
		assert((var("AB"s) ^ "x").errputl() eq "ABx");

		// char -> str
		assert((var('A') ^ "x").errputl() eq "Ax");

		assert(var("🍌").errputl("banana cstr = ").oconv("HEX").errputl("hex = ") eq "F09F8D8C");

		// multibyte chars
		printl("6) Ordinary multicharacter literal, e.g. 'AB', is conditionally-supported, has type int and implementation-defined value.");

		//works but as an integer not as a char
		//var('🍌').errputl("banana char(?) = ").oconv("HEX").errputl("hex = ");
		//assert(var('🍌').errputl("banana cstr = ").oconv("HEX").errputl("hex = ") eq "F09F8D8C");

		// Cannot provide var constructor for wide character -> string so they come in as ints at the moment
		assert(var(u8'A').errputl("u8=") eq "65");
		assert(var(L'A').errputl("L=")   eq "65");
		assert(var(u'A').errputl("u=")   eq "65");
		assert(var(U'A').errputl("U=")   eq "65");
		//  assert((var(u8'A') ^ "x").errputl() eq "Ax"); //char8_t is integer but not string convertible so we need a specific converter
		//  assert((var(L'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter
		//  assert((var(u'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter
		//  assert((var(U'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter

		// short -> int
		assert((var(short(65)) ^ "x").errputl() eq "65x");

		// unsigned short -> int
		assert((var(static_cast<unsigned short>(65)) ^ "x").errputl() eq "65x");
		assert((var(65) ^ "x").errputl()                   eq "65x");

		// unsigned int -> int
		assert((var(int(65)) ^ "x").errputl()            eq "65x");
		assert((var(static_cast<unsigned int>(65)) ^ "x").errputl() eq "65x");

		// unsigned long -> int
		assert((var(65L) ^ "x").errputl()                  eq "65x");
		assert((var(static_cast<unsigned long>(65L)) ^ "x").errputl() eq "65x");

		// unsigned long long -> int
		assert((var(65LL) ^ "x").errputl()                      eq "65x");
		assert((var(static_cast<unsigned long long>(65L)) ^ "x").errputl() eq "65x");

		// float -> dbl
		assert((var(123.456f).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0f) ^ "x").errputl()            eq "123x");

		// double -> dbl
		assert((var(123.456).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0) ^ "x").errputl()            eq "123x");

		// long double -> dbl
		assert((var(123.456L).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0L) ^ "x").errputl()            eq "123x");
	}

	{
		std::string s = "abc";
		assert(std::string(var(s)) eq s);
		assert(func_s(var(s))      eq s);

		const char* c = "abc";
		assert(*static_cast<const char*>(var(c)) eq *c);
		assert(*func_c(var(c))        eq *c);

		int i = 42;
		assert(int(var(i))    eq i);
		assert(func_i(var(i)) eq i);

		var v = "123";

		assert(func_c(var(v)) eq v);
		assert(func_s(var(v)) eq v);

		assert(func_i(var(v))          eq v);
		assert(func_li(var(v).toInt()) eq v.toInt());

		assert(func_d(var(v))  eq v);
		assert(func_ld(var(v)) eq v.toDouble());
	}

	{
		printl("Check conversion 'BYes,No'");
		TRACE(oconv(var(true), "BYes,No"))
		assert(oconv(var(true), "BYes,No").outputl()  eq "Yes");
		assert(oconv(var(false), "BYes,No").outputl() eq "No");
	}

	//check convert from double to string
	{
		var x = var(123.456).convert(".", "_");
		assert(x                                                        eq "123_456");
	}
	{
		//check various xxxxxxxx-er functions
		var v1_01;
		v1_01 = 1.01;
		assert(converter(v1_01, ".", "_").outputl().toString()          eq "1_01");
		v1_01 = 1.01;
		assert(textconverter(v1_01, ".", "_").outputl().toString()      eq "1_01");
		v1_01 = 1.01;
		assert(replacer(v1_01, ".", "_").outputl().toString()            eq "1_01");
		v1_01 = 1.01;
		assert(replacer(v1_01, "."_rex, "_").outputl().toString()     eq "____");
		v1_01 = 1.01;
		assert(replacer(v1_01, "."_rex, "_").outputl().toString() eq "____");
		v1_01 = 1.01;
		assert(paster(v1_01, 2, 1, "_").outputl().toString()           eq "1_01");
		v1_01 = 1.01;
		//assert(pasterall(v1_01, -1, "_").outputl().toString()             eq "1.0_");
		assert(paster(v1_01, -1, 999999, "_").outputl().toString()             eq "1.0_");
		v1_01 = 1.01;
		assert(popper(v1_01).outputl().toString()                       eq "1.0");
		v1_01 = 1.01;
		assert(quoter(v1_01).outputl().toString()                       eq "\"1.01\"");
		v1_01 = 1.01;
		assert(squoter(v1_01).outputl().toString()                      eq "'1.01'");
		v1_01 = 1.01;
		assert(unquoter(v1_01).outputl().toString()                     eq "1.01");
		v1_01 = 1.01;
		assert(ucaser(v1_01).outputl().toString() eq "1.01");  // utf8
		v1_01 = 1.01;
		assert(lcaser(v1_01).outputl().toString() eq "1.01");  // utf8
		v1_01 = 1.01;
		assert(tcaser(v1_01).outputl().toString() eq "1.01");  // utf8
		v1_01 = 1.01;
		assert(fcaser(v1_01).outputl().toString() eq "1.01");  // utf8
		v1_01 = 1.01;
		assert(normalizer(v1_01).outputl().toString() eq "1.01");  // utf8
		v1_01 = 1.01;
		assert(inverter(v1_01).outputl().toString() eq "ÎÑÏÎ");	 // utf8
		v1_01 = 1.01;
		assert(trimmer(v1_01).outputl().toString()                      eq "1.01");
		v1_01 = 1.01;
		assert(trimmerfirst(v1_01).outputl().toString()                     eq "1.01");
		v1_01 = 1.01;
		assert(trimmerlast(v1_01).outputl().toString()                     eq "1.01");
		v1_01 = 1.01;
		assert(trimmer(v1_01, var(" ")).outputl().toString()            eq "1.01");
		v1_01 = 1.01;
//		assert(trimmer(v1_01, var(" "), "FB").outputl().toString()      eq "1.01");
		assert(trimmerboth(v1_01, var(" ")).outputl().toString()      eq "1.01");
		v1_01 = 1.01;
		assert(trimmerfirst(v1_01, var(" ")).outputl().toString()           eq "1.01");
		v1_01 = 1.01;
		assert(trimmerlast(v1_01, var(" ")).outputl().toString()           eq "1.01");
		v1_01 = 1.01;
		assert(fieldstorer(v1_01, ".", 1, 1, "_").outputl().toString()  eq "_.01");
		v1_01 = 1.01;
		assert(substrer(v1_01, 2, 2).outputl().toString()               eq ".0");
		v1_01 = 1.01;
		assert(substrer(v1_01, 3).outputl().toString()                  eq "01");
		v1_01 = 1.01;
		assert(substrer(v1_01, 1).outputl().toString()                  eq "1.01");
		v1_01 = 1.01;
		assert(sorter(v1_01).outputl().toString()                       eq "1.01");
		v1_01 = 1.01;
		assert(lowerer(v1_01).outputl().toString()                      eq "1.01");
		v1_01 = 1.01;
		assert(raiser(v1_01).outputl().toString()                       eq "1.01");
		v1_01 = 1.01;
		assert(cropper(v1_01).outputl().toString()                      eq "1.01");
	}

	{
		var x = "  1  2  ";
		assert(trim(x) eq "1 2");
		assert(trimfirst(x) eq "1  2  ");
		assert(trimlast(x) eq "  1  2");
		assert(trimboth(x) eq "1  2");

		x = "  1  2  ";trimmer(x)     ;assert(x eq "1 2");
		x = "  1  2  ";trimmerfirst(x);assert(x eq "1  2  ");
		x = "  1  2  ";trimmerlast(x) ;assert(x eq "  1  2");
		x = "  1  2  ";trimmerboth(x) ;assert(x eq "1  2");
	}

	{
		var x = ".,1,.2.,";
#define BOOST_TRIM
#ifdef BOOST_TRIM
		// take the first if multiple inner
		assert(trim(x      , ".,").outputl() eq "1,2");

		var x2 = ".,1,.2.,";trimmer(x2     , ".,").outputl(); assert(x2 eq "1,2");
#else
		// take the last if multiple inner
		assert(trim(x      , ".,").outputl() eq "1.2");

		var x2 = ".,1,.2.,";trimmer(x2     , ".,").outputl(); assert(x2 eq "1.2");

#endif
		assert(trimfirst(x , ".,").outputl() eq "1,.2.,");
		assert(trimlast(x  , ".,").outputl() eq ".,1,.2");
		assert(trimboth(x  , ".,").outputl() eq "1,.2");

		x = ".,1,.2.,";trimmerfirst(x, ".,").outputl(); assert(x eq "1,.2.,");
		x = ".,1,.2.,";trimmerlast(x , ".,").outputl(); assert(x eq ".,1,.2");
		x = ".,1,.2.,";trimmerboth(x , ".,").outputl(); assert(x eq "1,.2");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
