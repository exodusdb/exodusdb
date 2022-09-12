#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

	function main() {

	{
		assert(oconv(1234.56, "MD0") eq "1235");
		assert(oconv(1234, "MD0")    eq "1234");

		assert(var(123.45).oconv("MD20P<") eq "123.45");
		assert(var(123.45).oconv("MD20P-") eq "123.45 ");
		assert(var(123.45).oconv("MD20PD") eq "123.45  ");
		assert(var(123.45).oconv("MD20PC") eq "123.45  ");

		assert(var(-123.45).oconv("MD20P<") eq "<123.45>");
		assert(var(-123.45).oconv("MC20P-") eq "123,45-");
		assert(var(-123.45).oconv("MD20PD") eq "123.45DB");
		assert(var(-123.45).oconv("MD20PC") eq "123.45CR");

		assert(var(-123).oconv("MD20P<")     eq "<123.00>");
		assert(var(-123.4).oconv("MD20P<")   eq "<123.40>");
		assert(var(-123.451).oconv("MC20P-") eq "123,45-");

		assert(var(-123456789.123).oconv("MC20P-,")    eq "123.456.789,12-");
		assert(var(-23456789.123).oconv("MC20P-,")     eq "23.456.789,12-");
		assert(var(-3456789.123).oconv("MC20P-,")      eq "3.456.789,12-");
		assert(var(-123456789.123456).oconv("MC60P-,") eq "123.456.789,123456-");

		assert(oconv(".56", "MD0") eq "1");

		assert(var(".45").oconv("MD20P<") eq "0.45");
		assert(var(".45").oconv("MD20P-") eq "0.45 ");
		assert(var(".45").oconv("MD20PD") eq "0.45  ");
		assert(var(".45").oconv("MD20PC") eq "0.45  ");

		assert(var("-.45").oconv("MD20P<") eq "<0.45>");
		assert(var("-.45").oconv("MC20P-") eq "0,45-");
		assert(var("-.45").oconv("MD20PD") eq "0.45DB");
		assert(var("-.45").oconv("MD20PC") eq "0.45CR");

		assert(var("-123").oconv("MD20P<")     eq "<123.00>");
		assert(var("-123.4").oconv("MD20P<")   eq "<123.40>");
		assert(var("-123.451").oconv("MC20P-") eq "123,45-");

		assert(var("-123456789.123").oconv("MC20P-,")    eq "123.456.789,12-");
		assert(var("-23456789.123").oconv("MC20P-,")     eq "23.456.789,12-");
		assert(var("-3456789.123").oconv("MC20P-,")      eq "3.456.789,12-");
		assert(var("-123456789.123456").oconv("MC60P-,") eq "123.456.789,123456-");
	}

	{
		//assert(setxlocale("fr_FR.utf8"));
		//assert(setxlocale(1036));
		var xx3 = "1234.5678";
		assert(xx3 + 1 eq 1235.5678);

		TRACE(oconv("", "MD20P"))
		assert(oconv("", "MD20P")      eq "");
		assert(oconv("X", "MD20P")     eq "X");
		assert(oconv("0", "MD20P")     eq "0.00");
		assert(oconv("0.0", "MD20P")   eq "0.00");
		assert(oconv("00.00", "MD20P") eq "0.00");
		assert(oconv("000", "MD20P")   eq "0.00");

		assert(oconv("", "MD20PZ")      eq "");
		assert(oconv("X", "MD20PZ")     eq "X");
		assert(oconv("0", "MD20PZ")     eq "");
		assert(oconv("0.0", "MD20PZ")   eq "");
		assert(oconv("00.00", "MD20PZ") eq "");
		assert(oconv("000", "MD20PZ")   eq "");

		assert(oconv(xx3, "MD20P")        eq "1234.57");
		assert(oconv(1234.567, "MD20P")   eq "1234.57");
		assert(oconv("1234.567", "MD20P") eq "1234.57");

		assert(oconv("", "MD20P")  eq "");
		assert(oconv("", "MD20PZ") eq "");

		//assert(var(L"123456") eq "123456");

		TRACE(_VM "0" _VM _VM)
		TRACE("MD20PZ")
		TRACE(oconv(_VM "0" _VM _VM, "MD20PZ"));
		assert(oconv(_VM "0" _VM _VM, "MD20PZ") eq _VM _VM _VM);

		assert(oconv(1234.567, "MC20PZ") eq "1234,57");

		//from https://arev.neosys.com/x/4AEz.html
		assert(oconv(1234, "MD0")    eq "1234");
		assert(oconv(1234, "MD2")    eq "12.34");
		assert(oconv(12, "MD2")      eq ".12");
		//assert(oconv(12,     "MD2L")          eq "0.12");
		assert(oconv(150000, "MD24") eq "15.00");
		assert(oconv(1234, "MD2$")   eq "$12.34");
		assert(oconv(-1234, "MD2$")  eq "$-12.34");
		assert(oconv(1234, "MD2$C")  eq "$12.34  ");
		assert(oconv(1234, "MD2$D")  eq "$12.34  ");
		assert(oconv(-1234, "MD2$C") eq "$12.34CR");
		assert(oconv(-1234, "MD2$D") eq "$12.34DB");
		assert(oconv(-1234, "MD1-")  eq "123.4-");
		//assert(oconv(1234,     "MD2,10*")       eq "*****12.34");
		assert(oconv(500, "MD0#")    eq "#500");
		//assert(oconv(50,     "MD0[DM]")       eq "DM50");
		//assert(oconv(50,     "MD0[DM ]")      eq "DM 50");
		//assert(oconv(252525, "MC2.[<_>Bfr]S") eq "2.525,25 Bfr");
		assert(oconv(-12345, "MC1.") eq "-1.234,5");
		assert(oconv(-12345, "MC1,") eq "-1.234,5");
	}

	{
		assert(var(1).oconv("MD60PZ").squote()           eq "'1.000000'");
		//Z means suppress zeros
		//Z means suppress zeros
		assert(var(0.000000001).oconv("MD60PZ").squote() eq "''");
		assert(var(0.00000001).oconv("MD60PZ").squote()  eq "''");
		assert(var(0.0000001).oconv("MD60PZ").squote()   eq "''");
		assert(var(0.000001).oconv("MD60PZ").squote()    eq "'0.000001'");
		assert(var(0.00001).oconv("MD60PZ").squote()     eq "'0.000010'");
		assert(var(0.0001).oconv("MD60PZ").squote()      eq "'0.000100'");
		//assert(var(0.0001)   .oconv("MD60PZ").squote() eq "''");

		assert(var(0.000000000001).oconv("MD60P").squote() eq "'0.000000'");
		assert(var(0.000001).oconv("MD60P").squote()       eq "'0.000001'");
		assert(var(0.00001).oconv("MD60P").squote()        eq "'0.000010'");
		assert(var(0.0001).oconv("MD60P").squote()         eq "'0.000100'");
		assert(var(-0.000001).oconv("MD60P").squote()      eq "'-0.000001'");
		assert(var(-0.00001).oconv("MD60P").squote()       eq "'-0.000010'");
		assert(var(-0.0001).oconv("MD60P").squote()        eq "'-0.000100'");

		assert(var(1E9).oconv("MD60P").squote()  eq "'1000000000.000000'");
		assert(var(1E10).oconv("MD60P").squote() eq "'10000000000.000000'");
		assert(var(1E11).oconv("MD60P").squote() eq "'100000000000.000000'");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
