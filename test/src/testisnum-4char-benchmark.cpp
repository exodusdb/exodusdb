#include <assert.h>
#include <exodus/program.h>

#define LOOPS 10000
programinit()

function main()
{
	print("Start of isnum() benchmark: ");
	var begin = timedate();
	printl( begin.oconv("MTS"));
	for( int i=0; i<LOOPS; i++) {
	//full combination of up to four characters + - . 9

	//0
	assert(var("").isnum());

	//1
	assert(!var("+").isnum());
	assert(!var("-").isnum());
	assert(!var(".").isnum());
	assert( var("9").isnum());

	//2
	assert(!var("++").isnum());
	assert(!var("+-").isnum());
	assert(!var("+.").isnum());
	assert( var("+9").isnum());

	assert(!var("-+").isnum());
	assert(!var("--").isnum());
	assert(!var("-.").isnum());
	assert( var("-9").isnum());

	assert(!var(".+").isnum());
	assert(!var(".-").isnum());
	assert(!var("..").isnum());
	assert( var(".9").isnum());

	assert(!var("9+").isnum());
	assert(!var("9-").isnum());
	assert( var("9.").isnum());
	assert( var("99").isnum());

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
	assert( var("+.9").isnum());
	assert(!var("+9+").isnum());
	assert(!var("+9-").isnum());
	assert( var("+9.").isnum());
	assert( var("+99").isnum());

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
	assert( var("-.9").isnum());
	assert(!var("-9+").isnum());
	assert(!var("-9-").isnum());
	assert( var("-9.").isnum());
	assert( var("-99").isnum());

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
	assert( var(".99").isnum());

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
	assert( var("9.9").isnum());
	assert(!var("99+").isnum());
	assert(!var("99-").isnum());
	assert( var("99.").isnum());
	assert( var("999").isnum());


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
	assert( var("+.99").isnum());
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
	assert( var("+9.9").isnum());
	assert(!var("+99+").isnum());
	assert(!var("+99-").isnum());
	assert( var("+99.").isnum());
	assert( var("+999").isnum());

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
	assert( var("-.99").isnum());
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
	assert( var("-9.9").isnum());
	assert(!var("-99+").isnum());
	assert(!var("-99-").isnum());
	assert( var("-99.").isnum());
	assert( var("-999").isnum());

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
	assert( var(".999").isnum());

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
	assert( var("9.99").isnum());
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
	assert( var("99.9").isnum());
	assert(!var("999+").isnum());
	assert(!var("999-").isnum());
	assert( var("999.").isnum());
	assert( var("9999").isnum());

	//various hand coded negatives

	assert(!var("x").isnum());
	assert(!var("+x").isnum());
	assert(!var("-x").isnum());
	assert(!var(".x").isnum());
	assert(!var("9x").isnum());

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

	printl("OK");

	var end = timedate();
	print("End of  isnum()  benchmark: ");
	printl( end.oconv("MTS"));

	return 0;
}

programexit()
