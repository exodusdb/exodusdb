#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
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

func main() {
	var cmd;

//    {
//		var x = "";
//		var("عشرة ترليون") eq "x";
//		var("xxxxddx") eq "y";
//	}
	TRACE(osgetenv().convert(FM, "\n"))
	var exodusdir = osgetenv("GITHUB_WORKSPACE");
	TRACE(exodusdir)
	if (exodusdir) {
		printl(exodusdir);
		assert(oscwd(exodusdir ^ "/test/src"));
	}

	//	this code fragment tests UTF8 coding/decoding by reading utf8.html and writing its copy ...
	var utf8_html = "utf8.html";
	var buf;
	if (not buf.osread(utf8_html) )	//default external and internal format is utf8
	 abort("test_multilang needs access to test data file utf8.html");

	assert(buf.oswrite("t_utf8copy.html"));
	cmd = "diff " ^ utf8_html ^ " t_utf8copy.html";
	printl(cmd);
	assert(osshell(cmd));

	assert(buf.osread(utf8_html, "utf8"));			 // read with boost_utf8-facet
	assert(buf.oswrite("t_utf8utf8.html", "utf8"));	 // write with boost_utf8_facet
	cmd = "diff " ^ utf8_html ^ " t_utf8utf8.html";
	printl(cmd);
	assert(osshell(cmd));

	// and chinese page
	assert(buf.osread("calblur8.html", "utf8"));
	assert(buf.oswrite("t_calblur8utf8.html", "utf8"));
	cmd = "diff calblur8.html t_calblur8utf8.html";
	printl(cmd);
	assert(osshell(cmd));

	//  this code fragment tests locale specific characters IO with narrow files (1 char = 1 byte codepages)
	var cyrillic_cp = VOLUMES ? "Ukrainian_Ukraine.1251" : "CP1251";
	//osshell("locale ru_RU.CP1251 || locale-gen ru_RU.CP1251");
	//[<D0><F3><F1><F1><EA><E8><E9> <FF><E7><FB><EA>][<D3><EA><F0><E0><BF><ED><F1><FC><EA><E0> <EC><EE><E2><E0>]
	//                       [English language][Русский язык][Українська мова]
	var EN_RU_UA_cp1251 = "[English language]" ^ iconv("5BD0f3f1f1eae8e920ffe7fbea5d5bd3eaf0e0bfedf1fceae020eceee2e05d", "HEX");
	printl(EN_RU_UA_cp1251.from_codepage("cp1251"));
	assert(EN_RU_UA_cp1251.len().outputl()            eq 49);
	var EN_RU_UA_utf8 = "[English language][Русский язык][Українська мова]";
	//                     [English language][Русский язык][Українська мова]
	assert(EN_RU_UA_utf8.len()                        eq 74);
	assert(EN_RU_UA_cp1251.from_codepage(cyrillic_cp) eq EN_RU_UA_utf8);
	assert(EN_RU_UA_cp1251                            eq EN_RU_UA_utf8.to_codepage(cyrillic_cp));

	std::cout << ">[English language][Русский язык][Українська мова]" << std::endl;
	EN_RU_UA_cp1251.from_codepage(cyrillic_cp).outputl(">");

	//convert write utf8 with code page
	var EN_RU_UA_file = "t_EN_RU_UA.txt";
	assert(oswrite(EN_RU_UA_utf8, EN_RU_UA_file, cyrillic_cp));
	printl(osfile(EN_RU_UA_file));
	assert(osfile(EN_RU_UA_file).f(1) eq 49);

	//convert write cp1251 WITHOUT code page - will go out unchanged
	assert(oswrite(EN_RU_UA_cp1251, EN_RU_UA_file));
	assert(osfile(EN_RU_UA_file).f(1) eq 49);

	//read it with code page
	var EN_RU_UA_txt2;
	assert(osread(EN_RU_UA_txt2, EN_RU_UA_file, cyrillic_cp));
	assert(EN_RU_UA_txt2 eq EN_RU_UA_utf8);
	EN_RU_UA_txt2.outputl("<");

	//read it WITHOUT code page (it will arrive back as NOT UTF8)
	assert(osread(EN_RU_UA_txt2, EN_RU_UA_file));
	assert(EN_RU_UA_txt2 eq EN_RU_UA_cp1251);
	EN_RU_UA_txt2.from_codepage("CP1251").outputl("<");

	// Test throwing MvException on wrong locale name
	// Uncomment following line to test
	//	oswrite( EN_GREEK_utf8, EN_GREEK_file, "some_bad_locale");

	//  this code fragment tests locale specific characters IO with UTF8 files
	var MIXTURE_txt1 = "[English]" ^ "5bd0f3f1f1eae8e95d5bd3eaf0e0bfedf1fceae05d"_var.iconv("HEX") ^ "[Greek Char:\\u03A3][\u03A3]\n";
	var MIXTURE_txt2;
	var MIXTURE_file = "t_test_MIXTURE.txt";
	assert(oswrite(MIXTURE_txt1, MIXTURE_file, "utf8"));
	MIXTURE_file.osclose();
	assert(MIXTURE_txt2.osread(MIXTURE_file, "utf8"));
	MIXTURE_txt1.outputl("Written   text:");
	MIXTURE_txt2.outputl("Read back text:");

	//printl( "Press Enter key to continue ...");
	//c = getchar();

	//  this code fragment tests positioning in UTF8 files
	var BUF_0_9, BUF_9_9, BUF_18_12, BUF_19_12, BUF_20_12, BUF_30_23;
	var position = 0;
	assert(BUF_0_9.osbread(MIXTURE_file, position, 9));
	position = 9;
	assert(BUF_9_9.osbread(MIXTURE_file, position, 9));
	position = 18;
	assert(BUF_18_12.osbread(MIXTURE_file, position, 12));
	position = 19;
	assert(BUF_19_12.osbread(MIXTURE_file, position, 12));
	position = 20;
	assert(BUF_20_12.osbread(MIXTURE_file, position, 12));
	position = 30;
	assert(BUF_30_23.osbread(MIXTURE_file, position, 23));
//	BUF_9_9=[][][Gree
//	BUF_18_12=k Char:\u03A
//	BUF_19_12= Char:\u03A3
//	BUF_30_23=3][Σ]
	assert(BUF_0_9.outputl("BUF_0_9=") == "[English]");
	assert(BUF_9_9.outputl("BUF_9_9=") == "[][][Gree");
	assert(BUF_18_12.outputl("BUF_18_12=") == "k Char:\\u03A");
	assert(BUF_19_12.outputl("BUF_19_12=") == " Char:\\u03A3");
	assert(BUF_30_23.outputl("BUF_30_23=") == "3][Σ]\n");//335D5BCEA35D0A

	position = 0;
	assert(BUF_0_9.osbread(MIXTURE_file, position, 9));
	assert(BUF_9_9.osbread(MIXTURE_file, position, 9));
	assert(BUF_18_12.osbread(MIXTURE_file, position, 12));
	assert(BUF_30_23.osbread(MIXTURE_file, position, 23));
//	BUF_0_9=[English]
//	BUF_9_9=[][][Gree
//	BUF_18_12=k Char:\u03A
//	BUF_30_23=3][Σ]
	assert(BUF_0_9.outputl("BUF_0_9=") == "[English]");
	assert(BUF_9_9.outputl("BUF_9_9=") == "[][][Gree");
	assert(BUF_18_12.outputl("BUF_18_12=") == "k Char:\\u03A");
	assert(BUF_30_23.outputl("BUF_30_23=") == "3][Σ]\n");

	{
		var tfilename = "t_test.txt";
		assert(var(L"abc")                                                                                                                           eq "abc");
		assert(var(L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.") eq "Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.");
		var x = L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.";
		assert(x.replace("[[:alnum:]]"_rex, "").convert(", ", "").outputl()                                                                        eq "/.");

		assert(oswrite(L"Α α, Β β, Γ γ, Δ δ, Ε ε, Ζ ζ, Η η, Θ θ, Ι ι, Κ κ, Λ λ, Μ μ, Ν ν, Ξ ξ, Ο ο, Π π, Ρ ρ, Σ σ/ς, Τ τ, Υ υ, Φ φ, Χ χ, Ψ ψ, and Ω ω.", tfilename));
		assert(osfile(tfilename));
		assert(osread(tfilename) eq x);
	}

	var cpdata		= "d4f0e0e7e020b3e720323220f1e8ece2eeebb3e2"_var.iconv("HEX");
	var OUTPUT_file = "t_test_OUTPUT_UTF8.txt";
	assert(oswrite("", OUTPUT_file, "utf8"));
	position = 5;
	assert(osbwrite(L"1234567890", OUTPUT_file, position));
	assert(position eq 15);
	assert(osbwrite(L"1234567890\n", OUTPUT_file, position));
	assert(position eq 26);
	assert(osbwrite(cpdata, OUTPUT_file, position));
	assert(position eq 46);
	assert(osbwrite(L"\n1234567890", OUTPUT_file, position));
	assert(position eq 57);
	assert(osbwrite(L"1234567890\n", OUTPUT_file, position));
	assert(position eq 68);
	position = 10;
	assert(osbwrite(L"XYZ", OUTPUT_file, position));
	assert(position eq 13);

	// Lets test osbwrite() with position update and default locale (1251)
	OUTPUT_file = "t_test_OUTPUT_1251.txt";
	assert(oswrite("", OUTPUT_file, ""));
	position = 5;
	assert(osbwrite(L"1234567890", OUTPUT_file, position));
	assert(position eq 15);
	assert(osbwrite(L"1234567890\n", OUTPUT_file, position));
	assert(position eq 26);

	assert(osbwrite(cpdata, OUTPUT_file, position));
	assert(position eq 46);
	assert(osbwrite(cpdata, OUTPUT_file, position));
	assert(position eq 66);
	assert(osbwrite(L"\n1234567890", OUTPUT_file, position));
	assert(position eq 77);
	assert(osbwrite(L"1234567890\n", OUTPUT_file, position));
	assert(position eq 88);
	position = 10;
	assert(osbwrite(L"XYZ", OUTPUT_file, position));
	assert(position eq 13);

	logputl("----------");
	{
		assert(var(123456.789).numberinwords("").outputl()        eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(123456.789).numberinwords("english").outputl() eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(-123456.789).numberinwords("").outputl()        eq "minus one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(-123456.789).numberinwords("english").outputl() eq "minus one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
		assert(var(10000000000000).numberinwords("").outputl()        eq "ten trillion");
		assert(var(10000000000000).numberinwords("english").outputl() eq "ten trillion");
		assert(var(1.2345678901).numberinwords("").outputl()        eq "one point two three four five six seven eight nine zero one");
		assert(var(1.2345678901).numberinwords("english").outputl() eq "one point two three four five six seven eight nine zero one");

		assert(var(123456.789).numberinwords("BADLOCALE").outputl()        eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");
	}
	logputl("----------");

//	// Skip due to taking long time to generate locales
//	var ghws = osgetenv("GITHUB_WORKSPACE");
//	TRACE(ghws)
//	if (not ghws)
	{
		printl();
		printl("===============");
		assert(var(123456.789).numberinwords("en_AU").outputl() eq "one hundred twenty-three thousand four hundred fifty-six point seven eight nine");

		assert(var(123456.789).numberinwords("greek").outputl()   eq "εκατόν είκοσι τρεις χίλιάδες τετρακόσια πενήντα έξι κόμμα επτά οκτώ εννέα");
		assert(var(123456.789).numberinwords("arabic").outputl()  eq "مائة و ثلاثة و عشرون ألف و أربعة مائة و ستة و خمسون فاصل سبعة ثمانية تسعة"
			or var(123456.789).numberinwords("arabic").outputl()  eq "مائة وثلاثة وعشرون ألف وأربعة مائة وستة وخمسون فاصل سبعة ثمانية تسعة");
		assert(var(123456.789).numberinwords("french").outputl()  eq "cent vingt-trois mille quatre cent cinquante-six virgule sept huit neuf");
		assert(var(123456.789).numberinwords("spanish").outputl() eq "ciento veintitrés mil cuatrocientos cincuenta y seis punto siete ocho nueve");

		printl();
		printl("===============");

		assert(var(-123456.789).numberinwords("greek").outputl()   eq "μείον εκατόν είκοσι τρεις χίλιάδες τετρακόσια πενήντα έξι κόμμα επτά οκτώ εννέα");
		assert(var(-123456.789).numberinwords("arabic").outputl()  eq "ناقص مائة و ثلاثة و عشرون ألف و أربعة مائة و ستة و خمسون فاصل سبعة ثمانية تسعة"
			or var(-123456.789).numberinwords("arabic").outputl()  eq "ناقص مائة وثلاثة وعشرون ألف وأربعة مائة وستة وخمسون فاصل سبعة ثمانية تسعة");
		assert(var(-123456.789).numberinwords("french").outputl()  eq "moins cent vingt-trois mille quatre cent cinquante-six virgule sept huit neuf");
		assert(var(-123456.789).numberinwords("spanish").outputl() eq "menos ciento veintitrés mil cuatrocientos cincuenta y seis punto siete ocho nueve");

		printl();
		printl("===============");

		assert(var(10000000000000).numberinwords("greek").outputl()   eq "δέκα τρισεκατομμύρια");
		assert(var(10000000000000).numberinwords("arabic").outputl()  eq "عشرة ترليون");
		assert(var(10000000000000).numberinwords("french").outputl()  eq "dix billions");
		assert(var(10000000000000).numberinwords("spanish").outputl() eq "diez billones");

		printl();
		printl("===============");

		assert(var(1.2345678901).numberinwords("greek").outputl()   eq "ένα κόμμα δύο τρία τέσσερα πέντε έξι επτά οκτώ εννέα μηδέν ένα");
		assert(var(1.2345678901).numberinwords("arabic").outputl()  eq "واحد فاصل إثنان ثلاثة أربعة خمسة ستة سبعة ثمانية تسعة صفر واحد");
		assert(var(1.2345678901).numberinwords("french").outputl()  eq "un virgule deux trois quatre cinq six sept huit neuf zéro un");
		assert(var(1.2345678901).numberinwords("spanish").outputl() eq "uno punto dos tres cuatro cinco seis siete ocho nueve cero uno");
	}

var locales =R"V0G0N(
af-ZA
am-ET
ar-AE
ar-BH
ar-DZ
ar-EG
ar-IQ
ar-JO
ar-KW
ar-LB
ar-LY
ar-MA
arn-CL
ar-OM
ar-QA
ar-SA
ar-SD
ar-SY
ar-TN
ar-YE
as-IN
az-az
az-Cyrl-AZ
az-Latn-AZ
ba-RU
be-BY
bg-BG
bn-BD
bn-IN
bo-CN
br-FR
bs-Cyrl-BA
bs-Latn-BA
ca-ES
co-FR
cs-CZ
cy-GB
da-DK
de-AT
de-CH
de-DE
de-LI
de-LU
dsb-DE
dv-MV
el-CY
el-GR
en-029
en-AU
en-BZ
en-CA
en-cb
en-GB
en-IE
en-IN
en-JM
en-MT
en-MY
en-NZ
en-PH
en-SG
en-TT
en-US
en-ZA
en-ZW
es-AR
es-BO
es-CL
es-CO
es-CR
es-DO
es-EC
es-ES
es-GT
es-HN
es-MX
es-NI
es-PA
es-PE
es-PR
es-PY
es-SV
es-US
es-UY
es-VE
et-EE
eu-ES
fa-IR
fi-FI
fil-PH
fo-FO
fr-BE
fr-CA
fr-CH
fr-FR
fr-LU
fr-MC
fy-NL
ga-IE
gd-GB
gd-ie
gl-ES
gsw-FR
gu-IN
ha-Latn-NG
he-IL
hi-IN
hr-BA
hr-HR
hsb-DE
hu-HU
hy-AM
id-ID
ig-NG
ii-CN
in-ID
is-IS
it-CH
it-IT
iu-Cans-CA
iu-Latn-CA
iw-IL
ja-JP
ka-GE
kk-KZ
kl-GL
km-KH
kn-IN
kok-IN
ko-KR
ky-KG
lb-LU
lo-LA
lt-LT
lv-LV
mi-NZ
mk-MK
ml-IN
mn-MN
mn-Mong-CN
moh-CA
mr-IN
ms-BN
ms-MY
mt-MT
nb-NO
ne-NP
nl-BE
nl-NL
nn-NO
no-no
nso-ZA
oc-FR
or-IN
pa-IN
pl-PL
prs-AF
ps-AF
pt-BR
pt-PT
qut-GT
quz-BO
quz-EC
quz-PE
rm-CH
ro-mo
ro-RO
ru-mo
ru-RU
rw-RW
sah-RU
sa-IN
se-FI
se-NO
se-SE
si-LK
sk-SK
sl-SI
sma-NO
sma-SE
smj-NO
smj-SE
smn-FI
sms-FI
sq-AL
sr-BA
sr-CS
sr-Cyrl-BA
sr-Cyrl-CS
sr-Cyrl-ME
sr-Cyrl-RS
sr-Latn-BA
sr-Latn-CS
sr-Latn-ME
sr-Latn-RS
sr-ME
sr-RS
sr-sp
sv-FI
sv-SE
sw-KE
syr-SY
ta-IN
te-IN
tg-Cyrl-TJ
th-TH
tk-TM
tlh-QS
tn-ZA
tr-TR
tt-RU
tzm-Latn-DZ
ug-CN
uk-UA
ur-PK
uz-Cyrl-UZ
uz-Latn-UZ
uz-uz
vi-VN
wo-SN
xh-ZA
yo-NG
zh-CN
zh-HK
zh-MO
zh-SG
zh-TW
zu-ZA
)V0G0N";
	//var locales = osshellread("locale -a").convert("\n", FM);
	locales.converter("\n", FM);
	TRACE(locales)
	for (var locale : locales) {
		printl(locale ^ " 123456.78: " ^var(123456.78).numberinwords(locale));
	}

	{
		// Count unicode points (not the same as graphemes since unicode points may combine into one visible character)
		var en = "one point two three four five six seven eight nine zero one xx";
		var gr = "ένα κόμμα δύο τρία τέσσερα πέντε έξι επτά οκτώ εννέα μηδέν ένα";
		var ar = "واحد فاصل إثنان ثلاثة أربعة خمسة ستة سبعة ثمانية تسعة صفر واحد";

		assert(en.textlen() eq 62);
		assert(gr.textlen() eq 62);
		assert(ar.textlen() eq 62);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
