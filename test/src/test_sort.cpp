#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

	// Also tests var::reverse

	var sep = "^";

	assert(sort(var("10^20.1^2^1^20"), sep).outputl("Numbers sort correctly : ") eq "1^2^10^20^20.1");

	assert(sort(var("10^20.1^^2^1^20^"), sep).outputl("Empty fields sort first : ") eq "^^1^2^10^20^20.1");

	assert(sort(var("10a^2a"), sep).outputl("Numbers with suffixes sort alphabetically : ") eq "10a^2a");

	assert(sort(var("a^ccc^bb"), sep).outputl("Simple alphabetic sort : ") eq "a^bb^ccc");

	assert(sort(var("A^a^ccc^BB^bb"), sep).outputl("alphabetic case insensitive sort : ") eq "a^A^bb^BB^ccc");

//	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
//	assert(sort(var("bb^10^aa^2^10a^2a"), sep).outputl("Numbers with suffixes dont get sorted well : ") eq "10a^2^10^2a^aa^bb");
//
//	//numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2
//	assert(sort(var("bb^10^aa^2^a10^a2"), sep).outputl("Numbers before letters : ") eq "2^10^a10^a2^aa^bb");

    printl("GCC: Numbers with suffixes dont get sorted well on gcc but do on clang");
    printl(" numbers sorted correctly and before letters but a2 and a10 are sorted a10 before a2");
    printl("Clang: Better");
    var un_sorted1 = "bb^10^aa^2^10a^2a";
    var sorted1 = sort(un_sorted1, sep);
    var gccsorted1 = "10a^2^10^2a^aa^bb";
    var clgsorted1 = "2^10^10a^2a^aa^bb";
    TRACE(sorted1)
    TRACE(gccsorted1)
    TRACE(clgsorted1)
    assert(sorted1 eq gccsorted1 or sorted1 eq clgsorted1);

    var un_sorted2 = "bb^10^aa^2^a10^a2";
    var sorted2 = sort(un_sorted2, sep);
    var gccsorted2 = "10a^2^10^2a^aa^bb";
    var clgsorted2 = "2^10^a10^a2^aa^bb";
    TRACE(sorted2)
    TRACE(gccsorted2)
    TRACE(clgsorted2)
    assert(sorted2 eq gccsorted2 or sorted2 eq clgsorted2);

	//sort via dim
	//////////////

	//sort is actually done by splitting into a dimensioned array of vars and comparing them using the standard var<var operator
	assert(var("10^2").split(sep).sort().join(sep)     eq "2^10");
	assert(var("10a^2a").split(sep).sort().join(sep)   eq "10a^2a");
	assert(var("a^ccc^bb").split(sep).sort().join(sep) eq "a^bb^ccc");

