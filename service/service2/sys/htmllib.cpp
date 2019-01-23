#include <exodus/library.h>
libraryinit()


#include <gen.h>

var params;
var params2;
var filler;

function main(in mode, io datax, in params0="", in params20="") {
	//c sys in,io,"",""

	//BEING REPLACED BY LOWERCASE VERSION HTMLLIB2()
	//GBP all HTMLLIB2
	//BP mostly HTMLLIB still
	//ABP neither used

	//jbase
	//global datax

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
		datax.trimmer();
		var tt = " " ^ VM;
		while (true) {
		///BREAK;
		if (not(datax.index(tt))) break;;
			datax.swapper(tt, VM);
		}//loop;
		tt = " " ^ FM;
		while (true) {
		///BREAK;
		if (not(datax.index(tt))) break;;
			datax.swapper(tt, FM);
		}//loop;
		filler = "&nbsp;";
		gosub fill( datax);

		datax.swapper(FM, "</TD></TR>" ^ FM ^ "<TR><TD>");
		datax.swapper(VM, "</TD>" ^ VM ^ "<TD>");
		datax = "<TR><TD>" ^ datax ^ "</TD></TR>";

		//trth='<TR><TD>'
		if (params2) {
			//trth='<TH>'
			var line1 = datax.a(1);
			line1.swapper("</TR>", "</tr>");
			line1.swapper("</tr>", "</TR></THEAD><TBODY>");
			line1.swapper("<TD>", "<th>");
			line1.swapper("</TD>", "</th>");
			line1.swapper("<td>", "<th>");
			line1.swapper("</td>", "</th>");
			datax.r(1, line1);
		}

		//prefix
		//tableid='T':rnd(999999999)[-7,7]
		params2 = params;
		//if params2 else params2='<TABLE><THEAD>'
		params2.swapper("<TABLE", "<table");
		//swap '<table' with '<table id=':tableid in params2
		datax.splicer(1, 0, params2 ^ "<TBODY>");

		//postfix
		datax ^= "</TBODY></TABLE>";
		//datax=datax:'</TBODY></TABLE ID=':tableid:'>'

		//if not(count(datax,fm)) and index(datax<1>,'<TH>',1) then
		// swap '</TD>' with '</TH>' in datax
		// end

		//highlight last row if a total
		var nlines = datax.count(FM) + 1;
		var lastline = datax.a(nlines);
	//Total
		if (lastline.index("<TD>" ^ gen.glang.a(28))) {
			lastline.swapper("TD>", "TH>");
			lastline.swapper("td>", "th>");
			datax.r(nlines, lastline);
		}

		datax.converter(FM ^ VM, "");

	//fill in the missing cells
	} else if (mode == "TABLE.FILL") {
		filler = "<TD>&nbsp;</TD>";
		gosub fill( datax);

	} else if (mode == "HASHTABLE") {

		var nv = datax.count(VM) + 1;
		for (var vn = 1; vn <= nv; ++vn) {
			var data2 = datax.a(1, vn);

			//insert a break before all space+#
			data2.swapper(" #", FM ^ "#");

			//restore #+space to be NOT broken
			data2.swapper(FM ^ "# ", " # ");

			data2.converter(TM, FM);

			var nlines = data2.count(FM) + 1;
			if (nlines > 1) {

				for (var ln = 1; ln <= nlines; ++ln) {
					var line = data2.a(ln).trim();
					if (line[1] == "#") {
						line.splicer(1, 1, "");
						var tt = line.field(" ", 2, 9999);
						if (tt[1] == "=") {
							tt.splicer(1, 1, "");
						}
						data2.r(ln, "<td>" ^ line.field(" ", 1) ^ "</td>" "<td>" ^ tt ^ "</td>");
					}else{
						data2.r(ln, "<td colspan=2>" ^ line ^ "</td>");
					}
				};//ln;

				data2.swapper(FM, "</tr>" "<tr xvalign=top>");
				data2.splicer(1, 0, "<table class=\"hashtable\">" "<tr xvalign=top>");
				data2 ^= "</tr>" "</table>";

				datax.r(1, vn, data2);

			}

		};//vn;

	} else if (mode == "STRIPTAGS") {
		while (true) {
			var tag1 = datax.index("<");
			var tag2 = datax.index(">");
		///BREAK;
		if (not(tag1 and (tag1 < tag2))) break;;
			datax.splicer(tag1, tag2 - tag1 + 1, "");
		}//loop;

	} else if (mode == "DECODEHTML") {
		datax.swapper("&nbsp;", " ");
		datax.swapper("&lt;", "<");
		datax.swapper("&gt;", ">");
		datax.swapper("&amp;", "&");

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
		if (datax.index("NT 10.0")) {
			osname = "10";
		} else if (datax.index("NT 6.3")) {
			osname = "8.1";
		} else if (datax.index("NT 6.2")) {
			osname = "8";
		} else if (datax.index("NT 6.1")) {
			osname = "7";
		} else if (datax.index("NT 6.0")) {
			osname = "Vista/2008";
		} else if (datax.index("NT 5.2")) {
			osname = "XP-64";
		} else if (datax.index("NT 5.1")) {
			osname = "XP";
		} else if (datax.index("Windows NT ")) {
			var tt = datax.index("Windows NT ");
			osname = ((datax.substr(tt + 11,9999)).field(";", 1)).field(")", 1);
		}
		if (datax.index("WOW64")) {
			osname ^= "-64";
		}
		if (osname) {
			osname = "Win" ^ osname;
			//add touch if Windows but not Windows Phone
			if (datax.index("Touch")) {
				osname ^= " Touch";
			}
		}

		if (not osname) {
			var tt = 0;
			if (not tt) {
				tt = datax.index("Android ");
			}
			if (not tt) {
				tt = datax.index("Android");
			}
			if (not tt) {
				tt = datax.index("iPhone OS");
			}
			if (not tt) {
				tt = datax.index("CPU OS");
			}
			if (not tt) {
				tt = datax.index("Mac OS");
			}
			if (not tt) {
				tt = datax.index("Windows Phone ");
				if (tt) {
					datax.splicer(tt, 13, "WinPhone");
				}
			}
			if (not tt) {
				tt = datax.index("Linux");
			}
			if (not tt) {
				tt = datax.index("CrOS");
			}
			if (tt) {
				osname = datax.substr(tt,9999);
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
		if (datax.index("Trident/7.0")) {
			browser = "11";
		} else if (datax.index("Trident/6.0")) {
			browser = "10";
		} else if (datax.index("Trident/5.0")) {
			browser = "9";
		} else if (datax.index("Trident/4.0")) {
			browser = "8";
		}
//L1363:
		//MSIE tells you the operating mode for MS
		//MSIE 10.0 Internet Explorer 10
		//MSIE 9.0 Internet Explorer 9
		//MSIE 8.0 Internet Explorer 8 or IE8 Compatibility View/Browser Mode
		//MSIE 7.0 Windows Internet Explorer 7 or IE7 Compatibility View/Browser Mode
		//MSIE 6.0 Microsoft Internet Explorer 6
		var iemode = "";
		if (datax.index("MSIE 10.0")) {
			iemode = "10";
		} else if (datax.index("MSIE 9.0")) {
			iemode = "9";
		} else if (datax.index("MSIE 8.0")) {
			iemode = "8";
		} else if (datax.index("MSIE 7.0")) {
			iemode = "7";
		} else if (datax.index("MSIE 6.0")) {
			iemode = "6";
		}
//L1472:
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
			var tt = datax.index("Chrome");
			if (not tt) {
				tt = datax.index("Firefox");
			}
			if (not tt) {
				tt = datax.index("Safari");
			}
			if (not tt) {
				tt = datax.index("Opera");
			}
			if (not tt) {
				tt = datax.index("Netscape");
			}
			if (tt) {
				browser = datax.substr(tt,9999).field(";", 1).field(" ", 1).field(")", 1);
			}
		}
		if (browser.substr(-2,2) == ".0") {
			browser.splicer(-2, 2, "");
		}

		var submode = mode.field(".", 3);
		if (submode == "OS") {
			datax = osname;
			return 0;
		} else if (submode == "BROWSER") {
			datax = browser;
			return 0;
		}
//L1687:
		if (osname or browser) {
			if (osname) {
				osname ^= "; ";
			}
			if (browser) {
				osname ^= browser;
			}else{
				osname ^= datax;
			}
			//tt=osname:'<br>':agent
			datax = osname;
		}

		//returns agent in datax see equate above

	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " unknown mode in HTMLLIB");
	}
//L1771:
	return 0;

}

subroutine fill(io datax) {
	//fill(io datax)
	var nn = datax.count(FM) + (datax ne "");

	//find max number of columns
	var n2 = 0;
	for (var ii = 1; ii <= nn; ++ii) {
		var tt = datax.a(ii).count(VM) + 1;
		if (tt > n2) {
			n2 = tt;
		}
	};//ii;

	//make sure all columns are filled
	for (var ii = 1; ii <= nn; ++ii) {
		for (var i2 = 1; i2 <= n2; ++i2) {
			if (datax.a(ii, i2) == "") {
				datax.r(ii, i2, filler);
			}
		};//i2;
	};//ii;

	return;

}


libraryexit()
