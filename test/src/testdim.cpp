#include <exodus/program.h>
#include <cassert>
#include <locale>

programinit()

function main()
{
{
	//std::locale locE ( "Ukrainian_Ukraine.1251" );	// en-US
	//std::locale loc ( "Greek_Greece.1253" );	// en-US
	std::locale loc;
	std::locale loc1;
	std::cout << "The initial locale is: " << loc1.name( ) << std::endl;
	std::locale loc2 = std::locale::global ( loc );
	std::locale loc3;
	std::cout << "The current locale is: " << loc3.name( ) << std::endl;
	std::cout << "The previous locale was: " << loc2.name( ) << std::endl;
}
	//std::locale mylocale("de");
	std::locale mylocale("C");

	///// STEP1. Test on default (Ukrainian) locale
	var locale0 = getxlocale().outputl("Original Locale=");

	var test = L"[English language][Русский язык][Українська мова]\n"
		L"can you write one A\n"
		L"[16:55:08] neosys: one й\n";
	var test_file = "test_1.txt";
	oswrite( test, test_file);
	test_file.osclose();

	var testback;
	testback.osread( test_file);
	test.outputl( "What was in test variable:");
	testback.outputl( "What was read:");

	printl( test == testback);

	///// STEP2. Test on Greek (1032) locale
	setxlocale( 1032);
	locale0 = getxlocale().outputl("New Locale=");

	test = L"[English language][we stick to greek \\u03A3][\u03A3]\n";
	test_file = "test_2.txt";
	oswrite( test, test_file);
	test_file.osclose();

	testback.osread( test_file);
	test.outputl( "What was in test variable:");
	testback.outputl( "What was read:");

	assert( test == testback);

/*	this code fragment tests UTF8 coding/decoding by reading utf8.html and writing its copy ...
	var utf8_html = "utf8.html";
	var buf;
	buf.osread( "utf8.html");
	buf.oswrite( "utf8copy.html");
*/
/* this is about greek character:
	var tempfilename5 = "tempfilename5W.txt";
	var record5;
	 var Greek =L"This is string with character \\u038A: '\u038A'\n"
				L"And this one is character \\u0393: '\u0393'";
	 oswrite(Greek, tempfilename5);
     tempfilename5.osflush();

	 var Greek2 =L"This is string without character \\u038A\n"
				L"And this one has no character \\u0393.";
	 assert(osbwrite(Greek2,tempfilename5,100));

	 assert(osread(record5,tempfilename5));
	 assert(record5.oconv("HEX2") eq "000000003738");
*/
	var tempfilename5 = "tempfilename5.txt";
	oswrite("",tempfilename5);

//	assert( osbwrite("78",tempfilename5,2));
	var offset=2;
	assert( osbwrite("78",tempfilename5,offset));

	osclose(tempfilename5);
	osdelete(tempfilename5);

	tempfilename5 = "tempfilename6.txt";
	oswrite("",tempfilename5);
	offset=10;
	assert(osbwrite("This is written from 10th byte", tempfilename5, offset));

	tempfilename5 = "tempfilename7.txt";
	oswrite("",tempfilename5);
	offset=0;
	assert(osbwrite("This is written from 0th byte", tempfilename5, offset));

	tempfilename5 = "tempfilename8.txt";
	oswrite("",tempfilename5);
	offset=0;
	assert(osbwrite("Just to test dynamically grown handle cache table", tempfilename5, offset));

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
		offset=i;
		buf.osbread( stroustrup.lcase(), offset, 5);
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

	printl();
	printl("Press Enter to execute other types of tests ...");
	int c = getchar();
	var(c).outputl("char=");

	var counting = "counting.txt";
	print( L"Start of osbwrite() benchmark: ");
	begin = begin.timedate();
	printl( begin.oconv( L"MTS"));
	for( int i = 0; i < 3459406 / 5; i += 5)
	{
		var buf(i);
		buf ^= L"\n";
		offset=i;
		buf.osbwrite( counting, offset);
	}
	end = end.timedate();
	print( L"End of  osbwrite()  benchmark: ");
	printl( end.oconv( L"MTS"));

//	var duration = end - begin;
//	printl( duration.oconv( L"MTS"));

	counting.osclose();

	printl();
	printl( L"Press any key to execute other types of tests ...");
	c = getchar();

	dim a9;
	var a10;
	assert(split("xx"^FM^"bb",a9) eq 2);
	assert(join(a9) eq ("xx" ^FM^ "bb"));

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

    printl( "dim test OK");

    return 0;
}

programexit()
