#include <cassert>
#include <sstream>

#include <exodus/program.h>

// TESTING

// if it runs ok with no static assertion failures
// 1. compile test_main && test_main

// then test no changes in output with the following
// 2. compile test_main && test_main > t_test_main 2> /dev/null && diff test_main.out t_test_main

// DEVELOPING

// any variable output (including dates/times/random numbers) must use errput (not print or output)
//
// if you add to or change the output in any way then update the expected output file
//
// test_main 1> test_main.out

programinit()

function main()
{

	//output time and date to stderr
	errputl("Using Exodus library version:" ^ var().version());
	date().oconv("D").errputl("Date is:");
	time().oconv("MTS").errputl("Time is:");

	//14:30:46 04 JAN 2021
	assert(timedate().match("\\d{2}:\\d{2}:\\d{2} \\d{2} [A-Z]{3} \\d{4}"));

	printl("----------------------------------------------");


    assert("abc"_var == "abc");
    assert(12345_var == 12345);
    assert(1234.567_var == 1234.567);

    var v=123.456;
    assert(int(v) != v);
    assert(int(v) == 123);
    assert(double(v) == v);
    assert(double(v) == 123.456);
    assert(floor(v) != v);
    assert(floor(v) == int(v));

    v="123.456";
    assert(int(v) != v);
    assert(int(v) == 123);
    assert(double(v) == v);
    assert(double(v) == 123.456);
    assert(floor(v) != v);
    assert(floor(v) == int(v));

    v=123456;
    v=v/1000;
    assert(int(v) != v);
    assert(int(v) == 123);
    assert(double(v) == v);
    assert(double(v) == 123.456);
    assert(floor(v) != v);
    assert(floor(v) == int(v));

	double d=1234.567;
	assert(var(d)=="1234.567");
	assert(var(-d)=="-1234.567");
	assert(int(var(d))==1234);
	assert(int(var(-d))==-1235);

	//test accessing var as a range of fields separated by FM
	{
		var fields = "a1" _FM_ "b2" _FM_ "c3";
		var fieldn=0;
		for (var field : fields) {
			fieldn++;
			if (fieldn==1)
				assert(field=="a1");
			else if (fieldn==2)
				assert(field=="b2");
			if (fieldn==3)
				assert(field=="c3");
		}
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
	"Western	: 0 1 2 3 4 5 6 7 8 9  \n"
	"Arabic		: ٠ ١ ٢ ٣ ٤ ٥ ٦ ٧ ٨ ٩  \n"
	"Devanagari	: ० १ २ ३ ४ ५ ६ ७ ८ ९  \n"
	"Gujarati 	: ૦ ૧ ૨ ૩ ૪ ૫ ૬ ૭ ૮ ૯  \n"
	"Punjabi 	: ੦ ੧ ੨ ੩ ੪ ੫ ੬ ੭ ੮ ੯  \n"
	"Bengali 	: ০ ১ ২ ৩ ৪ ৫ ৬ ৭ ৮ ৯  \n"
	"Odia 		: ୦ ୧ ୨ ୩ ୪ ୫ ୬ ୭ ୮ ୯  \n"
	"Telugu 	: ౦ ౧ ౨ ౩ ౪ ౫ ౬ ౭ ౮ ౯  \n"
	"Kannada 	: ೦ ೧ ೨ ೩ ೪ ೫ ೬ ೭ ೮ ೯  \n"
	"Tamil 		: ௦ ௧ ௨ ௩ ௪ ௫ ௬ ௭ ௮ ௯  \n"
	"Malayalam 	: ൦ ൧ ൨ ൩ ൪ ൫ ൬ ൭ ൮ ൯  \n";

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

	printl("sizeof");
	printl("int:      ",(int)sizeof(int));
	printl("long:     ",(int)sizeof(long));
	printl("float:    ",(int)sizeof(float));
	printl();
	printl("string:   ",(int)sizeof(std::string));
	printl("long long:",(int)sizeof(long long));
	printl("double:   ",(int)sizeof(double));
	printl("char:     ",(int)sizeof(char));
	printl("var:      ",(int)sizeof(var));

	auto size = sizeof(std::string);
	auto capacity = std::string().capacity();
	auto small = std::string(capacity, '*');
	auto big = std::string(capacity + 1, '*');

 	std::cout << "\nstd:string implementation\n";
	std::cout << "sizeof  : " << size << std::endl;
	std::cout << "Capacity: " << capacity << std::endl;
	std::cout << "Small   : " << small.capacity() << std::endl;
	std::cout << "Big     : " << big.capacity() << std::endl;

	//test tcase and fcase
	assert(var("top of the world").tcase()=="Top Of The World");
	printl(var("top of the world").tcase().fcase());

	var value=R"('1')";
	assert(value.swap("'",R"(\')")=="\\'1\\'");

	//check conversion of unprintable field marks to unusual ASCII characters
	//except TM which is ESC
  	std::ostringstream stringstr;
	stringstr << var(RM ^ FM ^ VM ^ SM ^ TM ^ STM);
	std::cout << stringstr.str() << std::endl;
	//assert(var(stringstr.str()) == "~^]\\[|");
	assert(var(stringstr.str()) == "~^]\\" "\x1B" "|");

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
	assert(decomp_a.normalize().oconv("HEX")=="C3A1");

	//test read/write/delete normalization
	//exodus will normalize everything being written to the database because otherwise kind of duplicate keys can exist because
	//postgres says the following is false ... because one is a single unicode code unit and the other is two combined into one for presentation
	//SELECT 'á' = 'á';

	//check on invalid utf-8

	//should except on a abaft about above afore after along amid among an apud as aside at atop below but by circa down for from given in into lest like mid midst minus near next of off on onto out ov$
	var utftest,utftest2;
	var utftestfilename="utf-8-test.txt";

	//multivalued conversions performed one after the other
	//assert(oconv(1234.567,"MD20P" _VM_ "[NUMBER]" _VM_ "[TAGHTML,TD]")=="<TD>1,234.57</TD>");
	printl(oconv(1234.567,"MD20P" _VM_ "[NUMBER]"));
	assert(oconv(1234.567,"MD20P" _VM_ "[NUMBER]")=="1,234.57");

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
	assert(len(utftest)==osfile(utftestfilename).a(1));

	//check invalid utf8 has no change oswrite/osread round trip
	oswrite(utftest,"t_"^utftestfilename);
	osread(utftest2,"t_"^utftestfilename);
	assert(utftest2=utftest);

	//check invalid utf has no change on ucase/lcase round trip
	assert(lcase(ucase(utftest))==lcase(utftest));
	assert(ucase(lcase(utftest))==ucase(utftest));

	//check what normalize() does on invalid utf
	var fixed_utftest=utftest.normalize();
	assert(fixed_utftest.len()==22400);

	//match returning what it finds

	//groups
	var csvline1="abcdef abcdef";
	var csvre1="(bc).(ef)";
	assert(match(csvline1,csvre1).convert(FM^VM,"^]")=="bcdef]bc]ef^bcdef]bc]ef");

	//no groups
	var csvline2="abcdef abcdef";
	var csvre2="bc.ef";
	assert(match(csvline2,csvre2).convert(FM^VM,"^]")=="bcdef^bcdef");

	//not asserted but a complicated CSV match
	var csvline=R"(123,2.99,AMO024,Title,"Description, more info",,123987564)";
	var csvre=R"tag((?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$))tag";
	assert(match(csvline,csvre).convert(FM^VM,"^]")==R"raw(123]]123^,2.99]]2.99^,AMO024]]AMO024^,Title]]Title^,"Description, more info"]"]Description, more info^,^,123987564]]123987564)raw");

	//unicode case insensitive finding
	assert(match("αβγδεΑΒΓΔΕ","(Α).(γδ)","i").convert(FM^VM,"^]")=="αβγδ]α]γδ^ΑΒΓΔ]Α]ΓΔ");
	//unicode case sensitive NOT finding
	assert(match("αβγδεΑΒΓΔΕ","(Α).(γδ)","").convert(FM^VM,"^]")=="");

	//replacing unicode style numbers characters using javascript style regex
	assert(var("Ⅻ").replace(R"(\p{Number})","yes")=="yes");
	assert(var("⅝").replace(R"(\p{Number})","yes")=="yes");

	//test glob matching using * ? eg *.* and *.??? etc
        assert(var("test.htm").match("*.*","w")		=="test.htm");
        assert(var("test.htm").match("*","w")		=="test.htm");
        assert(var("test.htm").match(".*","w")		=="");
        assert(var("test.htm").match(".","w")		=="");
        assert(var("test.htm").match("*.","w")		=="");
        assert(var("test.htm").match("*.htm","w")	=="test.htm");
        assert(var("test.html").match("*.htm","w")	=="");
        assert(var("test.htm").match("t*.???","w")	="test.htm");
        assert(var("test.htm").match("t.???","w")	=="");
        assert(var("test.htm").match("x.???","w")	=="");
        assert(var("testx.htm").match("*x.???","w")	=="testx.htm");
        assert(var("test.html").match("t*.???","w")	=="");
        assert(var("test.html").match("*t?h*","w")	=="test.html");

	//test regular expression
	//four digits followed by dash or space) three times ... followed by four digits
	var regex1="(\\d{4}[- ]){3}\\d{4}";
	assert(var("1247-1234-1234-1234").match(regex1,"r").convert(FM^VM,"^]")=="1247-1234-1234-1234]1234-");
	assert(var("1247.1234-1234-1234").match(regex1,"r")=="");

	printl(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF"));
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF")=="文字- Kanji 漢字- Hanja 漢字(UTF");
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match(".*文字.*漢 字\\(UTF-8\\)")=="");

	//multibyte sep index
	var greek5x2="αβγδεαβγδε";
	assert(greek5x2.index("β")==3);
	assert(greek5x2.index("β",2)==13);

	//multibyte sep field
	var greek5x2f2=field(greek5x2,"β",2);
	assert(greek5x2f2=="γδεα");
	assert(greek5x2f2.length()==8);
	assert(greek5x2f2.oconv("HEX")=="CEB3CEB4CEB5CEB1");

	//multibyte sep fieldstore
	var greekstr2=fieldstore(greek5x2,"β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);

	greekstr2=fieldstore(greek5x2,"β",2,1,"1β2β3");
	assert(greekstr2=="αβ1βγδε");

	greekstr2=fieldstore(greek5x2,"β",2,-1,"1β2β3");
	assert(greekstr2=="αβ1β2β3βγδε");

	var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	assert(fieldstore(greek5x4,"β",2,3,"ζ")=="αβζβββγδε");
	assert(fieldstore(greek5x4,"β",2,-3,"ζ")=="αβζβγδε");

	//text four byte utf8
	var chinesechar=textchr(171416);
	assert(chinesechar=="𩶘");
	printl(chinesechar);
	assert(textseq(chinesechar^"abcd")=171416);
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

	var sort="a" _FM_ "b" _FM_ "d";
	var sortn;
	sort.locatebyusing("AL",FM,"b",sortn);
	assert(sortn==2);
	sort.locatebyusing("AL",FM,"c",sortn);
	assert(sortn==3);

	sort.locateusing(FM,"b",sortn);
	assert(sortn==2);
	sort.locateusing(FM,"c",sortn);
	assert(sortn==4);

	//exodus comparisions and conversion to book ignores numbers less than 1e-13
	//pick/arev bool and equality checks on doubles ignore less than 0.0001
	//but comparison operators work exactly (to max binary precision?)
	var smallestno = 1e-10;
	var toosmallno = smallestno/10;
	assert(smallestno);
	assert(smallestno>0);
	assert(smallestno!=0);
	assert(!toosmallno);
	assert(toosmallno==0);
	assert(!(toosmallno>0));

	assert(smallestno == SMALLEST_NUMBER);
	assert(var(1e-11).toString() == "0.00000000001");
	assert(var(1e-12).toString() == "0.000000000001");
	assert(var(1e-13).toString() == "0.0000000000001");
	assert(var(1e-14).toString() == "0.0");

	var d1=1.2;
	d1++;
	assert(d1==2.2);
	++d1;
	assert(d1==3.2);

	var i1=1;
	i1++;
	assert(i1==2.0);
	++i1;
	assert(i1==3.0);

	d1=1.2;
	d1+=1;
	assert(d1==2.2);
	d1+=1.1;
	assert(d1==3.3);

	//exodus accidentally equipped std:string with most of the attributes of a var (this could be viewed as a good thing)
	// the feature was removed by using "hidden friends" ie placing all global friend operator functions inside the body of the var class
	//
	//assert(d1.toString()==3.3);//strange that this compiles and works (no longer after switching to "hidden friends" in exodus mv.h
	//printl(std::string("1.1")+3.3);//strange this compiles! we have equipped std:string with the ability to convert to numeric for addition with a double!

	i1=1;
	i1+=1;
	assert(i1==2.0);
	i1+=1.1;
	assert(i1==3.1);

	printl(1e-14);
	printl(var(0));
	assert(1e-14==var(0));

	printl();
	printl("=== print ===");

	printl("l");
	printl();
	printl("l");
	printt();
	printl("l");
	printl("l");

	print("p");
	print("pl");
	printl();
	printl("l");
	printt("t");
	printl("l");

	print("0", "p");
	print("0", "pl");
	printl();
	printl("0", "l");
	printt("0", "t");
	printl("0", "l");

	print("0", "1", "2", "p");
	print("0", "1", "2", "pl");
	printl();
	printl("0", "1", "2", "l");
	printt("0", "1", "2", "t");
	printl("0", "1", "2", "l");

	print<'-'>("0", "1", "2", "p");
	print<'-'>("0", "1", "2", "pl");
	printl();
	printl<'-'>("0", "1", "2", "l");
	printt<'-'>("0", "1", "2", "t");
	printl<'-'>("0", "1", "2", "l");

	print(RM, FM, VM, SM, TM, STM);
	printl(RM, FM, VM, SM, TM, STM);

	{
		static char const sep[] = ", ";
		print<sep>("0", "1", "2", "p");
		print<sep>("0", "1", "2", "pl");
		printl();
		printl<sep>("0", "1", "2", "l");
		printt<sep>("0", "1", "2", "t");
		printl<sep>("0", "1", "2", "l");
	}

	output();
	output(0);
	output("0", "1", "2", "p");
	output("0", "1", "2", "pl");
	output(RM, FM, VM, SM, TM, STM);
	outputl();
	outputl(0);
	outputl("0", "1", "2", "l");
	outputl("0", "1", "2", "l");
	outputl(RM, FM, VM, SM, TM, STM);

	printl();
	printl("=== errput ===");

	//errput();
	errputl("l");
	errputl();
	errputl("l");
	errputl("l");
	errputl("l");

	errput("p");
	errput("pl");
	errputl();
	errputl("l");
	errputl("l");

	errput("0", "p");
	errput("0", "pl");
	errputl();
	errputl("0", "l");
	errputl("0", "l");

	errput("0", "1", "2", "p");
	errput("0", "1", "2", "pl");
	errputl();
	errputl("0", "1", "2", "l");
	errputl("0", "1", "2", "l");

	printl();
	printl("=== logput ===");

	logputl("l");
	logputl();
	logputl("l");
	logputl("l");
	logputl("l");

	logput("p");
	logput("pl");
	logputl();
	logputl("l");
	logputl("l");

	logput("0", "p");
	logput("0", "pl");
	logputl();
	logputl("0", "l");
	logputl("0", "l");

	logput("0", "1", "2", "p");
	logput("0", "1", "2", "pl");
	logputl();
	logputl("0", "1", "2", "l");
	logputl("0", "1", "2", "l");

	assert(crop(VM ^ FM) eq "");
	assert(crop("xxx" ^ VM ^ FM) eq "xxx");
	assert(crop("aaa" ^ VM ^ FM ^ "bbb") eq ("aaa" ^ FM ^ "bbb"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ "ddd") eq ("aaa" ^ FM ^ "bbb" ^ FM ^ FM ^ "ddd"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ RM ^ "ddd") eq ("aaa" ^ FM ^ "bbb" ^ RM ^ "ddd"));

	assert(crop("aa" _VM_ _FM_ "bb" _FM_)=="aa" _FM_ "bb");
	assert(crop("aa" _SM_ _VM_ _FM_ "bb" _FM_)=="aa" _FM_ "bb");
	assert(crop(_FM_ "aa" _VM_ _FM_ "bb" _FM_)==_FM_ "aa" _FM_ "bb");
	assert(crop(_FM_ "aa" _VM_ _FM_ "bb" _FM_ _VM_)==_FM_ "aa" _FM_ "bb");
	assert(crop(_FM_ "aa" _VM_ _FM_ _VM_ "bb" _FM_ _VM_)==_FM_ "aa" _FM_ _VM_ "bb");
	assert(crop(_FM_ "aa" _VM_ _FM_ "bb" _FM_ _RM_)==_FM_ "aa" _FM_ "bb");
	assert(crop(_FM_ _RM_ "aa" _VM_ _FM_ "bb" _FM_ _RM_)==_RM_ "aa" _FM_ "bb");

	var locii;
	var locxx="1 2 3 10 20 30 100 200 300";
	var locxd="300 200 100 30 20 10 3 2 1";
	locxx.converter(" ",VM);
	locxd.converter(" ",VM);
	var ar="AR";
	var locsep=",";

	assert(locxx.locate(10)==1);

	locxx.locate(30,locii);
	assert(locii==6);

	locxx.locate(31,locii);
	assert(locii==10);

	locxx.locate(30,locii,1);
	assert(locii==6);

	locxx.locate(31,locii,1);
	assert(locii==10);

	locxx.locate(31,locii,2);
	assert(locii==1);

	locxx.locateby(ar,21,locii);
	assert(locii==6);

	locxx.locateby("AR",21,locii);
	assert(locii==6);

	locxx.locateby("AL",21,locii);
	assert(locii==3);

	locxd.locateby("DR",21,locii);
	assert(locii==5);

	locxd.locateby("DL",21,locii);
	assert(locii==2);

	locxx.converter(VM,",");
	locxd.converter(VM,",");

	assert(locxx.locateusing(",",30)==1);
	assert(locxx.locateusing(",",31)==0);

	locxx.locateusing(",",30,locii);
	assert(locii==6);

	locxx.locateusing(locsep,30,locii);
	assert(locii==6);


	locxx.converter(",",VM);
	locxd.converter(",",VM);

	assert(locate(10,locxx)==1);

	locate(30,locxx,locii);
	assert(locii==6);

	locate(31,locxx,locii);
	assert(locii==10);

	locate(30,locxx,locii,1);
	assert(locii==6);

	locate(31,locxx,locii,1);
	assert(locii==10);

	locate(31,locxx,locii,2);
	assert(locii==1);

	locateby(ar,21,locxx,locii);
	assert(locii==6);

	locateby("AR",21,locxx,locii);
	assert(locii==6);

	locateby("AL",21,locxx,locii);
	assert(locii==3);

	locateby("DR",21,locxd,locii);
	assert(locii==5);

	locateby("DL",21,locxd,locii);
	assert(locii==2);

	locxx.converter(VM,",");
	locxd.converter(VM,",");

	assert(locateusing(",",30,locxx)==1);
	assert(locateusing(",",31,locxx)==0);

	locateusing(",",30,locxx,locii);
	assert(locii==6);

	locateusing(locsep,30,locxx,locii);
	assert(locii==6);


	var aaa="11"^VM^VM^"13"^VM^FM^"21";
	var bbb="1011"^VM^VM^"1013";

	assert(aaa.mv(":",bbb).convert(_VM_ _FM_, "]^")=="111011]]131013]^21");
	assert(aaa.mv("+",bbb).convert(_VM_ _FM_, "]^")=="1022]0]1026]0^21");
	assert(aaa.mv("-",bbb).convert(_VM_ _FM_, "]^")=="-1000]0]-1000]0^21");
	assert(aaa.mv("*",bbb).convert(_VM_ _FM_, "]^")=="11121]0]13169]0^0");

	assert(bbb.mv(":",aaa).convert(_VM_ _FM_, "]^")=="101111]]101313]^21");
	assert(bbb.mv("+",aaa).convert(_VM_ _FM_, "]^")=="1022]0]1026]0^21");
	assert(bbb.mv("-",aaa).convert(_VM_ _FM_, "]^")=="1000]0]1000]0^-21");
	assert(bbb.mv("*",aaa).convert(_VM_ _FM_, "]^")=="11121]0]13169]0^0");

	aaa="11"  ^VM^VM^"13"  ^VM^FM^"21";
	bbb="1011"^VM^"0"^VM^"1013"^VM^FM^"2011";
	printl(aaa.mv("/",bbb).oconv("MD90P").convert(_VM_ _FM_, "]^"));
	assert(aaa.mv("/",bbb).oconv("MD90P").convert(_VM_ _FM_, "]^")=="0.010880317]0.000000000]0.012833169]0.000000000^0.010442566");

	aaa=""^VM^"" ^VM^"0"^VM^"0"^VM;
	bbb=""^VM^"0"^VM^""^VM^"0"^VM^""^VM^"0"^VM^""^VM^"0";
	assert(aaa.mv("/",bbb).convert(_VM_ _FM_, "]^")=="0]0]0]0]0]0]0]0");

	//testing .mv(+ - * / :)
	var m1="1" _VM_ "2" _VM_ _VM_ "4";
	var m2="100" _VM_ "200" _VM_ "300"; 

	m1.convert(VM,"]").outputl("m1=");
	m2.convert(VM,"]").outputl("m2=");

	m1.mv("+",m2).convert(VM,"]").outputl("xxxx=");
	assert(m1.mv("+",m2).convert(VM,"]")=="101]202]300]4");
	assert(m1.mv("-",m2).convert(VM,"]")=="-99]-198]-300]4");
	assert(m1.mv("*",m2).convert(VM,"]")=="100]400]0]0");
	assert(m1.mv(":",m2).convert(VM,"]")=="1100]2200]300]4");

	printl();
	m2.r(1,4,400);
	m2.convert(VM,"]").outputl("m2=");
	printl(m1.mv("/",m2).convert(VM,"]"));
	printl("should be \"0.01]0.01]0]0.01\"");
	assert(m1.mv("/",m2).convert(VM,"]")=="0.01]0.01]0]0.01");

	//testing inserter
	var t1="aa";
	assert(t1.inserter(-1,"xyz").convert(FM^VM,"^]")=="aa^xyz");
	t1="aa";
	assert(t1.inserter(0,"xyz").convert(FM^VM,"^]")=="xyz^aa");
	t1="aa";
	assert(t1.inserter(1,"xyz").convert(FM^VM,"^]")=="xyz^aa");
	t1="aa";
	assert(t1.inserter(2,"xyz").convert(FM^VM,"^]")=="aa^xyz");
	t1="aa";
	assert(t1.inserter(3,"xyz").convert(FM^VM,"^]")=="aa^^xyz");
	t1="aa";
	assert(t1.inserter(1,1,"xyz").convert(FM^VM,"^]")=="xyz]aa");
	t1="aa";
	assert(t1.inserter(2,1,"xyz").convert(FM^VM,"^]")=="aa^xyz");
	t1="aa";
	assert(t1.inserter(2,2,"xyz").convert(FM^VM,"^]")=="aa^]xyz");

	t1="";
	assert(t1.inserter(-1,"xyz").convert(FM^VM,"^]")=="xyz");
	t1="";
	assert(t1.inserter(0,"xyz").convert(FM^VM,"^]")=="xyz");
	t1="";
	assert(t1.inserter(1,"xyz").convert(FM^VM,"^]")=="xyz");
	t1="";
	assert(t1.inserter(2,"xyz").convert(FM^VM,"^]")=="^xyz");
	t1="";
	assert(t1.inserter(3,"xyz").convert(FM^VM,"^]")=="^^xyz");
	t1="";
	assert(t1.inserter(1,1,"xyz").convert(FM^VM,"^]")=="xyz");
	t1="";
	assert(t1.inserter(2,1,"xyz").convert(FM^VM,"^]")=="^xyz");
	t1="";
	assert(t1.inserter(2,2,"xyz").convert(FM^VM,"^]")=="^]xyz");
	t1="";

	var errmsg;
	//if (not createdb("steve",errmsg))
	//	errmsg.outputl();
	//if (not deletedb("steve",errmsg))
	//	errmsg.outputl();

	var testfilename="t_vm.txt";
	var charout=GreekSmallGamma;//VM;
	assert(oswrite(charout,testfilename));
	assert(osfile(testfilename).a(1)==2);
	var offsetx=0;
	var testfilex;
	assert(osopen(testfilename,testfilex));
	//HORRIBLE HACK to bypass some weird failure on github actions testing
	if (unassigned(testfilex))
		testfilex=testfilename;
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
	testdata.osbread(testfilex,offsetx=0,2);
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

//	assert(testfilename.osfile().a(1) eq 2);
	var charin;
//fails on ubuntu 13.10x64
	assert(osread(charin,testfilename));
	//assert(charin eq charout);
	assert(osdelete(testfilename));


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
        oswrite(cp_allo1,"t_cp_allo.txt");
        assert(cp_allo1.oconv("HEX").outputl("cp_allo1=")=="B0DBDBDE");

        //hexdump -C t_cp_allo.txt
        //00000000  b0 db db de
        //00000004

        //check that we can read binary back in unconverted
        var cp_allo2=osread("t_cp_allo.txt");
        assert(cp_allo2.oconv("HEX").outputl("cp_allo2=")=="B0DBDBDE");

        //read in from binary cyrillic codepage text converting to utf8
        var utf8_allo3;
        if (not utf8_allo3.osread("t_cp_allo.txt","ISO-8859-5"))
                stop("cant read t_cp_allo.txt with cp ISO-8859-5");
        assert(utf8_allo3.length()==8);
        assert(utf8_allo3.oconv("HEX")=="D090D0BBD0BBD0BE");
        assert(utf8_allo3=="Алло");

        //output utf8 convering to ISO-8859-5
        oswrite(utf8_allo3,"t_cp_allo4.txt","ISO-8859-5");
/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump cp_allo4.txt -C
00000000  b0 db db de                                       |....|
00000004
*/
        //read back in binary to check that out output did convert from utf to cyrillic
        var cp_allo5;
        osread(cp_allo5,"t_cp_allo4.txt");
        assert(cp_allo5==cp_allo1);

/*
root@exodus:~/exodus/exodus/libexodus/exodus# hexdump t_utf8_allo4.txt -C
00000000  d0 90 d0 bb d0 bb d0 be                           |........|
00000008
*/

	var lbvn;
	//no fieldno/value given means using character VM
	assert(var("1" _VM_ "10" _VM_ "2" _VM_ "B").locateby("AL","A",lbvn)||lbvn==4);
	//fieldno given means search in that field using character VM
	assert(var("1" _VM_ "10" _VM_ "2" _VM_ "B").locateby("AL","A",lbvn,1)||lbvn==4);
	//fieldno given and =0 means search whole string using character FM
	assert(var("1" _FM_ "10" _FM_ "2" _FM_ "B").locateby("AL","A",lbvn,0)||lbvn==4);

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
        for (int ii=0;ii<65536;ii++)
                tx^=chr(ii);

        //output to binary - check roundtrip
        oswrite(tx,"test_main.$1");
        osread(tx2,"test_main.$1");
        assert(tx2==tx);

        //read as codepage
        //oswrite(tx,"test_main.$1","ISO-8859-5");
        osread(tx2,"test_main.$1","ISO-8859-5");
        //assert(tx2==tx);

	osdelete("test_main.$1");

	//hash
	var("xyz").hash(1000).outputl("hash(1000) of \"xyz\"=");
	//assert(var("xyz").hash(1000)==894);//wchar/wstring vars
	assert(var("xyz").hash(1000)==274);

	//test regex group and use group in replace
	//replace char+space with x+char+dash
	//TODO add g option
        var text="what a lot of money";
        assert(text.replace("(.) ","x$1-")=="whaxt-xa-loxt-oxf-money");

	//simple test of regex and case insensitive regex swap (commonly known as replace)
	assert(replace("abcd","b.","xyz").outputl() eq "axyzd");//right case to convert
	assert(replace("abc","B.","xyz").outputl() eq "abc"); //wrong case to convert
	assert(replace("abcd","B.","xyz","i").outputl() eq "axyzd");//case insensitive converts
	assert(replace("abc","b.","xyz","l").outputl() eq "abc");//literal wont convert

	//simple test of case sensitive/insensitive swap
	assert(swap("abc","b","xyz").outputl() eq "axyzc");//will convert right case
	assert(swap("abc","B","xyz").outputl() eq "abc");//wont convert wrong case
	assert(replace("abc","B","xyz","i").outputl() eq "axyzc");//will convert case insensitive
	assert(replace("ab*c","B*","xyz","il").outputl() eq "axyzc");//will convert case insensitive but not regex

	assert(swap("abababab","ab","x").outputl() eq "xxxx");
	assert(replace("abababab","ab","x").outputl() eq "xxxx");//regex
	assert(replace("abababab","a.","xy").outputl() eq "xyxyxyxy");//regex

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
		for (var ii=1;ii<nbinarychars;++ii)
			data^=chr(ii);
		assert(len(data) eq nbinarychars);

		//check can write characters 1-255 out as bytes using C locale
		oswrite(data,"t_x.txt");
		assert(osfile("t_x.txt")(1) eq nbinarychars);

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
		//HORRIBLE HACK TO BYPASS weird problem on github actions testing
		if (unassigned(tempfile))
			tempfile=tempfilename5;
		//test reading from beyond end of file - returns ""
		//offset2 is BYTE OFFSET NOT CHARACTER OFFSET!!!
		var data,offset2;
		data.osbread(tempfile,offset2=4,2);
		assert(data eq "");

		//reading from middle of utf8 sequence -> invalid data TODO check valid UTF8
		data.osbread(tempfile,offset2=3,2);
		assert(data.oconv("HEX")=="A3");
		data.osbread(tempfile,offset2=1,2);
		assert(data.oconv("HEX")=="B3");
		data.osbread(tempfile,offset2=2,2);
		data.oconv("HEX").outputl("test reading from middle of utf8 byte sequence test=");
		//assert(data.osbread(tempfile,offset2=3,2) eq "");

		//test reading in C/binary mode (not UTF8)
		assert(osopen(tempfilename5,tempfile,"C"));
		//HORRIBLE HACK TO BYPASS weird problem on github actions testing
		if (unassigned(tempfile))
			tempfile=tempfilename5;
		//assert(data.osbread(tempfile,offset2=0,1) eq greek2[1]);
		//assert(data.osbread(tempfile,offset2=1,1) eq greek2[2]);
		//assert(data.osbread(tempfile,offset2=2,1) eq greek2[3]);
		//assert(data.osbread(tempfile,offset2=3,1) eq greek2[4]);
		data.osbread(tempfile,offset2=0,1);
		assert(data eq greek2[1]);
		data.osbread(tempfile,offset2=1,1);
		assert(data eq greek2[2]);
		data.osbread(tempfile,offset2=2,1);
		assert(data eq greek2[3]);
		data.osbread(tempfile,offset2=3,1);
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

	printl("test_main says 'Hello World!'");
	//assert(setxlocale("fr_FR.utf8"));
	//assert(setxlocale(1036));
	var xx3="1234.5678";
	assert(xx3+1==1235.5678);

	assert(oconv("","MD20P")=="");
	assert(oconv("X","MD20P")=="X");
	assert(oconv("0","MD20P")=="0.00");
	assert(oconv("0.0","MD20P")=="0.00");
	assert(oconv("00.00","MD20P")=="0.00");
	assert(oconv("000","MD20P")=="0.00");

	assert(oconv("","MD20PZ")=="");
	assert(oconv("X","MD20PZ")=="X");
	assert(oconv("0","MD20PZ")=="");
	assert(oconv("0.0","MD20PZ")=="");
	assert(oconv("00.00","MD20PZ")=="");
	assert(oconv("000","MD20PZ")=="");

	assert(oconv(xx3,"MD20P")=="1234.57");
	assert(oconv(1234.567,"MD20P")=="1234.57");
	assert(oconv("1234.567","MD20P")=="1234.57");

	assert(oconv("","MD20P")=="");
	assert(oconv("","MD20PZ")=="");
	assert(oconv(_VM_ "0" _VM_ _VM_,"MD20PZ")== _VM_ _VM_ _VM_);

	assert(oconv(1234.567,"MC20PZ")=="1234,57");

	//input();
	//stop();

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
		printl(oscwd("OSCWD="));
		var greektestfilename="t_greeksas.txt";
		//check CANNOT write greek unicode characters using French codepage
		assert(not Greek_sas.oswrite(greektestfilename,"French"));//CANNOT write
		//check can write greek unicode characters to Greek codepage
		assert(Greek_sas.oswrite(greektestfilename,"Greek"));
		//check 3 (wide) characters output as 3 bytes
		assert(osfile(greektestfilename)(1) eq 3);
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
	assert(osdelete(tempfilename5));

	//check we cannot write to a non-existent file
	//osdelete(tempfilename5); //make sure the file doesnt exist
	assert(not osfile(tempfilename5) or osdelete(tempfilename5));
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
	assert(osdelete(tempfilename5));

	if (SLASH_IS_BACKSLASH) {
		//check cannot write non-codepage characters
		assert(oswrite("",tempfilename5));
		var offset=2;
		assert(not osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		//check can write codepage characters
		assert(osopen(tempfilename5,tempfilename5,"Greek"));
		offset=2;
		assert(osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		assert(osdelete(tempfilename5));
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
	expect.splicer(2,2,"??");
	expect.splicer(4,3,"???");
	//expect.outputl();
	//expect.oconv("HEX4").outputl();
	//swap(unicode,"\\p{L}","?","ri").outputl();
	//swap(unicode,"\\p{L}","?","ri").oconv("HEX4").outputl();
	//p(L} is regular expression for Unicode Letter
	if (SLASH_IS_BACKSLASH)
		assert(replace(unicode,"\\pL","?","ri") eq expect);
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
	assert(oconv(digits,"MRN") eq digits);
	assert(oconv(digits,"MRB") eq digits);
	assert(oconv(digits,"MR/N") eq "");
	assert(oconv(digits,"MR/B") eq "");
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

	oconv(letters,"MRA").outputl("Expected:"^letters^" Actual:");
	oconv(letters,"MRN").outputl("Expected:\"\" Actual:");
	oconv(letters,"MRB").outputl("Expected:"^letters^" Actual:");
	assert(oconv(letters,"MRA") eq letters);
	assert(oconv(letters,"MRN") eq "");
	assert(oconv(letters,"MRB") eq letters);
	assert(oconv(letters,"MR/A") eq "");
	assert(oconv(letters,"MR/N") eq letters);
	assert(oconv(letters,"MR/B") eq "");

	auto testinvert=[](var cc)
        {
	        var inverted=cc.invert();
        	var invertedtwice=invert(inverted);
                if (cc eq inverted or cc ne invertedtwice) {

                	cc				.outputl("original      =");
                	inverted			.outputl("inverted      =");
                	invertedtwice			.outputl("inverted twice=");

                	cc.oconv("HEX")			.outputl("original hex  =");
                	inverted.oconv("HEX")		.outputl("inverted hex  =");
                	invertedtwice.oconv("HEX")	.outputl("invertedx2 hex=");

                	cc.textseq()			.outputl("original seq  =");
                	inverted.textseq()		.outputl("inverted seq  =");
                	invertedtwice.textseq()		.outputl("invertedx2 seq=");

                	cc.len()			.outputl("original len  =");
                	inverted.len()			.outputl("inverted len  =");
                	invertedtwice.len()		.outputl("invertedx2 len=");

                	stop();
                }
		assert(cc == invertedtwice);
		assert(cc.oconv("HEX") == invertedtwice.oconv("HEX"));
        };

        //check invert works and is reversible for the first 65535 unicode characters
	testinvert("␚ ␛ ␜ ␝ ␞ ␟");

        //check invert is reversible for all bytes (only ASCII bytes are inverted)
        for (var ii=0;ii<=0x1FFFF;ii++) {
                var cc=textchr(ii);
                if (cc.len())
                	testinvert(cc);
                else if (ii < 0xD800 or ii > 0xDFFF )
			printl(ii);
        }

        //check unicode is invalid from 0xD800-0xDFF (UTF16 encoding) and 0x110000 onwards
        assert(textchr(0xD7FF)!="");
        assert(textchr(0xD800)=="");
        assert(textchr(0xDFFF)=="");
        assert(textchr(0xE000)!="");
        assert(textchr(0x10FFFF)!="");
        assert(textchr(0x110000)=="");

	COMMAND.outputl("COMMAND-");
	assert(COMMAND eq "service"
	 or COMMAND eq "main"
	 or COMMAND eq "main2"
	 or COMMAND eq "main2.out"
	 or COMMAND eq "test_main"
	 or COMMAND eq "test_main.out"
	 );

	//test int/string changes after inc/dec (should really check MANY other ops)
	var nn=0;
	assert(nn.toString() eq "0");//nn now has string available internally
	++nn;//this should destroy the internal string
	assert(nn.toString() eq "1");//the string should be recalculated after the ++
	nn++;
	assert(nn.toString() eq "2");
	--nn;
	assert(nn.toString() eq "1");
	nn--;
	assert(nn.toString() eq "0");
	nn+=1;
	assert(nn.toString() eq "1");
	nn-=1;
	assert(nn.toString() eq "0");

	//same for float/string
	nn=0.1;
	assert(nn.toString() eq "0.1");//nn now has string available internally
	++nn;//this should destroy the internal string
	assert(nn.toString() eq "1.1");//the string should be recalculated after the ++
	nn++;
	assert(nn.toString() eq "2.1");
	--nn;
	assert(nn.toString() eq "1.1");
	nn--;
	printl(nn);
	assert(nn.toString() eq "0.1");
	nn+=1.0;
	assert(nn.toString() eq "1.1");
	nn-=1.0;
	assert(nn.toString() eq "0.1");

	//test remove

	var rem="abc"^FM^"xyz";
	var ptr=2;
	var sep;

	//bc|5|2
	//xyz|9|0
	//abc|5|2
	//xyz|9|0
	//|999|0
	//|999|0

	var result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "bc|5|2");

	result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

	ptr=0;

	result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "abc|5|2");

	result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "xyz|9|0");

	ptr=999;

	result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");

	result=rem.substr2(ptr,sep);
	assert(var(result ^ "|" ^ ptr ^ "|" ^ sep).outputl() eq "|999|0");

	//test unquote
	assert(unquote("\"This is quoted?\"") eq "This is quoted?");

	var xyz;
	//xyz=xyz;
	printl("\nTest catching MVError");
	try {
		//runtime errors
		var x1=x1^=1;
		var undefx=undefx++;
		var z=z+1;
		//var xx=xx.operator++();
		var xx=xx++;
	}
	catch (MVError error) {
		print(error.description);
	}

	printl("\nVerify that exodus catches c++ defect at runtime");
	try {
		var abc=abc+1;
		//should never get here because above should cause a runtime undefined error
//		assert(false);
	}
	catch (MVUndefined mve) {
		mve.description.outputl();
	}
	catch(MVUnassigned mve) {
		mve.description.outputl();
	}
	catch(MVNonNumeric mve) {
		mve.description.outputl();
	}
	catch (...) {
		//should never get here because above should cause a runtime error
		assert(false);
	}

	//this turns on tracing SQL statements to postgres
	//DBTRACE=true;

	var ee="";
	var e0=0;
	var e1=1;
	var e2=2;
	var e3=3;

	assert((e2+true) eq 3);
	assert((e2+false) eq 2);
	assert((true+e2) eq 3);
	assert((false+e2) eq 2);

	assert((e2-true) eq 1);
	assert((e2-false) eq 2);
	assert((true-e2) eq -1);
	assert((false-e2) eq -2);

	assert((e2*true) eq 2);
	assert((e2*false) eq 0);
	assert((true*e2) eq 2);
	assert((false*e2) eq 0);

	//assert((e2*true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2*false) eq 0);//ditto
	assert((true/e2) eq 0.5);
	assert((false/e2) eq 0);

	//assert((e2%true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2%false) eq 0);//ditto
	assert((true % e2) eq 1);
	assert((false % e2) eq 0);

	//the six logical comparisons >= <= > < and the lesser precedence eq  ne
	//between logical results and vars are not supported by exodus
	//because they seem useless, probably non existant in real code
	//and because I am too lazy at this point.
	//real code would have to be something like
	// if (cc eq aa > bb)
	//which due to lower precendence of eq and ne is actually
	// if (cc eq (aa > bb))
	assert((e1 eq true) eq true);
	assert((e1 eq false) eq false);
	assert((true eq e1) eq true);
	assert((false eq e1) eq false);

	assert((e1!=true) eq false);
	assert((e1!=false) eq true);
	assert((true!=e1) eq false);
	assert((false!=e1) eq true);

	assert((e1 eq true) eq 1);
	assert((e1 eq false) eq 0);
	assert((true eq e1) eq 1);
	assert((false eq e1) eq 0);

	assert((e1 ne true) eq 0);
	assert((e1 ne false) eq 1);
	assert((true ne e1) eq 0);
	assert((false ne e1) eq 1);

	assert((e3-(e3 eq var(0))) eq 3);

	//exodus's concatenate operator ^ has the wrong priority versus logical comparison operators
	//to avoid this causing unnoticed different operation than classic multivalue basic
	//exodus refuses to concatenate the result of logical expressions like e2>e3
	//printl(e1^e2>e3); //wont compile (because it would produce the wrong result)

	//just add brackets to clarify and correct the precedence for exodus and it will compile
	assert( ((e1^e2)>e3) eq 1); //=1 ... correct result because (1^2)>3 i.e. "12">3 is true

	var s1,s2;
/* why wont this compile??
	s1="111";
	s2;
	exchange(s1,s2); //works even on unassigned variables (rather meaningless if both are unassigned though!)
	assert(s1.unassigned());
	assert(s2 eq "111");
	exchange(s1,s2); //check s1 can be unassigned too
	assert(s1 eq "111");
	assert(s2.unassigned());

	s1="1";
	s2="2";
	assert(exchange(s1,s2) eq "1");
	assert(s1 eq "2");
	assert(s2 eq "1");
*/
	s1="1";
	s2="2";
	assert(transfer(s1,s2) eq "1");
	assert(s1 eq "");
	assert(s2 eq "1");

	//accessing individual characters by index 1=first -1=last etc.
	var a="abc";
	assert(a[1] eq "a");	//a = first character
	assert(a[2] eq "b");	//b = second character
	assert(a[4] eq "");	//"" = if access after last character
	assert(a[-1] eq "c");	//c = last character
	assert(a[-2] eq "b");	//b = last but one character
	assert(a[-9] eq "a");	//a = first character if too negative
	assert(a[0] eq "a");	//a = zero is the same as too negative

	//replacing a section of a string:
	//given start character number, number of characters to replace and a replacement string
	//(this is equivalent to the following classic mv basic syntax
	//but the simple [x,y] syntax is not available in curly bracket languages)
	//tempstr[2,3]='abc'

	//replacing a section of a string - method 1
	a="abcde";
	splicer(a,3,2,"xx");
	assert(a eq "abxxe");

	//replacing a section of a string - method 2
	a="abcde";
	a.splicer(-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 3 - but may be slower
	a="abcde";
	a=splice(a,-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 4 - but may be slower
	a="abcde";
	a=a.splice(-2,-3,"xx");
	assert(a eq "axxe");

	//test single character extraction
	var expected="a" ^ FM ^ "a" ^ FM ^ "b" ^ FM ^ "a" ^ FM ^ "a" ^ FM ^ "b" ^ FM ^ "";
	var tempstr2="ab";
	for (var ii=-3; ii<=3; ++ii)
		assert(tempstr2[ii] eq expected.a(ii+4));

	//test single character extraction on ""
	tempstr2="";
	for (var ii=-3; ii<=3; ++ii)
		assert(tempstr2[ii] eq "");

	var str0="  xxx  xxx  ";
	var str1;

	//global function trim/f/b

	str1=str0;
	assert(trim(str1) eq "xxx xxx");
	assert(str1 eq str0);
	assert(trimf(str1) eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(trimb(str1) eq "  xxx  xxx");
	assert(str1 eq str0);

	assert(trim(str1," ","") eq "xxx xxx");
	assert(str1 eq str0);
	assert(trim(str1," ","F") eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(trim(str1," ","B") eq "  xxx  xxx");
	assert(str1 eq str0);
	assert(trim(str1," ","FB") eq "xxx  xxx");
	assert(str1 eq str0);

	//member function trim/f/b

	assert(str1.trim() eq "xxx xxx");
	assert(str1 eq str0);
	assert(str1.trimf() eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(str1.trimb() eq "  xxx  xxx");
	assert(str1 eq str0);

	assert(str1.trim(" ","") eq "xxx xxx");
	assert(str1 eq str0);
	assert(str1.trim(" ","F") eq "xxx  xxx  ");
	assert(str1 eq str0);
	assert(str1.trim(" ","B") eq "  xxx  xxx");
	assert(str1 eq str0);
	assert(str1.trim(" ","FB") eq "xxx  xxx");
	assert(str1 eq str0);


	//global function trimmer/f/b

	str1=str0;
	assert(trimmer(str1) eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmerf(str1) eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmerb(str1) eq "  xxx  xxx");
	assert(str1 ne str0);

	str1=str0;
	assert(trimmer(str1," ","") eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","F") eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","B") eq "  xxx  xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(trimmer(str1," ","FB") eq "xxx  xxx");
	assert(str1 ne str0);

	//member function trimmer/f/b

	str1=str0;
	assert(str1.trimmer() eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmerf() eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmerb() eq "  xxx  xxx");
	assert(str1 ne str0);

	str1=str0;
	assert(str1.trimmer(" ","") eq "xxx xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","F") eq "xxx  xxx  ");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","B") eq "  xxx  xxx");
	assert(str1 ne str0);
	str1=str0;
	assert(str1.trimmer(" ","FB") eq "xxx  xxx");
	assert(str1 ne str0);

	dim a9;
	var a10;
	assert(split("xx"^FM^"bb",a9) eq 2);
	assert(join(a9) eq ("xx" ^FM^ "bb"));

	dim dx(3);
	dx=1;
	assert(dx.join().outputl()==(1^FM^1^FM^1));

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

	var aa1,aa2,aa3,aa4;
	aa1=aa2=aa3="aa";
	assert(aa1 eq "aa");
	assert(aa2 eq "aa");
	assert(aa3 eq "aa");

	//string seed
        initrnd("cccc");
        printl(rnd(1000000000));
        initrnd("cccc");
        assert(rnd(1000000000)==231348803);

	//slightly different string seed
        initrnd("cccd");
        printl(rnd(1000000000));
        initrnd("cccd");
        assert(rnd(1000000000)==610052346);

	//integer seed
        //initrnd(123457);
        //printl(rnd(1000000000));
        initrnd(123457);
        assert(rnd(1000000000)==466803956);

	//slightly different integer seed
        //initrnd(123458);
        //printl(rnd(1000000000));
        initrnd(123458);
        assert(rnd(1000000000)==191396791);

	//initrnd treats floats as integers so same result as above
        //initrnd(123458.2);
        //printl(rnd(1000000000));
        initrnd(123458.2);
        assert(rnd(1000000000)==191396791);

	var tempinp;
//	input("Press Enter ...",tempinp);
	//ensure lower case sorts before uppercase (despite "A" \x41 is less than "a" \x61)
	a="a";
	var A="A";
	assert(a<A);

	var da1="aa"^FM^"b1"^VM^"b2"^SM^"b22"^FM^"cc";
	gosub internal_subroutine_xyz(da1);

	//extraction
	assert(da1(2) eq extract(da1,2));//this extracts field 2
	assert(da1(2,2) eq extract(da1,2,2));//this extracts field 2, value 2
	assert(da1(2,2,2) eq extract(da1,2,2,2));//this extracts field 2, value 2, subvalue 2

	//this wont work
	pickreplace(da1,3,"x");//or this
	pickreplace(da1,3,3,"x");//or this
	pickreplace(da1,3,3,3,"x");//or this
	insert(da1,3,"x");//or this
	insert(da1,3,3,"x");//or this
	insert(da1,3,3,3,"x");//or this

	//replacement
	da1(2)="x";//sadly this compile and runs without error but does nothing!

	da1="f1" ^FM^ "f2" ^FM^ "f3";

	//replace field 2 with "R2"
	da1="";
	assert(pickreplacer(da1, 2, "R2") eq ( FM ^ "R2"));

	//replace field 2, value 3 with "R22"
	da1="";
	assert(pickreplacer(da1, 2, 3, "R23") eq ( FM ^VM^VM^ "R23"));

	//replace field 2, value 3, subvalue 4 with "R234"
	da1="";
	assert(pickreplacer(da1, 2, 3, 4, "R234") eq ( FM^ VM^VM^ SM^SM^SM^ "R234"));

	//insert "I2" at field 2
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, "I2") eq ( "f1" ^FM^ "I2" ^FM^ "f2"));

	//insert "I21" at field 2, value 1
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, 1, "I21") eq ( "f1" ^FM^ "I21" ^VM^ "f2"));

	//insert "I211" at field 2, value 1, subvalue 1
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, 1, 1, "I211") eq ( "f1" ^FM^ "I211" ^SM^ "f2"));

	//remove (delete) field 1
	da1="f1" ^FM^ "f2";
	assert(remover(da1, 1) eq ( "f2"));

	//remove (delete) field 1, value 2
	da1="f1" ^VM^ "f1v2" ^VM^ "f1v3" ^FM^ "f2";
	assert(remover(da1, 1, 2) eq ("f1" ^VM^ "f1v3" ^FM^ "f2"));

	//remove (delete) field 1, value 2, subvalue 2
	da1="f1" ^VM^ "f1v2s1" ^SM^ "f1v2s2" ^SM^ "f1v2s3" ^VM^ "f1v3" ^FM^ "f2";
	assert(remover(da1, 1, 2, 2) eq ("f1" ^VM^ "f1v2s1" ^SM^ "f1v2s3" ^VM^ "f1v3" ^FM^ "f2"));

	var tempfile=SLASH^"129834192784";
	if (osopen(tempfile,tempfile))
		assert(false and var("non-existent file opened!"));

	//math.h seems to have been included in one of the boost or other special headers
	//in this main.cpp file and that causes confusion between math.h and exodus.h sin() and other functions.
	//we resolved the issue here by being specific about the namespace
	assert( exodus::sin(30).round(8) eq 0.5);
	assert( exodus::cos(60).round(8) eq 0.5);
	assert( exodus::tan(45).round(8) eq 1);
	assert( exodus::atan(1).round(6) eq 45);

	assert( exodus::abs(0)    eq 0);
	assert( exodus::abs(30)   eq 30);
	assert( exodus::abs(30.00) eq 30);
	assert( exodus::abs(30.10) eq 30.1);
	assert( exodus::abs(30.90) eq 30.9);

	assert( exodus::abs(-0)    eq 0);
	assert( exodus::abs(-30)   eq 30);
	assert( exodus::abs(-30.00) eq 30);
	assert( exodus::abs(-30.10) eq 30.1);
	assert( exodus::abs(-30.90) eq 30.9);

	assert( exodus::exp(1).round(9) eq 2.718281828);
	assert( exodus::loge(1) eq 0);
	assert( exodus::loge(2.718281828).round(9) eq 1);
	assert( exodus::loge(10).round(9) eq 2.302585093);
	assert( exodus::sqrt(100) eq 10);

	var xx=osshellread("dir");
	//xx.osread(xx);
	/*
	var inpz=input();

	var inpx;
	input(inpx);

	var inpy;
	input("prompt",inpy);

	var inpq;
	inputn(inpq,5);
	*/

	ossetenv("XYZ","abc");
	assert(osgetenv("XYZ") == "abc");

	//check floating point modulo
	assert(mod(2.3,var(1.499)).round(3) eq 0.801);
	assert(mod(-2.3,var(-1.499)).round(3) eq -0.801);
	//following is what c++ fmod does (a rather mathematical concept)
	//assert(mod(-2.3,var(1.499)).round(3).outputl() eq -0.801);
	//assert(mod(2.3,var(-1.499)).round(3).outputl() eq 0.801);
	//but arev and qm ensure that the result is somewhere from 0 up to or down to
	//(but not including) the divisor
	//method is ... do the fmod and if the result is not the same sign as the divisor, add the divisor
	assert(mod(-2.3,var(1.499)).round(3) eq 0.698);
	assert(mod(2.3,var(-1.499)).round(3) eq -0.698);

	assert(oconv(1234,"MD20P") eq "1234.00");
	assert(var(10000).oconv("DY0") eq "");

	assert(var("a")<var("B"));
	assert(var(1000).oconv("MD80") eq "1000.00000000");
	assert(var("31 JAN 2008").iconv("D") eq "14641");

	assert(var("1/31/2008").iconv("D") eq 14641);
	assert(var("2008/1/31").iconv("DS") eq "14641");

	assert(var("JAN/31/2008").iconv("D") eq "14641");
	assert(var("2008/JAN/31").iconv("DS") eq "14641");

	assert(var("1/1/92").iconv("D/E") eq "8767");

	assert(var("1/1/68").iconv("D/E") eq "1");
	assert(var("31/12/67").iconv("D/E") eq "0");
	assert(var("30/12/67").iconv("D/E") eq "-1");
	assert(var("1/1/1900").iconv("D/E") eq "-24835");

	assert(var("31/1/2008").iconv("DE") eq "14641");
	assert(var("31/1/2008").iconv("D/E") eq "14641");
	assert(var("31 1 2008").iconv("DE") eq "14641");
	assert(var("31-1-2008").iconv("DE") eq "14641");
	assert(var("31/JAN/2008").iconv("DE") eq "14641");
	assert(var("JAN/31/2008").iconv("DE") eq "14641");
	assert(var("29 FEB 2008").iconv("D") eq "14670");

	assert(var("32/1/2008").iconv("DE") eq "");
	assert(var("30/2/2008").iconv("DE") eq "");
	assert(var("1/31/2008").iconv("DE") eq "");

	assert(oconv(15036, "D")     == "01 MAR 2009");
	assert(oconv(15036, "D2")    == "01 MAR 09");
	assert(oconv(15036, "D4")    == "01 MAR 2009");

	assert(oconv(15036, "D/")    == "03/01/2009");
	assert(oconv(15036, "D2/")   == "03/01/09");
	assert(oconv(15036, "D*")    == "03*01*2009");

	assert(oconv(15036, "D/E")   == "01/03/2009");
	assert(oconv(15036, "D2E")   == "01 MAR 09");
	assert(oconv(15036, "D2/E")  == "01/03/09");

	assert(oconv(15036, "DQ")   == "1");
	assert(oconv(15036, "DW")   == "7");
	assert(oconv(15036, "DWA")  == "SUNDAY");

	//select("select test_alphanum with f1 between 20 and 21");
	printl(oconv(10.1,"MD20"));

	/*
	var alphanum1="Flat 10a";
	var alphanum2="Flat 2b";
	//Flat 2b is before Flat 10a
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="Part A-10";
	alphanum2="Part A-2";
	//Part A-2 is before Part A-10
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="Part -10";
	alphanum2="Part -2";
	//Part -10 is before Part -2
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));

	alphanum1="-10";
	alphanum2="-2";
	//-10 is before -2
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));

	alphanum1="-1.11";
	alphanum2="-1.2";
	//-1.2 is before -1.11
	assert(naturalorder(alphanum1.toString()) > naturalorder(alphanum2.toString()));

	alphanum1="01.10";
	alphanum2="1.1";
	//01.10 is equal to 1.1
	assert(naturalorder(alphanum1.toString()) = naturalorder(alphanum2.toString()));

	alphanum1="A B C..C+";
	alphanum2="A B C.C";
	//A B C..C+ is before A B C.C
	assert(naturalorder(alphanum1.toString()) < naturalorder(alphanum2.toString()));
*/

	/*
	for (double xx=-5;xx<=5;++xx)
	{
		ostringstream ostringstream1;

		appenddouble2sortstring(xx,ostringstream1);

		cout.width(3);
		cout<<xx<<" "<<ostringstream1.str()<<endl;
	}
	stop();
	*/

	//investigate the bytes of a double in hex for natural sort
	/*
	double d2=1;
	double d3=2;
	if (d2<d3)
		d2=d3;
	union {double d1; char chars[8];};
	for (d1=-5;d1<=5;++d1) {
		print(d1," ");
		for (unsigned int partn=0;partn<sizeof(chars);++partn)
			//var(chars[partn]).oconv("HEX").output();
			std::cout << std::hex << int(chars[partn]) << " " ;
		printl();
	}
	*/

	//var steve;steve.input(1);

	//catching errors - doesnt work now that backtrace aborts (to prevent system crashes ... maybe better solution is to trap in main()
	//var unass;
	//try{unass+1;}
	//catch(...){};

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
	//bool cannot be used numerically ON MSVC (unlike in pick)
	//could change all logical ops to return var or find a way to allow void* pointer to promote to bool
	//or perhaps add bool to the list of automatic constructors?
	var log1="xx";
	var log2="x";
	//following will not compile now that all exodus logical operators return bool instead of var
	//if (log1 eq log2^log1) {}
