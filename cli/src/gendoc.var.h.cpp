#include <cassert>
#include <exodus/program.h>
programinit()

function main() {

	var num;

	//abs() const;
	{
		let v1 = var(-12.34).abs(); // 12.34
		// or
		let v2 = abs(-12.34);
	}

	//pwr(in exponent) const;
	{
		let v1 = var(2).pwr(8); // 256
		// or
		let v2 = pwr(2, 8);
	}

	//rnd() const;
	{
		let v1 = var(100).rnd(); // 0 to 99 pseudo random
		// or
		let v2 = rnd(100);
	}

	//initrnd() const;
	{
		var(123).initrnd(); // Set seed to 123
		// or
		initrnd(123);
	}

	//exp() const;
	{
		let v1 = var(1).exp(); // 2.718281828459045
		// or
		let v2 = exp(1);
	}

	//sqrt() const;
	{
		let v1 = var(100).sqrt(); // 10
		// or
		let v2 = sqrt(100);
	}

	//sin() const;
	{
		let v1 = var(30).sin(); // 0.5
		// or
		let v2 = sin(30);
	}

	//cos() const;
	{
		let v1 = var(60).cos(); // 0.5
		// or
		let v2 = cos(60);
	}

	//tan() const;
	{
		let v1 = var(45).tan(); // 1
		// or
		let v2 = tan(45);
	}

	//atan() const;
	{
		let v1 = var(1).atan(); // 45
		// or
		let v2 = atan(1);
	}

	//loge() const;
	{
		let v1 = var(2.718281828459045).loge(); // 1
		// or
		let v2 = loge(2.718281828459045);
	}

	//integer() const;
	{
		let v1 = var(2.9).integer(); // 2
		// or
		let v2 = integer(2.9);
		var v3 = var(-2.9).integer(); // -2
		// or
		var v4 = integer(-2.9);
	}

	//floor() const;
	{
		let v1 = var(2.9).floor(); // 2
		// or
		let v2 = floor(2.9);
		var v3 = var(-2.9).floor(); // -3
		// or
		var v4 = floor(-2.9);
	}

	//round(const int ndecimals = 0) const;
	{
		let v1 = var(0.455).round(2); // 0.46
		// or
		let v2 = round(1.455, 2);
		var v3 = var(-0.455).round(2); // -0.46
		// or
		var v4 = round(-1.455, 2);
	}

	//mod(in modulus) const;
	{
		let v1 = var(11).mod(5); // 1
		// or
		let v2 = mod(11, 5);
	}

	//getxlocale();
	{
		let v1 = var().getxlocale(); // e.g. "en_US.utf8"
		// or
		let v2 = getxlocale();
	}

	var strvar;

	//setxlocale() const;
	{
		if (not "de_DE.utf8"_var.setxlocale()) {}; // true if successful
		// or
		if (setxlocale("de_DE.utf8")) {};
	}

	//chr(const int num) const;
	{
		let v1 = var().chr(0x61); // "a"
		// or
		let v2 = chr(0x61);
	}

	//textchr(const int num) const;
	{
		let v1 = var().textchr(171416); // "𩶘" or "\xF0A9B698"
		// or
		let v2 = textchr(171416);
	}

	//str(const int num) const;
	{
		let v1 = "ab"_var.str(3); // "ababab"
		// or
		let v2 = str("ab", 3);
	}

	//space() const;
	{
		let v1 = var(3).space(); // "␣␣␣"
		// or
		let v2 = space(3);
	}

	//numberinwords(in languagename_or_locale_id = "");
	{
		let v1 = var(123.45).numberinwords("de_DE"); // "ein­hundert­drei­und­zwanzig Komma vier fünf"
	}

	//seq() const;
	{
		let v1 = "abc"_var.seq(); // 0x61 97
		// or
		let v2 = seq("abc");
	}

	//textseq() const;
	{
		let v1 = "Γ"_var.textseq(); // 915 U+0393: Greek Capital Letter Gamma (Unicode Character)
		// or
		let v2 = textseq("Γ");
	}

	//len() const;
	{
		let v1 = "abc"_var.len(); // 3
		// or
		let v2 = len("abc");
	}

	//textwidth() const;
	{
		let v1 = "🤡x🤡"_var.textwidth(); // 5
		// or
		let v2 = textwidth("🤡x🤡");
	}

	//textlen() const;
	{
		let v1 = "Γιάννης"_var.textlen(); // 7
		// or
		let v2 = textlen("Γιάννης");
	}

	//fcount(SV sepstr) const;
	{
		let v1 = "aa**cc"_var.fcount("*"); // 3
		// or
		let v2 = fcount("aa**cc", "*");
	}

	//count(SV sepstr) const;
	{
		let v1 = "aa**cc"_var.count("*"); // 2
		// or
		let v2 = count("aa**cc", "*");
	}

	//starts(SV prefix) const;
	{
		let v1 = "abc"_var.starts("ab"); // true
		// or
		let v2 = starts("abc", "ab");
	}

	//ends(SV suffix) const;
	{
		let v1 = "abc"_var.ends("bc"); // true
		// or
		let v2 = ends("abc", "bc");
	}

	//contains(SV substr) const;
	{
		let v1 = "abcd"_var.contains("bc"); // true
		// or
		let v2 = contains("abcd", "bc");
	}

	//index(SV substr, const int startchar1 = 1) const;
	{
		let v1 = "abcd"_var.index("bc"); // 2
		// or
		let v2 = index("abcd", "bc");
	}

	//indexn(SV substr, const int occurrence) const;
	{
		let v1 = "abcabc"_var.index("bc", 2); // 5
		// or
		let v2 = index("abcabc", "bc", 2);
	}

	//indexr(SV substr, const int startchar1 = -1) const;
	{
		let v1 = "abcabc"_var.indexr("bc"); // 5
		// or
		let v2 = indexr("abcabc", "bc");
	}

	//match(SV regex_str, SV regex_options = "") const;
	{
		let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"
		// or
		let v2 = match("abc1abc2", "BC(\\d)", "i");
	}

	//search(SV regex_str, io startchar1, SV regex_options = "") const;
	{
		var startchar1 = 1;
		let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // returns "bc1]1" and startchar1 is updated to 5 ready for the next search
		// or
		startchar1 = 1;
		let v2 = search("abc1abc2", "BC(\\d)", startchar1, "i");
	}

	//ucase() const&;
	{
		let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
		// or
		let v2 = ucase("Γιάννης");
	}

	//lcase() const&;
	{
		let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
		// or
		let v2 = lcase("ΓΙΆΝΝΗΣ");
	}

	//tcase() const&;
	{
		let v1 = "γιάννης"_var.tcase(); // "Γιάννης"
		// or
		let v2 = tcase("γιάννης");
	}

	//invert() const&;
	{
		let v1 = "abc"_var.invert(); // "\x{C29EC29DC29C}"
		// or
		let v2 = invert("abc");
	}

	//lower() const&;
	{
		let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"
		// or
		let v2 = lower("a1^b2^c3"_var);
	}

	//raise() const&;
	{
		let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"
		// or
		let v2 = "a1]b2]c3"_var;
	}

	//crop() const&;
	{
		let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"
		// or
		let v2 = crop("a1^b2]]^c3^^"_var);
	}

	//quote() const&;
	{
		let v1 = "abc"_var.quote(); // ""abc""
		// or
		let v2 = quote("abc");
	}

	//squote() const&;
	{
		let v1 = "abc"_var.squote(); // "'abc'"
		// or
		let v2 = squote("abc");
	}

	//unquote() const&;
	{
		let v1 = "'abc'"_var.unquote(); // "abc"
		// or
		let v2 = unquote("'abc'");
	}

	//trim(SV trimchars = " ") const&;
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
		// or
		let v2 = trim("␣␣a1␣␣b2␣c3␣␣");
	}

	//trimfirst(SV trimchars = " ") const&;
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
		// or
		let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");
	}

	//trimlast(SV trimchars = " ") const&;
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
		// or
		let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");
	}

	//trimboth(SV trimchars = " ") const&;
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
		// or
		let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");
	}

	//first() const&;
	{
		let v1 = "abc"_var.first(); // "a"
		// or
		let v2 = first("abc");
	}

	//last() const&;
	{
		let v1 = "abc"_var.last(); // "c"
		// or
		let v2 = last("abc");
	}

	//first(const std::size_t length) const&;
	{
		let v1 = "abc"_var.first(2); // "ab"
		// or
		let v2 = first("abc", 2);
	}

	//last(const std::size_t length) const&;
	{
		let v1 = "abc"_var.last(2); // "bc"
		// or
		let v2 = last("abc", 2);
	}

	//cut(const int length) const&;
	{
		let v1 = "abcd"_var.cut(2); // "cd"
		// or
		let v2 = cut("abcd", 2);
	}

	//paste(const int pos1, const int length, SV insertstr) const&;
	{
		let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
		// or
		let v2 = paste("abcd", 2, 2, "XYZ");
	}

	//paste(const int pos1, SV insertstr) const&;
	{
		let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYbcd"
		// or
		let v2 = paste("abcd", 2, "XYZ");
	}

	//prefix(SV insertstr) const&;
	{
		let v1 = "abc"_var.prefix("XY"); // "XYabc"
		// or
		let v2 = prefix("abc", "XY");
	}

	//pop() const&;
	{
		let v1 = "abc"_var.pop(); // "ab"
		// or
		let v2 = pop("abc");
	}

	//fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;
	{
		let v1 = "aa*bb*cc*dd"_var.fieldstore("*", 2, 3, "X*Y"); // "aa*X*Y*"
		// or
		let v2 = fieldstore("aa*bb*cc*dd", "*", 2, 3, "X*Y");
	}

	//fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;
	{
		let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, 0, "X*Y"); // "a1*X*Y*b2*c3*d4"
		// or
		let v2 = fieldstore("a1*b2*c3*d4", "*", 2, 0, "X*Y");
	}

	//fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;
	{
		let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, -3, "X*Y"); // "a1*X*Y"
		// or
		let v2 = fieldstore("a1*b2*c3*d4", "*", 2, -3, "X*Y");
	}

	//substr(const int pos1, const int length) const&;
	{
		let v1 = "abcd"_var.substr(2, 2); // "bc"
		// or
		let v2 = substr("abcd", 2, 2);
	}

	//substr(const int pos1, const int length) const&;
	{
		let v1 = "abcd"_var.substr(3, -2); // "cb"
	}

	//substr(const int pos1, const int length) const&;
	{
		
	}

	//substr(const int pos1) const&;
	{
		let v1 = "abcd"_var.substr(2); // "bcd"
		// or
		let v2 = substr("abcd", 2);
	}

	//convert(SV fromchars, SV tochars) const&;
	{
		let v1 = "abcde"_var.convert("aZd", "XY"); // "Xbce" (a is replaced and d is removed)
		// or
		let v2 = convert("abcde", "aZd", "XY");
	}

	//textconvert(SV fromchars, SV tochars) const&;
	{
		let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
		// or
		let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");
	}

	//replace(SV fromstr, SV tostr) const&;
	{
		let v1 = "Abc Abc"_var.replace("bc", "X"); // "AX AX"
		// or
		let v2 = replace("Abc Abc", "bc", "X");
	}

	//replace(const rex& regex, SV tostr) const&;
	{
		let v1 = "A a B b"_var.replace("[A-Z]"_rex, "'$0'"); // "'A' a 'B' b"
		// or
		let v2 = replace("A a B b", "[A-Z]"_rex, "'$0'");
	}

	//unique() const&;
	{
		let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"
		// or
		let v2 = unique("a1^b2^a1^c2"_var);
	}

	//sort(SV sepchar = _FM) const&;
	{
		let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"
		// or
		let v2 = sort("20^10^2^1^1.1"_var);
	}

	//sort(SV sepchar = _FM) const&;
	{
		let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"
		// or
		let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);
	}

	//reverse(SV sepchar = _FM) const&;
	{
		let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"
		// or
		let v2 = reverse("20^10^2^1^1.1"_var);
	}

	//shuffle(SV sepchar = _FM) const&;
	{
		let v1 = "20^10^2^1^1.1"_var.shuffle(); // "2^1^20^1.1^10" (random order depending on initrand())
	}

	//shuffle(SV sepchar = _FM) const&;
	{
		
	}

	//parse(char sepchar = ' ') const&;
	{
		let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^"def,"123" fgh"^12.34"
		// or
		let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');
	}

	//ucaser() REF ;
	{
		var v1 = "abc";
		v1.ucaser(); // "ABC"
		// or
		ucaser(v1);
	}

	//hash(const std::uint64_t modulus = 0) const;
	{
		let v1 = "abc"_var.hash(); // 6715211243465481821
		// or
		let v2 = hash("abc");
	}

	//substr(const int pos1, SV delimiterchars, io pos2) const;
	{
		var pos1a = 4, pos2a; let v1 = "aa,bb,cc"_var.substr(pos1a, ",", pos2a); // "bb" and pos2 -> 6
		// or
		var pos1b = 4, pos2b; let v2 = substr("aa,bb,cc", pos1b, ",", pos2b);
	}

	//substr2(io pos1, out delimiterno) const;
	{
		var pos1a = 4, delim1;
		let v1 = "aa^bb^cc"_var.substr2(pos1a, delim1); // "bb", pos1a -> 7, delim -> 2 (FM)
		// or
		var pos1b = 4, delim2;
		let v2 = substr2("aa^bb^cc"_var, pos1b, delim2);
	}

	//field(SV strx, const int fieldnx = 1, const int nfieldsx = 1) const;
	{
		let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
		// or
		let v2 = field("aa*bb*cc", "*", 2);
	}

	//field2(SV separator, const int fieldno, const int nfields = 1) const
	{
		let v1 = "aa*bb*cc"_var.field2("*", -1); // "cc"
		// or
		let v2 = field2("aa*bb*cc", "*", -1);
	}

	//oconv(const char* convstr) const;
	{
		let v1 = var(30123).oconv("D/E"); // "21/06/2050"
		// or
		let v2 = oconv(30123, "D/E");
	}

	//iconv(const char* convstr) const;
	{
		let v1 = "21 JUN 2050"_var.iconv("D/E"); // 30123
		// or
		let v2 = iconv("21 JUN 2050", "D/E");
	}

	//format(in fmt_str, Args&&... args) const
	{
		let v1 = var(12.345).format("'{:_>8.2f}'"); // "'___12.35'"
		let v2 = var(12.345).format("'{::MD20P|R(_)#8}'");
		// or
		var v3 = format("'{:_>8.2f}'", var(12.345)); // "'___12.35'"
		var v4 = format("'{::MD20P|R(_)#8}'", var(12.345));
	}

	//from_codepage(const char* codepage) const;
	{
		let v1 = "\xa4"_var.from_codepage("CP1124"); // "Є"
		// or
		let v2 = from_codepage("\xa4", "CP1124");
		// U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode Character
	}

	//to_codepage(const char* codepage) const;
	{
		let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
		// or
		let v2 = to_codepage("Є", "CP1124").oconv("HEX");
	}

	//f(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	{
		let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
		let v2 = v1.f(2, 2); // "f2v2"
	}

	//extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const
	{
		let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
		let v2 = v1.extract(2, 2); // "f2v2"
		//
		// The alias "f" is normally used instead for brevity
		var v3 = v1.f(2, 2);
	}

	//sum() const;
	{
		let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"
		// or
		let v2 = sum("1]2]3^4]5]6"_var);
	}

	//sumall() const;
	{
		let v1 = "1]2]3^4]5]6"_var.sumall(); // "21"
		// or
		let v2 = sumall("1]2]3^4]5]6"_var);
	}

	//sum(SV sepchar) const;
	{
		let v1 = "10,20,33"_var.sum(","); // "60"
		// or
		let v2 = sum("10,20,33", ",");
	}

	//mv(const char* opcode, in var2) const;
	{
		let v1 = "10]20]30"_var.mv("+","2]3]4"_var); // "12]23]34"
	}

	//r(const int fieldno, in replacement) REF
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, "X"); // v1 -> "f1^X^f3"
	}

	//r(const int fieldno, const int valueno, in replacement) REF
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, "X"); // v1 -> "f1^v1]X^f3"
	}

	//r(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s3^f3"
	}

	//inserter(const int fieldno, in insertion) REF
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, "X"); // v1 -> "f1^X^v1]v2}s2}s3^f3"
		// or
		inserter(v1, 2, "X");
	}

	//inserter(const int fieldno, const int valueno, in insertion) REF
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, 2, "X"); // v1 -> "f1^v1]X]v2}s2}s3^f3"
		// or
		inserter(v1, 2, 2, "X");
	}

	//inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s2}s3^f3"
		// or
		v1.inserter(2, 2, 2, "X");
	}

	//remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.remover(2, 2); // v1 -> "f1^v1^f3"
		// or
		remover(v1, 2, 2);
	}

	//locate(in target) const;
	{
		if ("UK]US]UA"_var.locate("US")) {}; // true
		// or
		if (locate("US", "UK]US]UA"_var)) {};
	}

	//locate(in target, out valueno) const;
	{
		var valueno;
		if ("UK]US]UA"_var.locate("US", valueno)) {}; // returns true and valueno = 2
	}

	//locate(in target, out setting, const int fieldno, const int valueno = 0) const;
	{
		var setting;
		if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", setting, 2, 3)) {}; // returns true and setting = 4
	}

	//locateby(const char* ordercode, in target, out valueno) const;
	{
		var valueno; if ("aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) {}; // returns false and valueno = 2 where it could be correctly inserted.
	}

	//locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno = 0) const;
	{
		var setting;
		if ("f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", setting, 3)) {}; // returns false and setting = 2 where it could be correctly inserted.
	}

	//locateusing(const char* usingchar, in target) const;
	{
		if ("AB,EF,CD"_var.locateusing(",", "EF")) {}; // true
	}

	//locateusing(const char* usingchar, in target, out setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	{
		var setting;
		if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", setting, 3)) {}; // returns true and setting = 2
	}

	var conn;

	//connect(in conninfo = "");
	{
		let conninfo = "dbname=exodus user=exodus password=somesillysecret";
		var db;
		if (not db.connect(conninfo)) abort(lasterror());
		db.disconnect();
		// or
		if (not connect("exodus")) {};
		// or
		if (not connect()) {};
		disconnect();
	}

	//disconnect();
	{
		conn.disconnect();
		// or
		disconnect();
	}

	//disconnectall();
	{
		conn.disconnectall();
		// or
		disconnectall();
	}

	//attach(in filenames) const;
	{
		let filenames = "definitions^dict.definitions"_var, conn = "exodus";
		if (not conn.attach(filenames)) {};
		// or
		if (not attach(filenames)) {};
	}

	//begintrans() const;
	{
		var conn = "exodus";
		if (not conn.begintrans()) {};
		// or
		if (not begintrans()) {};
	}

	//rollbacktrans() const;
	{
		var conn = "exodus";
		if (not conn.rollbacktrans()) {};
		// or
		if (not rollbacktrans()) {};
	}

	//committrans() const;
	{
		var conn = "exodus";
		if (not conn.committrans()) {};
		// or
		if (not committrans()) {};
	}

	//statustrans() const;
	{
		var conn = "exodus";
		if (not conn.statustrans()) {};
		// or
		if (not statustrans()) {};
	}

	//sqlexec(in sqlcmd) const;
	{
		var sqlcmd = "select 2 + 2;", response, conn = "exodus";
		if (not conn.sqlexec("select 2 + 2;")) {}; // True
		// or
		if (not sqlexec(sqlcmd)) {};
	}

	//sqlexec(in sqlcmd, io response) const;
	{
		var sqlcmd = "select 2 + 2;", response, conn = "exodus";
		if (not conn.sqlexec(sqlcmd, response)) {}; // True and response = 4
		// or
		if (not sqlexec(sqlcmd, response)) {};
	}

	//dbcreate(in new_dbname, in old_dbname = "") const;
	{
		var conn = "exodus";
		if (not conn.dbcreate("mydb")) {};
		// or
		if (not dbcreate("mydb")) {};
	}

	//dbcopy(in from_dbname, in to_dbname) const;
	{
		var conn = "exodus";
		if (not conn.dbcopy("mydb", "mydb2")) {};
		// or
		if (not dbcopy("mydb", "mydb2")) {};
	}

	//dbdelete(in dbname) const;
	{
		var conn = "exodus";
		if (not conn.dbdelete("mydb")) {};
		// or
		if (not dbdelete("mydb")) {};
	}

	//dblist() const;
	{
		let v1 = conn.dblist();
		// or
		let v2 = dblist();
	}

	//createfile(in filename) const;
	{
		let filename = "gendoc_temp", conn = "exodus";
		if (not conn.createfile(filename)) {};
		// or
		if (not createfile(filename)) {};
	}

	//renamefile(in filename, in newfilename) const;
	{
		let conn = "exodus", filename = "gendoc_temp", new_filename = "gendoc_temp2";
		if (not conn.renamefile(filename, new_filename)) {};
		// or
		if (not renamefile(filename, new_filename)) {};
	}

	//listfiles() const;
	{
		var conn = "exodus";
		if (not conn.listfiles()) {};
		// or
		if (not listfiles()) {};
	}

	//clearfile(in filename) const;
	{
		let conn = "exodus", filename = "gendoc_temp2";
		if (not conn.clearfile(filename)) {};
		// or
		if (not clearfile(filename)) {};
	}

	//deletefile(in filename) const;
	{
		let conn = "exodus", filename = "gendoc_temp2";
		if (not conn.deletefile(filename)) {};
		// or
		if (not deletefile(filename)) {};
	}

	var conn_or_file;

	//reccount(in filename = "") const;
	{
		let conn = "exodus", filename = "definitions";
		var nrecs1 = conn.reccount(filename);
		// or
		var nrecs2 = reccount(filename);
	}

	var file;

	//open(in dbfilename, in connection = "");
	{
		var file, filename = "definitions";
		if (not file.open(filename)) {};
		// or
		if (not open(filename to file)) {};
	}

	//close() const;
	{
		var file = "definitions";
		file.close();
		// or
		close(file);
	}

	//createindex(in fieldname, in dictfile = "") const;
	{
		var filename = "definitions", fieldname = "DATE_TIME";
		if (not filename.createindex(fieldname)) {};
		// or
		if (not createindex(filename, fieldname)) {};
	}

	//deleteindex(in fieldname) const;
	{
		var file = "definitions", fieldname = "DATE_TIME";
		if (not file.deleteindex(fieldname)) {};
		// or
		if (not deleteindex(file, fieldname)) {};
	}

	//listindex(in file_or_filename = "", in fieldname = "") const;
	{
		var conn = "exodus";
		if (not conn.listindex()) {};
		// or
		if (not listindex()) {};
	}

	//lock(in key) const;
	{
		var file = "definitions", key = "1000";
		if (not file.lock(key)) {};
		// or
		if (not lock(file, key)) {};
	}

	//unlock(in key) const;
	{
		var file = "definitions", key = "1000";
		if (not file.unlock(key)) {};
		// or
		if (not unlock(file, key)) {};
	}

	//unlockall() const;
	{
		var conn = "exodus";
		if (not conn.unlockall()) {};
		// or
		if (not unlockall(conn)) {};
	}

	var rec;

	//read(in file, in key);
	{
		var rec, file = "definitions", key = "1000";
		if (not rec.read(file, key)) {};
		// or
		if (not read(rec from file, key)) {};
	}

	//write(in file, in key) const;
	{
		var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
		
	}

	//write(in file, in key) const;
	{
		
	}

	//updaterecord(in file, in key) const;
	{
		var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
		if (not rec.updaterecord(file, key)) {};
		// or
		if (not updaterecord(rec on file, key)) {};
	}

	//insertrecord(in file, in key) const;
	{
		var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
		if (not rec.insertrecord(file, key)) {};
		// or
		if (not insertrecord(rec on file, key)) {};
	}

	//deleterecord(in key) const;
	{
		var file = "definitions", key = "1000";
		if (not file.deleterecord(key)) {};
		// or
		if (not deleterecord(file, key)) {};
	}

	//readf(in file, in key, const int fieldno);
	{
		var field, file = "definitions", key = "1000", fieldno = 3;
		if (not field.readf(file, key, fieldno)) {};
		// or
		if (not readf(field from file, key, fieldno)) {};
	}

	//writef(in file, in key, const int fieldno) const;
	{
		var field = "f3", file = "definitions", key = "1000", fieldno = 3;
		field.writef(file, key, fieldno);
		// or
		writef(field on file, key, fieldno);
	}

	//readc(in file, in key);
	{
		var rec, file = "definitions", key = "1000";
		if (not rec.readc(file, key)) {};
		// or
		if (not readc(rec from file, key)) {};
	}

	//writec(in file, in key) const;
	{
		var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
		rec.writec(file, key);
		// or
		writec(rec on file, key);
	}

	var dbfile;

	//deletec(in key) const;
	{
		var file = "definitions", key = "1000";
		if (not file.deletec(key)) {};
		// or
		if (not deletec(file, key)) {};
	}

	//cleardbcache() const;
	{
		conn.cleardbcache();
		// or
		cleardbcache(conn);
	}

	//xlate(in filename, in fieldno, const char* mode) const;
	{
		let key = "SB001";
		let client_name = key.xlate("xo_clients", 1, "X"); assert(client_name == "Client AAA");
		// or
		let name_and_type = xlate("xo_clients", key, "NAME_AND_TYPE", "X"); assert("Client AAA (A)");
	}

	//date() const;
	{
		var today1 = var().date(); // e.g. was 20821 from 2025-01-01 00:00:00 UTC
		// or
		var today2 = date();
	}

	//time() const;
	{
		var now1 = var().time(); // range 0 - 86399 since there are 24*60*60 (86400) seconds in a day.
		// or
		var now2 = time();
	}

	//ostime() const;
	{
		var now1 = var().ostime(); // e.g. 23343.704387955 approx. 06:29:03 UTC
		// or
		var now2 = ostime();
	}

	//timestamp() const;
	{
		var now1 = var().timestamp(); // was 20821.99998842593 around 2025-01-01 23:59:59 UTC
		// or
		var now2 = timestamp();
	}

	//timestamp(in ostime) const;
	{
		var idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
		var ts1 = idate.timestamp(itime); // 20821.99998842593
		// or
		var ts2 = timestamp(idate, itime);
	}

	//ossleep(const int milliseconds) const;
	{
		var().ossleep(500); // sleep for 500ms
		// or
		ossleep(500);
	}

	var file_dir_list;

	//oswait(const int milliseconds) const;
	{
		let v1 = ".^/etc/hosts"_var.oswait(500); // e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
		// or
		let v2 = oswait(".^/etc/hosts"_var, 500);
	}

	var osfilevar;

	//osopen(in osfilename, const bool utf8 = true) const;
	{
		var hostsfile;
		if (not hostsfile.osopen("/etc/hosts")) {};
		// or
		if (not osopen("/etc/hosts" to hostsfile)) {};
	}

	//osbread(in osfilevar, io offset, const int length);
	{
		let hostsfile = "/etc/hosts";
		var text, offset = 0;
		if (not text.osbread(hostsfile, offset, 1024)) {};
		// or
		if (not osbread(text from hostsfile, offset, 1024)) {};
	}

	//osbwrite(in osfilevar, io offset) const;
	{
		let text = "\n#comment", tempfilename = ostempfilename();
		var offset = osfile(tempfilename).f(1); // size of file -> append
		if (not text.osbwrite(tempfilename, offset)) abort(lasterror()); // offset -> 9
		// or
		if (not osbwrite(text on tempfilename, offset)) {};
	}

	//osclose() const;
	{
		if (not osfilevar.open("/etc/hosts")) {};
		osfilevar.osclose();
		// or
		osclose(osfilevar);
	}

	//osread(const char* osfilename, const char* codepage = "");
	{
		var text;
		if (not text.osread("/etc/hosts")) {};
		// or
		if (not osread(text from "/etc/hosts")) {};
	}

	//oswrite(in osfilename, const char* codepage = "") const;
	{
		let text = "aaa = 123\nbbb = 234";
		let osfilename = ostempdirpath() ^ "xo.conf";
		if (not text.oswrite(osfilename)) abort(lasterror());
		// or
		if (not oswrite(text on osfilename)) {};
	}

	var osfilename;

	//osremove() const;
	{
		let osfilename = ostempfilename();
		if (not osfilename.osremove()) abort(lasterror());
		// or
		if (not osremove(osfilename)) {};
	}

	var osfileordirname;

	//osrename(in new_dirpath_or_filepath) const;
	{
		let osfilename = ostempfilename();
		if (not osfilename.osrename(osfilename ^ ".bak")) abort(lasterror());
		// or
		if (not osrename(osfilename, osfilename ^ ".bak")) {};
	}

	//oscopy(in to_osfilename) const;
	{
		let osfilename = ostempfilename();
		if (not osfilename.oscopy(osfilename ^ ".bak")) abort(lasterror());
		// or
		if (not oscopy(osfilename, osfilename ^ ".bak")) {};
	}

	//osmove(in to_osfilename) const;
	{
		let osfilename = ostempfilename();
		if (not osfilename.osmove(osfilename ^ ".bak")) abort(lasterror());
		// or
		if (not osmove(osfilename, osfilename ^ ".bak")) {};
	}

	var dirpath;

	//oslist(SV globpattern = "", const int mode = 0) const;
	{
		var entries1 = "/etc/"_var.oslist("*.cfg"); // "adduser.conf^ca-certificates.conf^ {};"
		// or
		var entries2 = oslist("/etc/" "*.conf");
	}

	var osfileordirpath;

	//osinfo(const int mode = 0) const;
	{
		var info1 = "/etc/hosts"_var.osinfo(); // "221^20597^78309"
		// or
		var info2 = osinfo("/etc/hosts");
	}

	//osfile() const;
	{
		var fileinfo1 = "/etc/hosts"_var.osfile(); // "221^20597^78309"
		// or
		var fileinfo2 = osfile("/etc/hosts");
	}

	//osdir() const;
	{
		var dirinfo1 = "/etc/"_var.osdir(); // "^20848^44464"
		// or
		var dirinfo2 = osfile("/etc/");
	}

	//osmkdir() const;
	{
		if (not "abc/def"_var.osmkdir()) {};
		// or
		if (not osmkdir("abc/def")) {};
	}

	//osrmdir(bool evenifnotempty = false) const;
	{
		if (not "abc/def"_var.osrmdir()) {};
		// or
		if (not osrmdir("abc/def")) {};
	}

	//oscwd() const;
	{
		var cwd1 = var().oscwd(); // "/home/exodus"
		// or
		var cwd2 = oscwd();
	}

	//oscwd(in newpath) const;
	{
		if (not "abc/def"_var.oscwd()) {};
		// or
		if (not oscwd("abc/def")) {};
	}

	var command;

	//osshell() const;
	{
		let cmd = "ls -l xyz";
		if (not cmd.osshell()) {};
		// or
		if (not osshell(cmd)) {};
	}

	var instr;

	//osshellread(in oscmd);
	{
		var text;
		if (not text.osshellread("ls -l")) {};
		// or capturing stdout but ignoring exit status
		text = osshellread("ls -l");
	}

	var outstr;

	//osshellwrite(in oscmd) const;
	{
		let text = "abc xyz";
		if (not text.osshellwrite("grep xyz")) {};
		// or
		if (not osshellwrite(text, "grep xyz")) {};
	}

	//ostempdirpath() const;
	{
		let v1 = var().ostempdirpath(); // e.g. "/tmp/"
		// or
		let v2 = ostempdirpath();
	}

	//ostempfilename() const;
	{
		var tempfilename1 = var().ostempfilename(); // "/tmp/~exoEcLj3C"
		// or
		var tempfilename2 = ostempfilename();
	}

	var envvalue;

	//ossetenv(SV envcode) const;
	{
		let envcode = "EXO_ABC", envvalue = "XYZ";
		envvalue.ossetenv(envcode);
		// or
		ossetenv(envcode, envvalue);
	}

	//osgetenv(SV envcode);
	{
		var envvalue1;
		if (not envvalue1.osgetenv("HOME")) {}; // e.g. "/home/exodus"
		// or
		var envvalue2 = osgetenv("EXO_ABC"); // "XYZ"
	}

	//ospid() const;
	{
		let pid1 = var().ospid(); // 663237
		// or
		let pid2 = ospid();
	}

	//ostid() const;
	{
		let tid1 = var().ostid(); // 663237
		// or
		let tid2 = ostid();
	}

	//version() const;
	{
		let v1 = var().version(); // "29 JAN 2025 14:56:52"
	}

	//osflush() const;
	{
		var().osflush();
		// or
		osflush();
	}


	return 0;
}

programexit()
