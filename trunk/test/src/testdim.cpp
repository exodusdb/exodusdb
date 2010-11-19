#include <exodus/program.h>
#include <cassert>

programinit()

function main()
{
	var tempfilename5 = "tempfilename5.txt";
	var record5;
	 oswrite("",tempfilename5);//
	 assert(osbwrite("78",tempfilename5,2));
	 assert(osread(record5,tempfilename5));
	 assert(record5.oconv("HEX2") eq "000000003738");
	 osclose(tempfilename5);
	 osdelete(tempfilename5);

	// Use any file with size 2-10Mb
	var stroustrup = "Stroustrup B. - The C++ programming language (3rd edition) (1997).pdf";
	var CppCounter = 0;
	var begin, end;
	print( L"Start of osbread() benchmark: ");
	begin = begin.timedate();
	printl( begin.oconv( L"MTS"));

	for( int i = 0; i < 3459406 / 5; i += 5)
	{
		var buf;
		buf.osbread( stroustrup, i, 5);
//		if( buf == L"C")
//			CppCounter ++;
//		print( var(i)^L" "^CppCounter^L"\r");
	}
	end = end.timedate();
	print( L"End of  osbread()  benchmark: ");
	printl( end.oconv( L"MTS"));

//	var duration = end - begin;
//	printl( duration.oconv( L"MTS"));

	stroustrup.osclose();

	printl( L"");
	print( L"Print any key to execute other types of tests ...");
	int c = getchar();

	var counting = "counting.txt";
	print( L"Start of osbwrite() benchmark: ");
	begin = begin.timedate();
	printl( begin.oconv( L"MTS"));
	for( int i = 0; i < 3459406 / 5; i += 5)
	{
		var buf(i);
		buf ^= L"\n";
		buf.osbwrite( counting, i);
	}
	end = end.timedate();
	print( L"End of  osbwrite()  benchmark: ");
	printl( end.oconv( L"MTS"));

//	var duration = end - begin;
//	printl( duration.oconv( L"MTS"));

	counting.osclose();

	print( L"Print any key to execute other types of tests ...");
	c = getchar();

	dim a9;
	var a10;
	assert(matparse("xx"^FM^"bb",a9) eq 2);
	assert(matunparse(a9) eq ("xx" ^FM^ "bb"));

	var r[2];
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

	for (int ii=1;ii<=4;++ii) {
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj).outputt("=");
//		printl();
	}

	assert(a7.parse("xx"^FM^"bb") eq 2);
	assert(a7(1) eq "xx");
	assert(a7(2) eq "bb");
	assert(a7.unparse() eq ("xx"^FM^"bb"));

	dim arrx(2,2),arry;
	arrx(1,1)="xx";
	assert(arrx(1,1) eq "xx");
	arrx(1,2)=arrx(1,1);
	assert(arrx(1,2) eq "xx");
	arry=arrx;
	assert(arry(1,1) eq "xx");

    printl( "dim test OK");

    return 0;
}

programexit()