/*
in.cpp(181) : error C2666: 'exodus::operator ^' : 7 overloads have similar conversions
could be 'exodus::var exodus::operator +(const int,const exodus::var &)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const double)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const int)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const exodus::var &)' [found using argument-dependent lookup]
or 'built-in C++ operator+(int, bool)'
or 'built-in C++ operator+(unsigned int, bool)'
or 'built-in C++ operator+(bool, bool)'
while trying to match the argument list '(exodus::var, bool)'
*/

	//neither will the following
	//var log3=count(log1,"x")+(log1 ne "");
	//just convert it to the better
	assert(dcount(log1,"x") eq 3);

	printl(SENTENCE);
	assert(var("xyz").substr(4,1) eq "");

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS

	var subs="xyz";
	assert(subs.substr(-1) eq "z");
	assert(subs[-1] eq "z");

	printl(oconv("a","L#3").quote());
	assert(oconv("a","L#3") eq "a  ");
	assert(oconv("abc","L#3") eq "abc");
	assert(oconv("abcd","L#3") eq "abc");
	assert(oconv("a"^FM^"abc"^FM^"abcd","L#3") eq ("a  "^FM^"abc"^FM^"abc"));

	assert(oconv("a","R#3") eq "  a");
	assert(oconv("abc","R#3") eq "abc");
	assert(oconv("abcd","R#3") eq "bcd");
	assert(oconv("a"^FM^"abc"^FM^"abcd","R#3") eq ("  a"^FM^"abc"^FM^"bcd"));

	assert(oconv("a","T#3") eq "a  ");
	assert(oconv("abc","T#3") eq "abc");
	assert(oconv("abcd","T#3") eq ("abc"^TM^"d  "));
	assert(oconv("a"^FM^"abc"^FM^"abcd","T#3") eq ("a  "^FM^"abc"^FM^"abc"^TM^"d  "));

	assert(oconv("a","L(0)#3") eq "a00");
	assert(oconv("a","R(0)#3") eq "00a");
	assert(oconv("a","T(0)#3") eq "a00");
	assert(oconv("abcd","T(0)#3") eq ("abc"^TM^"d00"));

	assert(oconv("ab","C#-2") eq "ab");
	assert(oconv("ab","C#-1") eq "ab");
	assert(oconv("ab","C#0") eq "");
	assert(oconv("ab","C#1") eq "a");
	assert(oconv("ab","C#2") eq "ab");
	assert(oconv("ab","C#3") eq "ab ");
	assert(oconv("ab","C#4") eq " ab ");
	assert(oconv("ab","C#5") eq " ab  ");
	assert(oconv("ab","C(0)#5") eq "0ab00");

	assert(iconv("23 59 59","MT") eq 86399);
	assert(iconv("xx11yy12zz13P","MT") eq 83533);
	assert(iconv("24 00 00","MT") eq "");

	//http://www.regular-expressions.info/examples.html
	assert(replace("Steve Bush Bash bish","B.","Ru","ri") eq "Steve Rush Rush Rush");

	assert(oconv("Aa019KK","HEX") eq "41613031394B4B");
	assert(oconv("Aa019KK","HEX2") eq "41613031394B4B");
	//assert(oconv("Aa019KK","HEX4") eq "00410061003000310039004B004B");
	//assert(oconv("Aa019KK","HEX8") eq "00000041000000610000003000000031000000390000004B0000004B");
	assert(var("41613031394B4B").iconv("HEX") eq "Aa019KK");
	assert(var("41613031394B4B").iconv("HEX2") eq "Aa019KK");
	//assert(var("00410061003000310039004B004B").iconv("HEX4") eq "Aa019KK");
	//assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX8") eq "Aa019KK");

	//doesnt accept FMs etc yet
	//assert(var("FF"^FM^"00").iconv("HEX") eq ("00FF"^FM^"00FF"));
	assert(var("FF"^FM^"00").iconv("HEX2").oconv("HEX2") eq "");
	//anything invalid returns empty string
	assert(var("XF").iconv("HEX").oconv("HEX") eq "");

	var time1=var("10:10:10").iconv("MT");
	assert(var("abcabdef").trim("abef") eq "cbd");
	assert(var("abcabdef").trimf("abef").trimb("abef") eq "cabd");

	var temp3="c";
	var temp2("c");
	var temp1="abc";
	//var temp2=temp2="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//show cause MVUndefined and not simply crash do to assigning an uninitialised string
	//var undefinedassign=undefinedassign="xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//var undefinedassign2=undefinedassign2=11;

	//undefined
	//var undefinedboolconv=undefinedboolconv?true:false;

	//undefined
	//var conn1=conn1.connect();

	assert(space(-11) eq "");
	assert(var("x").str(-7) eq "");

	var x9;
