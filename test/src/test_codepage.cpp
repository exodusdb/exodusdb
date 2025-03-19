#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {


	{
		let tempfile = "t_allo.txt";
		let allo_utf8 =  "Алло";
		let allo_cp   = "\xb0" "\xdb" "\xdb" "\xde";
		let cyrillic_cp = "ISO-8859-5";
		assert(allo_utf8.len() == 8);
		assert(allo_cp.len() == 4);

		// Check manual code page conversion round trip
		// utf8 to codepage
		assert(allo_utf8.to_codepage(cyrillic_cp) == allo_cp);
		// codepage to utf8
		assert(allo_cp.from_codepage(cyrillic_cp) == allo_utf8);

		// Check oswrite internal utf8 converting to external codepage
		assert(oswrite(allo_utf8 on tempfile, cyrillic_cp));
		assert(osfile(tempfile).f(1) == 4);

		// Check can read from codepage file unconverted
		var temp;
		//read in from binary cyrillic codepage text as is
		assert(osread(temp from tempfile));
		assert(temp == allo_cp);

		// Check can read from codepage and convert to utf8
		// Read in from binary cyrillic codepage text converting to utf8
		assert(osread(temp from tempfile, cyrillic_cp));
		assert(temp == allo_utf8);
	}

	//check conversion from and to codepages
	test_codepage("CP437", "latin");  //win3
	test_codepage("CP850", "latin");  //win10
	test_codepage("CP737", "greek");
	//test_codepage("CP720","arabic");//doesnt exist so use CP1256
	test_codepage("CP1256", "arabic");	//720
	//test_codepage("CP1252","latin");
	//test_codepage("CP1253","greek");//737

	//check osread/write with and without conversion to codepages

	//cyrillic ISO-8859-5 cp bytes
	var cp_allo1{"\xB0\xDB\xDB\xDE"};

	//output binary unconverted
	assert(oswrite(cp_allo1, "t_cp_allo.txt"));
	assert(cp_allo1.oconv("HEX").outputl("cp_allo1=") eq "B0DBDBDE");

	//hexdump -C t_cp_allo.txt
	//00000000  b0 db db de
	//00000004

	//check that we can read binary back in unconverted
	var cp_allo2 = osread("t_cp_allo.txt");
	assert(cp_allo2.oconv("HEX").outputl("cp_allo2=") eq "B0DBDBDE");

	//read in from binary cyrillic codepage text converting to utf8
	var utf8_allo3;
	assert(utf8_allo3.osread("t_cp_allo.txt", "ISO-8859-5"));
	assert(utf8_allo3.len()        eq 8);
	assert(utf8_allo3.oconv("HEX") eq "D090D0BBD0BBD0BE");
	assert(utf8_allo3              eq "Алло");

	//output utf8 convering to ISO-8859-5
	assert(oswrite(utf8_allo3, "t_cp_allo4.txt", "ISO-8859-5"));
	/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump cp_allo4.txt -C
00000000  b0 db db de                                       |....|
00000004
*/
	//read back in binary to check that out output did convert from utf to cyrillic
	var cp_allo5;
	assert(osread(cp_allo5, "t_cp_allo4.txt"));
	assert(cp_allo5 eq cp_allo1);

	/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump t_utf8_allo4.txt -C
00000000  d0 90 d0 bb d0 bb d0 be                           |........|
00000008
*/

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

func test_codepage(in codepage, in lang) {
	printl("---------- " ^ lang ^ " " ^ codepage ^ " ----------");

	var v256 = "";
	for (int ii = 0; ii le 255; ++ii)
		v256 ^= chr(ii);
	assert(oswrite(v256, "t_codep.bin"));
	assert(osfile("t_codep.bin").f(1) eq 256);

	//convert to utf8
	var as_utf8a = v256.from_codepage(codepage);
	assert(as_utf8a ne v256);
	assert(as_utf8a.len() gt v256.len());

	//convert back to codepage
	var as_cp = as_utf8a.to_codepage(codepage);
	printl(as_cp.len());
	//assert(as_cp.len()==256);

	//convert to utf8 again
	var as_utf8b = as_cp.from_codepage(codepage);

	//oswrite(as_utf8a, "t_utf8a." ^ lang);
	//oswrite(as_cp,   "t_codep." ^ lang);
	//oswrite(as_utf8b,"t_utf8b." ^ lang);

	//check loop back
	//(only if loop back does produced 256 bytes)
	printl(as_utf8a.b(32));
	printl("round trip " ^ var(as_cp eq v256));
	if (as_cp.len() eq 256)
		assert(as_cp                    eq v256);

	//check double trip
	printl("double trip " ^ var(as_utf8a eq as_utf8b));
	printl(as_utf8b.b(32));
	assert(as_utf8a                      eq as_utf8b);

	return 0;
}

programexit()
