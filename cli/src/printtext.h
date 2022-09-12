#include <exodus/exodus.h>

class printtext {

	var printptr;
	//var tx;
	var topmargin;
	var ownprintfile;
	var printfilename;
	var letterhead;
	var pagelns;
	var bodyln;
	var realpagen;
	var pagen;
	var newpage;
	var bottomline;
	var printtxmark;
	var rfmt;
	var headx;
	var footx;
	var head;
	var foot;
	var nbodylns;
	var newpagetag;
	var style;

   public:
	var html;

	void init() {}

	void printnext(io bodyln, io pagen) {
		bodyln = 999;
		pagen = 0;
		return;
	}

	void setheadfoot(in newhead, in newfoot) {
		head = newhead;
		foot = newfoot;
	}

	void close() {
		printfilename.osclose();
	}

	void printtx(MvEnvironment& mv, io tx) {
		//print(tx);
		if (printptr.unassigned())
			printptr = 0;

		if (printptr == 0) {

			if (topmargin.unassigned())
				topmargin = 0;

			if (tx.unassigned())
				tx = "";

			//call setptr('prn':char(0),1)
			SYSTEM.r(3, 1);

			//if no printfilename assume command mode and make an output file name
			printfilename = SYSTEM.f(2);
			if (html.unassigned()) {
				html = printfilename.substr(-4, 4) eq ".htm";
			}

			var ownprintfile = 0;
			if (printfilename == "") {
				printfilename = (var(999999999).rnd()).substr(-8, 8);
				if (html)
					printfilename ^= ".htm";
				else
					printfilename ^= ".txt";
				SYSTEM.r(2, printfilename);
				ownprintfile = 1;
			}

			//change the file extension to htm
			if (html and printfilename.substr(-4, 4) not_eq ".htm") {

				printfilename.osclose();
				printfilename.osremove();

				//create a new filename
				var tt = (field2(printfilename, "\\", -1)).length();
				var t2 = ((var(10).pwr(15)).rnd()).substr(1, 8);
				//var t2 = ((var(pow(10,15)).rnd()).substr(1, 8);
				printfilename.splicer(-tt, tt, t2 ^ ".htm");

				SYSTEM.r(2, printfilename);
			}

			if (letterhead.unassigned()) {
				if (html)
					gosub gethtml("HEAD", letterhead, "");
				else
					letterhead = "";
			}

			//pagelns = mv.LPTRHIGH - 1;
			bodyln = 0;
			realpagen = 0;
			pagen = 0;
			newpage = 0;

			if (bottomline.unassigned()) {
				bottomline = "";
				if (html)
					bottomline ^= "</tbody></table>";
				gosub getmark("OWN", html, printtxmark);
				bottomline ^= printtxmark;
			}

			if (rfmt.unassigned())
				rfmt = "R#69";
			if (head.unassigned())
				head = "";
			if (foot.unassigned())
				foot = "";

			gosub getheadfoot(head, foot, nbodylns);
		}

		//if tx[-1,1] ne char(12) then tx:=fm
		//tx ^= FM;

		var ntxlns = tx.count(FM) + 1;
		bodyln += ntxlns;

		if (bodyln == ntxlns or bodyln > nbodylns or newpage) {

			//skip page break in some circumstance
			if (html and not newpage and pagen > 0 and bodyln < 999999) {
				printtx2(tx);
				return;
			}

			gosub getheadfoot(head, foot, nbodylns);

			realpagen += 1;
			pagen += 1;

			//var().getcursor();
			//cout << _AW.f(30)<< var().cursor(36, _CRTHIGH / 2)<< "Page"<< " "<< realpagen<< ".";
			//cursor.setcursor();

			//insert page break except on first page
			var temp = FM.str(topmargin) ^ letterhead ^ headx;
			if (html) {
				if (printptr not_eq 0) {
					if (newpagetag.unassigned())
						newpagetag = "<div style=\"page-break-before:always\">";
					temp.splicer(1, 0, newpagetag ^ "&nbsp;</div>");
				}
			}

			//swap '<h2>' with '<h2~style="page-break-before:auto">' in head

			//insert page heading
			tx.splicer(1, 0, temp ^ FM);

			//insert text style page break
			if (printptr and not html)
				tx.splicer(1, 0, FM ^ var().chr(12));
		}

		if (bodyln > nbodylns or newpage) {
			var tt = html ? "<p>" : "";
			if (bodyln < 999) {
				if (foot) {
					tx.splicer(1, 0, foot);
				} else {
					if (html)
						tt ^= "<p style=\"text-align:center\">";
					//tx[1,0]=bottomline:fm:t:'continues ...' rfmt
					tx.splicer(1, 0, bottomline);
				}
			}
			if (html and not bottomline.index("</tbody></table>", 1))
				tx.splicer(1, 0, "</tbody></table>");
			bodyln = ntxlns;
		}
		printtx2(tx);
		return;
	}

