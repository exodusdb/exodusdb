#include <exodus/library.h>
libraryinit()

var fromquestion;
var uptoquestion;
var defaultperiod;

func main(in fromquestion0, in uptoquestion0, io ifromdate, io iuptodate) {

//	if (fromquestion0.unassigned()) {
//		fromquestion = "";
//	} else {
//		fromquestion = fromquestion0;
//	}
//	if (uptoquestion0.unassigned()) {
//		uptoquestion = "";
//	} else {
//		uptoquestion = uptoquestion0;
//	}
	fromquestion = fromquestion0.or_default("");
	uptoquestion = uptoquestion0.or_default("");
	if (ifromdate.unassigned()) {
		let fromdate = "";
	}

	if (fromquestion == "") {
		fromquestion = "From date ?";
	}
	if (uptoquestion == "") {
		uptoquestion = "Up to date ?";
	}
	if (ifromdate == "") {
		let fromdate = date();
	}
//	if (defaultperiod.unassigned()) {
//		defaultperiod = "";
//	}
	defaultperiod.defaulter("");

	var fromdate = oconv(ifromdate, "[DATE,*4]");
inpfromdate:
	call note(fromquestion, "RCE", fromdate);
	if (fromdate == "" or fromdate == chr(27)) {
		// cancel:
		ifromdate = "";
		iuptodate = "";
		return 0;
	}
	let ifromdate2 = iconv(fromdate, "[DATE,*4]");
	if (not ifromdate2) {
		call note(fromdate ^ "  is not a valid date");
		goto inpfromdate;
	}
	fromdate = oconv(ifromdate2, "[DATE,*4]");

	// uptodate is fromdate plus defaultperiod
	if (iuptodate.unassigned()) {
		iuptodate = "";
	}
	if (iuptodate == "" or not(iuptodate.isnum())) {
		if (iuptodate == "TODAY") {
			iuptodate = date();
		} else if (iuptodate == "YESTERDAY") {
			iuptodate = date() - 1;
		} else if (iuptodate == "TOMORROW") {
			iuptodate = date() + 1;
		} else if (iuptodate.match("^\\d*D$")) {
			iuptodate = ifromdate2 + iuptodate.first(iuptodate.len() - 1);
		} else if (iuptodate.match("^\\d*W$")) {
			iuptodate = ifromdate2 + iuptodate.first(iuptodate.len() - 1) * 7;
		}
	}

	// make sure todate is not before fromdate
	if (iuptodate < ifromdate2) {
		iuptodate = ifromdate2;
	}

	var uptodate = oconv(iuptodate, "[DATE,*4]");
inpuptodate:
	call note(uptoquestion, "RCE", uptodate);
	if (uptodate == "" or uptodate == chr(27)) {
		goto inpfromdate;
	}

	let iuptodate2 = iconv(uptodate, "[DATE,*4]");
	if (not iuptodate2) {
		call note(uptodate ^ "  is not a valid date");
		goto inpuptodate;
	}
	uptodate = oconv(iuptodate2, "[DATE,*4]");
	if (iuptodate2 < ifromdate2) {
		call note("\"TO DATE\" " ^ uptodate ^ " CANNOT BE BEFORE \"FROM DATE\" " ^ fromdate);
		goto inpuptodate;
	}

	ifromdate = ifromdate2;
	iuptodate = iuptodate2;

	return 0;
}

}; // libraryexit()
