#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#if EXO_MODULE
	import std;
//#	include <vector>
#else
#	include <algorithm>
#endif

#include <exodus/program.h>
programinit()

	// Sadly "Non-static data members can only be initialized with member initializer list or with a default member initializer."
	// dim ww(100);
	// dim registerx(10);
	//
	// Compiles but initializes to 0
	//
	dim ww{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
dim registerx{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//
// Solution is to use default constructor and dimension in a default constructor of this common array
//
//dim ww;
//dim registerx;
//
// Default constructor to properly dimension the dim arrays
//ExodusProgram() {
//	ww.redim(100);
//	registerx.redim(10);
//}

function main() {

	//dim array

	{
		// Empty array with no rows is allowed. (Defaults to one column)
		dim d0(0);
		assert(d0.join() eq "");
	}

	{
		// Empty array is allowed with zero rows but must have cols
		dim d0(0, 1);
		assert(d0.join() eq "");

		d0.redim(2, 2);
		d0 = "x";
		assert(d0.join().outputl() eq "x^x^x^x"_var);

		// Redim to empty array is allowed
		d0.redim(0, 0);

		// It can no longer be used
		try {
			assert(d0.join() eq "");
			assert(false);
		}
		catch (DimUndimensioned e) {};
	}
	{
		// wll not compile since assignment function returns void
		//dim d0 = dim() = "";
	}

	{
		// Try to constuct dim from a temporary but
		// Doesnt work due to excellent RVO

		dim d1 = dim(2, 3);
		dim d2(dim(2, 3));

		// var::split returns a dim but RVO results in it being constructed in place
		dim d3 = var("aaaa").split();
		dim d4(var("asdasd").split());

		dim d5 = split("11^22"_var);
		assert(d5.join() eq "11" _FM "22");

		dim d6(split("aa^bb"_var));
		assert(d6.join() eq "aa" _FM "bb");

//		std::vector<dim> vofdims{dim(0), dim(0)};
//		std::vector<dim> vofdims{dim(1), dim(1)};
	}

	{
		dim d1(0);
		dim d2(0, 0);
		dim d3(0, 1);
		dim d4(0, 2);
		d1 = "x";
		//d2 = "x";
		d3 = "x";
		d4 = "x";
		assert(join(d1)          eq "");
//		assert(join(d2)          eq "");
		assert(join(d3)          eq "");
		assert(join(d4)          eq "");
	}
	{
		dim d1(0);
		dim d2(0, 0);
		dim d3(1, 0);
		dim d4(2, 0);

		d1 = "x";
		try{d2 = "x";assert(false);} catch (DimUndimensioned e) {};
		try{d3 = "x";assert(false);} catch (DimUndimensioned e) {};
		try{d4 = "x";assert(false);} catch (DimUndimensioned e) {};

		assert(join(d1) == "");
		try{join(d2) = "x";assert(false);} catch (DimUndimensioned e) {};
		try{join(d3) = "x";assert(false);} catch (DimUndimensioned e) {};
		try{join(d4) = "x";assert(false);} catch (DimUndimensioned e) {};
	}
	{
		//dim_iter
		dim d1;
		d1 = split("aa" _FM "bb");
		var count = 0;
		for (var v1 : d1) {
			//for (var& v1 : d1) {
			count++;
			assert(count ne 1 or v1 eq "aa");
			assert(count ne 2 or v1 eq "bb");
			if (count eq 2)
				v1 = "cc";	//will have no effect on d1
		}
		TRACE(d1.join())

		assert(d1.join().outputl() eq "aa" _FM "bb");	//default separator is FM
		assert(d1.join("").outputl() eq
			   "aa"
			   "bb");								 //sep can be ""
		assert(d1.join("Δ").outputl() eq "aaΔbb");		 //sep can be multibyte UTF8
		assert(d1.join("XYZ").outputl() eq "aaXYZbb");	 //sep can be multichar
	}

	{
		printl("Create a dim from a split var");
		dim d1 = "aa^bb^cc"_var.split();
	}

	{
		dim d1(3, 2);
		d1 = "x";
		TRACE(d1.join())
		assert(d1.at(1, 1) eq "x");
		assert(d1.at(2, 1) eq "x");
		assert(d1.at(3, 1) eq "x");
		assert(d1.at(1, 2) eq "x");
		assert(d1.at(2, 2) eq "x");
		assert(d1.at(3, 2) eq "x");

		const dim d2 = {1, 2, 3, 4, 5, 6};
		assert(d2.at(6, 1) eq 6);

		const dim d3 = {1, 2, 3, 4, 5, 6};
		assert(d2.at(6, 1) eq 6);

		d1.at(0, 0) = "qwe";
		assert(d1.at(0, 0) eq "qwe");

		//const dim d4 = {{1,2},{3,4},{5,6}};
		//assert(d2.at(6,1) eq 6);
	}
	{
		// Cannot dim(0,0)
		dim d1(1, 1);

		// But can redim(0) to clear all rows (but ncols remains)
		d1.redim(0);
		assert(d1.join()         eq "");

		// But can redim(0, 0) to clear all data
		d1.redim(0, 0);

		// but no longer use it
		try{assert(d1.join()         eq ""); assert(false);} catch(DimUndimensioned e) {};
	}
	{
		dim d1;
		d1 = split("aa" _FM "bb");
		var count = 0;
		//for (var v1 : d1) {
		for (var& v1 : d1) {
			count++;
			assert(count ne 1 or v1 eq "aa");
			assert(count ne 2 or v1 eq "bb");
			if (count eq 2)
				v1 = "cc";	//will update d1
		}
		TRACE(d1.join())
		assert(d1.join() eq "aa" _FM "cc");	 //d1 updated
	}

	{

		// Construct from list
		dim d1 = {1, 2, 3};
		TRACE(d1.join())
		TRACE(d1.join().outputl())
		assert(d1.join() eq "1^2^3"_var);

		// Copy construction (from lvalue)
		dim d2 = d1;
		assert(d2.join() eq "1^2^3"_var);

		// Move construction (from rvalue)
		dim d3 = "1^2^3^4"_var.split();
		assert(d3.join() eq "1^2^3^4"_var);

		// Move assign (from rvalue)
		d3 = "1^2^3^4^5"_var.split();
		assert(d3.join() eq "1^2^3^4^5"_var);

		// Copy assign (from lvalue)
		d3 = d2;
		assert(d1.join() eq "1^2^3"_var);

		//		assert(d1 eq d2);
		//		assert(d1 ne d3);
	}

	dim a9;
	var a10;
	//check global split returns number of elements and an array out
	a9 = split("xx" ^ FM ^ "bb");
	TRACE(a9.rows())
	assert(a9.rows() eq 2);
	//check join returns correct string
	assert(join(a9) eq("xx" ^ FM ^ "bb"));
	dim dx(3);
	//assign all elements to one value
	dx = 1;
	//check join
	dx.join().outputl();
	(1 ^ FM ^ 1 ^ FM ^ 1);
	assert(dx.join().outputl() eq (1 ^ FM ^ 1 ^ FM ^ 1));

	//test dim=var.split()
	dim a = var("abc" _FM "def").split();
	assert(a[2] eq "def");

//	{// No way to limit number of fields at the moment
//		//dim.split(stringvar)
//		dim a12(4);
//		//test not enough fields - initialises rest of array elements to ""
//		assert(a12.split("a" _FM "b").rows());
//		assert(a12(2) eq "b");
//		assert(a12(3) eq "");
//		assert(a12(4) eq "");
//		//test extra fields are forced into last element
//		assert(a12.split("a" _FM "b" _FM "c" _FM "d" _FM "e").rows());
//		assert(a12(2) eq "b");
//		assert(a12(3) eq "c");
//		assert(a12(4) eq "d" _FM "e");
//	}

	//test sort and reverse sort
	dim a13 = var("10" _FM "2" _FM "1" _FM "20" _FM "-2").split();
	printl(a13.join());
	assert(a13.join("^") eq "10^2^1^20^-2");

	a13.sorter();
	printl(a13.join());
	assert(a13.join("^") eq "-2^1^2^10^20");

	a13.sorter(true);
	printl(a13.join());
	assert(a13.join("^") eq "20^10^2^1^-2");

	dim a11(10);
	a11 = "1";	//fill all with 1
	assert(a11[1]  eq 1);
	assert(a11[10] eq 1);
	a11 = split("");	//fill all with "" TODO should this not just make an array of 1 element?
	assert(a11.rows() eq 1);
//	assert(a11(1)  eq "");
//	assert(a11(10) eq "");

	{
		dim d = {1,2,3};
		d.redim(0);
		assert(d.join() eq "");
	}
	{
		// Reverse - odd
		dim d1 = {1,2,3,4,5};
		d1.reverser();
		assert(d1.join(",") eq "5,4,3,2,1");
		d1.reverser();
		assert(d1.reverse().join(",") eq "5,4,3,2,1");

		// Reverse - even
		dim d2 = {1,2,3,4};
		d2.reverser();
		assert(d2.join(",") eq "4,3,2,1");
		d2.reverser();
		assert(d2.reverse().join(",") eq "4,3,2,1");

		// Reverse - empty will not work since it is undimensioned
		dim d3 = {};
		try {
			assert(d3.reverse().join(",") eq "");
			assert(false);
		} catch (DimUndimensioned& e) { var(e.description).errputl();}

		// Reverse - unassigned should be error
		dim d4;
		try {
			assert(d4.reverse().join(",") eq "");
			assert(false);
		} catch (DimUndimensioned& e) { var(e.description).errputl();}

	}
	{
		// std algorithmns should work too!
		//std::range::reverse(d1);
		var v1 = "10^2^1^20"_var;
		var v2 = "20^1^2^10"_var;

		dim d1 = v1.split();
		std::reverse(std::begin(d1), std::end(d1));
		assert(d1.join() eq v2);

		d1 = v1.split();
		std::reverse(d1.begin(), d1.end());
		assert(d1.join() eq v2);

		// remove_if
		d1 = "10^2^1^20"_var.split();
		[[maybe_unused]] auto dummy1 = std::remove_if(d1.begin(), d1.end(), [](var x){return x eq "2";});
		// TODO add erase to exo::dim?
//		d1.erase(std::remove_if(d1.begin(), d1.end(), [](var x){return x eq "2";}), d1.end());
		TRACE(d1.join("^"))
		assert(d1.join() eq "10^1^20^"_var);

		// remove_if
		d1 = v1.split();
		[[maybe_unused]] auto dummy2 = std::remove_if(d1.begin(), d1.end(), [](var x){return x eq "2";});
		// TODO add erase to exo::dim?
//		d1.erase(std::remove_if(d1.begin(), d1.end(), [](var x){return x eq "2";}), d1.end());
		assert(d1.join() eq "10^1^20^"_var);

		// unique?
		d1 = "b^a^a^b^c"_var.split();
		[[maybe_unused]] auto dummy3 = std::unique(d1.begin(), d1.end());
		assert(d1.join().outputl() eq "b^a^b^c^"_var);

		// std::find
		d1 = "b^a^a^b^c"_var.split();
		auto x = std::find(d1.begin(), d1.end(), "c");
		const auto pos = std::distance(d1.begin(), x);
		assert(pos eq 4);
		//assert(d1.join().outputl() eq "b^a^b^c"_var);

		// erase_if?
		printl("erase_if doesnt compile despite dim having an erase function");
		printl("but remove_if combined with dim::erase will achieve the same result");
		d1 = "1^2^4^5^3"_var.split();
		TRACE(d1.join())
		//std::erase_if(d.begin(), d.end(), [](auto i){return i lt 3;});
// Removed dim::eraser(dim_iter, dim_iter) due to difficulty of converting a dim_iter to a vector::iterator
//		d1.eraser(
//			std::remove_if(
//				d1.begin(),
//				d1.end(),
//				[](auto i){return i lt 3;}
//			),
//			d1.end()
//		);
//		TRACE(d1.join())
//		assert(d1.join() eq "4^5^3"_var);

	}

	//test redimensioning
	dim aaaa(10);
	aaaa.redim(20, 30);

	//c array
	var r[2];

	//dim array
	assert(unassigned(r[0]));

	dim a7(2, 3);

	for (int ii = 1; ii le 2; ++ii) {
		for (int jj = 1; jj le 3; ++jj) {
			a7.at(ii, jj) = ii ^ var(".") ^ jj;
		}
	}

	dim a8(4, 5);
	for (int ii = 1; ii le 4; ++ii) {
		for (int jj = 1; jj le 5; ++jj) {
			a8.at(ii, jj) = ii ^ var(".") ^ jj;
		}
	}
	a8 = 2.2;

	for (int ii = 1; ii le 4; ++ii) {
		for (int jj = 1; jj le 5; ++jj) {
			a8.at(ii, jj).outputt("=");
		}
		//		printl();
	}

	a8 = a7;

	for (int ii = 1; ii le 2; ++ii) {
		for (int jj = 1; jj le 3; ++jj) {
			a8.at(ii, jj).outputt("=");
			assert(a8.at(ii, jj) eq a7.at(ii, jj));
			//		printl();
		}
	}
	printl();

	a7 = split("xx" ^ FM ^ "bb");
	assert(a7[1]                      eq "xx");
	assert(a7[2]                      eq "bb");
	assert(a7.join() eq("xx" ^ FM ^ "bb"));

	dim arrx(2, 2), arry;
	arrx	   = "";
	arrx.at(1, 1) = "11";
	assert(arrx.at(1, 1) eq "11");
	arrx.at(1, 2) = arrx.at(1, 1);
	assert(arrx.at(1, 2) eq "11");
	arry = arrx;
	assert(arry.at(1, 2) eq "11");

	//using mv dimensioned arrays
	//mv dimensioned arrays have a zero element that is
	//used in case either or both of the indexes are zero
	dim arr1(3), arr2(3, 3);
	arr1[0]	   = 0;
	arr1.at(0, 0) = 0;
	for (int ii = 1; ii le 3; ++ii) {
		arr1[ii] = ii;
		for (int jj = 1; jj le 3; ++jj) {
			arr2.at(ii, jj) = ii * 3 + jj;
		}
	}
	assert(arr1[0]        eq "0");
	assert(arr1.at(0, 0)     eq "0");
	for (int ii = 1; ii le 3; ++ii) {
		assert(arr1[ii]      eq ii);
		for (int jj = 1; jj le 3; ++jj) {
			assert(arr2.at(ii, jj) eq ii * 3 + jj);
		}
	}

	/* 
	//using c arrays UNSAFE! USE exodus dim INSTEAD;
	var arrxxx[10];

	//can use int but not var for indexing c arrays
	int intx=0;
	arrxxx[intx]="x";
	var varx=0;
	//following will not compile on MSVC (g++ is ok) due to "ambiguous" due to using var for index element
	//arrxxx[varx]="y";
	arrxxx[int(varx)]="y";
*/

	//another sort test

	var array = "";

	array(3) = 1 ^ VM ^ 2 ^ VM ^ 20 ^ VM ^ 10;
	array(6) = 61 ^ VM ^ 62 ^ VM ^ 620 ^ VM ^ 610;
	array(7) = 71 ^ VM ^ 72 ^ VM ^ 720 ^ VM ^ 710;

	array.convert(VM ^ FM, "]\n").outputl();

	call sortarray(array, 3 ^ VM ^ 7 ^ VM ^ 6, "DR");

	printl("-------------------");
	array.convert(VM ^ FM, "]\n").outputl();

	converter(array, VM, "]");
	assert(array.f(3) eq "20]10]2]1");
	assert(array.f(6) eq "620]610]62]61");
	assert(array.f(7) eq "720]710]72]71");

	{
		// Check extra seps get split and joined too
		var x = ",,a,,b,,";
		assert(x.split(",").rows() eq 7);
		assert(x.split(",").join(",") eq ",,a,,b,,");
	}

	{
		// Check dim.oswrite preserves line endings
		var tfilename = ostempfilename();
		var tx = "\n\nA\n\nB\n\n";
		assert(tx.split("\n").oswrite(tfilename));
		assert(osfile(tfilename).f(1) eq 8);

		// Check dim.osread preserves line endings
		dim d;
		assert(d.osread(tfilename));
		assert(d.rows().outputl() eq 7);
		assert(d.join("\n") eq tx);
	}

	//test reading and writing text files into and from dim arrays
	{
		var osfilename = "t_dim_rw.txt";
		var txt		   = "a\nb\n\nc";
		assert(oswrite(txt on osfilename));

		printl("test reading a test file into a dim array");
		dim d1;
		assert(d1.osread(osfilename));
		assert(d1.join("\n") eq txt);

		printl("Test writing a dim array to a text file");
		d1[3] = "bb";
		TRACE(d1.join())

		txt = "a\nb\nbb\nc\n";
		d1 = split(txt);
		TRACE(txt)
		TRACE(txt.oconv("HEX"))
		TRACE(txt.replace("\n","|"))
		TRACE(txt.replace("\n","|"))

		TRACE(d1.rows())
		assert(d1.oswrite(osfilename));
		assert(osfile(osfilename).f(1) eq 9);
		printl();
		assert(d1.osread(osfilename));
		TRACE(d1.join("\n").replace("\n","|"))
		TRACE(d1.rows())
		TRACE(txt)
		TRACE(txt.replace("\n","|"))
		//assert((d1.join("\n") ^ "\n") eq txt);
		assert(d1.join("\n") eq txt);

		printl("Make a wintext file");
		var temposfilename = ostempfilename().outputl();
		var wintext1 =
			"aaa\r\n"
			"bbb\r\n"
			"ccc\r\n"
			"ddd\r\n"
			"eee"
			"\r\n";
		assert(oswrite(wintext1 on temposfilename));

		printl("read wintext1 into dim array");
		dim d2;
		assert(d2.osread(temposfilename));
		TRACE(d2.join())

		printl("verify dimmary array element (5) is eee");
		assert(d2[5] eq "eee");

		printl("Verify line ending \r\n has been remembered in array element 0");
		TRACE(d2[0].oconv("HEX"))
		assert(d2[0] eq "\r\n");

		printl("Check round trip");
		assert(d2.join("\r\n") eq wintext1);

		printl("Check writing dim array to wintext file");
		var temposfilename2 = ostempfilename().outputl();
		assert(d2.oswrite(temposfilename2));

		printl("Check round trip");
		var wintext2;
		assert(osread(wintext2 from temposfilename2));
		TRACE(wintext1)
		TRACE(wintext2)
		TRACE(wintext1.oconv("HEX"))
		TRACE(wintext2.oconv("HEX"))
		assert(wintext2 eq wintext1);

		// Check cannot read a non-existent osfile
		assert(osremove(temposfilename));
		assert(osremove(temposfilename2));
		assert(not d1.osread(temposfilename));

		//assert(osremove(osfilename));
	}

	//constr nrows
	{
		dim x(4);
		TRACE(x.rows())
		TRACE(x.cols())
		assert(x.rows() eq 4);
		assert(x.cols() eq 1);

		//set each element to "x"
		x = "x";
		TRACE(x.join())

		//check join uses FM
		assert(x.join() eq "x^x^x^x"_var);

		//set each element to ""
		x = "";

		//check join doesnt trims trailing FM
		assert(x.join() eq "^^^"_var);
	}

	//constr nrows, ncols
	{
		dim x(2, 3);
		TRACE(x.rows())
		TRACE(x.cols())
		assert(x.rows() eq 2);
		assert(x.cols() eq 3);

		//set each element to "x"
		x = "x";
		TRACE(x.join())

		//check join uses FM
		assert(x.join() eq "x^x^x^x^x^x"_var);

		//set each element to ""
		x = "";

		//check join doesnt trim trailing FM
		assert(x.join() eq "^^^^^"_var);
	}

	// CONSTRUCT ASSIGN = {}

	// 1
	{
		dim x = {3};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3"_var);
		x = "x";
		assert(x.join() eq "x"_var);
		assert(x.rows() eq 1);
		assert(x.cols() eq 1);
	}

	// 2
	{
		dim x = {3, 4};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3^4"_var);
		x = "x";
		assert(x.join() eq "x^x"_var);
		assert(x.rows() eq 2);
		assert(x.cols() eq 1);
	}

	// 3
	{
		dim x = {3, 4, 5};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3^4^5"_var);
		x = "x";
		assert(x.join() eq "x^x^x"_var);
		assert(x.rows() eq 3);
		assert(x.cols() eq 1);
	}

	// CONSTRUCT PLAIN {}

	// 1
	{
		dim x{3};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3");
		x = "x";
		assert(x.join() eq "x");
		assert(x.rows() eq 1);
		assert(x.cols() eq 1);
	}

	// 2
	{
		dim x{3, 4};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3^4"_var);
		x = "x";
		assert(x.join() eq "x^x"_var);
		assert(x.rows() eq 2);
		assert(x.cols() eq 1);
	}

	// 3

	{
		dim x{3, 4, 5};
		TRACE(x.rows())
		TRACE(x.cols())
		TRACE(x.join())
		assert(x.join() eq "3^4^5"_var);
		x = "x";
		assert(x.join() eq "x^x^x"_var);
		assert(x.rows() eq 3);
		assert(x.cols() eq 1);
	}

	{
		// Check dim.splitter(v)
		dim d1;
		var rec = "aa^bb^cc"_var;
		d1.splitter(rec);
		TRACE(d1.join())
		assert(d1.join() eq rec);

		// Check split into existing array is padded with ""
		dim d2(5);
		d2.splitter(rec);
		TRACE(d2.join())
		assert(d2.join() eq (rec ^ FM ^ FM));

		// Check split into existing array overflows into final element
		dim d3(2);
		d3.splitter(rec);
		TRACE(d3[1])
		TRACE(d3[2])
		TRACE(d3.join())
		assert(d3.join() eq rec);
		assert(d3[2] eq "bb^cc"_var);
	}

	{
		dim d = {str("a",30), str("b",30), str("c",30)};
		var s = {str("a",30), str("b",30), str("c",30)};
		TRACE(d.join())
		TRACE(s)
		assert(d.join() eq
							"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa^"
							"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb^"
							"cccccccccccccccccccccccccccccc"_var);
		assert(d.join() eq
							"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa^"
							"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb^"
							"cccccccccccccccccccccccccccccc"_var);

		// Test JOIN on rvalue dim;
		assert(s.split().join() eq
							"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa^"
							"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb^"
							"cccccccccccccccccccccccccccccc"_var);
	}

	{
		// join unassigned should be error
		dim d1;
		try {
			var x = d1.join(",");
			assert(false);
		} catch (DimUndimensioned& e) { var(e.description).errputl();}
	}

	{
		// Testing two dimensional esp. negative row and col

		dim d(3,2);
		d[1,1]=11;
		d[1,2]="aa";
		d[2,1]=22;
		d[2,2]="bb";
		d[3,1]=33;
		d[3,2]="cc";

		//TODO could be ] and ^
		assert(d.join().outputl() == "11^aa^22^bb^33^cc"_var);

		assert(d.rows().outputl() == 3);
		assert(d.cols().outputl() == 2);

		{
			// coln = 1

			assert((d[-3,1].outputl() == "11"));
			assert((d[-2,1].outputl() == "22"));
			assert((d[-1,1].outputl() == "33"));

			assert((d[1,1].outputl() == "11"));
			assert((d[2,1].outputl() == "22"));
			assert((d[3,1].outputl() == "33"));

			//DimIndexOutOfBounds:row:-4 can be -3 to +3 - Aborting.
			try {
				assert((d[-4,1] == ""));
				assert(false);
			}
			catch (DimIndexOutOfBounds e) {};
			try {
				assert((d[4,1] == ""));
				assert(false);
			}
			catch (DimIndexOutOfBounds e) {};

		}

		{
			// coln 2

			assert((d[-3, 2].outputl() == "aa"));
			assert((d[-2, 2].outputl() == "bb"));
			assert((d[-1, 2].outputl() == "cc"));

			assert((d[1, 2].outputl() == "aa"));
			assert((d[2, 2].outputl() == "bb"));
			assert((d[3, 2].outputl() == "cc"));

		}

		{
			// coln 2 == -1

			assert((d[-3, 2].outputl() == d[-3, -1]));
			assert((d[-2, 2].outputl() == d[-2, -1]));
			assert((d[-1, 2].outputl() == d[-1, -1]));

			assert((d[1, 2].outputl() == d[1, -1]));
			assert((d[2, 2].outputl() == d[2, -1]));
			assert((d[3, 2].outputl() == d[3, -1]));

		}

		{
			// coln 1 == -2

			assert((d[-3, 1].outputl() == d[-3, -2]));
			assert((d[-2, 1].outputl() == d[-2, -2]));
			assert((d[-1, 1].outputl() == d[-1, -2]));

			assert((d[1, 1].outputl() == d[1, -2]));
			assert((d[2, 1].outputl() == d[2, -2]));
			assert((d[3, 1].outputl() == d[3, -2]));

		}

		{
			//col -3 and 3 error
			//DimIndexOutOfBounds:row:-4 can be -3 to +3 - Aborting.

			try {
				assert((d[1,-3] == ""));
				assert(false);
			}
			catch (DimIndexOutOfBounds e) {};
			try {
				assert((d[1,3] == ""));
				assert(false);
			}
			catch (DimIndexOutOfBounds e) {};

		}

		{
			// Row 0?
			try {
				assert((d[0,1] == ""));
				assert(false);
			}
			catch (VarUnassigned e) {};

			// Col 0?
			try {
				assert((d[1,0] == ""));
				assert(false);
			}
			catch (VarUnassigned e) {};

			// both 0?
			try {
				assert((d[0,0] == ""));
				assert(false);
			}
			catch (VarUnassigned e) {};

		}

	}

	{
		// Single dim negative row and col

		dim d {11,22,33};

		assert(d[-3] == "11");
		assert(d[-2] == "22");
		assert(d[-1] == "33");

		assert(d[1] == "11");
		assert(d[2] == "22");
		assert(d[3] == "33");

		//DimIndexOutOfBounds:row:-4 can be -3 to +3 - Aborting.
		try {
			assert((d[-4] == ""));
			assert(false);
		}
		catch (DimIndexOutOfBounds e) {};
		try {
			assert((d[4] == ""));
			assert(false);
		}
		catch (DimIndexOutOfBounds e) {};

		// Row 0?
		try {
			assert((d[0] == ""));
			assert(false);
		}
		catch (VarUnassigned e) {};

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
