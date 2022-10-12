#include <exodus/library.h>
libraryinit()

#include <langdate.h>
#include <authorised.h>

// For sys.gcurrcompcode (for compcode), sys.companies (to read letterheadcompany and imagecompany) and sys.glang (months for langdate)
#include <sys_common.h>

var modex;
var hascompanies;
var compcode;
var letterheadcompany;
var keyx;
var tt;

function main(in mode0, out letterhead_out, in compcode0="", in qr_text="") {

	// gethtml may be "perform"ed, "execute"d  or "call"ed
	//
	// 1. PERFORM/EXECUTE
	//
	//  letterhead = perform("gethtml");
	//
	// nlist PERFORMs gethtml (via printtx.hpp) when service lib list.cpp performs nlist.
	//
	// nlist does not perform, nor calls, gethtml when cli list.cpp command line calls nlist.
	//
	// 2. CALL
	//
	//  call gethtml(mode, letterhead, compcode, qr_text);
	//
	// Lots of service and application code CALLs gethtml directly
	// or indirectly via included printtx.hpp
	//

	var mode;
	if (mode0.assigned()) {
		mode = mode0;
		modex = mode;
	}

	//returns source in MODE ie COMPANY or definitions file key

	//TODO what about letterhead with vehicle logos?

	//cant use global company info if not initialised in c++
	hascompanies = SYSTEM.f(133);

	compcode = "";
	if (mode0.unassigned() or compcode0.unassigned()) {
		if (hascompanies) {
			compcode = sys.gcurrcompcode;
		}
	} else if (compcode0) {
		compcode = compcode0;
	} else {
		if (hascompanies) {
			compcode = sys.gcurrcompcode;
		}
	}

	letterheadcompany = "";
	if (hascompanies) {
		if (not(sys.companies.unassigned() or sys.companies eq "")) {
			if (not(letterheadcompany.read(sys.companies, compcode))) {
				letterheadcompany = "";
			}
		}
	}

	//0. get config from company file if present

	//otherwise old style DEFINITIONS X_HEAD,HEAD.HTM,INVHEAD.HTM etc
	//order of searching if multiple modes like INVHEAD]HEAD
	//where X is the compcode
	//1. X_INVHEAD.HTM
	//2. INVHEAD.HTM
	//3. X_HEAD.HTM
	//4. HEAD.HTM

	//get html depending on company file config
	//////////////////////////////////////////

	mode = "Company File ";
	var letterhead;
	gosub getcompanyconfig(letterhead, mode);

	//otherwise get from definitions
	///////////////////////////////

	if (not letterhead) {
		gosub getheadhtm(letterhead);
		if (letterhead) {
			mode = "DEFINITIONS " ^ keyx;
		} else {
			mode = "No letterhead defined";
		}

		//normalise handcoded html so later conversions will work
		letterhead.replacer("<image", "<img");
		letterhead.replacer("<IMAGE", "<img");

		//image should not have closing tag but dont bother removing it
		//swap '</image>' with '' in letterhead
		//swap '</IMAGE>' with '' in letterhead

	}

	//process the HTML, adding various macros

	var clientmark = letterheadcompany.f(27);
	if (hascompanies) {
		//if clientmark else clientmark=agp<1>
		if (not clientmark) {
			clientmark = SYSTEM.f(14);
		}
	}
	letterhead.replacer("%AGENCY%", clientmark);

	//similar code in GETHTML and GENERALMACROS TODO: use GENERALMACROS instead

	letterhead.replacer("%COMPANY_NAME%", letterheadcompany.f(1));
	letterhead.replacer("%TAX_REGISTRATION_NO%", letterheadcompany.f(21));
	letterhead.replacer("%TAX_REG_NO%", letterheadcompany.f(21));
	letterhead.replacer("%COMPANY_REG_NO%", letterheadcompany.f(59));

	var datetime = date() ^ "." ^ time().oconv("R(0)#5");
	tt = "L";
	var tdate;
	if (hascompanies) {
		call langdate("OCONV", datetime, tt, tdate, sys.glang);
	} else {
		call langdate("OCONV", datetime, tt, tdate, "");
	}
	letterhead.replacer("%DATE%", tdate);
	letterhead.replacer("%TIME%", oconv(datetime.field(".", 2), "[TIME2]"));
	letterhead.replacer("%STATION%", STATION);

	letterhead.replacer("%DATAURL%", "%URL%/data/%DATABASE%");
	letterhead.replacer("%URL%", SYSTEM.f(114, 1));
	letterhead.replacer("%DATABASE%", SYSTEM.f(17, 1));

	// QR code (requires apt install qrencode)
	if (letterhead.contains("%QR%")) {

		var svg = "";
		if (qr_text) {

			var qr_body = qr_text.f(1);
			var qr_tip = qr_text.f(2) ^ qr_body;
			var cmd = "qrencode --size=2 --type=SVG " ^ qr_body.squote();
			svg = osshellread(cmd);

			//remove everything like xml and comments before the opening <svg tag
			svg.cutter(svg.index("<svg") - 1);

//			#tooltip1 { position: relative; }
//			#tooltip1 a span { display: none; color: #FFFFFF; }
//			#tooltip1 a:hover span { display: block; position: absolute; width: 200px; background: #aaa url(images/horses200x50.jpg); height: 50px; left: 100px; top: -10px; color: #FFFFFF; padding: 0 5px; }
//			The html markup is:-
//			<p id="tooltip1"><a href="introduction.php">Introduction<span>Introduction to HTML and CSS: tooltip with extra text</span></a></p>

			svg ="<style>\n"
			"#tooltip1 { position: relative; }\n"
			"#tooltip1 a span { display: none; color: #FFFFFF; }\n"
			"#tooltip1 a:hover span { display: block; position: absolute; white-space: nowrap; background-color: #222; left: 20px; top: 20px; color: #FFFFFF; padding: 0 5px; font-size:170%}\n"
			"</style>\n"
			"<p id='tooltip1'><a>" ^ svg ^ "<span> " ^ qr_tip.replace("\n","<br />") ^ "</span></a></p>";
		}

		letterhead.replacer("%QR%", svg);
	}

	//check valid html .. html from company file is prechecked anyway

	if (letterhead) {

		//check simple HTML
		if (letterhead.count("<") ne letterhead.count(">")) {
			call mssg(keyx.quote() ^ " page header is not valid HTML");
			letterhead = "";
		}

		//check various tags exist in equal numbers
		//this doesnt check if they are in a correct sequence or hierarchy etc
		var tags = "div,span,table,thead,tbody,tr,td,a,b,i,u,big,small,centre,abbr";
		let ntags = tags.count(",") + 1;
		var letterhead2 = letterhead.lcase();
		for (const var tagn : range(1, ntags)) {
			var tag = tags.field(",", tagn);
			if (letterhead2.count("<" ^ tag ^ ">") + letterhead2.count("<" ^ tag ^ " ") ne letterhead2.count("</" ^ tag ^ ">")) {
				letterhead = keyx.quote() ^ " has mismatched &lt;" ^ tag ^ "&gt; tags";
				//tagn = ntags;
				break;
			}
		} //tagn;

	}

	letterhead.replacer(_FM, _EOL);
	letterhead.trimmer(_EOL);

	if (authorised("EDIT PRINTOUTS")) {

		//button
		var onclick = "javascript:";
		onclick ^= "if (document.body.getAttribute('contentEditable')) {";
		onclick ^= " edithtml.innerHTML='Edit is Off';";
		onclick ^= " window.setTimeout('edithtml.innerHTML=\\'\\';edithtml.style.display=\\'none\\'',1000);";
		onclick ^= " document.body.removeAttribute('contenteditable')";
		onclick ^= "} else {";
		onclick ^= " edithtml.style.display='';";
		onclick ^= " edithtml.innerHTML='Edit is On';";
		onclick ^= " document.body.setAttribute('contenteditable','true')";
		onclick ^= "}";

		tt = "<button id=edithtml class=\"noprint\"";
		tt ^= " style=\"position:fixed;top:2px;left:2px;font-size:60%;display:none\"";
		tt ^= " onclick=" ^ (onclick.quote());
		tt ^= "></button>";
		letterhead.prefixer(tt);

		//click logos to switch on/off editing
		onclick = "javascript:edithtml.click()";
		letterhead.replacer("<IMG", "<img");
		letterhead.replacer("<img", "<img style=\"cursor:pointer\" onclick=" ^ (onclick.quote()));

	}

	// Also return letterhead in ANS in case gethtml was performed, not called.
	if (mode0.unassigned()) {
		ANS = letterhead;
		return ANS;
	}

	letterhead.move(letterhead_out);
	return 0;

}

