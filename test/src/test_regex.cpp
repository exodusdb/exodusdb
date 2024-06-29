#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#define BOOST_REGEX 1

#include <exodus/program.h>
programinit()

	function main() {

	{
		// Demo simplicity of exodus regex
		//
		// Compare to standard c++ example of the same at
		// https://en.cppreference.com/w/cpp/regex
		//

		// Sample text
		let v1 = "Some people, when confronted with a problem, think "
			"\"I know, I'll use regular expressions.\" "
			"Now they have two problems.";

		// Case insensitive match
		if (v1.match("REGULAR EXPRESSIONS", "i"))
			printl("Text contains the phrase 'regular expressions");

		// Count occurrences
		//println("Found {} words", v1.match("\\w+").fcount(FM));
		//println("Found {:} words", v1.match("\\w+").fcount(FM));
		//let nwords = v1.match("\\w+").fcount(FM);
		int nwords = v1.match("\\w+").fcount(FM);
		println("Found {} words", nwords);

// clang 15 on u22.04 has a problem with the above var in println

///root/exodus/test/src/test_regex.cpp:28:11: error: call to consteval function 'fmt::basic_format_string<char, exodus::var>::basic_format_string<cha
//r[15], 0>' is not a constant expression                                                                                                            
//                println("Found {} words", v1.match("\\w+").fcount(FM));                                                                            
//                        ^                                                                                                                          
///root/exodus/test/src/../../fmt/include/fmt/core.h:2561:12: note: non-literal type 'formatter<mapped_type, char_type>' (aka 'formatter<exodus::var,
// char>') cannot be used in a constant expression                                                                                                   
//    return formatter<mapped_type, char_type>().parse(ctx);               
//           ^                                                             
///root/exodus/test/src/../../fmt/include/fmt/core.h:2654:39: note: in call to 'parse_format_specs(checker(s).context_)'                             
//    return id >= 0 && id < num_args ? parse_funcs_[id](context_) : begin;                                                                          
//                                      ^
///root/exodus/test/src/../../fmt/include/fmt/core.h:2647:5: note: in call to '&checker(s)->on_format_specs(0, &"Found {} words"[7], &"Found {} words
//"[7])'
//    on_format_specs(id, begin, begin);  // Call parse() on empty specs.
//    ^
///root/exodus/test/src/../../fmt/include/fmt/core.h:2472:13: note: in call to '&checker(s)->on_replacement_field(0, &"Found {} words"[7])'
//    handler.on_replacement_field(handler.on_arg_id(), begin);
//            ^
///root/exodus/test/src/../../fmt/include/fmt/core.h:2504:21: note: in call to 'parse_replacement_field(&"Found {} words"[7], &"Found {} words"[14], 
//checker(s))'
//        begin = p = parse_replacement_field(p - 1, end, handler);
//                    ^
///root/exodus/test/src/../../fmt/include/fmt/core.h:2757:7: note: in call to 'parse_format_string({&"Found {} words"[0], 14}, checker(s))'
//      detail::parse_format_string<true>(str_, checker(s));
//      ^
///root/exodus/test/src/test_regex.cpp:28:11: note: in call to 'basic_format_string("Found {} words")'
//                println("Found {} words", v1.match("\\w+").fcount(FM));
//                        ^
//

		// Use search loop to process occurrences. Could allow complex amendments.
		var n = 6;
		printl("Words longer than", n, "characters:");
		var index = 1;
		while (index < v1.len()) {
			var word = v1.search("\\w+", index);
			if (word.len() > n)
				printl(word);
		}
		// Although could just use match "\w{7,}"
		//printl("Words longer than", n, "characters:\n" ^ v1.match("\\w{7,}").convert(FM, "\n"));

		// Replace large words
		printl(v1.replace("(\\w{7,})"_rex, "[$&]"));

	}
	{
		// first
		assert(replace("aaa bbb ccc", rex("b", "f"), "Q") eq "aaa Qbb ccc");

		// all
		assert(replace("aaa bbb ccc", "b"_rex, "Q") eq "aaa QQQ ccc");

		// case insensitive
		assert(replace("aAa bbb ccc", rex("a", "i"), "Q") eq "QQQ bbb ccc");

		// single-line. ^ only matches beginning of string and not any embedded \n chars
		assert(replace("aaa\nbbb\nccc\n", rex("^b.*?$", "s"), "QQQ").outputl() eq "aaa\nbbb\nccc\n");

		// multi-line ^ matches beginning of string AND any embedded \n chars
		assert(replace("aaa\nbbb\nccc\n", rex("^b.*?$", "m"), "QQQ").outputl() eq "aaa\nQQQ\nccc\n");

		// l - literal - no special regex characters ... but better to use var::replace() for this
		assert(replace("aaa\n***\nccc\n", rex("*", "l"), "Q").outputl() eq "aaa\nQQQ\nccc\n");

		// lif - literal, case insensitive and first only
		assert(replace("aaa\nb*b*b\nccc\n", rex("B*", "fil"), "Q").outputl() eq "aaa\nQb*b\nccc\n");

	}

	//match returning what it finds

	//groups
	var csvline1 = "abcdef abcdef";
	var csvre1	 = "(bc).(ef)";
	// bcdef]bc]ef^bcdef]bc]ef
	assert(match(csvline1, csvre1).convert(_FM _VM, "^]").outputl() eq "bcdef]bc]ef^bcdef]bc]ef");

	//no groups
	var csvline2 = "abcdef abcdef";
	var csvre2	 = "bc.ef";
	assert(match(csvline2, csvre2).convert(_FM _VM, "^]") eq "bcdef^bcdef");

	//Gives the following error when using std::regex. No such error with boost:regex
	//Error: Invalid regex "(?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$)" "Invalid special open parenthesis."
	//
	//not asserted but a complicated CSV match
	var csvline = R"(123,2.99,AMO024,Title,"Description, more info",,123987564)";
	var csvre	= R"tag((?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$))tag";

	var notposix_extended = R"(\(XYZ\))";  //escaped () will indicate groups to be captured
	var posix_extended	  = R"((XYZ))";	   //unescaped () will match () in the source string

	assert("raw(XYZ)raw"_var.match(posix_extended, "i").outputl()    eq "XYZ]XYZ"_var);
	assert("raw(XYZ)raw"_var.match(notposix_extended, "i").outputl() eq "(XYZ)"_var);

	// no need to backslash many letters

#define BOOST_REGEX 1
	{

		// \d \D and [:number:]
		assert(var("a1b23c99").replace("\\d"_rex, "N")            eq "aNbNNcNN");
		assert(var("a1b23c99").replace("\\D"_rex, "N")            eq "N1N23N99");
		if (BOOST_REGEX) {
			assert(var("a1b23c99").replace("[[:number:]]"_rex, "N")  eq "aNbNNcNN");
			assert(var("a1b23c99").replace("[^[:number:]]"_rex, "N") eq "N1N23N99");
		}

		// [:alnum:] = letters and numbers. No _
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").replace("[[:alnum:]]"_rex, "N").outputl()  eq "NNNN_ \nNNNN");
			assert(var("a1b2_ \n3c99").replace("[^[:alnum:]]"_rex, "N").outputl() eq "a1b2NNN3c99");
		}

		// \w and \W in alnum + _
		assert(var("a1b2_ \n3c99").replace("\\w"_rex, "N").outputl()            eq "NNNNN \nNNNN");
		assert(var("a1b2_ \n3c99").replace("\\W"_rex, "N").outputl()            eq "a1b2_NN3c99");
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").replace("[_[:alnum:]]"_rex, "N").outputl()  eq "NNNNN \nNNNN");
			assert(var("a1b2_ \n3c99").replace("[^_[:alnum:]]"_rex, "N").outputl() eq "a1b2_NN3c99");
		}

		// \s \S and [[:space:]]  means whitespace
		assert(var("a1b2_ \n3c99").replace("\\s"_rex, "N").outputl()           eq "a1b2_NN3c99");
		assert(var("a1b2_ \n3c99").replace("\\S"_rex, "N").outputl()           eq "NNNNN \nNNNN");
		if (BOOST_REGEX) {
			assert(var("a1b2_ \n3c99").replace("[[:space:]]"_rex, "N").outputl()  eq "a1b2_NN3c99");
			assert(var("a1b2_ \n3c99").replace("[^[:space:]]"_rex, "N").outputl() eq "NNNNN \nNNNN");
		}

		if (BOOST_REGEX) {
			// \{Number}
			assert(var("a1b2_ \n3c99").replace("\\p{Number}"_rex, "N").outputl() eq "aNbN_ \nNcNN");
		}
	}

	if (BOOST_REGEX) {
		//123]]123^,2.99]]2.99^,AMO024]]AMO024^,Title]]Title^,"Description, more info"]"]Description, more info^,]]^,123987564]]123987564
		assert(match(csvline, csvre).convert(_FM _VM, "^]").outputl() eq R"raw(123]]123^,2.99]]2.99^,AMO024]]AMO024^,Title]]Title^,"Description, more info"]"]Description, more info^,]]^,123987564]]123987564)raw");

		//unicode case insensitive finding
		assert(match("αβγδεΑΒΓΔΕ", "(Α).(γδ)", "i").convert(_FM _VM, "^]") eq "αβγδ]α]γδ^ΑΒΓΔ]Α]ΓΔ");

		//unicode case sensitive NOT finding
		assert(match("αβγδεΑΒΓΔΕ", "(Α).(γδ)", "").convert(_FM _VM, "^]") eq "");
	}

	var r1 = _FM "0.123";
	assert(r1.replace("([\x1A-\x1F]-?)0."_rex, "$1.") eq _FM ".123");
	var r2 = _ST "-0.123";
	assert(r2.replace("([\x1A-\x1F]-?)0."_rex, "$1.") eq _ST "-.123");

	//replacing unicode style numbers characters using javascript style regex
	//assert(var("Ⅻ").replace(R"(\p{Number})"_rex,"yes")=="yes");
	//assert(var("⅝").replace(R"(\p{Number})"_rex,"yes")=="yes");
	assert(var("1").replace(R"([[:digit:]])"_rex, "yes") eq "yes");
	if (BOOST_REGEX) {
		assert(var("Ⅻ").replace(R"(\p{Number})"_rex, "yes") eq "yes");
		assert(var("⅝").replace(R"(\p{Number})"_rex, "yes") eq "yes");

		//assert(var("Ⅻ").replace(R"(\p{Number})"_rex,"yes").outputl()!="yes");
		//assert(var("⅝").replace(R"(\p{Number})"_rex,"yes").outputl()!="yes");
		assert(var("Ⅻ").replace(R"([[:digit:]])"_rex, "yes").outputl() ne "yes");
		assert(var("⅝").replace(R"([[:digit:]])"_rex, "yes").outputl() ne "yes");
	}

	//test glob matching using * ? eg *.* and *.??? etc
	assert(var("test.htm").match("*.*", "w").outputl() eq "test.htm");
	assert(var("test.htm").match("*", "w")             eq "test.htm");
	assert(var("test.htm").match(".*", "w")            eq "");
	assert(var("test.htm").match(".", "w")             eq "");
	assert(var("test.htm").match("*.", "w")            eq "");
	assert(var("test.htm").match("*.htm", "w")         eq "test.htm");
	assert(var("test.html").match("*.htm", "w")        eq "");
	assert(var("test.htm").match("t*.???", "w")        eq "test.htm");
	assert(var("test.htm").match("t.???", "w")         eq "");
	assert(var("test.htm").match("x.???", "w")         eq "");
	assert(var("testx.htm").match("*x.???", "w")       eq "testx.htm");
	assert(var("test.html").match("t*.???", "w")       eq "");
	assert(var("test.html").match("*t?h*", "w")        eq "test.html");

	//test regular expression
	//four digits followed by dash or space) three times ... followed by four digits
	var regex1 = "(\\d{4}[- ]){3}\\d{4}";
	assert(var("1247-1234-1234-1234").match(regex1, "r").convert(_FM _VM, "^]") eq "1247-1234-1234-1234]1234-");
	assert(var("1247.1234-1234-1234").match(regex1, "r")                        eq "");

	printl(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF"));
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match("文字.*漢字\\(UTF")         eq "文字- Kanji 漢字- Hanja 漢字(UTF");
	assert(var("Unicode table CJK 1: Chinese 文字- Kanji 漢字- Hanja 漢字(UTF-8)").match(".*文字.*漢 字\\(UTF-8\\)") eq "");

	{
		//test regex group and use group in replace
		//replace char+space with x+char+dash
		//TODO add g option
		var text = "what a lot of money";
		assert(text.replace("(.) "_rex, "x$1-") eq "whaxt-xa-loxt-oxf-money");

		//simple test of regex and case insensitive regex swap (commonly known as replace)
		assert(replace("abcd", "b."_rex, "xyz").outputl() eq "axyzd");	   //right case to convert
		assert(replace("abc", "B."_rex, "xyz").outputl() eq "abc");		   //wrong case to convert
		assert(replace("abcd", rex("B.", "i"), "xyz").outputl() eq "axyzd");  //case insensitive converts
		if (BOOST_REGEX)
			assert(replace("abc", rex("b.", "l"), "xyz").outputl() eq "abc");	//literal wont convert

		//simple test of case sensitive/insensitive swap
		assert(replace("abc", "b", "xyz").outputl() eq "axyzc");				 //will convert right case
		assert(replace("abc", "B", "xyz").outputl() eq "abc");					 //wont convert wrong case
		assert(replace("abc", rex("B", "i"), "xyz").outputl() eq "axyzc");	 //will convert case insensitive
		if (BOOST_REGEX)
			assert(replace("ab*c", rex("B*", "il"), "xyz").outputl() eq "axyzc");	//will convert case insensitive but not regex

		assert(replace("abababab", "ab", "x").outputl()           eq "xxxx");
		assert(replace("abababab", "ab"_rex, "x").outputl() eq "xxxx");		//regex
		assert(replace("abababab", "a."_rex, "xy").outputl() eq "xyxyxyxy");	//regex
	}

	{
		var utf8 = "αβγδεΑΒΓΔΕ";
		var notutf8 = "       \xff"_var;
		TRACE(notutf8.oconv("HEX"))

		// MATCHING

		// bad data
		try {
			notutf8.match("X").errputl();
			assert(false);
		} catch (VarError& e) {errputl(e.description);}

		// bad target
		try {
			utf8.match(notutf8).errputl();
			assert(false);
		} catch (VarError& e) {errputl(e.description);}

		// REPLACING

		// bad data
		try {
			notutf8.replace("X"_rex, "Y").errputl();
			assert(false);
		} catch (VarError& e) {errputl(e.description);}

		// bad target
		try {
			utf8.replace(rex(notutf8), "Y").errputl();
			assert(false);
		} catch (VarError& e) {errputl(e.description);}

		// bad replacement
		try {
			utf8.replace("X"_rex, notutf8).errputl();
			assert(false);
		} catch (VarError& e) {errputl(e.description);}
	}

	{

		// From https://en.cppreference.com/w/cpp/regex/syntax_option_type

		let str = "zzxayyzz";
		let re = ".*(a|xayy)";

		//n - Test nosubs option
		assert(str.match(re, "n").outputl("ECMA nosubs=") == "zzxa");
		assert(str.match(re).outputl("ECMA=") == "zzxa]a"_var);

		// default options
		// ECMA (depth first search) match: zzxa
		printl("Searching for .*(a|xayy) in zzxayyzz:");
		printl("ECMA (depth first search) match:", str.match(re));

		// e - extended posix
		// EXTENDED POSIX (leftmost longest)  match: zzxayy
		printl("POSIX (leftmost longest)  match:", str.match(re, "e"));

		//ECMA (depth first search) match: zzxa
		assert(str.match(re).f(1,1).outputl() == "zzxa");

		// b - basic posix
		// BASIC POSIX (leftmost longest)  match: zzxayy
		// TODO why returns empty?
		// assert(str.match(re, "b").f(1,1).outputl() == "zzxayy");

		// e - extended posix
		// EXTENDED POSIX (leftmost longest)  match: zzxayy
		assert(str.match(re, "e").f(1,1).outputl() == "zzxayy");

		// a - awk
		// like EXTENDED POSIX (leftmost longest)  match: zzxayy
		assert(str.match(re, "a").f(1,1).outputl() == "zzxayy");

		// g - grep
		// like EXTENDED POSIX (leftmost longest)  match: zzxayy
		// TODO why returns empty?
		// assert(str.match(re, "g").f(1,1).outputl() == "zzxayy");

	}

	{

		// Check progressive search

		// Finds all b's
		assert(let("aaabBbccc").match("b", "i").outputl() == "b^B^b"_var);

		// f = find/format_first_only
		assert(let("aaabBbccc").match("b", "fi").outputl() == "b");

		// Find b's progressively
		var index;
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "b");
		assert(index == 5);
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "B");
		assert(index == 6);
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "b");
		assert(index == 7);
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "");
		//assert(index == 10);
		assert(index == 0);

