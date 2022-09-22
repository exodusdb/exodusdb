#include <exodus/library.h>
libraryinit()

#include <gethtml.h>
#include <getmark.h>
#include <getcss.h>
#include <sysmsg.h>

//settable by modes
var html;
var letterhead;
var bottomline;
var printtxmark;
var rfmt;
var head;
var foot;
var style;
var topmargin;//num

//internal utilised between calls something like closure :-)
//not touchable by caller
var newpage;//num
var nbodylns;//num
var pagelns;//num
var temp;
var headx;
var newpagetag;
var css;
var printptr;//num
var realpagen;//num
var printfilename;
var printfile;
var bodyln;
var pagen;

//arg2 mode can be commands or settings with arg3 the new value
//commands: init/pagebreak/getprintfilename
//settings: html/style/head/foot/bottomline/rfmt/printtxmark/letterhead/topmargin

function main(io tx, in mode="", in modevalue="") {

	//blank mode prints tx, other modes set parameters
	if (mode) {

		//clear existing output
		if (mode eq "init") {
			printptr=0;
			if (printfilename.assigned() && printfilename) {
				printfilename.osremove();
			}

		//trigger new page on next print
		} else if (mode eq "pagebreak") {
			bodyln = 999;
			pagen = 0;

		} else if (mode eq "getprintfilename") {
			return printfilename;

		//parameters
		} else if (mode eq "html") {
			html = modevalue;
		} else if (mode eq "style") {
			style = modevalue;
		} else if (mode eq "head") {
			head = modevalue;
		} else if (mode eq "foot") {
			foot = modevalue;
		} else if (mode eq "bottomline") {
			bottomline = modevalue;
		} else if (mode eq "rfmt") {
			rfmt = modevalue;
		} else if (mode eq "printtxmark") {
			printtxmark = modevalue;
		} else if (mode eq "letterhead") {
			letterhead = modevalue;
		} else if (mode eq "topmargin") {
			topmargin = modevalue;
		} else {
			var msg=mode.quote() ^ " invalid mode in printtx";
			call sysmsg(msg);
			call mssg(msg);
		}
		return 1;
	}

//printtx:

	if (printptr.unassigned()) {
		printptr = 0;
	}
	if (printptr == 0) {

		if (topmargin.unassigned()) {
			topmargin = 8;
		}
		if (tx.unassigned()) {
			tx = "";
		}

		//call setptr('prn':char(0),1)
		SYSTEM(3) = 1;

		//if no printfile assume command mode and make an output file name
		printfilename = SYSTEM.f(2);
		if (html.unassigned()) {
			html = printfilename.lcase().contains(".htm");
		}
		var ownprintfile = 0;
		//if (printfilename == "") {
		//	printfilename = var("9999999999").rnd().last(8);
		//	printfilename = printfilename ^ (html ? ".htm" : ".5~txt");
		//	SYSTEM(2) = printfilename;
		//	ownprintfile = 1;
		//}

		//change the file extension to HTM
		if (html and printfilename and printfilename.last(4).lcase() ne ".htm") {

			printfilename.osclose();
			printfilename.osremove();

			//create a new filename
			var tt = (field2(printfilename, "\\", -1)).len();
			var t2 = (var(10).pwr(15)).rnd().first(8);
			printfilename.paster(-tt, tt, t2 ^ ".htm");

			SYSTEM(2) = printfilename;
		}

		//open printout file
		if (printfilename) {
			call oswrite("", printfilename);
			if (not printfile.osopen(printfilename)) {
				call mssg("SYSTEM ERROR - PRINTTX CANNOT OPEN PRINTFILE " ^ (DQ ^ (printfilename ^ DQ)));
				return 0;
			}
		}

		if (letterhead.unassigned()) {
			if (html) {
				var mode = "HEAD";
				call gethtml(mode, letterhead, "");
			}else{
				letterhead = "";
			}
		}

		pagelns = LPTRHIGH - 1;
		bodyln = 0;
		realpagen = 0;
		pagen = 0;
		newpage = 0;

		if (bottomline.unassigned()) {
			bottomline = "";
			if (html) {
				bottomline ^= "</tbody></table>";
			}
			call getmark("OWN", html, printtxmark);
			bottomline ^= printtxmark;
		}

		if (rfmt.unassigned()) {
			rfmt = "R#69";
		}
		if (head.unassigned()) {
			head = "";
		}
		if (foot.unassigned()) {
			foot = "";
		}

		gosub getheadfoot();

	}

	//if tx[-1,1] ne char(12) then tx:=fm
	tx ^= FM;

	var ntxlns = tx.count(FM);
	bodyln += ntxlns;

	if (bodyln == ntxlns or bodyln > nbodylns or newpage) {

		if (html and not newpage and pagen > 0 and bodyln < 999999) {
			goto printtx2;
		}

		gosub getheadfoot();

		realpagen += 1;
		pagen += 1;

		//get.cursor(cursor)
		//print @AW<30>:@(36,@CRTHIGH/2):'Page':' ':realpagen:'.':
		//put.cursor(cursor)

		temp = FM.str(topmargin) ^ letterhead ^ headx;
		if (html) {
			if (printptr ne 0) {
				if (newpagetag.unassigned()) {
					newpagetag = "<div style=\"page-break-before:always\">";
				}
				temp.paster(1, 0, newpagetag ^ "&nbsp;</div>");
			}
		}

		//swap '<h2>' with '<h2~style="page-break-before:auto">' in head

		tx.paster(1, 0, temp ^ FM);
		if (printptr and not html) {
			tx.paster(1, 0, FM ^ chr(12));
		}
	}

	if (bodyln > nbodylns or newpage) {

		var tt = html ? "<p>" : "";

		//removed so always footing
		//if bodyln<999 then

		//commented so foot always follows bottom line (printtxmark)
		//if foot then
		tx.paster(1, 0, foot);
		//end else
		if (bottomline) {
			if (html) {
				tt ^= "<p style=\"text-align:center\">";
			}
			//tx[1,0]=bottomline:fm:t:'continues ...' rfmt
			tx.paster(1, 0, bottomline);
		}
		// end
		// end
		if (html and not bottomline.contains("</tbody></table>")) {
			tx.paster(1, 0, "</tbody></table>");
		}
		bodyln = ntxlns;
	}

//alternative entry point not implemented atm only required in one bp program
printtx2:
	if (html) {
		if (not printptr) {

			call getcss(css);

			if (style.unassigned()) {
				style = "";
			}
			if (style) {
				css.replacer("</style>", style ^ "</style>");
			}
			//htmltitle=field(headx<1>,"'",1)
			var htmltitle = headx.f(1);
			if (htmltitle.starts("<")) {
				htmltitle = htmltitle.field(">", 2).field("<", 1);
			}
			tx.paster(1, 0, "<html><head>" ^ FM ^ "<meta charset=\"utf-8\" /> " ^ FM ^ "<title>" ^ htmltitle ^ "</title>" ^ FM ^ css ^ "</head><body style=\"background-color:#ffffff\"><div align=\"center\" style=\"text-align:center\">\n");
			tx.paster(1, 0, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
			css = "";
		}
	}
	tx.replacer(FM, "\r\n");
	var result=1;
	if (printfilename) {
		if (not osbwrite(tx, printfile, printptr)) {
			call mssg("printtx: osbwritefailed to write tx on "^printfile^" "^printptr);
			result = 0;
		}
	} else {
		tx.outputl();
		printptr+=tx.len();
	}

	tx = "";
	newpage = 0;

	return result;

}

subroutine convoptions() {

	temp.replacer("\'\'", "^%^");

	//convert options
	//loop while index(temp,"L'",1) do swap "L'" with "'":fm in temp repeat
	//swap "'P'" with pagen in temp
	//swap "'T'" with timedate2() in temp
	//swap "''" with "" in temp
	while (true) {
		var optioncharn = temp.index("\'");
		if (not optioncharn) {
			break;
		}
		var optionchars = "";
		while (true) {
			optioncharn += 1;
			var optionchar = temp[optioncharn];
			if (not(optionchar ne "" and optionchar ne "\'")) {
				break;
			}
			optionchars ^= optionchar;
		}
		var newoptions = "";
		//space to defeat convsyntax
		for ( var ii = 1; ii <= optionchars.len(); ii++) {
		//for (const var ii : range(1, optionchars.len())) {
		//while (var ii = 0; ++ii <= optionchars.len()) {
			var optionchar = optionchars[ii];
			if (optionchar == "T") {
//				newoptions ^= timedate();
				newoptions ^= oconv(time(), "MTS") ^ " " ^ oconv(date(), "D");
			} else if (optionchar == "D") {
				newoptions ^= oconv(date(),"[DATE,*]");
			} else if (optionchar == "P") {
				newoptions ^= pagen + 1;
			} else if (optionchar == "L") {
				newoptions ^= FM;
				//if html then newoptions:='<p>'
				if (html) {
					newoptions ^= "<br />";
				}
			} else if (optionchar == "S") {
				var spaceoptionsize = "";
				while (optionchars[ii + 1] ne "" and optionchars[ii + 1].isnum()) {
					spaceoptionsize ^= optionchars[ii + 1];
					ii += 1;
				}
				newoptions ^= spaceoptionsize.space();
			}
		}//ii;
		//temp[optioncharn,-len(optionchars)-2]=newoptions
		temp.paster(optioncharn - optionchars.len() - 1, optionchars.len() + 2, newoptions);
	}

	temp.replacer("^%^", "\'");

	return;

}

subroutine getheadfoot() {
	headx = head;
	if (headx.contains("\'")) {
		temp = headx;
		gosub convoptions();
		headx = temp;
		headx.converter("`", "`");
	}
	var footx = foot;
	if (footx.contains("\'")) {
		temp = footx;
		gosub convoptions();
		footx = temp;
		footx.converter("`", "`");
	}

	if (html) {
		var head1 = headx.f(1);
		head1.replacer(" ", "&nbsp;");
		head1.converter("~", " ");
		while (true) {
		///BREAK;
		if (not(head1.ends("&nbsp;"))) break;
			head1.cutter(-6);
		}//loop;
		headx(1) = head1;
		head1 = "";
	}

	//nheadlns=topmargin+count(headx,fm)+(headx<>'')
	//nfootlns=count(footx,fm)+(footx<>'')
	//nbodylns=pagelns-nheadlns-nfootlns
	//cannot work out header lines anymore from html eg nlist includes fm in colheads
	nbodylns = 999999;

	return;

}


libraryexit()
