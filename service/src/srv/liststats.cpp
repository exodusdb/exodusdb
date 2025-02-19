#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <crosstab.h>

#include <exodus/printtx.hpp>

#include <service_common.h>

#include <srv_common.h>

var msg;
//var html;
var reqdate;
var vn;
var output;
var usercoln;  // num
var tx;
var userx;

//var printptr;	// num
//var topmargin;	// num
//var printfilename;
var ownprintfile;  // num
var ptx_filenamelen;
var ptx_random;
//var printfile;
//var letterhead;
//var pagelns;
//var bodyln;		// num
//var realpagen;	// num
//var pagen;		// num
//var newpage;	// num
//var bottomline;
//var printtxmark;
//var rfmt;
//var head;
//var foot;
var ntxlns;	   // num
//var nbodylns;  // num
var ptx_temp;
//var headx;
//var newpagetag;
var ptx_css;
//var cssver;
//var style;
var stylennx;  // num
//var htmltitle;
//var head_or_foot;
var footx;
var head1;
var optioncharn;  // num
var optionchars;
var optionchar;
var newoptions;
var printtx_ii;	 // num
var spaceoptionsize;

function main() {

	// for printtx

	if (not authorised("STATISTICS ACCESS", msg, "LS")) {
		call mssg(msg);
		stop();
	}

	var rowfields = PSEUDO.f(1);
	let colfield  = PSEUDO.f(2);
	let format	  = PSEUDO.f(3);
	let datemode  = PSEUDO.f(4);

	html				= format == 1;
	var			  dedup = html;

	if (datemode > 1) {
		var selectcmd = "SELECT STATISTICS WITH DATE ";
		if (datemode == 2) {
			reqdate = date();
		} else {
			reqdate = date() - 1;
		}

		selectcmd ^= reqdate.oconv("D4").quote();
		perform(selectcmd ^ " (S)");
		if (not LISTACTIVE) {
			call mssg("No records found");
			stop();
		}
	}

	// usercodename=',DEPARTMENT,USER_NAME'
	let usercodename = ",USER_CODE";

	let filename = "STATISTICS";
	rowfields.converter(SM, VM);

	// remove col field from row fields because not logical or useful
	if (rowfields.locate(colfield, vn)) {
		rowfields.remover(1, vn);
	}

	var	 datafield	= "REQUESTS";
	let	 nrowfields = rowfields.fcount(VM);
	call crosstab(filename, rowfields, colfield, datafield, output);

	let nrows = output.fcount(FM);
	let ncols = output.f(1).fcount(VM);

	// de-duplicate
	if (dedup) {
		var row1 = output.f(nrows);
		//for (var rown = nrows; rown >= 3; --rown) {
		for (let rown : reverse_range(3, nrows)) {
			var row2	 = row1;
			row1		 = output.f(rown - 1);
			var replaced = 0;
			for (const var coln : range(1, nrowfields)) {
				// /BREAK;
				if (row1.f(1, coln) != row2.f(1, coln))
					break;
				replaced	  = 1;
				row2(1, coln) = "-";
			}  // coln;
			if (replaced) {
				output(rown) = row2;
			}
		}  // rown;
	}

	if (not rowfields.locate("USER_CODE", usercoln)) {
		usercoln = 0;
	}

	if (html) {
		let tr	 = "<TR valign=top>";
		let trx	 = "</TR>" _EOL;
		let th	 = "<TH>";
		let thx	 = "</TH>" _EOL;
		let td	 = "<TD>";
		let tdx	 = "</TD>" _EOL;
		let nbsp = "&nbsp;";

		tx = "<H1 align=center>EXODUS USAGE STATISTICS</H1>";
		gosub printtx();

		tx = "<table border=1 class=exodustable";
		tx ^= " CELLSPACING=0 CELLPADDING=2 ALIGN=CENTER";
		tx ^= " STYLE=\"font-size:66%\"";
		tx ^= ">";

		tx ^= "<thead>";

		for (const var coln : range(1, ncols)) {
			tx ^= "<col";
			if (coln > nrowfields) {
				tx ^= " align=right";
			}
			tx ^= ">";
			// extra col for user browser
			if (coln == usercoln) {
				tx ^= "<col>";
			}
		}  // coln;

		tx ^= tr;
		for (const var coln : range(1, ncols)) {
			var cell = output.f(1, coln);
			if (not cell.len()) {
				cell = nbsp;
			}
			tx ^= th ^ cell ^ thx;
			// extra col for user browser
			if (coln == usercoln) {
				tx ^= th ^ "Browser Last Seen" ^ thx;
			}
		}  // coln;
		tx ^= trx;

		tx ^= "</thead>";

		for (const var rown : range(2, nrows)) {
			let row	  = output.f(rown);
			var rowtx = "";
			for (const var coln : range(1, ncols)) {
				var cell = row.f(1, coln);
				if (dedup and coln <= nrowfields) {
					if (cell != "-") {
						var rowspan = 1;
						for (const var rown2 : range(rown + 1, nrows)) {
							// /BREAK;
							if (output.f(rown2, coln) != "-")
								break;
							rowspan += 1;
						}  // rown2;

						var celltd = "<TD";
						if (rowspan > 1) {
							celltd ^= " rowspan=" ^ rowspan;
						}
						celltd ^= ">";

						if (not cell.len()) {
							cell = nbsp;
						}

						// convert to user name
						if (coln == usercoln) {
							userx  = xlate("USERS", cell, "", "X");
							let tt = userx.f(1);
							if (tt and cell != tt) {
								cell = tt ^ " (" ^ cell ^ ")";
							}
						}

						rowtx ^= celltd ^ cell ^ tdx;

						// browser after user
						if (coln == usercoln) {
							rowtx ^= celltd;
							var agent = userx.f(39, 6);

							call htmllib2("OCONV.AGENT", agent);

							rowtx ^= agent;
							rowtx ^= tdx;
						}
					}
				} else {
					if (not cell.len()) {
						cell = nbsp;
					}
					rowtx ^= td ^ cell ^ tdx;
					if (coln == usercoln) {
						rowtx ^= td ^ nbsp ^ tdx;
					}
				}
			}  // coln;
			tx ^= tr ^ rowtx ^ trx;
			if (tx.len() > 64000) {
				gosub printtx();
			}
		}  // rown;
		tx ^= "</table>" _EOL;

		gosub printtx();

	} else {

		SYSTEM(3) = 1;
		var sys2  = SYSTEM.f(2);
		sys2.paster(-3, 3, "xls");
		SYSTEM(2) = sys2;
		output.replacer(FM, _EOL);
		output.replacer(VM, "\t");
		//call oswrite(output, SYSTEM.f(2));
		if (not oswrite(output, SYSTEM.f(2))) {
			abort(lasterror());
		}
	}

	stop();
}

libraryexit()
