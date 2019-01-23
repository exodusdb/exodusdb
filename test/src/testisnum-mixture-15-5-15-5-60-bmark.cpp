#include <assert.h>
#include <exodus/exodus.h>

#define LOOPS 20000
program()
{
	var begin, end;

	print( L"Start of isnum() benchmark: ");
	begin = begin.timedate();
	printl( begin.oconv( L"MTS"));
	for( int i=0; i<LOOPS; i++)
		// mixture of strings in the following proporsion: ([14:58:30] neosys: )
		//	15% 11111
		//	 5% -11111
		//	15% 11111.11
		//	 5% -11111.11
		//	50% xxxxxx
		{
		//0
		assert( var(L"11111").isnum());
		assert( var(L"2").isnum());
		assert( var(L"00000876229").isnum());
		assert( var(L"+999").isnum());
		assert( var(L"0").isnum());
		assert( var(L"12222").isnum());
		assert( var(L"22222222").isnum());
		assert( var(L"0000000000").isnum());
		assert( var(L"+2").isnum());
		assert( var(L"222").isnum());
		assert( var(L"222").isnum());
		assert( var(L"222").isnum());
		assert( var(L"222").isnum());
		assert( var(L"222").isnum());
		assert( var(L"222").isnum());

		assert( var(L"-11111").isnum());
		assert( var(L"-0000000000").isnum());
		assert( var(L"-2").isnum());
		assert( var(L"-222").isnum());
		assert( var(L"-222").isnum());

		assert( var(L"11111.11").isnum());
		assert( var(L"8.76").isnum());
		assert( var(L"8.76").isnum());
		assert( var(L"8.76").isnum());
		assert( var(L"8.76").isnum());
		assert( var(L".9").isnum());
		assert( var(L"+.9").isnum());
		assert( var(L"000008.76229").isnum());
		assert( var(L"2208.76229").isnum());
		assert( var(L".9000000000").isnum());
		assert( var(L"+.1239").isnum());
		assert( var(L"000008.76229").isnum());
		assert( var(L"2208.76229").isnum());
		assert( var(L"8.76229").isnum());
		assert( var(L"8.76").isnum());

		assert( var(L"-11111.11").isnum());
		assert( var(L"-0.").isnum());
		assert( var(L"-2.27770").isnum());
		assert( var(L"-.222").isnum());
		assert( var(L"-2.22").isnum());

		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());
		assert(!var(L"missing 10%").isnum());

		assert(!var(L"xxxxxx").isnum());
		assert(!var(L"TABLE1").isnum());
		assert(!var(SLASH).isnum());
		assert(!var(L"X").isnum());
		assert(!var(L"aaa").isnum());
		assert(!var(L"FILE.TXT").isnum());
		assert(!var(L"USER1").isnum());
		assert(!var(L"12345;").isnum());
		assert(!var(L"X+Y").isnum());
		assert(!var(L"x").isnum());

		assert(!var(L"+").isnum());
		assert(!var(L"-").isnum());
		assert(!var(L".").isnum());
		assert(!var(L"++").isnum());
		assert(!var(L"+-").isnum());
		assert(!var(L"+.").isnum());
		assert(!var(L"-+").isnum());
		assert(!var(L".+").isnum());
		assert(!var(L".-").isnum());
		assert(!var(L"..").isnum());

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

		assert(!var(L"x").isnum());
		assert(!var(L"+x").isnum());
		assert(!var(L"-x").isnum());
		assert(!var(L".x").isnum());
		assert(!var(L"9x").isnum());

		assert(!var(L"NTUSER.DAT{6cced2f1-6e01-11de-8bed-001e0bcd1824}.TM.blf").isnum());
		assert(!var(L"Monthly dynamic of stock rates for Jan 2010").isnum());
		assert(!var(L"Cellphone: +45 77 66 233").isnum());
		assert(!var(L"Address: 221b BAKER STREET").isnum());
		assert(!var(L"\n").isnum());

		assert(!var(L"x").isnum());
		assert(!var(L"+x").isnum());
		assert(!var(L"-x").isnum());
		assert(!var(L".x").isnum());
		assert(!var(L"9x").isnum());

		assert(!var(L"NTUSER.DAT{6cced2f1-6e01-11de-8bed-001e0bcd1824}.TM.blf").isnum());
		assert(!var(L"Monthly dynamic of stock rates for Jan 2010").isnum());
		assert(!var(L"Cellphone: +45 77 66 233").isnum());
		assert(!var(L"Address: 221b BAKER STREET").isnum());
		assert(!var(L"\n").isnum());
	}

	printl("OK");

	end = end.timedate();
	print( L"End of  isnum()  benchmark: ");
	printl( end.oconv( L"MTS"));

}
