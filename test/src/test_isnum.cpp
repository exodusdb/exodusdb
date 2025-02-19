#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

// clang-format off

#define LOOPS 1

	var begin, end;

	for (int i = 0; i lt LOOPS; i++)
	// mixture of strings in the following proporsion: ([14:58:30] neosys: )
	//	15% 11111
	//	 5% -11111
	//	15% 11111.11
	//	 5% -11111.11
	//	50% xxxxxx
	{
		//0
		assert(var("").isnum());
		assert(var("11111").isnum());
		assert(var("2").isnum());
		assert(var("00000876229").isnum());
		assert(var("+999").isnum());
		assert(var("12222").isnum());
		assert(var("22222222").isnum());
		assert(var("+2").isnum());
		assert(var("222").isnum());
		assert(var("222").isnum());
		assert(var("222").isnum());
		assert(var("222").isnum());
		assert(var("222").isnum());
		assert(var("222").isnum());

		assert(var("0").isnum());
		assert(var("00").isnum());
		assert(var("0000000000").isnum());
		assert(var("-0000000000").isnum());
		assert(var("-0").isnum());
		assert(var("-00").isnum());
		assert(var("+0").isnum());
		assert(var("+00").isnum());

		assert(var("0.0").isnum());
		assert(var("-0.0").isnum());
		assert(var("0.00").isnum());
		assert(var("-0.00").isnum());
		assert(var("00.0").isnum());
		assert(var("-00.0").isnum());
		assert(var("00.00").isnum());
		assert(var("-00.00").isnum());
		assert(var("0.").isnum());
		assert(var("-0.").isnum());
		assert(var(".0").isnum());
		assert(var("-.0").isnum());

		assert(var("-2").isnum());
		assert(var("-222").isnum());
		assert(var("-222").isnum());

		assert(var("11111.11").isnum());
		assert(var("8.76").isnum());
		assert(var("8.76").isnum());
		assert(var("8.76").isnum());
		assert(var("8.76").isnum());
		assert(var(".9").isnum());
		assert(var("+.9").isnum());
		assert(var("000008.76229").isnum());
		assert(var("2208.76229").isnum());
		assert(var(".9000000000").isnum());
		assert(var("+.1239").isnum());
		assert(var("000008.76229").isnum());
		assert(var("2208.76229").isnum());
		assert(var("8.76229").isnum());
		assert(var("8.76").isnum());

		assert(var("-11111.11").isnum());
		assert(var("-0.").isnum());
		assert(var("-2.27770").isnum());
		assert(var("-.222").isnum());
		assert(var("-2.22").isnum());

		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());
		assert(!var("missing 10%").isnum());

		assert(!var("xxxxxx").isnum());
		assert(!var("TABLE1").isnum());
		assert(!var("/").isnum());
		assert(!var("\\").isnum());
		assert(!var("X").isnum());
		assert(!var("aaa").isnum());
		assert(!var("FILE.TXT").isnum());
		assert(!var("USER1").isnum());
		assert(!var("12345;").isnum());
		assert(!var("X+Y").isnum());
		assert(!var("x").isnum());

		assert(!var("+").isnum());
		assert(!var("-").isnum());
		assert(!var(".").isnum());
		assert(!var("++").isnum());
		assert(!var("+-").isnum());
		assert(!var("+.").isnum());
		assert(!var("-+").isnum());
		assert(!var(".+").isnum());
		assert(!var(".-").isnum());
		assert(!var("..").isnum());

		assert(!var("++++").isnum());
		assert(!var("+++-").isnum());
		assert(!var("+++.").isnum());
		assert(!var("+++9").isnum());
		assert(!var("++-+").isnum());
		assert(!var("++--").isnum());
		assert(!var("++-.").isnum());
		assert(!var("++-9").isnum());
		assert(!var("++.+").isnum());
		assert(!var("++.-").isnum());

		assert(!var("x").isnum());
		assert(!var("+x").isnum());
		assert(!var("-x").isnum());
		assert(!var(".x").isnum());
		assert(!var("9x").isnum());

		assert(!var("NTUSER.DAT{6cced2f1-6e01-11de-8bed-001e0bcd1824}.TM.blf").isnum());
		assert(!var("Monthly dynamic of stock rates for Jan 2010").isnum());
		assert(!var("Cellphone: +45 77 66 233").isnum());
		assert(!var("Address: 221b BAKER STREET").isnum());
		assert(!var("\n").isnum());

		assert(!var("x").isnum());
		assert(!var(" ").isnum());
		assert(!var(" 0").isnum());
		assert(!var("0 ").isnum());
		assert(!var("+x").isnum());
		assert(!var("-x").isnum());
		assert(!var(".x").isnum());
		assert(!var("9x").isnum());

		assert(!var("NTUSER.DAT{6cced2f1-6e01-11de-8bed-001e0bcd1824}.TM.blf").isnum());
		assert(!var("Monthly dynamic of stock rates for Jan 2010").isnum());
		assert(!var("Cellphone: +45 77 66 233").isnum());
		assert(!var("Address: 221b BAKER STREET").isnum());
		assert(!var("\n").isnum());

		//0
		assert(var("").isnum());

		//1
		assert(!var("+").isnum());
		assert(!var("-").isnum());
		assert(!var(".").isnum());
		assert( var("9").isnum());//

		//2
		assert(!var("++").isnum());
		assert(!var("+-").isnum());
		assert(!var("+.").isnum());
		assert( var("+9").isnum());//

		assert(!var("-+").isnum());
		assert(!var("--").isnum());
		assert(!var("-.").isnum());
		assert( var("-9").isnum());//

		assert(!var(".+").isnum());
		assert(!var(".-").isnum());
		assert(!var("..").isnum());
		assert( var(".9").isnum());//

		assert(!var("9+").isnum());
		assert(!var("9-").isnum());
		assert( var("9.").isnum());//
		assert( var("99").isnum());//

		//3
		assert(!var("+++").isnum());
		assert(!var("++-").isnum());
		assert(!var("++.").isnum());
		assert(!var("++9").isnum());
		assert(!var("+-+").isnum());
		assert(!var("+--").isnum());
		assert(!var("+-.").isnum());
		assert(!var("+-9").isnum());
		assert(!var("+.+").isnum());
		assert(!var("+.-").isnum());
		assert(!var("+..").isnum());
		assert( var("+.9").isnum());//
		assert(!var("+9+").isnum());
		assert(!var("+9-").isnum());
		assert( var("+9.").isnum());//
		assert( var("+99").isnum());//

		assert(!var("-++").isnum());
		assert(!var("-+-").isnum());
		assert(!var("-+.").isnum());
		assert(!var("-+9").isnum());
		assert(!var("--+").isnum());
		assert(!var("---").isnum());
		assert(!var("--.").isnum());
		assert(!var("--9").isnum());
		assert(!var("-.+").isnum());
		assert(!var("-.-").isnum());
		assert(!var("-..").isnum());
		assert( var("-.9").isnum());//
		assert(!var("-9+").isnum());
		assert(!var("-9-").isnum());
		assert( var("-9.").isnum());//
		assert( var("-99").isnum());//

		assert(!var(".++").isnum());
		assert(!var(".+-").isnum());
		assert(!var(".+.").isnum());
		assert(!var(".+9").isnum());
		assert(!var(".-+").isnum());
		assert(!var(".--").isnum());
		assert(!var(".-.").isnum());
		assert(!var(".-9").isnum());
		assert(!var("..+").isnum());
		assert(!var("..-").isnum());
		assert(!var("...").isnum());
		assert(!var("..9").isnum());
		assert(!var(".9+").isnum());
		assert(!var(".9-").isnum());
		assert(!var(".9.").isnum());
		assert( var(".99").isnum());//

		assert(!var("9++").isnum());
		assert(!var("9+-").isnum());
		assert(!var("9+.").isnum());
		assert(!var("9+9").isnum());
		assert(!var("9-+").isnum());
		assert(!var("9--").isnum());
		assert(!var("9-.").isnum());
		assert(!var("9-9").isnum());
		assert(!var("9.+").isnum());
		assert(!var("9.-").isnum());
		assert(!var("9..").isnum());
		assert( var("9.9").isnum());//
		assert(!var("99+").isnum());
		assert(!var("99-").isnum());
		assert( var("99.").isnum());//
		assert( var("999").isnum());//

		//4
		assert(!var("++++").isnum());
		assert(!var("+++-").isnum());
		assert(!var("+++.").isnum());
		assert(!var("+++9").isnum());
		assert(!var("++-+").isnum());
		assert(!var("++--").isnum());
		assert(!var("++-.").isnum());
		assert(!var("++-9").isnum());
		assert(!var("++.+").isnum());
		assert(!var("++.-").isnum());
		assert(!var("++..").isnum());
		assert(!var("++.9").isnum());
		assert(!var("++9+").isnum());
		assert(!var("++9-").isnum());
		assert(!var("++9.").isnum());
		assert(!var("++99").isnum());
		assert(!var("+-++").isnum());
		assert(!var("+-+-").isnum());
		assert(!var("+-+.").isnum());
		assert(!var("+-+9").isnum());
		assert(!var("+--+").isnum());
		assert(!var("+---").isnum());
		assert(!var("+--.").isnum());
		assert(!var("+--9").isnum());
		assert(!var("+-.+").isnum());
		assert(!var("+-.-").isnum());
		assert(!var("+-..").isnum());
		assert(!var("+-.9").isnum());
		assert(!var("+-9+").isnum());
		assert(!var("+-9-").isnum());
		assert(!var("+-9.").isnum());
		assert(!var("+-99").isnum());
		assert(!var("+.++").isnum());
		assert(!var("+.+-").isnum());
		assert(!var("+.+.").isnum());
		assert(!var("+.+9").isnum());
		assert(!var("+.-+").isnum());
		assert(!var("+.--").isnum());
		assert(!var("+.-.").isnum());
		assert(!var("+.-9").isnum());
		assert(!var("+..+").isnum());
		assert(!var("+..-").isnum());
		assert(!var("+...").isnum());
		assert(!var("+..9").isnum());
		assert(!var("+.9+").isnum());
		assert(!var("+.9-").isnum());
		assert(!var("+.9.").isnum());
		assert( var("+.99").isnum());//
		assert(!var("+9++").isnum());
		assert(!var("+9+-").isnum());
		assert(!var("+9+.").isnum());
		assert(!var("+9+9").isnum());
		assert(!var("+9-+").isnum());
		assert(!var("+9--").isnum());
		assert(!var("+9-.").isnum());
		assert(!var("+9-9").isnum());
		assert(!var("+9.+").isnum());
		assert(!var("+9.-").isnum());
		assert(!var("+9..").isnum());
		assert( var("+9.9").isnum());//
		assert(!var("+99+").isnum());
		assert(!var("+99-").isnum());
		assert( var("+99.").isnum());//
		assert( var("+999").isnum());//

		assert(!var("-+++").isnum());
		assert(!var("-++-").isnum());
		assert(!var("-++.").isnum());
		assert(!var("-++9").isnum());
		assert(!var("-+-+").isnum());
		assert(!var("-+--").isnum());
		assert(!var("-+-.").isnum());
		assert(!var("-+-9").isnum());
		assert(!var("-+.+").isnum());
		assert(!var("-+.-").isnum());
		assert(!var("-+..").isnum());
		assert(!var("-+.9").isnum());
		assert(!var("-+9+").isnum());
		assert(!var("-+9-").isnum());
		assert(!var("-+9.").isnum());
		assert(!var("-+99").isnum());
		assert(!var("--++").isnum());
		assert(!var("--+-").isnum());
		assert(!var("--+.").isnum());
		assert(!var("--+9").isnum());
		assert(!var("---+").isnum());
		assert(!var("----").isnum());
		assert(!var("---.").isnum());
		assert(!var("---9").isnum());
		assert(!var("--.+").isnum());
		assert(!var("--.-").isnum());
		assert(!var("--..").isnum());
		assert(!var("--.9").isnum());
		assert(!var("--9+").isnum());
		assert(!var("--9-").isnum());
		assert(!var("--9.").isnum());
		assert(!var("--99").isnum());
		assert(!var("-.++").isnum());
		assert(!var("-.+-").isnum());
		assert(!var("-.+.").isnum());
		assert(!var("-.+9").isnum());
		assert(!var("-.-+").isnum());
		assert(!var("-.--").isnum());
		assert(!var("-.-.").isnum());
		assert(!var("-.-9").isnum());
		assert(!var("-..+").isnum());
		assert(!var("-..-").isnum());
		assert(!var("-...").isnum());
		assert(!var("-..9").isnum());
		assert(!var("-.9+").isnum());
		assert(!var("-.9-").isnum());
		assert(!var("-.9.").isnum());
		assert( var("-.99").isnum());//
		assert(!var("-9++").isnum());
		assert(!var("-9+-").isnum());
		assert(!var("-9+.").isnum());
		assert(!var("-9+9").isnum());
		assert(!var("-9-+").isnum());
		assert(!var("-9--").isnum());
		assert(!var("-9-.").isnum());
		assert(!var("-9-9").isnum());
		assert(!var("-9.+").isnum());
		assert(!var("-9.-").isnum());
		assert(!var("-9..").isnum());
		assert( var("-9.9").isnum());//
		assert(!var("-99+").isnum());
		assert(!var("-99-").isnum());
		assert( var("-99.").isnum());//
		assert( var("-999").isnum());//

		assert(!var(".+++").isnum());
		assert(!var(".++-").isnum());
		assert(!var(".++.").isnum());
		assert(!var(".++9").isnum());
		assert(!var(".+-+").isnum());
		assert(!var(".+--").isnum());
		assert(!var(".+-.").isnum());
		assert(!var(".+-9").isnum());
		assert(!var(".+.+").isnum());
		assert(!var(".+.-").isnum());
		assert(!var(".+..").isnum());
		assert(!var(".+.9").isnum());
		assert(!var(".+9+").isnum());
		assert(!var(".+9-").isnum());
		assert(!var(".+9.").isnum());
		assert(!var(".+99").isnum());
		assert(!var(".-++").isnum());
		assert(!var(".-+-").isnum());
		assert(!var(".-+.").isnum());
		assert(!var(".-+9").isnum());
		assert(!var(".--+").isnum());
		assert(!var(".---").isnum());
		assert(!var(".--.").isnum());
		assert(!var(".--9").isnum());
		assert(!var(".-.+").isnum());
		assert(!var(".-.-").isnum());
		assert(!var(".-..").isnum());
		assert(!var(".-.9").isnum());
		assert(!var(".-9+").isnum());
		assert(!var(".-9-").isnum());
		assert(!var(".-9.").isnum());
		assert(!var(".-99").isnum());
		assert(!var("..++").isnum());
		assert(!var("..+-").isnum());
		assert(!var("..+.").isnum());
		assert(!var("..+9").isnum());
		assert(!var("..-+").isnum());
		assert(!var("..--").isnum());
		assert(!var("..-.").isnum());
		assert(!var("..-9").isnum());
		assert(!var("...+").isnum());
		assert(!var("...-").isnum());
		assert(!var("....").isnum());
		assert(!var("...9").isnum());
		assert(!var("..9+").isnum());
		assert(!var("..9-").isnum());
		assert(!var("..9.").isnum());
		assert(!var("..99").isnum());
		assert(!var(".9++").isnum());
		assert(!var(".9+-").isnum());
		assert(!var(".9+.").isnum());
		assert(!var(".9+9").isnum());
		assert(!var(".9-+").isnum());
		assert(!var(".9--").isnum());
		assert(!var(".9-.").isnum());
		assert(!var(".9-9").isnum());
		assert(!var(".9.+").isnum());
		assert(!var(".9.-").isnum());
		assert(!var(".9..").isnum());
		assert(!var(".9.9").isnum());
		assert(!var(".99+").isnum());
		assert(!var(".99-").isnum());
		assert(!var(".99.").isnum());
		assert( var(".999").isnum());//

		assert(!var("9+++").isnum());
		assert(!var("9++-").isnum());
		assert(!var("9++.").isnum());
		assert(!var("9++9").isnum());
		assert(!var("9+-+").isnum());
		assert(!var("9+--").isnum());
		assert(!var("9+-.").isnum());
		assert(!var("9+-9").isnum());
		assert(!var("9+.+").isnum());
		assert(!var("9+.-").isnum());
		assert(!var("9+..").isnum());
		assert(!var("9+.9").isnum());
		assert(!var("9+9+").isnum());
		assert(!var("9+9-").isnum());
		assert(!var("9+9.").isnum());
		assert(!var("9+99").isnum());
		assert(!var("9-++").isnum());
		assert(!var("9-+-").isnum());
		assert(!var("9-+.").isnum());
		assert(!var("9-+9").isnum());
		assert(!var("9--+").isnum());
		assert(!var("9---").isnum());
		assert(!var("9--.").isnum());
		assert(!var("9--9").isnum());
		assert(!var("9-.+").isnum());
		assert(!var("9-.-").isnum());
		assert(!var("9-..").isnum());
		assert(!var("9-.9").isnum());
		assert(!var("9-9+").isnum());
		assert(!var("9-9-").isnum());
		assert(!var("9-9.").isnum());
		assert(!var("9-99").isnum());
		assert(!var("9.++").isnum());
		assert(!var("9.+-").isnum());
		assert(!var("9.+.").isnum());
		assert(!var("9.+9").isnum());
		assert(!var("9.-+").isnum());
		assert(!var("9.--").isnum());
		assert(!var("9.-.").isnum());
		assert(!var("9.-9").isnum());
		assert(!var("9..+").isnum());
		assert(!var("9..-").isnum());
		assert(!var("9...").isnum());
		assert(!var("9..9").isnum());
		assert(!var("9.9+").isnum());
		assert(!var("9.9-").isnum());
		assert(!var("9.9.").isnum());
		assert( var("9.99").isnum());//
		assert(!var("99++").isnum());
		assert(!var("99+-").isnum());
		assert(!var("99+.").isnum());
		assert(!var("99+9").isnum());
		assert(!var("99-+").isnum());
		assert(!var("99--").isnum());
		assert(!var("99-.").isnum());
		assert(!var("99-9").isnum());
		assert(!var("99.+").isnum());
		assert(!var("99.-").isnum());
		assert(!var("99..").isnum());
		assert( var("99.9").isnum());//
		assert(!var("999+").isnum());
		assert(!var("999-").isnum());
		assert( var("999.").isnum());//
		assert( var("9999").isnum());//

		//various hand coded negatives

		assert(!var("x").isnum());
		assert(!var("+x").isnum());
		assert(!var("-x").isnum());
		assert(!var(".x").isnum());
		assert(!var("9x").isnum());
		assert(!var(" 0").isnum());
		assert(!var("0 ").isnum());
		assert(!var(" ").isnum());

		//various hand coded positives

		assert(var("09").isnum());
		assert(var("09.").isnum());
		assert(var(".09").isnum());
		assert(var("09.09").isnum());
		assert(var("9").isnum());
		assert(var("9.").isnum());
		assert(var(".9").isnum());
		assert(var("9.9").isnum());

		assert(var("+09").isnum());
		assert(var("+09.").isnum());
		assert(var("+.09").isnum());
		assert(var("+09.09").isnum());
		assert(var("+9").isnum());
		assert(var("+9.").isnum());
		assert(var("+.9").isnum());
		assert(var("+9.9").isnum());

		assert(var("-09").isnum());
		assert(var("-09.").isnum());
		assert(var("-.09").isnum());
		assert(var("-09.09").isnum());
		assert(var("-9").isnum());
		assert(var("-9.").isnum());
		assert(var("-.9").isnum());
		assert(var("-9.9").isnum());
	}

	{
		assert(var("9e9").isnum());
		assert(var("-9e9").isnum());
		assert(var("-9e-9").isnum());
		assert(!var("e").isnum());
		assert(!var("9e").isnum());
		assert(!var("e9").isnum());

		assert(var("+9e-9").isnum());
		assert(var("9e+9").isnum());

		assert(var("9e9").isnum());
		assert(var("-9e9").isnum());
		assert(var("-9e-9").isnum());
		assert(!var("e").isnum());
		assert(!var("9e").isnum());
		assert(!var("e9").isnum());

		assert(!var("9e9.").isnum());
		assert(!var("-9e9.").isnum());
		assert(!var("-9e-9.").isnum());
		assert(!var("e.").isnum());
		assert(!var("9e.").isnum());
		assert(!var("e9.").isnum());

		assert(!var("+9e-9.").isnum());
		assert(!var("9e+9.").isnum());

		assert(!var("9e9.").isnum());
		assert(!var("-9e9.").isnum());
		assert(!var("-9e-9.").isnum());
		assert(!var("e.").isnum());
		assert(!var("9e.").isnum());
		assert(!var("e9.").isnum());

		assert(!var("++123").isnum());
		assert(!var("+-123").isnum());
		assert(!var("--123").isnum());
		assert(!var("-+123").isnum());

		assert(var("+123").isnum());
		assert(var("+123").isnum());
		assert(var("-123").isnum());
		assert(var("-123").isnum());

		assert(!var("+123-").isnum());
		assert(!var("+123-").isnum());
		assert(!var("-123-").isnum());
		assert(!var("-123-").isnum());

		assert(!var("+123+").isnum());
		assert(!var("+123+").isnum());
		assert(!var("-123+").isnum());
		assert(!var("-123+").isnum());
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
