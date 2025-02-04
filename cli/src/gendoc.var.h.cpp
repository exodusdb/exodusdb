#include <cassert>
#include <exodus/program.h>
programinit()

function main() {
	// Clean up before starting
	if (not dbdelete("xo_gendoc_testdb")) {};
	if (not dbdelete("xo_gendoc_testdb2")) {};

	var num;

	printl("abs() const;");
	{
		let v1 = var(-12.34).abs(); // 12.34
		// or
		let v2 = abs(-12.34);
	}

	printl("pwr(in exponent) const;");
	{
		let v1 = var(2).pwr(8); // 256
		// or
		let v2 = pwr(2, 8);
	}

	printl("rnd() const;");
	{
		let v1 = var(100).rnd(); // 0 to 99 pseudo random
		// or
		let v2 = rnd(100);
	}

	printl("initrnd() const;");
	{
		var(123).initrnd(); // Set seed to 123
		// or
		initrnd(123);
	}

	printl("exp() const;");
	{
		let v1 = var(1).exp(); // 2.718281828459045
		// or
		let v2 = exp(1);
	}

	printl("sqrt() const;");
	{
		let v1 = var(100).sqrt(); // 10
		// or
		let v2 = sqrt(100);
	}

	printl("sin() const;");
	{
		let v1 = var(30).sin(); // 0.5
		// or
		let v2 = sin(30);
	}

	printl("cos() const;");
	{
		let v1 = var(60).cos(); // 0.5
		// or
		let v2 = cos(60);
	}

	printl("tan() const;");
	{
		let v1 = var(45).tan(); // 1
		// or
		let v2 = tan(45);
	}

	printl("atan() const;");
	{
		let v1 = var(1).atan(); // 45
		// or
		let v2 = atan(1);
	}

	printl("loge() const;");
	{
		let v1 = var(2.718281828459045).loge(); // 1
		// or
		let v2 = loge(2.718281828459045);
	}

	printl("integer() const;");
	{
		let v1 = var(2.9).integer(); // 2
		// or
		let v2 = integer(2.9);
		var v3 = var(-2.9).integer(); // -2
		// or
		var v4 = integer(-2.9);
	}

	printl("floor() const;");
	{
		let v1 = var(2.9).floor(); // 2
		// or
		let v2 = floor(2.9);
		var v3 = var(-2.9).floor(); // -3
		// or
		var v4 = floor(-2.9);
	}

	printl("round(const int ndecimals = 0) const;");
	{
		let v1 = var(0.455).round(2); // 0.46
		// or
		let v2 = round(1.455, 2);
		var v3 = var(-0.455).round(2); // -0.46
		// or
		var v4 = round(-1.455, 2);
	}

	printl("mod(in modulus) const;");
	{
		let v1 = var(11).mod(5); // 1
		// or
		let v2 = mod(11, 5);
	}

	printl("getxlocale();");
	{
		let v1 = var().getxlocale(); // e.g. "en_US.utf8"
		// or
		let v2 = getxlocale();
	}

	var strvar;

	printl("setxlocale() const;");
	{
		if ("en_US.utf8"_var.setxlocale()) {/*ok*/} else  {abort("setxlocale" ": " ^ lasterror());} // true if successful
		// or
		if (setxlocale("en_US.utf8")) {/*ok*/} else  {abort("setxlocale" ": " ^ lasterror());}
	}

	printl("chr(const int num) const;");
	{
		let v1 = var().chr(0x61); // "a"
		// or
		let v2 = chr(0x61);
	}

	printl("textchr(const int num) const;");
	{
		let v1 = var().textchr(171416); // "𩶘" or "\xF0A9B698"
		// or
		let v2 = textchr(171416);
	}

	printl("str(const int num) const;");
	{
		let v1 = "ab"_var.str(3); // "ababab"
		// or
		let v2 = str("ab", 3);
	}

	printl("space() const;");
	{
		let v1 = var(3).space(); // "␣␣␣"
		// or
		let v2 = space(3);
	}

	printl("numberinwords(in languagename_or_locale_id = "");");
	{
		let v1 = var(123.45).numberinwords("de_DE"); // "ein­hundert­drei­und­zwanzig Komma vier fünf"
	}

	printl("seq() const;");
	{
		let v1 = "abc"_var.seq(); // 0x61 97
		// or
		let v2 = seq("abc");
	}

	printl("textseq() const;");
	{
		let v1 = "Γ"_var.textseq(); // 915 U+0393: Greek Capital Letter Gamma (Unicode Character)
		// or
		let v2 = textseq("Γ");
	}

	printl("len() const;");
	{
		let v1 = "abc"_var.len(); // 3
		// or
		let v2 = len("abc");
	}

	printl("textwidth() const;");
	{
		let v1 = "🤡x🤡"_var.textwidth(); // 5
		// or
		let v2 = textwidth("🤡x🤡");
	}

	printl("textlen() const;");
	{
		let v1 = "Γιάννης"_var.textlen(); // 7
		// or
		let v2 = textlen("Γιάννης");
	}

	printl("fcount(SV sepstr) const;");
	{
		let v1 = "aa**cc"_var.fcount("*"); // 3
		// or
		let v2 = fcount("aa**cc", "*");
	}

	printl("count(SV sepstr) const;");
	{
		let v1 = "aa**cc"_var.count("*"); // 2
		// or
		let v2 = count("aa**cc", "*");
	}

	printl("starts(SV prefix) const;");
	{
		let v1 = "abc"_var.starts("ab"); // true
		// or
		let v2 = starts("abc", "ab");
	}

	printl("ends(SV suffix) const;");
	{
		let v1 = "abc"_var.ends("bc"); // true
		// or
		let v2 = ends("abc", "bc");
	}

	printl("contains(SV substr) const;");
	{
		let v1 = "abcd"_var.contains("bc"); // true
		// or
		let v2 = contains("abcd", "bc");
	}

	printl("index(SV substr, const int startchar1 = 1) const;");
	{
		let v1 = "abcd"_var.index("bc"); // 2
		// or
		let v2 = index("abcd", "bc");
	}

	printl("indexn(SV substr, const int occurrence) const;");
	{
		let v1 = "abcabc"_var.index("bc", 2); // 5
		// or
		let v2 = index("abcabc", "bc", 2);
	}

	printl("indexr(SV substr, const int startchar1 = -1) const;");
	{
		let v1 = "abcabc"_var.indexr("bc"); // 5
		// or
		let v2 = indexr("abcabc", "bc");
	}

	printl("match(SV regex_str, SV regex_options = "") const;");
	{
		let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"
		// or
		let v2 = match("abc1abc2", "BC(\\d)", "i");
	}

	printl("search(SV regex_str, io startchar1, SV regex_options = "") const;");
	{
		var startchar1 = 1;
		let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // returns "bc1]1" and startchar1 is updated to 5 ready for the next search
		// or
		startchar1 = 1;
		let v2 = search("abc1abc2", "BC(\\d)", startchar1, "i");
	}

	printl("ucase() const&;");
	{
		let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
		// or
		let v2 = ucase("Γιάννης");
	}

	printl("lcase() const&;");
	{
		let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
		// or
		let v2 = lcase("ΓΙΆΝΝΗΣ");
	}

	printl("tcase() const&;");
	{
		let v1 = "γιάννης"_var.tcase(); // "Γιάννης"
		// or
		let v2 = tcase("γιάννης");
	}

	printl("invert() const&;");
	{
		let v1 = "abc"_var.invert(); // "\x{C29EC29DC29C}"
		// or
		let v2 = invert("abc");
	}

	printl("lower() const&;");
	{
		let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"
		// or
		let v2 = lower("a1^b2^c3"_var);
	}

	printl("raise() const&;");
	{
		let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"
		// or
		let v2 = "a1]b2]c3"_var;
	}

	printl("crop() const&;");
	{
		let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"
		// or
		let v2 = crop("a1^b2]]^c3^^"_var);
	}

	printl("quote() const&;");
	{
		let v1 = "abc"_var.quote(); // ""abc""
		// or
		let v2 = quote("abc");
	}

	printl("squote() const&;");
	{
		let v1 = "abc"_var.squote(); // "'abc'"
		// or
		let v2 = squote("abc");
	}

	printl("unquote() const&;");
	{
		let v1 = "'abc'"_var.unquote(); // "abc"
		// or
		let v2 = unquote("'abc'");
	}

	printl("trim(SV trimchars = " ") const&;");
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
		// or
		let v2 = trim("␣␣a1␣␣b2␣c3␣␣");
	}

	printl("trimfirst(SV trimchars = " ") const&;");
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
		// or
		let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");
	}

	printl("trimlast(SV trimchars = " ") const&;");
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
		// or
		let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");
	}

	printl("trimboth(SV trimchars = " ") const&;");
	{
		let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
		// or
		let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");
	}

	printl("first() const&;");
	{
		let v1 = "abc"_var.first(); // "a"
		// or
		let v2 = first("abc");
	}

	printl("last() const&;");
	{
		let v1 = "abc"_var.last(); // "c"
		// or
		let v2 = last("abc");
	}

	printl("first(const std::size_t length) const&;");
	{
		let v1 = "abc"_var.first(2); // "ab"
		// or
		let v2 = first("abc", 2);
	}

	printl("last(const std::size_t length) const&;");
	{
		let v1 = "abc"_var.last(2); // "bc"
		// or
		let v2 = last("abc", 2);
	}

	printl("cut(const int length) const&;");
	{
		let v1 = "abcd"_var.cut(2); // "cd"
		// or
		let v2 = cut("abcd", 2);
	}

	printl("paste(const int pos1, const int length, SV insertstr) const&;");
	{
		let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
		// or
		let v2 = paste("abcd", 2, 2, "XYZ");
	}

	printl("paste(const int pos1, SV insertstr) const&;");
	{
		let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYbcd"
		// or
		let v2 = paste("abcd", 2, "XYZ");
	}

	printl("prefix(SV insertstr) const&;");
	{
		let v1 = "abc"_var.prefix("XY"); // "XYabc"
		// or
		let v2 = prefix("abc", "XY");
	}

	printl("pop() const&;");
	{
		let v1 = "abc"_var.pop(); // "ab"
		// or
		let v2 = pop("abc");
	}

	printl("fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;");
	{
		let v1 = "aa*bb*cc*dd"_var.fieldstore("*", 2, 3, "X*Y"); // "aa*X*Y*"
		// or
		let v2 = fieldstore("aa*bb*cc*dd", "*", 2, 3, "X*Y");
	}

	printl("fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;");
	{
		let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, 0, "X*Y"); // "a1*X*Y*b2*c3*d4"
		// or
		let v2 = fieldstore("a1*b2*c3*d4", "*", 2, 0, "X*Y");
	}

	printl("fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;");
	{
		let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, -3, "X*Y"); // "a1*X*Y"
		// or
		let v2 = fieldstore("a1*b2*c3*d4", "*", 2, -3, "X*Y");
	}

	printl("substr(const int pos1, const int length) const&;");
	{
		let v1 = "abcd"_var.substr(2, 2); // "bc"
		// or
		let v2 = substr("abcd", 2, 2);
	}

	printl("substr(const int pos1, const int length) const&;");
	{
		let v1 = "abcd"_var.substr(3, -2); // "cb"
		// or
		let v2 = substr("abcd", 3, -2);
	}

	printl("substr(const int pos1) const&;");
	{
		let v1 = "abcd"_var.substr(2); // "bcd"
		// or
		let v2 = substr("abcd", 2);
	}

	printl("convert(SV fromchars, SV tochars) const&;");
	{
		let v1 = "abcde"_var.convert("aZd", "XY"); // "Xbce" (a is replaced and d is removed)
		// or
		let v2 = convert("abcde", "aZd", "XY");
	}

	printl("textconvert(SV fromchars, SV tochars) const&;");
	{
		let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
		// or
		let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");
	}

	printl("replace(SV fromstr, SV tostr) const&;");
	{
		let v1 = "Abc Abc"_var.replace("bc", "X"); // "AX AX"
		// or
		let v2 = replace("Abc Abc", "bc", "X");
	}

	printl("replace(const rex& regex, SV tostr) const&;");
	{
		let v1 = "A a B b"_var.replace("[A-Z]"_rex, "'$0'"); // "'A' a 'B' b"
		// or
		let v2 = replace("A a B b", "[A-Z]"_rex, "'$0'");
	}

	printl("unique() const&;");
	{
		let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"
		// or
		let v2 = unique("a1^b2^a1^c2"_var);
	}

	printl("sort(SV sepchar = _FM) const&;");
	{
		let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"
		// or
		let v2 = sort("20^10^2^1^1.1"_var);
	}

	printl("sort(SV sepchar = _FM) const&;");
	{
		let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"
		// or
		let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);
	}

	printl("reverse(SV sepchar = _FM) const&;");
	{
		let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"
		// or
		let v2 = reverse("20^10^2^1^1.1"_var);
	}

	printl("shuffle(SV sepchar = _FM) const&;");
	{
		let v1 = "20^10^2^1^1.1"_var.shuffle(); // "2^1^20^1.1^10" (random order depending on initrand())
		// or
		let v2 = shuffle("20^10^2^1^1.1"_var);
	}

	printl("parse(char sepchar = ' ') const&;");
	{
		let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^"def,"123" fgh"^12.34"
		// or
		let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');
	}

	printl("ucaser() REF ;");
	{
		var v1 = "abc";
		v1.ucaser(); // "ABC"
		// or
		ucaser(v1);
	}

	printl("hash(const std::uint64_t modulus = 0) const;");
	{
		let v1 = "abc"_var.hash(); // 6715211243465481821
		// or
		let v2 = hash("abc");
	}

	printl("substr(const int pos1, SV delimiterchars, io pos2) const;");
	{
		var pos1a = 4, pos2a; let v1 = "aa,bb,cc"_var.substr(pos1a, ",", pos2a); // "bb" and pos2 -> 6
		// or
		var pos1b = 4, pos2b; let v2 = substr("aa,bb,cc", pos1b, ",", pos2b);
	}

	printl("substr2(io pos1, out delimiterno) const;");
	{
		var pos1a = 4, delim1;
		let v1 = "aa^bb^cc"_var.substr2(pos1a, delim1); // "bb", pos1a -> 7, delim -> 2 (FM)
		// or
		var pos1b = 4, delim2;
		let v2 = substr2("aa^bb^cc"_var, pos1b, delim2);
	}

	printl("field(SV strx, const int fieldnx = 1, const int nfieldsx = 1) const;");
	{
		let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
		// or
		let v2 = field("aa*bb*cc", "*", 2);
	}

	printl("field2(SV separator, const int fieldno, const int nfields = 1) const");
	{
		let v1 = "aa*bb*cc"_var.field2("*", -1); // "cc"
		// or
		let v2 = field2("aa*bb*cc", "*", -1);
	}

	printl("oconv(const char* convstr) const;");
	{
		let v1 = var(30123).oconv("D/E"); // "21/06/2050"
		// or
		let v2 = oconv(30123, "D/E");
	}

	printl("iconv(const char* convstr) const;");
	{
		let v1 = "21 JUN 2050"_var.iconv("D/E"); // 30123
		// or
		let v2 = iconv("21 JUN 2050", "D/E");
	}

	printl("format(in fmt_str, Args&&... args) const");
	{
		let v1 = var(12.345).format("'{:_>8.2f}'"); // "'___12.35'"
		let v2 = var(12.345).format("'{::MD20P|R(_)#8}'");
		// or
		var v3 = format("'{:_>8.2f}'", var(12.345)); // "'___12.35'"
		var v4 = format("'{::MD20P|R(_)#8}'", var(12.345));
	}

	printl("from_codepage(const char* codepage) const;");
	{
		let v1 = "\xa4"_var.from_codepage("CP1124"); // "Є"
		// or
		let v2 = from_codepage("\xa4", "CP1124");
		// U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode Character
	}

	printl("to_codepage(const char* codepage) const;");
	{
		let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
		// or
		let v2 = to_codepage("Є", "CP1124").oconv("HEX");
	}

	printl("f(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;");
	{
		let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
		let v2 = v1.f(2, 2); // "f2v2"
	}

	printl("extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const");
	{
		let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
		let v2 = v1.extract(2, 2); // "f2v2"
		//
		// The alias "f" is normally used instead for brevity
		var v3 = v1.f(2, 2);
	}

	printl("sum() const;");
	{
		let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"
		// or
		let v2 = sum("1]2]3^4]5]6"_var);
	}

	printl("sumall() const;");
	{
		let v1 = "1]2]3^4]5]6"_var.sumall(); // "21"
		// or
		let v2 = sumall("1]2]3^4]5]6"_var);
	}

	printl("sum(SV sepchar) const;");
	{
		let v1 = "10,20,33"_var.sum(","); // "60"
		// or
		let v2 = sum("10,20,33", ",");
	}

	printl("mv(const char* opcode, in var2) const;");
	{
		let v1 = "10]20]30"_var.mv("+","2]3]4"_var); // "12]23]34"
	}

	printl("r(const int fieldno, in replacement) REF");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, "X"); // v1 -> "f1^X^f3"
	}

	printl("r(const int fieldno, const int valueno, in replacement) REF");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, "X"); // v1 -> "f1^v1]X^f3"
	}

	printl("r(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.r(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s3^f3"
	}

	printl("inserter(const int fieldno, in insertion) REF");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, "X"); // v1 -> "f1^X^v1]v2}s2}s3^f3"
		// or
		inserter(v1, 2, "X");
	}

	printl("inserter(const int fieldno, const int valueno, in insertion) REF");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, 2, "X"); // v1 -> "f1^v1]X]v2}s2}s3^f3"
		// or
		inserter(v1, 2, 2, "X");
	}

	printl("inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.inserter(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s2}s3^f3"
		// or
		v1.inserter(2, 2, 2, "X");
	}

	printl("remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;");
	{
		var v1 = "f1^v1]v2}s2}s3^f3"_var;
		v1.remover(2, 2); // v1 -> "f1^v1^f3"
		// or
		remover(v1, 2, 2);
	}

	printl("locate(in target) const;");
	{
		if ("UK]US]UA"_var.locate("US")) {/*ok*/} else  {abort("locate" ": " ^ lasterror());} // true
		// or
		if (locate("US", "UK]US]UA"_var)) {/*ok*/} else  {abort("locate" ": " ^ lasterror());}
	}

	printl("locate(in target, out valueno) const;");
	{
		var valueno;
		if ("UK]US]UA"_var.locate("US", valueno)) {/*ok*/} else  {abort("locate" ": " ^ lasterror());} // returns true and valueno = 2
	}

	printl("locate(in target, out setting, const int fieldno, const int valueno = 0) const;");
	{
		var setting;
		if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", setting, 2, 3)) {/*ok*/} else  {abort("locate" ": " ^ lasterror());} // returns true and setting = 4
	}

	printl("locateby(const char* ordercode, in target, out valueno) const;");
	{
		var valueno; if ("aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) {abort("locateby" ": " ^ lasterror());} // returns false and valueno = 2 where it could be correctly inserted.
	}

	printl("locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno = 0) const;");
	{
		var setting;
		if ("f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", setting, 3)) {abort("locateby" ": " ^ lasterror());} // returns false and setting = 2 where it could be correctly inserted.
	}

	printl("locateusing(const char* usingchar, in target) const;");
	{
		if ("AB,EF,CD"_var.locateusing(",", "EF")) {/*ok*/} else  {abort("locateusing" ": " ^ lasterror());} // true
	}

	printl("locateusing(const char* usingchar, in target, out setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;");
	{
		var setting;
		if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", setting, 3)) {/*ok*/} else  {abort("locateusing" ": " ^ lasterror());} // returns true and setting = 2
	}

	var conn;

	printl("connect(in conninfo = "");");
	{
		let conninfo = "dbname=exodus user=exodus password=somesillysecret";
		if (not conn.connect(conninfo)) {abort("connect" ": " ^ lasterror());};
		// or
		if (not connect()) {abort("connect" ": " ^ lasterror());}
		// or
		if (not connect("exodus")) {abort("connect" ": " ^ lasterror());}
	}

	printl("attach(in filenames) const;");
	{
		let filenames = "definitions^dict.definitions"_var, conn = "exodus";
		if (conn.attach(filenames)) {/*ok*/} else  {abort("attach" ": " ^ lasterror());}
		// or
		if (attach(filenames)) {/*ok*/} else  {abort("attach" ": " ^ lasterror());}
	}

	printl("begintrans() const;");
	{
		if (not conn.begintrans()) {abort("begintrans" ": " ^ lasterror());}
		// or
		if (not begintrans()) {abort("begintrans" ": " ^ lasterror());}
	}

	printl("statustrans() const;");
	{
		if (conn.statustrans()) {/*ok*/} else  {abort("statustrans" ": " ^ lasterror());}
		// or
		if (statustrans()) {/*ok*/} else  {abort("statustrans" ": " ^ lasterror());}
	}

	printl("rollbacktrans() const;");
	{
		if (conn.rollbacktrans()) {/*ok*/} else  {abort("rollbacktrans" ": " ^ lasterror());}
		// or
		if (rollbacktrans()) {/*ok*/} else  {abort("rollbacktrans" ": " ^ lasterror());}
	}

	printl("committrans() const;");
	{
		if (conn.committrans()) {/*ok*/} else  {abort("committrans" ": " ^ lasterror());}
		// or
		if (committrans()) {/*ok*/} else  {abort("committrans" ": " ^ lasterror());}
	}

	printl("sqlexec(in sqlcmd) const;");
	{
		var sqlcmd = "select 2 + 2;", response;
		if (conn.sqlexec("select 2 + 2;")) {/*ok*/} else  {abort("sqlexec" ": " ^ lasterror());}
		// or
		if (sqlexec(sqlcmd)) {/*ok*/} else  {abort("sqlexec" ": " ^ lasterror());}
	}

	printl("disconnect();");
	{
		conn.disconnect();
		// or
		disconnect();
	}

	printl("disconnectall();");
	{
		conn.disconnectall();
		// or
		disconnectall();
	}

	printl("dbcreate(in new_dbname, in old_dbname = "") const;");
	{
		var conn = "exodus";
		if (conn.dbcreate("xo_gendoc_testdb")) {/*ok*/} else  {abort("dbcreate" ": " ^ lasterror());}
		// or
		if (dbcreate("xo_gendoc_testdb")) {abort("dbcreate" ": " ^ lasterror());}
	}

	printl("dbcopy(in from_dbname, in to_dbname) const;");
	{
		var conn = "exodus";
		if (conn.dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) {/*ok*/} else  {abort("dbcopy" ": " ^ lasterror());}
		// or
		if (dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) {abort("dbcopy" ": " ^ lasterror());}
	}

	printl("dblist() const;");
	{
		let v1 = conn.dblist();
		// or
		let v2 = dblist();
	}

	printl("dbdelete(in dbname) const;");
	{
		var conn = "exodus";
		if (conn.dbdelete("xo_gendoc_testdb2")) {/*ok*/} else  {abort("dbdelete" ": " ^ lasterror());}
		// or
		if (dbdelete("xo_gendoc_testdb2")) {abort("dbdelete" ": " ^ lasterror());}
	}

	printl("createfile(in filename) const;");
	{
		let filename = "xo_gendoc_temp", conn = "exodus";
		if (conn.createfile(filename)) {/*ok*/} else  {abort("createfile" ": " ^ lasterror());}
		// or
		if (createfile(filename)) {abort("createfile" ": " ^ lasterror());}
	}

	printl("renamefile(in filename, in newfilename) const;");
	{
		let conn = "exodus", filename = "xo_gendoc_temp", new_filename = "xo_gendoc_temp2";
		if (conn.renamefile(filename, new_filename)) {/*ok*/} else  {abort("renamefile" ": " ^ lasterror());}
		// or
		if (renamefile(filename, new_filename)) {abort("renamefile" ": " ^ lasterror());}
	}

	printl("listfiles() const;");
	{
		var conn = "exodus";
		if (not conn.listfiles()) {abort("listfiles" ": " ^ lasterror());}
		// or
		if (not listfiles()) {abort("listfiles" ": " ^ lasterror());}
	}

	printl("clearfile(in filename) const;");
	{
		let conn = "exodus", filename = "xo_gendoc_temp2";
		if (not conn.clearfile(filename)) {abort("clearfile" ": " ^ lasterror());}
		// or
		if (not clearfile(filename)) {abort("clearfile" ": " ^ lasterror());}
	}

	printl("deletefile(in filename) const;");
	{
		let conn = "exodus", filename = "xo_gendoc_temp2";
		if (conn.deletefile(filename)) {/*ok*/} else  {abort("deletefile" ": " ^ lasterror());}
		// or
		if (deletefile(filename)) {abort("deletefile" ": " ^ lasterror());}
	}

	var conn_or_file;

	printl("reccount(in filename = "") const;");
	{
		let conn = "exodus", filename = "xo_clients";
		var nrecs1 = conn.reccount(filename);
		// or
		var nrecs2 = reccount(filename);
	}

	var file;

	printl("open(in dbfilename, in connection = "");");
	{
		var file, filename = "definitions";
		if (not file.open(filename)) {abort("open" ": " ^ lasterror());}
		// or
		if (not open(filename to file)) {abort("open" ": " ^ lasterror());}
	}

	printl("close() const;");
	{
		var file = "definitions";
		file.close();
		// or
		close(file);
	}

	printl("createindex(in fieldname, in dictfile = "") const;");
	{
		var filename = "definitions", fieldname = "DATE_TIME";
		if (not filename.deleteindex(fieldname)) {}; // Cleanup first
		if (filename.createindex(fieldname)) {/*ok*/} else  {abort("createindex" ": " ^ lasterror());}
		// or
		if (createindex(filename, fieldname)) {abort("createindex" ": " ^ lasterror());}
	}

	printl("listindex(in file_or_filename = "", in fieldname = "") const;");
	{
		var conn = "exodus";
		if (conn.listindex()) {/*ok*/} else  {abort("listindex" ": " ^ lasterror());} // includes xo_clients__date_time
		// or
		if (listindex()) {/*ok*/} else  {abort("listindex" ": " ^ lasterror());}
	}

	printl("deleteindex(in fieldname) const;");
	{
		var file = "definitions", fieldname = "DATE_TIME";
		if (file.deleteindex(fieldname)) {/*ok*/} else  {abort("deleteindex" ": " ^ lasterror());}
		// or
		if (deleteindex(file, fieldname)) {abort("deleteindex" ": " ^ lasterror());}
	}

	printl("lock(in key) const;");
	{
		var file = "xo_clients", key = "1000";
		if (file.lock(key)) {/*ok*/} else  {abort("lock" ": " ^ lasterror());}
		// or
		if (lock(file, key)) {abort("lock" ": " ^ lasterror());}
	}

	printl("unlock(in key) const;");
	{
		var file = "xo_clients", key = "1000";
		if (file.unlock(key)) {/*ok*/} else  {abort("unlock" ": " ^ lasterror());}
		// or
		if (unlock(file, key)) {abort("unlock" ": " ^ lasterror());}
	}

	printl("unlockall() const;");
	{
		var conn = "exodus";
		if (not conn.unlockall()) {abort("unlockall" ": " ^ lasterror());}
		// or
		if (not unlockall(conn)) {abort("unlockall" ": " ^ lasterror());}
	}

	var rec;

	printl("write(in file, in key) const;");
	{
		let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
		let file = "xo_clients", key = "GD001";
		if (not file.deleterecord(key)) {}; // Cleanup first
		rec.write(file, key);
		// or
		write(rec on file, key);
	}

	printl("read(in file, in key);");
	{
		var rec;
		let file = "xo_clients", key = "GD001";
		if (not rec.read(file, key)) {abort("read" ": " ^ lasterror());} // rec -> "Client GD^G^20855^30000^1001.00^20855.76539"_var
		// or
		if (not read(rec from file, key)) {abort("read" ": " ^ lasterror());}
	}

	printl("deleterecord(in key) const;");
	{
		let file = "xo_clients", key = "GD001";
		if (file.deleterecord(key)) {/*ok*/} else  {abort("deleterecord" ": " ^ lasterror());}
		// or
		if (deleterecord(file, key)) {abort("deleterecord" ": " ^ lasterror());}
	}

	printl("insertrecord(in file, in key) const;");
	{
		let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
		let file = "xo_clients", key = "GD001";
		if (rec.insertrecord(file, key)) {/*ok*/} else  {abort("insertrecord" ": " ^ lasterror());}
		// or
		if (insertrecord(rec on file, key)) {abort("insertrecord" ": " ^ lasterror());}
	}

	printl("updaterecord(in file, in key) const;");
	{
		let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
		let file = "xo_clients", key = "GD001";
		if (not rec.updaterecord(file, key)) {abort("updaterecord" ": " ^ lasterror());}
		// or
		if (not updaterecord(rec on file, key)) {abort("updaterecord" ": " ^ lasterror());}
	}

	printl("readf(in file, in key, const int fieldno);");
	{
		var field, file = "xo_clients", key = "GD001", fieldno = 2;
		if (not field.readf(file, key, fieldno)) {abort("readf" ": " ^ lasterror());} // field -> "G"
		// or
		if (not readf(field from file, key, fieldno)) {abort("readf" ": " ^ lasterror());}
	}

	printl("writef(in file, in key, const int fieldno) const;");
	{
		var field = "f3", file = "definitions", key = "1000", fieldno = 3;
		field.writef(file, key, fieldno);
		// or
		writef(field on file, key, fieldno);
	}

	printl("writec(in file, in key) const;");
	{
		let rec = "Client XD^X^20855^30000^1001.00^20855.76539"_var;
		let file = "xo_clients", key = "XD001";
		rec.writec(file, key);
		// or
		writec(rec on file, key);
	}

	printl("readc(in file, in key);");
	{
		var rec;
		let file = "xo_clients", key = "XD001";
		if (rec.readc(file, key)) {/*ok*/} else  {abort("readc" ": " ^ lasterror());}
		// or
		if (readc(rec from file, key)) {/*ok*/} else  {abort("readc" ": " ^ lasterror());}
		// Verify not in actual file by using read() not readc()
		if (read(rec from file, key)) abort("Error: " ^ key ^ " should not be in the actual file"); // error
	}

	var dbfile;

	printl("deletec(in key) const;");
	{
		var file = "xo_clients", key = "XD001";
		if (file.deletec(key)) {/*ok*/} else  {abort("deletec" ": " ^ lasterror());}
		// or
		if (deletec(file, key)) {abort("deletec" ": " ^ lasterror());}
	}

	printl("cleardbcache() const;");
	{
		conn.cleardbcache();
		// or
		cleardbcache(conn);
	}

	printl("xlate(in filename, in fieldno, const char* mode) const;");
	{
		let key = "SB001";
		let client_name = key.xlate("xo_clients", 1, "X"); assert(client_name == "Client AAA");
		// or
		let name_and_type = xlate("xo_clients", key, "NAME_AND_TYPE", "X"); assert("Client AAA (A)");
	}

	printl("date() const;");
	{
		var today1 = var().date(); // e.g. was 20821 from 2025-01-01 00:00:00 UTC
		// or
		var today2 = date();
	}

	printl("time() const;");
	{
		var now1 = var().time(); // range 0 - 86399 since there are 24*60*60 (86400) seconds in a day.
		// or
		var now2 = time();
	}

	printl("ostime() const;");
	{
		var now1 = var().ostime(); // e.g. 23343.704387955 approx. 06:29:03 UTC
		// or
		var now2 = ostime();
	}

	printl("timestamp() const;");
	{
		var now1 = var().timestamp(); // was 20821.99998842593 around 2025-01-01 23:59:59 UTC
		// or
		var now2 = timestamp();
	}

	printl("timestamp(in ostime) const;");
	{
		var idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
		var ts1 = idate.timestamp(itime); // 20821.99998842593
		// or
		var ts2 = timestamp(idate, itime);
	}

	printl("ossleep(const int milliseconds) const;");
	{
		var().ossleep(500); // sleep for 500ms
		// or
		ossleep(500);
	}

	var file_dir_list;

	printl("oswait(const int milliseconds) const;");
	{
		let v1 = ".^/etc/hosts"_var.oswait(500); // e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
		// or
		let v2 = oswait(".^/etc/hosts"_var, 500);
	}

	var osfilevar;

	printl("osopen(in osfilename, const bool utf8 = true) const;");
	{
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		if (oswrite("" on osfilename)) {/*ok*/} else  {abort("osopen" ": " ^ lasterror());} // Create an empty os file
		var ostempfile;
		if (ostempfile.osopen(osfilename)) {/*ok*/} else  {abort("osopen" ": " ^ lasterror());}
		// or
		if (osopen(osfilename to ostempfile)) {/*ok*/} else  {abort("osopen" ": " ^ lasterror());}
	}

	printl("osbwrite(in osfilevar, io offset) const;");
	{
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		let text = "aaa=123\nbbb=456\n";
		var offset = osfile(osfilename).f(1); // size of file -> append
		if (text.osbwrite(osfilename, offset)) {/*ok*/} else  {abort("osbwrite" ": " ^ lasterror());} // offset -> 16
		// or
		if (not osbwrite(text on osfilename, offset)) {abort("osbwrite" ": " ^ lasterror());}
	}

	printl("osbread(in osfilevar, io offset, const int length);");
	{
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		var text, offset = 0;
		if (text.osbread(osfilename, offset, 8)) {/*ok*/} else  {abort("osbread" ": " ^ lasterror());} // text -> "aaa=123\n", offset = 8
		// or
		if (osbread(text from osfilename, offset, 1024)) {/*ok*/} else  {abort("osbread" ": " ^ lasterror());} // text -> "bbb=456\n"), offset = 16
	}

	printl("osclose() const;");
	{
		osfilevar.osclose();
		// or
		osclose(osfilevar);
	}

	printl("oswrite(in osfilename, const char* codepage = "") const;");
	{
		let text = "aaa = 123\nbbb = 456";
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		if (text.oswrite(osfilename)) {/*ok*/} else  {abort("oswrite" ": " ^ lasterror());}
		// or
		if (oswrite(text on osfilename)) {/*ok*/} else  {abort("oswrite" ": " ^ lasterror());}
	}

	printl("osread(const char* osfilename, const char* codepage = "");");
	{
		var text;
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		if (text.osread(osfilename)) {/*ok*/} else  {abort("osread" ": " ^ lasterror());} // text -> "aaa = 123\nbbb = 456"
		// or
		if (osread(text from osfilename)) {/*ok*/} else  {abort("osread" ": " ^ lasterror());}
	}

	var osfileordirname;

	printl("osrename(in new_dirpath_or_filepath) const;");
	{
		let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		let to_osfilename = from_osfilename ^ ".bak";
		if (not osremove(to_osfilename)) {}; // Cleanup first
		if (from_osfilename.osrename(to_osfilename)) {/*ok*/} else  {abort("osrename" ": " ^ lasterror());}
		// or
		if (osrename(from_osfilename, to_osfilename)) {abort("osrename" ": " ^ lasterror());}
	}

	printl("osmove(in to_osfilename) const;");
	{
		let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf.bak";
		let to_osfilename = from_osfilename.cut(-4);
		if (not osremove(to_osfilename)) {}; // Cleanup first
		if (from_osfilename.osmove(to_osfilename)) {/*ok*/} else  {abort("osmove" ": " ^ lasterror());}
		// or
		if (osmove(from_osfilename, to_osfilename)) {abort("osmove" ": " ^ lasterror());}
	}

	printl("oscopy(in to_osfilename) const;");
	{
		let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		let to_osfilename = from_osfilename ^ ".bak";
		if (from_osfilename.oscopy(to_osfilename)) {/*ok*/} else  {abort("oscopy" ": " ^ lasterror());};
		// or
		if (oscopy(from_osfilename, to_osfilename)) {/*ok*/} else  {abort("oscopy" ": " ^ lasterror());}
	}

	var osfilename;

	printl("osremove() const;");
	{
		let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
		if (osfilename.osremove()) {/*ok*/} else  {abort("osremove" ": " ^ lasterror());}
		// or
		if (osremove(osfilename)) {abort("osremove" ": " ^ lasterror());}
	}

	var dirpath;

	printl("oslist(SV globpattern = "", const int mode = 0) const;");
	{
		var entries1 = "/etc/"_var.oslist("*.cfg"); // "adduser.conf^ca-certificates.conf^ {abort("oslist" ": " ^ lasterror());}"
		// or
		var entries2 = oslist("/etc/" "*.conf");
	}

	var osfileordirpath;

	printl("osinfo(const int mode = 0) const;");
	{
		var info1 = "/etc/hosts"_var.osinfo(); // "221^20597^78309"
		// or
		var info2 = osinfo("/etc/hosts");
	}

	printl("osfile() const;");
	{
		var fileinfo1 = "/etc/hosts"_var.osfile(); // "221^20597^78309"
		// or
		var fileinfo2 = osfile("/etc/hosts");
	}

	printl("osdir() const;");
	{
		var dirinfo1 = "/etc/"_var.osdir(); // "^20848^44464"
		// or
		var dirinfo2 = osfile("/etc/");
	}

	printl("osmkdir() const;");
	{
		let osdirname = "xo_test/aaa";
		if (osrmdir(osdirname)) {}; // Cleanup first
		if (osdirname.osmkdir()) {/*ok*/} else  {abort("osmkdir" ": " ^ lasterror());}
		// or
		if (osmkdir(osdirname)) {abort("osmkdir" ": " ^ lasterror());}
	}

	printl("oscwd(in newpath) const;");
	{
		let osdirname = "xo_test/aaa";
		if (osdirname.oscwd()) {/*ok*/} else  {abort("oscwd" ": " ^ lasterror());}
		// or
		if (oscwd(osdirname)) {/*ok*/} else  {abort("oscwd" ": " ^ lasterror());}
	}

	printl("oscwd() const;");
	{
		var cwd1 = var().oscwd(); // {abort("oscwd" ": " ^ lasterror());} "xo_test/aaa"
		// or
		var cwd2 = oscwd();
	}

	printl("osrmdir(bool evenifnotempty = false) const;");
	{
		let osdirname = "xo_test/aaa";
		if (oscwd("../..")) {/*ok*/} else  {abort("osrmdir" ": " ^ lasterror());} // Change up before removing
		if (osdirname.osrmdir()) {/*ok*/} else  {abort("osrmdir" ": " ^ lasterror());}
		// or
		if (osrmdir(osdirname)) {abort("osrmdir" ": " ^ lasterror());}
	}

	var command;

	printl("osshell() const;");
	{
		let cmd = "echo $HOME";
		if (cmd.osshell()) {/*ok*/} else  {abort("osshell" ": " ^ lasterror());}
		// or
		if (osshell(cmd)) {/*ok*/} else  {abort("osshell" ": " ^ lasterror());}
	}

	var instr;

	printl("osshellread(in oscmd);");
	{
		let cmd = "echo $HOME";
		var text;
		if (text.osshellread(cmd)) {/*ok*/} else  {abort("osshellread" ": " ^ lasterror());}
		// or capturing stdout but ignoring exit status
		text = osshellread(cmd);
	}

	var outstr;

	printl("osshellwrite(in oscmd) const;");
	{
		let outtext = "abc xyz";
		if (outtext.osshellwrite("grep xyz")) {/*ok*/} else  {abort("osshellwrite" ": " ^ lasterror());}
		// or
		if (osshellwrite(outtext, "grep xyz")) {/*ok*/} else  {abort("osshellwrite" ": " ^ lasterror());}
	}

	printl("ostempdirpath() const;");
	{
		let v1 = var().ostempdirpath(); // e.g. "/tmp/"
		// or
		let v2 = ostempdirpath();
	}

	printl("ostempfilename() const;");
	{
		var temposfilename1 = var().ostempfilename(); // "/tmp/~exoEcLj3C"
		// or
		var temposfilename2 = ostempfilename();
	}

	var envvalue;

	printl("ossetenv(SV envcode) const;");
	{
		let envcode = "EXO_ABC", envvalue = "XYZ";
		envvalue.ossetenv(envcode);
		// or
		ossetenv(envcode, envvalue);
	}

	printl("osgetenv(SV envcode);");
	{
		var envvalue1;
		if (envvalue1.osgetenv("HOME")) {/*ok*/} else  {abort("osgetenv" ": " ^ lasterror());} // e.g. "/home/exodus"
		// or
		var envvalue2 = osgetenv("EXO_ABC"); // "XYZ"
	}

	printl("ospid() const;");
	{
		let pid1 = var().ospid(); // e.g. 663237
		// or
		let pid2 = ospid();
	}

	printl("ostid() const;");
	{
		let tid1 = var().ostid(); // e.g. 663237
		// or
		let tid2 = ostid();
	}

	printl("version() const;");
	{
		let v1 = var().version(); // "29 JAN 2025 14:56:52"
	}

	printl("osflush() const;");
	{
		var().osflush();
		// or
		osflush();
	}


	return 0;
}

programexit()
