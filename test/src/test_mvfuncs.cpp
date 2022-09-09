#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_mvfuncs says 'Hello World!'");

	/////////
	//convert
	/////////

	var x="abcdefghij";
	assert(convert(x,"abcdefghij","1234567890") == "1234567890");
	assert(convert(x,"bcdefghij","234567890") == "a234567890");
	assert(convert(x,"acegi","12345") == "1b2d3f4h5j");
//	assert(convert(x,"abcdefghi","123456789") == "123456789j");

	assert(convert("abc","","") == "abc");
	assert(convert("abc","abc","") == "");
	assert(convert("abc","","def") == "abc");
	assert(convert("abc","def","") == "abc");
	assert(convert("abc","a","bc") == "bbc");

	assert(convert("abc","a","") == "bc");
	assert(convert("abc","b","") == "ac");
	assert(convert("abc","c","") == "ab");
	assert(convert("abc","ab","") == "c");
	assert(convert("abc","bc","") == "a");
	assert(convert("abc","ac","x") == "xb");
	assert(convert("abc","ab","x") == "xc");
	assert(convert("abc","bc","x") == "ax");
	assert(convert("abc","ac","x") == "xb");

	assert(convert("abc","bca","abc") == "cab");
	assert(convert("abc","cba","abc") == "cba");

	/////////////
	//textconvert
	/////////////

	//including a hash to prevent numerical comparison which would fail
	var w = "0123456789#";//western
    var a = "٠١٢٣٤٥٦٧٨٩#";//arabic

	TRACE(w);
	TRACE(a);
	TRACE(w.oconv("HEX"));
    TRACE(textconvert("0123456789#",w,a).oconv("HEX"));
	TRACE(a.oconv("HEX"));
    TRACE(textconvert("0123456789#",w,a));

    assert(convert("0123456789#",w,a) != a);//bytewise conversion of utf8 must not work

    assert(textconvert("0123456789#",w,a) == "٠١٢٣٤٥٦٧٨٩#");
    assert(textconvert("0123456789#",w,a) == a);

    assert(textconvert("٠١٢٣٤٥٦٧٨٩#",a,w) == "0123456789#");
    assert(textconvert("٠١٢٣٤٥٦٧٨٩#",a,w) == w);

	var sigma = "Σ";
	assert(textconvert("SxyzStevS","S","Σ") == "ΣxyzΣtevΣ");
	assert(textconvert("ΣxyzΣtevΣ","Σ","S") == "SxyzStevS");
	assert(textconvert("ΣxyzΣtevΣ","Σ","") == "xyztev");
	assert(textconvert("ΣxyzΣßtevΣ","Σ","S") == "SxyzSßtevS");
	assert(textconvert("ΣxyzΣßtevΣ","Σ","") == "xyzßtev");

	//toInt and oconv "MX"

    TRACE(oconv(-3.5,"MX"));
    TRACE(oconv(-3,"MX"));
    TRACE(oconv(-2.5,"MX"));
    TRACE(oconv(-2,"MX"));
    TRACE(oconv(-1.5,"MX"));
    TRACE(oconv(-1,"MX"));
    TRACE(oconv(-.5,"MX"));
    TRACE(oconv(0,"MX"));
    TRACE(oconv(.5,"MX"));
    TRACE(oconv(1,"MX"));
    TRACE(oconv(1.5,"MX"));
    TRACE(oconv(2,"MX"));
    TRACE(oconv(2.5,"MX"));
    TRACE(oconv(3,"MX"));
    TRACE(oconv(3.5,"MX"));

    //MX OCONV converts to integer and represents as HEX
    //1. exodus rounds to nearest integer (tie away from zero)
    //2. pick/arev since rounds to nearest even integer
    // Note: different between EXODUS and Pick/AREV
    
    assert(oconv(-3.5,"MX") == "FFFFFFFFFFFFFFFC");
    assert(oconv(-3,"MX") == "FFFFFFFFFFFFFFFD");

    //assert(oconv(-2.5,"MX") == "FFFFFFFFFFFFFFFE");//arev
    assert(oconv(-2.5,"MX") == "FFFFFFFFFFFFFFFD");//exodus

    assert(oconv(-2,"MX") == "FFFFFFFFFFFFFFFE");
    assert(oconv(-1.5,"MX") == "FFFFFFFFFFFFFFFE");
    assert(oconv(-1,"MX") == "FFFFFFFFFFFFFFFF");

    //assert(oconv(-.5,"MX") == "0");//arev
    assert(oconv(-.5,"MX") == "FFFFFFFFFFFFFFFF");//exodus

    assert(oconv(0,"MX") == "0");

    //assert(oconv(.5,"MX") == "0");//arev
    assert(oconv(.5,"MX") == "1");//exodus

    assert(oconv(1,"MX") == "1");
    assert(oconv(1.5,"MX") == "2");
    assert(oconv(2,"MX") == "2");

    //assert(oconv(2.5,"MX") == "2");;//arev
    assert(oconv(2.5,"MX") == "3");

    assert(oconv(3,"MX") == "3");
    assert(oconv(3.5,"MX") == "4");

	{
		assert(var(' ').oconv("HEX").outputl() eq "20");
		assert(var(uint(48)).oconv("HEX").outputl() eq "3438");
		assert(oconv(uint(48), "HEX").outputl() eq "3438");
		assert(oconv(uint(128), "HEX").outputl() eq "313238");

		assert(var(uint(128)).oconv("MX").outputl() eq "80");
		assert(var(uint(48)).oconv("MX").outputl() eq "30");
		assert(("x" ^ var(uint(256)).oconv("MX")).outputl() eq "x100");
		assert(("x" ^ var(uint(255)).oconv("MX")).outputl() eq "xFF");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