subroutine getcompanyconfig(out letterhead, io mode) {

	letterhead = "";

	var ncols = 0;
	for (const var fn : range(61, 66)) {
		tt = letterheadcompany.f(fn);
		if (tt) {
			//call max(count(tt,vm)+1,ncols,ncols)
			tt = tt.count(VM) + 1;
			if (tt gt ncols) {
				ncols = tt;
			}
		}
	} //fn;
	if (not ncols) {
		return;
	}

	var aligns = letterheadcompany.f(61);
	var imagetypes = letterheadcompany.f(62);
	var texts = letterheadcompany.f(63);
	var fontsizes = letterheadcompany.f(64);
	var imagecompcodes = letterheadcompany.f(65);
	var textcompcodes = letterheadcompany.f(66);

	var tab = "";

	//using a table because
	//a) to give the divs something left/center/right to float within
	//b) to provide a central block in case of left+center+right style heads

	//table columns currently forced equal to allow centralisation in the page
	//but may need allow configuration control over the column widths
	//in case need to allow more space in some columns and less in others
	//eg 20% 60% 20% if the central letterhead is very wide for example

	var usetable = 1;

	//start a one row 100% width table to aid formatting
	if (usetable) {
		tab(-1) = "<table width=100% cellspacing=0 cellpadding=0 borderpadding=0";
		tab ^= " style=\"border-collapse:collapse\"";
		tab ^= ">";
		tab(-1) = " <tr>";
	} else {
		tab(-1) = "<div>";
	}

	for (const var coln : range(1, ncols)) {

		var align = aligns.f(1, coln);
		var imagetype = imagetypes.f(1, coln);
		var imagecompcode = imagecompcodes.f(1, coln);
		var text = texts.f(1, coln);
		var fontsize = fontsizes.f(1, coln);
		if (fontsize and fontsize.isnum()) {
			fontsize ^= "%";
		}

		//start a new TD
		if (usetable) {
			tab(-1) = "";
			tab(-1) = "  <td";
			tab ^= " width=" ^ (100 / ncols).floor() ^ "%";
			tab ^= ">";
		}

		//wrap td contents in a div if any styling
		var divstyle = "";
		if (align) {
			if (align eq "center") {
				//there is no FLOAT CENTER
				divstyle ^= "display:table;margin-left:auto;margin-right:auto;";
			} else {
				divstyle ^= "float:" ^ align ^ ";";
			}
		}
		if (fontsize) {
			divstyle ^= "font-size:" ^ fontsize ^ ";";
		}
//		if (divstyle) {
//			div = FM ^ "   <div style=" ^ (divstyle.quote()) ^ ">";
//			divx = FM ^ "   </div>";
//		} else {
//			div = "";
//			divx = "";
//		}
		var	div = divstyle ? FM ^ "   <div style=" ^ (divstyle.quote()) ^ ">" : "";
		var	divx = divstyle ? FM ^ "   </div>" : "";
		//add image
		if (imagetype or imagecompcode) {
			tab ^= div;

			//use other company image and type
			if (imagecompcode) {
				//get image type from the other company
				var imagecomp;
				if (not(imagecomp.read(sys.companies, imagecompcode))) {
					imagecomp = "";
				}
				imagetype = imagecomp.f(62, coln);

				mode ^= ", Col " ^ coln ^ " image from company " ^ imagecompcode;

			//use this company image and type
			} else {
				imagecompcode = compcode;
			}

			//FULL http path to images so EMAIL/OFFICE programs can get images
			var url = SYSTEM.f(114, 1);
			if (url.ends("/")) {
				url.popper();
			}

			//path to uploaded company logo files
			//var imagepath = "/images/" ^ SYSTEM.f(17) ^ "/UPLOAD/COMPANIES/";
			var imagepath = "/images/" ^ SYSTEM.f(17) ^ "/upload/companies/";

			//logo_companycode_coln .jpg .png /gif
			var imagefilename = "logo_" ^ imagecompcode ^ "_" ^ coln ^ "." ^ imagetype;

			var fullimageurl = url ^ imagepath ^ imagefilename;
			var relativeimagefilename = "../.." ^ imagepath ^ imagefilename;

			tab(-1) = "   <img src=" ^ (fullimageurl.quote()) ^ " alt=" ^ (fullimageurl.quote());
			var sq = "'";
			tab ^= " onerror=\"this.onerror=null;this.src=" ^ sq ^ relativeimagefilename ^ sq ^ ";\"";
			tab ^= " style=\"margin:0;border:0\"";

			tab ^= " />";
			tab ^= divx;
		}

		//maybe get text from another company
		var textcompcode = textcompcodes.f(1, coln);
		if (textcompcode) {
			//ignore any given text if textcompany given
			text = "";
			var textcompany = "";
			if (hascompanies) {
				if (not(textcompany.read(sys.companies, textcompcode))) {
					textcompany = "";
				}
			}
			//get text from different company SAME COLUMN NO
			text = textcompany.f(63, coln);
			mode ^= ", Col " ^ coln ^ " text from company " ^ textcompcode;
		}

		//add text div
		if (text) {
			text.replacer(_TM, "<br />" _EOL);
			tab ^= div;
			tab(-1) = text;
			tab ^= divx;
		}

		if (usetable) {
			tab(-1) = "  </td>";
		}

	} //coln;

	if (usetable) {
		tab(-1) = "";
		tab(-1) = " </tr>";
		tab(-1) = "</table>";
	} else {
		tab(-1) = "</div>";
	}

	letterhead = tab;

	return;
}

subroutine getheadhtm(io letterhead) {

nextmodex:
//////////
	var prefix = compcode ^ "_";

nextprefix:
///////////
	keyx = prefix ^ modex.f(1, 1) ^ var(".htm").ucase();

	if (not(letterhead.read(DEFINITIONS, keyx))) {

		//try again same mode but without company code prefix
		if (prefix) {
			prefix = "";
			goto nextprefix;
		}

		//try again with next mode and with company code prefix
		if (modex.f(1, 2)) {
			modex = modex.field(VM, 2, 9999);
			goto nextmodex;
		}

	}

	return;
}

libraryexit()
