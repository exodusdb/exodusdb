#include <cassert>
#include <sstream>

#include<exodus/program.h>
programinit()

func main() {


	printl(R"V0G0N(
NAME
    demo_precision - Demonstrate Exodus' handling of floating-point numbers for comparison and output

SYNOPSIS
    demo_precision [PRECISION]

DESCRIPTION
    The demo_precision program illustrates how the Exodus library manages IEEE 754 8-byte std::double floating-point numbers for automatic string output and comparisons. This applies only to implicit conversions and does not cover explicit conversions via format(), oconv(), or round().

    For most general numerical applications (e.g., business or everyday calculations), Exodus' default settings are sufficient and manual adjustment of precision is not required. However, scientific or engineering applications may need finer control over very large or very small numbers.

    Floating-point arithmetic (e.g., 0.1 + 0.2) can produce imprecise results like 0.30000000000000004 due to binary representation limits. Exodus addresses this as follows:

    OUTPUT FORMATTING
        - All numbers are output with a fixed precision of 12 significant digits using std::chars_format::general.
        - This ensures clean output (e.g., "0.3" instead of "0.30000000000000004").
        - Numbers exceeding 1e+12 or smaller than 1e-12 use scientific notation:
            1234567890123.456             -> "1.23456789012e+12"
            0.000000000001234567890123456 -> "1.23456789012e-12"
        - Numbers numbers within the same limits do not use scientific notation:
            123456789012.3456             -> "123456789012"
            0.00000000001234567890123456  -> "0.0000000000123456789012"
		- The 13th digit, if present, is used to round the 12 digit away from zero.
            123456789012.3456             -> "123456789012"
            123456789012.56               -> "123456789013"
    PRECISION SETTING
        - Exodus uses a "precision" value to define an epsilon (10^-precision) for:
          - Comparing floating-point numbers (e.g., two numbers are equal if their difference is less than epsilon).
          - Suppressing output of numbers smaller than epsilon (treated as zero).
        - The default Exodus precision is 4, yielding an epsilon of 0.0001 (10^-4), suitable for most common cases.
        - With default Exodus precision, numbers between 0.0001 and 99,999,999.9999 (±) are handled accurately without explicit formatting.
        - The setprecision(newprecision) function sets this precision globally for the current thread, typically needed only for specialized use cases. The getprecision() function gets this precision.

OPTIONS
    PRECISION
        An optional integer between -307 and 308 defining the precision (number of decimal places in epsilon). Default is 4.
        - Example: PRECISION=12 sets epsilon to 0.000000000001 (10^-12), allowing comparison and output of very small numbers:
            TRACE(var(0.0000000000123456789012)) -> "0.0000000000123456789012"
        - Higher precision suits scientific use but may affect handling of larger numbers.

EXAMPLES
    demo_precision
        Runs with default precision of 4 (epsilon = 0.0001).
    demo_precision 12
        Runs with precision of 12 (epsilon = 0.000000000001).

NOTES
    - Unlike C++'s std::setprecision, Exodus' "precision" does not affect output digits (fixed at 12) but controls epsilon for comparisons and suppression.
    - Numbers smaller than epsilon are output as "0" unless explicitly formatted.
    - Use precision(newprecision) to programmatically adjust precision within a thread, though this is rarely needed for typical applications.

SEE ALSO
    format(), oconv(), round(), getprecision(), setprecision()
)V0G0N");

	auto test = [](std::string sd1, std::string s1, std::string expected_vs) {

		auto d1d = std::stod(sd1);

		var v1 = sd1;
		v1 += 0;

		let ok = v1.squote() eq var(expected_vs).squote() ? "ok" : "diff";

		std::cout <<
				std::setw(45) << (DQ ^ sd1 ^ DQ) <<
				std::setw(15) << d1d <<
				std::setw(32) << v1 <<
				std::setw(25) << expected_vs <<
				" " << ok << std::endl;

		std::stringstream ss1;
		ss1 << d1d;
		assert(ss1.str() eq s1);

		if (getprecision() == 4)
			assert(v1.squote() eq var(expected_vs).squote());

	}; // lamda function

	// Option to change precision
	let word2 = COMMAND.f(2);
//	if (word2 ne "" and isnum(word2)) {
	if (not word2.empty()) {
		let new_precision = setprecision(word2);
		if (new_precision != word2)
			abort("Could not set precision to " ^ word2);
	}
	printl(COMMAND.f(1), "Testing precision.", "EXO_PRECISION =", getprecision());

	std::cout <<
			std::setw(45) << "std::string" <<
			std::setw(15) << "cout<<double" <<
			std::setw(32) << ("cout<<var @ precision " ^ var(getprecision())) <<
			std::setw(25) << "Expected @ precision 4" << std::endl;

	test( ".00000000000000000000", "0"				, "0");

//	test( ".00000000000000000090", "9e-19"			, "0");
//	test( ".00000000000000000890", "8.9e-18"		, "0");
//	test( ".00000000000000007890", "7.89e-17"		, "0");
//	test( ".00000000000000067890", "6.789e-16"		, "0");
//	test( ".00000000000000567890", "5.6789e-15"		, "0");
//	test( ".00000000000004567890", "4.56789e-14"	, "0");
//	test( ".00000000000034567890", "3.45679e-13"	, "0");
//	test( ".00000000000234567890", "2.34568e-12"	, "0");
//	test( ".00000000001234567890", "1.23457e-11"	, "0");
//	test( ".00000000901234567890", "9.01235e-09"	, "0");
//	test( ".00000008901234567890", "8.90123e-08"	, "0");
//	test( ".00000078901234567890", "7.89012e-07"	, "0");
//	test( ".00000678901234567890", "6.78901e-06"	, "0");
//	test( ".00005678901234567890", "5.6789e-05"		, "0");
//	test( ".00045678901234567890", "0.000456789"	, "0.000456789012346");
//	test( ".00345678901234567890", "0.00345679"		, "0.00345678901235");
//	test( ".02345678901234567890", "0.0234568"		, "0.0234567890123");

	test( ".0000000000000000012345678901234567890", "1.23457e-18"	, "0");
	test( ".000000000000000012345678901234567890", "1.23457e-17"	, "0");
	test( ".00000000000000012345678901234567890", "1.23457e-16"		, "0");
	test( ".0000000000000012345678901234567890", "1.23457e-15"		, "0");
	test( ".000000000000012345678901234567890", "1.23457e-14"		, "0");
	test( ".00000000000012345678901234567890", "1.23457e-13"		, "0");
	test( ".0000000000012345678901234567890", "1.23457e-12"			, "0");
	test( ".000000000012345678901234567890", "1.23457e-11"			, "0");
	test( ".00000000012345678901234567890", "1.23457e-10"			, "0");
	test( ".0000000012345678901234567890", "1.23457e-09"			, "0");
	test( ".000000012345678901234567890", "1.23457e-08"				, "0");
	test( ".00000012345678901234567890", "1.23457e-07"				, "0");
	test( ".0000012345678901234567890", "1.23457e-06"				, "0");
	test( ".000012345678901234567890", "1.23457e-05"				, "0");
	test( ".00012345678901234567890", "0.000123457"	, "0.000123456789012");
	test( ".0012345678901234567890", "0.00123457"	, "0.00123456789012");
	test( ".012345678901234567890", "0.0123457"		, "0.0123456789012");
	test( ".12345678901234567890", "0.123457"		, "0.123456789012");

	test( "1.2345678901234567890", "1.23457"		, "1.23456789012");
	test( "12.345678901234567890", "12.3457"		, "12.3456789012");
	test( "123.45678901234567890", "123.457"		, "123.456789012");
	test( "1234.5678901234567890", "1234.57"		, "1234.56789012");
	test( "12345.678901234567890", "12345.7"		, "12345.6789012");
	test( "123456.78901234567890", "123457"			, "123456.789012");
	test( "1234567.8901234567890", "1.23457e+06"	, "1234567.89012");
	test( "12345678.901234567890", "1.23457e+07"	, "12345678.9012");
	test( "123456789.01234567890", "1.23457e+08"	, "123456789.012");
	test( "1234567890.1234567890", "1.23457e+09"	, "1234567890.12");
	test( "12345678901.234567890", "1.23457e+10"	, "12345678901.2");
	test( "123456789012.34567890", "1.23457e+11"	, "123456789012");
//	test( "1234567890123.4567890", "1.23457e+12"	, "1234567890120");
//	test( "12345678901234.567890", "1.23457e+13"	, "12345678901200");
//	test( "123456789012345.67890", "1.23457e+14"	, "123456789012000");
//	test( "1234567890123456.7890", "1.23457e+15"	, "1234567890120000");
	test( "1234567890123.4567890", "1.23457e+12"	, "1.23456789012e+12");
	test( "12345678901234.567890", "1.23457e+13"	, "1.23456789012e+13");
	test( "123456789012345.67890", "1.23457e+14"	, "1.23456789012e+14");
	test( "1234567890123456.7890", "1.23457e+15"	, "1.23456789012e+15");
	test( "12345678901234567.890", "1.23457e+16"	, "1.23456789012e+16");
	test( "123456789012345678.90", "1.23457e+17"	, "1.23456789012e+17");
	test( "1234567890123456789.0", "1.23457e+18"	, "1.23456789012e+18");
	test( "12345678901234567890.", "1.23457e+19"	, "1.23456789012e+19");

	printl();
	test( "999999999999.00000000", "1e+12"			, "999999999999");
	test( "999999999999.49993896", "1e+12"			, "999999999999");
	test( "999999999999.49993897", "1e+12"			, "1e+12");

	TRACE(var(1) / var(3))       // "0.333333333333"
	TRACE(var(2) / var(3))       // "0.666666666667"

	printl("Well known problem is avoided.");
	TRACE(0.1_var + 0.2_var)     // "0.3"

	printl("13th digit is used to round the 12th digit away from zero.");
	TRACE((123456789012.56_var)) // "123456789013"

	TRACE(setprecision(12)) // "12"
	TRACE(var(0.000000000012345678901234567890))    // "0.0000000000123456789012"

	return 0;
}

programexit()
