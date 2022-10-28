#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main() {

	// Quit if on github where unknown how to call another built test program
	if (osgetenv("GITHUB_WORKSPACE")) {
		printl("Test passed");
		return 0;
	}

	var env = osgetenv();
	TRACE(osgetenv("PATH"))
	TRACE(env)
	TRACE(oslist())
	assert(env.fcount(FM));

	TRACE(COMMAND)
	TRACE(EXECPATH)
	var execdir = EXECPATH.field(OSSLASH, 1, fcount(EXECPATH, OSSLASH) - 1);
//	execdir = "";
	execdir = "./";
	TRACE(execdir ^ "testcommandline");

	if (not osfile(execdir ^ "testcommandline"))
		abort("Test passed because testcommandline cannot be found");

	assert(osshell(execdir ^ "testcommandline '(ab)'"));
	assert(osshellread(execdir ^ "testcommandline '(ab)'") eq "testcommandline (ab)\ntestcommandline\nab\n");
	assert(osshellread(execdir ^ "testcommandline '{ab}'") eq "testcommandline {ab}\ntestcommandline\nab\n");

	assert(osshellread(execdir ^ "testcommandline a b c '(ab)'") eq "testcommandline a b c (ab)\ntestcommandline^a^b^c\nab\n");
	assert(osshellread(execdir ^ "testcommandline a b c '{ab}'") eq "testcommandline a b c {ab}\ntestcommandline^a^b^c\nab\n");

	assert(osshellread(execdir ^ "testcommandline a b c '(a b)'").outputl() eq "testcommandline a b c (a b)\ntestcommandline^a^b^c\na b\n");
	assert(osshellread(execdir ^ "testcommandline a b c '{a b}'").outputl() eq "testcommandline a b c {a b}\ntestcommandline^a^b^c\na b\n");

	assert(osshellread(execdir ^ "testcommandline a b c '(' a or b ')'").outputl() eq "testcommandline a b c ( a or b )\ntestcommandline^a^b^c^(^a^or^b^)\n\n");

	// Syntax error
//    assert(osshellread(execdir ^ "testcommandline a b c (a b)").outputl() eq "testcommandline a b c (a b)\ntestcommandline^a^b^c^(a^b)\n\n");
//    assert(osshellread(execdir ^ "testcommandline a b c {a b}").outputl() eq "testcommandline a b c {a b}\ntestcommandline^a^b^c^{a^b}\n\n");

    assert(osshellread(execdir ^ "testcommandline '(' a b ')'") eq "testcommandline ( a b )\ntestcommandline^(^a^b^)\n\n");

	if (libinfo("testcommandlib")) {
		perform("testcommandlib a b c (xyz)");
		//if (not oswrite(USER1 on "x"))
		//	loglasterror();
		TRACE(USER1)
		// TODO COMMAND should be parsed in libexodus (exoprog.cpp) in FM separated parts as is done for COMMAND from the command line
		//assert(USER1 eq "testcommandlib a b c (xyz)\ntestcommandlib^a^b^c\nxyz\n");
		//assert(USER1 eq "testcommandlib a b c (xyz)\ntestcommandlib a b c\nxyz\n");
		assert(USER1 eq "testcommandlib a b c (xyz)\ntestcommandlib^a^b^c\nxyz\n"_var);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
