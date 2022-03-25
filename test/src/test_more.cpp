#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <functional>
#include <limits>

#include <string> //for ""_s suffix
using namespace std::string_literals;

#include <exodus/program.h>

template <class A, class B>
bool less(A a,B b) {
	return a < b and a <= b and b > a and b >= a;
}

template <class A, class B>
bool equal(A a,B b) {

	{
		errputl("equal:", a, b);

		assert((var(a) ==     b  and var(b) ==     a  and var(a) <=     b  and var(a) >=     b  and var(b) <=     a  and var(b) >=     a)) ;//return false;
		assert((    a  == var(b) and     b  == var(a) and     a  <= var(b) and     a  >= var(b) and     b  <= var(a) and     b  >= var(a)));//return false;
		assert((var(a) == var(b) and var(b) == var(a) and var(a) <= var(b) and var(a) >= var(b) and var(b) <= var(a) and var(b) >= var(a)));//return false;

		assert ((a == a && b == b))                    ;//return false;
		assert ((var(a) == a && var(b) == b))          ;//return false;
		assert ((a == var(a) && b == var(b)))          ;//return false;
		assert ((var(a) == var(a) && var(b) == var(b)));//return false;

		assert ((a >= a && b >= b))                    ;//return false;
		assert ((var(a) >= a && var(b) >= b))          ;//return false;
		assert ((a >= var(a) && b >= var(b)))          ;//return false;
		assert ((var(a) >= var(a) && var(b) >= var(b)));//return false;

		assert ((a <= a && b <= b))                    ;//return false;
		assert ((var(a) <= a && var(b) <= b))          ;//return false;
		assert ((a <= var(a) && b <= var(b)))          ;//return false;
		assert ((var(a) <= var(a) && var(b) <= var(b)));//return false;
	}

	return true;
}

// Test how different c++ types are handled in numeric operations with vars.

