#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#define BOOST_REGEX 1

#include <exodus/program.h>
programinit()

	function main() {

	{
		// first
		assert(regex_replace("aaa bbb ccc", "b", "Q", "f") eq "aaa Qbb ccc");

		// all
		assert(regex_replace("aaa bbb ccc", "b", "Q") eq "aaa QQQ ccc");
	}

	//match returning what it finds

	//groups
	var csvline1 = "abcdef abcdef";
	var csvre1	 = "(bc).(ef)";
	assert(match(csvline1, csvre1).convert(_FM _VM, "^]") eq "bcdef]bc]ef^bcdef]bc]ef");

	//no groups
	var csvline2 = "abcdef abcdef";
	var csvre2	 = "bc.ef";
	assert(match(csvline2, csvre2).convert(_FM _VM, "^]") eq "bcdef^bcdef");

	//Gives the following error when using std::regex. No such error with boost:regex
	//Error: Invalid regex "(?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$)" "Invalid special open parenthesis."
	//
	//not asserted but a complicated CSV match
	var csvline = R"(123,2.99,AMO024,Title,"Description, more info",,123987564)";
	var csvre	= R"tag((?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$))tag";

	var notposix_extended = R"(\(XYZ\))";  //escaped () will indicate groups to be captured
	var posix_extended	  = R"((XYZ))";	   //unescaped () will match () in the source string

	assert("raw(XYZ)raw"_var.match(posix_extended, "i").outputl()    eq "XYZ]XYZ"_var);
	assert("raw(XYZ)raw"_var.match(notposix_extended, "i").outputl() eq "(XYZ)"_var);

	// no need to backslash many letters

#define BOOST_REGEX 1
	{

		// \d \D and [:number:]
		assert(var("a1b23c99").regex_replace("\\d", "N")            eq "aNbNNcNN");
		assert(var("a1b23c99").regex_replace("\\D", "N")            eq "N1N23N99");
		if (BOOST_REGEX) {
			assert(var("a1b23c99").regex_replace("[[:number:]]", "N")  eq "aNbNNcNN");
			assert(var("a1b23c99").regex_replace("[^[:number:]]", "N") eq "N1N23N99");
		}

		// [:alnum:] = letters and numbers. No _
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").regex_replace("[[:alnum:]]", "N").outputl()  eq "NNNN_ \nNNNN");
			assert(var("a1b2_ \n3c99").regex_replace("[^[:alnum:]]", "N").outputl() eq "a1b2NNN3c99");
		}

		// \w and \W in alnum + _
		assert(var("a1b2_ \n3c99").regex_replace("\\w", "N").outputl()            eq "NNNNN \nNNNN");
		assert(var("a1b2_ \n3c99").regex_replace("\\W", "N").outputl()            eq "a1b2_NN3c99");
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").regex_replace("[_[:alnum:]]", "N").outputl()  eq "NNNNN \nNNNN");
			assert(var("a1b2_ \n3c99").regex_replace("[^_[:alnum:]]", "N").outputl() eq "a1b2_NN3c99");
		}

		// \s \S and [[:space:]]  means whitespace
		assert(var("a1b2_ \n3c99").regex_replace("\\s", "N").outputl()           eq "a1b2_NN3c99");
		assert(var("a1b2_ \n3c99").regex_replace("\\S", "N").outputl()           eq "NNNNN \nNNNN");
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").regex_replace("[[:space:]]", "N").outputl()  eq "a1b2_NN3c99");
			assert(var("a1b2_ \n3c99").regex_replace("[^[:space:]]", "N").outputl() eq "NNNNN \nNNNN");
		}

		if (BOOST_REGEX) {
			// \{Number}
			assert(var("a1b2_ \n3c99").regex_replace("\\p{Number}", "N").outputl() eq "aNbN_ \nNcNN");
		}
	}

	if (BOOST_REGEX) {
		assert(match(csvline, csvre).convert(_FM _VM, "^]") eq R"raw(123]]123^,2.99]]2.99^,AMO024]]AMO024^,Title]]Title^,"Description, more info"]"]Description, more info^,^,123987564]]123987564)raw");

		//unicode case insensitive finding
		assert(match("αβγδεΑΒΓΔΕ", "(Α).(γδ)", "i").convert(_FM _VM, "^]") eq "αβγδ]α]γδ^ΑΒΓΔ]Α]ΓΔ");

		//unicode case sensitive NOT finding
		assert(match("αβγδεΑΒΓΔΕ", "(Α).(γδ)", "").convert(_FM _VM, "^]") eq "");
	}

	var r1 = _FM "0.123";
	assert(r1.regex_replace("([\x1A-\x1F]-?)0.", "$1.") eq _FM ".123");
	var r2 = _ST "-0.123";
	assert(r2.regex_replace("([\x1A-\x1F]-?)0.", "$1.") eq _ST "-.123");

	//replacing unicode style numbers characters using javascript style regex
	//assert(var("Ⅻ").regex_replace(R"(\p{Number})","yes")=="yes");
	//assert(var("⅝").regex_replace(R"(\p{Number})","yes")=="yes");
	assert(var("1").regex_replace(R"([[:digit:]])", "yes") eq "yes");
	if (BOOST_REGEX) {
		assert(var("Ⅻ").regex_replace(R"(\p{Number})", "yes") eq "yes");
		assert(var("⅝").regex_replace(R"(\p{Number})", "yes") eq "yes");

		//assert(var("Ⅻ").regex_replace(R"(\p{Number})","yes").outputl()!="yes");
		//assert(var("⅝").regex_replace(R"(\p{Number})","yes").outputl()!="yes");
		assert(var("Ⅻ").regex_replace(R"([[:digit:]])", "yes").outputl() != "yes");
		assert(var("⅝").regex_replace(R"([[:digit:]])", "yes").outputl() != "yes");
	}

	//test glob matching using * ? eg *.* and *.??? etc
	assert(var("test.htm").match("*.*", "w").outputl() eq "test.htm");
	assert(var("test.htm").match("*", "w")             eq "test.htm");
	assert(var("test.htm").match(".*", "w")            eq "");
	assert(var("test.htm").match(".", "w")             eq "");
	assert(var("test.htm").match("*.", "w")            eq "");
	assert(var("test.htm").match("*.htm", "w")         eq "test.htm");
	assert(var("test.html").match("*.htm", "w")        eq "");
	assert(var("test.htm").match("t*.???", "w")        eq "test.htm");
	assert(var("test.htm").match("t.???", "w")         eq "");
	assert(var("test.htm").match("x.???", "w")         eq "");
	assert(var("testx.htm").match("*x.???", "w")       eq "testx.htm");
	assert(var("test.html").match("t*.???", "w")       eq "");
	assert(var("test.html").match("*t?h*", "w")        eq "test.html");

	//test regular expression
	//four digits followed by dash or space) three times ... followed by four digits
	var regex1 = "(\\d{4}[- ]){3}\\d{4}";
	assert(var("1247-1234-1234-1234").match(regex1, "r").convert(_FM _VM, "^]") eq "1247-1234-1234-1234]1234-");
	assert(var("1247.1234-1234-1234").match(regex1, "r")                        eq "");

	printl(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF"));
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF")         eq "文字- Kanji 漢字- Hanja 漢字(UTF");
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match(".*文字.*漢 字\\(UTF-8\\)") eq "");

	{
		//test regex group and use group in replace
		//replace char+space with x+char+dash
		//TODO add g option
		var text = "what a lot of money";
		assert(text.regex_replace("(.) ", "x$1-") eq "whaxt-xa-loxt-oxf-money");

		//simple test of regex and case insensitive regex swap (commonly known as replace)
		assert(regex_replace("abcd", "b.", "xyz").outputl() eq "axyzd");	   //right case to convert
		assert(regex_replace("abc", "B.", "xyz").outputl() eq "abc");		   //wrong case to convert
		assert(regex_replace("abcd", "B.", "xyz", "i").outputl() eq "axyzd");  //case insensitive converts
		if (BOOST_REGEX)
			assert(regex_replace("abc", "b.", "xyz", "l").outputl() eq "abc");	//literal wont convert

		//simple test of case sensitive/insensitive swap
		assert(replace("abc", "b", "xyz").outputl() eq "axyzc");				 //will convert right case
		assert(replace("abc", "B", "xyz").outputl() eq "abc");					 //wont convert wrong case
		assert(regex_replace("abc", "B", "xyz", "i").outputl() eq "axyzc");	 //will convert case insensitive
		if (BOOST_REGEX)
			assert(regex_replace("ab*c", "B*", "xyz", "il").outputl() eq "axyzc");	//will convert case insensitive but not regex

		assert(replace("abababab", "ab", "x").outputl()           eq "xxxx");
		assert(regex_replace("abababab", "ab", "x").outputl() eq "xxxx");		//regex
		assert(regex_replace("abababab", "a.", "xy").outputl() eq "xyxyxyxy");	//regex
	}

	{
		var utf8 = "αβγδεΑΒΓΔΕ";
		var notutf8 = "       \xff"_var;
		TRACE(notutf8.oconv("HEX"))

		// MATCHING

		// bad data
		try {
			notutf8.match("X").errputl();
			assert(false);
		} catch (VarError e) {errputl(e.description);}

		// bad target
		try {
			utf8.match(notutf8).errputl();
			assert(false);
		} catch (VarError e) {errputl(e.description);}

		// REPLACING

		// bad data
		try {
			notutf8.regex_replace("X", "Y").errputl();
			assert(false);
		} catch (VarError e) {errputl(e.description);}

		// bad target
		try {
			utf8.regex_replace(notutf8, "Y").errputl();
			assert(false);
		} catch (VarError e) {errputl(e.description);}

		// bad replacement
		try {
			utf8.regex_replace("X", notutf8).errputl();
			assert(false);
		} catch (VarError e) {errputl(e.description);}
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
