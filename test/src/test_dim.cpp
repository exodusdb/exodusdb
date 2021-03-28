#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

	//dim array

	dim a9;
	var a10;
	//check global split returns number of elements and an array out
	assert(split("xx"^FM^"bb",a9) eq 2);
	//check join returns correct string
	assert(join(a9) eq ("xx" ^FM^ "bb"));

	dim dx(3);
	//assign all elements to one value
	dx=1;
	//check join
	assert(dx.join().outputl()==(1^FM^1^FM^1));

	//test dim=var.split()
	dim a=var("abc" _FM_ "def").split();
	assert(a(2) == "def");

	//dim.split(stringvar)
	dim a12(4);
	//test not enough fields - initialises rest of array elements to ""
	assert(a12.split("a" _FM_ "b"));
	assert(a12(2) == "b");
	assert(a12(3) == "");
	assert(a12(4) == "");
	//test extra fields are forced into last element
	assert(a12.split("a" _FM_ "b" _FM_ "c" _FM_ "d" _FM_ "e"));
	assert(a12(2) == "b");
	assert(a12(3) == "c");
	assert(a12(4) == "d" _FM_ "e");

	//test sort and reverse sort
	dim a13 = var("10" _FM_ "2" _FM_ "1" _FM_ "20" _FM_ "-2").split();
	//printl(a13.join());
	assert(a13.join("^") == "10^2^1^20^-2");
	a13.sort();
	//printl(a13.join());
	assert(a13.join("^") == "-2^1^2^10^20");
	a13.sort(true);
	//printl(a13.join());
	assert(a13.join("^") == "20^10^2^1^-2");

	dim a11(10);
	a11 = "1";//fill all with 1
	assert(a11(1) == 1);
	assert(a11(10) == 1);
	a11.split("");//fill all with "" TODO should this not just make an array of 1 element?
	assert(a11(1) == "");
	assert(a11(10) == "");

	//test redimensioning
	dim aaaa(10);
	aaaa.redim(20,30);

	//c array
	var r[2];

	//dim array
	assert(unassigned(r[0]));

	dim a7(2,3);

	for (int ii=1;ii<=2;++ii)
		for (int jj=1;jj<=3;++jj)
			a7(ii,jj)=ii^var(".")^jj;

	dim a8(4,5);
	for (int ii=1;ii<=4;++ii)
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj)=ii^var(".")^jj;

	a8=2.2;

	for (int ii=1;ii<=4;++ii) {
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj).outputt("=");
//		printl();
	}

	a8=a7;

	for (int ii=1;ii<=2;++ii) {
		for (int jj=1;jj<=3;++jj) {
			a8(ii,jj).outputt("=");
			assert(a8(ii,jj)==a7(ii,jj));
//		printl();
		}
	}
	printl();

	assert(a7.split("xx"^FM^"bb") eq 2);
	assert(a7(1) eq "xx");
	assert(a7(2) eq "bb");
	assert(a7.join() eq ("xx"^FM^"bb"));

	dim arrx(2,2),arry;
	arrx(1,1)="xx";
	assert(arrx(1,1) eq "xx");
	arrx(1,2)=arrx(1,1);
	assert(arrx(1,2) eq "xx");
	arry=arrx;
	assert(arry(1,1) eq "xx");

	//using mv dimensioned arrays
	//mv dimensioned arrays have a zero element that is
	//used in case either or both of the indexes are zero
	dim arr1(3), arr2(3,3);
	arr1(0)=0;
	arr1(0,0)=0;
	for (int ii=1; ii<=3; ++ii) {
		arr1(ii)=ii;
		for (int jj=1; jj<=3; ++jj)
			arr2(ii,jj)=ii*3+jj;
	}
	assert(arr1(0) eq "0");
	assert(arr1(0,0) eq "0");
	for (int ii=1; ii<=3; ++ii) {
		assert(arr1(ii) eq ii);
	for (int jj=1; jj<=3; ++jj)
		assert(arr2(ii,jj) eq ii*3+jj);
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

	var array="";

	array.r(3,  1  ^VM ^2  ^VM ^20  ^VM ^10);
	array.r(6,  61 ^VM ^62 ^VM ^620 ^VM ^610);
	array.r(7,  71 ^VM ^72 ^VM ^720 ^VM ^710);

	array.convert(VM^FM,"]\n").outputl();

	call sortarray(array,3 ^ VM ^ 7 ^ VM ^ 6,"DR");

	printl("-------------------");
	array.convert(VM^FM ,"]\n").outputl();

	array.converter(VM,"]");
	assert(array.a(3)=="20]10]2]1");
	assert(array.a(6)=="620]610]62]61");
	assert(array.a(7)=="720]710]72]71");

	printl("Test passed");

	return 0;
	}

programexit()