template <class A, class B>
bool numtest(A a, B b, in c, in d) {

	c.dump("c");

	d.dump("d");

	errputl("numtest: a,b,c,d", a, b, c, d);

	var no = 1;
	var no2 = 1;
	var add = c+d;
	add.dump("add = c+d");

	assert(var(    a  +     b).errput(no++ ^ ". ")  == (add).errputl(" should be "));//return false;
	assert(var(var(a) +     b).errput(no++ ^ ". ")  == (add).errputl(" should be "));//return false;
	assert(var(    a  + var(b)).errput(no++ ^ ". ") == (add).errputl(" should be "));//return false;
	assert(var(var(a) + var(b)).errput(no++ ^ ". ") == (add).errputl(" should be "));//return false;
	assert(var(    b  +     a ).errput(no++ ^ ". ") == (add).errputl(" should be "));//return false;
	assert(var(    b  + var(a)).dump("222") == (add).errputl(" should be ").dump("1"));//return false;


	//assert((    b  + var(a)).dump("1") != add.dump("2"));//return false;
	assert(var(var(b) + var(a)).errput(no++ ^ ". ").dump("1B") == (add).errputl(" should be ").dump("2B"));//return false;

	var sub = c-d;
	assert(var(    a  -     b).errput(no++ ^ ". ")  == (sub).errputl(" should be "));//return false;
	assert(var(var(a) -     b).errput(no++ ^ ". ")  == (sub).errputl(" should be "));//return false;
	assert(var(    a  - var(b)).errput(no++ ^ ". ") == (sub).errputl(" should be "));//return false;
	assert(var(var(a) - var(b)).errput(no++ ^ ". ") == (sub).errputl(" should be "));//return false;

	var mul = c*d;
	assert(var(    a  *     b).errput(no++ ^ ". ")  == (mul).errputl(" should be "));//return false;
	assert(var(var(a) *     b).errput(no++ ^ ". ")  == (mul).errputl(" should be "));//return false;
	assert(var(    a  * var(b)).errput(no++ ^ ". ") == (mul).errputl(" should be "));//return false;
	assert(var(var(a) * var(b)).errput(no++ ^ ". ") == (mul).errputl(" should be "));//return false;

	assert(var(    b  *     a).errput(no++ ^ ". ")  == (mul).errputl(" should be "));//return false;
	assert(var(var(b) *     a).errput(no++ ^ ". ")  == (mul).errputl(" should be "));//return false;
	assert(var(    b  * var(a)).errput(no++ ^ ". ") == (mul).errputl(" should be "));//return false;
	assert(var(var(b) * var(a)).errput(no++ ^ ". ") == (mul).errputl(" should be "));//return false;

	var div = var(c)/var(d);
	assert(var(    a  /     b).errput(no++ ^ ". ")  == div.errputl(" should be "));//return false;
	assert(var(var(a) /     b).errput(no++ ^ ". ")  == div.errputl(" should be "));//return false;
	assert(var(    a  / var(b)).errput(no++ ^ ". ") == div.errputl(" should be "));//return false;
	assert(var(var(a) / var(b)).errput(no++ ^ ". ") == div.errputl(" should be "));//return false;
	assert(var(1/(    b  /     a)).errput(no++ ^ ". ")  == div.errputl(" should be "));//return false;
	assert(var(1/(var(b) /     a)).errput(no++ ^ ". ")  == div.errputl(" should be "));//return false;
	assert(var(1/(    b  / var(a))).errput(no++ ^ ". ") == div.errputl(" should be "));//return false;
	assert(var(1/(var(b) / var(a))).errput(no++ ^ ". ") == div.errputl(" should be "));//return false;

	var mod = var(c)%var(d);
	//assert((    a  %     b)  != mod);//return false;
	assert((var(a) %     b).errput(no++ ^ ". ")  == mod.errputl(" should be "));//return false;
	assert((    a  % var(b)).errput(no++ ^ ". ") == mod.errputl(" should be "));//return false;
	assert((var(a) % var(b)).errput(no++ ^ ". ") == mod.errputl(" should be "));//return false;

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
		equal('a','a'  );
		equal('a',"a"  );

		equal('3' , '3');
		equal('3' , "3");
		equal('3' , 3.0);

		equal("3" , '3');
		equal("3" , 3  );
		equal("3" , 3.0);

		equal("03", '3');
		equal("03", 3  );
		equal("03", 3.0);
	}

	//note
	assert(numtest(var(223),        123,   223,   123   ));
	assert(numtest(    223,     var(123),  223,   123   ));
	assert(numtest(    223,         123.1, 223,   123.1 ));
	assert(numtest(    223.1,       123,   223.1, 123   ));
	assert(numtest(    223.1,       123.1, 223.1, 123.1 ));

	assert(numtest("223"  , var(123),     223,   123   ));
	assert(numtest("223"  , var(123.1),   223,   123.1 ));
	assert(numtest("223.1", var(123),     223.1, 123   ));
	assert(numtest("223.1", var(123.1),   223.1, 123.1 ));
	assert(numtest("223.1", var("123"),   223.1, 123   ));
	assert(numtest("223.1", var("123.1"), 223.1, 123.1 ));

	// Construct from initializer list of int double and const char*
	{
		assert(var({1, 22, -0, 333}) eq "1" _FM_ "22" _FM_ "0" _FM_ "333");
		assert(var({1.1, 1.2, 1000.0}) eq "1.1" _FM_ "1.2" _FM_ "1000");
		assert(var({"a", "bb", "", "ccc"}) eq "a" _FM_ "bb" _FM_ _FM_ "ccc");
	}

	// Construct from literal int, double and const char*
	{
//		assert(     123456_var.errputl() eq "123456");
//		assert(    123.456_var.errputl() eq "123.456");
		assert(     123456_var           eq "123456");
		assert(    123.456_var           eq "123.456");
		assert("a^bb^^ccc"_var.errputl() eq "a" _FM_ "bb" _FM_ _FM_ "ccc");
		assert("a^bb^^ccc"_var.errputl() eq "a" _FM_ "bb" _FM_ _FM_ "ccc");
		assert(   "_^]}|~"_var.errputl() eq _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);
	}


	//Construct fixed dim array from initializer list
	{
		//check FM list from initializer and and FM list from _var agree
		var x = {1, 20, 30};
		assert(x eq "1^20^30"_var);

		//create fixed dim array using list of int
		dim d = {1, 20, 30};
		assert(d(1) eq 1);
		assert(d(3) eq 30);

		//create fixed dim array using list of double
		dim d2 = {1.0, 20.0, 30.0};
		assert(d2(1) eq 1.0);
		assert(d2(3) eq 30.0);

		//create fixed dim array using list of const char*
		dim d3 = {"1x", "20x", "30x"};
		assert(d3(1) eq "1x");
		assert(d3(3) eq "30x");

		//oswrite dim to file
		var tfilename = "t_dimoswrite.txt";
		d.oswrite(tfilename);

		//check roundtrip agrees
		assert(osread(tfilename) eq (d.join('\n') ^ '\n'));

	}

	{
		// bool -> int
		assert((var(true) ^ "x").errputl() eq "1x");
		assert((var(false) ^ "x").errputl() eq "0x");

		// char -> str
		assert((var(char('A')) ^ "x").errputl() eq "Ax");

		// signed char -> int
		assert((var((signed char)('A')) ^ "x").errputl() eq "65x"); //numeric character -> int

		// unsigned char -> int
		assert((var((unsigned char)('A')) ^ "x").errputl() eq "65x"); //numeric character -> int

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
		assert(var(L'A').errputl("L=") eq "65");
		assert(var(u'A').errputl("u=") eq "65");
		assert(var(U'A').errputl("U=") eq "65");
		//  assert((var(u8'A') ^ "x").errputl() eq "Ax"); //char8_t is integer but not string convertible so we need a specific converter
		//  assert((var(L'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter
		//  assert((var(u'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter
		//  assert((var(U'A') ^ "x").errputl() eq "Ax"); //wchar_t is integer but not string convertible so we need a specific converter

		// short -> int
		assert((var(short(65)) ^ "x").errputl() eq "65x");

		// unsigned short -> int
		assert((var((unsigned short)(65)) ^ "x").errputl() eq "65x");
		assert((var(65) ^ "x").errputl() eq "65x");

		// unsigned int -> int
		assert((var(int(65)) ^ "x").errputl() eq "65x");
		assert((var((unsigned int)(65)) ^ "x").errputl() eq "65x");

		// unsigned long -> int
		assert((var(65L) ^ "x").errputl() eq "65x");
		assert((var((unsigned long)(65L)) ^ "x").errputl() eq "65x");

		// unsigned long long -> int
		assert((var(65LL) ^ "x").errputl() eq "65x");
		assert((var((unsigned long long)(65L)) ^ "x").errputl() eq "65x");

		// float -> dbl
		assert((var(123.456f).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0f) ^ "x").errputl() eq "123x");

		// double -> dbl
		assert((var(123.456).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0) ^ "x").errputl() eq "123x");

		// long double -> dbl
		assert((var(123.456L).round(3) ^ "x").errputl() eq "123.456x");
		assert((var(123.0L) ^ "x").errputl() eq "123x");
	}

	{
	    std::string s = "abc";
	    assert(std::string(var(s)) == s);
	    assert(func_s(var(s)) == s);

	    const char* c = "abc";
	    assert(*(const char*)(var(c)) == *c);
	    assert(*func_c(var(c)) == *c);

	    int i = 42;
	    assert(int(var(i)) == i);
	    assert(func_i(var(i)) == i);

	    var v = "123";

	    assert(func_c(var(v)) == v);
	    assert(func_s(var(v)) == v);

	    assert(func_i(var(v)) == v);
	    assert(func_li(var(v).toInt()) == v.toInt());

	    assert(func_d(var(v)) == v);
	    assert(func_ld(var(v)) == v.toDouble());

	}

	//check convert from double to string
	{
		var x = var(123.456).convert(".", "_");
		assert(x eq "123_456");

		assert(var(1.01).converter(".", "_").outputl().toString() eq "1_01");
		assert(var(1.01).textconverter(".", "_").outputl().toString() eq "1_01");
		assert(var(1.01).swapper(".", "_").outputl().toString() eq "1_01");
		assert(var(1.01).regex_replacer(".", "_").outputl().toString() eq "____");
		assert(var(1.01).regex_replacer(".", "_" , "").outputl().toString() eq "____");
		assert(var(1.01).splicer(2, 1, "_").outputl().toString() eq "1_01");
		assert(var(1.01).splicer(-1, "_").outputl().toString() eq "1.0_");
		assert(var(1.01).popper().outputl().toString() eq "1.0");
		assert(var(1.01).quoter().outputl().toString() eq "\"1.01\"");
		assert(var(1.01).squoter().outputl().toString() eq "'1.01'");
		assert(var(1.01).unquoter().outputl().toString() eq "1.01");
		assert(var(1.01).ucaser().outputl().toString() eq "1.01");        // utf8
		assert(var(1.01).lcaser().outputl().toString() eq "1.01");        // utf8
		assert(var(1.01).tcaser().outputl().toString() eq "1.01");        // utf8
		assert(var(1.01).fcaser().outputl().toString() eq "1.01");        // utf8
		assert(var(1.01).normalizer().outputl().toString() eq "1.01");    // utf8
		assert(var(1.01).inverter().outputl().toString() eq "ÎÑÏÎ");    // utf8
		assert(var(1.01).trimmer().outputl().toString() eq "1.01");
		assert(var(1.01).trimmerf().outputl().toString() eq "1.01");
		assert(var(1.01).trimmerb().outputl().toString() eq "1.01");
		assert(var(1.01).trimmer(var(" ")).outputl().toString() eq "1.01");
		assert(var(1.01).trimmer(var(" "), "FB").outputl().toString() eq "1.01");
		assert(var(1.01).trimmerf(var(" ")).outputl().toString() eq "1.01");
		assert(var(1.01).trimmerb(var(" ")).outputl().toString() eq "1.01");
		assert(var(1.01).fieldstorer(".", 1, 1, "_").outputl().toString() eq "_.01");
		assert(var(1.01).substrer(2,2).outputl().toString() eq ".0");
		assert(var(1.01).substrer(3).outputl().toString() eq "01");
		assert(var(1.01).substrer(1).outputl().toString() eq "1.01");

		assert(var(1.01).sort().outputl().toString() eq "1.01");

		assert(var(1.01).lowerer().outputl().toString() eq "1.01");
		assert(var(1.01).raiser().outputl().toString() eq "1.01");
		assert(var(1.01).cropper().outputl().toString() eq "1.01");

	}

	printl("Test passed");

	return 0;
}

programexit()

