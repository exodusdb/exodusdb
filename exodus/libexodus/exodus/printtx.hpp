#ifndef EXO_PRINTTX_HPP
#define EXO_PRINTTX_HPP

#include <exodus/htmllib2.h>

//////////////////////////////////////////////////////////////
// gethtml2, sendmail and xselect will found in the ~/inc  dir
// when using printtx.hpp in service programs but will not
// be available when building cli/list.cpp library
//////////////////////////////////////////////////////////////

#if __has_include(<gethtml.h>)
#	include <gethtml.h>
#	define EXO_HAS_GETHTML
#else
//subroutine gethtml(in mode0, out html_letterhead, in compcode0="", in qr_text="") {
subroutine gethtml(in, out html_letterhead, in = "", in = "") {
	html_letterhead = "";
	return;
}
#endif

#if __has_include(<sendmail.h>)
#	include <sendmail.h>
#	define EXO_HAS_SENDMAIL
#else
//subroutine sendmail(in toaddress0, in ccaddress0, in subject0, in body0, in attachfilename0, in delete0, out errormsg, in replyto0=var(), in params0=var()) {
subroutine sendmail(in, in, in, in, in, in, out errormsg, in = var(), in = var()) {
	errormsg = "";
	return;
}
#endif

#if __has_include(<xselect.h>)
#   include <xselect.h>
#else
#   define xselect select
#endif

/////////////////////////////////////////////////////
// STUBS to pass old function calls to htmllib2 modes
/////////////////////////////////////////////////////
subroutine getcss(io css, in version = "") {
	call htmllib2("GETCSS", css, version);
	return;
}

subroutine docmods(io tx, in params = "") {
	call htmllib2("DOCMODS", tx, params);
	return;
}

subroutine getmark(in mode, in html, io mark) {
	call htmllib2("GETMARK", mark, mode, html);
}

//////////////////////////
// Main printtx() function
//////////////////////////

subroutine printtx() {


	//Initialise on first call
	//////////////////////////
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
		//if (html.unassigned()) {
		if (printfilename.lcase().ends(".htm"))
			html = 1;

		var ownprintfile = 0;
		//if (printfilename == "") {
		//	printfilename = var(999999999).rnd().last(8);
		//	//printfilename:=if html then '.htm' else '.txt'
		//	if (html) {
		//		printfilename ^= ".htm";
		//	}else{
		//		printfilename ^= ".txt";
		//	}
		//	SYSTEM(2) = printfilename;
		//	ownprintfile = 1;
		//}

		//change the file extension to HTM
		if (html and printfilename ne "-" and not printfilename.lcase().ends(".htm")) {

			printfilename.osclose();
			printfilename.osremove();

			//create a new filename
			var ptx_filenamelen = (field2(printfilename, "\\", -1)).len();
			var ptx_random = var(10).pwr(15).rnd().first(8);
			printfilename.paster(-ptx_filenamelen, ptx_filenamelen, ptx_random ^ ".htm");

			SYSTEM(2) = printfilename;
		}

		// printfilename "-" means stdout
		if (printfilename == "-")
			printfilename = "";

		//open printout file
		if (printfilename) {
			call oswrite("", printfilename);
			if (not(printfile.osopen(printfilename))) {
				call mssg("SYSTEM ERROR - CANNOT OPEN PRINTFILE " ^ (DQ ^ (printfilename ^ DQ)));
				stop();
			}
		}

		if (letterhead.unassigned()) {
			letterhead = "";
			if (html) {
				call gethtml("HEAD", letterhead, "");
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
				call getmark("OWN", html, printtxmark);
				bottomline(-1) = printtxmark;
				//line below document on screen but not on print
				//bottomline<-1>='<hr class="pagedivider noprint"/>'
			}
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

	} // end of auto initialisation

	//if tx[-1,1] ne char(12) then tx:=fm
	tx ^= FM;

	var ntxlns = tx.count(FM);
	bodyln += ntxlns;

	if (((bodyln == ntxlns) or (bodyln > nbodylns)) or newpage) {

		if (((html and not(newpage)) and (pagen > 0)) and (bodyln < 999999)) {
			gosub printtx2();
			return;
		}

		gosub getheadfoot();

		realpagen += 1;
		pagen += 1;

		var ptx_temp = FM.str(topmargin) ^ letterhead ^ headx;
		if (html) {
			if (printptr ne 0) {
				if (newpagetag.unassigned()) {
					newpagetag = "<div style=\"page-break-before:always\">";
				}
				ptx_temp.prefixer(newpagetag ^ "&nbsp;</div>");
			}
		}

		//replace '<h2>' with '<h2~style="page-break-before:auto">' in head

		tx.prefixer(ptx_temp ^ FM);
		if (printptr and not(html)) {
			tx.prefixer(FM ^ var().chr(12));
		}
	}

	///////////////////////////////////
	// Page break if nlines > page size
	///////////////////////////////////
	if ((bodyln > nbodylns) or newpage) {
		//if html then t_='<p>' else t_=''

		//removed so always footing
		//if bodyln<999 then

		// Footer always follows bottom line (printtxmark)
		tx.prefixer(foot);
		if (bottomline) {
			tx.prefixer(bottomline);
		}
		bodyln = ntxlns;
	}

	// Output a single line
	gosub printtx2();

	return;
}

