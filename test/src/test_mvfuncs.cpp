#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("test_mvfuncs says 'Hello World!'");

	/////////
	//convert
	/////////

	var x = "abcdefghij";
	assert(convert(x, "abcdefghij", "1234567890") .errputl() eq "1234567890");
	assert(convert(x, "bcdefghij", "234567890")   .errputl() eq "a234567890");
	assert(convert(x, "acegi", "12345")           .errputl() eq "1b2d3f4h5j");
	//	assert(convert(x,"abcdefghi","123456789") .errputl() eq "123456789j");

	assert(convert("abc", "", "")    .errputl() eq "abc");
	assert(convert("abc", "abc", "") .errputl() eq "");
	assert(convert("abc", "", "def") .errputl() eq "abc");
	assert(convert("abc", "def", "") .errputl() eq "abc");
	assert(convert("abc", "a", "bc") .errputl() eq "bbc");

	assert(convert("abc", "a", "")   .errputl() eq "bc");
	assert(convert("abc", "b", "")   .errputl() eq "ac");
	assert(convert("abc", "c", "")   .errputl() eq "ab");
	assert(convert("abc", "ab", "")  .errputl() eq "c");
	assert(convert("abc", "bc", "")  .errputl() eq "a");
	assert(convert("abc", "ac", "x") .errputl() eq "xb");
	assert(convert("abc", "ab", "x") .errputl() eq "xc");
	assert(convert("abc", "bc", "x") .errputl() eq "ax");
	assert(convert("abc", "ac", "x") .errputl() eq "xb");

	assert(convert("abc", "bca", "abc") .errputl() eq "cab");
	assert(convert("abc", "cba", "abc") .errputl() eq "cba");

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

	assert(textconvert("0123456789#", w, a) .errputl() eq "٠١٢٣٤٥٦٧٨٩#");
	assert(textconvert("0123456789#", w, a) eq a);

	assert(textconvert("٠١٢٣٤٥٦٧٨٩#", a, w) .errputl() eq "0123456789#");
	assert(textconvert("٠١٢٣٤٥٦٧٨٩#", a, w) eq w);

	var sigma = "Σ";
	assert(textconvert("SxyzStevS", "S", "Σ")  .errputl() eq "ΣxyzΣtevΣ");
	assert(textconvert("ΣxyzΣtevΣ", "Σ", "S")  .errputl() eq "SxyzStevS");
	assert(textconvert("ΣxyzΣtevΣ", "Σ", "")   .errputl() eq "xyztev");
	assert(textconvert("ΣxyzΣßtevΣ", "Σ", "S") .errputl() eq "SxyzSßtevS");
	assert(textconvert("ΣxyzΣßtevΣ", "Σ", "")  .errputl() eq "xyzßtev");

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

	assert(oconv(-3.5, "MX") .errputl() eq "FFFFFFFFFFFFFFFC");
	assert(oconv(-3, "MX")   .errputl() eq "FFFFFFFFFFFFFFFD");

	//assert(oconv(-2.5,"MX") .errputl() eq "FFFFFFFFFFFFFFFE");//arev
	assert(oconv(-2.5, "MX") .errputl() eq "FFFFFFFFFFFFFFFD");  //exodus

	assert(oconv(-2, "MX")   .errputl() eq "FFFFFFFFFFFFFFFE");
	assert(oconv(-1.5, "MX") .errputl() eq "FFFFFFFFFFFFFFFE");
	assert(oconv(-1, "MX")   .errputl() eq "FFFFFFFFFFFFFFFF");

	//assert(oconv(-.5,"MX") .errputl() eq "0");//arev
	assert(oconv(-.5, "MX") .errputl() eq "FFFFFFFFFFFFFFFF");	 //exodus

	assert(oconv(0, "MX") .errputl() eq "0");

	//assert(oconv(.5,"MX") .errputl() eq "0");//arev
	assert(oconv(.5, "MX") .errputl() eq "1");	 //exodus

	assert(oconv(1, "MX")   .errputl() eq "1");
	assert(oconv(1.5, "MX") .errputl() eq "2");
	assert(oconv(2, "MX")   .errputl() eq "2");

	//assert(oconv(2.5,"MX") .errputl() eq "2");;//arev
	assert(oconv(2.5, "MX") .errputl() eq "3");

	assert(oconv(3, "MX")   .errputl() eq "3");
	assert(oconv(3.5, "MX") .errputl() eq "4");

	{
		// MX iconv

		// Check iconv "MX" works
		// a) on chars 0-9a-fA-F
		// b) NOT on any other chars
		// iconv handles multiple fields, values etc. so general field marks are not tested
		for (var chn : range(1, 255)) {
			var ch = chr(chn);
			if (not var(_ALL_FMS).contains(ch)) {
				errput(chn, ".\t", oconv(ch, "HEX"), "\t", iconv(ch, "MX").squote(), "\t");
				if ("0123456789abcdefABCDEF"_var.contains(ch))
					assert(not iconv(ch, "MX").empty());
				else
					assert(iconv(ch, "MX").empty());
			}
		}
		printl();

		printl("\nCheck round trip up to 64Kib + 1");
		for (var chn : range(0, 65536)) {
			if (chn < ST_ or chn >= RM_) {
				var hex = oconv(chn, "MX");
	//			logputl(chn, "\t", hex.squote(), "\t", hex.iconv("MX").squote());
				assert(chn== iconv(oconv(chn, "MX"), "MX"));
			}
		}

	}

	{
		assert(oconv(15, "MX") .errputl() eq "F");
//		assert(oconv(15, "MX1") .errputl() eq "F");
		assert(oconv(15, "MX2") .errputl() eq "0F");
//		assert(oconv(15, "MX3") .errputl() eq "00F");
		assert(oconv(15, "MX4") .errputl() eq "000F");
//		assert(oconv(15, "MX5") .errputl() eq "0000F");
		assert(oconv(15, "MX6") .errputl() eq "00000F");
//		assert(oconv(15, "MX7") .errputl() eq "000000F");
		assert(oconv(15, "MX8") .errputl() eq "0000000F");
//		assert(oconv(15, "MX9") .errputl() eq "00000000F");
		assert(oconv(15, "MXA") .errputl() eq "000000000F");
//		assert(oconv(15, "MXB") .errputl() eq "0000000000F");
		assert(oconv(15, "MXC") .errputl() eq "00000000000F");
//		assert(oconv(15, "MXD") .errputl() eq "000000000000F");
		assert(oconv(15, "MXE") .errputl() eq "0000000000000F");
//		assert(oconv(15, "MXF") .errputl() eq "00000000000000F");
		assert(oconv(15, "MXG") .errputl() eq "000000000000000F");
	}

	{
		assert(iconv("F", "MX") .errputl() eq "15");
		assert(iconv("FF", "MX") .errputl() eq "255");
		assert(iconv("FFF", "MX") .errputl() eq "4095");
		assert(iconv("FFFF", "MX") .errputl() eq "65535");

		assert(iconv("FFFFF", "MX") .errputl() eq "1048575");
		assert(iconv("FFFFFF", "MX") .errputl() eq "16777215");
		assert(iconv("FFFFFFF", "MX") .errputl() eq "268435455");
		assert(iconv("FFFFFFFF", "MX") .errputl() eq "4294967295");

		assert(iconv("FFFFFFFFF", "MX") .errputl() eq "68719476735");
		assert(iconv("FFFFFFFFFF", "MX") .errputl() eq "1099511627775");
		assert(iconv("FFFFFFFFFFF", "MX") .errputl() eq "17592186044415");
		assert(iconv("FFFFFFFFFFFF", "MX") .errputl() eq "281474976710655");

		assert(iconv("FFFFFFFFFFFFF", "MX") .errputl() eq "4503599627370495");
		assert(iconv("FFFFFFFFFFFFFF", "MX") .errputl() eq "72057594037927935");
		assert(iconv("FFFFFFFFFFFFFFF", "MX") .errputl() eq "1152921504606846975");
		assert(iconv("FFFFFFFFFFFFFFFF", "MX") .errputl() eq "-1");

		assert(iconv("7FFFFFFFFFFFFFFF", "MX") .errputl() eq "9223372036854775807"); // max positive var integer


	}

	{
		assert(oconv(15, "MX") .errputl() eq "F");
		assert(oconv(255, "MX") .errputl() eq "FF");
		assert(oconv(4095, "MX") .errputl() eq "FFF");
		assert(oconv(65535, "MX") .errputl() eq "FFFF");

		assert(oconv(1048575, "MX") .errputl() eq "FFFFF");
		assert(oconv(16777215, "MX") .errputl() eq "FFFFFF");
		assert(oconv(268435455, "MX") .errputl() eq "FFFFFFF");
		assert(oconv(4294967295, "MX") .errputl() eq "FFFFFFFF");

		assert(oconv(68719476735, "MX") .errputl() eq "FFFFFFFFF");
		assert(oconv(1099511627775, "MX") .errputl() eq "FFFFFFFFFF");
		assert(oconv(17592186044415, "MX") .errputl() eq "FFFFFFFFFFF");
		assert(oconv(281474976710655, "MX") .errputl() eq "FFFFFFFFFFFF");

		assert(oconv(4503599627370495, "MX") .errputl() eq "FFFFFFFFFFFFF");
		assert(oconv(72057594037927935, "MX") .errputl() eq "FFFFFFFFFFFFFF");
		assert(oconv(1152921504606846975, "MX") .errputl() eq "FFFFFFFFFFFFFFF");
		assert(oconv(-1, "MX") .errputl() eq "FFFFFFFFFFFFFFFF");

		assert(oconv(9223372036854775807, "MX") .errputl() eq "7FFFFFFFFFFFFFFF"); // max positive var integer
	}

	{
		assert(var(' ').oconv("HEX").outputl()      .errputl() eq "20");
		assert(var(static_cast<unsigned int>(48)).oconv("HEX").outputl() .errputl() eq "3438");
		assert(oconv(static_cast<unsigned int>(48), "HEX").outputl()     .errputl() eq "3438");
		assert(oconv(static_cast<unsigned int>(128), "HEX").outputl()    .errputl() eq "313238");

		assert(var(static_cast<unsigned int>(128)).oconv("MX").outputl()         .errputl() eq "80");
		assert(var(static_cast<unsigned int>(48)).oconv("MX").outputl()          .errputl() eq "30");
		assert(("x" ^ var(static_cast<unsigned int>(256)).oconv("MX")).outputl() .errputl() eq "x100");
		assert(("x" ^ var(static_cast<unsigned int>(255)).oconv("MX")).outputl() .errputl() eq "xFF");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
