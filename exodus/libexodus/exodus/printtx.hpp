subroutine printnext() {
	bodyln = 999;
	pagen = 0;
	return;

}

subroutine printtx() {

	if (printptr.unassigned()) {
		printptr = 0;
	}
	if (printptr == 0) {

		//common /system/ system
		//PRINTER OFF;

		if (topmargin.unassigned()) {
			topmargin = 8;
		}
		if (tx.unassigned()) {
			tx = "";
		}

		//call setptr('prn':char(0),1)
		SYSTEM.r(3, 1);

		//if no printfile assume command mode and make an output file name
		printfilename = SYSTEM.f(2);
		if (html.unassigned()) {
			html = printfilename.lcase().index(".htm");
		}
		var ownprintfile = 0;
		//if (printfilename == "") {
		//	printfilename = var(999999999).rnd().substr(-8,8);
		//	//printfilename:=if html then '.htm' else '.txt'
		//	if (html) {
		//		printfilename ^= ".htm";
		//	}else{
		//		printfilename ^= ".txt";
		//	}
		//	SYSTEM.r(2, printfilename);
		//	ownprintfile = 1;
		//}

		//change the file extension to HTM
		if (html and ((printfilename.substr(-4,4)).lcase() ne ".htm")) {

			printfilename.osclose();
			printfilename.osremove();

			//create a new filename
			var ptx_filenamelen = (field2(printfilename, "\\", -1)).length();
			var ptx_random = var(10).pwr(15).rnd().substr(1,8);
			printfilename.splicer(-ptx_filenamelen, ptx_filenamelen, ptx_random ^ ".htm");

			SYSTEM.r(2, printfilename);
		}

		//open printout file
		if (printfilename) {
			call oswrite("", printfilename);
			if (not(printfile.osopen(printfilename))) {
				call mssg("SYSTEM ERROR - CANNOT OPEN PRINTFILE " ^ (DQ ^ (printfilename ^ DQ)));
				stop();
			}
		}

		if (letterhead.unassigned()) {
			if (html) {
#ifndef EXO_NOHTML
				call gethtml("HEAD", letterhead, "");
#endif
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
#ifndef EXO_NOHTML
			if (html) {
				bottomline ^= "</tbody></table>";
				call getmark("OWN", html, printtxmark);
				bottomline.r(-1, printtxmark);
				//line below document on screen but not on print
				//bottomline<-1>='<hr class="pagedivider noprint"/>'
			}
#endif
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

	if (((bodyln == ntxlns) or (bodyln > nbodylns)) or newpage) {

		if (((html and not(newpage)) and (pagen > 0)) and (bodyln < 999999)) {
			gosub printtx2();
			return;
		}

		gosub getheadfoot();

		realpagen += 1;
		pagen += 1;

		//get.cursor(cursor)
		//print @AW<30>:@(36,@CRTHIGH/2):'Page':' ':realpagen:'.':
		//put.cursor(cursor)

		var ptx_temp = FM.str(topmargin) ^ letterhead ^ headx;
		if (html) {
			if (printptr ne 0) {
				if (newpagetag.unassigned()) {
					newpagetag = "<div style=\"page-break-before:always\">";
				}
				ptx_temp.splicer(1, 0, newpagetag ^ "&nbsp;</div>");
			}
		}

		//swap '<h2>' with '<h2~style="page-break-before:auto">' in head

		tx.splicer(1, 0, ptx_temp ^ FM);
		if (printptr and not(html)) {
			tx.splicer(1, 0, FM ^ var().chr(12));
		}
	}

	if ((bodyln > nbodylns) or newpage) {
		//if html then t_='<p>' else t_=''

		//removed so always footing
		//if bodyln<999 then

			//commented so foot always follows bottom line (printtxmark)
			//if foot then
		tx.splicer(1, 0, foot);
			//end else
		if (bottomline) {
					//if html then t_:='<p style="text-align:center">'
					//tx[1,0]=bottomline:fm:t_:'continues' rfmt
			tx.splicer(1, 0, bottomline);
		}
			// end
		// end
		//removed because too automatic for PRINTJOB
		//if html and not(index(bottomline,'</tbody></table>',1)) then tx[1,0]='</tbody></table>'
		bodyln = ntxlns;
	}

	gosub printtx2();
	return;
}

subroutine printtx2() {
#ifndef EXO_NOHTML
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
				ptx_css.splicer(stylennx + 8, 0, FM ^ style);
			}

			//htmltitle=field(headx<1>,"'",1)
			if (htmltitle.unassigned()) {
				htmltitle = headx.f(1);
				}
			if (htmltitle[1] == "<") {
				htmltitle = htmltitle.field(">", 2).field("<", 1);
			}

			tx.splicer(1, 0, "<!DOCTYPE html>" ^ FM ^ "<html>" ^ FM ^ "<head>" ^ FM ^ "<meta charset=\"utf-8\" /> " ^ FM ^ "<title>" ^ htmltitle ^ "</title>" ^ FM ^ ptx_css ^ "</head><body style=\"background-color:#ffffff\"><div align=\"center\">" ^ FM);

			call docmods("", tx);

			//ptx_css = "";
		}
	}
#endif
	tx.swapper(FM, "\r\n");
	if (printfilename)
		call osbwrite(tx, printfile,  printptr);
	else {
		tx.output();
		printptr+=tx.length();
	}

	tx = "";
	newpage = 0;

	return;

}

