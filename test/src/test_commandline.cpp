#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

	function main() {

    assert(osshellread("./testcommandline '(ab)'") eq "testcommandline (ab)\ntestcommandline\nab\n");
    assert(osshellread("./testcommandline '{ab}'") eq "testcommandline {ab}\ntestcommandline\nab\n");

    assert(osshellread("./testcommandline a b c '(ab)'") eq "testcommandline a b c (ab)\ntestcommandline^a^b^c\nab\n");
    assert(osshellread("./testcommandline a b c '{ab}'") eq "testcommandline a b c {ab}\ntestcommandline^a^b^c\nab\n");

    assert(osshellread("./testcommandline a b c '(a b)'").outputl() eq "testcommandline a b c (a b)\ntestcommandline^a^b^c\na b\n");
    assert(osshellread("./testcommandline a b c '{a b}'").outputl() eq "testcommandline a b c {a b}\ntestcommandline^a^b^c\na b\n");

    assert(osshellread("./testcommandline a b c '(' a or b ')'").outputl() eq "testcommandline a b c ( a or b )\ntestcommandline^a^b^c^(^a^or^b^)\n\n");

	// Syntax error
//    assert(osshellread("./testcommandline a b c (a b)").outputl() eq "testcommandline a b c (a b)\ntestcommandline^a^b^c^(a^b)\n\n");
//    assert(osshellread("./testcommandline a b c {a b}").outputl() eq "testcommandline a b c {a b}\ntestcommandline^a^b^c^{a^b}\n\n");

    assert(osshellread("./testcommandline '(' a b ')'") eq "testcommandline ( a b )\ntestcommandline^(^a^b^)\n\n");

	perform("testcommandlib a b c (xyz)");
	TRACE(USER1)
	// TODO COMMAND should be parsed in libexodus (exoprog.cpp) in FM separated parts as is done for COMMAND from the command line
	//assert(USER1 eq "testcommandlib a b c (xyz)\ntestcommandlib^a^b^c\nxyz\n");
	assert(USER1 eq "testcommandlib a b c (xyz)\ntestcommandlib a b c\nxyz\n");

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
