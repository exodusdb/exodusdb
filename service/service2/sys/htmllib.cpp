#include <exodus/library.h>
libraryinit()

#include <gen.h>

subroutine main(in mode, io datax, in params="", in params20="") {

	var params2=params20;

	if (mode == "TABLE.MAKE") {
		datax.trimmer();
		var tt = " " ^ VM;
		while (true) {
		///BREAK;
		if (not(datax.index(tt, 1))) break;;
			datax.swapper(tt, VM);
		}//loop;
		tt = " " ^ FM;
		while (true) {
		///BREAK;
		if (not(datax.index(tt, 1))) break;;
			datax.swapper(tt, FM);
		}//loop;
		var filler = "&nbsp;";
		gosub fill(datax, filler);

		datax.swapper(FM, "</TD></TR>" ^ FM ^ "<TR><TD>");
		datax.swapper(VM, "</TD>" ^ VM ^ "<TD>");
		datax = "<TR><TD>" ^ datax ^ "</TD></TR>";

		//trth='<TR><TD>'
		if (params2) {
			//trth='<TH>'
			var line1 = datax.a(1);
			line1.swapper("</TR>", "</tr>");
			line1.swapper("</tr>", "</TR></THEAD><TBODY>");
			line1.swapper("<TD>", "<th>");
			line1.swapper("</TD>", "</th>");
			line1.swapper("<td>", "<th>");
			line1.swapper("</td>", "</th>");
			datax.r(1, line1);
		}

		//prefix
		//tableid='T':rnd(999999999)[-7,7]
		params2 = params;
		//if params2 else params2='<TABLE><THEAD>'
		params2.swapper("<TABLE", "<table");
		//swap '<table' with '<table id=':tableid in params2
		datax.splicer(1, 0, params2);

		//postfix
		datax ^= "</TBODY></TABLE>";
		//datax=datax:'</TBODY></TABLE ID=':tableid:'>'

		//if not(count(datax,fm)) and index(datax<1>,'<TH>',1) then
		// swap '</TD>' with '</TH>' in datax
		// end

		//highlight last row if a total
		var nlines = datax.count(FM) + 1;
		var lastline = datax.a(nlines);
	//Total
		if (lastline.index("<TD>" ^ gen.glang.a(28), 1)) {
			lastline.swapper("TD>", "TH>");
			lastline.swapper("td>", "th>");
			datax.r(nlines, lastline);
		}

		datax.converter(FM ^ VM, "");

	//fill in the missing cells
	} else if (mode == "TABLE.FILL") {
		var filler = "<TD>&nbsp;</TD>";
		gosub fill(datax, filler);

	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " unknown mode in HTMLLIB");
	}

	return;
}

subroutine fill(io datax, in filler) {
	var nn = datax.count(FM) + (datax ne "");

	//find max number of columns
	var n2 = 0;
	for (var ii = 1; ii <= nn; ++ii) {
		var tt = (datax.a(ii)).count(VM) + 1;
		if (tt > n2) {
			n2 = tt;
		}
	};//ii;

	//make sure all columns are filled
	for (var ii = 1; ii <= nn; ++ii) {
		for (var i2 = 1; i2 <= n2; ++i2) {
			if (datax.a(ii, i2) == "") {
				datax.r(ii, i2, filler);
			}
		};//i2;
	};//ii;

}

libraryexit()
