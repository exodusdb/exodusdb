#include <exodus/library.h>
libraryinit()

#include <sys_common.h>

var params;
var params2;
var tagsep;

function main(in mode, io dataio, in params0="", in params20="") {
	//c sys in,io,"",""

	//REPLACING UPPERCASE VERSION HTMLLIB()

	#include <system_common.h>

	var td = "<td>";
	var tdx = "</td>";
	var tr = "<tr>";
	var trx = "</tr>";

	if (params0.unassigned()) {
		params = "";
	} else {
		params = params0;
	}
	if (params20.unassigned()) {
		params2 = "";
		} else {
		params2 = params20;
	}

	if (mode eq "TABLE.MAKE") {

		dataio.trimmer();
		var tt = " " ^ VM;
		while (true) {
			///BREAK;
			if (not(dataio.contains(tt))) break;
			dataio.replacer(tt, VM);
		}//loop;
		tt = " " ^ FM;
		while (true) {
			///BREAK;
			if (not(dataio.contains(tt))) break;
			dataio.replacer(tt, FM);
		}//loop;

		//filler='&nbsp;'
		//gosub fill

		dataio.replacer(FM, "</td></tr>" ^ FM ^ "<tr><td>");
		dataio.replacer(VM, "</td>" ^ VM ^ "<td>");
		dataio = "<tr><td>" ^ dataio ^ "</td></tr>";

		if (params2) {
			var line1 = dataio.f(1);
			line1.replacer("</TR>", "</tr>");
			line1.replacer("</tr>", "</tr>" ^ FM ^ "</thead>" ^ FM ^ "<tbody>");
			line1.replacer("<td>", "<th>");
			line1.replacer("</td>", "</th>");
			line1.replacer("<td>", "<th>");
			line1.replacer("</td>", "</th>");
			if (not(line1.contains("<thead"))) {
				line1 = FM ^ "<thead>" ^ FM ^ line1;
			}
			dataio(1) = line1;
		}

		//prefix
		//tableid='T':rnd(999999999)[-7,7]
		tt = params;
		//swap '<TABLE' with '<table' in tt
		if (not params2) {
			tt ^= FM ^ "<tbody>";
		}
		dataio.paster(1, 0, tt ^ FM);

		//postfix
		dataio = dataio ^ FM ^ "</tbody>" ^ FM ^ "</table>";

		//highlight last row if a total
		var nlines = dataio.count(FM) + 1;
		var lastline = dataio.f(nlines);
	//Total
		if (lastline.contains("<td>" ^ sys.glang.f(28))) {
			lastline.replacer("TD>", "th>");
			lastline.replacer("td>", "th>");
			dataio(nlines) = lastline;
		}

		//convert fm:vm to '' in dataio
		dataio.converter(VM, "");

	//fill in the missing cells
	} else if (mode eq "TABLE.FILL") {

		var filler = "<TD>&nbsp;</TD>";
		//gosub fill

	} else if (mode eq "HASHTABLE") {

		let nv = dataio.count(VM) + 1;
		for (const var vn : range(1, nv)) {
			var datax = dataio.f(1, vn);

			//insert a break before all space+#
			datax.replacer(" #", FM ^ "#");

			//restore #+space to be NOT broken
			datax.replacer(FM ^ "# ", " # ");

			datax.converter(TM, FM);

			let nlines = datax.count(FM) + 1;
			if (nlines gt 1) {

				for (const var ln : range(1, nlines)) {
					var line = datax.f(ln).trim();
					if (line.starts("#")) {
						line.cutter(1);
						//if colon : present then before colon is the tag name
						if (line.contains(":")) {
							tagsep = ":";
						} else {
							tagsep = " ";
						}
						var tt1 = line.field(tagsep, 1);
						tt1.converter("_", " ");
						var tt2 = line.field(tagsep, 2, 9999);
						if (tt2.starts("=")) {
							tt2.cutter(1);
						}
						datax(ln) = td ^ tt1 ^ ":" ^ tdx ^ td ^ tt2 ^ tdx;
					} else {
						datax(ln) = "<td colspan=2>" ^ line ^ tdx;
					}
				} //ln;

				datax.replacer(FM, trx ^ tr);
				datax.paster(1, 0, "<table class=\"hashtable\">" ^ tr);
				datax ^= trx ^ "</table>";

				dataio(1, vn) = datax;

			}

		} //vn;

	} else if (mode eq "STRIPTAGS") {
		while (true) {
			var tag1 = dataio.index("<");
			var tag2 = dataio.index(">");
			///BREAK;
			if (not(tag1 and tag1 lt tag2)) break;
			dataio.paster(tag1, tag2 - tag1 + 1, "");
		}//loop;

	} else if (mode eq "DECODEHTML") {
		dataio.replacer("&nbsp;", " ");
		dataio.replacer("&lt;", "<");
		dataio.replacer("&gt;", ">");
		dataio.replacer("&amp;", "&");

	} else if (mode.field(".", 1, 2) eq "OCONV.AGENT") {

		//to check versus DOS BROWSERS.TXT
		//GET NEW RUN GBP2 CHK.AGENTSTRING

		//equ agent to dataio

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
		if (dataio.contains("NT 10.0")) {
			osname = "10";
		} else if (dataio.contains("NT 6.3")) {
			osname = "8.1";
		} else if (dataio.contains("NT 6.2")) {
			osname = "8";
		} else if (dataio.contains("NT 6.1")) {
			osname = "7";
		} else if (dataio.contains("NT 6.0")) {
			osname = "Vista/2008";
		} else if (dataio.contains("NT 5.2")) {
			osname = "XP-64";
		} else if (dataio.contains("NT 5.1")) {
			osname = "XP";
		} else if (dataio.contains("Windows NT ")) {
			var tt = dataio.index("Windows NT ");
			osname = ((dataio.cut(tt + 10)).field(";", 1)).field(")", 1);
		}
		if (dataio.contains("WOW64")) {
			osname ^= "-64";
		}
		if (osname) {
			osname = "Win" ^ osname;
			//add touch if Windows but not Windows Phone
			if (dataio.contains("Touch")) {
				osname ^= " Touch";
			}
		}

		if (not osname) {
			var tt = 0;
			if (not tt) {
				tt = dataio.index("Android ");
			}
			if (not tt) {
				tt = dataio.index("Android");
			}
			if (not tt) {
				tt = dataio.index("iPhone OS");
			}
			if (not tt) {
				tt = dataio.index("CPU OS");
			}
			if (not tt) {
				tt = dataio.index("Mac OS");
			}
			if (not tt) {
				tt = dataio.index("Windows Phone ");
				if (tt) {
					dataio.paster(tt, 13, "WinPhone");
				}
			}
			if (not tt) {
				tt = dataio.index("Linux");
			}
			if (not tt) {
				tt = dataio.index("CrOS");
			}
			if (tt) {
				osname = dataio.cut(tt - 1);
				if (osname.starts("CPU ")) {
					osname.paster(1, 3, "iPad");
				}
				tt = osname.index(" like ");
				if (tt) {
					osname.firster(tt - 1);
				}
				osname = osname.field(";", 1);
				osname = osname.field(")", 1);
				osname.replacer(" x86_64", "-64");
			}
		}
		if (osname.ends(".0")) {
			osname.cutter(-2);
		}

		//Trident tells you the actual browser software for MS
		var browser = "";
		if (dataio.contains("Trident/7.0")) {
			browser = "11";
		} else if (dataio.contains("Trident/6.0")) {
			browser = "10";
		} else if (dataio.contains("Trident/5.0")) {
			browser = "9";
		} else if (dataio.contains("Trident/4.0")) {
			browser = "8";
		}

		//MSIE tells you the operating mode for MS
		//MSIE 10.0 Internet Explorer 10
		//MSIE 9.0  Internet Explorer 9
		//MSIE 8.0  Internet Explorer 8 or IE8 Compatibility View/Browser Mode
		//MSIE 7.0  Windows Internet Explorer 7 or IE7 Compatibility View/Browser Mode
		//MSIE 6.0  Microsoft Internet Explorer 6
		var iemode = "";
		if (dataio.contains("MSIE 10.0")) {
			iemode = "10";
		} else if (dataio.contains("MSIE 9.0")) {
			iemode = "9";
		} else if (dataio.contains("MSIE 8.0")) {
			iemode = "8";
		} else if (dataio.contains("MSIE 7.0")) {
			iemode = "7";
		} else if (dataio.contains("MSIE 6.0")) {
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
			//tt=index(dataio,'Chrome',1)
			var tt = dataio.index("Edge");
			if (not tt) {
				tt = dataio.index("Chrome");
			}
			if (not tt) {
				tt = dataio.index("Firefox");
			}
			if (not tt) {
				tt = dataio.index("Safari");
			}
			if (not tt) {
				tt = dataio.index("Opera");
			}
			if (not tt) {
				tt = dataio.index("Netscape");
			}
			if (tt) {
				browser = dataio.cut(tt - 1).field(";", 1).field(" ", 1).field(")", 1);
			}

			//browsernames='Edge,Chrome,Firefox,Safari,Opera,Netscape'
			//for browsern=1 to 6
			// browsername=field(browsernames,',',browsern)
			// tt=index(dataio,browsername,1)
			// if tt then browser:=' ':field(field(field(dataio[tt,9999],';',1),' ',1),')',1)
			// next browsern
			//browser=trimf(browser)

		}
		if (browser.ends(".0")) {
			browser.paster(-2, 2, "");
		}

		var submode = mode.field(".", 3);
		if (submode eq "OS") {
			dataio = osname;
			return 0;
		} else if (submode eq "BROWSER") {
			dataio = browser;
			return 0;
		}

		if (osname or browser) {
			if (osname) {
				osname ^= "; ";
			}
			if (browser) {
				osname ^= browser;
			} else {
				osname ^= dataio;
			}
			//tt=osname:'<br>':dataio
			dataio = osname;
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
