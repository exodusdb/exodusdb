#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <sstream>
//#include <cmath>

#include <exodus/program.h>

// TESTING
// =======
//
// if it runs ok with no static assertion failures
//
//   compile test_main && test_main
//
// then test no changes in output with the following
//
//   test_main1 &> t_test_main1 && diff test_main1.out t_test_main1
//
// there will be a limited number of differences due to time/date/random output

// DEVELOPING
// ==========
//
// if you add to or change the output in any way,
// which includes adding comments at the top due since program line numbers are part of output during error reporting
// then update the expected output file
//
//   test_main1 &> test_main1.out

programinit()

	function main() {

	{
		// Check conversion of visible to internal field marks in user literals
		assert("`^]}|~"_var eq _RM _FM _VM _SM _TM _ST);
	}

	{
		outputt("xxx", "yyy");
		outputl();
		outputl("===\t===");

		errput("aaa", "bbb");
		errputl();
		errputl("---", "---");

		logput("111", "222");
		logputl();
		logputl("...", "...");
	}

	{
		printl("Check randomness of rnd(10)");

		var ntests = 1'000'000;
		var n	   = 10;
		dim d1(n);
		d1	  = 0;
		d1(0) = 0;

		for (auto i [[maybe_unused]] : range(1, ntests)) {
			var r1 = rnd(n);
			//print(r1,"");
			d1(r1) += 1;
		}
		printl();

		printl("Check each output frequency is between 99% and 101% of the average expected frequency\nShould really do a Chi test");
		for (auto i : range(0, n - 1)) {
			printt(i, d1(i), "\n");
			assert(d1(i) > ntests / n * 0.99);
			assert(d1(i) < ntests / n * 1.01);
		}
	}

	{
		printl("Check randomness of rnd(-10)");

		var ntests = 1'000'000;
		var n	   = 10;
		dim d1(n);
		d1	  = 0;
		d1(0) = 0;

		for (auto i [[maybe_unused]] : range(1, ntests)) {
			//var r1 = rnd(n);
			var r1 = -rnd(-n);
			//print(r1,"");
			d1(r1) += 1;
		}
		printl();

		printl("Check each output frequency is between 99% and 101% of the average expected frequency\nShould really do a Chi test");
		for (auto i : range(0, n - 1)) {
			printt(i, d1(i), "\n");
			assert(d1(i) > ntests / n * 0.98);
			assert(d1(i) < ntests / n * 1.02);
		}
	}

	{
		// no seed uses a random seed

		var r1;

		//int seed
		initrnd();
		r1 = rnd(1'000'000'000);

		//test reseed is *not* the same
		initrnd();
		assert(rnd(1'000'000'000) != r1);
	}

	{
		// seed 0 uses random seed
		var r1;

		//int seed
		initrnd(0);
		r1 = rnd(1'000'000'000);

		//test reseed is *not* the same
		initrnd(0);
		assert(rnd(1'000'000'000) != r1);
	}

	{
		printl("Prevent rnd always returning 0");
		try {
			var x = rnd(0);
			assert(false);
		} catch (VarDivideByZero e) {
		};
	}

	{
		// Random seed based on high res clock
		initrnd();

		//string seed
		initrnd("cccc");
		var r1 = rnd(1'000'000'000);
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
		var r1 = rnd(1'000'000'000);

		//test reseed is the same
		initrnd(123456);
		assert(rnd(1'000'000'000) eq r1);

		//test reseed with string of int is the same
		initrnd("123456");
		assert(rnd(1'000'000'000) eq r1);

		//test reseed with double of int is the same
		initrnd(123456.4);
		assert(rnd(1'000'000'000) eq r1);

		//test reseed with double of int is the same
		initrnd(123456.6);
		assert(rnd(1'000'000'000) eq r1);

		//slightly different seed
		initrnd(123457);
		assert(rnd(1'000'000'000) ne r1);

		//slightly different max int DOESNT CHANGE RESULT!
		//initrnd(123456);
		//assert(rnd(1'000'000'001) ne r1);
	}

	{
		var unitx;
		//printl("x", amountunit("USD",ID), "y", ID.quote());
		var amount = amountunit("USD", unitx);
		printl("amount:", amount.quote(), "unit:", unitx.quote());

		assert(test_amountunit("1.23456E-6USD", 1.23456E-6, "USD"));
		assert(test_amountunit("", "", ""));
		assert(test_amountunit(" ", "", " "));
		assert(test_amountunit("E", "", "E"));
		assert(test_amountunit("E-", "", "E-"));
		assert(test_amountunit("E-3", "E-3", ""));
		assert(test_amountunit("123", 123, ""));
		assert(test_amountunit("123X", 123, "X"));
		assert(test_amountunit("123 ", 123, " "));

		// Check spaces
		assert(test_amountunit("123.456XYZZZZ", 123.456, "XYZZZZ"));
		assert(test_amountunit(" 123.456XYZZZZ", " 123.456", "XYZZZZ"));
		assert(test_amountunit("123.456 XYZZZZ", 123.456, " XYZZZZ"));
		assert(test_amountunit("123.456XYZZZZ ", 123.456, "XYZZZZ "));
	}

	{

		assert(oconv("12345", "L#6")    eq "12345 ");
		assert(oconv("12345", "R(*)#8") eq "***12345");
		assert(oconv("ABCDEFG", "R#4")  eq "DEFG");
		assert(oconv("ABCD", "C#6")     eq " ABCD ");
		//assert(oconv("1234567890","L(###)###-####") eq "(123)456-7890");

		//TX

		//FM
		assert(test_ioconv_TX("TX", "X\\nY\nZ" _FM "ABC", "X\\\\nY\\nZ\nABC"));
		//VM
		assert(test_ioconv_TX("TX", "X\\nY\nZ" _VM "ABC", "X\\\\nY\\nZ\\\nABC"));
		//SM
		assert(test_ioconv_TX("TX", "X\\nY\nZ" _SM "ABC", "X\\\\nY\\nZ\\\\\nABC"));
		//TM
		assert(test_ioconv_TX("TX", "X\\nY\nZ" _TM "ABC", "X\\\\nY\\nZ\\\\\\\nABC"));
		//ST
		assert(test_ioconv_TX("TX",
							  "X"
							  "\\n"
							  "Y"
							  "\n"
							  "Z" _ST "ABC",
							  "X"
							  "\\\\n"
							  "Y"
							  "\\n"
							  "Z"
							  "\\\\\\\\\n"
							  "ABC"));

		assert(test_ioconv_TX("TX", _RM _FM _VM _SM _TM _ST, _RM "\n"
																 "\\\n"
																 "\\\\\n"
																 "\\\\\\\n"
																 "\\\\\\\\\n"));

		// \n, literal "\n" and literal "\\n"
		assert(test_ioconv_TX("TX", "\n", "\\n"));
		assert(test_ioconv_TX("TX", "\\n", "\\\\n"));
		assert(test_ioconv_TX("TX", "\\\\n", "\\\\\\n"));
		assert(test_ioconv_TX("TX", "\n\\n\\\\n\\\\n", "\\n\\\\n\\\\\\n\\\\\\n"));

		// trailing backslashes need escaping too
		assert(test_ioconv_TX("TX", "ABC\\" _FM "DEF", "ABC{Back_Slash}\nDEF"));

		//TX1 (raw = FM only)

		//FM
		assert(test_ioconv_TX("TXR", "X\\nY\nZ" _FM "ABC", "X\\\\nY\\nZ\nABC"));
		//VM
		assert(test_ioconv_TX("TXR", "X\\nY\nZ" _VM "ABC", "X\\\\nY\\nZ" _VM "ABC"));
		//SM
		assert(test_ioconv_TX("TXR", "X\\nY\nZ" _SM "ABC", "X\\\\nY\\nZ" _SM "ABC"));
		//TM
		assert(test_ioconv_TX("TXR", "X\\nY\nZ" _TM "ABC", "X\\\\nY\\nZ" _TM "ABC"));
		//ST
		assert(test_ioconv_TX("TXR", "X\\nY\nZ" _ST "ABC", "X\\\\nY\\nZ" _ST "ABC"));

		assert(test_ioconv_TX("TXR", _RM _FM _VM _SM _TM _ST, _RM "\n" _VM _SM _TM _ST));
	}

	{
		//stop("Test passed");

		//check that we cannot assign or copy from an unassigned variable
		{
			var x1;
			try {
				var x3 = x1;  //unassigned var
				//assert(false && "var x3 = x1 should throw unassigned error)");
				assert(false);
			} catch (VarUnassigned e) {
				//assert(true || "var v3 = v1) does give unassigned error)");
			}
		}

		//check that we cannot assign or copy from an unassigned dim element
		{
			var x1;
			try {
				var x2(x1);	 //unassigned dim element
				//assert(false && "var x2(x1) should throw unassigned error)");
				assert(false);
			} catch (VarUnassigned e) {
				//assert(true || "x2(x1) does give unassigned error)");
			}
		}

		//output time and date to stderr
		errputl("Using Exodus library version:" ^ var().version());
		date().oconv("D").errputl("Date is:");
		time().oconv("MTS").errputl("Time is:");

		//14:30:46 04 JAN 2021
		//assert(timedate().match("\\d{2}:\\d{2}:\\d{2} \\d{2} [A-Z]{3} \\d{4}"));
		// Check twice in case date() changes over midnight
		assert(date() eq int(timestamp()) or date() eq int(timestamp()));

		// Check constuction of a timestamp manually agrees closely with plain timestamp.
		{
			var ts1 = date().timestamp(time());
			var ts2 = timestamp();
			assert((ts2 - ts1) < 0.001);
		}

		// Check constuction of a timestamp manually agrees closely with plain timestamp.
		{
			var ts0 = timestamp();
			var dt = int(ts0);
			var tm = ts0.mod(1) * 86'400;
			var ts1 = dt.timestamp(tm);
			assert((ts1 - ts0) < 0.001);
		}

		printl("----------------------------------------------");

		assert("abc"_var    eq "abc");
		assert(12345_var    eq 12345);
		assert(1234.567_var eq 1234.567);

		var v = 123.456;
		assert(int(v) != v);
		assert(int(v)    eq 123);
		assert(double(v) eq v);
		assert(double(v) eq 123.456);
		assert(floor(v) != v);
		assert(floor(v)  eq int(v));

		v = "123.456";
		assert(int(v) != v);
		assert(int(v)    eq 123);
		assert(double(v) eq v);
		assert(double(v) eq 123.456);
		assert(floor(v) != v);
		assert(floor(v)  eq int(v));

		v = 123456;
		v = v / 1000;
		TRACE(v)
		assert(int(v) != v);
		assert(int(v)    eq 123);
		assert(double(v) eq v);
		assert(double(v) eq 123.456);
		assert(floor(v) != v);
		assert(floor(v)  eq int(v));

		double d = 1234.567;
		//false && d;
		assert(var(d)       eq "1234.567");
		assert(var(-d)      eq "-1234.567");
		assert(int(var(d))  eq 1234);
		assert(int(var(-d)) eq -1234);

		//test accessing var as a range of fields separated by FM
		{
			var fields = "a1" _FM "b2" _FM "c3";
			var fieldn = 0;
			for (var field : fields) {
				fieldn++;
				if (fieldn eq 1)
					assert(field   eq "a1");
				else if (fieldn eq 2)
					assert(field   eq "b2");
				if (fieldn eq 3)
					assert(field   eq "c3");
			}
		}
	}

	{

		printl("sizeof");
		printl("int:	  ", (int)sizeof(int));
		printl("long:	 ", (int)sizeof(long));
		printl("long int: ", (int)sizeof(long int));
		printl("long long:", (int)sizeof(long long));
		printl("float:	", (int)sizeof(float));
		printl("double:   ", (int)sizeof(double));
		printl();
		printl("string:   ", (int)sizeof(std::string));
		printl("long long:", (int)sizeof(long long));
		printl("double:   ", (int)sizeof(double));
		printl("uint:	 ", (int)sizeof(uint));
		printl("uint:	 ", (int)sizeof(uint), " padding1");
		printl("long long:", (int)sizeof(long long), " padding2");
		printl("		   ==");
		printl("var:	  ", (int)sizeof(var));

		auto size	  = sizeof(std::string);
		auto capacity = std::string().capacity();
		auto small	  = std::string(capacity, '*');
		auto big	  = std::string(capacity + 1, '*');

		std::cout << "\nstd:string implementation\n";
		std::cout << "sizeof  : " << size << std::endl;
		std::cout << "Capacity: " << capacity << std::endl;
		std::cout << "Small   : " << small.capacity() << std::endl;
		std::cout << "Big	 : " << big.capacity() << std::endl;

		//test tcase and fcase
		printl(var("top of the world").tcase().fcase());

		var g = "Grüßen";
		assert(g.fcase()             eq "grüssen");
		assert(var("Grüßen").fcase() eq "grüssen");
		assert(g.fcaser()            eq "grüssen");

		var v = "top of the world";
		assert(v.tcase()                       eq "Top Of The World");
		assert(var("top of the world").tcase() eq "Top Of The World");
		assert(v.tcaser()                      eq "Top Of The World");

		printl(round(var("6000.50") / 20, 2));
		assert(round(var("6000.50") / 20, 2) eq 300.03);
	}

	{
		//exodus currently configured to use 2. for very great speed (see mv.cpp USE_RYU) BUT it will include over accurate figures without any rounding
		//1. precision 16 using sstring. SLOW (1850ns)
		//2. full precision using VERY FAST ryu algorithm (450ns)
		//full accuracy of ryu shows why calculations always must be rounded after every calculation because every calculation introduces more inaccuracies.
		TRACE(var(10.0 / 3.0))
		printl(var(10.0 / 3.0).len());

		//ryu full accuracy shows the inevitable inaccuracies inherent in using doubles for financial calculations
		//assert(var(10.0/3.0).toString() eq "3.3333333333333335");

		//old exodus crude reduction in precision to 16 using sstream hides inaccuracies when there are only few calculations.
		//assert(var(10.0/3.0).toString() eq "3.333333333333333");

		assert(
			var(10.0 / 3.0).toString()
				//old exodus crude reduction in precision to 16 using sstream hides inaccuracies when there are only few calculations.
				== "3.333333333333333"

			||
			var(10.0 / 3.0).toString()
				//ryu full accuracy shows the inevitable inaccuracies inherent in using doubles for financial calculations
				== "3.3333333333333335"

			//		  ||
			//		  var(10.0/3.0).toString()
			//		  //g++11 to_chars implementation full accuracy shows the inevitable inaccuracies inherent in using doubles for financial calculations
			//		  eq  "3.333333333333335"
		);

		assert(var(10.0 / 3.0 * 2.0).toString() eq "6.666666666666667");
		assert(var(10.0 / 3.0 * 3.0).toString() eq "10");

		var x = 10;
		var y = 3;
		var z = 3;
		assert(x / y * z eq x);
		assert(x / y * z eq 10);
		assert(x / y * z eq 10.0);
		assert(x / y * z eq "10");
		assert(x / y * z eq "10.0");

		x = 10.0;
		y = 3.0;
		z = 3.0;
		assert(x / y * z eq x);
		assert(x / y * z eq 10);
		assert(x / y * z eq 10.0);
		assert(x / y * z eq "10");
		assert(x / y * z eq "10.0");

		x = "10.0";
		y = "3.0";
		z = "3.0";
		assert(x / y * z eq x);
		assert(x / y * z eq 10);
		assert(x / y * z eq 10.0);
		assert(x / y * z eq "10");
		assert(x / y * z eq "10.0");

		x = "10";
		y = "3";
		z = "3";
		assert(x / y * z eq x);
		assert(x / y * z eq 10);
		assert(x / y * z eq 10.0);
		assert(x / y * z eq "10");
		assert(x / y * z eq "10.0");
	}

	{
		assert(capitalise("aa \"AAA\"")      eq "Aa \"AAA\"");
		assert(capitalise("aa \"aAa\"")      eq "Aa \"aAa\"");
		assert(capitalise("aa 'AAA'")        eq "Aa 'AAA'");
		assert(capitalise("aa 'aAa'")        eq "Aa 'aAa'");
		assert(capitalise("1aA 2AA 3Aa 4aa") eq "1AA 2AA 3AA 4AA");

		//raw strings
		var value = R"('1')";
		assert(value.replace("'", R"(\')") eq "\\'1\\'");

		//check conversion of unprintable field marks to unusual ASCII characters
		//except TM which is ESC
		std::ostringstream stringstr;
		stringstr << var(_RM _FM _VM _SM _TM _ST);
		std::cout << stringstr.str() << std::endl;
		//assert(var(stringstr.str()) eq "~^]\\[|");
		assert(var(stringstr.str()) ==
			   "~^]}"
			   "\x1B"
			   "|");
	}

	{
		/* now using epsilon to judge small numbers and differences see MVeq()

		//no longer applicable
		//exodus comparisions and conversion to book ignores numbers less than 1e-13
		//pick/arev bool and equality checks on doubles ignore less than 0.0001
		//but comparison operators work exactly (to max binary precision?)
		var smallestno = 1e-10;
		var toosmallno = smallestno/10;
		assert(smallestno);
		assert(smallestno>0);
		assert(smallestno!=0);
		assert(!toosmallno);
		assert(toosmallno==0);
		assert(!(toosmallno>0));
		assert(smallestno eq SMALLEST_NUMBER);
		*/

		var d1 = 1.2;
		d1++;
		assert(d1 eq 2.2);
		++d1;
		assert(d1 eq 3.2);

		var i1 = 1;
		i1++;
		assert(i1 eq 2.0);
		++i1;
		assert(i1 eq 3.0);

		d1 = 1.2;
		d1 += 1;
		assert(d1 eq 2.2);
		d1 += 1.1;
		assert(d1 eq 3.3);

		//exodus accidentally equipped std:string with most of the attributes of a var (this could be viewed as a good thing)
		// the feature was removed by using "hidden friends" ie placing all global friend operator functions inside the body of the var class
		//
		//assert(d1.toString()==3.3);//strange that this compiles and works (no longer after switching to "hidden friends" in exodus mv.h
		//printl(std::string("1.1")+3.3);//strange this compiles! we have equipped std:string with the ability to convert to numeric for addition with a double!

		i1 = 1;
		i1 += 1;
		assert(i1 eq 2.0);
		i1 += 1.1;
		assert(i1 eq 3.1);

		printl(1e-14);
		printl(var(0));
		//assert(1e-14==var(0));

		printl();
		printl("=== print ===");

		printl("l");
		printl();
		printl("l");
		printt();
		printl("l");
		printl("l");

		print("p");
		print("pl");
		printl();
		printl("l");
		printt("t");
		printl("l");

		print("0", "p");
		print("0", "pl");
		printl();
		printl("0", "l");
		printt("0", "t");
		printl("0", "l");

		print("0", "1", "2", "p");
		print("0", "1", "2", "pl");
		printl();
		printl("0", "1", "2", "l");
		printt("0", "1", "2", "t");
		printl("0", "1", "2", "l");

		print<'-'>("0", "1", "2", "p");
		print<'-'>("0", "1", "2", "pl");
		printl();
		printl<'-'>("0", "1", "2", "l");
		printt<'-'>("0", "1", "2", "t");
		printl<'-'>("0", "1", "2", "l");

		print(RM, FM, VM, SM, TM, ST);
		printl(RM, FM, VM, SM, TM, ST);

		{
			static char const sep[] = ", ";
			print<sep>("0", "1", "2", "p");
			print<sep>("0", "1", "2", "pl");
			printl();
			printl<sep>("0", "1", "2", "l");
			printt<sep>("0", "1", "2", "t");
			printl<sep>("0", "1", "2", "l");
		}

		output();
		output(0);
		output("0", "1", "2", "p");
		output("0", "1", "2", "pl");
		output(RM, FM, VM, SM, TM, ST);
		outputl();
		outputl(0);
		outputl("0", "1", "2", "l");
		outputl("0", "1", "2", "l");
		outputl(RM, FM, VM, SM, TM, ST);

		printl();
		printl("=== errput ===");

		//errput();
		errputl("l");
		errputl();
		errputl("l");
		errputl("l");
		errputl("l");

		errput("p");
		errput("pl");
		errputl();
		errputl("l");
		errputl("l");

		errput("0", "p");
		errput("0", "pl");
		errputl();
		errputl("0", "l");
		errputl("0", "l");

		errput("0", "1", "2", "p");
		errput("0", "1", "2", "pl");
		errputl();
		errputl("0", "1", "2", "l");
		errputl("0", "1", "2", "l");

		printl();
		printl("=== logput ===");

		logputl("l");
		logputl();
		logputl("l");
		logputl("l");
		logputl("l");

		logput("p");
		logput("pl");
		logputl();
		logputl("l");
		logputl("l");

		logput("0", "p");
		logput("0", "pl");
		logputl();
		logputl("0", "l");
		logputl("0", "l");

		logput("0", "1", "2", "p");
		logput("0", "1", "2", "pl");
		logputl();
		logputl("0", "1", "2", "l");
		logputl("0", "1", "2", "l");
	}
	{
		// These member functions do not get called by their free function equivalents
		"xxxx"_var.errput();
		"xxxx"_var.errputl();
		"xxxx"_var.logputl();
	}
	{
		assert(crop(_VM _FM)       eq "");
		assert(crop("xxx" _VM _FM) eq "xxx");
		assert(crop("aaa" _VM _FM "bbb") eq("aaa" _FM "bbb"));
		assert(crop("aaa" _FM "bbb" _FM _VM _SM _FM "ddd") eq("aaa" _FM "bbb" _FM _FM "ddd"));
		assert(crop("aaa" _FM "bbb" _FM _VM _SM _FM _RM "ddd") eq("aaa" _FM "bbb" _RM "ddd"));

		assert(crop("aa" _VM _FM "bb" _FM)             eq "aa" _FM "bb");
		assert(crop("aa" _SM _VM _FM "bb" _FM)         eq "aa" _FM "bb");
		assert(crop(_FM "aa" _VM _FM "bb" _FM)         eq _FM "aa" _FM "bb");
		assert(crop(_FM "aa" _VM _FM "bb" _FM _VM)     eq _FM "aa" _FM "bb");
		assert(crop(_FM "aa" _VM _FM _VM "bb" _FM _VM) eq _FM "aa" _FM _VM "bb");
		assert(crop(_FM "aa" _VM _FM "bb" _FM _RM)     eq _FM "aa" _FM "bb");
		assert(crop(_FM _RM "aa" _VM _FM "bb" _FM _RM) eq _RM "aa" _FM "bb");

		// crop temporaries
		assert("^]^]^}^|^~^"_var.crop().outputl()        eq "");
		assert("a^b]^c]^d}^e|^f~^g"_var.crop().outputl() eq "a^b^c^d^e^f^g"_var);

		var errmsg;
		//if (not dbcreate("steve",errmsg))
		//	errmsg.outputl();
		//if (not dbdelete("steve",errmsg))
		//	errmsg.outputl();
	}

	{

		COMMAND.outputl("COMMAND-");
		assert(COMMAND eq "service" or COMMAND.starts("test_main"));

		//test int/string changes after inc/dec (should really check MANY other ops)
		var nn = 0;
		assert(nn.toString() eq "0");  //nn now has string available internally
		++nn;						   //this should destroy the internal string
		assert(nn.toString() eq "1");  //the string should be recalculated after the ++
		nn++;
		assert(nn.toString() eq "2");
		--nn;
		assert(nn.toString() eq "1");
		nn--;
		assert(nn.toString() eq "0");
		nn += 1;
		assert(nn.toString() eq "1");
		nn -= 1;
		assert(nn.toString() eq "0");

		//same for float/string
		nn = 0.1;
		printl(nn);
		assert(nn.toString() eq "0.1");	 //nn now has string available internally
		++nn;							 //this should destroy the internal string
		printl(nn);
		assert(nn.toString() eq "1.1");	 //the string should be recalculated after the ++
		nn++;
		printl(nn);
		assert(nn.toString() eq "2.1");
		--nn;
		printl(nn);
		assert(nn.toString() eq "1.1");
		nn--;
		printl(nn);
		//assert(nn.toString() eq "0.1"); //fails at precision 15 = 0.1000000000000001
		assert(nn eq "0.1");  //succeeds numerically
		nn += 1.0;
		printl(nn);
		assert(nn.toString() eq "1.1");
		nn -= 1.0;
		printl(nn);
		//assert(nn.toString() eq "0.1"); //fails at precision 15 = 0.1000000000000001
		assert(nn eq "0.1");  //succeeds numerically

		//test remove

		//bc|5|2
		//xyz|9|0
		//abc|5|2
		//xyz|9|0
		//|999|0
		//|999|0

		{
			// member functions

			var rem = "abc" ^ FM ^ "xyz";
			var ptr = 2;
			var sep;

			var result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "bc|5|2");

			result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

			ptr = 0;

			result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "abc|5|2");

			result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

			ptr = 999;

			result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");

			result = rem.substr2(ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");
		}

		{
			// free functions

			var rem = "abc" ^ FM ^ "xyz";
			var ptr = 2;
			var sep;

			var result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "bc|5|2");

			result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

			ptr = 0;

			result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "abc|5|2");

			result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

			ptr = 999;

			result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");

			result = substr2(rem, ptr, sep);
			assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");
		}

		//test unquote
		assert(unquote("\"This is quoted?\"") eq "This is quoted?");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

bool test_amountunit(in inputx, in amount, in unitcode) {

	printl("amountunit :", inputx.quote(), "amount :", amount.quote(), "unit :", unitcode.quote());

	// Check amount
	var unitcode2;
	if (amountunit(inputx, unitcode2) ne amount)
		return false;

	// Check unit
	return unitcode2 eq unitcode;
}

function test_ioconv_TX(in fmt, in rec, in txt) {

	TRACE(fmt)
	TRACE(rec)
	TRACE(txt)
	TRACE(oconv(rec, fmt))

	if (rec.oconv(fmt) ne txt)
		return false;

	TRACE(iconv(txt, fmt))
	if (txt.iconv(fmt) ne rec)
		return false;

	if (rec.oconv(fmt).iconv(fmt) ne rec)
		return false;

	return true;
}

programexit()
