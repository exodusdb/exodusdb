//
// Exodus Development Platform
//
// testmultilang.cpp - console application, for testing locale specific IO and conversions
//
// NOTES.
//
#include <exodus/program.h>
#include <cassert>

programinit()

function main()
{
//	this code fragment tests UTF8 coding/decoding by reading utf8.html and writing its copy ...
	var utf8_html = "utf8.html";
	var buf;
	buf.osread( "utf8.html");				// read with built in UTF8 conversion wstringfromUTF8()
	buf.oswrite( "utf8copy.html");			// write with built in UTF8 conversion
	buf.osread( "utf8.html", "utf8");		// read with boost_utf8-facet
	buf.oswrite( "utf8utf8.html", "utf8");	// write with boost_utf8_facet
	printl( "Check that 'utf8.html', 'utf8copy.html', 'utf8utf8.html' in working directory - identical");
	print( "Press any good key to continue ...");
	char c = getchar();

	// and chinese page
	buf.osread( "calblur8.html", "utf8");
	buf.oswrite( "calblur8utf8.html", "utf8");
	printl( "Check that 'calblur8.html' and 'calblur8utf8.html' in working directory - identical");
	print( "Press any good key to continue ...");
	c = getchar();

//  this code fragment tests locale specific characters IO with narrow files (1 char = 1 byte codepages)
	var EN_RU_UA_txt1 = L"[English language][Русский язык][Українська мова]\n";
	var EN_RU_UA_txt2;
	var EN_RU_UA_file = "test_EN_RU_UA.txt";
//	oswrite( EN_RU_UA_txt1, EN_RU_UA_file, 1058);
	oswrite( EN_RU_UA_txt1, EN_RU_UA_file, "Ukrainian_Ukraine.1251");
	EN_RU_UA_file.osclose();
	EN_RU_UA_txt2.osread( EN_RU_UA_file, "Ukrainian_Ukraine.1251");
	EN_RU_UA_txt1.outputl( "Written   text:");
	EN_RU_UA_txt2.outputl( "Read back text:");

	print( "Press any good key to continue ...");
	c = getchar();

//  Do it again, dynamically changing locale to Greek
	var EN_GREEK_txt1 = L"[Greek character \\u03A3][\u03A3]\n";
	var EN_GREEK_txt2;
	var EN_GREEK_file = "test_EN_GREEK.txt";
	oswrite( EN_GREEK_txt1, EN_GREEK_file, "Greek_Greece.1253");
	EN_GREEK_file.osclose();
	EN_GREEK_txt2.osread( EN_GREEK_file, "Greek_Greece.1253");
	EN_GREEK_txt1.outputl( "Written   text:");
	EN_GREEK_txt2.outputl( "Read back text:");

	print( "Press any good key to continue ...");
	c = getchar();

// Test throwing MvException on wrong locale name
// Uncomment following line to test
//	oswrite( EN_GREEK_txt1, EN_GREEK_file, "some_bad_locale");

//  this code fragment tests locale specific characters IO with UTF8 files
	var MIXTURE_txt1 = L"[English][Русский][Українська][Greek Char:\\u03A3][\u03A3]\n";
	var MIXTURE_txt2;
	var MIXTURE_file = "test_MIXTURE.txt";
	oswrite( MIXTURE_txt1, MIXTURE_file, "utf8");
	MIXTURE_file.osclose();
	MIXTURE_txt2.osread( MIXTURE_file, "utf8");
	MIXTURE_txt1.outputl( "Written   text:");
	MIXTURE_txt2.outputl( "Read back text:");

	print( "Press any good key to continue ...");
	c = getchar();

//  this code fragment tests positioning in UTF8 files
	var BUF_0_9, BUF_9_9, BUF_18_12, BUF_19_12, BUF_20_12, BUF_30_23;
	var position = 0;
	BUF_0_9.osbread( MIXTURE_file, position, 9, "utf8");
	position = 9;
	BUF_9_9.osbread( MIXTURE_file, position, 9, "utf8");
	position = 18;
	BUF_18_12.osbread( MIXTURE_file, position, 12, "utf8");
	position = 19;
	BUF_19_12.osbread( MIXTURE_file, position, 12, "utf8");
	position = 20;
	BUF_20_12.osbread( MIXTURE_file, position, 12, "utf8");
	position = 30;
	BUF_30_23.osbread( MIXTURE_file, position, 23, "utf8");
	BUF_0_9.outputl( "BUF_0_9=");
	BUF_9_9.outputl( "BUF_9_9=");
	BUF_18_12.outputl( "BUF_18_12=");
	BUF_19_12.outputl( "BUF_18_12=");
	BUF_30_23.outputl( "BUF_30_23=");

	position = 0;
	BUF_0_9.osbread( MIXTURE_file, position, 9, "utf8");
	BUF_9_9.osbread( MIXTURE_file, position, 9, "utf8");
	BUF_18_12.osbread( MIXTURE_file, position, 12, "utf8");
	BUF_30_23.osbread( MIXTURE_file, position, 23, "utf8");
	BUF_0_9.outputl( "BUF_0_9=");
	BUF_9_9.outputl( "BUF_9_9=");
	BUF_18_12.outputl( "BUF_18_12=");
	BUF_30_23.outputl( "BUF_30_23=");

	// Lets test osbwrite() with position update and UTF8
	var OUTPUT_file = "test_OUTPUT_UTF8.txt";
	oswrite( "", OUTPUT_file, "utf8");
	position = 5;
	assert( osbwrite( L"1234567890", OUTPUT_file, position, "utf8"));
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position, "utf8"));
	assert( osbwrite( L"Фраза із 22 символів", OUTPUT_file, position, "utf8"));
	assert( osbwrite( L"\n1234567890", OUTPUT_file, position, "utf8"));
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position, "utf8"));
	position = 10;
	assert( osbwrite( L"XYZ", OUTPUT_file, position, "utf8"));

	// Lets test osbwrite() with position update and default locale (1251)
	OUTPUT_file = "test_OUTPUT_1251.txt";
	oswrite( "", OUTPUT_file, "");
	position = 5;
	assert( osbwrite( L"1234567890", OUTPUT_file, position));
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	assert( osbwrite( L"Фраза із 22 символів", OUTPUT_file, position));
	assert( osbwrite( L"Фраза из 22 символов", OUTPUT_file, position));
	assert( osbwrite( L"\n1234567890", OUTPUT_file, position));
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	position = 10;
	assert( osbwrite( L"XYZ", OUTPUT_file, position));
	return 0;
}

