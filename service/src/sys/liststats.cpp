#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <crosstab.h>
#include <printtx.hpp>
#include <htmllib2.h>
#include <gethtml.h>
#include <getmark.h>
#include <getcss.h>
#include <docmods.h>
#include <timedate2.h>

#include <gen_common.h>

var msg;
var html;
var reqdate;
var vn;
var output;
var usercoln;//num
var tx;
var userx;
var printptr;//num
var topmargin;//num
var printfilename;
var ownprintfile;//num
var ptx_filenamelen;
var ptx_random;
var printfile;
var letterhead;
var pagelns;
var bodyln;//num
var realpagen;//num
var pagen;//num
var newpage;//num
var bottomline;
var printtxmark;
var rfmt;
var head;
var foot;
var ntxlns;//num
var nbodylns;//num
var ptx_temp;
var headx;
var newpagetag;
var ptx_css;
var cssver;
var style;
var stylennx;//num
var htmltitle;
var head_or_foot;
var footx;
var head1;
var optioncharn;//num
var optionchars;
var optionchar;
var newoptions;
var printtx_ii;//num
var spaceoptionsize;

function main() {
	//c sys

	//for printtx
	//global html,head,foot,cssver,htmltitle,topmargin,bottomline,tx

	#include <system_common.h>
	var interactive = not(SYSTEM.a(33));

	if (not(authorised("STATISTICS ACCESS", msg, "LS"))) {
		call mssg(msg);
		stop();
	}

	var rowfields = PSEUDO.a(1);
	var colfield = PSEUDO.a(2);
	var format = PSEUDO.a(3);
	var datemode = PSEUDO.a(4);

	html = format eq 1;
	var dedup = html;

	if (datemode gt 1) {
		var selectcmd = "SELECT STATISTICS WITH DATE ";
		if (datemode eq 2) {
			reqdate = var().date();
		} else {
			reqdate = var().date() - 1;
		}

		selectcmd ^= reqdate.oconv("D4").quote();
		perform(selectcmd ^ " (S)");
		if (not LISTACTIVE) {
			call mssg("No records found");
			stop();
		}

	}

	//usercodename=',DEPARTMENT,USER_NAME'
	var usercodename = ",USER_CODE";

	var filename = "STATISTICS";
	rowfields.converter(SVM, VM);

	//remove col field from row fields because not logical or useful
	if (rowfields.locate(colfield, vn)) {
		rowfields.remover(1, vn);
	}

	var datafield = "REQUESTS";
	var nrowfields = rowfields.count(VM) + 1;
	call crosstab(filename, rowfields, colfield, datafield, output);

	var nrows = output.count(FM) + 1;
	var ncols = output.a(1).count(VM) + 1;

	//de-duplicate
	if (dedup) {
		var row1 = output.a(nrows);
		for (var rown = nrows; rown >= 3; --rown) {
			var row2 = row1;
			row1 = output.a(rown - 1);
			var replaced = 0;
			for (var coln = 1; coln <= nrowfields; ++coln) {
				///BREAK;
				if (not(row1.a(1, coln) eq row2.a(1, coln))) break;
				replaced = 1;
				row2.r(1, coln, "-");
			} //coln;
			if (replaced) {
				output.r(rown, row2);
			}
		} //rown;
	}

	if (not(rowfields.locate("USER_CODE", usercoln))) {
		usercoln = 0;
	}

	if (html) {
		var tr = "<TR valign=top>";
		var trx = "</TR>" "\r\n";
		var th = "<TH>";
		var thx = "</TH>" "\r\n";
		var td = "<TD>";
		var tdx = "</TD>" "\r\n";
		var nbsp = "&nbsp;";

		tx = "<H1 align=center>EXODUS USAGE STATISTICS</H1>";
		gosub printtx();

		tx = "<table border=1 class=exodustable";
		tx ^= " CELLSPACING=0 CELLPADDING=2 ALIGN=CENTER";
		tx ^= " STYLE=\"font-size:66%\"";
		tx ^= ">";

		tx ^= "<thead>";

		for (var coln = 1; coln <= ncols; ++coln) {
			tx ^= "<col";
			if (coln gt nrowfields) {
				tx ^= " align=right";
			}
			tx ^= ">";
			//extra col for user browser
			if (coln eq usercoln) {
				tx ^= "<col>";
			}
		} //coln;

		tx ^= tr;
		for (var coln = 1; coln <= ncols; ++coln) {
			var cell = output.a(1, coln);
			if (not(cell.length())) {
				cell = nbsp;
			}
			tx ^= th ^ cell ^ thx;
			//extra col for user browser
			if (coln eq usercoln) {
				tx ^= th ^ "Browser Last Seen" ^ thx;
			}
		} //coln;
		tx ^= trx;

		tx ^= "</thead>";

		for (var rown = 2; rown <= nrows; ++rown) {
			var row = output.a(rown);
			var rowtx = "";
			for (var coln = 1; coln <= ncols; ++coln) {
				var cell = row.a(1, coln);
				if (dedup and coln le nrowfields) {
					if (cell ne "-") {
						var rowspan = 1;
						for (var rown2 = rown + 1; rown2 <= nrows; ++rown2) {
							///BREAK;
							if (not(output.a(rown2, coln) eq "-")) break;
							rowspan += 1;
						} //rown2;

						var celltd = "<TD";
						if (rowspan gt 1) {
							celltd ^= " rowspan=" ^ rowspan;
						}
						celltd ^= ">";

						if (not(cell.length())) {
							cell = nbsp;
						}

						//convert to user name
						if (coln eq usercoln) {
							userx = xlate("USERS", cell, "", "X");
							var tt = userx.a(1);
							if (tt and cell ne tt) {
								cell = tt ^ " (" ^ cell ^ ")";
							}
						}

						rowtx ^= celltd ^ cell ^ tdx;

						//browser after user
						if (coln eq usercoln) {
							rowtx ^= celltd;
							var agent = userx.a(39, 6);

							call htmllib2("OCONV.AGENT", agent);

							rowtx ^= agent;
							rowtx ^= tdx;
						}

					}
				} else {
					if (not(cell.length())) {
						cell = nbsp;
					}
					rowtx ^= td ^ cell ^ tdx;
					if (coln eq usercoln) {
						rowtx ^= td ^ nbsp ^ tdx;
					}
				}
			} //coln;
			tx ^= tr ^ rowtx ^ trx;
			if (tx.length() gt 64000) {
				gosub printtx();
			}
		} //rown;
		tx ^= "</table>" "\r\n";

		gosub printtx();

	} else {

		SYSTEM.r(3, 1);
		var sys2 = SYSTEM.a(2);
		sys2.splicer(-3, 3, "xls");
		SYSTEM.r(2, sys2);
		output.swapper(FM, "\r\n");
		output.swapper(VM, "\t");
		call oswrite(output, SYSTEM.a(2));

	}

	stop();

	return 0;
}

libraryexit()
