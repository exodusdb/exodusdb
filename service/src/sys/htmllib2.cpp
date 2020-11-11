#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

var params;
var params2;
var tagsep;

function main(in mode, io dataio, in params0="", in params20="") {
	//c sys in,io,"",""

	//REPLACING UPPERCASE VERSION HTMLLIB()

	#include <general_common.h>

	var td = "<td>";
	var tdx = "</td>";
	var tr = "<tr>";
	var trx = "</tr>";

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
			if (not(dataio.index(tt))) break;
			dataio.swapper(tt, VM);
		}//loop;
		tt = " " ^ FM;
		while (true) {
			///BREAK;
			if (not(dataio.index(tt))) break;
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
			if (not(line1.index("<thead"))) {
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
		if (lastline.index("<td>" ^ gen.glang.a(28))) {
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

			var nlines = datax.count(FM) + 1;
			if (nlines > 1) {

				for (var ln = 1; ln <= nlines; ++ln) {
					var line = datax.a(ln).trim();
					if (line[1] == "#") {
						line.splicer(1, 1, "");
						//if colon : present then before colon is the tag name
						if (line.index(":")) {
							tagsep = ":";
						}else{
							tagsep = " ";
						}
						var tt1 = line.field(tagsep, 1);
						tt1.converter("_", " ");
						var tt2 = line.field(tagsep, 2, 9999);
						if (tt2[1] == "=") {
							tt2.splicer(1, 1, "");
						}
						datax.r(ln, td ^ tt1 ^ ":" ^ tdx ^ td ^ tt2 ^ tdx);
					}else{
						datax.r(ln, "<td colspan=2>" ^ line ^ tdx);
					}
				};//ln;

				datax.swapper(FM, trx ^ tr);
				datax.splicer(1, 0, "<table class=\"hashtable\">" ^ tr);
				datax ^= trx ^ "</table>";

				dataio.r(1, vn, datax);

			}

		};//vn;

	} else if (mode == "STRIPTAGS") {
		while (true) {
			var tag1 = dataio.index("<");
			var tag2 = dataio.index(">");
			///BREAK;
			if (not(tag1 and (tag1 < tag2))) break;
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

		#define agent dataio

		//Windows NT 6.3     Windows 8.1
		//Windows NT 6.2     Windows 8
		//Windows NT 6.1     Windows 7
		//Windows NT 6.0     Windows Server 2008/Vista
		//Windows NT 5.2     Windows Server 2003; Windows XP x64 Edition
		//Windows NT 5.1     Windows XP

		//MS useragent string documentation
		//https://msdn.microsoft.com/en-us/library/ms537503%28v=vs.85%29.aspx
		//MS Window Versions
		//https://msdn.microsoft.com/en-us/library/windows/desktop/ms724832%28v=vs.85%29.aspx
		//similar code in INIT.GENERAL and HTMLLIB
		//name can be CLIENT OS or SERVER OS
		var osname = "";
		if (var(agent).index("NT 10.0")) {
			osname = "10";
		} else if (var(agent).index("NT 6.3")) {
			osname = "8.1";
		} else if (var(agent).index("NT 6.2")) {
			osname = "8";
		} else if (var(agent).index("NT 6.1")) {
			osname = "7";
		} else if (var(agent).index("NT 6.0")) {
			osname = "Vista/2008";
		} else if (var(agent).index("NT 5.2")) {
			osname = "XP-64";
		} else if (var(agent).index("NT 5.1")) {
			osname = "XP";
		} else if (var(agent).index("Windows NT ")) {
			var tt = var(agent).index("Windows NT ");
			osname = ((var(agent).substr(tt + 11,9999)).field(";", 1)).field(")", 1);
		}
		if (var(agent).index("WOW64")) {
			osname ^= "-64";
		}
		if (osname) {
			osname = "Win" ^ osname;
			//add touch if Windows but not Windows Phone
			if (var(agent).index("Touch")) {
				osname ^= " Touch";
			}
		}

		if (not osname) {
			var tt = 0;
			if (not tt) {
				tt = var(agent).index("Android ");
			}
			if (not tt) {
				tt = var(agent).index("Android");
			}
			if (not tt) {
				tt = var(agent).index("iPhone OS");
			}
			if (not tt) {
				tt = var(agent).index("CPU OS");
			}
			if (not tt) {
				tt = var(agent).index("Mac OS");
			}
			if (not tt) {
				tt = var(agent).index("Windows Phone ");
				if (tt) {
					var(agent).splicer(tt, 13, "WinPhone");
				}
			}
			if (not tt) {
				tt = var(agent).index("Linux");
			}
			if (not tt) {
				tt = var(agent).index("CrOS");
			}
			if (tt) {
				osname = var(agent).substr(tt,9999);
				if (osname.substr(1,4) == "CPU ") {
					osname.splicer(1, 3, "iPad");
				}
				tt = osname.index(" like ");
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
		if (var(agent).index("Trident/7.0")) {
			browser = "11";
		} else if (var(agent).index("Trident/6.0")) {
			browser = "10";
		} else if (var(agent).index("Trident/5.0")) {
			browser = "9";
		} else if (var(agent).index("Trident/4.0")) {
			browser = "8";
		}

		//MSIE tells you the operating mode for MS
		//MSIE 10.0 Internet Explorer 10
		//MSIE 9.0  Internet Explorer 9
		//MSIE 8.0  Internet Explorer 8 or IE8 Compatibility View/Browser Mode
		//MSIE 7.0  Windows Internet Explorer 7 or IE7 Compatibility View/Browser Mode
		//MSIE 6.0  Microsoft Internet Explorer 6
		var iemode = "";
		if (var(agent).index("MSIE 10.0")) {
			iemode = "10";
		} else if (var(agent).index("MSIE 9.0")) {
			iemode = "9";
		} else if (var(agent).index("MSIE 8.0")) {
			iemode = "8";
		} else if (var(agent).index("MSIE 7.0")) {
			iemode = "7";
		} else if (var(agent).index("MSIE 6.0")) {
			iemode = "6";
		}

		//add the mode to the browser if different (MS only)
		if (not browser) {
			browser = iemode;
		}
		if ((browser and iemode) and iemode ne browser) {
			browser ^= " IE" ^ iemode ^ "mode";
		}
		if (browser) {
			browser = "IE" ^ browser;
		}

		if (not browser) {
			//tt=index(agent,'Chrome',1)
			var tt = var(agent).index("Edge");
			if (not tt) {
				tt = var(agent).index("Chrome");
			}
			if (not tt) {
				tt = var(agent).index("Firefox");
			}
			if (not tt) {
				tt = var(agent).index("Safari");
			}
			if (not tt) {
				tt = var(agent).index("Opera");
			}
			if (not tt) {
				tt = var(agent).index("Netscape");
			}
			if (tt) {
				browser = var(agent).substr(tt,9999).field(";", 1).field(" ", 1).field(")", 1);
			}

			//browsernames='Edge,Chrome,Firefox,Safari,Opera,Netscape'
			//for browsern=1 to 6
			// browsername=field(browsernames,',',browsern)
			// tt=index(agent,browsername,1)
			// if tt then browser:=' ':field(field(field(agent[tt,9999],';',1),' ',1),')',1)
			// next browsern
			//browser=trimf(browser)

		}
		if (browser.substr(-2,2) == ".0") {
			browser.splicer(-2, 2, "");
		}

		var submode = mode.field(".", 3);
		if (submode == "OS") {
			agent = osname;
			return 0;
		} else if (submode == "BROWSER") {
			agent = browser;
			return 0;
		}

		if (osname or browser) {
			if (osname) {
				osname ^= "; ";
			}
			if (browser) {
				osname ^= browser;
			}else{
				osname ^= agent;
			}
			//tt=osname:'<br>':agent
			agent = osname;
		}

		//returns agent in dataio see equate above

	} else {
		call mssg(mode.quote() ^ " unknown mode in HTMLLIB");
	}

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
