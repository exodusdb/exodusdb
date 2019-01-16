#include <exodus/library.h>
libraryinit()


function main(io css, in version="") {
	//c sys io,""

	//NB quirk in ie mimiced by mozilla table dont inherit size
	//but work around is TABLE {FONT-SIZE:100%} in css

	//prevents IE later versions from working in standards mode
	//<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
	//<meta http-equiv="x-ua-compatible" content="IE=6" />

	css = "";

	var gbp;
	if (not(gbp.open("GBP", ""))) {
		return 0;
	}

	var csskey = "$CSS";
	if (not(version.unassigned())) {
		if (version) {
			csskey ^= "." ^ version;
		}
	}

	if (not(css.read(gbp, csskey))) {
		return 0;
	}

	var charset = SYSTEM.a(127);
	if (charset) {
		css = "<meta http-equiv=\"content-type\" content=\"text/html;charset=" ^ charset ^ "\" />" ^ FM ^ css;
	}

	css.converter(FM, "\r\n");

	//remove trailing EOF character
	css = css.substr(1,0x1A);

	//remove trailing EOL characters
	while (true) {
	///BREAK;
	if (not(var("\r\n").index(css[-1], 1))) break;;
		css.splicer(-1, 1, "");
	}//loop;

	var thcolor = SYSTEM.a(46, 1);
	var tdcolor = SYSTEM.a(46, 2);

	var font = SYSTEM.a(46, 3);
	if (font == "Default") {
		font = "";
	}
	if (font) {
		font ^= ",";
	}

	var fontsize = SYSTEM.a(46, 8);
	if (not fontsize) {
		fontsize = 100;
	}
	if (fontsize and fontsize.isnum()) {
		fontsize ^= "%";
	}
	if (thcolor) {
		css.swapper("#ffff80", "%thcol%");
	}
	if (tdcolor) {
		css.swapper("#ffffc0", "%tdcol%");
	}
	if (thcolor) {
		css.swapper("%thcol%", thcolor);
	}
	if (tdcolor) {
		css.swapper("%tdcol%", tdcolor);
	}

	css.swapper("neosysfont,", font);
	css.swapper("neosyssize", fontsize);

	var agent = SYSTEM.a(40);
	if (agent.index("MSIE 7", 1)) {
		css.swapper("xborder-collapse", "border-collapse");
	}

	//oswrite css on 'css'
	return 0;

}


libraryexit()
