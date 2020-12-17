#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

function main(in d1, in d2, out result, io lang) {
	//c sys in,in,out,io

	//returns minimal text representation of date range
	// if @sentence='DATERANGETEXT' then goto testit
	//1 Jan 2010
	//Jan 2010
	//2 - 20 Jan 2010
	//Jan - Feb 2010
	//15 Feb - 20 Mar 2010
	//Jan - Dec 2010
	//Feb 2010 - Jan 2011
	//10 Mar 2010 - 5 Feb 2010

	#include <general_common.h>
	if (lang.unassigned()) {
		lang = gen.glang;
	}

	var t2 = d2.oconv("D/E");
	var tt = d1.oconv("D/E") ^ "/" ^ t2;
	tt.converter("/", FM);

	tt.r(1, tt.a(1) + 0);
	tt.r(4, tt.a(4) + 0);

	if (tt.a(3) == tt.a(6)) {
		//dont show both years if same
		tt.r(3, "");
		if (tt.a(2) == tt.a(5)) {
			//dont show both months if same year and month
			tt.r(2, "");
		}
	}

	//dont show start and end day of month if complete calendar months
	if (tt.a(1) == 1) {
		//calculate end of month of stop date
		t2 = iconv(t2.oconv("D2/E").field("/", 2, 2), "[DATEPERIOD]");
		if (tt.a(4) == t2.oconv("D2/E").field("/", 1)) {
			tt.r(1, "");
			tt.r(4, "");
		}
	}

	//monthnames
	if (tt.a(2)) {
		tt.r(2, lang.a(2).field("|", tt.a(2)));
	}
	if (tt.a(5)) {
		tt.r(5, lang.a(2).field("|", tt.a(5)));
	}

	//eliminate first dom if same dom
	if (((tt.a(1) == tt.a(4)) and not(tt.a(2))) and not(tt.a(3))) {
		tt.r(1, "");
	}

	//add hyphen if still a range
	if (not(tt.substr(1,3) == (FM ^ FM ^ FM))) {
		if ((tt.a(2) == "") and (tt.a(3) == "")) {
			tt.r(3, tt.a(3) ^ " ^ ");
		}else{
			tt.r(3, tt.a(3) ^ " - ");
		}
	}

	tt.converter(FM, " ");
	result = tt.trim();
	result.swapper(" ^ ", "-");

	return 0;

	/*;
	testit:
			@sentence='';
			dd='1/1/2010 1/1/2010';gosub test;
			dd='2/1/2010 2/1/2010';gosub test;
			dd='1/1/2010 31/1/2010';gosub test;
			dd='2/1/2010 31/1/2010';gosub test;

			dd='1/1/2010 28/2/2010';gosub test;
			dd='15/1/2010 21/1/2010';gosub test;
			dd='15/2/2010 21/3/2010';gosub test;

			dd='1/1/2010 31/12/2010';gosub test;

			//multi-year
			dd='1/2/2010 31/1/2011';gosub test;
			dd='1/1/2010 31/12/2011';gosub test;
			dd='10/3/2010 5/2/2011';gosub test;

			return 0;

	test:
			d1=iconv(field(dd,' ',1),'D2/E');
			d2=iconv(field(dd,' ',2),'D2/E');

			call daterangetext(d1,d2,result,lang);
			call msg(dd:fm:result);
			return 0;
	*/

}

libraryexit()
