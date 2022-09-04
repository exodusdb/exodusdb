#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

	var sep = "^";

	assert(sort(var("10^20.1^2^1^20"), sep).outputl("Numbers sort correctly : ") == "1^2^10^20^20.1");

	assert(sort(var("10^20.1^^2^1^20^"), sep).outputl("Empty fields sort first : ") == "^^1^2^10^20^20.1");

	assert(sort(var("10a^2a"), sep).outputl("Numbers with suffixes sort alphabetically : ") == "10a^2a");

	assert(sort(var("a^ccc^bb"), sep).outputl("Simple alphabetic sort : ") == "a^bb^ccc");

	assert(sort(var("A^a^ccc^BB^bb"), sep).outputl("alphabetic case insensitive sort : ") == "a^A^bb^BB^ccc");

	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
	assert(sort(var("bb^10^aa^2^10a^2a"), sep).outputl("Numbers with suffixes dont get sorted well : ") == "10a^2^10^2a^aa^bb");

	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
	assert(sort(var("bb^10^aa^2^a10^a2"), sep).outputl("Numbers before letters : ") == "2^10^a10^a2^aa^bb");


	//sort via dim
	//////////////

	//sort is actually done by splitting into a dimensioned array of vars and comparing them using the standard var<var operator
	assert(var("10^2").split(sep).sort().join(sep) == "2^10");
	assert(var("10a^2a").split(sep).sort().join(sep) == "10a^2a");
	assert(var("a^ccc^bb").split(sep).sort().join(sep) == "a^bb^ccc");

	//mixed numbers, letters and numbers with letter suffixes doesnt sort well
	assert(var("bb^10^aa^2^10a^2a").split(sep).sort().join(sep) == "10a^2^10^2a^aa^bb");

	assert(var("bb^10^aa^2^a10^a2").split(sep).sort().join(sep) == "2^10^a10^a2^aa^bb");

	printl("Test passed");

	return 0;
}

programexit()