//		// Repeat after failure remains a failure
//		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "");
//		assert(index == 10);

		// Search from past end finds nothing and returns length + 1
		index = 99;
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "");
//		assert(index == 10);
		assert(index == 0);

		// Negative (and zero) starts from beginning
		index = -99;
		assert(let("aaabBbccc").search("b", index, "i").outputl(index ^ " ") == "b");
		assert(index == 5);

	}

	{
		// I can eat glass
		let iceg = R"--(Sanskrit: ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥
Sanskrit (standard transcription): kācaṃ śaknomyattum; nopahinasti mām.
Classical Greek: ὕαλον ϕαγεῖν δύναμαι· τοῦτο οὔ με βλάπτει.
Greek (monotonic): Μπορώ να φάω σπασμένα γυαλιά χωρίς να πάθω τίποτα.
Greek (polytonic): Μπορῶ νὰ φάω σπασμένα γυαλιὰ χωρὶς νὰ πάθω τίποτα.
Etruscan: (NEEDED)
Latin: Vitrum edere possum; mihi non nocet.
Old French: Je puis mangier del voirre. Ne me nuit.
French: Je peux manger du verre, ça ne me fait pas mal.
Provençal / Occitan: Pòdi manjar de veire, me nafrariá pas.
Québécois: J'peux manger d'la vitre, ça m'fa pas mal.
Walloon: Dji pou magnî do vêre, çoula m' freut nén må.
Champenois: (NEEDED)
Lorrain: (NEEDED)
Picard: Ch'peux mingi du verre, cha m'foé mie n'ma.
Corsican/Corsu: (NEEDED)
Jèrriais: (NEEDED)
Kreyòl Ayisyen (Haitï): Mwen kap manje vè, li pa blese'm.
Basque: Kristala jan dezaket, ez dit minik ematen.
Catalan / Català: Puc menjar vidre, que no em fa mal.
Spanish: Puedo comer vidrio, no me hace daño.
Aragonés: Puedo minchar beire, no me'n fa mal .
Aranés: (NEEDED)
Mallorquín: (NEEDED)
Galician: Eu podo xantar cristais e non cortarme.
European Portuguese: Posso comer vidro, não me faz mal.
Brazilian Portuguese (8): Posso comer vidro, não me machuca.
Caboverdiano/Kabuverdianu (Cape Verde): M' podê cumê vidru, ca ta maguâ-m'.
Papiamentu: Ami por kome glas anto e no ta hasimi daño.
Italian: Posso mangiare il vetro e non mi fa male.
Milanese: Sôn bôn de magnà el véder, el me fa minga mal.
Roman: Me posso magna' er vetro, e nun me fa male.
Napoletano: M' pozz magna' o'vetr, e nun m' fa mal.
Venetian: Mi posso magnare el vetro, no'l me fa mae.
Zeneise (Genovese): Pòsso mangiâ o veddro e o no me fà mâ.
Sicilian: Puotsu mangiari u vitru, nun mi fa mali.
Campinadese (Sardinia): (NEEDED)
Lugudorese (Sardinia): (NEEDED)
Romansch (Grischun): Jau sai mangiar vaider, senza che quai fa donn a mai.
Romany / Tsigane: (NEEDED)
Romanian: Pot să mănânc sticlă și ea nu mă rănește.
Esperanto: Mi povas manĝi vitron, ĝi ne damaĝas min.
Pictish: (NEEDED)
Breton: (NEEDED)
Cornish: Mý a yl dybry gwéder hag éf ny wra ow ankenya.
Welsh: Dw i'n gallu bwyta gwydr, 'dyw e ddim yn gwneud dolur i mi.
Manx Gaelic: Foddym gee glonney agh cha jean eh gortaghey mee.
Old Irish (Ogham): ᚛᚛ᚉᚑᚅᚔᚉᚉᚔᚋ ᚔᚈᚔ ᚍᚂᚐᚅᚑ ᚅᚔᚋᚌᚓᚅᚐ᚜
Old Irish (Latin): Con·iccim ithi nglano. Ním·géna.
Irish: Is féidir liom gloinne a ithe. Ní dhéanann sí dochar ar bith dom.
Ulster Gaelic: Ithim-sa gloine agus ní miste damh é.
Scottish Gaelic: S urrainn dhomh gloinne ithe; cha ghoirtich i mi.
Anglo-Saxon (Runes): ᛁᚳ᛫ᛗᚨᚷ᛫ᚷᛚᚨᛋ᛫ᛖᚩᛏᚪᚾ᛫ᚩᚾᛞ᛫ᚻᛁᛏ᛫ᚾᛖ᛫ᚻᛖᚪᚱᛗᛁᚪᚧ᛫ᛗᛖ᛬
Anglo-Saxon (Latin): Ic mæg glæs eotan ond hit ne hearmiað me.
Middle English: Ich canne glas eten and hit hirtiþ me nouȝt.
English: I can eat glass and it doesn't hurt me.
English (IPA): [aɪ kæn iːt glɑːs ænd ɪt dɐz nɒt hɜːt miː] (Received Pronunciation)
English (Braille): ⠊⠀⠉⠁⠝⠀⠑⠁⠞⠀⠛⠇⠁⠎⠎⠀⠁⠝⠙⠀⠊⠞⠀⠙⠕⠑⠎⠝⠞⠀⠓⠥⠗⠞⠀⠍⠑
Jamaican: Mi kian niam glas han i neba hot mi.
Lalland Scots / Doric: Ah can eat gless, it disnae hurt us.
Glaswegian: (NEEDED)
Gothic (4): 𐌼𐌰𐌲 𐌲𐌻𐌴𐍃 𐌹̈𐍄𐌰𐌽, 𐌽𐌹 𐌼𐌹𐍃 𐍅𐌿 𐌽𐌳𐌰𐌽 𐌱𐍂𐌹𐌲𐌲𐌹𐌸.
Old Norse (Runes): ᛖᚴ ᚷᛖᛏ ᛖᛏᛁ ᚧ ᚷᛚᛖᚱ ᛘᚾ ᚦᛖᛋᛋ ᚨᚧ ᚡᛖ ᚱᚧᚨ ᛋᚨᚱ
Old Norse (Latin): Ek get etið gler án þess að verða sár.
Norsk / Norwegian (Nynorsk): Eg kan eta glas utan å skada meg.
Norsk / Norwegian (Bokmål): Jeg kan spise glass uten å skade meg.
Føroyskt / Faroese: Eg kann eta glas, skaðaleysur.
Íslenska / Icelandic: Ég get etið gler án þess að meiða mig.
Svenska / Swedish: Jag kan äta glas utan att skada mig.
Dansk / Danish: Jeg kan spise glas, det gør ikke ondt på mig.
Sønderjysk: Æ ka æe glass uhen at det go mæ naue.
Frysk / Frisian: Ik kin glês ite, it docht me net sear.
Nederlands / Dutch: Ik kan glas eten, het doet mĳ geen kwaad.
Kirchröadsj/Bôchesserplat: Iech ken glaas èèse, mer 't deet miech jing pieng.
Afrikaans: Ek kan glas eet, maar dit doen my nie skade nie.
Lëtzebuergescht / Luxemburgish: Ech kan Glas iessen, daat deet mir nët wei.
Deutsch / German: Ich kann Glas essen, ohne mir zu schaden.
Ruhrdeutsch: Ich kann Glas verkasematuckeln, ohne dattet mich wat jucken tut.
Langenfelder Platt: Isch kann Jlaas kimmeln, uuhne datt mich datt weh dääd.
Lausitzer Mundart ("Lusatian"): Ich koann Gloos assn und doas dudd merr ni wii.
Odenwälderisch: Iech konn glaasch voschbachteln ohne dass es mir ebbs daun doun dud.
Sächsisch / Saxon: 'sch kann Glos essn, ohne dass'sch mer wehtue.
Pfälzisch: Isch konn Glass fresse ohne dasses mer ebbes ausmache dud.
Schwäbisch / Swabian: I kå Glas frässa, ond des macht mr nix!
Deutsch (Voralberg): I ka glas eassa, ohne dass mar weh tuat.
Bayrisch / Bavarian: I koh Glos esa, und es duard ma ned wei.
Allemannisch: I kaun Gloos essen, es tuat ma ned weh.
Schwyzerdütsch (Zürich): Ich chan Glaas ässe, das schadt mir nöd.
Schwyzerdütsch (Luzern): Ech cha Glâs ässe, das schadt mer ned.
Plautdietsch: (NEEDED)
Hungarian: Meg tudom enni az üveget, nem lesz tőle bajom.
Suomi / Finnish: Voin syödä lasia, se ei vahingoita minua.
Sami (Northern): Sáhtán borrat lása, dat ii leat bávččas.
Erzian: Мон ярсан суликадо, ды зыян эйстэнзэ а ули.
Northern Karelian: Mie voin syvvä lasie ta minla ei ole kipie.
Southern Karelian: Minä voin syvvä st'oklua dai minule ei ole kibie.
Vepsian: (NEEDED)
Votian: (NEEDED)
Livonian: (NEEDED)
Estonian: Ma võin klaasi süüa, see ei tee mulle midagi.
Latvian: Es varu ēst stiklu, tas man nekaitē.
Lithuanian: Aš galiu valgyti stiklą ir jis manęs nežeidžia
Old Prussian: (NEEDED)
Sorbian (Wendish): (NEEDED)
Czech: Mohu jíst sklo, neublíží mi.
Slovak: Môžem jesť sklo. Nezraní ma.
Polska / Polish: Mogę jeść szkło i mi nie szkodzi.
Slovenian: Lahko jem steklo, ne da bi mi škodovalo.
Bosnian, Croatian, Montenegrin and Serbian (Latin): Ja mogu jesti staklo, i to mi ne šteti.
Bosnian, Montenegrin and Serbian (Cyrillic): Ја могу јести стакло, и то ми не штети.
Macedonian: Можам да јадам стакло, а не ме штета.
Russian: Я могу есть стекло, оно мне не вредит.
Belarusian (Cyrillic): Я магу есці шкло, яно мне не шкодзіць.
Belarusian (Lacinka): Ja mahu jeści škło, jano mne ne škodzić.
Ukrainian: Я можу їсти скло, і воно мені не зашкодить.
Bulgarian: Мога да ям стъкло, то не ми вреди.
Georgian: მინას ვჭამ და არა მტკივა.
Armenian: Կրնամ ապակի ուտել և ինծի անհանգիստ չըներ։
Albanian: Unë mund të ha qelq dhe nuk më gjen gjë.
Turkish: Cam yiyebilirim, bana zararı dokunmaz.
Turkish (Ottoman): جام ييه بلورم بڭا ضررى طوقونمز
Tatar: Алам да бар, пыяла, әмма бу ранит мине.
Uzbek / O’zbekcha: (Roman): Men shisha yeyishim mumkin, ammo u menga zarar keltirmaydi.
Uzbek / Ўзбекча (Cyrillic): Мен шиша ейишим мумкин, аммо у менга зарар келтирмайди.
Bangla / Bengali: আমি কাঁচ খেতে পারি, তাতে আমার কোনো ক্ষতি হয় না।
Marathi (masculine): मी काच खाऊ शकतो, मला ते दुखत नाही.
Marathi (feminine):   मी काच खाऊ शकते, मला ते दुखत नाही.
Kannada: ನನಗೆ ಹಾನಿ ಆಗದೆ, ನಾನು ಗಜನ್ನು ತಿನಬಹುದು
Hindi (masculine): मैं काँच खा सकता हूँ और मुझे उससे कोई चोट नहीं पहुंचती.
Hindi (feminine):   मैं काँच खा सकती हूँ और मुझे उससे कोई चोट नहीं पहुंचती.
Malayalam: എനിക്ക് ഗ്ലാസ് തിന്നാം. അതെന്നെ വേദനിപ്പിക്കില്ല.
Tamil: நான் கண்ணாடி சாப்பிடுவேன், அதனால் எனக்கு ஒரு கேடும் வராது.
Telugu: నేను గాజు తినగలను మరియు అలా చేసినా నాకు ఏమి ఇబ్బంది లేదు
Sinhalese: මට වීදුරු කෑමට හැකියි. එයින් මට කිසි හානියක් සිදු නොවේ.
Urdu(3): میں کانچ کھا سکتا ہوں اور مجھے تکلیف نہیں ہوتی ۔
Pashto(3): زه شيشه خوړلې شم، هغه ما نه خوږوي
Farsi / Persian(3): .من می توانم بدونِ احساس درد شيشه بخورم
Arabic(3): أنا قادر على أكل الزجاج و هذا لا يؤلمني.
Aramaic: (NEEDED)
Maltese: Nista' niekol il-ħġieġ u ma jagħmilli xejn.
Hebrew(3): אני יכול לאכול זכוכית וזה לא מזיק לי.
Yiddish(3): איך קען עסן גלאָז און עס טוט מיר נישט װײ.
Judeo-Arabic: (NEEDED)
Ladino: (NEEDED)
Gǝʼǝz: (NEEDED)
Amharic: (NEEDED)
Twi: Metumi awe tumpan, ɜnyɜ me hwee.
Hausa (Latin): Inā iya taunar gilāshi kuma in gamā lāfiyā.
Hausa (Ajami) (2): إِنا إِىَ تَونَر غِلَاشِ كُمَ إِن غَمَا لَافِىَا
Yoruba(4): Mo lè je̩ dígí, kò ní pa mí lára.
Lingala: Nakokí kolíya biténi bya milungi, ekosála ngáí mabé tɛ́.
(Ki)Swahili: Naweza kula bilauri na sikunyui.
Malay: Saya boleh makan kaca dan ia tidak mencederakan saya.
Tagalog: Kaya kong kumain nang bubog at hindi ako masaktan.
Chamorro: Siña yo' chumocho krestat, ti ha na'lalamen yo'.
Fijian: Au rawa ni kana iloilo, ia au sega ni vakacacani kina.
Javanese: Aku isa mangan beling tanpa lara.
Burmese (Unicode 4.0): က္ယ္ဝန္‌တော္‌၊က္ယ္ဝန္‌မ မ္ယက္‌စားနုိင္‌သည္‌။ ၎က္ရောင္‌့ ထိခုိက္‌မ္ဟု မရ္ဟိပာ။ (9)
Burmese (Unicode 5.0): ကျွန်တော် ကျွန်မ မှန်စားနိုင်တယ်။ ၎င်းကြောင့် ထိခိုက်မှုမရှိပါ။ (9)
Vietnamese (quốc ngữ): Tôi có thể ăn thủy tinh mà không hại gì.
Vietnamese (nôm) (4): 些 𣎏 世 咹 水 晶 𦓡 空 𣎏 害 咦
Khmer: ខ្ញុំអាចញុំកញ្ចក់បាន ដោយគ្មានបញ្ហារ
Lao: ຂອ້ຍກິນແກ້ວໄດ້ໂດຍທີ່ມັນບໍ່ໄດ້ເຮັດໃຫ້ຂອ້ຍເຈັບ.
Thai: ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ
Mongolian (Cyrillic): Би шил идэй чадна, надад хортой биш
Mongolian (Classic) (5): ᠪᠢ ᠰᠢᠯᠢ ᠢᠳᠡᠶᠦ ᠴᠢᠳᠠᠨᠠ ᠂ ᠨᠠᠳᠤᠷ ᠬᠣᠤᠷᠠᠳᠠᠢ ᠪᠢᠰᠢ
Dzongkha: (NEEDED)
Nepali: ﻿म काँच खान सक्छू र मलाई केहि नी हुन्‍न् ।
Tibetan: ཤེལ་སྒོ་ཟ་ནས་ང་ན་གི་མ་རེད།
Chinese: 我能吞下玻璃而不伤身体。
Chinese (Traditional): 我能吞下玻璃而不傷身體。
Taiwanese(6): Góa ē-tàng chia̍h po-lê, mā bē tio̍h-siong.
Japanese: 私はガラスを食べられます。それは私を傷つけません。
Korean: 나는 유리를 먹을 수 있어요. 그래도 아프지 않아요
Bislama: Mi save kakae glas, hemi no save katem mi.
Hawaiian: Hiki iaʻu ke ʻai i ke aniani; ʻaʻole nō lā au e ʻeha.
Marquesan: E koʻana e kai i te karahi, mea ʻā, ʻaʻe hauhau.
Inuktitut (10): ᐊᓕᒍᖅ ᓂᕆᔭᕌᖓᒃᑯ ᓱᕋᙱᑦᑐᓐᓇᖅᑐᖓ
Chinook Jargon: Naika məkmək kakshət labutay, pi weyk ukuk munk-sik nay.
Navajo: Tsésǫʼ yishą́ągo bííníshghah dóó doo shił neezgai da.
Cherokee (and Cree, Chickasaw, Cree, Micmac, Ojibwa, Lakota, Náhuatl, Quechua, Aymara, and other American languages): (NEEDED)
Garifuna: (NEEDED)
Gullah: (NEEDED)
Lojban: mi kakne le nu citka le blaci .iku'i le se go'i na xrani mi
Nórdicg: Ljœr ye caudran créneþ ý jor cẃran.
)--";

		var iceg1 = iceg.trim();
		var iceg2 = "";
		var index = 1;
		while (index <= iceg1.len()) {

			// Find word/not word chars
			var chars = iceg1.search("([\\w]+)|([^\\w]+)", index);

			var wchars = chars.f(1,2);
			var xchars = chars.f(1,3);
			var nl = xchars.index("\n");
			if (nl)
				xchars = "\\n";

			if (wchars.len())
				printx(wchars.quote(), " ");
			else
				printx(xchars.squote(), " ");
			if (nl)
				printl();

			iceg2 ^= chars.f(1,1);
		}

		assert(iceg2 == iceg1);

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
