#include <exodus/library.h>
libraryinit()

#include <date.h>

#include <gen_common.h>
#include <agy_common.h>

var usercode;
var username;
var companycode;
var datetime;
var stationx;
var xglang;
var clientmark;
var img;
var tdate;

function main(in mode, out ftx, in clientmark0, in companycode0, in username0, in usercode0, in datetime0, in station0, in xglang0) {
	//c gen in,out

	//WARNING
	//calls {EXECUTIVE_NAME} so requires @dict, @record to be setup correctly

	#include <general_common.h>
	#include <agency_common.h>

	//evade c++ warning of unused argument
	if (not mode) {
		{}
	}

	if (usercode0.unassigned()) {
		usercode = "";
	}else{
		usercode = usercode0;
	}
	if (usercode == "") {
		usercode = USERNAME;
	}

	if (username0.unassigned()) {
		username = "";
	}else{
		username = username0;
	}
	if (not username) {
		username = usercode.xlate("USERS", 1, "C");
	}
	if (usercode == "-") {
		usercode = "";
	}
	if (username == "-") {
		username = "";
	}

	if (companycode0.unassigned()) {
		companycode = gen.gcurrcompany;
	}else{
		companycode = companycode0;
	}
	if (datetime0.unassigned()) {
		datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
	}else{
		datetime = datetime0;
	}
	if (station0.unassigned()) {
		stationx = STATION;
	}else{
		stationx = station0;
	}
	if (xglang0.unassigned()) {
		xglang = gen.glang;
	}else{
		xglang = xglang0;
	}

	//should be from company file
	if (clientmark0.unassigned()) {
		clientmark = "";
	}else{
		clientmark = clientmark0;
	}
	if (not clientmark) {
		clientmark = gen.company.a(27).invert();
	}
	if (not clientmark) {
		clientmark = agy.companyname;
	}

	ftx.swapper("%AGENCY%", clientmark);
	ftx.swapper("%AGENCY NAME", clientmark);
	ftx.swapper("%AGENCY", clientmark);

	ftx.swapper("%COMPANY%", gen.company.a(1));
	ftx.swapper("%COMPANYCODE%", companycode);

	//similar code in GETHTML and AGENCYMACROS

	ftx.swapper("%COMPANY_REG_NO%", gen.company.a(59));
	ftx.swapper("%TAX_REG_NO%", gen.company.a(21));

	ftx.swapper("%EMAIL%", usercode.xlate("USERS", 7, "X"));

	ftx.swapper("%USERCODE%", usercode);
	ftx.swapper("%EXECUTIVE%", calculate("EXECUTIVE_NAME"));

	//standardise various old forms
	ftx.swapper("%USERNAME%", "%TEMP%");
	ftx.swapper("%USERNAME", "%TEMP%");
	ftx.swapper("%USER%", "%TEMP%");
	ftx.swapper("%USER", "%TEMP%");

	//option to add signature next to %USERNAME%
	//(only if no other image file already configured)
	var addsig = "";
	var tt = ftx.ucase();
	if (tt.index(".JPG")) {
		addsig = "n";
	}
	if (tt.index(".JPEG")) {
		addsig = "n";
	}
	if (tt.index(".GIF")) {
		addsig = "n";
	}
	if (tt.index(".PNG")) {
		addsig = "n";
	}
	if (not addsig) {
		addsig = SYSTEM.a(119);
	}
	if (not addsig) {
		addsig = "b";
	}
	if (ftx.index("%TEMP%")) {
		if ((addsig and not(ftx.index("%SIGNATURE%"))) and not(ftx.index("%NOSIGNATURE%"))) {
			//above,below,left,right,no
			if (addsig == "a") {
				ftx.swapper("%TEMP%", "%SIGNATURE%<br />%TEMP%");
			} else if (addsig == "b") {
				ftx.swapper("%TEMP%", "%TEMP%<br />%SIGNATURE%");
			} else if (addsig == "l") {
				ftx.swapper("%TEMP%", "%SIGNATURE% %TEMP%");
			} else if (addsig == "r") {
				ftx.swapper("%TEMP%", "%TEMP% %SIGNATURE%");
			}
		}
		ftx.swapper("%TEMP%", username);
	}

	//%USERNAME%
	ftx.swapper("%TEMP%", username);

	//%SIGNATURE%
	if (ftx.index("%NOSIGNATURE%")) {
		ftx.swapper("%NOSIGNATURE%", "");
	}else{

		if (ftx.index("%SIGNATURE%")) {

			//check for an uploaded vehicle logo (ASSUMES MAX 8.3 CHARACTERS)
			var sigfilename = SYSTEM.a(17, 1) ^ "/upload/users/" ^ usercode.convert(" ", "") ^ "_signature.jpg";
			tt = (SYSTEM.a(49) ^ sigfilename).lcase();
			sigfilename.converter("/", OSSLASH);
			//call shell2('dir ':tt,errors)
			//if errors then
			// sigfilename=''
			// end
			if (not(oslistf(tt))) {
				sigfilename = "";
			}
			if (sigfilename) {
				img = "<img src=" ^ (("../../images/" ^ sigfilename).quote()) ^ " alt=\"\"";
				img.converter(OSSLASH, "/");
				//unfortunately cannot assume script will run (eg in printpreview)
				//img:=' width=0 height=0 onload=':quote("this.removeAttribute('height');this.removeAttribute('width')")
				img ^= " />";
			}else{
				img = "";
			}

			ftx.swapper("%SIGNATURE%", img);
		//capital B option not in configuration options or tested at time of writing
		//B=add signature at bottom (unless %SIGNATURE or %NOSIGNATURE% configured)
		} else if (addsig == "B") {
			ftx ^= "<br /" ^ img;
		}

	}

	//similar code in GETHTML and AGENCYMACROS

	//swap '%DATE%' with field(rr<20>,'.',1) '[DATE,4*]' in ftx
	call date("OCONV", datetime, "L", tdate, xglang);
	ftx.swapper("%DATE%", tdate);

	ftx.swapper("%TIME%", oconv(datetime.field(".", 2), "[TIME2]"));

	ftx.swapper("%STATION%", stationx);

	return 0;
}

libraryexit()