//	if (var xx eq x) {};
	//TODO implement some kind of a switch to turn off exit in mvexceptions
	var filehandle;
/*
	try {
		printl(filehandle);
	}
	catch (MVError error) {
		error.description.outputl();
	}
*/
	assert(oconv(0,"D4") eq "31 DEC 1967");
	assert(oconv("xxx","") eq "xxx");

	assert(oconv("","MX") eq "");
	assert(oconv(" ","MX") eq " ");
	assert(oconv(0,"MX") eq "0");
	assert(oconv(-0,"MX") eq "0");
	assert(oconv("X","MX") eq "X");
	//assert(oconv("-1.5","MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1.5","MX") eq "FFFFFFFE");
	assert(oconv("-1","MX") eq "FFFFFFFF");
	assert(oconv("1.5","MX") eq "2");
	assert(oconv("20" ^ FM ^ 255,"MX") eq ("14" ^FM^ "FF"));

	assert(oconv("","D") eq "");
	assert(oconv("X","D") eq "X");
	assert(oconv("-1.5","D") eq "29 DEC 1967");
	assert(oconv("1.5","D") eq "01 JAN 1968");
	assert(oconv("1.5" ^ FM ^ -1.5,"D") eq ("01 JAN 1968"^FM^"29 DEC 1967"));

	assert(oconv(14276,"D") eq "31 JAN 2007");
	assert(oconv(14276,"D2") eq "31 JAN 07");
	assert(oconv(14276,"D4") eq "31 JAN 2007");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D ") eq "01 31 2007");
	assert(oconv(14276,"D2/") eq "01/31/07");
	assert(oconv(14276,"D2-") eq "01-31-07");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D/E") eq "31/01/2007");
	assert(oconv(14276,"D2 E") eq "31 01 07");
	assert(oconv(14276,"D S") eq "2007 01 31");
	assert(oconv(14276,"DM") eq "1");
	assert(oconv(14276,"DMA") eq "JANUARY");
	assert(oconv(14276,"DW") eq "3");
	assert(oconv(14276,"DWA") eq "WEDNESDAY");
	assert(oconv(14276,"DY") eq "2007");
	assert(oconv(14276,"DY2") eq "07");
	assert(oconv(14276,"D2Y") eq "07 JAN 31");
	assert(oconv(14276,"D5Y") eq "02007 JAN 31");
	assert(oconv(14276,"DD") eq "31");
	assert(oconv(14276,"DL") eq "31");
	assert(oconv(14276,"DQ") eq "1");
	assert(oconv(14276,"DJ") eq "31");

	var feb29_2004=13209;//iconv("29 FEB 2004","D");
	assert(oconv(feb29_2004,"DL") eq "29");

	assert(oconv(14591,"D") eq "12 DEC 2007");
	assert(oconv(14591,"D2/") eq "12/12/07");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"D-") eq "12-12-2007");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"DJ") eq "346");
	assert(oconv(14591,"DM") eq "12");
	assert(oconv(14591,"DMA") eq "DECEMBER");
	assert(oconv(14591,"DW") eq "3");
	assert(oconv(14591,"DWA") eq "WEDNESDAY");
	assert(oconv(14591,"DY") eq "2007");
	assert(oconv(14591,"DQ") eq "4");
	assert(oconv(14591,"DD") eq "12");
	assert(oconv(14591,"DL") eq "31");

	//check times around noon and midnight round trip ok
	for (var ii=0; ii<=61 ; ++ii)
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);
	for (var ii=43200-61; ii<=43200+61 ; ++ii)
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);

	//test that some random times iconv/oconv roundtrip ok
	initrnd(1000);
	var timex;
	for (int ii=1; ii<1000; ++ii) {
		timex=rnd(18600);
//		timex.oconv("MTHS").output(" ").iconv("MTHS").outputl(" ");
		assert(timex.oconv("MTHS").iconv("MTHS") eq timex);
	}

	//oconv(46622,"MTH").outputl("oconv 46622 MTH is" );
	assert(oconv(46622,"MTH") eq "12:57PM");

	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MT") eq "17:35");

	assert(oconv(0,"MTH") eq "12:00AM");
	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MTH") eq "05:35PM");
	assert(oconv(31653,"MTS") eq "08:47:33");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTHS") eq "05:35:06PM");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTS.") eq "17.35.06");
	assert(oconv(63306,"MTh") eq "17h35");

	assert(oconv(61201,"MT") eq "17:00");
	assert(oconv(61201,"MTS") eq "17:00:01");
	assert(oconv(61201,"MTH") eq "05:00PM");
	assert(oconv(61201,"MTHS") eq "05:00:01PM");

	var time2=43261;
	assert(time2.oconv("MT").outputl() eq "12:01");
	assert(time2.oconv("MTH").outputl() eq "12:01PM");
	assert(time2.oconv("MTS").outputl() eq "12:01:01");
	assert(time2.oconv("MTSH").outputl() eq "12H01H01");
	assert(time2.oconv("MTx") eq "12x01");
	assert(time2.oconv("MTHx") eq "12x01PM");
	assert(time2.oconv("MTSx") eq "12x01x01");
	assert(time2.oconv("MTSHx") eq "12H01H01");

	time2=0;
	assert(time2.oconv("MT").outputl() eq "00:00");
	assert(time2.oconv("MTH").outputl() eq "12:00AM");
	assert(time2.oconv("MTS").outputl() eq "00:00:00");
	assert(time2.oconv("MTHS").outputl() eq "12:00:00AM");

	//negative time
	time2=-1;
	assert(time2.oconv("MT").outputl() eq "23:59");
	assert(time2.oconv("MTH").outputl() eq "11:59PM");
	assert(time2.oconv("MTS").outputl() eq "23:59:59");
	assert(time2.oconv("MTHS").outputl() eq "11:59:59PM");
	time2=-86400/2;
	assert(time2.oconv("MT").outputl() eq "12:00");
	assert(time2.oconv("MTH").outputl() eq "12:00PM");
	assert(time2.oconv("MTS").outputl() eq "12:00:00");
	assert(time2.oconv("MTHS").outputl() eq "12:00:00PM");
	time2=-86400-1;
	assert(time2.oconv("MT").outputl() eq "23:59");

	//test some unlimited time
	assert(var(-100).oconv("MTU").outputl() eq "-00:01");
	assert(var(-100).oconv("MTUS").outputl() eq "-00:01:40");
	assert(var(-10000).oconv("MTUS").outputl() eq "-02:46:40");

	assert(var(100).oconv("MTU").outputl() eq "00:01");
	assert(var(100).oconv("MTUS").outputl() eq "00:01:40");
	assert(var(1000).oconv("MTUS").outputl() eq "00:16:40");
	assert(var(10000).oconv("MTUS").outputl() eq "02:46:40");
	//NB 27:46:40 NOT ROUNDED UP TO 27:47 because mins like on clock
	assert(var(100000).oconv("MTU").outputl() eq "27:46");
	assert(var(100000).oconv("MTUS").outputl() eq "27:46:40");

	//test some decimal hours based time
	assert(var(0).oconv("MT2").outputl() eq "00:00");
	assert(var(0).oconv("MT2S").outputl() eq "00:00:00");
	assert(var(0.25).oconv("MT2").outputl() eq "00:15");
	assert(var(0.25).oconv("MT2S").outputl() eq "00:15:00");
	assert(var(24).oconv("MT2S").outputl() eq "00:00:00");
	assert(var(25).oconv("MT2S").outputl() eq "01:00:00");
	assert(var(-25).oconv("MT2S").outputl() eq "23:00:00");

	//test some UNLIMITED decimal hours based time
	//NB negative unlimited time is symmetrical (unlike normal time_
	//negative time could be useful to refer to previous dates
	//but in a non-symmetrical fashion ie -1 means one second back into yesterday
	//ie 23:59:59 ... not -00:00:01. negative hours could perhaps be used
	//to credit hours in some imaginary volumetric numerical fashion
	//The "symmetric" feature should perhaps be a SEPARATE option
	//instead of being tied to the U unlimited option
	assert(var(.01).oconv("MT2US").outputl() eq "00:00:36");
	assert(var(-.01).oconv("MT2US").outputl() eq "-00:00:36");
	assert(var(.25).oconv("MT2US").outputl() eq "00:15:00");
	assert(var(-.25).oconv("MT2US").outputl() eq "-00:15:00");
	assert(var(25).oconv("MT2US").outputl() eq "25:00:00");
	assert(var(-25).oconv("MT2US").outputl() eq "-25:00:00");
	assert(var(125.25).oconv("MT2US").outputl() eq "125:15:00");
	assert(var(-125.25).oconv("MT2US").outputl() eq "-125:15:00");
	assert(var(9).oconv("MT2US").outputl() eq "09:00:00");
	assert(var(-9).oconv("MT2US").outputl() eq "-09:00:00");

