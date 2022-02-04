#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <functional>
#include <limits>

#include <exodus/program.h>

template <class A, class B>
bool less(A a,B b) {
	return a < b and a <= b and b > a and b >= a;
}

template <class A, class B>
bool equal(A a,B b) {
	if (!(var(a) ==     b  and var(b) ==     a  and var(a) <=     b  and var(a) >=     b  and var(b) <=     a  and var(b) >=     a))  return false;
	if (!(    a  == var(b) and     b  == var(a) and     a  <= var(b) and     a  >= var(b) and     b  <= var(a) and     b  >= var(a))) return false;
	if (!(var(a) == var(b) and var(b) == var(a) and var(a) <= var(b) and var(a) >= var(b) and var(b) <= var(a) and var(b) >= var(a))) return false;

	if  (!(a == a && b == b))                     return false;
	if  (!(var(a) == a && var(b) == b))           return false;
	if  (!(a == var(a) && b == var(b)))           return false;
	if  (!(var(a) == var(a) && var(b) == var(b))) return false;

	if  (!(a >= a && b >= b))                     return false;
	if  (!(var(a) >= a && var(b) >= b))           return false;
	if  (!(a >= var(a) && b >= var(b)))           return false;
	if  (!(var(a) >= var(a) && var(b) >= var(b))) return false;

	if  (!(a <= a && b <= b))                     return false;
	if  (!(var(a) <= a && var(b) <= b))           return false;
	if  (!(a <= var(a) && b <= var(b)))           return false;
	if  (!(var(a) <= var(a) && var(b) <= var(b))) return false;

	return true;
}

template <class A, class B>
bool numtest(A a, B b, in c, in d) {

	if ((    a  +     b)  != (c+d)) return false;
	if ((var(a) +     b)  != (c+d)) return false;
	if ((    a  + var(b)) != (c+d)) return false;
	if ((var(a) + var(b)) != (c+d)) return false;

	if ((    b  +     a)  != (c+d)) return false;
	if ((var(b) +     a)  != (c+d)) return false;
	if ((    b  + var(a)) != (c+d)) return false;
	if ((var(b) + var(a)) != (c+d)) return false;


	if ((    a  -     b)  != (c-d)) return false;
	if ((var(a) -     b)  != (c-d)) return false;
	if ((    a  - var(b)) != (c-d)) return false;
	if ((var(a) - var(b)) != (c-d)) return false;


	if ((    a  *     b)  != (c*d)) return false;
	if ((var(a) *     b)  != (c*d)) return false;
	if ((    a  * var(b)) != (c*d)) return false;
	if ((var(a) * var(b)) != (c*d)) return false;

	if ((    b  *     a)  != (c*d)) return false;
	if ((var(b) *     a)  != (c*d)) return false;
	if ((    b  * var(a)) != (c*d)) return false;
	if ((var(b) * var(a)) != (c*d)) return false;


	var div = var(c)/var(d);
	if ((    a  /     b)  != div) return false;
	if ((var(a) /     b)  != div) return false;
	if ((    a  / var(b)) != div) return false;
	if ((var(a) / var(b)) != div) return false;
	if (1/(    b  /     a)  != div) return false;
	if (1/(var(b) /     a)  != div) return false;
	if (1/(    b  / var(a)) != div) return false;
	if (1/(var(b) / var(a)) != div) return false;

	var mod = var(c)%var(d);
	//if ((    a  %     b)  != mod) return false;
	if ((var(a) %     b)  != mod) return false;
	if ((    a  % var(b)) != mod) return false;
	if ((var(a) % var(b)) != mod) return false;

	return true;
}

programinit()

function main() {
	printl("test_more says 'Hello World!'");

	equal('a','a');
	equal('a',"a");

	equal('3' , '3');
	equal('3' , "3");
	equal('3' , 3.0);

	equal("3" , '3');
	equal("3" , 3);
	equal("3" , 3.0);

	equal("03", '3');
	equal("03", 3);
	equal("03", 3.0);

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
//		assert(     123456_var.outputl() eq "123456");
//		assert(    123.456_var.outputl() eq "123.456");
		assert(     123456_var           eq "123456");
		assert(    123.456_var           eq "123.456");
		assert("a^bb^^ccc"_var.outputl() eq "a" _FM_ "bb" _FM_ _FM_ "ccc");
		assert("a^bb^^ccc"_var.outputl() eq "a" _FM_ "bb" _FM_ _FM_ "ccc");
		assert(   "_^]}|~"_var.outputl() eq _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);
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

	printl("Test passed");

	return 0;
}

programexit()

