#include <exodus/library.h>
libraryinit()


#include <gen.h>

var params;
var params2;

function main(in mode, io dataio, in params0="", in params20="") {
	//c sys in,io,"",""

	//REPLACING UPPERCASE VERSION HTMLLIB()

	if (params0.unassigned()) {
		params = "";
	}else{
		params = params0;
	}
	if (params20.unassigned()) {
		params2 = "";
	}else{
		params2 = params20;
	}

	if (mode == "TABLE.MAKE") {

		dataio.trimmer();
		var tt = " " ^ VM;
		while (true) {
		///BREAK;
		if (not(dataio.index(tt, 1))) break;;
			dataio.swapper(tt, VM);
		}//loop;
		tt = " " ^ FM;
		while (true) {
		///BREAK;
		if (not(dataio.index(tt, 1))) break;;
			dataio.swapper(tt, FM);
		}//loop;

		//filler='&nbsp;'
		//gosub fill

		dataio.swapper(FM, "</td></tr>" ^ FM ^ "<tr><td>");
		dataio.swapper(VM, "</td>" ^ VM ^ "<td>");
		dataio = "<tr><td>" ^ dataio ^ "</td></tr>";

		if (params2) {
			var line1 = dataio.a(1);
			line1.swapper("</TR>", "</tr>");
			line1.swapper("</tr>", "</tr>" ^ FM ^ "</thead>" ^ FM ^ "<tbody>");
			line1.swapper("<td>", "<th>");
			line1.swapper("</td>", "</th>");
			line1.swapper("<td>", "<th>");
			line1.swapper("</td>", "</th>");
			if (not(line1.index("<thead", 1))) {
				line1 = FM ^ "<thead>" ^ FM ^ line1;
			}
			dataio.r(1, line1);
		}

		//prefix
		//tableid='T':rnd(999999999)[-7,7]
		tt = params;
		//swap '<TABLE' with '<table' in tt
		if (not params2) {
			tt ^= FM ^ "<tbody>";
		}
		dataio.splicer(1, 0, tt ^ FM);

		//postfix
		dataio = dataio ^ FM ^ "</tbody>" ^ FM ^ "</table>";

		//highlight last row if a total
		var nlines = dataio.count(FM) + 1;
		var lastline = dataio.a(nlines);
	//Total
		if (lastline.index("<td>" ^ gen.glang.a(28), 1)) {
			lastline.swapper("TD>", "th>");
			lastline.swapper("td>", "th>");
			dataio.r(nlines, lastline);
		}

		//convert fm:vm to '' in dataio
		dataio.converter(VM, "");

	//fill in the missing cells
	} else if (mode == "TABLE.FILL") {

		var filler = "<TD>&nbsp;</TD>";
		//gosub fill

	} else if (mode == "HASHTABLE") {

		var nv = dataio.count(VM) + 1;
		for (var vn = 1; vn <= nv; ++vn) {
			var datax = dataio.a(1, vn);

			//insert a break before all space+#
			datax.swapper(" #", FM ^ "#");

			//restore #+space to be NOT broken
			datax.swapper(FM ^ "# ", " # ");

			datax.converter(TM, FM);

			//equ tr to '<tr xvalign=top>'

			var nlines = datax.count(FM) + 1;
			if (nlines > 1) {

				for (var ln = 1; ln <= nlines; ++ln) {
					var line = datax.a(ln).trim();
					if (line[1] == "#") {
						line.splicer(1, 1, "");
						var tt1 = line.field(" ", 1);
						tt1.converter("_", " ");
						var tt2 = line.field(" ", 2, 9999);
						if (tt2[1] == "=") {
							tt2.splicer(1, 1, "");
						}
						datax.r(ln, "<td>" ^ tt1 ^ ":" "</td>" "<td>" ^ tt2 ^ "</td>");
					}else{
						datax.r(ln, "<td colspan=2>" ^ line ^ "</td>");
					}
				};//ln;

				datax.swapper(FM, "</tr>" "<tr>");
				datax.splicer(1, 0, "<table class=\"hashtable\">" "<tr>");
				datax ^= "</tr>" "</table>";

				dataio.r(1, vn, datax);

			}

		};//vn;

	} else if (mode == "STRIPTAGS") {
		while (true) {
			var tag1 = dataio.index("<", 1);
			var tag2 = dataio.index(">", 1);
		///BREAK;
		if (not(tag1 and tag1 < tag2)) break;;
			dataio.splicer(tag1, tag2 - tag1 + 1, "");
		}//loop;

	} else if (mode == "DECODEHTML") {
		dataio.swapper("&nbsp;", " ");
		dataio.swapper("&lt;", "<");
		dataio.swapper("&gt;", ">");
		dataio.swapper("&amp;", "&");

	} else if (mode.field(".", 1, 2) == "OCONV.AGENT") {

		//to check versus DOS BROWSERS.TXT
		//GET NEW RUN GBP2 CHK.AGENTSTRING

		//Windows NT 6.3 Windows 8.1
		//Windows NT 6.2 Windows 8
		//Windows NT 6.1 Windows 7
		//Windows NT 6.0 Windows Server 2008/Vista
		//Windows NT 5.2 Windows Server 2003; Windows XP x64 Edition
		//Windows NT 5.1 Windows XP

		//MS useragent string documentation
		//https://msdn.microsoft.com/en-us/library/ms537503%28v=vs.85%29.aspx
		//MS Window Versions
		//https://msdn.microsoft.com/en-us/library/windows/desktop/ms724832%28v=vs.85%29.aspx
		//similar code in INIT.GENERAL and HTMLLIB
		//name can be CLIENT OS or SERVER OS
		var osname = "";
		if (dataio.index("NT 10.0", 1)) {
			osname = "10";
		} else if (dataio.index("NT 6.3", 1)) {
			osname = "8.1";
		} else if (dataio.index("NT 6.2", 1)) {
			osname = "8";
		} else if (dataio.index("NT 6.1", 1)) {
			osname = "7";
		} else if (dataio.index("NT 6.0", 1)) {
			osname = "Vista/2008";
		} else if (dataio.index("NT 5.2", 1)) {
			osname = "XP-64";
		} else if (dataio.index("NT 5.1", 1)) {
			osname = "XP";
		} else if (dataio.index("Windows NT ", 1)) {
			var tt = dataio.index("Windows NT ", 1);
			osname = dataio.substr(tt + 11,9999).field(";", 1).field(")", 1);
		}
		if (dataio.index("WOW64", 1)) {
			osname ^= "-64";
		}
		if (osname) {
			osname = "Win" ^ osname;
			//add touch if Windows but not Windows Phone
			if (dataio.index("Touch", 1)) {
				osname ^= " Touch";
			}
		}

		if (not osname) {
			var tt = 0;
			if (not tt) {
				tt = dataio.index("Android ", 1);
			}
			if (not tt) {
				tt = dataio.index("Android", 1);
			}
			if (not tt) {
				tt = dataio.index("iPhone OS", 1);
			}
			if (not tt) {
				tt = dataio.index("CPU OS", 1);
			}
			if (not tt) {
				tt = dataio.index("Mac OS", 1);
			}
			if (not tt) {
				tt = dataio.index("Windows Phone ", 1);
				if (tt) {
					dataio.splicer(tt, 13, "WinPhone");
				}
			}
			if (not tt) {
				tt = dataio.index("Linux", 1);
			}
			if (not tt) {
				tt = dataio.index("CrOS", 1);
			}
			if (tt) {
				osname = dataio.substr(tt,9999);
				if (osname.substr(1,4) == "CPU ") {
					osname.splicer(1, 3, "iPad");
				}
				tt = osname.index(" like ", 1);
				if (tt) {
					osname.splicer(tt, 9999, "");
				}
				osname = osname.field(";", 1);
				osname = osname.field(")", 1);
				osname.swapper(" x86_64", "-64");
			}
		}
		if (osname.substr(-2,2) == ".0") {
			osname.splicer(-2, 2, "");
		}

		//Trident tells you the actual browser software for MS
		var browser = "";
		if (dataio.index("Trident/7.0", 1)) {
			browser = "11";
		} else if (dataio.index("Trident/6.0", 1)) {
			browser = "10";
		} else if (dataio.index("Trident/5.0", 1)) {
			browser = "9";
		} else if (dataio.index("Trident/4.0", 1)) {
			browser = "8";
		}
//L1403:
		//MSIE tells you the operating mode for MS
		//MSIE 10.0 Internet Explorer 10
		//MSIE 9.0 Internet Explorer 9
		//MSIE 8.0 Internet Explorer 8 or IE8 Compatibility View/Browser Mode
		//MSIE 7.0 Windows Internet Explorer 7 or IE7 Compatibility View/Browser Mode
		//MSIE 6.0 Microsoft Internet Explorer 6
		var iemode = "";
		if (dataio.index("MSIE 10.0", 1)) {
			iemode = "10";
		} else if (dataio.index("MSIE 9.0", 1)) {
			iemode = "9";
		} else if (dataio.index("MSIE 8.0", 1)) {
			iemode = "8";
		} else if (dataio.index("MSIE 7.0", 1)) {
			iemode = "7";
		} else if (dataio.index("MSIE 6.0", 1)) {
			iemode = "6";
		}
//L1512:
		//add the mode to the browser if different (MS only)
		if (not browser) {
			browser = iemode;
		}
		if (browser and iemode and iemode ne browser) {
			browser ^= " IE" ^ iemode ^ "mode";
		}
		if (browser) {
			browser = "IE" ^ browser;
		}

		if (not browser) {
			var tt = dataio.index("Chrome", 1);
			if (not tt) {
				tt = dataio.index("Firefox", 1);
			}
			if (not tt) {
				tt = dataio.index("Safari", 1);
			}
			if (not tt) {
				tt = dataio.index("Opera", 1);
			}
			if (not tt) {
				tt = dataio.index("Netscape", 1);
			}
			if (tt) {
				browser = dataio.substr(tt,9999).field(";", 1).field(" ", 1).field(")", 1);
			}
		}
		if (browser.substr(-2,2) == ".0") {
			browser.splicer(-2, 2, "");
		}

		var submode = mode.field(".", 3);
		if (submode == "OS") {
			dataio = osname;
			return 0;
		} else if (submode == "BROWSER") {
			dataio = browser;
			return 0;
		}
//L1727:
		if (osname or browser) {
			if (osname) {
				osname ^= "; ";
			}
			if (browser) {
				osname ^= browser;
			}else{
				osname ^= dataio;
			}
			//tt=osname:'<br>':agent
			dataio = osname;
		}

		//returns agent in dataio see equate above

	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " unknown mode in HTMLLIB");
	}
//L1811:
	return 0;

	/*;
	/////
	fill:
	/////
			//no more filling
			return 0;

			n=count(dataio,fm)+(dataio ne '');

			//find max number of columns
			n2=0;
			for i=1 to n;
				tt=count(dataio<i>,vm)+1;
				if tt>n2 then n2=tt;
				next i;

			//make sure all columns are filled
			for i=1 to n;
				for i2=1 to n2;
					if dataio<i,i2>='' then dataio<i,i2>=filler;
					next i2;
				next i;

	*/

}


libraryexit()