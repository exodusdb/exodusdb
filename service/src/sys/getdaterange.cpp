#include <exodus/library.h>
libraryinit()

	var fromquestion;
var uptoquestion;
var defaultperiod;

function main(in fromquestion0, in uptoquestion0, io ifromdate, io iuptodate) {

	if (fromquestion0.unassigned()) {
		fromquestion = "";
	} else {
		fromquestion = fromquestion0;
	}
	if (uptoquestion0.unassigned()) {
		uptoquestion = "";
	} else {
		uptoquestion = uptoquestion0;
	}
	if (ifromdate.unassigned()) {
		var fromdate = "";
	}

	if (fromquestion eq "") {
		fromquestion = "From date ?";
	}
	if (uptoquestion eq "") {
		uptoquestion = "Up to date ?";
	}
	if (ifromdate eq "") {
		var fromdate = date();
	}
	if (defaultperiod.unassigned()) {
		defaultperiod = "";
	}

	var fromdate = oconv(ifromdate, "[DATE,*4]");
inpfromdate:
	call note(fromquestion, "RCE", fromdate, "");
	if (fromdate eq "" or fromdate eq chr(27)) {
		//cancel:
		ifromdate = "";
		iuptodate = "";
		return 0;
	}
	var ifromdate2 = iconv(fromdate, "[DATE,*4]");
	if (not ifromdate2) {
		call mssg(fromdate ^ "  is not a valid date");
		goto inpfromdate;
	}
	fromdate = oconv(ifromdate2, "[DATE,*4]");

	//uptodate is fromdate plus defaultperiod
	if (iuptodate.unassigned()) {
		iuptodate = "";
	}
	if (iuptodate eq "" or not(iuptodate.isnum())) {
		if (iuptodate eq "TODAY") {
			iuptodate = date();
		} else if (iuptodate eq "YESTERDAY") {
			iuptodate = date() - 1;
		} else if (iuptodate eq "TOMORROW") {
			iuptodate = date() + 1;
		} else if (iuptodate.match("^\\d*D$")) {
			iuptodate = ifromdate2 + iuptodate.first(iuptodate.len() - 1);
		} else if (iuptodate.match("^\\d*W$")) {
			iuptodate = ifromdate2 + iuptodate.first(iuptodate.len() - 1) * 7;
		}
	}

	//make sure todate is not before fromdate
	if (iuptodate lt ifromdate2) {
		iuptodate = ifromdate2;
	}

	var uptodate = oconv(iuptodate, "[DATE,*4]");
inpuptodate:
	call note(uptoquestion, "RCE", uptodate, "");
	if (uptodate eq "" or uptodate eq chr(27)) {
		goto inpfromdate;
	}

	var iuptodate2 = iconv(uptodate, "[DATE,*4]");
	if (not iuptodate2) {
		call mssg(uptodate ^ "  is not a valid date");
		goto inpuptodate;
	}
	uptodate = oconv(iuptodate2, "[DATE,*4]");
	if (iuptodate2 lt ifromdate2) {
		call mssg("\"TO DATE\" " ^ uptodate ^ " CANNOT BE BEFORE \"FROM DATE\" " ^ fromdate);
		goto inpuptodate;
	}

	ifromdate = ifromdate2;
	iuptodate = iuptodate2;

	return 0;
}

libraryexit()
