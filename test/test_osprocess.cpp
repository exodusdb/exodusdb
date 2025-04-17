#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("p11 says 'Hello World!'");

	printl("Test running a command via bash");
	test("bash -c \"grep xyz\"", "abc\nxyz", "xyz\n", "", true, 0, "");

	printl("Test running an invalid grep command via bash");
	test("bash -c \"grep -QZP xyz\"", "abc\nxyz", "",
"grep: invalid option -- 'Q'\n"
"Usage: grep [OPTION]... PATTERNS [FILE]...\n"
"Try 'grep --help' for more information.\n", false, 2, "var::osprocess failed. 2 \"bash -c \"grep -QZP xyz\"\"");

	printl("Test empty cmd");
	test("", "", "", "", false, 127, "var::osprocess failed. 127 \"\"");

	printl("Test no such program");
	test("/QQ1/QQ2", "", "", "execvp failed: No such file or directory\n", false, 127, "var::osprocess failed. 127 \"/QQ1/QQ2\"");

	printl("Test grep one line of three");
	test("grep xyz", "abc\nxyz 123\ndef", "xyz 123\n", "", true, 0, "");

	printl("Test grep finds nothing.");
	test("grep qxyz", "abc\nxyz 123\ndef", "", "", false, 1, "var::osprocess failed. 1 \"grep qxyz\"");

	printl("Test grep invalid file gives stdout");
	test("grep xyz /QQ1/QQ2", "abc\nxyz 123\ndef", "", "grep: /QQ1/QQ2: No such file or directory\n", false, 2, "var::osprocess failed. 2 \"grep xyz /QQ1/QQ2\"");

	if (not osshell("sleep .25 && killall sleep&")) loglasterror();
	printl("Test being killall'ed");
	test("sleep 1", "", "", "", false, 143, "var::osprocess failed. 143 \"sleep 1\"");

	if (not osshell("sleep .25 && killall -SIGHUP sleep&")) loglasterror();
	printl("Test being SIGHUP'ed ");
	test("sleep 1", "", "", "", false, 129, "var::osprocess failed. 129 \"sleep 1\"");

	printl("Test timeout 1 sec");
	test("sleep 3", "", "", "", false, -1, "var::osprocess failed. -1 \"sleep 3\"", 1);

	printl("Test cat 163MB returns the same");
	test("cat", space(163'000'001), space(163'000'001), "", true, 0, "");

	printl("Test bash cat 163MB to stderr and exit 42");
	test("bash -c \"cat >&2 && exit 42\"", space(163'000'001), "", space(163'000'001), false, 42, "var::osprocess failed. 42 \"bash -c \"cat >&2 && exit 42\"\"");

	printl("Test 100 x cat 3MB returns the same");
	test("cat", space(3'000'001), space(3'000'001), "", true, 0, "", 0, 100);

	printl("Test speed of 100 x echo x");
	test("echo x", "", "x\n", "", true, 0, "", 0, 100);

	printl("Test speed of 100 x true");
	test("true", "", "", "", true, 0, "", 0, 100);

	stop("Test passed.");
}
function test(in cmd, in stdin, in stdout_exp, in stderr_exp, in result_exp, in exit_status_exp, in lasterror_exp, in timeout_secs = 0, int nrepeats = 1) {

	TRACE(cmd)
	let started = ostime();

	var stdout_act, stderr_act, exit_status_act;
	bool result_act;
	for (auto rep [[maybe_unused]] : range(1, nrepeats))
		result_act = osprocess(cmd, stdin, stdout_act, stderr_act, exit_status_act, timeout_secs);

	let stopped = (ostime() - started) / nrepeats / 86400;
	let time_per_call = elapsedtimetext(0, stopped);

	TRACE(time_per_call)

	TRACE(stdout_act.first(64))
	if (stdout_act ne stdout_exp)
		TRACE(stdout_exp.first(64))

	TRACE(stderr_act.first(64))
	if (stderr_act ne stderr_exp)
		TRACE(stderr_exp.first(64))

	TRACE(result_act)
	if (result_act ne result_exp)
		TRACE(result_exp)

	TRACE(exit_status_act)
	if (exit_status_act ne exit_status_exp)
		TRACE(exit_status_exp)

	let lasterror_act = lasterror();
	TRACE(lasterror_act);
	if (lasterror_act ne lasterror_exp)
		TRACE(lasterror_exp);

	assert(stdout_act == stdout_exp);
	assert(stderr_act == stderr_exp);
	assert(result_act == result_exp);
	assert(exit_status_act == exit_status_exp);
	assert(lasterror_act == lasterror_exp);

	logputl();
	return true;
}

}; // programexit()
