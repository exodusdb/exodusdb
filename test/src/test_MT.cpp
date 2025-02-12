#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_MT says 'Hello World!'");

	// Fractional seconds using floor function where
	// A time is considered to be reached when you arrive at it exactly now before
	// So a fractional second before midnight is still 23:59:59
	// +0.9999 -> +0 -> 00:00:00
	// -0.0001 -> -1 -> 23:59:59
	assert(oconv(0.0001, "MTS").outputl()  eq "00:00:00");
	assert(oconv(0.5, "MTS").outputl()     eq "00:00:00");
	assert(oconv(-0.0001, "MTS").outputl() eq "23:59:59");
	assert(oconv(-0.9999, "MTS").outputl() eq "23:59:59");

	assert(iconv("23 59 59", "MT")      eq 86399);
	assert(iconv("xx11yy12zz13P", "MT") eq 83533);	//accept junk before and between
	assert(iconv("24 00 00", "MT") eq "");			//max hour is 23

	var time1 = var("10:10:10").iconv("MT");

	//check times around noon and midnight round trip ok
	for (const var ii : range(0, 61)) {
		assert(var(ii).outputl().oconv("MTHS").outputl().iconv("MTHS") eq ii);
	}
	for (const var ii : range(43200 - 61, 43200 + 61)) {
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);
	}

	//check oconv does multivalues
	assert(var("60" _RM "120").oconv("MT") eq "00:01" _RM "00:02");
	assert(var("60" _FM "120").oconv("MT") eq "00:01" _FM "00:02");
	assert(var("60" _VM "120").oconv("MT") eq "00:01" _VM "00:02");
	assert(var("60" _SM "120").oconv("MT") eq "00:01" _SM "00:02");
	assert(var("60" _TM "120").oconv("MT") eq "00:01" _TM "00:02");
	assert(var("60" _ST "120").oconv("MT") eq "00:01" _ST "00:02");

	//test that some random times iconv/oconv roundtrip ok
	initrnd(1000);
	var timex;
	for (int ii = 1; ii le 1000; ++ii) {
		timex = rnd(18600);
		//		timex.oconv("MTHS").output(" ").iconv("MTHS").outputl(" ");
		assert(timex.oconv("MTHS").iconv("MTHS") eq timex);
	}

	//oconv(46622,"MTH").outputl("oconv 46622 MTH is" );
	assert(oconv(46622, "MTH") eq "12:57PM");
	assert(oconv(31653, "MT")  eq "08:47");
	assert(oconv(63306, "MT")  eq "17:35");

	assert(oconv(0, "MTH")      eq "12:00AM");
	assert(oconv(31653, "MT")   eq "08:47");
	assert(oconv(63306, "MTH")  eq "05:35PM");
	assert(oconv(31653, "MTS")  eq "08:47:33");
	assert(oconv(63306, "MTS")  eq "17:35:06");
	assert(oconv(63306, "MTHS") eq "05:35:06PM");
	assert(oconv(63306, "MTS")  eq "17:35:06");
	assert(oconv(63306, "MTS.") eq "17.35.06");
	assert(oconv(63306, "MTh")  eq "17h35");

	assert(oconv(61201, "MT")   eq "17:00");
	assert(oconv(61201, "MTS")  eq "17:00:01");
	assert(oconv(61201, "MTH")  eq "05:00PM");
	assert(oconv(61201, "MTHS") eq "05:00:01PM");

	var time2 = 43261;
	assert(time2.oconv("MT")    eq "12:01");
	assert(time2.oconv("MTH")   eq "12:01PM");
	assert(time2.oconv("MTS")   eq "12:01:01");
	//assert(time2.oconv("MTSH") eq "12H01H01");
	assert(time2.oconv("MTSH")  eq "12:01:01PM");
	assert(time2.oconv("MTx")   eq "12x01");
	assert(time2.oconv("MTHx")  eq "12x01PM");
	assert(time2.oconv("MTSx")  eq "12x01x01");
	//assert(time2.oconv("MTSHx") eq "1201H01");
	assert(time2.oconv("MTSHx") eq "12x01x01PM");

	time2 = 0;
	assert(time2.oconv("MT")   eq "00:00");
	assert(time2.oconv("MTH")  eq "12:00AM");
	assert(time2.oconv("MTS")  eq "00:00:00");
	assert(time2.oconv("MTHS") eq "12:00:00AM");

	//negative time
	time2 = -1;
	assert(time2.oconv("MT")   eq "23:59");
	assert(time2.oconv("MTH")  eq "11:59PM");
	assert(time2.oconv("MTS")  eq "23:59:59");
	assert(time2.oconv("MTHS") eq "11:59:59PM");
	time2 = -86400 / 2;
	assert(time2.oconv("MT")   eq "12:00");
	assert(time2.oconv("MTH")  eq "12:00PM");
	assert(time2.oconv("MTS")  eq "12:00:00");
	assert(time2.oconv("MTHS") eq "12:00:00PM");
	time2 = -86400 - 1;
	assert(time2.oconv("MT")   eq "23:59");

	//test some unlimited time
	assert(var(-100).oconv("MTU")    eq "-00:01");
	assert(var(-100).oconv("MTUS")   eq "-00:01:40");
	assert(var(-10000).oconv("MTUS") eq "-02:46:40");

	assert(var(100).oconv("MTU")     eq "00:01");
	assert(var(100).oconv("MTUS")    eq "00:01:40");
	assert(var(1000).oconv("MTUS")   eq "00:16:40");
	assert(var(10000).oconv("MTUS")  eq "02:46:40");
	//NB 27:46:40 NOT ROUNDED UP TO 27:47 because mins like on clock
	assert(var(100000).oconv("MTU")  eq "27:46");
	assert(var(100000).oconv("MTUS") eq "27:46:40");

	//wrap around next midnight is 00:00
	assert(var(86400).oconv("MTS") eq "00:00:00");

	//wrap around 24 hours * 2 -> 00:00
	assert(var(86400 * 2).oconv("MTS") eq "00:00:00");

	//U = Unlimited hours
	assert(var(86400 * 2).oconv("MTUS")   eq "48:00:00");
	assert(var(86400 * 100).oconv("MTUS") eq "2400:00:00");

	//test some decimal hours based time
	assert(var(0).oconv("MT2")     eq "00:00");
	assert(var(0).oconv("MT2S")    eq "00:00:00");
	assert(var(0.25).oconv("MT2")  eq "00:15");
	assert(var(0.25).oconv("MT2S") eq "00:15:00");
	assert(var(24).oconv("MT2S")   eq "00:00:00");
	assert(var(25).oconv("MT2S")   eq "01:00:00");
	assert(var(-25).oconv("MT2S")  eq "23:00:00");

	//test some UNLIMITED decimal hours based time
	//NB negative unlimited time is symmetrical (unlike normal time_
	//negative time could be useful to refer to previous dates
	//but in a non-symmetrical fashion ie -1 means one second back into yesterday
	//ie 23:59:59 ... not -00:00:01. negative hours could perhaps be used
	//to credit hours in some imaginary volumetric numerical fashion
	//The "symmetric" feature should perhaps be a SEPARATE option
	//instead of being tied to the U unlimited option
	assert(var(.01).oconv("MT2US")     eq "00:00:36");
	assert(var(-.01).oconv("MT2US")    eq "-00:00:36");
	assert(var(.25).oconv("MT2US")     eq "00:15:00");
	assert(var(-.25).oconv("MT2US")    eq "-00:15:00");
	assert(var(25).oconv("MT2US")      eq "25:00:00");
	assert(var(-25).oconv("MT2US")     eq "-25:00:00");
	assert(var(125.25).oconv("MT2US")  eq "125:15:00");
	assert(var(-125.25).oconv("MT2US") eq "-125:15:00");
	assert(var(9).oconv("MT2US")       eq "09:00:00");
	assert(var(-9).oconv("MT2US")      eq "-09:00:00");

	//	assert(oconv(_FM "\x0035","HEX4") eq "00FE0035");
	//assert(oconv(_FM "\x0035","HEX4") eq "07FE0035");
	//assert(oconv(FM,"HEX4") eq "07FE");

	assert(oconv(43260, "MT")    eq "12:01");
	assert(oconv(43260, "MTH")   eq "12:01PM");
	assert(oconv(43260, "MTS")   eq "12:01:00");
	assert(oconv(43260, "MTHS")  eq "12:01:00PM");
	assert(oconv(61200, "MT")    eq "17:00");
	assert(oconv(61200, "MTHS,") eq "05,00,00PM");

	assert(oconv(61200, "MTHSZ") eq "05:00:00PM");
	assert(oconv(0, "MTHS")      eq "12:00:00AM");
	assert(oconv(0, "MTHSZ")     eq "");

	printl("Checking time oconv/iconv roundtrip for time (seconds) = 0 to 86400");
	//initrnd(999);
	//for (int ii = 0; ii le 86400-1; ++ii) {
	//	var time=ii;
	var started = ostime();
	for (const var itime : range(0, 86399)) {
		//		itime.outputl("itime=").oconv("MTHS").outputl("otime=").iconv("MTHS").outputl("itime=");
		assert(itime.oconv("MTHS").iconv("MTHS") eq itime);
		assert(itime.oconv("MTS").iconv("MTS")   eq itime);
	}
	var stopped = ostime();
	errputl(stopped - started, " seconds");

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
