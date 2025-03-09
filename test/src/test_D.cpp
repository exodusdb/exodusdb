#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

	{
		// oconv D failure returns the original
		assert(var("sss").oconv("D") == "sss");

		// iconv D failure returns ""
		assert(var("sss").iconv("D") == "");

		// oconv MD failure returns the original
		assert(var("sss").oconv("MD20P") == "sss");

		// iconv MD/MC implemented yet
		//  assert(var("sss").iconv("MD20P") == "");
	}

	// pickos day zero is "1968/12/31"
	// "1968/12/31" <-> 0
	// Fractional days eg 1.5 are handled by floor() function

	// [-1 to 0) is "30 DEC 1967" (pickos day -1)
	assert(oconv(-1, "D")         eq "30 DEC 1967");
	assert(oconv(-0.9999999, "D") eq "30 DEC 1967");
	assert(oconv(-0.5, "D")       eq "30 DEC 1967");
	assert(oconv(-0.0000001, "D") eq "30 DEC 1967");

	// [0 to 1) is "31 DEC 1967" (pickos day 0)
	assert(oconv(0, "D")         eq "31 DEC 1967");
	assert(oconv(0.5, "D")       eq "31 DEC 1967");
	assert(oconv(0.9999999, "D") eq "31 DEC 1967");

	// [1 to 2) is "01 JAN 1968" (pickos day 1)
	assert(oconv(1.0, "D")       eq "01 JAN 1968");
	assert(oconv(1.5, "D")       eq "01 JAN 1968");
	assert(oconv(1.9999999, "D") eq "01 JAN 1968");

	{
		// bad date testing

		// General pattern is:
		// JUNK1
		// (digitgroup | alphagroup)
		// JUNK2
		// (digitgroup | alphagroup)
		// JUNK2
		// (digitgroup | alphagroup)
		//
		// Where JUNK1 is [^A-Za-z0-9]*
		// Where JUNK2 is [^A-Za-z0-9]+
		// alphagroup is three letter month JAN-DEC case insensitive and can only appear once
		// digitgroup is \d{1,2} and can only appear twice if alphagroup exists

		// iconv of two digit years
		// years 0-49 are assumed to be 2000-2049
		// years 50-99 are assumed to be 1950-1999

		// any junk is allowed on the left and between digit groups and alpha groups but not at the end
		assert(iconv("(*& 1 *^^&* 2 &^%&^% 2000", "D"));

		// lower case latin month is ok (unlike pickos?)
		assert(iconv("1 jan 2000", "D"));
		assert(iconv("1 JAN 2020", "D"));

		assert(not iconv("1 JAN JAN 2020", "D"));

		assert(not iconv("", "D"));
		assert(not iconv("xxx", "D"));
		assert(not iconv("jan", "D"));
		assert(not iconv("jan 2000", "D"));
		assert(not iconv("jan 2000", "D"));
		assert(not iconv("1 jan", "D"));

		assert(iconv("1 2 2000", "D"));

		assert(not iconv("1 2 2000 ", "D"));	 // no trailing seps
		assert(not iconv("1/2/2000/", "D"));	 // ditto
		assert(not iconv("1 2 2000 333", "D"));	 // no extra parts
		assert(not iconv("1 XYZ 2000", "D"));	 // invalid literal month
		assert(not iconv("1 2000", "D"));		 // invalid month no
		assert(not iconv("1", "D"));			 // number alone is not sufficient

		// year 2
		assert(oconv(iconv("1 jan 2", "D"), "D") eq "01 JAN 2002");	 // questionable

		// long month is not ok
		assert(not iconv("1 january 2000", "D"));

		assert(var(10000).oconv("DY0") eq "");

		assert(iconv("1 2 2000", "D"));
		assert(not iconv("1 2 2000 333", "D"));
		assert(not iconv("1/2/2000/", "D"));
		assert(not iconv("1 XYZ 2000", "D"));
		assert(not iconv("1 2000", "D"));
		assert(not iconv("1", "D"));
	}

	assert(oconv(6666, "D2-") eq "04-01-86");

	assert(var(1000).oconv("MD80")       eq "1000.00000000");
	assert(var("31 JAN 2008").iconv("D") eq "14641");

	assert(var("1/31/2008").iconv("D")  eq 14641);
	assert(var("2008/1/31").iconv("DS") eq "14641");

	assert(var("JAN/31/2008").iconv("D")  eq "14641");
	assert(var("2008/JAN/31").iconv("DS") eq "14641");

	assert(var("1/1/92").iconv("D/E") eq "8767");

	assert(var("1/1/68").iconv("D/E")   eq "1");
	assert(var("31/12/67").iconv("D/E") eq "0");
	assert(var("30/12/67").iconv("D/E") eq "-1");
	assert(var("1/1/1900").iconv("D/E") eq "-24835");

	assert(var("31/1/2008").iconv("DE")   eq "14641");
	assert(var("31/1/2008").iconv("D/E")  eq "14641");
	assert(var("31 1 2008").iconv("DE")   eq "14641");
	assert(var("31-1-2008").iconv("DE")   eq "14641");
	assert(var("31/JAN/2008").iconv("DE") eq "14641");
	assert(var("JAN/31/2008").iconv("DE") eq "14641");

	assert(var("29 FEB 1900").iconv("D") eq "");	   //centuries not divisible by 400 are not leap years
	assert(var("29 FEB 2000").iconv("D") eq "11748");  //centuries divisible by 400 are leap years
	assert(var("29 FEB 2008").iconv("D") eq "14670");  //years divisible by 4 are leap years
	assert(var("29 FEB 2009").iconv("D") eq "");	   // years not divisible 4 are leap years

	assert(var("30 FEB 1900").iconv("D") eq "");
	assert(var("30 FEB 2000").iconv("D") eq "");
	assert(var("30 FEB 2008").iconv("D") eq "");
	assert(var("30 FEB 2009").iconv("D") eq "");

	assert(var("31 FEB 1900").iconv("D") eq "");
	assert(var("31 FEB 2000").iconv("D") eq "");
	assert(var("31 FEB 2008").iconv("D") eq "");
	assert(var("31 FEB 2009").iconv("D") eq "");

	assert(var("32 JAN 1900").iconv("D") eq "");
	assert(var("32 JAN 2000").iconv("D") eq "");
	assert(var("32 JAN 2008").iconv("D") eq "");
	assert(var("32 JAN 2009").iconv("D") eq "");

	// Check which months have 31 and which do not
	assert(var("31-01-2000").iconv("DE") eq "11719");
	assert(var("31-02-2000").iconv("DE") eq "");
	assert(var("31-03-2000").iconv("DE") eq "11779");
	assert(var("31-04-2000").iconv("DE") eq "");
	assert(var("31-05-2000").iconv("DE") eq "11840");
	assert(var("31-06-2000").iconv("DE") eq "");
	assert(var("31-07-2000").iconv("DE") eq "11901");
	assert(var("31-08-2000").iconv("DE") eq "11932");
	assert(var("31-09-2000").iconv("DE") eq "");
	assert(var("31-10-2000").iconv("DE") eq "11993");
	assert(var("31-11-2000").iconv("DE") eq "");
	assert(var("31-12-2000").iconv("DE") eq "12054");

	assert(var("32/1/2008").iconv("DE") eq "");
	assert(var("30/2/2008").iconv("DE") eq "");
	assert(var("1/31/2008").iconv("DE") eq "");

	//check uses yyyy-mm-dd format if data starts with four digit year
	assert(iconv("2000-12-31", "DE") eq 12054);

	assert(oconv(15036, "D")  eq "01 MAR 2009");
	assert(oconv(15036, "D2") eq "01 MAR 09");
	assert(oconv(15036, "D4") eq "01 MAR 2009");

	assert(oconv(15036, "D/")  eq "03/01/2009");
	assert(oconv(15036, "D2/") eq "03/01/09");
	assert(oconv(15036, "D*")  eq "03*01*2009");

	assert(oconv(15036, "D/E")  eq "01/03/2009");
	assert(oconv(15036, "D2E")  eq "01 MAR 09");
	assert(oconv(15036, "D2/E") eq "01/03/09");

	// Quarter of year
	TRACE(oconv(15036, "DQ"))
	assert(oconv(15036, "DQ")  eq "1");
	assert(oconv(iconv("1/1/2000","DE"),"DQ") eq 1);
	assert(oconv(iconv("1/4/2000","DE"),"DQ") eq 2);
	assert(oconv(iconv("1/7/2000","DE"),"DQ") eq 3);
	assert(oconv(iconv("1/10/2000","DE"),"DQ") eq 4);

	assert(oconv(iconv("31/12/2000","DE"),"DQ") eq 4);
	assert(oconv(iconv("31/3/2000","DE"),"DQ") eq 1);
	assert(oconv(iconv("30/6/2000","DE"),"DQ") eq 2);
	assert(oconv(iconv("30/9/2000","DE"),"DQ") eq 3);

	// Weekday number 1-7
	assert(oconv(15036, "DW")  eq "7");
	assert(oconv(15029, "DW")  eq "7");
	assert(oconv(15030, "DW")  eq "1");
	assert(oconv(15031, "DW")  eq "2");
	assert(oconv(15032, "DW")  eq "3");
	assert(oconv(15033, "DW")  eq "4");
	assert(oconv(15034, "DW")  eq "5");
	assert(oconv(15035, "DW")  eq "6");
	assert(oconv(15036, "DW")  eq "7");
	assert(oconv(15037, "DW")  eq "1");

	// Weekday name
	assert(oconv(15036, "DWA") eq "SUNDAY");
	assert(oconv(15029, "DWA")  eq "SUNDAY");
	assert(oconv(15030, "DWA")  eq "MONDAY");
	assert(oconv(15031, "DWA")  eq "TUESDAY");
	assert(oconv(15032, "DWA")  eq "WEDNESDAY");
	assert(oconv(15033, "DWA")  eq "THURSDAY");
	assert(oconv(15034, "DWA")  eq "FRIDAY");
	assert(oconv(15035, "DWA")  eq "SATURDAY");
	assert(oconv(15036, "DWA")  eq "SUNDAY");
	assert(oconv(15037, "DWA")  eq "MONDAY");

	assert(oconv(6666, "D2-") eq "04-01-86");
	assert(oconv(0, "D4")     eq "31 DEC 1967");
	assert(oconv("", "D")     eq "");
	assert(oconv("X", "D")    eq "X");
	assert(oconv("-1.5", "D") eq "29 DEC 1967");
	assert(oconv("1.5", "D")  eq "01 JAN 1968");
	assert(oconv("1.5" _FM "-1.5", "D") eq("01 JAN 1968" _FM "29 DEC 1967"));

	assert(oconv(14276, "D")    eq "31 JAN 2007");  // dd mmm yyyy
	assert(oconv(14276, "D2")   eq "31 JAN 07");    // dd mmm yy
	assert(oconv(14276, "D4")   eq "31 JAN 2007");  // dd mmmm yyyy
	assert(oconv(14276, "D/")   eq "01/31/2007");   // mm/dd/yyyy
	assert(oconv(14276, "D ")   eq "01 31 2007");   // mm dd yyyy
	assert(oconv(14276, "D2/")  eq "01/31/07");     // mm/dd/yyyy
	assert(oconv(14276, "D2-")  eq "01-31-07");     // mm-dd-yyyy
	assert(oconv(14276, "D/")   eq "01/31/2007");   // mm/dd/yyyy
	assert(oconv(14276, "D/E")  eq "31/01/2007");   // dd/mm/yyyy
	assert(oconv(14276, "D2 E") eq "31 01 07");     // dd mm yy
	assert(oconv(14276, "D S")  eq "2007 01 31");   // yyyy mm dd
	assert(oconv(14276, "DM")   eq "1");            // Month number 1-12
	assert(oconv(14276, "DMA")  eq "JANUARY");      // Month name
	assert(oconv(14276, "DW")   eq "3");            // week day number 1-7
	assert(oconv(14276, "DWA")  eq "WEDNESDAY");    // week day name
	assert(oconv(14276, "DY")   eq "2007");         // year 4 digits
	assert(oconv(14276, "DY2")  eq "7");            // year with 2 digits
	assert(oconv(14276, "D2Y")  eq "07 JAN 31");    // two digit year
	assert(oconv(14276, "D5Y")  eq "02007 JAN 31"); // 5 digit year?
	assert(oconv(14276, "DD")   eq "31");           // day number of month 1-31
	assert(oconv(14276, "DL")   eq "31");           // last day number of month
	assert(oconv(14276, "DQ")   eq "1");            // quarter number 1-4
	assert(oconv(14276, "DJ")   eq "31");           // day number of month 1-31

	var feb29_2004 = 13209;	 //iconv("29 FEB 2004","D");
	assert(oconv(feb29_2004, "DL") eq "29");

	//check does multivalues
	assert(oconv("14591" _VM _VM "14592", "D") eq "12 DEC 2007" _VM _VM "13 DEC 2007");
	assert(oconv("14591" _FM _VM "14592", "D") eq "12 DEC 2007" _FM _VM "13 DEC 2007");

	assert(oconv(14591, "D")   eq "12 DEC 2007");
	assert(oconv(14591, "D2/") eq "12/12/07");
	assert(oconv(14591, "D2-") eq "12-12-07");
	assert(oconv(14591, "D-")  eq "12-12-2007");
	assert(oconv(14591, "D2-") eq "12-12-07");
	assert(oconv(14591, "DJ")  eq "346");
	assert(oconv(14591, "DM")  eq "12");
	assert(oconv(14591, "DMA") eq "DECEMBER");
	assert(oconv(14591, "DW")  eq "3");
	assert(oconv(14591, "DWA") eq "WEDNESDAY");
	assert(oconv(14591, "DY")  eq "2007");
	assert(oconv(14591, "DQ")  eq "4");
	assert(oconv(14591, "DD")  eq "12");
	assert(oconv(14591, "DL")  eq "31");

	{
		var v1 = 19002;

		auto test = [&](in v1, in pattern, in expected, in comment = "") {
			printt(v1, pattern, expected, comment);
			printl();
			var actual = oconv(v1, pattern).squote();
			if (actual != expected.squote())
				TRACE(actual)
			return oconv(v1, pattern).squote() == expected.squote();
		};

		assert(test(v1, "D"			, "09 JAN 2020",	" Bare ''"));
		assert(test(v1, "DE"		, "09 JAN 2020",	" Bare E for international makes no difference without a separator"));
		assert(test(v1, "D!"		, "09JAN2020",  	" No separator"));
		assert(test(v1, "D/"		, "01/09/2020", 	" Separator - no E for international"));
		assert(test(v1, "DE/"		, "09/01/2020", 	" Separator with E for international"));
		assert(test(v1, "D/E"		, "09/01/2020", 	" Order doesnt matter"));

		assert(test(v1, "DZ"		, " 9 JAN 2020",	" Z - Spaces instead of leading zeros"));
		assert(test(v1, "DZZ"		, "9 JAN 2020", 	" ZZ - Trim leading leading zeros (no spaces)"));
		assert(test(v1, "DE/Z"		, " 9/ 1/2020",		" International date with / and leading zeros replaced with spaces"));

		assert(test(v1, "D!Z"		, " 9JAN2020",		" No separator"));
		assert(test(v1, "D!ZZ"		, "9JAN2020",		" No separator, trim leading zeros and spaces"));
		assert(test(v1, "D/Z"		, " 1/ 9/2020",		" American date with Separator / and Z = replace leading zeros with spaces"));
		assert(test(v1, "D/ZZ"		, "1/9/2020",		" American date with separator / and ZZ = remove leading zeros/spaces."));
		assert(test(v1, "DE/Z"		, " 9/ 1/2020",  	" International date with separator / and leading zeros replaced with spaces."));
		assert(test(v1, "DE/ZZ"		, "9/1/2020",		" International date with separator / Leading zeros and spaces removed."));
		assert(test(v1, "D!ZZ"		, "9JAN2020",     	" Universal date with no separators. Leading zeros and spaces removed."));
		assert(test(v1, "DE/ZZ"		, "9/1/2020",		" International date with / separator. Leading zeros and spaces removed."));
		assert(test(v1, "DE!Z"		, " 9JAN2020",		" Universal date with no separator. Leading zero replaced with space."));

		assert(test(v1, "D0"		, "09 JAN",			" Year last.  No year. Zero year digits"));
		assert(test(v1, "DS0"		, "JAN 09",			" Year first. No year. Zero year digits"));

//		// Check no change to old style call to [DATE oconv
		// New conversions on old style global DATEFMT
		// Should respect any 2 but always convert 0 to spaces
		printl();
		var save_DATEFMT = DATEFMT;
		DATEFMT = "D2/E";
		TRACE(DATEFMT)
		assert(test(v1, "[DATE]" 	, " 9/ 1/20",		" 1st Z assumed."));
		assert(test(v1, "[DATE,4]" 	, " 9/ 1/2020", 	" 1st Z assumed. Demands 4 digit year."));
		assert(test(v1, "[DATE,*4]" , "9/1/2020",    	" 1st Z assumed. * = 2nd Z option to remove spaces. Demands 4 digit year."));
		assert(test(v1, "[DATE,*]" 	, "9/1/20",     	" 1st Z assumed. * = 2nd Z option to remove spaces."));
		DATEFMT = save_DATEFMT;

		// 1st Z is always assumed with "[DATE" (unless arg starts with a D)
		// and it means remove leading zeros from day and month.

		// With new global DATEFMT "DE"
		printl();
		TRACE(DATEFMT)
		assert(test(v1, "[DATE]" 	, " 9 JAN 2020"	, "1st Z assumed. 4 digit year is the default."));
		assert(test(v1, "[DATE,4]" 	, " 9 JAN 2020"	, "1st Z assumed. Demands 4 digit year but is already the default"));
		assert(test(v1, "[DATE,*4]" , "9 JAN 2020"	, "1st Z assumed. * = 2nd Z option to remove spaces. Demands 4 digit year but it is already the default."));
		assert(test(v1, "[DATE,*]" 	, "9 JAN 2020"	, "1st Z assumed. * = 2nd Z option to remove spaces. 4 digit year is the default"));

		assert(test(v1, "[DATE,Z]" 	, "9 JAN 2020"	, "1st Z assumed. Z = 2nd Z option to remove spaces. 4 digit year is the default"));
		assert(test(v1, "[DATE,Z!]" , "9JAN2020"	, "1st Z assumed. Z = 2nd Z option to remove spaces. 4 digit year is the default"));

		// Only E is picked up from DATEFMT if the conv starts with a D
		assert(test(v1, "[DATE,D]" , "09 JAN 2020"	, "Only E option from DATEFMT is used and without a separator does nothing on output"));
		assert(test(v1, "[DATE,D!]" , "09JAN2020"	, "No separators"));
		assert(test(v1, "[DATE,D!Z]" , " 9JAN2020"	, "No separators. Leading zeros become spaces."));
		assert(test(v1, "[DATE,D!ZZ]" , "9JAN2020"	, "No separators. Leading zeros are removed."));

		// Check "[DATE" still picks up E flavour from DATEFMT
		assert(iconv("9 1 2020", "[DATE]") == 19002);
		assert(iconv("9 1 2020", "[DATE,D]") == 19002);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
