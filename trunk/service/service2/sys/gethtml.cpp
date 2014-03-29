#include <exodus/library.h>
libraryinit()

#include <gen.h>
#include <agy.h>

function main(in mode, out html, in companycode0="") {
	//jbase linemark uses agp under some circumstances
	var modex = mode;

	var companycode = companycode0;
	if (not companycode) {
		companycode = gen.gcurrcompany;
	}

	var compcode = companycode ^ "_";
	var keyx = "";
nextmode:
	html = "";
	//osread html from '..\data\':system<17>:'\':(compcode:modex<1,1>)[1,8]:'.HTM' else
	keyx = "..\\data\\" ^ SYSTEM.a(17) ^ "\\" ^ compcode ^ modex.a(1, 1).substr(1,8) ^ ".HTM";

	//no longer look for letterhead in os files
	//osread html from keyx else

	//read html from definitions,compcode:modex<1,1>:'.HTM' else
	keyx = compcode ^ modex.a(1, 1) ^ ".HTM";

	if (not(html.read(DEFINITIONS, keyx))) {
		//osread html from (compcode:modex<1,1>)[1,8]:'.HTM' else
		keyx = compcode ^ modex.a(1, 1).substr(1,8) ^ ".HTM";

		//no longer look for letterhead in os files
		//osread html from keyx else

		if (compcode) {
			compcode = "";
			goto nextmode;
		}
		if (modex.a(1, 2)) {
			modex = modex.field(VM, 2, 9999);
			compcode = companycode ^ "_";
			goto nextmode;
		}
		var logofilename = "c:\\agency.gif";
		if (not logofilename.osfile()) {
			logofilename = "";
		}
		var agencynameaddress = agy.agp.a(1) ^ " " ^ SYSTEM.a(18);
		var contacts = SYSTEM.a(19);
		var emailaddress = SYSTEM.a(10, 2);
		var website = SYSTEM.a(8);
		if (website.ucase().substr(1,7) == "HTTP://") {
			website.splicer(1, 7, "");
		}

		html = "";
		html ^= "<table border=\"0\" width=\"100%\">";
		html ^= "<tr>";

		if (logofilename) {
			html ^= "<td valign=\"top\">";
			html ^= "<img src=\"file:///" ^ logofilename ^ DQ;
			html ^= " alt=\"" ^ agy.agp.a(1) ^ DQ;
			html ^= " width=\"100\" height=\"70\"";
			html ^= ">";
			html ^= "</td>";
			html ^= "<td align=right>";

			//table in a table to get left aligned text in right aligned cell
			html ^= "<table><tr><td>";

		}else{
			html ^= "<td align=center>";
		}

		if (logofilename) {
			html ^= "<font size=\"4\">";
		}else{
			html ^= "<font size=\"6\"><b>";
		}
		html ^= agencynameaddress;
		html ^= "<br>";
		html ^= "</b></font>";

		html ^= "<font size=\"1\">";
		if (emailaddress) {
			html ^= " Email: <a href=\"mailto:" ^ emailaddress ^ "\">" ^ emailaddress ^ "</a>";
		}

		if (website) {
			var website2 = website;
			if (not((website2.ucase()).index("HTTP", 1))) {
				website2 = "http://" ^ website2;
			}
			html ^= " Website: <a href=\"" ^ website2 ^ "\">" ^ website ^ "</a>";
		}

		html ^= " " ^ contacts;

		html ^= "</font>";

		//table in a table to get left aligned text in right aligned cell
		if (logofilename) {
			html ^= "</td>";
			html ^= "</tr>";
			html ^= "</table>";
		}

		html ^= "</td>";
		html ^= "</tr>";
		html ^= "</table>";

		//end
	}
	//end

	html.swapper("%COMPANY_NAME%", gen.company.a(1));
	html.swapper("%TAX_REGISTRATION_NO%", gen.company.a(21));
	html.swapper("%TAX_REG_NO%", gen.company.a(21));
	html.swapper("%COMPANY_REG_NO%", gen.company.a(59));

	//check simple HTML
	if (html and html.count("<") ne html.count(">")) {
		call mssg(DQ ^ (keyx ^ DQ) ^ " page header is not valid HTML");
		html = "";
	}

	html.swapper(FM, "\r\n");
	html = html.field(var().chr(26), 1);
	while (true) {
	///BREAK;
	if (not(html and var("\r\n").index(html[-1], 1))) break;;
		html.splicer(-1, 1, "");
	}//loop;

	var batchmode = SYSTEM.a(33);
	if (batchmode) {
		html.swapper("file:///c:/neosys.gif", "http://www.neosys.com/neosys.gif");
	}

	return 0;

}


libraryexit()
