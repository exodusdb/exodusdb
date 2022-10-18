#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {

	var sep = "^";

	assert(sort(var("10^20.1^2^1^20"), sep).outputl("Numbers sort correctly : ") eq "1^2^10^20^20.1");

	assert(sort(var("10^20.1^^2^1^20^"), sep).outputl("Empty fields sort first : ") eq "^^1^2^10^20^20.1");

	assert(sort(var("10a^2a"), sep).outputl("Numbers with suffixes sort alphabetically : ") eq "10a^2a");

	assert(sort(var("a^ccc^bb"), sep).outputl("Simple alphabetic sort : ") eq "a^bb^ccc");

	assert(sort(var("A^a^ccc^BB^bb"), sep).outputl("alphabetic case insensitive sort : ") eq "a^A^bb^BB^ccc");

//	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
//	assert(sort(var("bb^10^aa^2^10a^2a"), sep).outputl("Numbers with suffixes dont get sorted well : ") eq "10a^2^10^2a^aa^bb");
//
//	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
//	assert(sort(var("bb^10^aa^2^a10^a2"), sep).outputl("Numbers before letters : ") eq "2^10^a10^a2^aa^bb");

    printl("GCC: Numbers with suffixes dont get sorted well on gcc but do on clang");
    printl(" numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2");
    printl("Clang: Better");
    var un_sorted1 = "bb^10^aa^2^10a^2a";
    var sorted1 = sort(un_sorted1, sep);
    var gccsorted1 = "10a^2^10^2a^aa^bb";
    var clgsorted1 = "2^10^10a^2a^aa^bb";
    TRACE(sorted1);
    TRACE(gccsorted1);
    TRACE(clgsorted1);
    assert(sorted1 eq gccsorted1 or sorted1 eq clgsorted1);

    var un_sorted2 = "bb^10^aa^2^a10^a2";
    var sorted2 = sort(un_sorted2, sep);
    var gccsorted2 = "10a^2^10^2a^aa^bb";
    var clgsorted2 = "2^10^a10^a2^aa^bb";
    TRACE(sorted2);
    TRACE(gccsorted2);
    TRACE(clgsorted2);
    assert(sorted2 eq gccsorted2 or sorted2 eq clgsorted2);

	//sort via dim
	//////////////

	//sort is actually done by splitting into a dimensioned array of vars and comparing them using the standard var<var operator
	assert(var("10^2").split(sep).sort().join(sep)     eq "2^10");
	assert(var("10a^2a").split(sep).sort().join(sep)   eq "10a^2a");
	assert(var("a^ccc^bb").split(sep).sort().join(sep) eq "a^bb^ccc");

//	//mixed numbers, letters and numbers with letter suffixes doesnt sort well
//	assert(var("bb^10^aa^2^10a^2a").split(sep).sort().join(sep) eq "10a^2^10^2a^aa^bb");
//
//	assert(var("bb^10^aa^2^a10^a2").split(sep).sort().join(sep) eq "2^10^a10^a2^aa^bb");
//
	var dimsorted1 = un_sorted1.split(sep).sort().join(sep);
	assert(dimsorted1 eq gccsorted1 or dimsorted1 eq clgsorted1);

	var dimsorted2 = un_sorted2.split(sep).sort().join(sep);
	assert(dimsorted2 eq gccsorted2 or dimsorted2 eq clgsorted2);

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
