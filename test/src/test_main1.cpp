#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
//#include <sstream>
//#include <cmath>

#include <exodus/program.h>

#define BOOST_REGEX 1

// TESTING
// =======
//
// if it runs ok with no static assertion failures
//
//   compile test_main && test_main
//
// then test no changes in output with the following
//
//   test_main1 &> t_test_main1 && diff test_main1.out t_test_main1
//
// there will be a limited number of differences due to time/date/random output

// DEVELOPING
// ==========
//
// if you add to or change the output in any way,
// which includes adding comments at the top due since program line numbers are part of output during error reporting
// then update the expected output file
//
//   test_main1 &> test_main1.out

programinit()

function main()
{

//EXODUS syntax directions
//
//
//	x[]
//
//	.substr(2,2) ... for char
//
//	xxx[1] ... for single chars or could be used to extract
//
//	in c++23 [] becomes multidimensional
//
//	??? extract in c++23 multidim []
//
//	FM (AREV) or AM (Pick)
//
//	AREV                          EXODUS
//
//	                              xxx.r(2,vn,yyy.f(2,vn)); OLD
//
//	xxx<2,vn,sn> = yyy<2,vn>      xxx(2,vn) = yyy.f(2,vn); CURRENT
//
//	                              xxx(2,vn) = yyy(2,vn);   POSSIBLE NOW WITH PROBLEMS
//
//
//	// wont work unless var() proxy is equipped with .write member function
//	yyy(2,vn).write(file,key);
//
//	AREV                              EXODUS?
//	xxx<2,vn,sn> = yyy<2,vn>          xxx(2,vn) = yyy[2,vn]
//
//
//	field(fromstr,sep,start,n=1)
//	fromstr.field(sep,start,n=1)
//
	{
		assert("1a^2b^3c"_var.f(2).str(3) == "2b2b2b");
	}

	/* UBUNTU locale-gen
	locale-gen de_DE.UTF-8
	locale-gen en_GB.UTF-8
	locale-gen en_US.UTF-8
	locale-gen el_GR.UTF-8
	locale-gen tr_TR.UTF-8
	dpkg-reconfigure locales
	*/

	/*Ubuntu
		apt-cache search locale |grep -i greek
		greek turkish german
		apt-get install language-support-el language-support-tr language-support-de
	*/

	//non-ASCII unicode characters

	var ArabicIndicDigits		="٠١٢٣٤٥٦٧٨٩";
	var alldigits=
	"Western    : 0 1 2 3 4 5 6 7 8 9  \n"
	"Arabic     : ٠ ١ ٢ ٣ ٤ ٥ ٦ ٧ ٨ ٩  \n"
	"Devanagari : ० १ २ ३ ४ ५ ६ ७ ८ ९  \n"
	"Gujarati   : ૦ ૧ ૨ ૩ ૪ ૫ ૬ ૭ ૮ ૯  \n"
	"Punjabi    : ੦ ੧ ੨ ੩ ੪ ੫ ੬ ੭ ੮ ੯  \n"
	"Bengali    : ০ ১ ২ ৩ ৪ ৫ ৬ ৭ ৮ ৯  \n"
	"Odia       : ୦ ୧ ୨ ୩ ୪ ୫ ୬ ୭ ୮ ୯  \n"
	"Telugu     : ౦ ౧ ౨ ౩ ౪ ౫ ౬ ౭ ౮ ౯  \n"
	"Kannada    : ೦ ೧ ೨ ೩ ೪ ೫ ೬ ೭ ೮ ೯  \n"
	"Tamil      : ௦ ௧ ௨ ௩ ௪ ௫ ௬ ௭ ௮ ௯  \n"
	"Malayalam  : ൦ ൧ ൨ ൩ ൪ ൫ ൬ ൭ ൮ ൯  \n";

	var GreekSmallAlpha         ="α";//\u03B1";//GREEK SMALL LETTER ALPHA
	var GreekSmallFinalSigma    ="ς";//\u03C2";//GREEK SMALL LETTER FINAL SIGMA
	var GreekSmallSigma         ="σ";//\u03C3";//GREEK SMALL LETTER SIGMA
	var GreekCapitalSigma       ="Σ";//\u03A3";//GREEK CAPITAL LETTER SIGMA
	var GreekCapitalAlpha       ="Α";//\u0391";//GREEK CAPITAL LETTER ALPHA
	var Greek_sas               ="σας";//GreekSmallSigma^GreekSmallAlpha^GreekSmallFinalSigma;
	var Greek_SAS               ="ΣΑΣ";//GreekCapitalSigma^GreekCapitalAlpha^GreekCapitalSigma;
	var GreekQuestionMark		=";";//\u037E";//GREEK QUESTION MARK (Punctuation)
	var GreekCapitalGamma		="Γ";//\u0393";//GREEK CAPITAL LETTER GAMMA (Letter) (Uppercase)
	var GreekSmallGamma		="γ";//\u03B3";//GREEK SMALL LETTER GAMMA (Letter) (Lowercase)
	var ArabicIndicDigitZero	="٠";//\u0660";//ARABIC-INDIC DIGIT ZERO (Decimal Digit)

	var TurkishCapitalDottedI   ="İ";//\u0130";
	var TurkishSmallDotlessI    ="ı";//\u0131";
	var LatinSmallI             ="i";
	var LatinCapitalI           ="I";

	var GermanEszet             ="ß";//"\u00DF";//German

	// test normalization

	//NFD - Decomposed Normal Form
	//NFC - Compact Normal Form
	var decomp_a="\x61\xCC\x81";//"á";
	var compact_a="\xC3\xA1";//"á";

	//should NOT compile since it has no effect
	// and convert defined as NODISCARD
	//decomp_a.convert("v","y");

	assert(decomp_a=="á");
	assert(compact_a=="á");

	assert(decomp_a.oconv("HEX")=="61CC81");
	assert(compact_a.oconv("HEX")=="C3A1");

	//test normalize (to NFC)
	assert(decomp_a != compact_a);
	assert(decomp_a.normalize() == compact_a);
	assert(normalize(decomp_a) == compact_a);//func
	assert(decomp_a.normalize().oconv("HEX")=="C3A1");
	var b = decomp_a;
	normalizer(b);
	assert(b eq compact_a);

	//test read/write/delete normalization
	//exodus will normalize everything being written to the database because otherwise kind of duplicate keys can exist because
	//postgres says the following is false ... because one is a single unicode code unit and the other is two combined into one for presentation
	//SELECT 'á' = 'á';

	//check on invalid utf-8

	//should except on a abaft about above afore after along amid among an apud as aside at atop below but by circa down for from given in into lest like mid midst minus near next of off on onto out ov$
	var utftest,utftest2;
	var utftestfilename="utf-8-test.txt";

	//osread invalid utf8 should read without change
	//will be unwritable to database which only accepts utf8 key and data
	if (not osread(utftest,utftestfilename)) {
		var base_path=osgetenv("GITHUB_WORKSPACE");
		base_path.outputl("GITHUB_WORKSPACE=");
		if (base_path)
			oscwd(base_path ^ "/test/src/");
		if (not osread(utftest,utftestfilename)) {
			abort("test_main must be run in ~/exodus/test/src to have access to " ^ utftestfilename);
		}
	}
	utftest.len().outputl("len=");
	assert(len(utftest)==osfile(utftestfilename).f(1));

	//check invalid utf8 has no change oswrite/osread round trip
	oswrite(utftest,"t_"^utftestfilename);
	osread(utftest2,"t_"^utftestfilename);
	assert(utftest2==utftest);

	//check invalid utf has no change on ucase/lcase round trip
	assert(lcase(ucase(utftest))==lcase(utftest));
	assert(ucase(lcase(utftest))==ucase(utftest));

	//check what normalize() does on invalid utf
	var fixed_utftest=utftest.normalize();
	assert(fixed_utftest.len()==22400);

	//multibyte sep index
	var greek5x2="αβγδεαβγδε";
	assert(greek5x2.index("β")==3);
	assert(greek5x2.index("β",2)==13);
	// alternative starting from byte no
	assert(greek5x2.index2("β")==3);
	assert(greek5x2.index2("β",3)==3);//start is optional and works the same as index if missing
	assert(greek5x2.index2("β",4)==13);// start on the 2nd byte of two byte utf-8 char skips it

	//empty target string always returns "" instead of 0
	assert(greek5x2.index("")=="");
	assert(greek5x2.index2("",4)=="");

	//target string not found returns zero
	assert(greek5x2.index("xyz")==0);
	assert(greek5x2.index2("xyz",4)==0);

	// free function version of above
	assert(index(greek5x2,"β")==3);
	assert(index(greek5x2,"β",2)==13);
	assert(index2(greek5x2,"β")==3);
	assert(index2(greek5x2,"β",3)==3);
	assert(index2(greek5x2,"β",4)==13);

	//multibyte sep field
	var greek5x2f2=field(greek5x2,"β",2);
	assert(greek5x2f2=="γδεα");
	// alternative starting from byte no so doesnt work well with utf8 other than ASCII
	assert(greek5x2f2.length()==8);
	assert(greek5x2f2.oconv("HEX")=="CEB3CEB4CEB5CEB1");

    var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	//var greekstr2=fieldstore(greek5x2,"β",2,1,"xxx");

	//text four byte utf8
	var chinesechar=textchr(171416);
	assert(chinesechar=="𩶘");
	printl(chinesechar);
	assert(textseq(chinesechar^"abcd")==171416);
	assert(oconv(chinesechar,"HEX")=="F0A9B698");

	printl(textchr(915));
	assert(textchr(915)==GreekCapitalGamma);
	assert(textseq(GreekCapitalGamma)==915);

	//text three byte utf8
	var euro="€";
	assert(euro==textchr(8364));
	assert(textseq(euro)==8364);
	assert(textseq(euro^"abc")==8364);
	assert(oconv(euro,"HEX")=="E282AC");

	//multibyte locate using
	var setting;
	assert(greek5x4.locateusing("β","γδεα",setting));
	assert(setting==2);

	var errmsg;
	//if (not dbcreate("steve",errmsg))
	//	errmsg.outputl();
	//if (not dbdelete("steve",errmsg))
	//	errmsg.outputl();

	var testfilename="t_vm.txt";
	var charout=GreekSmallGamma;//VM;
	assert(oswrite(charout,testfilename));
	assert(osfile(testfilename).f(1)==2);
	var offsetx=0;
	var testfilex;
	assert(osopen(testfilename,testfilex));
	TRACE(testfilex);
	assert(testfilex);
	var testdata;
	//assert(testdata.osbread(testfilex,offsetx=0,1) eq GreekSmallGamma);

	//write two greeksmallgammas (4 bytes)
	assert(oswrite(charout^charout,testfilename));

	//check reading 1 byte results in nothing
	TRACE(testfilex);
	TRACE(offsetx);
	//assert(testdata.osbread(testfilex,offsetx=0,1));
	assert(testdata.osbread(testfilex,offsetx,1));
	assert(testdata.length()==0);
	assert(testdata.oconv("HEX2")=="");
	assert(offsetx==0);

	//check reading 2 bytes results in 1 unicode character (2 bytes)
	offsetx=0;
	testdata.osbread(testfilex,offsetx,2);
	assert(testdata==charout);
	assert(testdata.length()==2);
	assert(testdata.oconv("HEX2")=="CEB3");
	assert(offsetx==2);

	//check reading 3 bytes results in 1 unicode character (2 bytes)
	offsetx=0;
	testdata.osbread(testfilex,offsetx,3);
	assert(testdata==charout);
	assert(testdata.length()==2);
	assert(testdata.oconv("HEX2")=="CEB3");
	printl(offsetx);
	assert(offsetx==2);

	//check reading 4 bytes results in 2 unicode characters (4 bytes)
	offsetx=0;
	testdata.osbread(testfilex,offsetx,4);
	assert(testdata==(charout^charout));
	assert(testdata.length()==4);
	assert(testdata.oconv("HEX2")=="CEB3CEB3");
	assert(offsetx==4);

	//check reading 5 bytes results in 2 unicode characters (4 bytes)
	offsetx=0;
	testdata.osbread(testfilex,offsetx,5);
	assert(testdata==(charout^charout));
	assert(testdata.length()==4);
	assert(testdata.oconv("HEX2")=="CEB3CEB3");
	assert(offsetx==4);

//	assert(testfilename.osfile().f(1) eq 2);
	var charin;
//fails on ubuntu 13.10x64
	assert(osread(charin,testfilename));
	//assert(charin eq charout);
	assert(osremove(testfilename));


	//check conversion from and to codepages
	test_codepage("CP437","latin");//win3
	test_codepage("CP850","latin");//win10
	test_codepage("CP737","greek");
	//test_codepage("CP720","arabic");//doesnt exist so use CP1256
	test_codepage("CP1256","arabic");//720
	//test_codepage("CP1252","latin");
	//test_codepage("CP1253","greek");//737

	//check osread/write with and without conversion to codepages

	//cyrillic ISO-8859-5 cp bytes
	var cp_allo1{"\xB0\xDB\xDB\xDE"};

	//output binary unconverted
	assert(oswrite(cp_allo1,"t_cp_allo.txt"));
	assert(cp_allo1.oconv("HEX").outputl("cp_allo1=")=="B0DBDBDE");

	//hexdump -C t_cp_allo.txt
	//00000000  b0 db db de
	//00000004

	//check that we can read binary back in unconverted
	var cp_allo2=osread("t_cp_allo.txt");
	assert(cp_allo2.oconv("HEX").outputl("cp_allo2=")=="B0DBDBDE");

	//read in from binary cyrillic codepage text converting to utf8
	var utf8_allo3;
	assert(utf8_allo3.osread("t_cp_allo.txt","ISO-8859-5"));
	assert(utf8_allo3.length()==8);
	assert(utf8_allo3.oconv("HEX")=="D090D0BBD0BBD0BE");
	assert(utf8_allo3=="Алло");

	//output utf8 convering to ISO-8859-5
	assert(oswrite(utf8_allo3,"t_cp_allo4.txt","ISO-8859-5"));
	/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump cp_allo4.txt -C
00000000  b0 db db de                                       |....|
00000004
*/
	//read back in binary to check that out output did convert from utf to cyrillic
	var cp_allo5;
	assert(osread(cp_allo5,"t_cp_allo4.txt"));
	assert(cp_allo5==cp_allo1);

/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump t_utf8_allo4.txt -C
00000000  d0 90 d0 bb d0 bb d0 be                           |........|
00000008
*/

	assert(seq("")=="");
	assert(seq(chr(-513))==255);
	assert(seq(chr(-512))==0);
	assert(seq(chr(-256))==0);
	assert(seq(chr(-1))==255);
	assert(seq(chr(0))==0);
	assert(seq(chr(1))==1);
	assert(seq(chr(127))==127);
	assert(seq(chr(128))==128);
	assert(seq(chr(255))==255);
	assert(seq(chr(256))==0);
	assert(seq(chr(512))==0);
	assert(seq(chr(513))==1);

	//test all 8 bit bytes can be written and read
	var tx="";
	var tx2;
	for (int ii = 0; ii <= 65535; ii++)
		tx^=chr(ii);

	//output to binary - check roundtrip
	oswrite(tx,"test_main.$1");
	osread(tx2,"test_main.$1");
	assert(tx2==tx);

	//read as codepage
	//oswrite(tx,"test_main.$1","ISO-8859-5");
	osread(tx2,"test_main.$1","ISO-8859-5");
	//assert(tx2==tx);

	osremove("test_main.$1");

	//hash
	var("xyz").hash(1000).outputl("hash(1000) of \"xyz\"=");
	//assert(var("xyz").hash(1000)==894);//wchar/wstring vars
	assert(var("xyz").hash(1000)==274);

	{	//null characters cannot be embedded in string constants in c/c++

		var data="\xFF\x30\x00\x32";
		assert(len(data) eq 2);
		//wont compile
		//data="\u0032";
		//cant put unicode in narrow character strings
		//data="\u0330";
	}

#if defined(WIN32) or defined(__APPLE__)
	var nbinarychars=256;
#else
	var nbinarychars=128;
#endif
	{	//test writing and reading bytes

		//make a string of first 256 (excluding 0 for the time being!)
		var data=chr(0);
		for (const var ii : range(1, nbinarychars-1)) {
			data^=chr(ii);
		}
		assert(len(data) eq nbinarychars);

		//check can write characters 1-255 out as bytes using C locale
		oswrite(data,"t_x.txt");
		assert(osfile("t_x.txt").f(1) eq nbinarychars);

		//check can read in bytes as characters using C locale
		var data2;
		osread(data2,"t_x.txt");
		assert(data2 eq data);
	}

	//test oswrite and osbread utf8
	//following code works on win32 and linux64 (ubuntu 10.04)
	//contents of tempfile should end up as unicode bytes ce b3 ce a3
	{

		//greek lowercase gamma and uppercase sigma
		var greek2="\u03B3\u03A3";//unicode code points
		assert(greek2.oconv("HEX")=="CEB3CEA3");//utf8 bytes
		//assert(greek2[1].seq()==947);
		//assert(greek2[2].seq()==931);

		//output as utf8 to t_temp5.txt
		var tempfilename5="t_temp5.txt";
		//greek2.outputl();
		assert(oswrite(greek2,tempfilename5,"utf8"));

		//open t_temp5.txt as utf8 for random access
		var tempfile;
		assert(osopen(tempfilename5,tempfile,"utf8"));
		//test reading from beyond end of file - returns ""
		//offset2 is BYTE OFFSET NOT CHARACTER OFFSET!!!
		var data,offset2;
		offset2=4;
		data.osbread(tempfile,offset2,2);
		assert(data eq "");

		//reading from middle of utf8 sequence -> invalid data TODO check valid UTF8
		offset2=3;
		data.osbread(tempfile,offset2,2);
		assert(data.oconv("HEX")=="A3");
		offset2=1;
		data.osbread(tempfile,offset2,2);
		assert(data.oconv("HEX")=="B3");
		offset2=2;
		data.osbread(tempfile,offset2,2);
		data.oconv("HEX").outputl("test reading from middle of utf8 byte sequence test=");
		//assert(data.osbread(tempfile,offset2=3,2) eq "");

		//test reading in C/binary mode (not UTF8)
		assert(osopen(tempfilename5,tempfile,"C"));
		//assert(data.osbread(tempfile,offset2=0,1) eq greek2[1]);
		//assert(data.osbread(tempfile,offset2=1,1) eq greek2[2]);
		//assert(data.osbread(tempfile,offset2=2,1) eq greek2[3]);
		//assert(data.osbread(tempfile,offset2=3,1) eq greek2[4]);
		offset2=0;
		data.osbread(tempfile,offset2,1);
		assert(data eq greek2[1]);
		offset2=1;
		data.osbread(tempfile,offset2,1);
		assert(data eq greek2[2]);
		offset2=2;
		data.osbread(tempfile,offset2,1);
		assert(data eq greek2[3]);
		offset2=3;
		data.osbread(tempfile,offset2,1);
		assert(data eq greek2[4]);

		//verify utf-8 bytes
		if (nbinarychars eq 256) {
			osread(data,tempfilename5,"C");
			assert(data eq "\u00ce\u00b3\u00ce\u00a3");
		}

		//check upper/lower case conversion on UTF8 Greek
		assert(lcase(GreekCapitalGamma)==GreekSmallGamma);
		assert(ucase(GreekSmallGamma)==GreekCapitalGamma);

		var greek_alphabet=" Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.";
		greek_alphabet.outputl();
		greek_alphabet.lcase().outputl();
		greek_alphabet.ucase().outputl();
		greek_alphabet.oconv("HEX").outputl();
		assert(greek_alphabet.lcase().ucase().lcase()==greek_alphabet.ucase().lcase());
		assert(greek_alphabet.oconv("HEX")=="20CE9120CEB12C20CE9220CEB22C20CE9320CEB32C20CE9420CEB42C20CE9520CEB52C20CE9620CEB62C20CE9720CEB72C20CE9820CEB82C20CE9920CEB92C20CE9A20CEBA2C20CE9B20CEBB2C20CE9C20CEBC2C20CE9D20CEBD2C20CE9E20CEBE2C20CE9F20CEBF2C20CEA020CF802C20CEA120CF812C20CEA320CF832FCF822C20CEA420CF842C20CEA520CF852C20CEA620CF862C20CEA720CF872C20CEA820CF882C20616E6420CEA920CF892E");

		printl("greek utf8 tested ok");
	}

	assert(oswrite("","t_temp1234.txt"));
	var offs=2;
	assert(osbwrite("78","t_temp1234.txt",offs));
//	abort("stopping");

	//SQL tracing
	//DBTRACE=true;

	//save original locale
	//setxlocale("C");
	var locale0=getxlocale().outputl("Original Locale=");

	//windows locales Windows XP and Windows Server 2003
	//http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
	var english_usuk="";
	var german_standard="";
	var greek_gr="";
	var turkish_tr="";
	var english_usuk_locale;
	var greek_gr_locale;
	var turkish_tr_locale;
	if (SLASH_IS_BACKSLASH) {
		english_usuk=1033;
		german_standard=1031;
		greek_gr=1032;
		turkish_tr=1055;
		//english_usuk_locale="English";
		greek_gr_locale="Greek";
		turkish_tr_locale="Turkish";
		english_usuk_locale="English";
		//greek_gr_locale=greek_gr;
	} else {
		english_usuk="en_US.utf8";
		german_standard="de_DE.utf8";
		greek_gr="el_GR.utf8";
		turkish_tr="tr_TR.utf8";
                //try mac versions
                //see locale -a for list
                if (not setxlocale(english_usuk))
			english_usuk="en_GB.utf8";
                if (not setxlocale(english_usuk)) {
                        english_usuk="en_US.UTF-8";
                        german_standard="de_DE.UTF-8";
                        greek_gr="el_GR.UTF-8";
                        turkish_tr="tr_TR.UTF-8";
                }
		english_usuk_locale=english_usuk;
		greek_gr_locale=greek_gr;
		turkish_tr_locale=turkish_tr;
	}

	//in English/US Locale
	//check ASCII upper/lower casing
//	setxlocale(english_usuk);
	assert(setxlocale(english_usuk));
	assert(ucase(LOWERCASE_) eq UPPERCASE_);
	assert(lcase(UPPERCASE_) eq LOWERCASE_);

	//in German/Standard Locale
	//check Eszet (like a Beta) uppercases to SS
	if (setxlocale(german_standard)) {
		assert(setxlocale(german_standard));
		GermanEszet.outputl("German Eszet:");
		ucase(GermanEszet).outputl("Uppercased German Eszet:");
		lcase(ucase(GermanEszet)).outputl("Lowercased German Eszet:");
		GermanEszet.oconv("HEX").outputl("German Eszet:");
		ucase(GermanEszet).oconv("HEX").outputl("Uppercased German Eszet:");
		//assert(ucase(lcase(GermanEszet))==GermanEszet);
		assert(ucase(GermanEszet) eq "SS");
	}
	//in Greek Locale
	//convert word ending in "capital sigma" lower cases to "lower final sigma"
	var hasgreeklocale=setxlocale(greek_gr);
	if (hasgreeklocale) {
		assert(setxlocale(greek_gr));
		Greek_sas       .outputl("Greek_sas=");
		Greek_SAS       .outputl("Greek_SAS=");
		ucase(Greek_sas).outputl("ucased   =");
		lcase(Greek_SAS).outputl("lcased   =");
		lcase(Greek_SAS).oconv("HEX").outputl();

		//ucase doesnt do languages TODO
		//assert(ucase(Greek_sas) eq Greek_SAS);
		//FAILS in Windows XPSP3UK and linux
		assert(ucase(Greek_sas) eq Greek_SAS);
	}
	//NB a codepage is a 256 x one byte map of characters selected from all unicode characters depending on locale

	//test windows codepages
	if (SLASH_IS_BACKSLASH) {
		//show where we are working
		TRACE(oscwd());
		var greektestfilename="t_greeksas.txt";
		//check CANNOT write greek unicode characters using French codepage
		assert(not Greek_sas.oswrite(greektestfilename,"French"));//CANNOT write
		//check can write greek unicode characters to Greek codepage
		assert(Greek_sas.oswrite(greektestfilename,"Greek"));
		//check 3 (wide) characters output as 3 bytes
		assert(osfile(greektestfilename).f(1) eq 3);
		//check can read greek wide unicode characters from greek codepage
		var rec2;
		assert(rec2.osread(greektestfilename,"Greek"));
		assert(rec2 eq Greek_sas);
		//check raw read as latin
		assert(rec2.osread(greektestfilename,"C"));
		var rec3="\xF3\xE1\xF2";
		assert(rec2 eq rec3);//greek code page characters

	}
	//in Turkish Locale
	//check Latin "I" lowercases to "turkish dotless i"
	//check Latin "i" uppercases to "turkish dotted I"
	//fails on Ubuntu 10.04
/* TODO language sensitive ucase
	if (setxlocale(turkish_tr)) {
		assert(setxlocale(turkish_tr));
		printl("Latin Capital I should lower case to dotless Turkish i:",lcase(LatinCapitalI));
		assert(lcase(TurkishCapitalDottedI) eq LatinSmallI);
		assert(ucase(TurkishSmallDotlessI) eq LatinCapitalI);
		if (SLASH_IS_BACKSLASH) {
			assert(lcase(LatinCapitalI) eq TurkishSmallDotlessI);
			assert(ucase(LatinSmallI)   eq TurkishCapitalDottedI);
		}
	}
*/
	//restore initial locale
	setxlocale(locale0);
	setxlocale(english_usuk);

	var tempfilename5;
	var record5;
	tempfilename5="t_temp7657.txt";
	assert(oswrite("",tempfilename5));
	assert(osremove(tempfilename5));

	//check we cannot write to a non-existent file
	//osremove(tempfilename5); //make sure the file doesnt exist
	assert(not osfile(tempfilename5) or osremove(tempfilename5));
	var offset=2;
	assert(not osbwrite("34",tempfilename5,offset));
	osclose(tempfilename5);

	//check we can osbwrite to an existent file beyond end of file
	//oswrite("",tempfilename5,"utf8");
	oswrite("",tempfilename5);
	assert(osopen(tempfilename5,tempfilename5));
	offset=2;
	assert(osbwrite("78",tempfilename5,offset));
	offset=2;
	var v78;
	v78.osbread(tempfilename5, offset, 2);
	assert(v78 eq "78");

	assert(osread(record5,tempfilename5));
	assert(record5.oconv("HEX2") eq "00003738");
	assert(osremove(tempfilename5));

	if (SLASH_IS_BACKSLASH) {
		//check cannot write non-codepage characters
		assert(oswrite("",tempfilename5));
		var offset=2;
		assert(not osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		//check can write codepage characters
		assert(osopen(tempfilename5,tempfilename5,"Greek"));
		offset=2;
		assert(osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		assert(osremove(tempfilename5));
	}

	//character replacement
	printl(oconv("ABc.123","MRN"));

	assert(oconv("ABc.123","MRN") eq "123");
	assert(oconv("ABc.123","MRA") eq "ABc");
	assert(oconv("ABc.123","MRB") eq "ABc123");
	assert(oconv("ABc.123","MR/N") eq "ABc.");
	assert(oconv("ABc.123","MR/A") eq ".123");
	assert(oconv("ABc.123","MR/B") eq ".");

	assert(oconv("ABc.123","MRL") eq "abc.123");
	assert(oconv("ABc.123","MRU") eq "ABC.123");

	//test unicode regular expressions

	//make some latin and greek upper and lower case letters and punctuation plus some digits.
	var unicode="";
	var expect;
	unicode^=GreekQuestionMark;
	unicode^=GreekCapitalGamma;
	unicode^=GreekSmallGamma;
	unicode^="ABc-123.456";//some LATIN characters and punctuation

	var status2 = oswrite( unicode, "t_GreekUTF-8File.txt", "utf8");

	var status1 = oswrite( unicode, "t_GreekLocalFile.txt");
	var status3 = oswrite( unicode, "t_GreekEnglFile.txt");

	//test swapping "letters" (i.e. alphabet) with "?"
	//We expect the question mark to remain as it is,
	// and the greek gammas and ABC to be replaced with latin question marks.
	expect=unicode;
	splicer(expect,2,2,"??");
	splicer(expect,4,3,"???");
	//expect.outputl();
	//expect.oconv("HEX4").outputl();
	//swap(unicode,"\\p{L}","?","ri").outputl();
	//swap(unicode,"\\p{L}","?","ri").oconv("HEX4").outputl();
	//p(L} is regular expression for Unicode Letter
	if (SLASH_IS_BACKSLASH)
		assert(regex_replace(unicode,"\\pL","?","ri") eq expect);
	//but what is its inverse?
	//assert(swap(unicode,"\\PL","?","ri") eq expect);

	//setxlocale(greek_gr);
	var punctuation=GreekQuestionMark;//(Punctuation)
	var uppercase=GreekCapitalGamma;//(Uppercase)
	var lowercase=GreekSmallGamma;//(Lowercase)
	var letters=lowercase^uppercase;
	var digits=ArabicIndicDigitZero;//(Decimal Digit)

	assert(oconv(punctuation,"MRA") eq "");//extract only alphabetic
	assert(oconv(punctuation,"MRN") eq "");//extract only numeric
	assert(oconv(punctuation,"MRB") eq "");//extract only alphanumeric
	assert(oconv(punctuation,"MR/A") eq punctuation);//extract non-alphabetic
	assert(oconv(punctuation,"MR/N") eq punctuation);//extract non-numeric
	assert(oconv(punctuation,"MR/B") eq punctuation);//extract non-alphanumeric

	assert(oconv(digits,"MRA") eq "");
	assert(oconv(digits,"MR/A") eq digits);
//#ifndef __APPLE__
	if (BOOST_REGEX) {
		assert(oconv(digits,"MRN") eq digits);
		assert(oconv(digits,"MRB") eq digits);
		assert(oconv(digits,"MR/N") eq "");
		assert(oconv(digits,"MR/B") eq "");
	}
//#endif
	assert(oconv("abc .DEF","MRU") eq "ABC .DEF");
	assert(oconv("abc .DEF","MRL") eq "abc .def");

	//uppercase/lowecase conversion only works for ascii at the moment
	//case conversion is perhaps generally done in order to do case insensitive
	//so as long as we provide a case insensitive comparison then the
	//pressure to do case conversion for all languages is less
	//assert(oconv(lowercase,"MCU") eq uppercase);
	//assert(oconv(uppercase,"MCL") eq lowercase);

	//printl(lowercase).;
	//printl(oconv(lowercase,"MRU"));
	//printl(uppercase);
	//printl(oconv(uppercase,"MRL"));
	//assert(oconv(lowercase,"MRU") eq uppercase);
	//assert(oconv(uppercase,"MRL") eq lowercase);

	//no change
	assert(oconv(uppercase,"MRU") eq uppercase);
	assert(oconv(lowercase,"MRL") eq lowercase);

	TRACE(letters)
	if (BOOST_REGEX) {
		oconv(letters,"MRA").outputl("Expected:"^letters^" Actual:");
		oconv(letters,"MRN").outputl("Expected:\"\" Actual:");
		oconv(letters,"MRB").outputl("Expected:"^letters^" Actual:");
		assert(oconv(letters,"MRA") eq letters);
		assert(oconv(letters,"MRN") eq "");
		assert(oconv(letters,"MRB") eq letters);
		assert(oconv(letters,"MR/A") eq "");
		assert(oconv(letters,"MR/N") eq letters);
		assert(oconv(letters,"MR/B") eq "");
	}

	//lambda function
	auto testinvert=[](var cc)
	{
		var inverted=cc.invert();
		var invertedtwice=invert(inverted);

		/*
		cc                        .outputl("original      =");
		inverted                  .outputl("inverted      =");
		invertedtwice             .outputl("inverted twice=");

		cc.oconv("HEX")           .outputl("original hex  =");
		inverted.oconv("HEX")     .outputl("inverted hex  =");
		invertedtwice.oconv("HEX").outputl("invertedx2 hex=");

		cc.textseq()              .outputl("original seq  =");
		inverted.textseq()        .outputl("inverted seq  =");
		invertedtwice.textseq()   .outputl("invertedx2 seq=");

		cc.len()                  .outputl("original len  =");
		inverted.len()            .outputl("inverted len  =");
		invertedtwice.len()       .outputl("invertedx2 len=");
		*/
		assert(cc ne inverted);
		assert(cc == invertedtwice);

		assert(cc.oconv("HEX") == invertedtwice.oconv("HEX"));
	};

	//check invert works and is reversible for the first 65535 unicode characters
	testinvert("␚ ␛ ␜ ␝ ␞ ␟");

	//check invert is reversible for all bytes (only ASCII bytes are inverted)
	for (const var ii : range(0, 0x1FFFF)) {
		var cc=textchr(ii);
		if (cc.len())
			testinvert(cc);
		else if (ii < 0xD800 or ii > 0xDFFF )
			printl(ii);
	}

    assert(invert("ÏÎÍÌËÊÉÈÇÆ¾½¼»º¹¸·¶µ´³²±°¯®­¬«ª©¨§¦¥") eq "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    assert(invert("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") eq "ÏÎÍÌËÊÉÈÇÆ¾½¼»º¹¸·¶µ´³²±°¯®­¬«ª©¨§¦¥");

	//check unicode is invalid from 0xD800-0xDFF (UTF16 encoding) and 0x110000 onwards
	assert(textchr(0xD7FF)!="");
	assert(textchr(0xD800)=="");
	assert(textchr(0xDFFF)=="");
	assert(textchr(0xE000)!="");
	assert(textchr(0x10FFFF)!="");
	assert(textchr(0x110000)=="");

	var cmd = "c:\\xwindowspath_test_main1_\\to\\xyz.exe arg1 arg2";
	//assert(var(to_oscmd_string(cmd) == cmd.convert("\\",OSSLASH));
	// No conversion at the moment
	//assert(to_oscmd_string(cmd) == cmd);
	assert(not osshell(cmd));

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
function test_codepage(in codepage, in lang) {
	printl("---------- " ^ lang ^ " " ^ codepage ^ " ----------");

	var v256="";
	for (int ii = 0; ii <= 255; ++ii)
		v256 ^= chr(ii);
	oswrite(v256,"t_codep.bin");
	assert(osfile("t_codep.bin").f(1)==256);

	//convert to utf8
	var as_utf8a=v256.from_codepage(codepage);
	assert(as_utf8a != v256);
	assert(as_utf8a.length() > v256.length());

	//convert back to codepage
	var as_cp=as_utf8a.to_codepage(codepage);
	printl(as_cp.length());
	//assert(as_cp.length()==256);

	//convert to utf8 again
	var as_utf8b=as_cp.from_codepage(codepage);

	//oswrite(as_utf8a, "t_utf8a." ^ lang);
	//oswrite(as_cp,   "t_codep." ^ lang);
	//oswrite(as_utf8b,"t_utf8b." ^ lang);

	//check loop back
	//(only if loop back does produced 256 bytes)
	printl(as_utf8a.substr(32));
	printl("round trip " ^ var(as_cp == v256));
	if (as_cp.length()==256)
		assert(as_cp == v256);

	//check double trip
	printl("double trip "  ^var (as_utf8a == as_utf8b));
	printl(as_utf8b.substr(32));
	assert(as_utf8a == as_utf8b);

	return 0;
}

programexit()