subroutine convoptions(io ptx_temp) {
	ptx_temp.swapper("\'\'", "^%^");

	//convert options
	//loop while index(ptx_temp,"L'",1) do swap "L'" with "'":fm in ptx_temp repeat
	//swap "'P'" with pagen in ptx_temp
	//swap "'T'" with timedate2() in ptx_temp
	//swap "''" with "" in ptx_temp
	while (true) {
		var optioncharn = ptx_temp.index("\'");
		///BREAK;
		if (not optioncharn) break;
		var optionchars = "";
		while (true) {
			optioncharn += 1;
			var optionchar = ptx_temp[optioncharn];
			///BREAK;
			if (not(optionchar ne "" and optionchar ne "\'")) break;
			optionchars ^= optionchar;
		}//loop;
		var newoptions = "";
		for (var i_ = 1; i_ <= optionchars.length(); ++i_) {
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
					///BREAK;
					if (not((optionchars[i_ + 1]).match("^\\d$"))) break;
					spaceoptionsize ^= optionchars[i_ + 1];
					i_ += 1;
				}//loop;
				newoptions ^= spaceoptionsize.space();
			}
		};//i_;
		//ptx_temp[optioncharn,-len(optionchars)-2]=newoptions
		ptx_temp.splicer(optioncharn - optionchars.length() - 1, optionchars.length() + 2, newoptions);
	}//loop;

	ptx_temp.swapper("^%^", "\'");

	return;

}

subroutine getheadfoot() {
	headx = head;
	if (headx.index("\'")) {
		var ptx_temp = headx;
		gosub convoptions(ptx_temp);
		headx = ptx_temp;
		//convert "`" to "`" in headx
	}
	var footx = foot;
	if (footx.index("\'")) {
		var ptx_temp = footx;
		gosub convoptions(ptx_temp);
		footx = ptx_temp;
		//convert "`" to "`" in footx
	}

	if (html) {
		var head1 = headx.f(1);
		head1.swapper(" ", "&nbsp;");
		head1.converter("~", " ");
		while (true) {
			///BREAK;
			if (not(head1.substr(-6,6) == "&nbsp;")) break;
			head1.splicer(-6, 6, "");
		}//loop;
		headx.r(1, head1);
		head1 = "";
		headx.swapper("{%20}", " ");
	}

	//nheadlns=topmargin+count(headx,fm)+(headx<>'')
	//nfootlns=count(footx,fm)+(footx<>'')
	//nbodylns=pagelns-nheadlns-nfootlns
	//cannot work out header lines anymore from html eg nlist includes fm in colheads
	nbodylns = 999999;

	return;

}