programexit()

//
//{
//   std::locale locE ( "Ukrainian_Ukraine.1251" );	// en-US
//   std::locale loc ( "Greek_Greece.1253" );	// en-US
//   std::locale loc1;
//   std::cout << "The initial locale is: " << loc1.name( ) << std::endl;
//   std::locale loc2 = std::locale::global ( loc );
//   std::locale loc3;
//   std::cout << "The current locale is: " << loc3.name( ) << std::endl;
//   std::cout << "The previous locale was: " << loc2.name( ) << std::endl;
//}
//    std::locale mylocale("de");
//
//	///// STEP1. Test on default (Ukrainian) locale
//	var locale0 = getxlocale().outputl("Original Locale=");
//
//	oswrite( test, test_file);
//	test_file.osclose();
//
//	var testback;
//	testback.osread( test_file);
//	test.outputl( "What was in test variable:");
//	testback.outputl( "What was read:");
//
//	printl( test == testback);
//
//	///// STEP2. Test on Greek (1032) locale
//	setxlocale( 1032);
//	locale0 = getxlocale().outputl("New Locale=");
//
//	test = L"[English language][we stick to greek \\u03A3][\u03A3]\n";
//	test_file = "test_2.txt";
//	oswrite( test, test_file);
//	test_file.osclose();
//
//	testback;
//	testback.osread( test_file);
//	test.outputl( "What was in test variable:");
//	testback.outputl( "What was read:");
//
//	assert( test == testback);
//
///* this is about greek character:
//	var tempfilename5 = "tempfilename5W.txt";
//	var record5;
//	 var Greek =L"This is string with character \\u038A: '\u038A'\n"
//				L"And this one is character \\u0393: '\u0393'";
//	 oswrite(Greek, tempfilename5);
//     tempfilename5.osflush();
//
//	 var Greek2 =L"This is string without character \\u038A\n"
//				L"And this one has no character \\u0393.";
//	 assert(osbwrite(Greek2,tempfilename5,100));
//
//	 assert(osread(record5,tempfilename5));
//	 assert(record5.oconv("HEX2") eq "000000003738");
//*/
//	var tempfilename5 = "tempfilename5.txt";
//	oswrite("",tempfilename5);
//	 assert( osbwrite("78",tempfilename5,2));
//	 osclose(tempfilename5);
//	 osdelete(tempfilename5);
//
//	tempfilename5 = "tempfilename6.txt";
//	oswrite("",tempfilename5);
//	assert(osbwrite("This is written from 10th byte", tempfilename5, 10));
//
//	tempfilename5 = "tempfilename7.txt";
//	oswrite("",tempfilename5);
//	assert(osbwrite("This is written from 0th byte", tempfilename5, 0));
//
//	tempfilename5 = "tempfilename8.txt";
//	oswrite("",tempfilename5);
//	assert(osbwrite("Just to test dynamically grown handle cache table", tempfilename5, 0));
//
//	// Use any file with size 2-10Mb
//	var stroustrup = "Stroustrup B. - The C++ programming language (3rd edition) (1997).pdf";
//	var CppCounter = 0;
//	var begin, end;
//	print( L"Start of osbread() benchmark: ");
//	begin = begin.timedate();
//	printl( begin.oconv( L"MTS"));
//
//	for( int i = 0; i < 3459406 / 5; i += 5)
//	{
//		var buf;
//		buf.osbread( stroustrup, i, 5);
////		if( buf == L"C")
////			CppCounter ++;
////		print( var(i)^L" "^CppCounter^L"\r");
//	}
//	end = end.timedate();
//	print( L"End of  osbread()  benchmark: ");
//	printl( end.oconv( L"MTS"));
//
////	var duration = end - begin;
////	printl( duration.oconv( L"MTS"));
//
//	stroustrup.osclose();
//
//	printl( L"");
//	print( L"Print any key to execute other types of tests ...");
//	int c = getchar();
//
//	var counting = "counting.txt";
//	print( L"Start of osbwrite() benchmark: ");
//	begin = begin.timedate();
//	printl( begin.oconv( L"MTS"));
//	for( int i = 0; i < 3459406 / 5; i += 5)
//	{
//		var buf(i);
//		buf ^= L"\n";
//		buf.osbwrite( counting, i);
//	}
//	end = end.timedate();
//	print( L"End of  osbwrite()  benchmark: ");
//	printl( end.oconv( L"MTS"));
//
////	var duration = end - begin;
////	printl( duration.oconv( L"MTS"));
//
//	counting.osclose();
//
//	print( L"Print any key to execute other types of tests ...");
//

