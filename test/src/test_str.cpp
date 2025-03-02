#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

//Ignore compiler warnings when testing that first(-1) etc. throw a runtime error.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic push

#include <exodus/program.h>
programinit()

function main() {

	{
        // using using SNITCH in var.h
		// Test temp var ^ temp var uses nrvo and std::move to save copying temporary strings
		var x = "============================================";
		var y = var("aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+") ^ var("1111+2222+3333+4444+5555+6666+7777+");
/*
0x7ffdd50bbcf8 var_base Str  1          0          0 '============================================'
0x7ffdd50bbcc0 var_base Str  1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+'
0x7ffdd50bbd30 var_base Str  1          0          0 '1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbd68 var_base Str  1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbd30 var_base -    1          0          0 '1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbcc0 var_base -    1          0          0 ''
0x7ffdd50bbd68 var_base -    1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbcf8 var_base -    1          0          0 '============================================'
*/
	}
	{
        // using using SNITCH in var.h
		// Test temp var ^ temp var uses nrvo and std::move to save copying temporary strings
		var x = "--------------------------------------------";
		var y = var("aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+") ^ "1111+2222+3333+4444+5555+6666+7777+";
/*
0x7ffdd50bbcf8 var_base Str  1          0          0 '--------------------------------------------'
0x7ffdd50bbcc0 var_base Str  1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+'
0x7ffdd50bbd68 var_base Str  1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbcc0 var_base -    1          0          0 ''
0x7ffdd50bbd68 var_base -    1          0          0 'aaaa+bbbb+cccc+dddd+eeee+ffff+gggg+1111+2222+3333+4444+5555+6666+7777+'
0x7ffdd50bbcf8 var_base -    1          0          0 '--------------------------------------------'
*/
	}

	{
		// on lvalue

		// var::first(n)

		var f = "abc";

		try {
			assert(f.first(-1).outputl() eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {var(e.message).errputl();}

		{
			assert(f.first() eq "a");
			assert(f.first(0) eq "");
			assert(f.first(1) eq "a");
			assert(f.first(2) eq "ab");
			assert(f.first(3) eq "abc");
			assert(f.first(4) eq "abc");

			assert(first(f) eq "a");
			assert(first(f, 2) eq "ab");
		}
		{
			assert(f.starts("a"));
			assert(not f.starts(""));
			assert(f.starts("a"));
			assert(f.starts("ab"));
			assert(f.starts("abc"));
			assert(f.starts("abc"));

//			assert(starts(f, "a"));
//			assert(starts(f, "ab"));
		}

		// var::last(n)

		var l = "abc";

		try {
			assert(l.last(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {var(e.message).errputl();}

		{
			assert(l.last() eq "c");
			assert(l.last(0) eq "");
			assert(l.last(1) eq "c");
			assert(l.last(2) eq "bc");
			assert(l.last(3) eq "abc");
			assert(l.last(4) eq "abc");

			assert(last(l) eq "c");
			assert(last(l, 2) eq "bc");
		}
		{
			assert(not l.ends(""));
			assert(l.ends("c"));
			assert(l.ends("bc"));
			assert(l.ends("abc"));
			assert(not l.ends("abcd"));

//			assert(ends(l, "c"));
//			assert(ends(l, "bc"));
		}
	}

	{
		// on rvalue

		// var().first(n)

		try {
			assert(var("abc").first(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {var(e.message).errputl();}

		{
//			assert(var("abc").first(-1) eq "");
//			assert(var("abc").first(-1) eq "c");
			assert(var("abc").first(0) eq "");
			assert(var("abc").first(1) eq "a");
			assert(var("abc").first(2) eq "ab");
			assert(var("abc").first(3) eq "abc");
			assert(var("abc").first(4) eq "abc");
		}

		{
			assert(not var("abc").starts(""));
			assert(var("abc").starts("a"));
			assert(var("abc").starts("ab"));
			assert(var("abc").starts("abc"));
			assert(var("abc").starts("abc"));
			assert(not var("abc").starts("abcd"));
		}

		// var().last(n)

		try {
			assert(var("abc").last(-1) eq "");
			assert(false);
		}
		catch (VarNonPositive& e) {var(e.message).errputl();}

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
		catch (VarNonPositive& e) {var(e.message).errputl();}

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
		catch (VarNonPositive& e) {var(e.message).errputl();}

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
