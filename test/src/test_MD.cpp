#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main()
{

	{
		assert(oconv(1234.56, "MD0").outputl() eq "1235");
		assert(oconv(1234, "MD0").outputl() eq "1234");

		assert(var(123.45).oconv("MD20P<").outputl() eq "123.45");
		assert(var(123.45).oconv("MD20P-").outputl() eq "123.45 ");
		assert(var(123.45).oconv("MD20PD").outputl() eq "123.45  ");
		assert(var(123.45).oconv("MD20PC").outputl() eq "123.45  ");

		assert(var(-123.45).oconv("MD20P<").outputl() eq "<123.45>");
		assert(var(-123.45).oconv("MC20P-").outputl() eq "123,45-");
		assert(var(-123.45).oconv("MD20PD").outputl() eq "123.45DB");
		assert(var(-123.45).oconv("MD20PC").outputl() eq "123.45CR");

		assert(var(-123).oconv("MD20P<").outputl() eq "<123.00>");
		assert(var(-123.4).oconv("MD20P<").outputl() eq "<123.40>");
		assert(var(-123.451).oconv("MC20P-").outputl() eq "123,45-");

		assert(var(-123456789.123).oconv("MC20P-,").outputl() eq "123.456.789,12-");
		assert(var(-23456789.123).oconv("MC20P-,").outputl() eq "23.456.789,12-");
		assert(var(-3456789.123).oconv("MC20P-,").outputl() eq "3.456.789,12-");

		assert(var(-123456789.123456).oconv("MC60P-,").outputl() eq "123.456.789,123456-");



		assert(oconv(".56", "MD0").outputl() eq "1");

		assert(var(".45").oconv("MD20P<").outputl() eq "0.45");
		assert(var(".45").oconv("MD20P-").outputl() eq "0.45 ");
		assert(var(".45").oconv("MD20PD").outputl() eq "0.45  ");
		assert(var(".45").oconv("MD20PC").outputl() eq "0.45  ");

		assert(var("-.45").oconv("MD20P<").outputl() eq "<0.45>");
		assert(var("-.45").oconv("MC20P-").outputl() eq "0,45-");
		assert(var("-.45").oconv("MD20PD").outputl() eq "0.45DB");
		assert(var("-.45").oconv("MD20PC").outputl() eq "0.45CR");

		assert(var("-123").oconv("MD20P<").outputl() eq "<123.00>");
		assert(var("-123.4").oconv("MD20P<").outputl() eq "<123.40>");
		assert(var("-123.451").oconv("MC20P-").outputl() eq "123,45-");

		assert(var("-123456789.123").oconv("MC20P-,").outputl() eq "123.456.789,12-");
		assert(var("-23456789.123").oconv("MC20P-,").outputl() eq "23.456.789,12-");
		assert(var("-3456789.123").oconv("MC20P-,").outputl() eq "3.456.789,12-");

		assert(var("-123456789.123456").oconv("MC60P-,").outputl() eq "123.456.789,123456-");

	}

	{
		//assert(setxlocale("fr_FR.utf8"));
		//assert(setxlocale(1036));
		var xx3="1234.5678";
		assert(xx3+1==1235.5678);

		TRACE(oconv("","MD20P"))
		assert(oconv("","MD20P")=="");
		assert(oconv("X","MD20P")=="X");
		assert(oconv("0","MD20P")=="0.00");
		assert(oconv("0.0","MD20P")=="0.00");
		assert(oconv("00.00","MD20P")=="0.00");
		assert(oconv("000","MD20P")=="0.00");

		assert(oconv("","MD20PZ")=="");
		assert(oconv("X","MD20PZ")=="X");
		assert(oconv("0","MD20PZ")=="");
		assert(oconv("0.0","MD20PZ")=="");
		assert(oconv("00.00","MD20PZ")=="");
		assert(oconv("000","MD20PZ")=="");

		assert(oconv(xx3,"MD20P")=="1234.57");
		assert(oconv(1234.567,"MD20P")=="1234.57");
		assert(oconv("1234.567","MD20P")=="1234.57");

		assert(oconv("","MD20P")=="");
		assert(oconv("","MD20PZ")=="");
		assert(oconv(_VM_ "0" _VM_ _VM_,"MD20PZ")== _VM_ _VM_ _VM_);

		assert(oconv(1234.567,"MC20PZ")=="1234,57");

		//from https://arev.neosys.com/x/4AEz.html
		assert(oconv(1234,     "MD0")           == "1234");
		assert(oconv(1234,     "MD2")           == "12.34");
		assert(oconv(12,       "MD2")           == ".12");
		//assert(oconv(12,     "MD2L")          == "0.12");
		assert(oconv(150000,   "MD24")          == "15.00");
		assert(oconv(1234,     "MD2$")          == "$12.34");
		assert(oconv(-1234,    "MD2$")          == "$-12.34");
		assert(oconv(1234,     "MD2$C")         == "$12.34  ");
		assert(oconv(1234,     "MD2$D")         == "$12.34  ");
		assert(oconv(-1234,    "MD2$C")         == "$12.34CR");
		assert(oconv(-1234,    "MD2$D")         == "$12.34DB");
		assert(oconv(-1234,    "MD1-")          == "123.4-");
		//assert(oconv(1234,     "MD2,10*")       == "*****12.34");
		assert(oconv(500,      "MD0#")          == "#500");
		//assert(oconv(50,     "MD0[DM]")       == "DM50");
		//assert(oconv(50,     "MD0[DM ]")      == "DM 50");
		//assert(oconv(252525, "MC2.[<_>Bfr]S") == "2.525,25 Bfr");
		assert(oconv(-12345,   "MC1.")          == "-1.234,5");
		assert(oconv(-12345,   "MC1,")          == "-1.234,5");

	}

	printl("Test passed");

	return 0;

}
programexit()