//	assert(oconv(FM ^ "\x0035","HEX4") eq "00FE0035");
	//assert(oconv(FM ^ "\x0035","HEX4") eq "07FE0035");
	//assert(oconv(FM,"HEX4") eq "07FE");


	//test copying files forced overwrite)
    osmkdir("d1/d1");
    oswrite("f1","d1/f1");
    oswrite("f2","d1/f2");
    oscopy("d1/f1","d1/f2");
    var data;
    osread(data,"d1/f2");
    printl(data=="f1");

	//test copying directories (recursive)
    oswrite("f1","d1/d1/f1");
    oscopy("d1","d2");
    osread(data,"d2/d1/f1");
    assert(data=="f1");

    //cannot copy a directory to a file
    assert(oscopy("d1","d2/d1/f1")==0);

    osrmdir("d1",true);
    osrmdir("d2",true);

	osmkdir("test_main.1");
	osmkdir("test_main.2");

	if (not SLASH_IS_BACKSLASH) {

		//unordered files and directories
		assert(oslist("*").convert(FM,"") == osshellread("ls . -AU1").convert("\n\r",""));

		//files (not directories)
		assert(oslistf("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . ! -type d -printf '%f\n'").convert("\n\r",""));

		//directories (not files)
		assert(oslistd("*").convert(FM,"") == osshellread("find . -maxdepth 1 ! -path . -type d -printf '%f\n\'").convert("\n\r",""));
	}
	osrmdir("test_main.1");
	osrmdir("test_main.2");

	printl();
	assert(osdir(SLASH).match(_FM_ "\\d{5}" _FM_ "\\d{1,5}"));

	//root directories

	//check one step multilevel subfolder creation (requires boost version > ?)
	var topdir1=SLASH^"exodus544";
	var topdir1b=topdir1^"b";
	var subdir2=topdir1^SLASH^"abcd";
	var subdir2b=topdir1b^SLASH^"abcd";

	var tempdir="exotemp746";
	osrmdir(tempdir,true);

	//try to remove any old versions (subdir first to avoid problems)
	osrmdir(topdir1b,true);
	osrmdir(topdir1);
	osrmdir(subdir2b,true);
	osrmdir(subdir2);

	//need oermission to test root directory access
	if (osmkdir(subdir2)) {

		//assert(osmkdir(subdir2));

		printl("\nCheck CANNOT rename multilevel root folders");
		assert(not osrename(topdir1,topdir1b));

		printl("\nCheck CANNOT force delete root folders");
		assert(not osrmdir(topdir1,true));
		printl();

		//check can remove root folders one by one without force
		assert(osrmdir(subdir2));
		assert(osrmdir(topdir1));

		//printl(osdir("c:\\config.sys"));

		//relative directories ie not-root
		if (osdir(tempdir))
			assert(osrmdir(tempdir,true));

		//check mkdir
		assert(osmkdir(tempdir));
		assert(osdir(tempdir));
		assert(not osmkdir(tempdir));

		//check rmdir
		assert(osrmdir(tempdir));
		assert(not osdir(tempdir));
	}

	//check writing a 1Mb file
	//restrict to ascii characters so size on disk=number of characters in string
	//also restrict to size 1 2 4 8 16 etc
	//var str1="1234ABC\x0160";//Note: you have to prefix strings with L if you want to put multibyte hex chars
	str1="1234ABCD";
	var filesize=1024*1024/8;
	printl(tempdir);
	assert(osmkdir(tempdir));
	assert(osrmdir(tempdir));
	assert(osmkdir(tempdir));
	var tempfilename=tempdir^SLASH^"temp1";
	printl(tempfilename);
	//printl(str(str1,filesize/len(str1)));
	assert(oswrite(str(str1,filesize/len(str1)),tempfilename));
	var filedate=date();
	assert(osfile(tempfilename));
	var info=osfile(tempfilename);
	assert(info.a(1) eq filesize);
	assert(info.a(2) eq filedate);

	//check copying to a new file
	var tempfilename2=tempfilename^2;
	if (osfile(tempfilename2))
	assert(osdelete(tempfilename2));
	assert(oscopy(tempfilename,tempfilename2));
	assert(osfile(tempfilename2) eq info);

	//check renaming
	var tempfilename3=tempfilename^3;
	assert(osrename(tempfilename2,tempfilename3));
	assert(osfile(tempfilename3) eq info);

	//check force delete of subdirectory
	assert(osrmdir(tempdir,true));

	var x;
	var y;
	x="0";
	y=date();

	assert(var(1000).oconv("MD20P,") eq "1,000.00");

	assert(var("0")<var(".5"));

	var ss;

	var tconv=FM.oconv("T#20");
	assert(tconv eq ("                    " _FM_ "                    "));
	tconv="xxxxx/xxxxx xxx" ^ FM ^ "xx";
	tconv=tconv.oconv("T#8");
	assert(tconv eq ("xxxxx/xx" ^ TM ^ "xxx xxx " ^ FM ^ "xx      "));

	//test redimensioning
	dim aaaa(10);
	aaaa.redim(20,30);

	var sentence=sentence();

    //wcout<<"main()"<<endl;
    //MVSystem mvsystem;
    //mvsystem.run();
    //wcout<<"exit"<<endl;

