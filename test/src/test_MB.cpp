#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function test_ioconv(in ovar, in ivar) {
	logputl();
	TRACE(ovar)
	TRACE(ivar)
	TRACE(ovar.iconv("MB"))
	TRACE(ivar.oconv("MB").trimfirst("0"))
	if (ovar.iconv("MB") ne ivar)
		return false;
	if (len(ivar) and ivar.oconv("MB").trimfirst("0") ne ovar)
		return false;
	return true;
}

function main() {

	assert(test_ioconv("111111111111111111111111111111111111111111111111111111111111111", "9223372036854775807"));
	assert(test_ioconv("1111111111111111111111111111111111111111111111111111111111111111", "-1"));
	assert(var(0).iconv("MB") eq "0");
	assert(test_ioconv("1", "1"));
	assert(test_ioconv("10", "2"));
	assert(test_ioconv("11", "3"));
	assert(test_ioconv("11111111", "255"));
	assert(test_ioconv("11111111", "255"));
	assert(test_ioconv("100000000", "256"));
	assert(test_ioconv("1111111111111111", "65535"));
	assert(test_ioconv("10000000000000000", "65536"));

	assert(test_ioconv("111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", ""));
	assert(test_ioconv("2", ""));
	assert(test_ioconv("", ""));
	assert(test_ioconv("-1", ""));
	assert(test_ioconv("-", ""));

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
