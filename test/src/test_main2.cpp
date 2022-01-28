#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <cmath> //for fmod

#include <exodus/program.h>

// TESTING
// =======
//
// if it runs ok with no static assertion failures
//
//   compile test_main2 && test_main2
//
// then test no changes in output with the following
//
//   test_main2 &> t_test_main2 && diff test_main2.out t_test_main2
//
// there will be a limited number of differences due to time/date/random output

// DEVELOPING
// ==========
//
// if you add to or change the output in any way,
// which includes adding comments at the top due since program line numbers are part of output during error reporting
// then update the expected output file
//
//   test_main2 &> test_main2.out

programinit()

function main()
{

	// range based for loop
	{
		var tot = 0;
		var count= 0;
		for (const var ii : range(1,10)) {
			tot += ii;
			count++;
		}
		assert(tot eq 55);
		assert(count eq 10);
	}

	// range based field iteration
	{
	    var fields = _FM_ "aaa" _FM_ _FM_ "bb" _FM_ ;
	    var count = 0;
	    for (var field : fields) {
	        count++;
	        assert(count le 5);
	        assert(count ne 1 or field eq "");
	        assert(count ne 2 or field eq "aaa");
	        assert(count ne 3 or field eq "");
	        assert(count ne 4 or field eq "bb");
	        assert(count ne 5 or field eq "");
	    }
	}

	var xyz;
	//xyz=xyz;
	printl("\nTest catching MVError");
	try {
		//runtime errors instead of compiler error sadly

		var x1=x1^=1;			//sadly c++ compiler allows this
		var undefx=undefx++;	//sadly c++ compiler allows this
		var z=z+1;				//sadly c++ compiler allows this
		//var x++;				//doesnt compile
		var xx=xx.operator++(); //sadly c++ compiler allows this
		//var xx=xx++;			//doesnt compile
	}
	catch (MVError error) {
		print(error.description);
	}

#define SAFE_DESTRUCTOR
#ifdef SAFE_DESTRUCTOR
	printl("\nVerify that exodus catches c++ defect at runtime");
	try {
		var abc=abc+1;
		//should never get here because above should cause a runtime undefined error
		assert(false);
	}
	catch (MVUndefined mve) {
		//g++ sees the above as undefined
		mve.description.outputl();
	}
	catch(MVUnassigned mve) {
		//clang sees the above as unassigned
		mve.description.outputl();
	}
	//catch(MVNonNumeric mve) {
	//	mve.description.outputl();
	//}
	catch (...) {
		//should never get here because above should cause a runtime error
		//assert(false);
		throw;
	}
#endif

	//this turns on tracing SQL statements to postgres
	//DBTRACE=true;

	var ee="";
	var e0=0;
	var e1=1;
	var e2=2;
	var e3=3;

	assert((e2+true) eq 3);
	assert((e2+false) eq 2);
	assert((true+e2) eq 3);
	assert((false+e2) eq 2);

	assert((e2-true) eq 1);
	assert((e2-false) eq 2);
	assert((true-e2) eq -1);
	assert((false-e2) eq -2);

	assert((e2*true) eq 2);
	assert((e2*false) eq 0);
	assert((true*e2) eq 2);
	assert((false*e2) eq 0);

	//assert((e2*true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2*false) eq 0);//ditto
	assert((true/e2) eq 0.5);
	assert((false/e2) eq 0);

	//assert((e2%true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2%false) eq 0);//ditto
	assert((true % e2) eq 1);
	assert((false % e2) eq 0);

	//the six logical comparisons >= <= > < and the lesser precedence eq  ne
	//between logical results and vars are not supported by exodus
	//because they seem useless, probably non existant in real code
	//and because I am too lazy at this point.
	//real code would have to be something like
	// if (cc eq aa > bb)
	//which due to lower precendence of eq and ne is actually
	// if (cc eq (aa > bb))
	assert((e1 eq true) eq true);
	assert((e1 eq false) eq false);
	assert((true eq e1) eq true);
	assert((false eq e1) eq false);

	assert((e1!=true) eq false);
	assert((e1!=false) eq true);
	assert((true!=e1) eq false);
	assert((false!=e1) eq true);

	assert((e1 eq true) eq 1);
	assert((e1 eq false) eq 0);
	assert((true eq e1) eq 1);
	assert((false eq e1) eq 0);

	assert((e1 ne true) eq 0);
	assert((e1 ne false) eq 1);
	assert((true ne e1) eq 0);
	assert((false ne e1) eq 1);

	assert((e3-(e3 eq var(0))) eq 3);

	//exodus's concatenate operator ^ has the wrong priority versus logical comparison operators
	//to avoid this causing unnoticed different operation than classic multivalue basic
	//exodus refuses to concatenate the result of logical expressions like e2>e3
	//printl(e1^e2>e3); //wont compile (because it would produce the wrong result)

	//just add brackets to clarify and correct the precedence for exodus and it will compile
	assert( ((e1^e2)>e3) eq 1); //=1 ... correct result because (1^2)>3 i.e. "12">3 is true

	var s1,s2;
/* why wont this compile??
	s1="111";
	s2;
	exchange(s1,s2); //works even on unassigned variables (rather meaningless if both are unassigned though!)
	assert(s1.unassigned());
	assert(s2 eq "111");
	exchange(s1,s2); //check s1 can be unassigned too
	assert(s1 eq "111");
	assert(s2.unassigned());

	s1="1";
	s2="2";
	assert(exchange(s1,s2) eq "1");
	assert(s1 eq "2");
	assert(s2 eq "1");
*/

	var seps = _RM_ _FM_ _VM_ _SM_ _TM_;
	assert(seps.lower() == _FM_ _VM_ _SM_ _TM_ _STM_);
	assert(seps.lower().raise() == seps);

	seps = _FM_ _VM_ _SM_ _TM_ _STM_;
	assert(seps.raise() == _RM_ _FM_ _VM_ _SM_ _TM_);
	assert(seps.raise().lower() == seps);

	s1="1";
	s2="2";
	assert(transfer(s1,s2) eq "1");
	assert(s1 eq "");
	assert(s2 eq "1");

	//replacing a section of a string:
	//given start character number, number of characters to replace and a replacement string
	//(this is equivalent to the following classic mv basic syntax
	//but the simple [x,y] syntax is not available in curly bracket languages)
	//tempstr[2,3]='abc'

	//replacing a section of a string - method 1
	var a="abcde";
	splicer(a,3,2,"xx");
	assert(a eq "abxxe");

	//replacing a section of a string - method 2
	a="abcde";
	a.splicer(-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 3 - but may be slower
	a="abcde";
	a=splice(a,-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 4 - but may be slower
	a="abcde";
	a=a.splice(-2,-3,"xx");
	assert(a eq "axxe");

	//test single character extraction
	var expected="a" _FM_ "a" _FM_ "b" _FM_ "a" _FM_ "a" _FM_ "b" _FM_ "";
	var tempstr2="ab";
	for (const var ii : range(-3, 3)) {
		assert(tempstr2[ii] eq expected.a(ii+4));
	}

	//test single character extraction on ""
	tempstr2="";
	for (const var ii : range(-3, 3)) {
		assert(tempstr2[ii] eq "");
	}

	var str0="  xxx  xxx  ";
	var str1;

	//global function trim/f/b

	str1=str0;
	assert(trim(str1) eq "xxx xxx");
	assert(str1 eq str0);
	assert(trimf(str1) eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(trimb(str1) eq "  xxx  xxx");
	assert(str1 eq str0);

	assert(trim(str1," ","") eq "xxx xxx");
	assert(str1 eq str0);
	assert(trim(str1," ","F") eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(trim(str1," ","B") eq "  xxx  xxx");
	assert(str1 eq str0);
	assert(trim(str1," ","FB") eq "xxx  xxx");
	assert(str1 eq str0);

	//member function trim/f/b

	assert(str1.trim() eq "xxx xxx");
	assert(str1 eq str0);
	assert(str1.trimf() eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(str1.trimb() eq "  xxx  xxx");
	assert(str1 eq str0);

	assert(str1.trim(" ","") eq "xxx xxx");
	assert(str1 eq str0);
	assert(str1.trim(" ","F") eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(str1.trim(" ","B") eq "  xxx  xxx");
	assert(str1 eq str0);
	assert(str1.trim(" ","FB") eq "xxx  xxx");
	assert(str1 eq str0);


	//global function trimmer/f/b

	str1=str0;
	assert(trimmer(str1) eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmerf(str1) eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmerb(str1) eq "  xxx  xxx");
	assert(str1 ne str0);

	str1=str0;
	assert(trimmer(str1," ","") eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","F") eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","B") eq "  xxx  xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","FB") eq "xxx  xxx");
	assert(str1 ne str0);

	//member function trimmer/f/b

	str1=str0;
	assert(str1.trimmer() eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmerf() eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmerb() eq "  xxx  xxx");
	assert(str1 ne str0);

	str1=str0;
	assert(str1.trimmer(" ","") eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","F") eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","B") eq "  xxx  xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","FB") eq "xxx  xxx");
	assert(str1 ne str0);

	//test daisychaining assignments NOT ALLOWED - TO PREVENT = being used instead of == by mistake in if() clauses
	//var aa1,aa2,aa3,aa4;
	//aa1=aa2=aa3="aa";
	//assert(aa1 eq "aa");
	//assert(aa2 eq "aa");
	//assert(aa3 eq "aa");

	{
		//string seed
		initrnd("cccc");
		var r1 =rnd(1'000'000'000);
		//test reseed
		initrnd("cccc");
		assert(rnd(1'000'000'000) eq r1);

		//slightly different string seed
		initrnd("cccd");
		assert(rnd(1'000'000'000) ne r1);

		//slightly different max int
		initrnd("cccd");
		assert(rnd(1'000'000'001) ne r1);

	}

	{
		//int seed
		initrnd(123456);
		var r1 =rnd(1'000'000'000);

		//test reseed is the same
		initrnd(123456);
		assert(rnd(1'000'000'000)==r1);

		//test reseed with string of int is the same
		initrnd("123456");
		assert(rnd(1'000'000'000)==r1);

		//test reseed with double of int is the same
		initrnd(123456.4);
		assert(rnd(1'000'000'000)==r1);

		//test reseed with double of int is the same
		initrnd(123456.6);
		assert(rnd(1'000'000'000)==r1);

		//slightly different seed
		initrnd(123457);
		assert(rnd(1'000'000'000) ne r1);

		//slightly different max int DOESNT CHANGE RESULT!
		//initrnd(123456);
		//assert(rnd(1'000'000'001) ne r1);
	}

	var tempinp;
//	input("Press Enter ...",tempinp);
	//ensure lower case sorts before uppercase (despite "A" \x41 is less than "a" \x61)
	a="a";
	var A="A";
	assert(a<A);

	//gosub is a throwaway word to indicate calling a local member function
	//call is similar but conventionally used for external function (shared lib functions)
	gosub internal_subroutine_xyzz("aa" _FM_ "b1" _VM_ "b2" _SM_ "b22" _FM_ "cc");

	//osopen fail
	var nonexistentfile=OSSLASH^"129834192784";
	assert(!osopen(nonexistentfile,nonexistentfile));

	//round

	printl(round(var("6000.50")/20,2));
	assert(round(var("6000.50")/20,2)==300.03);
	assert(round(var("-6000.50")/20,2)==-300.03);
	assert(round(var(6000.50)/20,2)==300.03);
	assert(round(var(-6000.50)/20,2)==-300.03);
	assert(round(var(6000.50f)/20,2)==300.03);
	assert(round(var(-6000.50f)/20,2)==-300.03);

	//math.h seems to have been included in one of the boost or other special headers
	//in this main.cpp file and that causes confusion between math.h and exodus.h sin() and other functions.
	//we resolved the issue here by being specific about the namespace
	assert( exodus::sin(30).round(8) eq 0.5);
	assert( exodus::cos(60).round(8) eq 0.5);
	assert( exodus::tan(45).round(8) eq 1);
	assert( exodus::atan(1).round(6) eq 45);

	assert( exodus::abs(0)    eq 0);
	assert( exodus::abs(30)   eq 30);
	assert( exodus::abs(30.00) eq 30);
	assert( exodus::abs(30.10) eq 30.1);
	assert( exodus::abs(30.90) eq 30.9);

	assert( exodus::abs(-0)    eq 0);
	assert( exodus::abs(-30)   eq 30);
	assert( exodus::abs(-30.00) eq 30);
	assert( exodus::abs(-30.10) eq 30.1);
	assert( exodus::abs(-30.90) eq 30.9);

	assert( exodus::exp(1).round(9) eq 2.718281828);
	assert( exodus::loge(1) eq 0);
	assert( exodus::loge(2.718281828).round(9) eq 1);
	assert( exodus::loge(10).round(9) eq 2.302585093);
	assert( exodus::sqrt(100) eq 10);

	var xx=osshellread("dir");
	//xx.osread(xx);
	/*
	var inpz=input();

	var inpx;
	input(inpx);

	var inpy;
	input("prompt",inpy);

	var inpq;
	inputn(inpq,5);
	*/

	ossetenv("XYZ","abc");
	assert(osgetenv("XYZ") == "abc");

	var dividend=100;
	TRACE(mod(dividend,30));
	assert(mod(dividend,30) eq 10);

	assert(mod(-4,3)==2);
	assert(mod(-3,3)==0);
	assert(mod(-2,3)==1);
	assert(mod(-1,3)==2);
	assert(mod(-0,3)==0);
	assert(mod(0,3)==0);
	assert(mod(1,3)==1);
	assert(mod(2,3)==2);
	assert(mod(3,3)==0);
	assert(mod(4,3)==1);

	assert(mod(-4.0,3)==2);
	assert(mod(-3.0,3)==0);
	assert(mod(-2.0,3)==1);
	assert(mod(-1.0,3)==2);
	assert(mod(-0.0,3)==0);
	assert(mod(0.0,3)==0);
	assert(mod(1.0,3)==1);
	assert(mod(2.0,3)==2);
	assert(mod(3.0,3)==0);
	assert(mod(4.0,3)==1);

	assert(mod(-4.0,3.0)==2);
	assert(mod(-3.0,3.0)==0);
	assert(mod(-2.0,3.0)==1);
	assert(mod(-1.0,3.0)==2);
	assert(mod(-0.0,3.0)==0);
	assert(mod(0.0,3.0)==0);
	assert(mod(1.0,3.0)==1);
	assert(mod(2.0,3.0)==2);
	assert(mod(3.0,3.0)==0);
	assert(mod(4.0,3.0)==1);

	assert(mod(-4,3.0)==2);
	assert(mod(-3,3.0)==0);
	assert(mod(-2,3.0)==1);
	assert(mod(-1,3.0)==2);
	assert(mod(-0,3.0)==0);
	assert(mod(0,3.0)==0);
	assert(mod(1,3.0)==1);
	assert(mod(2,3.0)==2);
	assert(mod(3,3.0)==0);
	assert(mod(4,3.0)==1);

	//negative dividend
/*
	assert(mod(-4,-3)==-1);
	assert(mod(-3,-3)==0);
	assert(mod(-2,-3)==-2);
	assert(mod(-1,-3)==-1);
	assert(mod(-0,-3)==0);
	assert(mod(0,-3)==0);
	TRACE(mod(1,-3));
	assert(mod(1,-3)==-2);
	assert(mod(2,-3)==-1);
	assert(mod(3,-3)==0);
	assert(mod(4,-3)==-2);
*/
	assert(mod(-4.0,-3.0)==-1);
	assert(mod(-3.0,-3.0)==0);
	assert(mod(-2.0,-3.0)==-2);
	assert(mod(-1.0,-3.0)==-1);
	assert(mod(-0.0,-3.0)==0);
	assert(mod(0.0,-3.0)==0);
	TRACE(mod(1.0,-3.0));
	TRACE(fmod(1.0,-3.0));
	TRACE(1 % -3)
	TRACE(-1 % 3)
	assert(mod(1.0,-3.0)==-2);
	assert(mod(2.0,-3.0)==-1);
	assert(mod(3.0,-3.0)==0);
	assert(mod(4.0,-3.0)==-2);

	//check floating point modulo
	TRACE(mod(2.3,var(1.499)).round(3));
	assert(mod(2.3,var(1.499)).round(3) eq 0.801);
	TRACE(mod(-2.3,var(-1.499)).round(3));
	assert(mod(-2.3,var(-1.499)).round(3) eq -0.801);
	TRACE(mod(-2.3,var(1.499)).round(3));
	assert(mod(-2.3,var(1.499)).round(3) eq 0.698);
	TRACE(mod(2.3,var(-1.499)).round(3));
	assert(mod(2.3,var(-1.499)).round(3) eq -0.698);

	assert(oconv(1234,"MD20P") eq "1234.00");
	assert(var(10000).oconv("DY0") eq "");

	assert(var("a")<var("B"));
	assert(var(1000).oconv("MD80") eq "1000.00000000");
	assert(var("31 JAN 2008").iconv("D") eq "14641");

	assert(var("1/31/2008").iconv("D") eq 14641);
	assert(var("2008/1/31").iconv("DS") eq "14641");

	assert(var("JAN/31/2008").iconv("D") eq "14641");
	assert(var("2008/JAN/31").iconv("DS") eq "14641");

	assert(var("1/1/92").iconv("D/E") eq "8767");

	assert(var("1/1/68").iconv("D/E") eq "1");
	assert(var("31/12/67").iconv("D/E") eq "0");
	assert(var("30/12/67").iconv("D/E") eq "-1");
	assert(var("1/1/1900").iconv("D/E") eq "-24835");

	assert(var("31/1/2008").iconv("DE") eq "14641");
	assert(var("31/1/2008").iconv("D/E") eq "14641");
	assert(var("31 1 2008").iconv("DE") eq "14641");
	assert(var("31-1-2008").iconv("DE") eq "14641");
	assert(var("31/JAN/2008").iconv("DE") eq "14641");
	assert(var("JAN/31/2008").iconv("DE") eq "14641");
	assert(var("29 FEB 2008").iconv("D") eq "14670");

	assert(var("32/1/2008").iconv("DE") eq "");
	assert(var("30/2/2008").iconv("DE") eq "");
	assert(var("1/31/2008").iconv("DE") eq "");

	//check uses yyyy-mm-dd format if data starts with four digit year
	assert(iconv("2000-12-31","DE") == 12054);

	assert(oconv(15036, "D")     == "01 MAR 2009");
	assert(oconv(15036, "D2")    == "01 MAR 09");
	assert(oconv(15036, "D4")    == "01 MAR 2009");

	assert(oconv(15036, "D/")    == "03/01/2009");
	assert(oconv(15036, "D2/")   == "03/01/09");
	assert(oconv(15036, "D*")    == "03*01*2009");

	assert(oconv(15036, "D/E")   == "01/03/2009");
	assert(oconv(15036, "D2E")   == "01 MAR 09");
	assert(oconv(15036, "D2/E")  == "01/03/09");

	assert(oconv(15036, "DQ")   == "1");
	assert(oconv(15036, "DW")   == "7");
	assert(oconv(15036, "DWA")  == "SUNDAY");

	//select("select test_alphanum with f1 between 20 and 21");
	assert(oconv(10.1,"MD20") == "10.10");

	/*
	var alphanum1="Flat 10a";
	var alphanum2="Flat 2b";
	//Flat 2b is before Flat 10a
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="Part A-10";
	alphanum2="Part A-2";
	//Part A-2 is before Part A-10
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="Part -10";
	alphanum2="Part -2";
	//Part -10 is before Part -2
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));

	alphanum1="-10";
	alphanum2="-2";
	//-10 is before -2
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));

	alphanum1="-1.11";
	alphanum2="-1.2";
	//-1.2 is before -1.11
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="01.10";
	alphanum2="1.1";
	//01.10 is equal to 1.1
	assert(naturalorder(alphanum1.toString()) = naturalorder(alphanum2.toString()));

	alphanum1="A B C..C+";
	alphanum2="A B C.C";
	//A B C..C+ is before A B C.C
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));
*/

	/*
	for (double xx=-5;xx<=5;++xx)
	{
		ostringstream ostringstream1;

		appenddouble2sortstring(xx,ostringstream1);

		cout.width(3);
		cout<<xx<<" "<<ostringstream1.str()<<endl;
	}
	stop();
	*/

	//investigate the bytes of a double in hex for natural sort
	/*
	double d2=1;
	double d3=2;
	if (d2<d3)
		d2=d3;
	union {double d1; char chars[8];};
	for (d1=-5;d1<=5;++d1) {
		print(d1," ");
		for (unsigned int partn=0;partn<sizeof(chars);++partn)
			//var(chars[partn]).oconv("HEX").output();
			std::cout << std::hex << int(chars[partn]) << " " ;
		printl();
	}
	*/

	//var steve;steve.input(1);

	//catching errors - doesnt work now that backtrace aborts (to prevent system crashes ... maybe better solution is to trap in main()
	//var unass;
	//try{unass+1;}
	//catch(...){};

	//bool cannot be used numerically ON MSVC (unlike in pick)
	//could change all logical ops to return var or find a way to allow void* pointer to promote to bool
	//or perhaps add bool to the list of automatic constructors?
	var log1="xx";
	var log2="x";
	//following will not compile now that all exodus logical operators return bool instead of var
	//if (log1 eq log2^log1) {}
	/*
	in.cpp(181) : error C2666: 'exodus::operator ^' : 7 overloads have similar conversions
	could be 'exodus::var exodus::operator +(const int,const exodus::var &)' [found using argument-dependent lookup]
	or 'exodus::var exodus::operator +(const exodus::var &,const double)' [found using argument-dependent lookup]
	or 'exodus::var exodus::operator +(const exodus::var &,const int)' [found using argument-dependent lookup]
	or 'exodus::var exodus::operator +(const exodus::var &,const exodus::var &)' [found using argument-dependent lookup]
	or 'built-in C++ operator+(int, bool)'
	or 'built-in C++ operator+(unsigned int, bool)'
	or 'built-in C++ operator+(bool, bool)'
	while trying to match the argument list '(exodus::var, bool)'
	*/

	//neither will the following
	//var log3=count(log1,"x")+(log1 ne "");
	//just convert it to the better
	assert(dcount(log1,"x") eq 3);

	printl(SENTENCE);
	assert(var("xyz").substr(4,1) eq "");

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS

	var subs="xyz";
	assert(subs.substr(-1) eq "z");
	assert(subs[-1] eq "z");

	printl(oconv("a","L#3").quote());
	assert(oconv("a","L#3") eq "a  ");
	assert(oconv("abc","L#3") eq "abc");
	assert(oconv("abcd","L#3") eq "abc");
	assert(oconv("a"^FM^"abc"^FM^"abcd","L#3") eq ("a  "^FM^"abc"^FM^"abc"));

	assert(oconv("a","R#3") eq "  a");
	assert(oconv("abc","R#3") eq "abc");
	assert(oconv("abcd","R#3") eq "bcd");
	assert(oconv("a"^FM^"abc"^FM^"abcd","R#3") eq ("  a"^FM^"abc"^FM^"bcd"));

	assert(oconv("a","T#3") eq "a  ");
	assert(oconv("abc","T#3") eq "abc");
	assert(oconv("abcd","T#3") eq ("abc"^TM^"d  "));
	assert(oconv("a"^FM^"abc"^FM^"abcd","T#3") eq ("a  "^FM^"abc"^FM^"abc"^TM^"d  "));

	assert(oconv("a","L(0)#3") eq "a00");
	assert(oconv("a","R(0)#3") eq "00a");
	assert(oconv("a","T(0)#3") eq "a00");
	assert(oconv("abcd","T(0)#3") eq ("abc"^TM^"d00"));

	assert(oconv("ab","C#-2") eq "ab");
	assert(oconv("ab","C#-1") eq "ab");
	assert(oconv("ab","C#0") eq "");
	assert(oconv("ab","C#1") eq "a");
	assert(oconv("ab","C#2") eq "ab");
	assert(oconv("ab","C#3") eq "ab ");
	assert(oconv("ab","C#4") eq " ab ");
	assert(oconv("ab","C#5") eq " ab  ");
	assert(oconv("ab","C(0)#5") eq "0ab00");

	assert(iconv("23 59 59","MT") eq 86399);
	assert(iconv("xx11yy12zz13P","MT") eq 83533);
	assert(iconv("24 00 00","MT") eq "");

	//http://www.regular-expressions.info/examples.html
	assert(replace("Steve Bush Bash bish","B.","Ru","ri") eq "Steve Rush Rush Rush");

	assert(oconv("Aa019KK","HEX") eq "41613031394B4B");
	assert(oconv("Aa019KK","HEX2") eq "41613031394B4B");
	//assert(oconv("Aa019KK","HEX4") eq "00410061003000310039004B004B");
	//assert(oconv("Aa019KK","HEX8") eq "00000041000000610000003000000031000000390000004B0000004B");
	assert(var("41613031394B4B").iconv("HEX") eq "Aa019KK");
	assert(var("41613031394B4B").iconv("HEX2") eq "Aa019KK");
	//assert(var("00410061003000310039004B004B").iconv("HEX4") eq "Aa019KK");
	//assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX8") eq "Aa019KK");

	//doesnt accept FMs etc yet
	//assert(var("FF"^FM^"00").iconv("HEX") eq ("00FF"^FM^"00FF"));
	assert(var("FF"^FM^"00").iconv("HEX2").oconv("HEX2") eq "");
	//anything invalid returns empty string
	assert(var("XF").iconv("HEX").oconv("HEX") eq "");

	var time1=var("10:10:10").iconv("MT");
	assert(var("abcabdef").trim("abef") eq "cbd");
	assert(var("abcabdef").trimf("abef").trimb("abef") eq "cabd");

	var temp3="c";
	var temp2("c");
	var temp1="abc";
	//var temp2=temp2="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//show cause MVUndefined and not simply crash do to assigning an uninitialised string
	//var undefinedassign=undefinedassign="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//var undefinedassign2=undefinedassign2=11;

	//undefined
	//var undefinedboolconv=undefinedboolconv?true:false;

	//undefined
	//var conn1=conn1.connect();

	assert(space(-11) eq "");
	assert(var("x").str(-7) eq "");

	var x9;
//	if (var xx eq x) {};
	//TODO implement some kind of a switch to turn off exit in mvexceptions
	var filehandle;
/*
	try {
		printl(filehandle);
	}
	catch (MVError error) {
		error.description.outputl();
	}
*/
	assert(oconv(0,"D4") eq "31 DEC 1967");
	assert(oconv("xxx","") eq "xxx");

	assert(oconv("","MX") eq "");
	assert(oconv(" ","MX") eq " ");
	assert(oconv(0,"MX") eq "0");
	assert(oconv(-0,"MX") eq "0");
	assert(oconv("X","MX") eq "X");
	//assert(oconv("-1.5","MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1.5","MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1","MX") eq "FFFFFFFFFFFFFFFF");
	assert(oconv("1.5","MX") eq "2");
	assert(oconv("20" _FM_ "255","MX") eq ("14" _FM_ "FF"));

	assert(oconv("","D") eq "");
	assert(oconv("X","D") eq "X");
	assert(oconv("-1.5","D") eq "29 DEC 1967");
	assert(oconv("1.5","D") eq "01 JAN 1968");
	assert(oconv("1.5" _FM_ "-1.5","D") eq ("01 JAN 1968" _FM_ "29 DEC 1967"));

	assert(oconv(14276,"D") eq "31 JAN 2007");
	assert(oconv(14276,"D2") eq "31 JAN 07");
	assert(oconv(14276,"D4") eq "31 JAN 2007");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D ") eq "01 31 2007");
	assert(oconv(14276,"D2/") eq "01/31/07");
	assert(oconv(14276,"D2-") eq "01-31-07");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D/E") eq "31/01/2007");
	assert(oconv(14276,"D2 E") eq "31 01 07");
	assert(oconv(14276,"D S") eq "2007 01 31");
	assert(oconv(14276,"DM") eq "1");
	assert(oconv(14276,"DMA") eq "JANUARY");
	assert(oconv(14276,"DW") eq "3");
	assert(oconv(14276,"DWA") eq "WEDNESDAY");
	assert(oconv(14276,"DY") eq "2007");
	assert(oconv(14276,"DY2") eq "07");
	assert(oconv(14276,"D2Y") eq "07 JAN 31");
	assert(oconv(14276,"D5Y") eq "02007 JAN 31");
	assert(oconv(14276,"DD") eq "31");
	assert(oconv(14276,"DL") eq "31");
	assert(oconv(14276,"DQ") eq "1");
	assert(oconv(14276,"DJ") eq "31");

	var feb29_2004=13209;//iconv("29 FEB 2004","D");
	assert(oconv(feb29_2004,"DL") eq "29");

	//check does multivalues
	assert(oconv("14591" _VM_ _VM_ "14592", "D") eq "12 DEC 2007" _VM_ _VM_ "13 DEC 2007");
	assert(oconv("14591" _FM_ _VM_ "14592", "D") eq "12 DEC 2007" _FM_ _VM_ "13 DEC 2007");

	assert(oconv(14591,"D") eq "12 DEC 2007");
	assert(oconv(14591,"D2/") eq "12/12/07");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"D-") eq "12-12-2007");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"DJ") eq "346");
	assert(oconv(14591,"DM") eq "12");
	assert(oconv(14591,"DMA") eq "DECEMBER");
	assert(oconv(14591,"DW") eq "3");
	assert(oconv(14591,"DWA") eq "WEDNESDAY");
	assert(oconv(14591,"DY") eq "2007");
	assert(oconv(14591,"DQ") eq "4");
	assert(oconv(14591,"DD") eq "12");
	assert(oconv(14591,"DL") eq "31");

	//check times around noon and midnight round trip ok
	for (const var ii : range(0, 61)) {
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);
	}
	for (const var ii : range(43200-61, 43200+61)) {
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);
	}

	//check oconv does multivalues
	assert(var("60" _RM_ "120").oconv("MT")=="00:01" _RM_ "00:02");
	assert(var("60" _FM_ "120").oconv("MT")=="00:01" _FM_ "00:02");
	assert(var("60" _VM_ "120").oconv("MT")=="00:01" _VM_ "00:02");
	assert(var("60" _SM_ "120").oconv("MT")=="00:01" _SM_ "00:02");
	assert(var("60" _TM_ "120").oconv("MT")=="00:01" _TM_ "00:02");
	assert(var("60" _STM_ "120").oconv("MT")=="00:01" _STM_ "00:02");

	//test that some random times iconv/oconv roundtrip ok
	initrnd(1000);
	var timex;
	for (int ii = 1; ii <= 1000; ++ii) {
		timex=rnd(18600);
//		timex.oconv("MTHS").output(" ").iconv("MTHS").outputl(" ");
		assert(timex.oconv("MTHS").iconv("MTHS") eq timex);
	}

	//oconv(46622,"MTH").outputl("oconv 46622 MTH is" );
	assert(oconv(46622,"MTH") eq "12:57PM");

	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MT") eq "17:35");

	assert(oconv(0,"MTH") eq "12:00AM");
	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MTH") eq "05:35PM");
	assert(oconv(31653,"MTS") eq "08:47:33");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTHS") eq "05:35:06PM");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTS.") eq "17.35.06");
	assert(oconv(63306,"MTh") eq "17h35");

	assert(oconv(61201,"MT") eq "17:00");
	assert(oconv(61201,"MTS") eq "17:00:01");
	assert(oconv(61201,"MTH") eq "05:00PM");
	assert(oconv(61201,"MTHS") eq "05:00:01PM");

	var time2=43261;
	assert(time2.oconv("MT").outputl() eq "12:01");
	assert(time2.oconv("MTH").outputl() eq "12:01PM");
	assert(time2.oconv("MTS").outputl() eq "12:01:01");
	//assert(time2.oconv("MTSH").outputl() eq "12H01H01");
	assert(time2.oconv("MTSH").outputl() eq "12:01:01PM");
	assert(time2.oconv("MTx") eq "12x01");
	assert(time2.oconv("MTHx") eq "12x01PM");
	assert(time2.oconv("MTSx") eq "12x01x01");
	//assert(time2.oconv("MTSHx") eq "1201H01");
	assert(time2.oconv("MTSHx") eq "12x01x01PM");

	time2=0;
	assert(time2.oconv("MT").outputl() eq "00:00");
	assert(time2.oconv("MTH").outputl() eq "12:00AM");
	assert(time2.oconv("MTS").outputl() eq "00:00:00");
	assert(time2.oconv("MTHS").outputl() eq "12:00:00AM");

	//negative time
	time2=-1;
	assert(time2.oconv("MT").outputl() eq "23:59");
	assert(time2.oconv("MTH").outputl() eq "11:59PM");
	assert(time2.oconv("MTS").outputl() eq "23:59:59");
	assert(time2.oconv("MTHS").outputl() eq "11:59:59PM");
	time2=-86400/2;
	assert(time2.oconv("MT").outputl() eq "12:00");
	assert(time2.oconv("MTH").outputl() eq "12:00PM");
	assert(time2.oconv("MTS").outputl() eq "12:00:00");
	assert(time2.oconv("MTHS").outputl() eq "12:00:00PM");
	time2=-86400-1;
	assert(time2.oconv("MT").outputl() eq "23:59");

	//test some unlimited time
	assert(var(-100).oconv("MTU").outputl() eq "-00:01");
	assert(var(-100).oconv("MTUS").outputl() eq "-00:01:40");
	assert(var(-10000).oconv("MTUS").outputl() eq "-02:46:40");

	assert(var(100).oconv("MTU").outputl() eq "00:01");
	assert(var(100).oconv("MTUS").outputl() eq "00:01:40");
	assert(var(1000).oconv("MTUS").outputl() eq "00:16:40");
	assert(var(10000).oconv("MTUS").outputl() eq "02:46:40");
	//NB 27:46:40 NOT ROUNDED UP TO 27:47 because mins like on clock
	assert(var(100000).oconv("MTU").outputl() eq "27:46");
	assert(var(100000).oconv("MTUS").outputl() eq "27:46:40");

    //wrap around next midnight is 00:00
    assert(var(86400).oconv("MTS").outputl() eq "00:00:00");

    //wrap around 24 hours * 2 -> 00:00
    assert(var(86400*2).oconv("MTS").outputl() eq "00:00:00");

    //U = Unlimited hours
    assert(var(86400*2).oconv("MTUS").outputl() eq "48:00:00");
    assert(var(86400*100).oconv("MTUS").outputl() eq "2400:00:00");

	//test some decimal hours based time
	assert(var(0).oconv("MT2").outputl() eq "00:00");
	assert(var(0).oconv("MT2S").outputl() eq "00:00:00");
	assert(var(0.25).oconv("MT2").outputl() eq "00:15");
	assert(var(0.25).oconv("MT2S").outputl() eq "00:15:00");
	assert(var(24).oconv("MT2S").outputl() eq "00:00:00");
	assert(var(25).oconv("MT2S").outputl() eq "01:00:00");
	assert(var(-25).oconv("MT2S").outputl() eq "23:00:00");

	//test some UNLIMITED decimal hours based time
	//NB negative unlimited time is symmetrical (unlike normal time_
	//negative time could be useful to refer to previous dates
	//but in a non-symmetrical fashion ie -1 means one second back into yesterday
	//ie 23:59:59 ... not -00:00:01. negative hours could perhaps be used
	//to credit hours in some imaginary volumetric numerical fashion
	//The "symmetric" feature should perhaps be a SEPARATE option
	//instead of being tied to the U unlimited option
	assert(var(.01).oconv("MT2US").outputl() eq "00:00:36");
	assert(var(-.01).oconv("MT2US").outputl() eq "-00:00:36");
	assert(var(.25).oconv("MT2US").outputl() eq "00:15:00");
	assert(var(-.25).oconv("MT2US").outputl() eq "-00:15:00");
	assert(var(25).oconv("MT2US").outputl() eq "25:00:00");
	assert(var(-25).oconv("MT2US").outputl() eq "-25:00:00");
	assert(var(125.25).oconv("MT2US").outputl() eq "125:15:00");
	printl(var(-125.25).oconv("MT2US"));
	assert(var(-125.25).oconv("MT2US").outputl() eq "-125:15:00");
	assert(var(9).oconv("MT2US").outputl() eq "09:00:00");
	assert(var(-9).oconv("MT2US").outputl() eq "-09:00:00");

//	assert(oconv(_FM_ "\x0035","HEX4") eq "00FE0035");
	//assert(oconv(_FM_ "\x0035","HEX4") eq "07FE0035");
	//assert(oconv(FM,"HEX4") eq "07FE");

	assert(oconv(43260,"MT")    == "12:01");
	assert(oconv(43260,"MTH")   == "12:01PM");
	assert(oconv(43260,"MTS")   == "12:01:00");
	assert(oconv(43260,"MTHS")  == "12:01:00PM");
	assert(oconv(61200,"MT")    == "17:00");
	assert(oconv(61200,"MTHS,") == "05,00,00PM");

	//test copying files forced overwrite)
    osmkdir("d1/d1");
    oswrite("f1","d1/f1");
    oswrite("f2","d1/f2");
    oscopy("d1/f1","d1/f2");
    var data;
    osread(data,"d1/f2");
    printl(data=="f1");

	//test copying directories (recursive)
    oswrite("f1","d1/d1/f1");
    oscopy("d1","d2");
    osread(data,"d2/d1/f1");
    assert(data=="f1");

    //cannot copy a directory to a file
    assert(oscopy("d1","d2/d1/f1")==0);

    osrmdir("d1",true);
    osrmdir("d2",true);

	osmkdir("test_main.1");
	osmkdir("test_main.2");

	if (not SLASH_IS_BACKSLASH) {

		//TRACE: dir1=".gitignore^a.out^alloc_traits.h.gcov^allocator.h.gcov^basic_string.h.gcov^basic_string.tcc.gcov^calblur8.html^char_traits.h.gcov^cmake_install.cmake^CMakeCache.txt^CMakeFiles^CMakeLists.txt^cpp_type_traits.h.gcov^CTestTestfile.cmake^DartConfiguration.tcl^exodusfuncs.h.gcov^exprtk.cpp^genregress2.cpp^gthr-default.h.gcov^hashtable_policy.h.gcov^hashtable.h.gcov^iomanip.gcov^ios_base.h.gcov^Makefile^move.h.gcov^mv.h.gcov^new_allocator.h.gcov^p4.cpp^ptr_traits.h.gcov^simple_decimal_conversion.h.gcov^std_mutex.h.gcov^stl_algobase.h.gcov^stl_iterator_base_funcs.h.gcov^stl_iterator_base_types.h.gcov^string_conversions.h.gcov^t_calblur8utf8.html^t_codep.bin^t_cp_allo.txt^t_cp_allo4.txt^t_EN_RU_UA.txt^t_GreekEnglFile.txt^t_GreekLocalFile.txt^t_GreekUTF-8File.txt^t_temp1234.txt^t_temp5.txt^t_test_MIXTURE.txt^t_test_OUTPUT_1251.txt^t_test_OUTPUT_UTF8.txt^t_utf-8-test.txt^t_utf8copy.html^t_utf8utf8.html^t_x.txt^test_asyncupd^test_asyncupd.cpp^test_comp^test_comp.cpp^test_db^test_db.cpp^test_dict^test_dict.cpp^test_dim^test_dim.cpp^test_isnum^test_isnum.cpp^test_main^test_main.1^test_main.2^test_main.cpp^test_main.out^test_math^test_math.cpp^test_mod^test_more^test_more.cpp^test_multilang^test_multilang.cpp^test_mvfuncs^test_mvfuncs.cpp^test_mvmv^test_mvmv.cpp^test_numfmt^test_numfmt.cpp^test_osopen^test_osopen.cpp^test_precision^test_precision.cpp^test_regress^test_regress.cpp^test_select^test_select.cpp^test_sort^test_sort.cpp^test_sortarray^test_sortarray.cpp^test_zzzclean^test_zzzclean.cpp^Testing^tests^type_traits.gcov^type_traits.h.gcov^unordered_map.h.gcov^utf-8-test.txt^utf8.html^x^y^z"

		//unordered files and directories
		var dir1 = oslist("*").sort();
		var dir2 = osshellread("ls . -AU1").convert("\n\r", _FM_).trim(FM_).sort();
		//TRACE(dir1)
		//TRACE(dir2)
		assert(dir1 eq dir2);

		//unordered directories
		var dirs = oslistd("*").sort();
		//TRACE(dirs)
		assert(dir1 ne dirs);

		//unordered files
		var files = oslistf("*").sort();
		//TRACE(files)
		assert(dir1 ne files);

		//check oslist = oslistd ^ oslistf (both sorted)
		dir2 = (dirs ^ FM ^ files).sort();
		assert(dir1 eq dir2);

		// ls xxx*.yyy returns a sorted list regardless of the -U unordered option
		dir1 = oslist("test_*.cpp").sort();
		dir2 = osshellread("ls test_*.cpp -AU1").convert("\n\r", _FM_ ).trim(FM_).sort();
		//TRACE(dir1)
		//TRACE(dir2)
		assert(dir1 eq dir2);

		//files (not directories)
		assert(oslistf("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . ! -type d -printf '%f\n'").convert("\n\r",""));

		//directories (not files)
		assert(oslistd("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . -type d -printf '%f\n\'").convert("\n\r",""));
	}
	osrmdir("test_main.1");
	osrmdir("test_main.2");

	printl();
	assert(osdir(OSSLASH).match(_FM_ "\\d{5}" _FM_ "\\d{1,5}"));

	//root directories

	//check one step multilevel subfolder creation (requires boost version > ?)
	var topdir1=OSSLASH^"exodus544";
	var topdir1b=topdir1^"b";
	var subdir2=topdir1^OSSLASH^"abcd";
	var subdir2b=topdir1b^OSSLASH^"abcd";

	var tempdir="exotemp746";
	osrmdir(tempdir,true);

	//try to remove any old versions (subdir first to avoid problems)
	osrmdir(topdir1b,true);
	osrmdir(topdir1);
	osrmdir(subdir2b,true);
	osrmdir(subdir2);

	//need oermission to test root directory access
	if (osmkdir(subdir2)) {

		//assert(osmkdir(subdir2));

		printl("\nCheck CANNOT rename multilevel root folders");
		assert(not osrename(topdir1,topdir1b));

		printl("\nCheck CANNOT force delete root folders");
		assert(not osrmdir(topdir1,true));
		printl();

		//check can remove root folders one by one without force
		assert(osrmdir(subdir2));
		assert(osrmdir(topdir1));

		//printl(osdir("c:\\config.sys"));

		//relative directories ie not-root
		if (osdir(tempdir))
			assert(osrmdir(tempdir,true));

		//check mkdir
		assert(osmkdir(tempdir));
		assert(osdir(tempdir));
		assert(not osmkdir(tempdir));

		//check rmdir
		assert(osrmdir(tempdir));
		assert(not osdir(tempdir));
	}

	//check writing a 1Mb file
	//restrict to ascii characters so size on disk=number of characters in string
	//also restrict to size 1 2 4 8 16 etc
	//var str1="1234ABC\x0160";//Note: you have to prefix strings with L if you want to put multibyte hex chars
	str1="1234ABCD";
	var filesize=1024*1024/8;
	printl(tempdir);
	assert(osmkdir(tempdir));
	assert(osrmdir(tempdir));
	assert(osmkdir(tempdir));
	var tempfilename=tempdir^OSSLASH^"temp1";
	printl(tempfilename);
	//printl(str(str1,filesize/len(str1)));
	assert(oswrite(str(str1,filesize/len(str1)),tempfilename));
	var filedate=date();
	assert(osfile(tempfilename));
	var info=osfile(tempfilename);
	assert(info.a(1) eq filesize);
	assert(info.a(2) eq filedate);

	//check copying to a new file
	var tempfilename2=tempfilename^2;
	if (osfile(tempfilename2))
	assert(osdelete(tempfilename2));
	assert(oscopy(tempfilename,tempfilename2));
	assert(osfile(tempfilename2) eq info);

	//check renaming
	var tempfilename3=tempfilename^3;
	assert(osrename(tempfilename2,tempfilename3));
	assert(osfile(tempfilename3) eq info);

	//check force delete of subdirectory
	assert(osrmdir(tempdir,true));

	var x;
	var y;
	x="0";
	y=date();

	assert(var(1000).oconv("MD20P,") eq "1,000.00");

	assert(var("0")<var(".5"));

	var ss;

	var tconv=FM.oconv("T#20");
	assert(tconv eq ("                    " _FM_ "                    "));
	tconv="xxxxx/xxxxx xxx" _FM_ "xx";
	tconv=tconv.oconv("T#8");
	assert(tconv eq ("xxxxx/xx" ^ TM ^ "xxx xxx " _FM_ "xx      "));

	var sentence=sentence();

    //wcout<<"main()"<<endl;
    //MVSystem mvsystem;
    //mvsystem.run();
    //wcout<<"exit"<<endl;

//    var xx="XXX";
//  xx.inverter();

	var env=osgetenv("");
	assert(osgetenv("PATH"));
	assert(osgetenv("HOME"));
	env="Steve";
	env.ossetenv("XYZ");
	assert(osgetenv("XYZ"));

//	var().debug();
//	var xx=xx[1];

	var temprecord;
	var tempfilename0="tempfile";
	assert(oswrite("123" on tempfilename0));
	assert(osfile(tempfilename0));
	assert(osread(temprecord from tempfilename0));
	assert(temprecord eq "123");
	assert(osdelete(tempfilename0));
	assert(not osfile(tempfilename0));

	{

		printl("\nTest pop()/popper()");

		var x = "abc";

		assert(x.pop() eq "ab");
		assert(x eq "abc");

		assert(x.popper() eq "ab");
		assert(x eq "ab");

		assert(x.popper() eq "a");
		assert(x eq "a");

		assert(x.popper() eq "");
		assert(x eq "");

		assert(x.popper() eq "");
		assert(x eq "");

		assert(var("abc").pop() eq "ab");
		assert(var("ab").pop() eq "a");
		assert(var("a").pop() eq "");
		assert(var("").pop() eq "");

		//test pops bytes not text characters
		//text three byte utf8
		var euro="€";
		assert(oconv(euro,"HEX")=="E282AC");
		euro.popper();
		assert(oconv(euro,"HEX")=="E282");
		euro.popper();
		assert(oconv(euro,"HEX")=="E2");

		//test procedural free functions
		var y = "abc";
		assert(popper(y) eq "ab");
		assert(y eq "ab");
		assert(pop(y) eq "a");
		assert(y eq "ab");
	}

//	var().stop();

// cannot connectlocal in main and thread until pipes are numbered
//	tss_environmentns.reset(new int(0));
//	tss_environmentns.reset(0);

//	if (!init_thread(0))
//		abort("Couldnt init thread 0");

	printl("\nPrint out 1st 256 unicode characters and their hashes");
	for (const var ii : range(0, 255)) {
		var xx=chr(ii);
		print(ii ^ ":" ^ xx ^ " " ^ xx.hash() ^ " ");
	}
	printl();

	printl("Checking time oconv/iconv roundtrip for time (seconds) =0 to 86400");
	//initrnd(999);
	//for (int ii = 0; ii <= 86400-1; ++ii) {
	//	var time=ii;
	var started=ostime();
	for (const var itime : range(0, 86399)) {
//		itime.outputl("itime=").oconv("MTHS").outputl("otime=").iconv("MTHS").outputl("itime=");
		assert(itime.oconv("MTHS").iconv("MTHS") eq itime);
		assert(itime.oconv("MTS").iconv("MTS") eq itime);
	}
	var stopped=ostime();
	errputl(stopped-started," seconds");
	printl("test_main finished ok and exiting OK");
	printl("Test passed");

	return 0;
}

//be careful not to name any subroutines the same as any variables
subroutine internal_subroutine_xyzz(in xyzz)
{
	printl("internal_subroutine_xyzz(in xyzz)");
	//var xx;
	//printl(xx);
	//false && xyzz;
	assert(xyzz.a(2,2,2) eq "b22");
	return;
}

programexit()
