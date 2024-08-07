#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_mvfuncs says 'Hello World!'");

	/////////
	//convert
	/////////

	var x = "abcdefghij";
	assert(convert(x, "abcdefghij", "1234567890") eq "1234567890");
	assert(convert(x, "bcdefghij", "234567890")   eq "a234567890");
	assert(convert(x, "acegi", "12345")           eq "1b2d3f4h5j");
	//	assert(convert(x,"abcdefghi","123456789") eq "123456789j");

	assert(convert("abc", "", "")    eq "abc");
	assert(convert("abc", "abc", "") eq "");
	assert(convert("abc", "", "def") eq "abc");
	assert(convert("abc", "def", "") eq "abc");
	assert(convert("abc", "a", "bc") eq "bbc");

	assert(convert("abc", "a", "")   eq "bc");
	assert(convert("abc", "b", "")   eq "ac");
	assert(convert("abc", "c", "")   eq "ab");
	assert(convert("abc", "ab", "")  eq "c");
	assert(convert("abc", "bc", "")  eq "a");
	assert(convert("abc", "ac", "x") eq "xb");
	assert(convert("abc", "ab", "x") eq "xc");
	assert(convert("abc", "bc", "x") eq "ax");
	assert(convert("abc", "ac", "x") eq "xb");

	assert(convert("abc", "bca", "abc") eq "cab");
	assert(convert("abc", "cba", "abc") eq "cba");

	/////////////
	//textconvert
	/////////////

	//including a hash to prevent numerical comparison which would fail
	var w = "0123456789#";	//western
	var a = "٠١٢٣٤٥٦٧٨٩#";	//arabic

	TRACE(w)
	TRACE(a)
	TRACE(w.oconv("HEX"))
	TRACE(textconvert("0123456789#", w, a).oconv("HEX"))
	TRACE(a.oconv("HEX"))
	TRACE(textconvert("0123456789#", w, a))

	assert(convert("0123456789#", w, a) ne a);	//bytewise conversion of utf8 must not work

	assert(textconvert("0123456789#", w, a) eq "٠١٢٣٤٥٦٧٨٩#");
	assert(textconvert("0123456789#", w, a) eq a);

	assert(textconvert("٠١٢٣٤٥٦٧٨٩#", a, w) eq "0123456789#");
	assert(textconvert("٠١٢٣٤٥٦٧٨٩#", a, w) eq w);

	var sigma = "Σ";
	assert(textconvert("SxyzStevS", "S", "Σ")  eq "ΣxyzΣtevΣ");
	assert(textconvert("ΣxyzΣtevΣ", "Σ", "S")  eq "SxyzStevS");
	assert(textconvert("ΣxyzΣtevΣ", "Σ", "")   eq "xyztev");
	assert(textconvert("ΣxyzΣßtevΣ", "Σ", "S") eq "SxyzSßtevS");
	assert(textconvert("ΣxyzΣßtevΣ", "Σ", "")  eq "xyzßtev");

	//toInt and oconv "MX"

	TRACE(oconv(-3.5, "MX"))
	TRACE(oconv(-3, "MX"))
	TRACE(oconv(-2.5, "MX"))
	TRACE(oconv(-2, "MX"))
	TRACE(oconv(-1.5, "MX"))
	TRACE(oconv(-1, "MX"))
	TRACE(oconv(-.5, "MX"))
	TRACE(oconv(0, "MX"))
	TRACE(oconv(.5, "MX"))
	TRACE(oconv(1, "MX"))
	TRACE(oconv(1.5, "MX"))
	TRACE(oconv(2, "MX"))
	TRACE(oconv(2.5, "MX"))
	TRACE(oconv(3, "MX"))
	TRACE(oconv(3.5, "MX"))

	//MX OCONV converts to integer and represents as HEX
	//1. exodus rounds to nearest integer (tie away from zero)
	//2. pick/arev since rounds to nearest even integer
	// Note: different between EXODUS and Pick/AREV

	assert(oconv(-3.5, "MX") eq "FFFFFFFFFFFFFFFC");
	assert(oconv(-3, "MX")   eq "FFFFFFFFFFFFFFFD");

	//assert(oconv(-2.5,"MX") eq "FFFFFFFFFFFFFFFE");//arev
	assert(oconv(-2.5, "MX") eq "FFFFFFFFFFFFFFFD");  //exodus

	assert(oconv(-2, "MX")   eq "FFFFFFFFFFFFFFFE");
	assert(oconv(-1.5, "MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv(-1, "MX")   eq "FFFFFFFFFFFFFFFF");

	//assert(oconv(-.5,"MX") eq "0");//arev
	assert(oconv(-.5, "MX") eq "FFFFFFFFFFFFFFFF");	 //exodus

	assert(oconv(0, "MX") eq "0");

	//assert(oconv(.5,"MX") eq "0");//arev
	assert(oconv(.5, "MX") eq "1");	 //exodus

	assert(oconv(1, "MX")   eq "1");
	assert(oconv(1.5, "MX") eq "2");
	assert(oconv(2, "MX")   eq "2");

	//assert(oconv(2.5,"MX") eq "2");;//arev
	assert(oconv(2.5, "MX") eq "3");

	assert(oconv(3, "MX")   eq "3");
	assert(oconv(3.5, "MX") eq "4");

	{
		assert(var(' ').oconv("HEX").outputl()      eq "20");
		assert(var(static_cast<unsigned int>(48)).oconv("HEX").outputl() eq "3438");
		assert(oconv(static_cast<unsigned int>(48), "HEX").outputl()     eq "3438");
		assert(oconv(static_cast<unsigned int>(128), "HEX").outputl()    eq "313238");

		assert(var(static_cast<unsigned int>(128)).oconv("MX").outputl()         eq "80");
		assert(var(static_cast<unsigned int>(48)).oconv("MX").outputl()          eq "30");
		assert(("x" ^ var(static_cast<unsigned int>(256)).oconv("MX")).outputl() eq "x100");
		assert(("x" ^ var(static_cast<unsigned int>(255)).oconv("MX")).outputl() eq "xFF");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