   private:
	void printtx2(io tx) {
		if (html) {
			if (not printptr) {

				var css;
				gosub readcss(css);

				if (style.unassigned())
					style = "";
				if (style)
					css.swapper("</style>", style ^ "</style>");
				var htmltitle = (headx.f(1)).field("\'", 1, 1);
				if (htmltitle[1] == "<")
					htmltitle = (htmltitle.field(">", 2, 1)).field("<", 1, 1);
				tx.splicer(1, 0, "\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n<title>" ^ htmltitle ^ "</title>" ^ FM ^ css ^ "</head>\n<body style=\"background-color:#ffffff\">\n<div style=\"text-align:center\">" ^ (var().chr(13) ^ var().chr(10)));
				tx.splicer(1, 0, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
				css = "";
			}
			//cannot remove these since they may be nonibm codepage letters
			//convert 'Í' to '=-' in tx
		}
		tx.swapper(FM, "\n");
		//osbwritex(tx, printfilename, printfilename, printptr);
		//tx.osbwrite(printfilename,printptr);
		printptr = 99;
		printl(tx);

		tx = "";
		newpage = 0;

		return;
	}

	void gethtml(in mode, io letterhead, in companycode) {
		letterhead = "";
		return;

		//prevent "warning unused" until properly implemented
		if (mode or companycode) {
		};
	}

	void readcss(io css) {
		if (osread(css, "default.css"))
			return;
		css =
			""
			"<style type=\"text/css\">\n"
			"body {background-color:#ffffff; font-family:neosysfont,trebuchet ms,arial,helvetica,sans serif; font-size: neosyssize}\n"
			"table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}\n"
			"a {color:blue;}\n"
			"a:visited {color:purple;}\n"
			"a:hover {color:red;}\n"
			"tt {font-family:courier new,courier;font-size:8px;}\n"
			"th {background-color:#ffff80;}\n"
			"thead {display:table-header-group}\n"
			"td.nb {border-bottom:none}\n"
			"td.nt {border-top:none}\n"
			"td.nx {border-top:none;border-bottom:none}\n"
			".maintable {background-color:#ffffc0; border-width:2px; border-collapse:separate; padding:1px}\n"
			"</style>\n"
			"\n"
			"<style type=\"text/css\" media=\"print\">\n"
			".maintable {border-collapse:collapse;border-color:#bbbbbb}\n"
			"td {border-color:#bbbbbb;border-width:1px}\n"
			"th {border-color:#bbbbbb;border-width:1px}\n"
			"</style>\n";
	}

	void getmark(in mode, in html, io printtxmark) {
		if (mode or html)
			printtxmark = "";
		else
			printtxmark = "";
		return;
	}

	void convoptions(io temp) {
		temp.swapper(SQ ^ SQ, "^%^");

		//convert options
		//loop while index(temp,"L'",1) do swap "L'" with "'":fm in temp repeat
		//swap "'P'" with pagen in temp
		//swap "'T'" with timedate2() in temp
		//swap "''" with "" in temp
		var optioncharn;
		while ((optioncharn = temp.index("\'", 1))) {

			var optionchars = "";
			while (true) {
				optioncharn += 1;
				var optionchar = temp.substr(optioncharn, 1);
				//BREAK
				if (!(optionchar not_eq "" and optionchar not_eq "\'"))
					break;
				optionchars ^= optionchar;
			}

			var newoptions = "";
			for (var ii = 1; ii <= optionchars.length(); ii++) {
				var optionchar = optionchars.substr(ii, 1);
				if (optionchar == "T") {
					newoptions ^= timedate();

				} else if (optionchar == "D") {
					newoptions ^= (var().date()).oconv("[DATE,*]");

				} else if (optionchar == "P") {
					newoptions ^= pagen + 1;

				} else if (optionchar == "L") {
					newoptions ^= FM;
					//if html then newoptions:='<p>'
					if (html)
						newoptions ^= "<br />";
				} else if (optionchar == "S") {
					var spaceoptionsize = "";
					while (optionchars.substr(ii + 1, 1).match("\\d", "r")) {
						spaceoptionsize ^= optionchars.substr(ii + 1, 1);
						ii += 1;
					}
					newoptions ^= spaceoptionsize.space();
				}
			};	//ii;
			//temp[optioncharn,-len(optionchars)-2]=newoptions
			temp.splicer(optioncharn - optionchars.length() - 1, optionchars.length() + 2, newoptions);
		}

		temp.swapper("^%^", "\'");

		return;
	}

	void getheadfoot(io head, io foot, io nbodylns) {

		headx = head;
		if (headx.contains("\'", 1)) {
			convoptions(headx);
			headx.converter("`", "`");
		}

		footx = foot;
		if (footx.contains("\'", 1)) {
			convoptions(footx);
			footx.converter("`", "`");
		}

		if (html) {
			var head1 = headx.f(1);
			head1.swapper(" ", "&nbsp;");
			head1.converter("~", " ");
			while (head1.substr(-6, 6) == "&nbsp;") {
				head1.splicer(-6, 6, "");
			}
			headx.r(1, head1);
			head1 = "";
		}

		//nheadlns=topmargin+count(headx,fm)+(headx<>'')
		//nfootlns=count(footx,fm)+(footx<>'')
		//nbodylns=pagelns-nheadlns-nfootlns
		//cannot work out header lines anymore from html eg nlist includes fm in colheads
		nbodylns = 999999;

		return;
	}

} printer1;