///////////////////////
// Output a single line
///////////////////////

subroutine printtx2() {
	if (html) {
		if (not(printptr)) {

			var ptx_css;
			call getcss(ptx_css, cssver);

			//insert any additional style
			if (style.unassigned()) {
				style = "";
			}
			if (style) {
				var stylennx = ptx_css.index("</style>");
				ptx_css.paster(stylennx + 8, FM ^ style);
			}

			if (htmltitle.unassigned()) {
				htmltitle = headx.f(1);
				}
			if (htmltitle.starts("<")) {
				htmltitle = htmltitle.field(">", 2).field("<", 1);
			}

			tx.prefixer("<!DOCTYPE html>" ^ FM ^ "<html>" ^ FM ^ "<head>" ^ FM ^ "<meta charset=\"utf-8\" /> " ^ FM ^ "<title>" ^ htmltitle ^ "</title>" ^ FM ^ ptx_css ^ "</head><body style=\"background-color:#ffffff\"><div align=\"center\">" ^ FM);

			call docmods(tx);

			//ptx_css = "";
		}
	}

	// Convert fields to text lines
	tx.replacer(FM, EOL);

	if (printfilename)
		call osbwrite(tx, printfile,  printptr);
	else {
		tx.output();
		printptr+=tx.len();
	}

	tx = "";
	newpage = 0;

	return;

}

subroutine convoptions(io ptx_temp) {
	ptx_temp.replacer("\'\'", "^%^");

	//convert options
	//loop while index(ptx_temp,"L'",1) do replace "L'" with "'":fm in ptx_temp repeat
	//replace "'P'" with pagen in ptx_temp
	//replace "'T'" with timedate2() in ptx_temp
	//replace "''" with "" in ptx_temp
	while (true) {
		var optioncharn = ptx_temp.index("\'");

		if (not optioncharn)
			break;

		var optionchars = "";
		while (true) {

			optioncharn += 1;
			var optionchar = ptx_temp[optioncharn];

			//if (not(optionchar ne "" and optionchar ne "\'"))
			if (optionchar eq _SQ or optionchar eq "")
				break;

			optionchars ^= optionchar;
		}
		var newoptions = "";
		for (var i_ = 1; i_ <= optionchars.len(); ++i_) {
			var optionchar = optionchars[i_];
			if (optionchar == "T") {
#ifdef EXO_NOHTML
				newoptions ^= oconv(time(), "MTS") ^ " " ^ oconv(date(), "D");
#else
				newoptions ^= timedate2();
#endif
			} else if (optionchar == "D") {
				newoptions ^= oconv(var().date(), "[DATE,*]");
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
				while (true) {

					if (not optionchars[i_ + 1].match("^\\d$"))
						break;

					spaceoptionsize ^= optionchars[i_ + 1];
					i_ += 1;
				}
				newoptions ^= spaceoptionsize.space();
			}
		};//i_;
		//ptx_temp[optioncharn,-len(optionchars)-2]=newoptions
		ptx_temp.paster(optioncharn - optionchars.len() - 1, optionchars.len() + 2, newoptions);
	}//loop;

	ptx_temp.replacer("^%^", "\'");

	return;

}

subroutine getheadfoot() {
	headx = head;
	if (headx.contains("\'")) {
		var ptx_temp = headx;
		gosub convoptions(ptx_temp);
		headx = ptx_temp;
		//convert "`" to "`" in headx
	}
	var footx = foot;
	if (footx.contains("\'")) {
		var ptx_temp = footx;
		gosub convoptions(ptx_temp);
		footx = ptx_temp;
		//convert "`" to "`" in footx
	}

	if (html) {
		var head1 = headx.f(1);
		head1.replacer(" ", "&nbsp;");
		head1.converter("~", " ");
		while (head1.ends("&nbsp;")) {
			head1.cutter(-6);
		}
		headx(1) = head1;
		head1 = "";
		headx.replacer("{%20}", " ");
	}

	//nheadlns=topmargin+count(headx,fm)+(headx<>'')
	//nfootlns=count(footx,fm)+(footx<>'')
	//nbodylns=pagelns-nheadlns-nfootlns
	//cannot work out header lines anymore from html eg nlist includes fm in colheads
	nbodylns = 999999;

	return;

}

subroutine printnext() {
	bodyln = 999;
	pagen = 0;
	return;

}

#endif // EXO_PRINTTX_HPP
