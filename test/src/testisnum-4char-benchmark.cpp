#include <assert.h>
#include <exodus/exodus.h>

#define LOOPS 10000
program()
{
	var begin, end;

	print( L"Start of isnum() benchmark: ");
	begin = begin.timedate();
	printl( begin.oconv( L"MTS"));
	for( int i=0; i<LOOPS; i++) {
	//full combination of up to four characters + - . 9

	//0
	assert(var(L"").isnum());

	//1
	assert(!var(L"+").isnum());
	assert(!var(L"-").isnum());
	assert(!var(L".").isnum());
	assert( var(L"9").isnum());

	//2
	assert(!var(L"++").isnum());
	assert(!var(L"+-").isnum());
	assert(!var(L"+.").isnum());
	assert( var(L"+9").isnum());

	assert(!var(L"-+").isnum());
	assert(!var(L"--").isnum());
	assert(!var(L"-.").isnum());
	assert( var(L"-9").isnum());

	assert(!var(L".+").isnum());
	assert(!var(L".-").isnum());
	assert(!var(L"..").isnum());
	assert( var(L".9").isnum());

	assert(!var(L"9+").isnum());
	assert(!var(L"9-").isnum());
	assert( var(L"9.").isnum());
	assert( var(L"99").isnum());

	//3
	assert(!var(L"+++").isnum());
	assert(!var(L"++-").isnum());
	assert(!var(L"++.").isnum());
	assert(!var(L"++9").isnum());
	assert(!var(L"+-+").isnum());
	assert(!var(L"+--").isnum());
	assert(!var(L"+-.").isnum());
	assert(!var(L"+-9").isnum());
	assert(!var(L"+.+").isnum());
	assert(!var(L"+.-").isnum());
	assert(!var(L"+..").isnum());
	assert( var(L"+.9").isnum());
	assert(!var(L"+9+").isnum());
	assert(!var(L"+9-").isnum());
	assert( var(L"+9.").isnum());
	assert( var(L"+99").isnum());

	assert(!var(L"-++").isnum());
	assert(!var(L"-+-").isnum());
	assert(!var(L"-+.").isnum());
	assert(!var(L"-+9").isnum());
	assert(!var(L"--+").isnum());
	assert(!var(L"---").isnum());
	assert(!var(L"--.").isnum());
	assert(!var(L"--9").isnum());
	assert(!var(L"-.+").isnum());
	assert(!var(L"-.-").isnum());
	assert(!var(L"-..").isnum());
	assert( var(L"-.9").isnum());
	assert(!var(L"-9+").isnum());
	assert(!var(L"-9-").isnum());
	assert( var(L"-9.").isnum());
	assert( var(L"-99").isnum());

	assert(!var(L".++").isnum());
	assert(!var(L".+-").isnum());
	assert(!var(L".+.").isnum());
	assert(!var(L".+9").isnum());
	assert(!var(L".-+").isnum());
	assert(!var(L".--").isnum());
	assert(!var(L".-.").isnum());
	assert(!var(L".-9").isnum());
	assert(!var(L"..+").isnum());
	assert(!var(L"..-").isnum());
	assert(!var(L"...").isnum());
	assert(!var(L"..9").isnum());
	assert(!var(L".9+").isnum());
	assert(!var(L".9-").isnum());
	assert(!var(L".9.").isnum());
	assert( var(L".99").isnum());

	assert(!var(L"9++").isnum());
	assert(!var(L"9+-").isnum());
	assert(!var(L"9+.").isnum());
	assert(!var(L"9+9").isnum());
	assert(!var(L"9-+").isnum());
	assert(!var(L"9--").isnum());
	assert(!var(L"9-.").isnum());
	assert(!var(L"9-9").isnum());
	assert(!var(L"9.+").isnum());
	assert(!var(L"9.-").isnum());
	assert(!var(L"9..").isnum());
	assert( var(L"9.9").isnum());
	assert(!var(L"99+").isnum());
	assert(!var(L"99-").isnum());
	assert( var(L"99.").isnum());
	assert( var(L"999").isnum());


	//4
	assert(!var(L"++++").isnum());
	assert(!var(L"+++-").isnum());
	assert(!var(L"+++.").isnum());
	assert(!var(L"+++9").isnum());
	assert(!var(L"++-+").isnum());
	assert(!var(L"++--").isnum());
	assert(!var(L"++-.").isnum());
	assert(!var(L"++-9").isnum());
	assert(!var(L"++.+").isnum());
	assert(!var(L"++.-").isnum());
	assert(!var(L"++..").isnum());
	assert(!var(L"++.9").isnum());
	assert(!var(L"++9+").isnum());
	assert(!var(L"++9-").isnum());
	assert(!var(L"++9.").isnum());
	assert(!var(L"++99").isnum());
	assert(!var(L"+-++").isnum());
	assert(!var(L"+-+-").isnum());
	assert(!var(L"+-+.").isnum());
	assert(!var(L"+-+9").isnum());
	assert(!var(L"+--+").isnum());
	assert(!var(L"+---").isnum());
	assert(!var(L"+--.").isnum());
	assert(!var(L"+--9").isnum());
	assert(!var(L"+-.+").isnum());
	assert(!var(L"+-.-").isnum());
	assert(!var(L"+-..").isnum());
	assert(!var(L"+-.9").isnum());
	assert(!var(L"+-9+").isnum());
	assert(!var(L"+-9-").isnum());
	assert(!var(L"+-9.").isnum());
	assert(!var(L"+-99").isnum());
	assert(!var(L"+.++").isnum());
	assert(!var(L"+.+-").isnum());
	assert(!var(L"+.+.").isnum());
	assert(!var(L"+.+9").isnum());
	assert(!var(L"+.-+").isnum());
	assert(!var(L"+.--").isnum());
	assert(!var(L"+.-.").isnum());
	assert(!var(L"+.-9").isnum());
	assert(!var(L"+..+").isnum());
	assert(!var(L"+..-").isnum());
	assert(!var(L"+...").isnum());
	assert(!var(L"+..9").isnum());
	assert(!var(L"+.9+").isnum());
	assert(!var(L"+.9-").isnum());
	assert(!var(L"+.9.").isnum());
	assert( var(L"+.99").isnum());
	assert(!var(L"+9++").isnum());
	assert(!var(L"+9+-").isnum());
	assert(!var(L"+9+.").isnum());
	assert(!var(L"+9+9").isnum());
	assert(!var(L"+9-+").isnum());
	assert(!var(L"+9--").isnum());
	assert(!var(L"+9-.").isnum());
	assert(!var(L"+9-9").isnum());
	assert(!var(L"+9.+").isnum());
	assert(!var(L"+9.-").isnum());
	assert(!var(L"+9..").isnum());
	assert( var(L"+9.9").isnum());
	assert(!var(L"+99+").isnum());
	assert(!var(L"+99-").isnum());
	assert( var(L"+99.").isnum());
	assert( var(L"+999").isnum());

	assert(!var(L"-+++").isnum());
	assert(!var(L"-++-").isnum());
	assert(!var(L"-++.").isnum());
	assert(!var(L"-++9").isnum());
	assert(!var(L"-+-+").isnum());
	assert(!var(L"-+--").isnum());
	assert(!var(L"-+-.").isnum());
	assert(!var(L"-+-9").isnum());
	assert(!var(L"-+.+").isnum());
	assert(!var(L"-+.-").isnum());
	assert(!var(L"-+..").isnum());
	assert(!var(L"-+.9").isnum());
	assert(!var(L"-+9+").isnum());
	assert(!var(L"-+9-").isnum());
	assert(!var(L"-+9.").isnum());
	assert(!var(L"-+99").isnum());
	assert(!var(L"--++").isnum());
	assert(!var(L"--+-").isnum());
	assert(!var(L"--+.").isnum());
	assert(!var(L"--+9").isnum());
	assert(!var(L"---+").isnum());
	assert(!var(L"----").isnum());
	assert(!var(L"---.").isnum());
	assert(!var(L"---9").isnum());
	assert(!var(L"--.+").isnum());
	assert(!var(L"--.-").isnum());
	assert(!var(L"--..").isnum());
	assert(!var(L"--.9").isnum());
	assert(!var(L"--9+").isnum());
	assert(!var(L"--9-").isnum());
	assert(!var(L"--9.").isnum());
	assert(!var(L"--99").isnum());
	assert(!var(L"-.++").isnum());
	assert(!var(L"-.+-").isnum());
	assert(!var(L"-.+.").isnum());
	assert(!var(L"-.+9").isnum());
	assert(!var(L"-.-+").isnum());
	assert(!var(L"-.--").isnum());
	assert(!var(L"-.-.").isnum());
	assert(!var(L"-.-9").isnum());
	assert(!var(L"-..+").isnum());
	assert(!var(L"-..-").isnum());
	assert(!var(L"-...").isnum());
	assert(!var(L"-..9").isnum());
	assert(!var(L"-.9+").isnum());
	assert(!var(L"-.9-").isnum());
	assert(!var(L"-.9.").isnum());
	assert( var(L"-.99").isnum());
	assert(!var(L"-9++").isnum());
	assert(!var(L"-9+-").isnum());
	assert(!var(L"-9+.").isnum());
	assert(!var(L"-9+9").isnum());
	assert(!var(L"-9-+").isnum());
	assert(!var(L"-9--").isnum());
	assert(!var(L"-9-.").isnum());
	assert(!var(L"-9-9").isnum());
	assert(!var(L"-9.+").isnum());
	assert(!var(L"-9.-").isnum());
	assert(!var(L"-9..").isnum());
	assert( var(L"-9.9").isnum());
	assert(!var(L"-99+").isnum());
	assert(!var(L"-99-").isnum());
	assert( var(L"-99.").isnum());
	assert( var(L"-999").isnum());

	assert(!var(L".+++").isnum());
	assert(!var(L".++-").isnum());
	assert(!var(L".++.").isnum());
	assert(!var(L".++9").isnum());
	assert(!var(L".+-+").isnum());
	assert(!var(L".+--").isnum());
	assert(!var(L".+-.").isnum());
	assert(!var(L".+-9").isnum());
	assert(!var(L".+.+").isnum());
	assert(!var(L".+.-").isnum());
	assert(!var(L".+..").isnum());
	assert(!var(L".+.9").isnum());
	assert(!var(L".+9+").isnum());
	assert(!var(L".+9-").isnum());
	assert(!var(L".+9.").isnum());
	assert(!var(L".+99").isnum());
	assert(!var(L".-++").isnum());
	assert(!var(L".-+-").isnum());
	assert(!var(L".-+.").isnum());
	assert(!var(L".-+9").isnum());
	assert(!var(L".--+").isnum());
	assert(!var(L".---").isnum());
	assert(!var(L".--.").isnum());
	assert(!var(L".--9").isnum());
	assert(!var(L".-.+").isnum());
	assert(!var(L".-.-").isnum());
	assert(!var(L".-..").isnum());
	assert(!var(L".-.9").isnum());
	assert(!var(L".-9+").isnum());
	assert(!var(L".-9-").isnum());
	assert(!var(L".-9.").isnum());
	assert(!var(L".-99").isnum());
	assert(!var(L"..++").isnum());
	assert(!var(L"..+-").isnum());
	assert(!var(L"..+.").isnum());
	assert(!var(L"..+9").isnum());
	assert(!var(L"..-+").isnum());
	assert(!var(L"..--").isnum());
	assert(!var(L"..-.").isnum());
	assert(!var(L"..-9").isnum());
	assert(!var(L"...+").isnum());
	assert(!var(L"...-").isnum());
	assert(!var(L"....").isnum());
	assert(!var(L"...9").isnum());
	assert(!var(L"..9+").isnum());
	assert(!var(L"..9-").isnum());
	assert(!var(L"..9.").isnum());
	assert(!var(L"..99").isnum());
	assert(!var(L".9++").isnum());
	assert(!var(L".9+-").isnum());
	assert(!var(L".9+.").isnum());
	assert(!var(L".9+9").isnum());
	assert(!var(L".9-+").isnum());
	assert(!var(L".9--").isnum());
	assert(!var(L".9-.").isnum());
	assert(!var(L".9-9").isnum());
	assert(!var(L".9.+").isnum());
	assert(!var(L".9.-").isnum());
	assert(!var(L".9..").isnum());
	assert(!var(L".9.9").isnum());
	assert(!var(L".99+").isnum());
	assert(!var(L".99-").isnum());
	assert(!var(L".99.").isnum());
	assert( var(L".999").isnum());

	assert(!var(L"9+++").isnum());
	assert(!var(L"9++-").isnum());
	assert(!var(L"9++.").isnum());
	assert(!var(L"9++9").isnum());
	assert(!var(L"9+-+").isnum());
	assert(!var(L"9+--").isnum());
	assert(!var(L"9+-.").isnum());
	assert(!var(L"9+-9").isnum());
	assert(!var(L"9+.+").isnum());
	assert(!var(L"9+.-").isnum());
	assert(!var(L"9+..").isnum());
	assert(!var(L"9+.9").isnum());
	assert(!var(L"9+9+").isnum());
	assert(!var(L"9+9-").isnum());
	assert(!var(L"9+9.").isnum());
	assert(!var(L"9+99").isnum());
	assert(!var(L"9-++").isnum());
	assert(!var(L"9-+-").isnum());
	assert(!var(L"9-+.").isnum());
	assert(!var(L"9-+9").isnum());
	assert(!var(L"9--+").isnum());
	assert(!var(L"9---").isnum());
	assert(!var(L"9--.").isnum());
	assert(!var(L"9--9").isnum());
	assert(!var(L"9-.+").isnum());
	assert(!var(L"9-.-").isnum());
	assert(!var(L"9-..").isnum());
	assert(!var(L"9-.9").isnum());
	assert(!var(L"9-9+").isnum());
	assert(!var(L"9-9-").isnum());
	assert(!var(L"9-9.").isnum());
	assert(!var(L"9-99").isnum());
	assert(!var(L"9.++").isnum());
	assert(!var(L"9.+-").isnum());
	assert(!var(L"9.+.").isnum());
	assert(!var(L"9.+9").isnum());
	assert(!var(L"9.-+").isnum());
	assert(!var(L"9.--").isnum());
	assert(!var(L"9.-.").isnum());
	assert(!var(L"9.-9").isnum());
	assert(!var(L"9..+").isnum());
	assert(!var(L"9..-").isnum());
	assert(!var(L"9...").isnum());
	assert(!var(L"9..9").isnum());
	assert(!var(L"9.9+").isnum());
	assert(!var(L"9.9-").isnum());
	assert(!var(L"9.9.").isnum());
	assert( var(L"9.99").isnum());
	assert(!var(L"99++").isnum());
	assert(!var(L"99+-").isnum());
	assert(!var(L"99+.").isnum());
	assert(!var(L"99+9").isnum());
	assert(!var(L"99-+").isnum());
	assert(!var(L"99--").isnum());
	assert(!var(L"99-.").isnum());
	assert(!var(L"99-9").isnum());
	assert(!var(L"99.+").isnum());
	assert(!var(L"99.-").isnum());
	assert(!var(L"99..").isnum());
	assert( var(L"99.9").isnum());
	assert(!var(L"999+").isnum());
	assert(!var(L"999-").isnum());
	assert( var(L"999.").isnum());
	assert( var(L"9999").isnum());

	//various hand coded negatives

	assert(!var(L"x").isnum());
	assert(!var(L"+x").isnum());
	assert(!var(L"-x").isnum());
	assert(!var(L".x").isnum());
	assert(!var(L"9x").isnum());

	//various hand coded positives

	assert(var(L"09").isnum());
	assert(var(L"09.").isnum());
	assert(var(L".09").isnum());
	assert(var(L"09.09").isnum());
	assert(var(L"9").isnum());
	assert(var(L"9.").isnum());
	assert(var(L".9").isnum());
	assert(var(L"9.9").isnum());

	assert(var(L"+09").isnum());
	assert(var(L"+09.").isnum());
	assert(var(L"+.09").isnum());
	assert(var(L"+09.09").isnum());
	assert(var(L"+9").isnum());
	assert(var(L"+9.").isnum());
	assert(var(L"+.9").isnum());
	assert(var(L"+9.9").isnum());

	assert(var(L"-09").isnum());
	assert(var(L"-09.").isnum());
	assert(var(L"-.09").isnum());
	assert(var(L"-09.09").isnum());
	assert(var(L"-9").isnum());
	assert(var(L"-9.").isnum());
	assert(var(L"-.9").isnum());
	assert(var(L"-9.9").isnum());
	}

	printl("OK");

	end = end.timedate();
	print( L"End of  isnum()  benchmark: ");
	printl( end.oconv( L"MTS"));

}