//	//mixed numbers, letters and numbers with letter suffixes doesnt sort well
//	assert(var("bb^10^aa^2^10a^2a").split(sep).sort().join(sep) eq "10a^2^10^2a^aa^bb");
//
//	assert(var("bb^10^aa^2^a10^a2").split(sep).sort().join(sep) eq "2^10^a10^a2^aa^bb");
//
	var dimsorted1 = un_sorted1.split(sep).sort().join(sep);
	assert(dimsorted1 eq gccsorted1 or dimsorted1 eq clgsorted1);

	var dimsorted2 = un_sorted2.split(sep).sort().join(sep);
	assert(dimsorted2 eq gccsorted2 or dimsorted2 eq clgsorted2);

	{

		// test var::reverse with single byte separator

		var sepchar = FM;

		// Identical code above/below

		assert(reverse("", sepchar)    == "");
		assert(reverse("a", sepchar)   == "a");
		assert(reverse("ab", sepchar)  == "ab");
		assert(reverse("abc", sepchar) == "abc");

		assert(reverse(sepchar, sepchar)                         == sepchar);
		assert(reverse(sepchar ^ sepchar, sepchar)               == (sepchar ^ sepchar));
		assert(reverse(sepchar ^ sepchar ^ sepchar, sepchar)     == (sepchar ^ sepchar ^ sepchar));

		assert(reverse(sepchar ^ "", sepchar)                    == (""    ^ sepchar));
		assert(reverse(sepchar ^ "a", sepchar)                   == ("a"   ^ sepchar));
		assert(reverse(sepchar ^ "ab", sepchar)                  == ("ab"  ^ sepchar));
		assert(reverse(sepchar ^ "abc", sepchar)                 == ("abc" ^ sepchar));

		assert(reverse("" ^ sepchar, sepchar)                    == (sepchar ^ ""));
		assert(reverse("a" ^ sepchar, sepchar)                   == (sepchar ^ "a"));
		assert(reverse("ab" ^ sepchar, sepchar)                  == (sepchar ^ "ab"));
		assert(reverse("abc" ^ sepchar, sepchar)                 == (sepchar ^ "abc"));

		assert(reverse("x" ^ sepchar ^ "", sepchar)              == (""    ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "a", sepchar)             == ("a"   ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "ab", sepchar)            == ("ab"  ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "abc", sepchar)           == ("abc" ^ sepchar ^ "x"));

		assert(reverse(sepchar ^ "x" ^ sepchar ^ "", sepchar)    == (""    ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "a", sepchar)   == ("a"   ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "ab", sepchar)  == ("ab"  ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "abc", sepchar) == ("abc" ^ sepchar ^ "x" ^ sepchar));

	}
	{
		// test var::reverse with multiple byte separator

		var sepchar = "qw";

		// Identical code above/below

		assert(reverse("", sepchar)    == "");
		assert(reverse("a", sepchar)   == "a");
		assert(reverse("ab", sepchar)  == "ab");
		assert(reverse("abc", sepchar) == "abc");

		assert(reverse(sepchar, sepchar)                         == sepchar);
		assert(reverse(sepchar ^ sepchar, sepchar)               == (sepchar ^ sepchar));
		assert(reverse(sepchar ^ sepchar ^ sepchar, sepchar)     == (sepchar ^ sepchar ^ sepchar));

		assert(reverse(sepchar ^ "", sepchar)                    == (""    ^ sepchar));
		assert(reverse(sepchar ^ "a", sepchar)                   == ("a"   ^ sepchar));
		assert(reverse(sepchar ^ "ab", sepchar)                  == ("ab"  ^ sepchar));
		assert(reverse(sepchar ^ "abc", sepchar)                 == ("abc" ^ sepchar));

		assert(reverse("" ^ sepchar, sepchar)                    == (sepchar ^ ""));
		assert(reverse("a" ^ sepchar, sepchar)                   == (sepchar ^ "a"));
		assert(reverse("ab" ^ sepchar, sepchar)                  == (sepchar ^ "ab"));
		assert(reverse("abc" ^ sepchar, sepchar)                 == (sepchar ^ "abc"));

		assert(reverse("x" ^ sepchar ^ "", sepchar)              == (""    ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "a", sepchar)             == ("a"   ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "ab", sepchar)            == ("ab"  ^ sepchar ^ "x"));
		assert(reverse("x" ^ sepchar ^ "abc", sepchar)           == ("abc" ^ sepchar ^ "x"));

		assert(reverse(sepchar ^ "x" ^ sepchar ^ "", sepchar)    == (""    ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "a", sepchar)   == ("a"   ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "ab", sepchar)  == ("ab"  ^ sepchar ^ "x" ^ sepchar));
		assert(reverse(sepchar ^ "x" ^ sepchar ^ "abc", sepchar) == ("abc" ^ sepchar ^ "x" ^ sepchar));

	}

	{
		// Shuffle dim using member functions

		dim cards1 = "🂡^🂢^🂣^🂤^🂥^🂦^🂧^🂨^🂩^🂪^🂫^🂬^🂭^🂮^🂱^🂲^🂳^🂴^🂵^🂶^🂷^🂸^🂹^🂺^🂻^🂼^🂽^🂾^🃁^🃂^🃃^🃄^🃅^🃆^🃇^🃈^🃉^🃊^🃋^🃌^🃍^🃎^🃑^🃒^🃓^🃔^🃕^🃖^🃗^🃘^🃙^🃚^🃛^🃜^🃝^🃞"_var.split();
		dim cards0 = cards1;
		dim cardsx = cards1;

		initrnd(123);
		dim cards2 = cards1.shuffle();

		initrnd(123);
		dim cardsy = cardsx.shuffler();

		initrnd(1234);
		dim cards3 = dim(cards1).shuffle();

		logputl();
		TRACE(cards1.join())
		TRACE(cards2.join())
		TRACE(cards3.join())
		TRACE(cardsx.join())
		TRACE(cardsy.join())
		assert(cards1.join() == cards0.join());
		assert(cards2.join() == cardsx.join());
		assert(cardsx.join() == cardsy.join());
		assert(cards2.join() == "🂪^🂥^🃎^🃛^🃇^🂣^🂶^🂭^🃜^🂵^🂱^🃞^🃙^🂼^🃍^🃕^🂲^🃃^🂤^🃊^🂧^🃗^🂺^🂢^🃘^🃚^🃑^🂮^🂸^🃆^🂹^🃄^🃓^🂩^🂴^🂬^🂨^🃔^🃝^🃂^🃉^🂾^🃒^🂫^🃈^🃁^🃋^🂽^🂦^🂻^🂳^🃌^🂡^🂷^🃅^🃖"_var);
		assert(cards3.join() == "🃑^🂤^🂮^🂹^🃋^🃇^🂻^🃂^🃞^🂲^🂾^🂺^🂢^🂳^🃍^🃊^🃙^🂣^🂡^🂧^🃈^🃌^🃕^🃁^🃘^🃔^🂷^🂭^🃎^🂶^🂱^🃅^🃉^🃗^🂽^🂨^🂬^🃛^🂴^🃆^🂥^🃜^🃖^🂫^🃝^🃚^🃓^🂦^🂪^🃒^🂵^🃃^🂸^🂼^🂩^🃄"_var);
	}

	{
		// Shuffle var using member functions

		var cards1 = "🂡^🂢^🂣^🂤^🂥^🂦^🂧^🂨^🂩^🂪^🂫^🂬^🂭^🂮^🂱^🂲^🂳^🂴^🂵^🂶^🂷^🂸^🂹^🂺^🂻^🂼^🂽^🂾^🃁^🃂^🃃^🃄^🃅^🃆^🃇^🃈^🃉^🃊^🃋^🃌^🃍^🃎^🃑^🃒^🃓^🃔^🃕^🃖^🃗^🃘^🃙^🃚^🃛^🃜^🃝^🃞"_var.lower();
		var cards0 = cards1;
		var cardsx = cards1;

		initrnd(123);
		var cards2 = cards1.shuffle(VM);

		initrnd(123);
		var cardsy = cardsx.shuffler(VM);

		initrnd(1234);
		var cards3 = cards1.shuffle(VM);

		logputl();
		TRACE(cards1)
		TRACE(cards2)
		TRACE(cards3)
		TRACE(cardsx)
		TRACE(cardsy)
		assert(cards1 == cards0);
		assert(cards2 == cardsx);
		assert(cardsx == cardsy);
		assert(cards2 == "🂪^🂥^🃎^🃛^🃇^🂣^🂶^🂭^🃜^🂵^🂱^🃞^🃙^🂼^🃍^🃕^🂲^🃃^🂤^🃊^🂧^🃗^🂺^🂢^🃘^🃚^🃑^🂮^🂸^🃆^🂹^🃄^🃓^🂩^🂴^🂬^🂨^🃔^🃝^🃂^🃉^🂾^🃒^🂫^🃈^🃁^🃋^🂽^🂦^🂻^🂳^🃌^🂡^🂷^🃅^🃖"_var.lower());
		assert(cards3 == "🃑^🂤^🂮^🂹^🃋^🃇^🂻^🃂^🃞^🂲^🂾^🂺^🂢^🂳^🃍^🃊^🃙^🂣^🂡^🂧^🃈^🃌^🃕^🃁^🃘^🃔^🂷^🂭^🃎^🂶^🂱^🃅^🃉^🃗^🂽^🂨^🂬^🃛^🂴^🃆^🂥^🃜^🃖^🂫^🃝^🃚^🃓^🂦^🂪^🃒^🂵^🃃^🂸^🂼^🂩^🃄"_var.lower());
	}

	{
		// Shuffle var using free functions

		var cards1 = "🂡^🂢^🂣^🂤^🂥^🂦^🂧^🂨^🂩^🂪^🂫^🂬^🂭^🂮^🂱^🂲^🂳^🂴^🂵^🂶^🂷^🂸^🂹^🂺^🂻^🂼^🂽^🂾^🃁^🃂^🃃^🃄^🃅^🃆^🃇^🃈^🃉^🃊^🃋^🃌^🃍^🃎^🃑^🃒^🃓^🃔^🃕^🃖^🃗^🃘^🃙^🃚^🃛^🃜^🃝^🃞"_var.lower();
		var cards0 = cards1;
		var cardsx = cards1;

		initrnd(123);
		var cards2 = shuffle(cards1, VM);

		initrnd(123);
		var cardsy = shuffler(cardsx, VM);

		initrnd(1234);
		var cards3 = shuffle(cards1, VM);

		logputl();
		TRACE(cards1)
		TRACE(cards2)
		TRACE(cards3)
		TRACE(cardsx)
		TRACE(cardsy)
		assert(cards1 == cards0);
		assert(cards2 == cardsx);
		assert(cardsy == cardsx);
		assert(cards2 == "🂪^🂥^🃎^🃛^🃇^🂣^🂶^🂭^🃜^🂵^🂱^🃞^🃙^🂼^🃍^🃕^🂲^🃃^🂤^🃊^🂧^🃗^🂺^🂢^🃘^🃚^🃑^🂮^🂸^🃆^🂹^🃄^🃓^🂩^🂴^🂬^🂨^🃔^🃝^🃂^🃉^🂾^🃒^🂫^🃈^🃁^🃋^🂽^🂦^🂻^🂳^🃌^🂡^🂷^🃅^🃖"_var.lower());
		assert(cards3 == "🃑^🂤^🂮^🂹^🃋^🃇^🂻^🃂^🃞^🂲^🂾^🂺^🂢^🂳^🃍^🃊^🃙^🂣^🂡^🂧^🃈^🃌^🃕^🃁^🃘^🃔^🂷^🂭^🃎^🂶^🂱^🃅^🃉^🃗^🂽^🂨^🂬^🃛^🂴^🃆^🂥^🃜^🃖^🂫^🃝^🃚^🃓^🂦^🂪^🃒^🂵^🃃^🂸^🂼^🂩^🃄"_var.lower());
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
