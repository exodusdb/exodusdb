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

	printl("Test passed");

	return 0;
}

programexit()