//    var xx="XXX";
//  xx.inverter();

	var dividend=100;
	assert(mod(dividend,30) eq 10);

	var env=osgetenv("");
	osgetenv("PATH");
	osgetenv("HOME");
	env="Steve";
	env.ossetenv("XYZ");
	osgetenv("XYZ");

//	var().debug();
//	var xx=xx[1];

//experiment with some syntactic sugar
#define on ,
#define from ,

	var temprecord;
	var tempfilename0="tempfile";
	assert(oswrite("123" on tempfilename0));
	assert(osfile(tempfilename0));
	assert(osread(temprecord from tempfilename0));
	assert(temprecord eq "123");
	assert(osdelete(tempfilename0));
	assert(not osfile(tempfilename0));

//	var().stop();

// cannot connectlocal in main and thread until pipes are numbered
//	tss_environmentns.reset(new int(0));
//	tss_environmentns.reset(0);

//	if (!init_thread(0))
//		abort("Couldnt init thread 0");

	printl("\nPrint out 1st 256 unicode characters and their hashes");
	for (var ii=0;ii<256;ii++) {
		var xx=chr(ii);
		print(ii ^ ":" ^ xx ^ " " ^ xx.hash() ^ " ");
	}
	printl();

	printl("Checking time oconv/iconv roundtrip for time (seconds) =0 to 86400");
	//initrnd(999);
	//for (int ii=0; ii<86400; ++ii) {
	//	var time=ii;
	var started=ostime();
	for (var itime=0; itime<86400; ++itime) {
//		itime.outputl("itime=").oconv("MTHS").outputl("otime=").iconv("MTHS").outputl("itime=");
		assert(itime.oconv("MTHS").iconv("MTHS") eq itime);
		assert(itime.oconv("MTS").iconv("MTS") eq itime);
	}
	var stopped=ostime();
	errputl(stopped-started," seconds");
	printl("test_main finished ok and exiting OK");
	printl("Test passed");

	return 0;
}

