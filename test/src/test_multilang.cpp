#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

//
// Exodus Development Platform
//
// testmultilang.cpp - console application, for testing locale specific IO and conversions
//
// NOTES.
//
#include <exodus/program.h>

programinit()

function main()
{
	var cmd;

	var exodusdir=osgetenv("GITHUB_WORKSPACE");
	if (exodusdir) {
		printl(exodusdir);
		oscwd(exodusdir ^ "/test/src");
	}

	//	this code fragment tests UTF8 coding/decoding by reading utf8.html and writing its copy ...
	var utf8_html = "utf8.html";
	var buf;
	buf.osread( utf8_html); //default external and internal format is utf8
	buf.oswrite( "t_utf8copy.html");
	cmd="diff " ^ utf8_html ^ " t_utf8copy.html";
	printl(cmd);
	assert(osshell(cmd));

	buf.osread( utf8_html, "utf8");		// read with boost_utf8-facet
	buf.oswrite( "t_utf8utf8.html", "utf8");	// write with boost_utf8_facet
	cmd="diff " ^ utf8_html ^ " t_utf8utf8.html";
	printl(cmd);
	assert(osshell(cmd));

	// and chinese page
	buf.osread( "calblur8.html", "utf8");
	buf.oswrite( "t_calblur8utf8.html", "utf8");
	cmd="diff calblur8.html t_calblur8utf8.html";
	printl(cmd);
	assert(osshell(cmd));

//  this code fragment tests locale specific characters IO with narrow files (1 char = 1 byte codepages)
	var cyrillic_cp = VOLUMES ? "Ukrainian_Ukraine.1251" : "CP1251";
	//osshell("locale ru_RU.CP1251 || locale-gen ru_RU.CP1251");
	//[<D0><F3><F1><F1><EA><E8><E9> <FF><E7><FB><EA>][<D3><EA><F0><E0><BF><ED><F1><FC><EA><E0> <EC><EE><E2><E0>]
	var EN_RU_UA_cp1251 = "[English language][������� ����][��������� ����]";
	var EN_RU_UA_utf8 =   "[English language][Русский язык][Українська мова]";
	assert(EN_RU_UA_cp1251.length() == 49);
	assert(EN_RU_UA_utf8.length() == 74);
	assert(EN_RU_UA_cp1251.from_codepage(cyrillic_cp) == EN_RU_UA_utf8);
	assert(EN_RU_UA_cp1251 == EN_RU_UA_utf8.to_codepage(cyrillic_cp));

	std::cout << ">[English language][Русский язык][Українська мова]" << std::endl;
	EN_RU_UA_cp1251.from_codepage(cyrillic_cp).outputl(">");

	//convert write utf8 with code page
	var EN_RU_UA_file = "t_EN_RU_UA.txt";
	oswrite(EN_RU_UA_utf8, EN_RU_UA_file, cyrillic_cp);
	printl(osfile(EN_RU_UA_file));
	assert(osfile(EN_RU_UA_file).a(1) == 49);

	//convert write cp1251 WITHOUT code page - will go out unchanged
	oswrite(EN_RU_UA_cp1251, EN_RU_UA_file);
	assert(osfile(EN_RU_UA_file).a(1) == 49);

	//read it with code page
	var EN_RU_UA_txt2;
	osread(EN_RU_UA_txt2, EN_RU_UA_file, cyrillic_cp);
	assert(EN_RU_UA_txt2 == EN_RU_UA_utf8);
	EN_RU_UA_txt2.outputl("<");

	//read it WITHOUT code page (it will arrive back as NOT UTF8)
	osread(EN_RU_UA_txt2, EN_RU_UA_file);
	assert(EN_RU_UA_txt2 == EN_RU_UA_cp1251);
	EN_RU_UA_txt2.from_codepage("CP1251").outputl("<");

// Test throwing MvException on wrong locale name
// Uncomment following line to test
//	oswrite( EN_GREEK_utf8, EN_GREEK_file, "some_bad_locale");

//  this code fragment tests locale specific characters IO with UTF8 files
	var MIXTURE_txt1 = "[English][�������][���������][Greek Char:\\u03A3][\u03A3]\n";
	var MIXTURE_txt2;
	var MIXTURE_file = "t_test_MIXTURE.txt";
	oswrite( MIXTURE_txt1, MIXTURE_file, "utf8");
	MIXTURE_file.osclose();
	MIXTURE_txt2.osread( MIXTURE_file, "utf8");
	MIXTURE_txt1.outputl( "Written   text:");
	MIXTURE_txt2.outputl( "Read back text:");

	//printl( "Press Enter key to continue ...");
	//c = getchar();

//  this code fragment tests positioning in UTF8 files
	var BUF_0_9, BUF_9_9, BUF_18_12, BUF_19_12, BUF_20_12, BUF_30_23;
	var position = 0;
	BUF_0_9.osbread( MIXTURE_file, position, 9);
	position = 9;
	BUF_9_9.osbread( MIXTURE_file, position, 9);
	position = 18;
	BUF_18_12.osbread( MIXTURE_file, position, 12);
	position = 19;
	BUF_19_12.osbread( MIXTURE_file, position, 12);
	position = 20;
	BUF_20_12.osbread( MIXTURE_file, position, 12);
	position = 30;
	BUF_30_23.osbread( MIXTURE_file, position, 23);
	BUF_0_9.outputl( "BUF_0_9=");
	BUF_9_9.outputl( "BUF_9_9=");
	BUF_18_12.outputl( "BUF_18_12=");
	BUF_19_12.outputl( "BUF_18_12=");
	BUF_30_23.outputl( "BUF_30_23=");

	position = 0;
	BUF_0_9.osbread( MIXTURE_file, position, 9);
	BUF_9_9.osbread( MIXTURE_file, position, 9);
	BUF_18_12.osbread( MIXTURE_file, position, 12);
	BUF_30_23.osbread( MIXTURE_file, position, 23);
	BUF_0_9.outputl( "BUF_0_9=");
	BUF_9_9.outputl( "BUF_9_9=");
	BUF_18_12.outputl( "BUF_18_12=");
	BUF_30_23.outputl( "BUF_30_23=");

	{
		assert(var(L"abc") eq "abc");
		assert(var(L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.") eq "Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.");
		var x = L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.";
		assert(x.replace("[[:alnum:]]","").convert(", ", "").outputl() == "/.");

		assert(oswrite(L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.", "t_file"));
		assert(osfile("t_file"));
		assert(osread("t_file") eq x);
	}

	var OUTPUT_file = "t_test_OUTPUT_UTF8.txt";
	oswrite( "", OUTPUT_file, "utf8");
	position = 5;
	assert( osbwrite( L"1234567890", OUTPUT_file, position));
	assert(position eq 15);
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	assert(position eq 26);
	assert( osbwrite( "����� �� 22 �������", OUTPUT_file, position));
	assert(position eq 46);
	assert( osbwrite( L"\n1234567890", OUTPUT_file, position));
	assert(position eq 57);
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	assert(position eq 68);
	position = 10;
	assert( osbwrite( L"XYZ", OUTPUT_file, position));
	assert(position eq 13);

	// Lets test osbwrite() with position update and default locale (1251)
	OUTPUT_file = "t_test_OUTPUT_1251.txt";
	oswrite( "", OUTPUT_file, "");
	position = 5;
	assert( osbwrite( L"1234567890", OUTPUT_file, position));
	assert(position eq 15);
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	assert(position eq 26);
	assert( osbwrite( "����� �� 22 �������", OUTPUT_file, position));
	assert(position eq 46);
	assert( osbwrite( "����� �� 22 ��������", OUTPUT_file, position));
	assert(position eq 66);
	assert( osbwrite( L"\n1234567890", OUTPUT_file, position));
	assert(position eq 77);
	assert( osbwrite( L"1234567890\n", OUTPUT_file, position));
	assert(position eq 88);
	position = 10;
	assert( osbwrite( L"XYZ", OUTPUT_file, position));
	assert(position eq 13);

	{
		assert(var(123456.789).numberinwords("").outputl() eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(123456.789).numberinwords("english").outputl() eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(123456.789).numberinwords("greek").outputl() eq "εκατόν είκοσι τρεις χίλιάδες τετρακόσια πενήντα έξι κόμμα επτά οκτώ εννέα");
		assert(var(123456.789).numberinwords("arabic").outputl() eq "مائة و ثلاثة و عشرون ألف و أربعة مائة و ستة و خمسون فاصل سبعة ثمانية تسعة");
		assert(var(123456.789).numberinwords("french").outputl() eq "cent vingt-trois mille quatre cent cinquante-six virgule sept huit neuf");
		assert(var(123456.789).numberinwords("spanish").outputl() eq "ciento veintitrés mil cuatrocientos cincuenta y seis punto siete ocho nueve");

		assert(var(-123456.789).numberinwords("").outputl() eq "minus one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(-123456.789).numberinwords("english").outputl() eq "minus one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(-123456.789).numberinwords("greek").outputl() eq "μείον εκατόν είκοσι τρεις χίλιάδες τετρακόσια πενήντα έξι κόμμα επτά οκτώ εννέα");
		assert(var(-123456.789).numberinwords("arabic").outputl() eq "ناقص مائة و ثلاثة و عشرون ألف و أربعة مائة و ستة و خمسون فاصل سبعة ثمانية تسعة");
		assert(var(-123456.789).numberinwords("french").outputl() eq "moins cent vingt-trois mille quatre cent cinquante-six virgule sept huit neuf");
		assert(var(-123456.789).numberinwords("spanish").outputl() eq "menos ciento veintitrés mil cuatrocientos cincuenta y seis punto siete ocho nueve");

		assert(var(10000000000000).numberinwords("").outputl() eq "ten trillion");
		assert(var(10000000000000).numberinwords("english").outputl() eq "ten trillion");
		assert(var(10000000000000).numberinwords("greek").outputl() eq "δέκα τρισεκατομμύρια");
		assert(var(10000000000000).numberinwords("arabic").outputl() eq "عشرة ترليون");
		assert(var(10000000000000).numberinwords("french").outputl() eq "dix billions");
		assert(var(10000000000000).numberinwords("spanish").outputl() eq "diez billones");

		assert(var(1.2345678901).numberinwords("").outputl() eq "one point two three four five six seven eight nine zero one");
		assert(var(1.2345678901).numberinwords("english").outputl() eq "one point two three four five six seven eight nine zero one");
		assert(var(1.2345678901).numberinwords("greek").outputl() eq "ένα κόμμα δύο τρία τέσσερα πέντε έξι επτά οκτώ εννέα μηδέν ένα");
		assert(var(1.2345678901).numberinwords("arabic").outputl() eq "واحد فاصل إثنان ثلاثة أربعة خمسة ستة سبعة ثمانية تسعة صفر واحد");
		assert(var(1.2345678901).numberinwords("french").outputl() eq "un virgule deux trois quatre cinq six sept huit neuf zéro un");
		assert(var(1.2345678901).numberinwords("spanish").outputl() eq "uno punto dos tres cuatro cinco seis siete ocho nueve cero uno");
	}

	printl("Test passed");

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
//	var tempfilename5 = "t_tempfilename5W.txt";
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
//	var tempfilename5 = "t_tempfilename5.txt";
//	oswrite("",tempfilename5);
//	 assert( osbwrite("78",tempfilename5,2));
//	 osclose(tempfilename5);
//	 osdelete(tempfilename5);
//
//	tempfilename5 = "t_tempfilename6.txt";
//	oswrite("",tempfilename5);
//	assert(osbwrite("This is written from 10th byte", tempfilename5, 10));
//
//	tempfilename5 = "t_tempfilename7.txt";
//	oswrite("",tempfilename5);
//	assert(osbwrite("This is written from 0th byte", tempfilename5, 0));
//
//	tempfilename5 = "t_tempfilename8.txt";
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

