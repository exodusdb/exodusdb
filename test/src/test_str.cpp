#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {

	{
		// on lvalue

		// var::first(n)

		var f = "abc";

		try {
			assert(f.first(-1).outputl() eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		assert(f.first(0) eq "");
		assert(f.first(1) eq "a");
		assert(f.first(2) eq "ab");
		assert(f.first(3) eq "abc");
		assert(f.first(4) eq "abc");

		// var::last(n)

		var l = "abc";

		try {
			assert(l.last(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		assert(l.last(0) eq "");
		assert(l.last(1) eq "c");
		assert(l.last(2) eq "bc");
		assert(l.last(3) eq "abc");
		assert(l.last(4) eq "abc");
	}

	{
		// on rvalue

		// var().first(n)

		try {
			assert(var("abc").first(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		assert(var("abc").first(0) eq "");
		assert(var("abc").first(1) eq "a");
		assert(var("abc").first(2) eq "ab");
		assert(var("abc").first(3) eq "abc");
		assert(var("abc").first(4) eq "abc");

		// var().last(n)

		try {
			assert(var("abc").last(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		assert(var("abc").last(0) eq "");
		assert(var("abc").last(1) eq "c");
		assert(var("abc").last(2) eq "bc");
		assert(var("abc").last(3) eq "abc");
		assert(var("abc").last(4) eq "abc");
	}

	{
		// mutation

		// var.firster(n)

		try {
			{var f = "abc";f.firster(-1);assert(f eq "");}
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		{var f = "abc";f.firster(0);assert(f eq "");}
		{var f = "abc";f.firster(1);assert(f eq "a");}
		{var f = "abc";f.firster(2);assert(f eq "ab");}
		{var f = "abc";f.firster(3);assert(f eq "abc");}
		{var f = "abc";f.firster(4);assert(f eq "abc");}

		// var.laster(n)

		try {
			{var l = "abc";l.laster(-1);assert(l eq "");}
			assert(false);
		}
		catch (VarNonPositive& e) {e.description.errputl();}

		{var l = "abc";l.laster(0);assert(l eq "");}
		{var l = "abc";l.laster(1);assert(l eq "c");}
		{var l = "abc";l.laster(2);assert(l eq "bc");}
		{var l = "abc";l.laster(3);assert(l eq "abc");}
		{var l = "abc";l.laster(4);assert(l eq "abc");}

	}
	{
		assert(var("").parse().outputl() eq ""_var);
		assert(var("a").parse().outputl() eq "a"_var);
		assert(var("ab").parse().outputl() eq "ab"_var);

		assert(var("a b").parse().outputl() eq "a^b"_var);
		assert(var(" a b").parse().outputl() eq "^a^b"_var);
		assert(var("a b ").parse().outputl() eq "a^b^"_var);
		assert(var(" a b ").parse().outputl() eq "^a^b^"_var);

		assert(var("a\\ b").parse().outputl() eq "a\\ b"_var);
		assert(var("\\ a\\ b").parse().outputl() eq "\\ a\\ b"_var);
		assert(var("a\\ b\\ ").parse().outputl() eq "a\\ b\\ "_var);
		assert(var("\\ a\\ b\\ ").parse().outputl() eq "\\ a\\ b\\ "_var);

		assert(var("a' 'b ").parse().outputl() eq "a' 'b^"_var);
		assert(var("a\" \"b ").parse().outputl() eq "a\" \"b^"_var);
		assert(var("a\"'\"b ").parse().outputl() eq "a\"'\"b^"_var);
		assert(var("a'\"'b ").parse().outputl() eq "a'\"'b^"_var);

		assert(var("a' 'b ").parse().outputl() eq "a' 'b^"_var);
		assert(var("a\" \"b ").parse().outputl() eq "a\" \"b^"_var);
		assert(var("a\"'\"b ").parse().outputl() eq "a\"'\"b^"_var);
		assert(var("a'\"'b ").parse().outputl() eq "a'\"'b^"_var);

		assert(var("\\a' 'b ").parse().outputl() eq "\\a' 'b^"_var);
		assert(var("\\a\" \"b ").parse().outputl() eq "\\a\" \"b^"_var);
		assert(var("\\a\"'\"b ").parse().outputl() eq "\\a\"'\"b^"_var);
		assert(var("\\a'\"'b ").parse().outputl() eq "\\a'\"'b^"_var);

		assert(var("\\\\' 'b ").parse().outputl() eq "\\\\' 'b^"_var);
		assert(var("\\\\\" \"b ").parse().outputl() eq "\\\\\" \"b^"_var);
		assert(var("\\\\\"'\"b ").parse().outputl() eq "\\\\\"'\"b^"_var);
		assert(var("\\\\'\"'b ").parse().outputl() eq "\\\\'\"'b^"_var);

		assert(var("\\' 'b ").parse().outputl() eq "\\'^'b "_var);
		assert(var("\\' 'b ").parse().outputl() eq "\\'^'b "_var);

		assert(var("\\\" \"b ").parse().outputl() eq "\\\"^\"b "_var);
		assert(var("\\\" \"b ").parse().outputl() eq "\\\"^\"b "_var);

		assert(var("abc 'def gh'qwe").parse().outputl() eq "abc^'def gh'qwe"_var);

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
