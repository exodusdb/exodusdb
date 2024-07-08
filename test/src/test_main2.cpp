#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <cmath>  //for fmod
#include <array>

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

	function main() {

	{
		// range based for loop
		{
			var tot	  = 0;
			var count = 0;
			var str   = "";
			for (const var ii : range(1, 10)) {
				tot += ii;
				count++;
				str ^= ii ^ " ";
			}
			assert(tot   eq 55);
			assert(count eq 10);
			assert(str == "1 2 3 4 5 6 7 8 9 10 ");
		}

		// range based for loop - bad range should not enter loop
		{
			var tot	  = 0;
			var count = 0;
			for (const var ii : range(10, 1)) {
				tot += ii;
				count++;
			}
			assert(tot   eq 0);
			assert(count eq 0);
		}
	}
	{
		// reverse_range based for loop
		{
			var tot	  = 0;
			var count = 0;
			var str   = "";
			for (const var ii : reverse_range(1, 10)) {
				TRACE(ii)
				tot += ii;
				count++;
				str ^= ii ^ " ";
			}
			assert(tot   eq 55);
			assert(count eq 10);
			TRACE(str)
			assert(str == "10 9 8 7 6 5 4 3 2 1 ");
		}

		// reverse_range based for loop going negative
		{
			var tot	  = 0;
			var count = 0;
			var str   = "";
			for (const var ii : reverse_range(-3, 3)) {
				TRACE(ii)
				tot += ii;
				count++;
				str ^= ii ^ " ";
			}
			TRACE(tot)
			assert(tot   eq 0);
			assert(count eq 7);
			TRACE(str)
			assert(str == "3 2 1 0 -1 -2 -3 ");
		}

		// reverse_range based for loop zero to negative
		{
			var tot	  = 0;
			var count = 0;
			var str   = "";
			for (const var ii : reverse_range(-3, 0)) {
				TRACE(ii)
				tot += ii;
				count++;
				str ^= ii ^ " ";
			}
			TRACE(tot)
			assert(tot   eq -6);
			assert(count eq 4);
			TRACE(str)
			assert(str == "0 -1 -2 -3 ");
		}

		// reverse_range based for loop does not enter loop
		{
			var tot	  = 0;
			var count = 0;
			var str   = "";
			for (const var ii : reverse_range(-3, -6)) {
				TRACE(ii)
				tot += ii;
				count++;
				str ^= ii ^ " ";
			}
			TRACE(tot)
			assert(tot   eq 0);
			assert(count eq 0);
			TRACE(str)
			assert(str == "");
		}

		// reverse_range based for loop - bad range doesnt enter loop
		{
			var tot	  = 0;
			var count = 0;
			for (const var ii : reverse_range(10, 1)) {
				tot += ii;
				count++;
			}
			assert(tot   eq 0);
			assert(count eq 0);
		}
	}

	// range based field iteration
	{
		var fields						 = _FM "aaa" _FM _FM "bb" _FM;
		var						   count = 0;
		for (var field : fields) {
			count++;
			assert(count le 5);
			assert(count ne 1 or field eq "");
			assert(count ne 2 or field eq "aaa");
			assert(count ne 3 or field eq "");
			assert(count ne 4 or field eq "bb");
			assert(count ne 5 or field eq "");
		}
		var empty = "";
		for (var field : empty) {
			assert(false && var("Should never get here for empty range string"));
		}
	}

	{
		// We disallow self assign on temporaries
		// because it is unusual programming and may indicate some misconception by the programmer
		//var x = ostime() += 100;

		// This works fine
		var x = ostime() + 100;

		// This is allowed because ^= (self append) is working on x, not a temporary
		x = "xyz";
		x.prefixer("abc") ^= "200";
		assert(x eq "abcxyz200");
	}

	var xyz;
	//xyz=xyz;
	printl("\nTest catching VarError");
	try {
		//runtime errors instead of compiler error sadly

//		var x1	   = x1 ^= 1;	//sadly c++ compiler allows this
		var x1	   = x1 + 1;	//sadly c++ compiler allows this
		var undefx = undefx++;	//sadly c++ compiler allows this
		var z	   = z + 1;		//sadly c++ compiler allows this
		//var x++;				//doesnt compile
//		var xx = xx.operator++();  //sadly c++ compiler allows this
		var xx = xx++;  //sadly c++ compiler allows this
////var x = "x";
////printl(x + x);
//		throw VarError("xxxxxx");
								   //var xx=xx++;			//doesnt compile
	} catch (VarError& error) {
		std::cout << "error in main2.cpp2\n";
		printx(error.description);
	}

#ifdef VAR_SAFE_DESTRUCTOR
	printl("\nVerify that exodus catches c++ defect at runtime");
	try {
		var abc = abc + 1;
		printl("should never get here because above should cause a runtime undefined error");
		assert(false);
	} catch (VarUndefined& e) {
		//g++ sees the above as undefined
		var(e.description).outputl();
	} catch (VarUnassigned& e) {
		//clang sees the above as unassigned
		var(e.description).outputl();
	}
	//catch(VarNonNumeric& e) {
	//	e.description.outputl();
	//}
	catch (...) {
		//should never get here because above should cause a runtime error
		//assert(false);
		throw;
	}
#endif

	//this turns on tracing SQL statements to postgres
	//DBTRACE=true;

	var ee = "";
	var e0 = 0;
	var e1 = 1;
	var e2 = 2;
	var e3 = 3;

	assert((e2 + true)  eq 3);
	assert((e2 + false) eq 2);
	assert((true + e2)  eq 3);
	assert((false + e2) eq 2);

	assert((e2 - true)  eq 1);
	assert((e2 - false) eq 2);
	assert((true - e2)  eq - 1);
	assert((false - e2) eq - 2);

	assert((e2 * true)  eq 2);
	assert((e2 * false) eq 0);
	assert((true * e2)  eq 2);
	assert((false * e2) eq 0);

	//assert((e2*true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2*false) eq 0);//ditto
	assert((true / e2)  eq 0.5);
	assert((false / e2) eq 0);

	//assert((e2%true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2%false) eq 0);//ditto
	assert((true % e2)  eq 1);
	assert((false % e2) eq 0);

	//the six logical comparisons ge le gt lt and the lesser precedence eq  ne
	//between logical results and vars are not supported by exodus
	//because they seem useless, probably non existant in real code
	//and because I am too lazy at this point.
	//real code would have to be something like
	// if (cc eq aa gt bb)
	//which due to lower precendence of eq and ne is actually
	// if (cc eq (aa gt bb))
	assert((e1    eq true)eq true);
	assert((e1    eq false)eq false);
	assert((true  eq e1)eq true);
	assert((false eq e1)eq false);

	assert((e1 ne true)  eq false);
	assert((e1 ne false) eq true);
	assert((true ne e1)  eq false);
	assert((false ne e1) eq true);

	assert((e1    eq true)eq 1);
	assert((e1    eq false)eq 0);
	assert((true  eq e1)eq 1);
	assert((false eq e1)eq 0);

	assert((e1 ne true)eq 0);
	assert((e1 ne false)eq 1);
	assert((true ne e1)eq 0);
	assert((false ne e1)eq 1);

	assert((e3 - (e3 eq var(0))) eq 3);

	//exodus's concatenate operator ^ has the wrong priority versus logical comparison operators
	//to avoid this causing unnoticed different operation than classic multivalue basic
	//exodus refuses to concatenate the result of logical expressions like e2>e3
	//printl(e1^e2>e3); //wont compile (because it would produce the wrong result)

	//just add brackets to clarify and correct the precedence for exodus and it will compile
	assert(((e1 ^ e2) gt e3) eq 1);	//=1 ... correct result because (1^2)>3 i.e. "12">3 is true

	var s1, s2;
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
	assert(s1              eq "2");
	assert(s2              eq "1");
*/

	{
		var x;
		x.default_to("zxc");
		assert(x eq "zxc");
	}

	{
		var x = "abc";
		x.default_to("zxc");
		assert(x eq "abc");
	}

	{
		var x;
		assert(x.default_from("zxc") eq "zxc");
		assert(x.unassigned());
	}

	{
		var x = "abc";
		assert(x.default_from("zxc") eq "abc");
		assert(x eq "abc");
	}

	{
		var seps = _RM _FM _VM _SM _TM;
		assert(lower(seps)         eq _FM _VM _SM _TM _ST);
		assert(lower(seps).raise() eq seps);

		seps = _FM _VM _SM _TM _ST;
		assert(raise(seps)         eq _RM _FM _VM _SM _TM);
		assert(raise(seps).lower() eq seps);
	}

	{
		var seps					 = _RM _FM _VM _SM _TM;
		var						   x = seps;
		lowerer(x);
		assert(x         eq _FM _VM _SM _TM _ST);
		assert(raiser(x) eq seps);

		seps = _FM _VM _SM _TM _ST;
		x	 = seps;
		raiser(x);
		assert(x eq _RM _FM _VM _SM _TM);
		lowerer(x);
		assert(x eq seps);
	}

	//replacing a section of a string:
	//given start character number, number of characters to replace and a replacement string
	//(this is equivalent to the following classic mv basic syntax
	//but the simple [x,y] syntax is not available in curly bracket languages)
	//tempstr[2,3]='abc'

	//replacing a section of a string - method 1
	var a = "abcde";
	paster(a, 3, 2, "xx");
	assert(a eq "abxxe");

	//replacing a section of a string - method 2
	a = "abcde";
	paster(a, -2, -3, "xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 3 - but may be slower
	a = "abcde";
	a = paste(a, -2, -3, "xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 4 - but may be slower
	a = "abcde";
	a = a.paste(-2, -3, "xx");
	assert(a eq "axxe");

	//test single character extraction
	var expected = "a" _FM "a" _FM "b" _FM "a" _FM "a" _FM "b" _FM "";
	var tempstr2 = "ab";
	for (const var ii : range(-3, 3)) {
		assert(tempstr2.at(ii) eq expected.f(ii + 4));
	}

	//test single character extraction on ""
	tempstr2 = "";
	for (const var ii : range(-3, 3)) {
		assert(tempstr2.at(ii) eq "");
	}

	var str0 = "  xxx  xxx  ";
	var str1;

	//global function trim/f/b

	str1 = str0;
	assert(trim(str1)  eq "xxx xxx");
	assert(str1        eq str0);
	assert(trimfirst(str1) eq "xxx  xxx  ");
	assert(str1        eq str0);
	assert(trimlast(str1) eq "  xxx  xxx");
	assert(str1        eq str0);

	assert(trim(str1, " ")   eq "xxx xxx");
	assert(str1                  eq str0);
	assert(trimfirst(str1, " ")  eq "xxx  xxx  ");
	assert(str1                  eq str0);
	assert(trimlast(str1, " ")  eq "  xxx  xxx");
	assert(str1                  eq str0);
	assert(trimboth(str1, " ") eq "xxx  xxx");
	assert(str1                  eq str0);

	//member function trim/f/b

	assert(str1.trim()  eq "xxx xxx");
	assert(str1         eq str0);
	assert(str1.trimfirst() eq "xxx  xxx  ");
	assert(str1         eq str0);
	assert(str1.trimlast() eq "  xxx  xxx");
	assert(str1         eq str0);

	assert(str1.trim(" ")   eq "xxx xxx");
	assert(str1                 eq str0);
	assert(str1.trimfirst(" ")  eq "xxx  xxx  ");
	assert(str1                 eq str0);
	assert(str1.trimlast(" ")  eq "  xxx  xxx");
	assert(str1                 eq str0);
	assert(str1.trimboth(" ") eq "xxx  xxx");
	assert(str1                 eq str0);

	//global function trimmer/f/b

	str1 = str0;
	assert(trimmer(str1)  eq "xxx xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(trimmerfirst(str1) eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1 = str0;
	assert(trimmerlast(str1) eq "  xxx  xxx");
	assert(str1 ne str0);

	str1 = str0;
	assert(trimmer(str1, " ")   eq "xxx xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(trimmerfirst(str1, " ")  eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1 = str0;
	assert(trimmerlast(str1, " ")  eq "  xxx  xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(trimmerboth(str1, " ") eq "xxx  xxx");
	assert(str1 ne str0);

	//member function trimmer/f/b

	str1 = str0;
	assert(str1.trimmer()  eq "xxx xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(str1.trimmerfirst() eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1 = str0;
	assert(str1.trimmerlast() eq "  xxx  xxx");
	assert(str1 ne str0);

	str1 = str0;
	assert(str1.trimmer(" ")   eq "xxx xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(str1.trimmerfirst(" ")  eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1 = str0;
	assert(str1.trimmerlast(" ")  eq "  xxx  xxx");
	assert(str1 ne str0);
	str1 = str0;
	assert(str1.trimmerboth(" ") eq "xxx  xxx");
	assert(str1 ne str0);

#define BOOST_TRIM
#ifdef BOOST_TRIM
	// take the first if multiple inner
	{
		// on temporary
		assert(trim("XYZabcXYZdefXYZ", "XYZ").outputl() eq "abcXdef");
		// on reference
		var x = "XYZabcXYZdefXYZ";
		trimmer(x, "XYZ");
		TRACE(x)
		assert(x.outputl()                                        eq "abcXdef");
	}

	{
		assert(trimfirst("XYZabcXYZdefXYZ", "XYZ").outputl() eq "abcXYZdefXYZ");
		var x = "XYZabcXYZdefXYZ";
		trimmerfirst(x, "XYZ");
		assert(x.outputl()                                         eq "abcXYZdefXYZ");
	}

	{
		assert(trimlast("XYZabcXYZdefXYZ", "XYZ").outputl() eq "XYZabcXYZdef");
		var x = "XYZabcXYZdefXYZ";
		trimmerlast(x, "XYZ");
		assert(x.outputl()                                         eq "XYZabcXYZdef");
	}
#else
	// take the last if multiple inner
	{
		assert(trim("XYZabcXYZdefXYZ", "XYZ").outputl() eq "abcZdef");
		var x = "XYZabcXYZdefXYZ";
		trimmer(x, "XYZ");
		assert(x                                        eq "abcZdef");
	}

	{
		assert(trimfirst("XYZabcXYZdefXYZ", "XYZ").outputl() eq "abcXYZdefXYZ");
		var x = "XYZabcXYZdefXYZ";
		trimmerfirst(x, "XYZ");
		assert(x                                         eq "abcXYZdefXYZ");
	}

	{
		assert(trimlast("XYZabcXYZdefXYZ", "XYZ").outputl() eq "XYZabcXYZdef");
		var x = "XYZabcXYZdefXYZ";
		trimmerlast(x, "XYZ");
		assert(x                                         eq "XYZabcXYZdef");
	}

#endif

	//test daisychaining assignments NOT ALLOWED - TO PREVENT = being used instead of eq by mistake in if() clauses
	//var aa1,aa2,aa3,aa4;
	//aa1=aa2=aa3="aa";
	//assert(aa1 eq "aa");
	//assert(aa2 eq "aa");
	//assert(aa3 eq "aa");

	var tempinp;
	//	input("Press Enter ...",tempinp);
	//ensure lower case sorts before uppercase (despite "A" \x41 is less than "a" \x61)
	a	  = "a";
	var A = "A";
	assert(a lt A);

	//gosub is a throwaway word to indicate calling a local member function
	//call is similar but conventionally used for external function (shared lib functions)
	gosub internal_subroutine_xyzz("aa" _FM "b1" _VM "b2" _SM "b22" _FM "cc");

	//osopen fail
	var nonexistentfile = OSSLASH ^ "129834192784";
	assert(!osopen(nonexistentfile, nonexistentfile));

	var xx = osshellread("dir");

	//xx.osread(xx);

	var eof1 = var().eof();

	ossetenv("XYZ", "abc");
	assert(osgetenv("XYZ") eq "abc");

	assert(oconv(1234, "MD20P").outputl() eq "1234.00");

	assert(var("a") lt var("B"));

	//select("select test_alphanum with f1 between 20 and 21");
	assert(oconv(10.1, "MD20") eq "10.10");

	{
		printl("Investigate the bytes of a double in hex for naturalorder");
		double d2 = 1;
		double d3 = 2;
		if (d2 lt d3)
			d2 = d3;
		union {
			double d1;
			// char   chars[sizeof d1];
			std::array<char, sizeof d1> chars;
		};
		for (d1 = -5; d1 le 5; ++d1) {
			printx("Decimal ", d1, "= ");
			for (unsigned int partn = 0; partn lt sizeof(chars); ++partn) {
				//var(chars[partn]).oconv("HEX").output();
				//std::cout << std::hex << static_cast<unsigned int>(chars[partn]) << " " ;
				printx(oconv(chars[partn], "HEX"), "");
			}
			printl();
		}
	}

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

	//var steve;steve.input(1);

	//catching errors - doesnt work now that backtrace aborts (to prevent system crashes ... maybe better solution is to trap in main()
	//var unass;
	//try{unass+1;}
	//catch(...){};

	//bool cannot be used numerically ON MSVC (unlike in pick)
	//could change all logical ops to return var or find a way to allow void* pointer to promote to bool
	//or perhaps add bool to the list of automatic constructors?
	var log1 = "xx";
	var log2 = "x";
	//following will not compile now that all exodus logical operators return bool instead of var
	//if (log1 eq log2^log1) {}
	/*
	in.cpp(181) : error C2666: 'exo::operator ^' : 7 overloads have similar conversions
	could be 'exo::var exo::operator +(const int,const exo::var &)' [found using argument-dependent lookup]
	or 'exo::var exo::operator +(const exo::var &,const double)' [found using argument-dependent lookup]
	or 'exo::var exo::operator +(const exo::var &,const int)' [found using argument-dependent lookup]
	or 'exo::var exo::operator +(const exo::var &,const exo::var &)' [found using argument-dependent lookup]
	or 'built-in C++ operator+(int, bool)'
	or 'built-in C++ operator+(unsigned int, bool)'
	or 'built-in C++ operator+(bool, bool)'
	while trying to match the argument list '(exo::var, bool)'
	*/

	//neither will the following
	//var log3=count(log1,"x")+(log1 ne "");
	//just convert it to the better
	assert(fcount(log1, "x") eq 3);
TRACE("qqq")
	printl(SENTENCE);
	assert(var("xyz").b(4, 1) eq "");

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS
TRACE("qqq0")

	var subs = "xyz";
TRACE("qqq1")
	assert(subs.b(-1) eq "z");
TRACE("qqq2")
	assert(subs.at(-1)        eq "z");

TRACE("qqq3")
	printl(oconv("a", "L#3").quote());
TRACE("qqq4")
	assert(oconv("a", "L#3")    eq "a  ");
TRACE("qqq7")
	TRACE("a  ")
TRACE("qqq5")
	assert(oconv("abc", "L#3")  eq "abc");
TRACE("qqq6")
	assert(oconv("abcd", "L#3") eq "abc");

TRACE("a  " ^ FM)
TRACE("b  " ^ FM ^ "abc")
TRACE("c  " ^ FM ^ "abc" ^ FM)
TRACE("d  " ^ FM ^ "abc" ^ FM ^ "abc")

TRACE("qqq8")
	TRACE(oconv("a" ^ FM ^ "abc" ^ FM ^ "abcd", "L#3"))
TRACE("qqq9")
	assert(oconv("a" ^ FM ^ "abc" ^ FM ^ "abcd", "L#3") eq("a  " ^ FM ^ "abc" ^ FM ^ "abc"));
TRACE("qqqa")

	assert(oconv("a", "R#3")    eq "  a");
	assert(oconv("abc", "R#3")  eq "abc");
	assert(oconv("abcd", "R#3") eq "bcd");
	assert(oconv("a" ^ FM ^ "abc" ^ FM ^ "abcd", "R#3") eq("  a" ^ FM ^ "abc" ^ FM ^ "bcd"));

	assert(oconv("a", "T#3")   eq "a  ");
	assert(oconv("abc", "T#3") eq "abc");
	assert(oconv("abcd", "T#3") eq("abc" ^ TM ^ "d  "));
	assert(oconv("a" ^ FM ^ "abc" ^ FM ^ "abcd", "T#3") eq("a  " ^ FM ^ "abc" ^ FM ^ "abc" ^ TM ^ "d  "));
	assert(var("x xx xxx xxxxx xxxxx xxxxxx").oconv("T#6") eq "x xx  |xxx   |xxxxx |xxxxx |xxxxxx"_var);
	assert(var("x xx xxx xxxxx xxxxx xxxxxxx").oconv("T#6") eq "x xx  |xxx   |xxxxx |xxxxx |xxxxxx|x     "_var);
	assert(var("").oconv("T#6") eq "      ");

	assert(oconv("a", "L(0)#3") eq "a00");
	assert(oconv("a", "R(0)#3") eq "00a");
	assert(oconv("a", "T(0)#3") eq "a00");
	assert(oconv("abcd", "T(0)#3") eq("abc" ^ TM ^ "d00"));

	assert(oconv("ab", "C#-2")   eq "ab");
	assert(oconv("ab", "C#-1")   eq "ab");
	assert(oconv("ab", "C#0")    eq "");
	assert(oconv("ab", "C#1")    eq "a");
	assert(oconv("ab", "C#2")    eq "ab");
	assert(oconv("ab", "C#3")    eq "ab ");
	assert(oconv("ab", "C#4")    eq " ab ");
	assert(oconv("ab", "C#5")    eq " ab  ");
	assert(oconv("ab", "C(0)#5") eq "0ab00");

	assert(oconv("12345", "L#6")    eq "12345 ");
	assert(oconv("12345", "R(*)#8") eq "***12345");
	assert(oconv("ABCDEFG", "R#4")  eq "DEFG");
	assert(oconv("ABCD", "C#6")     eq " ABCD ");

	// Pending implementation of masking
	//assert(oconv("1234567890","L(###)###-####") eq "(123)456-7890");

	//http://www.regular-expressions.info/examples.html
	assert(replace("Steve Bush Bash bish", rex("B.", "ri"), "Ru") eq "Steve Rush Rush Rush");

	assert(oconv("Aa019KK", "HEX")             eq "41613031394B4B");
	assert(oconv("Aa019KK", "HEX2")            eq "41613031394B4B");
	//assert(oconv("Aa019KK","HEX4") eq "00410061003000310039004B004B");
	//assert(oconv("Aa019KK","HEX8") eq "00000041000000610000003000000031000000390000004B0000004B");
	assert(var("41613031394B4B").iconv("HEX")  eq "Aa019KK");
	assert(var("41613031394B4B").iconv("HEX2") eq "Aa019KK");
	//assert(var("00410061003000310039004B004B").iconv("HEX4") eq "Aa019KK");
	//assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX8") eq "Aa019KK");

	//doesnt accept FMs etc yet
	//assert(var("FF"^FM^"00").iconv("HEX") eq ("00FF"^FM^"00FF"));
	assert(var("FF" ^ FM ^ "00").iconv("HEX2").oconv("HEX2") eq "");
	//anything invalid returns empty string
	assert(var("XF").iconv("HEX").oconv("HEX")               eq "");

#ifdef BOOST_TRIM
	assert(var("abcabdef").trim("abef").outputl()                eq "cad");
#else
	assert(var("abcabdef").trim("abef").outputl()                eq "cbd");
#endif

	assert(var("abcabdef").trimfirst("abef").trimlast("abef") eq "cabd");

	var temp3 = "c";
	var temp2("c");
	var temp1 = "abc";
	//var temp2=temp2="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//show cause VarUndefined and not simply crash do to assigning an uninitialised string
	//var undefinedassign=undefinedassign="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//var undefinedassign2=undefinedassign2=11;

	//undefined
	//var undefinedboolconv=undefinedboolconv?true:false;

	//undefined
	//var conn1=conn1.connect();

	assert(space(-11)       eq "");
	assert(var("x").str(-7) eq "");

	var x9;
	//	if (var xx eq x) {};
	//TODO implement some kind of a switch to turn off exit in mvexceptions
	var filehandle;
	/*
	try {
		printl(filehandle);
	}
	catch (VarError& error) {
		error.description.outputl();
	}
*/

	assert(oconv("xxx", "") eq "xxx");

	assert(oconv("", "MX")     eq "");
	assert(oconv(" ", "MX")    eq " ");
	assert(oconv(0, "MX")      eq "0");
	assert(oconv(-0, "MX")     eq "0");
	assert(oconv("X", "MX")    eq "X");
	//assert(oconv("-1.5","MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1.5", "MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1", "MX")   eq "FFFFFFFFFFFFFFFF");
	assert(oconv("1.5", "MX")  eq "2");
	assert(oconv("20" _FM "255", "MX") eq("14" _FM "FF"));

	var x;
	var y;
	x = "0";
	y = date();


	{
		// Test insertion of commas
		assert(var(1000).oconv("MD20P,") eq "1,000.00");
		assert(var(1000).oconv("MC20P,").outputl() eq "1.000,00");
		assert(var(1).oconv("MD20P,") eq "1.00");
		assert(var(12).oconv("MD20P,") eq "12.00");
		assert(var(123).oconv("MD20P,") eq "123.00");
		assert(var(1234).oconv("MD20P,") eq "1,234.00");
		assert(var(12345).oconv("MD20P,") eq "12,345.00");
		assert(var(123456).oconv("MD20P,") eq "123,456.00");
		assert(var(1234567).oconv("MD20P,") eq "1,234,567.00");
		assert(var(12345678).oconv("MD20P,") eq "12,345,678.00");
	}

	{
		// Test insertion of commas with negative
		assert(var(-1000).oconv("MD20P,") eq "-1,000.00");
		assert(var(-1000).oconv("MC20P,") eq "-1.000,00");
		assert(var(-1).oconv("MD20P,") eq "-1.00");
		assert(var(-12).oconv("MD20P,") eq "-12.00");
		assert(var(-123).oconv("MD20P,") eq "-123.00");
		assert(var(-1234).oconv("MD20P,") eq "-1,234.00");
		assert(var(-12345).oconv("MD20P,") eq "-12,345.00");
		assert(var(-123456).oconv("MD20P,") eq "-123,456.00");
		assert(var(-1234567).oconv("MD20P,") eq "-1,234,567.00");
		assert(var(-12345678).oconv("MD20P,") eq "-12,345,678.00");
	}

	assert(var("0") lt var(".5"));

	var ss;

	var tconv = FM.oconv("T#20");
	assert(tconv eq("                    " _FM "                    "));
	tconv = "xxxxx/xxxxx xxx" _FM "xx";
	tconv = tconv.oconv("T#8");
	assert(tconv eq("xxxxx/xx" ^ TM ^ "xxx xxx " _FM "xx      "));

	var sentence = SENTENCE;

	{

		printl("\nTest pop()/popper()");

		var x = "abc";

		assert(x.pop() eq "ab");
		assert(x       eq "abc");

		assert(x.popper() eq "ab");
		assert(x          eq "ab");

		assert(x.popper() eq "a");
		assert(x          eq "a");

		assert(x.popper() eq "");
		assert(x          eq "");

		assert(x.popper() eq "");
		assert(x          eq "");

		assert(var("abc").pop() eq "ab");
		assert(var("ab").pop()  eq "a");
		assert(var("a").pop()   eq "");
		assert(var("").pop()    eq "");

		//test pops bytes not text characters
		//text three byte utf8
		var euro = "€";
		assert(oconv(euro, "HEX") eq "E282AC");
		popper(euro);
		assert(oconv(euro, "HEX") eq "E282");
		popper(euro);
		assert(oconv(euro, "HEX") eq "E2");

		//test procedural free functions
		var y = "abc";
		assert(popper(y) eq "ab");
		assert(y         eq "ab");
		assert(pop(y)    eq "a");
		assert(y         eq "ab");
	}

	//	var().stop();

	// cannot connectlocal in main and thread until pipes are numbered
	//	tss_environmentns.reset(new int(0));
	//	tss_environmentns.reset(0);

	//	if (!init_thread(0))
	//		abort("Couldnt init thread 0");

	printl("\nPrint out 1st 256 unicode characters and their hashes");
	for (const var ii : range(0, 255)) {
		var xx = chr(ii);
		printx(ii ^ ":" ^ xx ^ " " ^ xx.hash() ^ " ");
	}
	printl();

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

//be careful not to name any subroutines the same as any variables
subroutine internal_subroutine_xyzz(in xyzz) {
	printl("internal_subroutine_xyzz(in xyzz) ENTERED");
	//var xx;
	//printl(xx);
	//false && xyzz;
	assert(xyzz.f(2, 2, 2) eq "b22");
	printl("internal_subroutine_xyzz(in xyzz) LEAVING");
	return;
}

programexit()
