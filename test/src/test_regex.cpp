#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#define BOOST_REGEX 1

#include <exodus/program.h>
programinit()

function main() {

	//match returning what it finds

	//groups
	var csvline1="abcdef abcdef";
	var csvre1="(bc).(ef)";
	assert(match(csvline1,csvre1).convert(_FM_ _VM_,"^]")=="bcdef]bc]ef^bcdef]bc]ef");

	//no groups
	var csvline2="abcdef abcdef";
	var csvre2="bc.ef";
	assert(match(csvline2,csvre2).convert(_FM_ _VM_,"^]")=="bcdef^bcdef");

	//Gives the following error when using std::regex. No such error with boost:regex
	//Error: Invalid regex "(?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$)" "Invalid special open parenthesis."
	//
	//not asserted but a complicated CSV match
	var csvline=R"(123,2.99,AMO024,Title,"Description, more info",,123987564)";
	var csvre=R"tag((?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$))tag";

	var notposix_extended = R"(\(XYZ\))"; //escaped () will indicate groups to be captured
	var posix_extended = R"((XYZ))";      //unescaped () will match () in the source string

	assert("raw(XYZ)raw"_var.match(posix_extended, "i").dump() eq "XYZ]XYZ"_var);
	assert("raw(XYZ)raw"_var.match(notposix_extended, "i").dump() eq "(XYZ)"_var);

	// no need to backslash many letters

#define BOOST_REGEX 1
	{

        // \d \D and [:number:]
        assert(var("a1b23c99").regex_replace("\\d", "N") == "aNbNNcNN");
        assert(var("a1b23c99").regex_replace("\\D", "N") == "N1N23N99");
		if (BOOST_REGEX) {
	        assert(var("a1b23c99").regex_replace("[[:number:]]", "N") == "aNbNNcNN");
    	    assert(var("a1b23c99").regex_replace("[^[:number:]]", "N") == "N1N23N99");
		}

        // [:alnum:] = letters and numbers. No _
		if (BOOST_REGEX) {
	        assert(var("a1b2_ \n3c99").regex_replace("[[:alnum:]]", "N").outputl() == "NNNN_ \nNNNN");
    	    assert(var("a1b2_ \n3c99").regex_replace("[^[:alnum:]]", "N").outputl() == "a1b2NNN3c99");
		}

        // \w and \W in alnum + _
        assert(var("a1b2_ \n3c99").regex_replace("\\w", "N").outputl() == "NNNNN \nNNNN");
        assert(var("a1b2_ \n3c99").regex_replace("\\W", "N").outputl() == "a1b2_NN3c99");
		if (BOOST_REGEX) {
	        assert(var("a1b2_ \n3c99").regex_replace("[_[:alnum:]]", "N").outputl() == "NNNNN \nNNNN");
    	    assert(var("a1b2_ \n3c99").regex_replace("[^_[:alnum:]]", "N").outputl() == "a1b2_NN3c99");
		}

        // \s \S and [[:space:]]  means whitespace
        assert(var("a1b2_ \n3c99").regex_replace("\\s", "N").outputl() == "a1b2_NN3c99");
        assert(var("a1b2_ \n3c99").regex_replace("\\S", "N").outputl() == "NNNNN \nNNNN");
		if (BOOST_REGEX) {
    	    assert(var("a1b2_ \n3c99").regex_replace("[[:space:]]", "N").outputl() == "a1b2_NN3c99");
	        assert(var("a1b2_ \n3c99").regex_replace("[^[:space:]]", "N").outputl() == "NNNNN \nNNNN");
		}

        if (BOOST_REGEX) {
            // \{Number}
            assert(var("a1b2_ \n3c99").regex_replace("\\p{Number}", "N").outputl() == "aNbN_ \nNcNN");
        }

	}

	if (BOOST_REGEX) {
		assert(match(csvline,csvre).convert(_FM_ _VM_,"^]")==R"raw(123]]123^,2.99]]2.99^,AMO024]]AMO024^,Title]]Title^,"Description, more info"]"]Description, more info^,^,123987564]]123987564)raw");

		//unicode case insensitive finding
		assert(match("αβγδεΑΒΓΔΕ","(Α).(γδ)","i").convert(_FM_ _VM_,"^]")=="αβγδ]α]γδ^ΑΒΓΔ]Α]ΓΔ");

		//unicode case sensitive NOT finding
		assert(match("αβγδεΑΒΓΔΕ","(Α).(γδ)","").convert(_FM_ _VM_,"^]")=="");
	}

	var r1 = _FM_ "0.123";
	assert(r1.regex_replace("([\x1A-\x1F]-?)0.","$1.") == _FM_ ".123");
	var r2 = _STM_ "-0.123";
    assert(r2.regex_replace("([\x1A-\x1F]-?)0.","$1.") == _STM_ "-.123");

	//replacing unicode style numbers characters using javascript style regex
	//assert(var("Ⅻ").regex_replace(R"(\p{Number})","yes")=="yes");
	//assert(var("⅝").regex_replace(R"(\p{Number})","yes")=="yes");
	assert(var("1").regex_replace(R"([[:digit:]])","yes")=="yes");
	if (BOOST_REGEX) {
		assert(var("Ⅻ").regex_replace(R"(\p{Number})","yes")=="yes");
		assert(var("⅝").regex_replace(R"(\p{Number})","yes")=="yes");

		//assert(var("Ⅻ").regex_replace(R"(\p{Number})","yes").outputl()!="yes");
		//assert(var("⅝").regex_replace(R"(\p{Number})","yes").outputl()!="yes");
		assert(var("Ⅻ").regex_replace(R"([[:digit:]])","yes").outputl()!="yes");
		assert(var("⅝").regex_replace(R"([[:digit:]])","yes").outputl()!="yes");
	}

	//test glob matching using * ? eg *.* and *.??? etc
        assert(var("test.htm").match("*.*","w").outputl()		=="test.htm");
        assert(var("test.htm").match("*","w")		=="test.htm");
        assert(var("test.htm").match(".*","w")		=="");
        assert(var("test.htm").match(".","w")		=="");
        assert(var("test.htm").match("*.","w")		=="");
        assert(var("test.htm").match("*.htm","w")	=="test.htm");
        assert(var("test.html").match("*.htm","w")	=="");
        assert(var("test.htm").match("t*.???","w")	=="test.htm");
        assert(var("test.htm").match("t.???","w")	=="");
        assert(var("test.htm").match("x.???","w")	=="");
        assert(var("testx.htm").match("*x.???","w")	=="testx.htm");
        assert(var("test.html").match("t*.???","w")	=="");
        assert(var("test.html").match("*t?h*","w")	=="test.html");

	//test regular expression
	//four digits followed by dash or space) three times ... followed by four digits
	var regex1="(\\d{4}[- ]){3}\\d{4}";
	assert(var("1247-1234-1234-1234").match(regex1,"r").convert(_FM_ _VM_,"^]")=="1247-1234-1234-1234]1234-");
	assert(var("1247.1234-1234-1234").match(regex1,"r")=="");

	printl(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF"));
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF")=="文字- Kanji 漢字- Hanja 漢字(UTF");
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match(".*文字.*漢 字\\(UTF-8\\)")=="");


	{
		//test regex group and use group in replace
		//replace char+space with x+char+dash
		//TODO add g option
		var text="what a lot of money";
		assert(text.regex_replace("(.) ","x$1-")=="whaxt-xa-loxt-oxf-money");

		//simple test of regex and case insensitive regex swap (commonly known as replace)
		assert(regex_replace("abcd","b.","xyz").outputl() eq "axyzd");//right case to convert
		assert(regex_replace("abc","B.","xyz").outputl() eq "abc"); //wrong case to convert
		assert(regex_replace("abcd","B.","xyz","i").outputl() eq "axyzd");//case insensitive converts
		if (BOOST_REGEX)
			assert(regex_replace("abc","b.","xyz","l").outputl() eq "abc");//literal wont convert

		//simple test of case sensitive/insensitive swap
		assert(swap("abc","b","xyz").outputl() eq "axyzc");//will convert right case
		assert(swap("abc","B","xyz").outputl() eq "abc");//wont convert wrong case
		assert(regex_replace("abc","B","xyz","i").outputl() eq "axyzc");//will convert case insensitive
		if (BOOST_REGEX)
			assert(regex_replace("ab*c","B*","xyz","il").outputl() eq "axyzc");//will convert case insensitive but not regex

		assert(swap("abababab","ab","x").outputl() eq "xxxx");
		assert(regex_replace("abababab","ab","x").outputl() eq "xxxx");//regex
		assert(regex_replace("abababab","a.","xy").outputl() eq "xyxyxyxy");//regex
	}

	printl("Test passed");

	return 0;
}

programexit()

