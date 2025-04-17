#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

	// Maximum C++ int: 2147483647 (hex: 0x7fffffff)
	// Maximum Unicode code point: 1114111 (hex: U+10ffff)
	// Max int (2147483647) is greater than max Unicode code point (1114111) by 2146369536.

	{
		//check unicode is invalid from 0xD800-0xDFF (UTF16 encoding) and 0x110000 onwards
		assert(textchr(0xD7FF) ne "");
		assert(textchr(0xD800)   eq "");
		assert(textchr(0xDFFF)   eq "");
		assert(textchr(0xE000) ne "");
		assert(textchr(0x10FFFF) ne "");
		assert(textchr(0x110000) eq "");

	//	var cmd = "c:\\xwindowspath_test_main1_\\to\\xyz.exe arg1 arg2";
	//	//assert(var(to_oscmd_string(cmd) eq cmd.convert("\\",OSSLASH));
	//	// No conversion at the moment
	//	//assert(to_oscmd_string(cmd) eq cmd);
	//	assert(not osshell(cmd));

	}

	{
		printl("\U0001F600"); // This is the grinning face emoji
		printl(textchr(0x0001F600)); // This is the grinning face emoji

		// Maximum C++ int: 2147483647 (hex: 0x7fffffff)
		// Maximum Unicode code point: 1114111 (hex: U+10ffff)
		// Max int (2147483647) is greater than max Unicode code point (1114111) by 2146369536.

		// Check can create upper plane utf8 chars
		{
			textchr(0x0001F600).dump().errputl();
			assert("\U0001F600" == textchr(0x0001F600));
			assert(0x0001F600 == textord(textchr(0x0001F600)).toInt());
		}
		{
			TRACE(0x0001F600)
			textchr(128512).dump().errputl();
			assert("\U0001F600" == textchr(128512));
			assert(128512 == textord(textchr(128512)).toInt());
		}
	}
	{

		assert(textchrname(-2).errputl()	eq "");
		assert(textchrname(-1).errputl()	eq "");
		assert(textchrname(0).errputl()	eq "");
		assert(textchrname(31).errputl()	eq "");
		assert(textchrname(0x10FFFF).errputl()	eq "");
		assert(textchrname(0x10FFFF + 1).errputl()	eq "");
		assert(textchrname(128).errputl()	eq "");
		assert(textchrname(129).errputl()	eq "");

		assert(textchrname(32).errputl()	eq "SPACE");
		assert(textchrname(33).errputl()	eq "EXCLAMATION MARK");
		assert(textchrname(34).errputl()	eq "QUOTATION MARK");
		assert(textchrname(35).errputl()	eq "NUMBER SIGN");
		assert(textchrname(36).errputl()	eq "DOLLAR SIGN");
		assert(textchrname(37).errputl()	eq "PERCENT SIGN");
		assert(textchrname(38).errputl()	eq "AMPERSAND");
		assert(textchrname(39).errputl()	eq "APOSTROPHE");

		assert(textchrname(40).errputl()	eq "LEFT PARENTHESIS");
		assert(textchrname(41).errputl()	eq "RIGHT PARENTHESIS");
		assert(textchrname(42).errputl()	eq "ASTERISK");
		assert(textchrname(43).errputl()	eq "PLUS SIGN");
		assert(textchrname(44).errputl()	eq "COMMA");
		assert(textchrname(45).errputl()	eq "HYPHEN-MINUS");
		assert(textchrname(46).errputl()	eq "FULL STOP");
		assert(textchrname(47).errputl()	eq "SOLIDUS");
		assert(textchrname(48).errputl()	eq "DIGIT ZERO");
		assert(textchrname(49).errputl()	eq "DIGIT ONE");

		assert(textchrname(50).errputl()	eq "DIGIT TWO");
		assert(textchrname(51).errputl()	eq "DIGIT THREE");
		assert(textchrname(52).errputl()	eq "DIGIT FOUR");
		assert(textchrname(53).errputl()	eq "DIGIT FIVE");
		assert(textchrname(54).errputl()	eq "DIGIT SIX");
		assert(textchrname(55).errputl()	eq "DIGIT SEVEN");
		assert(textchrname(56).errputl()	eq "DIGIT EIGHT");
		assert(textchrname(57).errputl()	eq "DIGIT NINE");
		assert(textchrname(58).errputl()	eq "COLON");
		assert(textchrname(59).errputl()	eq "SEMICOLON");

		assert(textchrname(60).errputl()	eq "LESS-THAN SIGN");
		assert(textchrname(61).errputl()	eq "EQUALS SIGN");
		assert(textchrname(62).errputl()	eq "GREATER-THAN SIGN");
		assert(textchrname(63).errputl()	eq "QUESTION MARK");
		assert(textchrname(64).errputl()	eq "COMMERCIAL AT");
		assert(textchrname(65).errputl()	eq "LATIN CAPITAL LETTER A");
		assert(textchrname(66).errputl()	eq "LATIN CAPITAL LETTER B");
		assert(textchrname(67).errputl()	eq "LATIN CAPITAL LETTER C");
		assert(textchrname(68).errputl()	eq "LATIN CAPITAL LETTER D");
		assert(textchrname(69).errputl()	eq "LATIN CAPITAL LETTER E");
		assert(textchrname(70).errputl()	eq "LATIN CAPITAL LETTER F");

		assert(textchrname(71).errputl()	eq "LATIN CAPITAL LETTER G");
		assert(textchrname(72).errputl()	eq "LATIN CAPITAL LETTER H");
		assert(textchrname(73).errputl()	eq "LATIN CAPITAL LETTER I");
		assert(textchrname(74).errputl()	eq "LATIN CAPITAL LETTER J");
		assert(textchrname(75).errputl()	eq "LATIN CAPITAL LETTER K");
		assert(textchrname(76).errputl()	eq "LATIN CAPITAL LETTER L");
		assert(textchrname(77).errputl()	eq "LATIN CAPITAL LETTER M");
		assert(textchrname(78).errputl()	eq "LATIN CAPITAL LETTER N");
		assert(textchrname(79).errputl()	eq "LATIN CAPITAL LETTER O");
		assert(textchrname(80).errputl()	eq "LATIN CAPITAL LETTER P");

		assert(textchrname(81).errputl()	eq "LATIN CAPITAL LETTER Q");
		assert(textchrname(82).errputl()	eq "LATIN CAPITAL LETTER R");
		assert(textchrname(83).errputl()	eq "LATIN CAPITAL LETTER S");
		assert(textchrname(84).errputl()	eq "LATIN CAPITAL LETTER T");
		assert(textchrname(85).errputl()	eq "LATIN CAPITAL LETTER U");
		assert(textchrname(86).errputl()	eq "LATIN CAPITAL LETTER V");
		assert(textchrname(87).errputl()	eq "LATIN CAPITAL LETTER W");
		assert(textchrname(88).errputl()	eq "LATIN CAPITAL LETTER X");
		assert(textchrname(89).errputl()	eq "LATIN CAPITAL LETTER Y");
		assert(textchrname(90).errputl()	eq "LATIN CAPITAL LETTER Z");

		assert(textchrname(91).errputl()	eq "LEFT SQUARE BRACKET");
		assert(textchrname(92).errputl()	eq "REVERSE SOLIDUS");
		assert(textchrname(93).errputl()	eq "RIGHT SQUARE BRACKET");
		assert(textchrname(94).errputl()	eq "CIRCUMFLEX ACCENT");
		assert(textchrname(95).errputl()	eq "LOW LINE");
		assert(textchrname(96).errputl()	eq "GRAVE ACCENT");
		assert(textchrname(97).errputl()	eq "LATIN SMALL LETTER A");
		assert(textchrname(98).errputl()	eq "LATIN SMALL LETTER B");
		assert(textchrname(99).errputl()	eq "LATIN SMALL LETTER C");
		assert(textchrname(100).errputl()	eq "LATIN SMALL LETTER D");

		assert(textchrname(101).errputl()	eq "LATIN SMALL LETTER E");
		assert(textchrname(102).errputl()	eq "LATIN SMALL LETTER F");
		assert(textchrname(103).errputl()	eq "LATIN SMALL LETTER G");
		assert(textchrname(104).errputl()	eq "LATIN SMALL LETTER H");
		assert(textchrname(105).errputl()	eq "LATIN SMALL LETTER I");
		assert(textchrname(106).errputl()	eq "LATIN SMALL LETTER J");
		assert(textchrname(107).errputl()	eq "LATIN SMALL LETTER K");
		assert(textchrname(108).errputl()	eq "LATIN SMALL LETTER L");
		assert(textchrname(109).errputl()	eq "LATIN SMALL LETTER M");
		assert(textchrname(110).errputl()	eq "LATIN SMALL LETTER N");

		assert(textchrname(111).errputl()	eq "LATIN SMALL LETTER O");
		assert(textchrname(112).errputl()	eq "LATIN SMALL LETTER P");
		assert(textchrname(113).errputl()	eq "LATIN SMALL LETTER Q");
		assert(textchrname(114).errputl()	eq "LATIN SMALL LETTER R");
		assert(textchrname(115).errputl()	eq "LATIN SMALL LETTER S");
		assert(textchrname(116).errputl()	eq "LATIN SMALL LETTER T");
		assert(textchrname(117).errputl()	eq "LATIN SMALL LETTER U");
		assert(textchrname(118).errputl()	eq "LATIN SMALL LETTER V");
		assert(textchrname(119).errputl()	eq "LATIN SMALL LETTER W");
		assert(textchrname(120).errputl()	eq "LATIN SMALL LETTER X");

		assert(textchrname(121).errputl()	eq "LATIN SMALL LETTER Y");
		assert(textchrname(122).errputl()	eq "LATIN SMALL LETTER Z");
		assert(textchrname(123).errputl()	eq "LEFT CURLY BRACKET");
		assert(textchrname(124).errputl()	eq "VERTICAL LINE");
		assert(textchrname(124).errputl()	eq "VERTICAL LINE");
		assert(textchrname(125).errputl()	eq "RIGHT CURLY BRACKET");
		assert(textchrname(126).errputl()	eq "TILDE");
		assert(textchrname(127).errputl()	eq "");

		assert(textchrname(1000).errputl()	eq "COPTIC CAPITAL LETTER HORI");
		assert(textchrname(2000).errputl()	eq "NKO LETTER O");
		assert(textchrname(3000).errputl()	eq "TAMIL LETTER SA");
		assert(textchrname(4000).errputl()	eq "TIBETAN SUBJOINED LETTER THA");
		assert(textchrname(5000).errputl()	eq "ETHIOPIC SYLLABLE SEBATBEIT FWA");
		assert(textchrname(6000).errputl()	eq "TAGBANWA LETTER SA");
		assert(textchrname(7000).errputl()	eq "BALINESE DIGIT EIGHT");
		assert(textchrname(8000).errputl()	eq "GREEK SMALL LETTER OMICRON WITH PSILI");
		assert(textchrname(9000).errputl()	eq "KEYBOARD");

		assert(textchrname(10000).errputl()	eq "UPPER RIGHT PENCIL");
		assert(textchrname(20000).errputl()	eq "CJK UNIFIED IDEOGRAPH-4E20");
		assert(textchrname(30000).errputl()	eq "CJK UNIFIED IDEOGRAPH-7530");
		assert(textchrname(40000).errputl()	eq "CJK UNIFIED IDEOGRAPH-9C40");
		assert(textchrname(50000).errputl()	eq "HANGUL SYLLABLE SSYAEN");
		assert(textchrname(70000).errputl()	eq "MAHAJANI LETTER SA");

		assert(textchrname(100000).errputl()	eq "TANGUT IDEOGRAPH-186A0");
		assert(textchrname(200000).errputl()	eq "CJK UNIFIED IDEOGRAPH-30D40");

	//	for (var i : range(32, 100))
	//		printl(i, textchr(i), var::textchrname(i));

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
