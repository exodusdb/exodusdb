#include <exodus/library.h>
libraryinit()

#include <date.h>
#include <authorised.h>

#include <gen_common.h>

var modex;
var hascompanies;
var compcode;
var lhcompany;
var keyx;
var tt;
var tdate;
var xx;
var div;
var divx;

function main(in mode0, out html, in compcode0="") {
	//c sys in,out,""

	#include <general_common.h>
	//$insert bp,agency.common
	//global lhcompany,modex,tt,hascompanies,compcode

	var mode = mode0;
	modex = mode;
	html = "";

	//returns source in MODE ie COMPANY or definitions file key

	//TODO what about letterhead with vehicle logos?

	//cant use global company info if not initialised in c++
	hascompanies = SYSTEM.a(133);

	compcode = "";
	if (compcode0.unassigned()) {
		if (hascompanies) {
			compcode = gen.gcurrcompany;
		}
	} else if (compcode0) {
		compcode = compcode0;
	} else {
		if (hascompanies) {
			compcode = gen.gcurrcompany;
		}
	}

	lhcompany = "";
	if (hascompanies) {
		if (not(gen.companies.unassigned() or (gen.companies == ""))) {
			if (not(lhcompany.read(gen.companies, compcode))) {
				{}
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
	gosub getcompanyconfig(html, mode);

	//otherwise get from definitions
	///////////////////////////////

	if (not html) {
		gosub getheadhtm(html);
		if (html) {
			mode = "DEFINITIONS " ^ keyx;
		}else{
			mode = "No letterhead defined";
		}

		//normalise handcoded html so later conversions will work
		html.swapper("<image", "<img");
		html.swapper("<IMAGE", "<img");

		//image should not have closing tag but dont bother removing it
		//swap '</image>' with '' in html
		//swap '</IMAGE>' with '' in html

	}

	//process the HTML, adding various macros

	var clientmark = lhcompany.a(27).invert();
	if (hascompanies) {
		//if clientmark else clientmark=agp<1>
		if (not clientmark) {
			clientmark = SYSTEM.a(14);
		}
	}
	html.swapper("%AGENCY%", clientmark);

	//similar code in GETHTML and GENERALMACROS TODO: use GENERALMACROS instead

	html.swapper("%COMPANY_NAME%", lhcompany.a(1));
	html.swapper("%TAX_REGISTRATION_NO%", lhcompany.a(21));
	html.swapper("%TAX_REG_NO%", lhcompany.a(21));
	html.swapper("%COMPANY_REG_NO%", lhcompany.a(59));

	var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
	tt = "L";
	if (hascompanies) {
		call date("OCONV", datetime, tt, tdate, gen.glang);
	}else{
		call date("OCONV", datetime, tt, tdate, "");
	}
	html.swapper("%DATE%", tdate);
	html.swapper("%TIME%", oconv(datetime.field(".", 2), "[TIME2]"));
	html.swapper("%STATION%", STATION);

	html.swapper("%DATAURL%", "%URL%/data/%DATABASE%");
	html.swapper("%URL%", SYSTEM.a(114, 1));
	html.swapper("%DATABASE%", SYSTEM.a(17, 1));

	//check valid html .. html from company file is prechecked anyway

	if (html) {

		//check simple HTML
		if (html.count("<") ne html.count(">")) {
			call mssg(keyx.quote() ^ " page header is not valid HTML");
			html = "";
		}

		//check various tags exist in equal numbers
		//this doesnt check if they are in a correct sequence or hierarchy etc
		var tags = "div,span,table,thead,tbody,tr,td,a,b,i,u,big,small,centre,abbr";
		var ntags = tags.count(",") + 1;
		var html2 = html.lcase();
		for (var tagn = 1; tagn <= ntags; ++tagn) {
			var tag = tags.field(",", tagn);
			if (html2.count("<" ^ tag ^ ">") + html2.count("<" ^ tag ^ " ") ne html2.count("</" ^ tag ^ ">")) {
				html = keyx.quote() ^ " has mismatched &lt;" ^ tag ^ "&gt; tags";
				tagn = ntags;
			}
		};//tagn;

	}

	html.swapper(FM, "\r\n");
	//html=field(html,char(26),1)
	while (true) {
		///BREAK;
		if (not(html and (var("\r\n").index(html[-1])))) break;
		html.splicer(-1, 1, "");
	}//loop;

	//presumably obsolete
	//batchmode=system<33>
	//if batchmode then
	// swap 'file:///c:/exodus.gif' with 'http://www.neosys.com/exodus.gif' in html
	// end

	if (authorised("EDIT PRINTOUTS", xx)) {

		//button
		var onclick = "javascript:";
		onclick ^= "if (document.body.getAttribute(\'contentEditable\')) {";
		onclick ^= " edithtml.innerHTML=\'Edit is Off\';";
		onclick ^= " window.setTimeout(\'edithtml.innerHTML=\\\'\\\';edithtml.style.display=\\\'none\\\'\',1000);";
		onclick ^= " document.body.removeAttribute(\'contenteditable\')";
		onclick ^= "} else {";
		onclick ^= " edithtml.style.display=\'\';";
		onclick ^= " edithtml.innerHTML=\'Edit is On\';";
		onclick ^= " document.body.setAttribute(\'contenteditable\',\'true\')";
		onclick ^= "}";

		tt = "<button id=edithtml class=\"noprint\"";
		tt ^= " style=\"position:fixed;top:2px;left:2px;font-size:60%;display:none\"";
		tt ^= " onclick=" ^ (onclick.quote());
		tt ^= "></button>";
		html.splicer(1, 0, tt);

		//click logos to switch on/off editing
		onclick = "javascript:edithtml.click()";
		html.swapper("<IMG", "<img");
		html.swapper("<img", "<img style=\"cursor:pointer\" onclick=" ^ (onclick.quote()));

	}

	return 0;
}

subroutine getcompanyconfig(io html, io mode) {

	html = "";

	var ncols = 0;
	for (var fn = 61; fn <= 66; ++fn) {
		tt = lhcompany.a(fn);
		if (tt) {
			//call max(count(tt,vm)+1,ncols,ncols)
			tt = tt.count(VM) + 1;
			if (tt > ncols) {
				ncols = tt;
			}
		}
	};//fn;
	if (not ncols) {
		return;
	}

	var aligns = lhcompany.a(61);
	var imagetypes = lhcompany.a(62);
	var texts = lhcompany.a(63);
	var fontsizes = lhcompany.a(64);
	var imagecompcodes = lhcompany.a(65);
	var textcompcodes = lhcompany.a(66);

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
		tab.r(-1, "<table width=100% cellspacing=0 cellpadding=0 borderpadding=0");
		tab ^= " style=\"border-collapse:collapse\"";
		tab ^= ">";
		tab.r(-1, " <tr>");
	}else{
		tab.r(-1, "<div>");
	}

	for (var coln = 1; coln <= ncols; ++coln) {

		var align = aligns.a(1, coln);
		var imagetype = imagetypes.a(1, coln);
		var imagecompcode = imagecompcodes.a(1, coln);
		var text = texts.a(1, coln);
		var fontsize = fontsizes.a(1, coln);
		if (fontsize and fontsize.isnum()) {
			fontsize ^= "%";
		}

		//start a new TD
		if (usetable) {
			tab.r(-1, "");
			tab.r(-1, "  <td");
			tab ^= " width=" ^ (100 / ncols).floor() ^ "%";
			tab ^= ">";
		}

		//wrap td contents in a div if any styling
		var divstyle = "";
		if (align) {
			if (align == "center") {
				//there is no FLOAT CENTER
				divstyle ^= "display:table;margin-left:auto;margin-right:auto;";
			}else{
				divstyle ^= "float:" ^ align ^ ";";
			}
		}
		if (fontsize) {
			divstyle ^= "font-size:" ^ fontsize ^ ";";
		}
		if (divstyle) {
			div = FM ^ "   <div style=" ^ (divstyle.quote()) ^ ">";
			divx = FM ^ "   </div>";
		}else{
			div = "";
			divx = "";
		}

		//add image
		if (imagetype or imagecompcode) {
			tab ^= div;

			//use other company image and type
			if (imagecompcode) {
				//get image type from the other company
				var imagecomp;
				if (not(imagecomp.read(gen.companies, imagecompcode))) {
					imagecomp = "";
				}
				imagetype = imagecomp.a(62, coln);

				mode ^= ", Col " ^ coln ^ " image from company " ^ imagecompcode;

			//use this company image and type
			}else{
				imagecompcode = compcode;
			}

			//FULL http path to images so EMAIL/OFFICE programs can get images
			var url = SYSTEM.a(114, 1);
			if (url[-1] == "/") {
				url.splicer(-1, 1, "");
			}

			//path to uploaded company logo files
			var imagepath = "/images/" ^ SYSTEM.a(17) ^ "/UPLOAD/COMPANIES/";

			//logo_companycode_coln .jpg .png /gif
			var imagefilename = "logo_" ^ imagecompcode ^ "_" ^ coln ^ "." ^ imagetype;

			var fullimageurl = url ^ imagepath ^ imagefilename;
			var relativeimagefilename = "../.." ^ imagepath ^ imagefilename;

			tab.r(-1, "   <img src=" ^ (fullimageurl.quote()) ^ " alt=" ^ (fullimageurl.quote()));
			var sq = "\'";
			tab ^= " onerror=\"this.onerror=null;this.src=" ^ sq ^ relativeimagefilename ^ sq ^ ";\"";
			tab ^= " style=\"margin:0;border:0\"";

			tab ^= " />";
			tab ^= divx;
		}

		//maybe get text from another company
		var textcompcode = textcompcodes.a(1, coln);
		if (textcompcode) {
			//ignore any given text if textcompany given
			text = "";
			var textcompany = "";
			if (hascompanies) {
				if (not(textcompany.read(gen.companies, textcompcode))) {
					textcompany = "";
				}
			}
			//get text from different company SAME COLUMN NO
			text = textcompany.a(63, coln);
			mode ^= ", Col " ^ coln ^ " text from company " ^ textcompcode;
		}

		//add text div
		if (text) {
			text.swapper(TM, "<br />" "\r\n");
			tab ^= div;
			tab.r(-1, text);
			tab ^= divx;
		}

		if (usetable) {
			tab.r(-1, "  </td>");
		}

	};//coln;

	if (usetable) {
		tab.r(-1, "");
		tab.r(-1, " </tr>");
		tab.r(-1, "</table>");
	}else{
		tab.r(-1, "</div>");
	}

	html = tab;

	return;
}

subroutine getheadhtm(io html) {

	{}

nextmodex:
//////////
	var prefix = compcode ^ "_";

nextprefix:
///////////
	keyx = prefix ^ modex.a(1, 1) ^ ".HTM";

	if (not(html.read(DEFINITIONS, keyx))) {

		//try again same mode but without company code prefix
		if (prefix) {
			prefix = "";
			goto nextprefix;
		}

		//try again with next mode and with company code prefix
		if (modex.a(1, 2)) {
			modex = modex.field(VM, 2, 9999);
			goto nextmodex;
		}

	}

	return;
}

libraryexit()