//be careful not to name any subroutines the same as any variables
function internal_subroutine_xyz(in xyzz)
{
	printl("internal_subroutine_xyz(in xyzz)");
	//var xx;
	//printl(xx);
	assert(xyzz(2,2,2) eq "b22");
	return 1;
}

function accrest() {
        var infilename="\\tapex";//=field(sentence()," ",2);
        var infile;
        if (not osopen(infilename,infile))
                abort("Cant read "^infilename);

        var fms=FM^VM^SM^TM^STM;
        var visibles="^]\???";
        var EOL="\r\n";
        var offset=0;
        var blocksize=50000;
        while (true) {
                var block;
                block.osbread(infile,offset,blocksize);
 //printl(offset," ",len(block));
                if (not len(block))
                        break;
                offset+=blocksize;
                converter(block,fms,visibles);
                swapper(block,IM,EOL);
                print(block);
				var xx;
                inputn(1);
        }
        return 0;
}

function test_codepage(in codepage, in lang) {
	printl("---------- " ^ lang ^ " " ^ codepage ^ " ----------");

	var v256="";
	for (int ii=0;ii<=255;++ii)
		v256 ^= chr(ii);
	oswrite(v256,"t_codep.bin");
	assert(osfile("t_codep.bin").a(1)==256);

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
