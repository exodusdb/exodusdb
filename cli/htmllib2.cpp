#ifndef EXO_MODULE
#	include <stack>
#	include <vector> // Actually already comes with dim.h
#	include <map>    // Actually already comes with ?
#endif
#include <cassert>

#include <exodus/library.h>
libraryinit()

var params;
var params2;
var tagsep;
var quiet = true;

//Note that the argument names are not accurate for GETCSS, GETSORTJS, COLROWSPAN, CONVCSS, ADDUNITS, DOCMODS, GETMARK modes
func main(in mode, io dataio, in params0 = "", in params20 = "", in glang = "") {

	// First for speed since called for every column and row
	if (mode == "ADDUNITS") {

		// Add/merge aa into bb

		//wire up accurate names to the given parameters
		var& bb = dataio;
		let& aa = params0;
		let& sep = params20;

		// Note aa and bb are switched in order only
		gosub addunits(aa, bb, sep);

		return 0;
	}

	//REPLACING UPPERCASE VERSION HTMLLIB()

	let td = "<td>";
	let tdx = "</td>";
	let tr = "<tr>";
	let trx = "</tr>";

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

	if (mode == "TABLE.MAKE") {

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
		dataio.prefixer(tt ^ FM);

		//postfix
		dataio ^= FM ^ "</tbody>" ^ FM ^ "</table>";

		//highlight last row if a total
		let nlines = dataio.count(FM) + 1;
		var lastline = dataio.f(nlines);
	//Total
		//if (lastline.contains("<td>" ^ sys.glang.f(28))) {
		if (lastline.contains("<td>" ^ glang.f(28))) {
			lastline.replacer("TD>", "th>");
			lastline.replacer("td>", "th>");
			dataio(nlines) = lastline;
		}

		//convert fm:vm to '' in dataio
		dataio.converter(VM, "");

	//fill in the missing cells
	} else if (mode == "TABLE.FILL") {

		let filler = "<TD>&nbsp;</TD>";
		//gosub fill

	} else if (mode == "HASHTABLE") {

		let nv = dataio.count(VM) + 1;
		for (const var vn : range(1, nv)) {
			var datax = dataio.f(1, vn);

			//insert a break before all space+#
			datax.replacer(" #", FM ^ "#");

			//restore #+space to be NOT broken
			datax.replacer(FM ^ "# ", " # ");

			datax.converter(TM, FM);

			let nlines = datax.count(FM) + 1;
			if (nlines > 1) {

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
				datax.prefixer("<table class=\"hashtable\">" ^ tr);
				datax ^= trx ^ "</table>";

				dataio(1, vn) = datax;

			}

		} //vn;

	} else if (mode == "STRIPTAGS") {
		while (true) {
			let tag1 = dataio.index("<");
			let tag2 = dataio.index(">");
			///BREAK;
			if (not(tag1 and tag1 < tag2)) break;
			dataio.paster(tag1, tag2 - tag1 + 1, "");
		}//loop;

	} else if (mode == "DECODEHTML") {
		dataio.replacer("&nbsp;", " ");
		dataio.replacer("&lt;", "<");
		dataio.replacer("&gt;", ">");
		dataio.replacer("&amp;", "&");

	} else if (mode.field(".", 1, 2) == "OCONV.AGENT") {

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
			let tt = dataio.index("Windows NT ");
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
		if ((browser and iemode) and iemode != browser) {
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
			//browser=trimfirst(browser)

		}
		if (browser.ends(".0")) {
			browser.cutter(-2);
		}

		let submode = mode.field(".", 3);
		if (submode == "OS") {
			dataio = osname;
			return 0;
		} else if (submode == "BROWSER") {
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

	} else if (mode == "GETSORTJS") {

		//in c++ get from raw text string below
		dataio = getvogonpoetry_sortjs();

		gosub replace_literal_comparison_operators(dataio);

		dataio.replacer(FM, "\n");
		dataio.replacer(FM, "\n");

		var jsdatefmt;
		if (DATEFMT.contains("E")) {
			jsdatefmt = "d/M/yyyy";
		} else if (DATEFMT.contains("J")) {
			jsdatefmt = "yyyy/M/d";
		} else {
			jsdatefmt = "M/d/yyyy";
		}

		dataio.replacer("gdateformat='d/M/yyyy'", "gdateformat='" ^ jsdatefmt ^ "'");

	} else if (mode == "GETCSS") {

		//wire up accurate names to the given parameters
		let& version = params0;
		let& stationery = params20;

		gosub getcss(dataio, version, stationery);

	} else if (mode == "COLROWSPAN") {

		//wire up accurate names to the given parameters
		var& colhdg = dataio;
		let& thproperties = params0;
		let& nobase0 = params20;
		let& basecurrcode = glang;

		gosub colrowspan(colhdg, thproperties, nobase0, basecurrcode);

	} else if (mode == "CONVCSS") {

		//wire up accurate names to the given parameters
		var& outx = dataio;
		let& tableid = params0;
		let& in0 = params20;

		gosub convcss(outx, tableid, in0);

	} else if (mode == "DOCMODS") {

		let& stationery = params20;
		gosub docmods(dataio, stationery);

	} else if (mode == "GETMARK") {

		//wire up accurate names to the given parameters
		var& mark = dataio;
		let& getmarkmode = params0;
		let& html = params20;

		gosub getmark(getmarkmode, html, mark);

	} else if (mode == "T2H") {
		// Convert Exodus comment text (pseudo markdown) to html
        let& text_in = params0;
		return t2h_main(text_in);

	} else if (mode == "H2M") {
		// Convert html to man page format
        let& text_in = params0;
        var& exit_status = dataio;
		return h2m_main(text_in, exit_status);

	} else if (mode == "HTMLTIDY") {
		// Properly indent html
		// 0 Success
		// 1 Excessive indent
		// 2 Excessive undent
		let& html_in = params0;
		var& html_out = dataio;
		var messages;
		return html_tidy_main(html_in, html_out, messages);

	} else if (mode == "BOX2TABLE") {
		dataio = boxdrawing_to_html(dataio);
		return 0;

	} else {
		call note(mode.quote() ^ " unknown mode in HTMLLIB2");
	}

///////
//exit:
///////
	return 0;
}

func getvogonpoetry_sortjs() {

	return
R"V0G0N(
<script type="text/javascript">
gdateformat='d/M/yyyy'
function sorttable(event)
{
 if (document.body.getAttribute('contenteditable'))
  return true
 //locate the current element
 event=event||window.event
 event.target=event.target||event.srcElement
 var th=event.target
 if (th.tagName!="TH") return(0)
 //var rows=th.parentElement.parentElement.parentElement.rows
 var rows=th.parentElement.parentElement.parentElement.tBodies[0].getElementsByTagName('tr')
 var coln=th.cellIndex
 var rown=th.parentElement.rowIndex
 var nrows=rows.length
 fromrown=0
 uptorown=nrows-=1
 /*
 var rown=th.parentElement.rowIndex
 //locate the first td row
 var fromrown=rown

 //search forwards (incl current row) for the first row with a td element
 //if not found then rown=nrows+1
 while (fromrown<nrows && ( rows[fromrown].cells.length<(coln+1) || rows[fromrown].cells[coln].tagName=="TH" ) )
  ++fromrownn

 //include prior td rows
 while (fromrown>0 && rows[fromrown-1].cells.length>coln && rows[fromrown-1].cells[coln].tagName=="TD")
  --fromrown

 //include following td rows
 var uptorown=(rown>fromrown)?rown:fromrown
 */
 while ((uptorown<nrows) && rows[uptorown+1] && (rowchildNodes=rows[uptorown+1].cells) && (rowchildNodes.length>coln) && (rowchildNodes[coln].tagName=="TD"))
  uptorown++

 if (gdateformat == 'M/d/yyyy')
  var dateformat=[2,0,1]
 else if (gdateformat == 'yyyy/M/d')
  var dateformat=[0,1,2]
 else //gdateformat = 'd/M/yyyy'
  var dateformat=[2,1,0]
 var yy=dateformat[0]+1
 var mm=dateformat[1]+1
 var dd=dateformat[2]+1

 var dateregex=/ ?(\d{1,2})\/ ?(\d{1,2})\/(\d{4}|\d{2})/
 var periodregex=/ ?(\d{1,2})\/(\d{4})/g
 for ( var ii=fromrown;ii<=uptorown;++ii){
  var cell=rows[ii].cells[coln]
  if (cell.getAttribute('sortvalue'))
   break
  var value=(cell.textContent||cell.innerText||"").toUpperCase()
  var match
  while(match=value.match(dateregex)) {
   //convert dates like n/n/yy or n/n/yyyy to sortable yyyy|mm|dd format
   value=value.replace(dateregex,('0000'+match[yy]).slice(-4)+'|'+('00'+match[mm]).slice(-2)+'|'+('00'+match[dd]).slice(-2))
   //console.log(value)
  }
  value=value.replace(periodregex,'$2|$1')
  value=value.replace(/([-+]?[1234567890.,]+)([A-Z]{2,3})/g,'$2$1')//prefix currency
  //natural sort
  //value=value.replace(/[-+]?[1234567890.,]+/g,function(x){if (x.slice(0,1)=='-') {y='';x=(100000000000000000000+Number(x.replace(/,/g,''))).toString();alert(x)} else y='';return y+('00000000000000000000000'+x).slice(-20)})
  value=value.replace(
   /[-+]?[1234567890.,]+/g,
   function(x){
     /*decimal*/ x=x.replace(/,/g,'');
     /*comma*/   //x=x.replace(/./g,'').replace(/,/,'.');
     if (x.slice(0,1)=='-') {
       y='-';
       x=(999999999999.999+Number(x.replace(/,/g,''))).toString();
     } else {
       y='';
     }
     x=x.split('.');
     y+=('00000000000000000000'+x[0]).slice(-20);
     if (x[1])
      y+='.'+(x[1]+'0000000000').slice(0,10);
     return y;
   }
  )
  value+=('000000000000'+ii).slice(-10)//stable sort
  cell.setAttribute('sortvalue',value)
  //cell.innerHTML=value
  //console.log(ii+' '+value)
 }
 QuickSort(rows,coln,fromrown,uptorown);
}

function QuickSort(rows,coln,min,max){

 if (max<=min) return(true)

 var low = min;
 var high = max;
 var mid=rows[Math.floor((low+high)/2)].cells[coln].getAttribute('sortvalue');

 do {
  while (rows[low].cells[coln].getAttribute('sortvalue') < mid) low++;
  while (rows[high].cells[coln].getAttribute('sortvalue') > mid) high--;

  if (low <= high) {

   //if (stopped) return;

   rows[low].swapNode(rows[high]);

   low++;
   high--;
  }

  //pauseSort();

 } while (low <= high);

 if (high > min) QuickSort(rows, coln, min, high);
 if (low < max) QuickSort(rows, coln, low, max);
}
if (!document.swapNode) {
 Node.prototype.swapNode = function (node) {
  var p = node.parentNode;
  var s = node.nextSibling;
  this.parentNode.replaceChild(node, this);
  p.insertBefore(this, s);
  return this;
 }
}
</script>
)V0G0N";

}

func getcss(io css, in version = "", in stationery = "") {

	//NB quirk in ie mimiced by mozilla table dont inherit size
	//but work around is TABLE {FONT-SIZE:100%} in css

	//prevents IE later versions from working in standards mode
	// <meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
	// <meta http-equiv="x-ua-compatible" content="IE=6" />

	let charset = SYSTEM.f(127);
	if (charset) {
		css = "<meta http-equiv=\"content-type\" content=\"text/html;charset=" ^ charset ^ "\" />\n";
	} else {
		css = "<!-- no " ^ SYSTEM.f(17) ^ " charset found -->\n";
	}

	// stationery option
	// 1 - No letterhead
	// 2 - Letterhead
	// P - pdf Portrait
	// L - pdf Landsdape
	// If converting to pdf using chromium, then add css to configure paper size A4, orientation and header/footer
	// since they cannot be configured on command line as for the obsolete wkhtmltopdf converter
	// and using the chromium scripting interface requires the installation of complex dependencies
	// Test for chromium is duplicated in convpdf.cpp and htmllib2.cpp

	let pdfcmd = "chromium";
	if (stationery.assigned() and stationery > 2 and osshell("which " ^ pdfcmd ^ " > /dev/null")) {

		// Override and use below css properties when printing or converting to pdf
		css ^= "<style type=\"text/css\" media=\"print\">\n";
		css ^= "	@page {\n";

		// Force A4
		css ^= "		size: A4";
		// Landscape/Portrait
		if (stationery == "L") {
			css ^= " landscape;\n";
		} else {
			css ^= " portrait;\n";
		}

		// Suppress top left, center and right headings
		css ^= "		@top-left {\n";
		css ^= "			content: \"\";\n";
		css ^= "		}\n";

		// Bottom left show page n of N as "9/99"
		css ^= "		@bottom-left {\n";
		css ^= "			content: counter(page) \"/\" counter(pages);\n";
		css ^= "		}\n";
		css ^= "	}\n";

		// Resize chromium default size to match old wkhtml2pdf
		// nb printjob.cpp overrides this zoom
		css ^= "    body {zoom: 85% !important;}\n";

		css ^= "</style>\n";
	}

	css ^= getvogonpoetry_css(version);

	gosub replace_literal_comparison_operators(css);

	// get system config report font, font size and colour
	var font = SYSTEM.f(46, 3);
	if (font == "Default") {
		font = "";
	}
	if (font) {
		font ^= ",";
	}
	var fontsize = SYSTEM.f(46, 8);
	if (not fontsize) {
		fontsize = 100;
	}
	if (fontsize and fontsize.isnum()) {
		fontsize ^= "%";
	}

	let thcolor = SYSTEM.f(46, 1);
	let tdcolor = SYSTEM.f(46, 2);

	if (thcolor) {
		//css.replacer("#ffff80", "%thcol%");
		css.replacer("#fff099", "%thcol%");
	}
	if (tdcolor) {
		//css.replacer("#ffffc0", "%tdcol%");
		css.replacer("#fdf5e6", "%tdcol%");
	}
	if (thcolor) {
		css.replacer("%thcol%", thcolor);
	}
	if (tdcolor) {
		css.replacer("%tdcol%", tdcolor);
	}

	css.replacer("exodusfont,", font);
	css.replacer("exodussize", fontsize);

	let agent = SYSTEM.f(40);
	if (agent.contains("MSIE 7")) {
		css.replacer("xborder-collapse", "border-collapse");
	}

	return 0;

}

func getvogonpoetry_css(in version) {

	if (version.assigned() and version == 2) {

		return
R"V0G0N(
<style type="text/css">
/*ver2 for reports e.g list of invoices*/
.BHEAD {display:none}
.BHEAD2 {}
body {background-color:#ffffff; font-family:exodusfont,verdana,sans-serif,arial,helvetica; font-size: exodussize}
table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}
a {color:blue;}
a:visited {color:purple;}
a:hover {color:red;}
tt {font-family:courier new,courier;font-size:80%}
//xth {background-color:#ffff80;}
xth {background-color:#fff099;}
thead {display:table-header-group}
tfoot {page-break-inside:avoid}
td.nb {border-bottom:none}
td.nt {border-top:none}
td.nx {border-top:none;border-bottom:none}
.arightnowrap {text-align:right;white-space:nowrap}
.aright {text-align:right}
.aleft {text-align:left}
.acenter {text-align:center}
.nobr {white-space:nowrap;}
.bold {font-weight:bold;}
.vbottom {vertical-align:bottom}
.num {text-align:right;mso-number-format:General;white-space:nowrap;}

table.exodustable {
 margin:0px;
 padding:0px;
 //border-spacing:0px;
 border-collapse:collapse;}

table.exodustable th {
 //background-color:#ffff80;
 background-color:#fff099;
 padding:2px;
 border:1px solid #808080;}

@media screen{
 table.exodustable > thead th {
  top: 0;
  z-index: 2;
  position: sticky;
  outline: 1px solid #808080;
  outline-offset: -0.5px;
  box-shadow: 0 2px 2px rgba(0, 0, 0, 0.2);}}

table.exodustable > tbody > tr > td {
 //background-color:#ffffc0;
 background-color:#fdf5e6;
 padding:2px;
 border:1px solid #d3d3d3;}

table.exodustable > tbody > tr > td:first-child {
 border-left:1px solid #808080}

table.exodustable > tbody > tr > td:last-child {
 border-right:1px solid #808080}

table.exodustable hr {height:0px; border:0px; border-top:1px solid #d3d3d3}

table.hashtable td {padding:0px; margin:0px; text-align:left; vertical-align:top; border-collapse:collapse; border:0px;}

.pagedivider {border-top:1px dashed #808080; border-bottom:0px;}

</style>

<style type="text/css" media="print">
 /*.exodustable {}*/
 /* #Header, #Footer { display: none !important; } */
 .noprint {display:none}
</style>

<script type="text/javascript">
var togglendisplayed=0
function toggle(t, mode) {

	if (typeof t == 'string') {
		if (document.getElementsByClassName)
			t = document.getElementsByClassName(t)
		else
			t = document.getElementsByName(t)
	}

	if (t.tagName) t=[t]
	if (!t.length) return

	var display = "none"
	for (var ii = t.length - 1; ii>=0; ii--) {
		if (t[ii].style.display == "") {
			t[ii].style.display = "none"
		} else {
			t[ii].style.display = ""
			display = ""
		}
	}

	if (!mode)  {
		var toggleheading=false
		if (display == '') {
			togglendisplayed++
			if (togglendisplayed == 1)
				toggleheading = true

		} else {
			togglendisplayed--
			if (togglendisplayed == 0)
				toggleheading = true
		}
		if (toggleheading) {
			//toggle(document.getElementsByName('BHEAD'),true)
			var rules=document.styleSheets[0].cssRules||document.styleSheets[0].rules
			var bheads=[rules[0],rules[1]]
			toggle(bheads,true)
		}
	}
}

window.onload=function (){
 if (!window.opener)
  return
 //ctrl+click doesnt provide window.opener
 //glogincode="DEVDTEST*EXODUS*"
 //gwindowopenparameters={}
 //glogincode=window.opener.glogincode
 //gwindowopenparameters.readonlymode=true
 var links=document.getElementsByTagName('a')
 var vhtm
 for ( var ii=0;ii<links.length;++ii){
  var href=links[ii].href.toString()
  if (href.indexOf('nwin')<0)
   continue
  href=href.toString().split("'")
  if (href[3]=='V')
   href[3]='finance/vouchers.htm'
  if (!vhtm) {
   vhtm=window.opener.location.toString().split("/")
   vhtm.pop()
   if (href[3].indexOf('/')>=0)
    vhtm.pop()
   vhtm=vhtm.join('/')
  }
  links[ii].href=vhtm+'/'+href[3]+'?key='+href[1]+'&openreadonly=true'
  links[ii].target='_blank'
 }
}
</script>
)V0G0N";


	} else {

		return
R"V0G0N(
<style type="text/css">
/*ver1 for documents like invoices*/
.BHEAD {display:none}
.BHEAD2 {}
body {background-color:#ffffff; font-family:exodusfont,verdana,sans-serif,arial,helvetica; font-size: exodussize}
table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}
a {color:blue;}
a:visited {color:purple;}
a:hover {color:red;}
tt {font-family:courier new,courier;font-size:80%}
//th {background-color:#ffff80;}
th {background-color:#fff099;}
thead {display:table-header-group}
td.nb {border-bottom:none}
td.nt {border-top:none}
td.nx {border-top:none;border-bottom:none}
.aright {text-align:right}
.aleft {text-align:left}
.acenter {text-align:center}
.nobr {white-space:nowrap;}
.num {text-align:right;mso-number-format:General}
//.exodustable {background-color:#ffffc0; border-width:2px; border-collapse:collapse; padding:1px}
.exodustable {background-color:#fdf5e6; border-width:2px; border-collapse:collapse; padding:1px}
.hashtable td {padding:0px; margin:0px; vertical-align:top; border-collapse:collapse; border:0px solid lightgrey;}
.pagedivider {border-top:1px dashed #808080; border-bottom:0px;}
</style>

<style type="text/css" media="print">
.exodustable {}
.noprint {display:none}
</style>

<script type="text/javascript">
var togglendisplayed=0
function toggle(t, mode) {

    if (typeof t == 'string') {
        if (document.getElementsByClassName)
            t = document.getElementsByClassName(t)
        else
            t = document.getElementsByName(t)
    }

    if (t.tagName) t=[t]
    if (!t.length) return

    var display = "none"
    for (var ii = t.length - 1; ii>=0; ii--) {
        if (t[ii].style.display == "") {
            t[ii].style.display = "none"
        } else {
            t[ii].style.display = ""
            display = ""
        }
    }

    if (!mode)  {
        var toggleheading=false
        if (display == '') {
            togglendisplayed++
            if (togglendisplayed == 1)
                toggleheading = true

        } else {
            togglendisplayed--
            if (togglendisplayed == 0)
                toggleheading = true
        }
        if (toggleheading) {
            //toggle(document.getElementsByName('BHEAD'),true)
            var rules=document.styleSheets[0].cssRules||document.styleSheets[0].rules
            var bheads=[rules[0],rules[1]]
            toggle(bheads,true)
        }
    }
}
</script>
)V0G0N";

	}
}

subr colrowspan(io colhdg, in thproperties, in nobase0, in basecurrcode) {

	//called from nlist, adxtab


	//given fm list of column heading and vm per row of column heading
	//return the same but converted to tagged something like <th>xx</th>
	//but with rowspan= and colspan= inserted to span any duplicates

	//thproperties can be fm list otherwise one is used for all

	var nobase;//num
	if (nobase0.unassigned()) {
		nobase = 0;
	} else {
		nobase = nobase0;
	}

	//determine ncols
	let ncols2 = colhdg.count(FM) + 1;

	//determine nrows
	var nrows = 1;
	for (const var coln : range(1, ncols2)) {
		let tt = colhdg.f(coln).count(VM) + 1;
		if (tt > nrows) {
			nrows = tt;
		}
	} //coln;

	//var t2;
	var colspan;//num
	var rowspan;//num

	var thprop = thproperties;
	let nocell = "%~%";
	// Additional space to defeat convsyntax until clanf-format
	for (var rown = 1; rown <= nrows; rown++) {

		// Additional space to defeat convsyntax until clang-format
		for (var coln = 1; coln <= ncols2; coln++) {
		// Reverted because we do need to skip coln's
		//for (const var coln : range(1, ncols2)) {

			let tt = colhdg.f(coln, rown);
			if (tt == nocell) {
				continue;
			}

			//t2='<th style="background-color:':thcolor:'"'
			if (thproperties.contains(FM)) {
				thprop = thproperties.f(coln);
			}
			var t2 = "\n" " <th " ^ thprop;

			colspan = 1;
			rowspan = 1;

			//determine any colspan (duplicate cells to the right)
			while (true) {
				let coln2 = coln + colspan;
				let t3 = colhdg.f(coln2, rown);
				///BREAK;
				if (not((coln2 <= ncols2 and t3 == tt) and t3 != nocell)) break;
				colspan += 1;
				//colhdg(coln2, rown) = nocell;
				updater(colhdg, coln2, rown, nocell);
			}//loop;

			//if usecols else t:=coldict(coln)<14>
			if (colspan > 1) {
				t2 ^= " colspan=" ^ colspan ^ " align=center";

			} else {

				//determine any rowspan (duplicate cells below)
				while (true) {
					let rown2 = rown + rowspan;
					let t3 = colhdg.f(coln, rown2);
					///BREAK;
					if (not((rown2 <= nrows and ((t3 == tt or t3 == ""))) and t3 != nocell)) break;
					rowspan += 1;
					//colhdg(coln, rown2) = nocell;
					updater(colhdg, coln, rown2, nocell);
				}//loop;

				if (rowspan > 1) {
					t2 ^= " rowspan=" ^ rowspan;
				}
			}

			t2 ^= ">" ^ tt ^ "</th>";
			//colhdg(coln, rown) = t2;
			updater(colhdg, coln, rown, t2);

			coln += colspan - 1;

		} //coln;

	} //rown;
	colhdg.replacer(nocell, "");

	colhdg = invertarray(colhdg);
	colhdg.replacer(FM, "</tr><tr>");
	colhdg.replacer(VM, "");
	colhdg = "<tr>" ^ colhdg ^ "</tr>";

	//change all "(Base)" in dictionary column headings to the base currency
	//unless the keyword NO-BASE is present in which case replace with blank
	//this is useful if all the columns are base and no need to see the currency
	//var basecurrcode = sys.company.f(3);
	//t2 = sys.company.f(3);
	var t2 = basecurrcode;
	if (t2) {
		if (nobase) {
			t2 = "";
		} else {
			t2 = "(" ^ t2 ^ ")";
		}
		colhdg.replacer("(Base)", t2);
		colhdg.replacer("%BASE%", basecurrcode);
	}
	return;
}

subr convcss(out outx, in tableid, in in0) {

	//convert <COL ALIGN=RIGHT> to nth-child style

	let inp = in0.trim().lcase();

	outx = "\n<style type=\"text/css\">\n";
	var align = "left";

	let ncols = inp.count(VM) + 1;
	for (const var coln : range(1, ncols)) {
		let tt = inp.f(1, coln);

		if (tt.contains("left")) {
			align = "left";
		}
		if (tt.contains("right")) {
			align = "right";
		}
		if (tt.contains("center")) {
			align = "center";
		}

		var otherstyle = tt.field(DQ, 2);
		otherstyle.converter("{}", "");
		otherstyle.trimmer();
		if (not otherstyle.starts(";")) {
			otherstyle.prefixer(";");
		}
		if (not otherstyle.ends(";")) {
			otherstyle ^= ";";
		}

		//> means dont inherit to subtables
		//td
		outx ^= "table#" ^ tableid ^ " > tbody > tr > td:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}\n";
		//th
		outx ^= "table#" ^ tableid ^ " > tbody > tr > th:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}\n";
	} //coln;

	outx ^= "</style>\n";
	return;
}

subr addunits(in a0, io bb, in sep) {

	//BP  ADXTAB  sm
	//BP  ANALSCH vm
	//GBP NLIST   vm

	if (a0 == "") {
		return;
	}
	//add a to b
	//a and b can be a mv or sv list of amounts with unit codes eg 200STG]300YEN]100USD
	//b must be ascii alphabetic order

	//quick calc and exit if both are plain numeric
	if (a0.isnum() and bb.isnum()) {
		bb += a0;
		return;
	}

	var aa = a0;

	//work as if vms
	if (sep == SM) {
		aa.converter(SM, VM);
		bb.converter(SM, VM);
	}

	var acode;
	let na = aa.fcount(VM);
	for (const var an : range(1, na)) {

		let anum = amountunit(aa.f(1, an), acode);
		var bcode = "";

		let nb = bb.fcount(VM);
		//assist ADECOM c++ decompiler to lift bn out of loop
		//bn=1
		//for bn=1 to nb
		var bn = 0;
		while (true) {
			bn += 1;
			///BREAK;
			if (not(bn <= nb)) break;
			let bnum = amountunit(bb.f(1, bn), bcode);
			//call msg(na:' ':nb:' ':an:' ':bn:' ':acode:' ':bcode)

			if (bcode == acode) {
				if (bnum.len() or anum.len()) {
					var ndecs = anum.field(".", 2).len();
					let bndecs = bnum.field(".", 2).len();
					if (bndecs > ndecs) {
						ndecs = bndecs;
					}
					if (anum.isnum() and bnum.isnum()) {
						updater(bb, 1, bn, (bnum + anum).oconv("MD" ^ ndecs ^ "0P") ^ acode);
					}
				} else {
					updater(bb, 1, bn, acode);
				}
				break;
			}

			//could be faster if input was guaranteed to be in order
			//until bcode>=acode

			//next bn
		}//loop;

		//if bcode<>acode and len(anum) then
		if (bcode != acode) {
			bb.inserter(1, bn, anum ^ acode);
		}

	} //an;

	//work as if vms
	if (sep == SM) {
		bb.converter(VM, SM);
	}

	return;
}

subr docmods(io tx, in stationery) {

	//html2pdf.exe messes up repeating headers on continuation pages
	//so turn thead into additional tbody
	if (not(stationery.isnum())) {

		//swap 'THEAD>' with 'thead>' in tx
		//swap 'TBODY>' with 'body>' in tx
		//swap '<thead>' with '' in tx
		//swap '<tbody>' with '' in tx
		//swap '</thead>' with '' in tx
		//swap '</tbody>' with '' in tx

		tx.replacer("<THEAD", "<tbody");
		tx.replacer("</THEAD", "</tbody");
		tx.replacer("<thead", "<tbody");
		tx.replacer("</thead", "</tbody");

	}

	return;
}

func getmark(in mode, in html, io mark) {
	//eg
	//call getmark('CLIENT',1,clientmark)
	//call getmark('OWN',1,ownmark)

	if (mark.unassigned()) {
		mark = "";
	}

var link;

	if (mode == "OWN") {

		if (mark) {
			mark ^= " ";
		}
		//S o f t  w a r e   b y
		mark ^= var("7962206572617774666F53").iconv("HEX2").b(-1, -999);
		if (html) {
			mark.prefixer("<div style=\"font-size:60%;margin:0px;text-align:left;page-break-before:avoid;page-break-after:avoid\">");
		}

		mark ^= " ";

		mark ^= var("4D4F432E5359534F454E").iconv("HEX2").b(-1, -99);

		if (html) {
			mark ^= "</div>";
		}

	} else {
		mark = "";

		if (html) {

			//hyper link to client's email
			link = SYSTEM.f(10, 2);
			if (link) {
				mark ^= "<a href=\"";
				mark ^= "mailto:" ^ link;
				mark ^= "\">";
			}

		}

		//client mark
		mark ^= SYSTEM.f(14);

		if (html) {
			if (link) {
				mark ^= "</a>";
			}
			//mark:='</small>'
			if (SYSTEM.f(17).ends("_test")) {
				mark = "<span style=\"color:red\">" ^ mark ^ " - " ^ SYSTEM.f(17) ^ "</span>";
			}
		}

		mark = "<div style=\"font-size:60%;margin:0px;text-align:left;page-break-before:avoid;page-break-after:avoid\">" ^ mark ^ "</div>";

	}

	return 0;
}

// In case refactoring c++ code mangles javascript code
subr replace_literal_comparison_operators(io dataio) {
	dataio.replacer(" == ", " == ");
	dataio.replacer(" != ", " != ");
	dataio.replacer(" < ", " < ");
	dataio.replacer(" <= ", " <= ");
	dataio.replacer(" > ", " > ");
	dataio.replacer(" >= ", " >= ");
}

// Convert exodus comment text (pseudo markdown) to html
function t2h_main(in inp) {

	bool in_code = false;
	var outp = "";

	// lines of text become "fields" separated by \x1e chars
	// We are still going to refer to them "lines"
	// even while we use "f"/"field" functions to access them
	var lines = inp.convert("\n", FM);
	var lastindent = 1;
	var linetypes = "";

	// lambda to return the correct tag
	///////////////////////////////////

	auto linetypetag = [] (in linetype, bool opening) -> var {
		if (linetype eq "*" or linetype eq "#")
			return "li";
		if (not opening and linetype eq "dt")
			return "dd";
		return linetype;
	};

	// lambda to close old tags and open new ones
	/////////////////////////////////////////////

	auto close_andor_open = [&](in newindent, in newlinetype) {

// 2 1 * * on exit from one indent
//printl(lastindent, newindent, newlinetype, linetypes.convert(FM, "^"));

		// Close old indents if decreasing indent
		while (lastindent > newindent) {
			let lastlinetype = linetypes.f(lastindent);
			if (lastlinetype) {
				outp.appender("</", linetypetag(lastlinetype, false), ">\n");
				// Close old list?
				if (lastlinetype eq "*")
					outp.appender("</ul>\n");
				else if (lastlinetype eq "#")
					outp.appender("</ol>\n");
				else if (lastlinetype eq "dt" or lastlinetype eq "dd")
					outp.appender("</dl>\n");
			}
			lastindent--;
		}

		// Note that if we are going to a deeper indent then lastlinetype will of course be ""
		// but the reverse is not true if we are staying on the same indent or rising up to a shallower indent
		lastindent = newindent;
		let lastlinetype = linetypes.f(lastindent);

		if (lastlinetype)
			outp.appender("</", linetypetag(lastlinetype, false), ">\n");

		// Changing line type at new current level?
		// If going deeper then this is always true because last line type is ""
		if (lastlinetype ne newlinetype) {

			// Close old list?
			if (lastlinetype eq "*")
				outp.appender("</ul>\n");
			else if (lastlinetype eq "#")
				outp.appender("</ol>\n");
			else if (lastlinetype eq "dt" or lastlinetype eq "dd")
				outp.appender("</dl>\n");

			// Start new list?
			if (newlinetype eq "*")
				outp.appender("<ul>\n");
			else if (newlinetype eq "#")
				outp.appender("<ol>\n");
			else if (newlinetype eq "dt" or lastlinetype eq "dd")
				outp.appender("<dl>\n");
		}

		linetypes(newindent) = newlinetype;

		// Cut off any deeper indents since we have handled and close them all
		linetypes = linetypes.field(FM, 1, newindent);
	};

	// Process all text lines
	/////////////////////////

	for (let rawline : lines) {

		var line = rawline.trimfirst();

		// Determine new indent (1 based)
		let newindent = len(rawline) - len(line) + 1;

		// Skip over, into or out of code block
		///////////////////////////////////////

		if (in_code) {
			if (line.last() eq "`") {
				// Switch out of code
				in_code = false;
			}
			outp.appender(space(newindent - 1), line, NL_);
			continue;
		}
		else if (line.first() eq "`") {
			in_code = true;
			outp ^= NL_;
			outp.appender(space(newindent - 1), line, NL_);
			continue;
		}

		// Inbound line conversions
		// Convert (not code block) since we may add html tags ourselves
		// This preserves any < & > in the text but disallows any html pass through
		line.replacer("&", "&amp;");
		line.replacer("<", "&lt;");
		line.replacer(">", "&gt;");

		// Determine linetype and line
		var linetype;
		{
			let match1 = line.match(R"__(^\* (.*?) \* (.*)$)__");
			if (match1) {

				//////////////////
				// Definition list -> <dl>
				//////////////////
				// * aaaa * bbbbbb
				//////////////////
				linetype = "dt";
				line = match1.f(1, 2).trimlast() ^ "</dt><dd>" ^ match1.f(1, 3).trimboth("* ");

			} else if (line.match(R"__(^\* *)__")) {

				////////////////
				// Bulleted list -> <ul> Unordered list
				////////////////
				// * xxxxxxxxxxx
				////////////////
				linetype = line.first();
				line.cutter(1);
				line.trimmerfirst();

			} else if (line.match("^\\d+\\. ")) {

				////////////////
				// Numbered list -> <ol> Ordered list
				////////////////
				// 99. xxxxxxxxx
				////////////////
				linetype = "#";
				line = line.field(".", 2, 999).trimfirst();

			} else {
				// xxxxxxxx
				linetype = "p";
			}
		}

		// Close any previous tags and open any group tags
		close_andor_open(newindent, linetype);
		linetypes(newindent) = linetype;

		// Outbound line conversions
		{
			// http links not already quoted or inside tags
			// https://en.cppreference.com/w/cpp/container/vector.html
			static rex href_rex = R"__(http[s]?:/[/a-zA-Z0-9_&=.:]+)__"_rex;
			line.replacer(href_rex, "<a href=\"$0\">$0</a>");
//			static rex href_rex = R"__(([^">])(http[s]?:/[/a-zA-Z0-9_&=.:]+))__"_rex;
//			line.replacer(href_rex, "$1<a href=\"$2\">$2</a>");

			// Emphasise first phrases ending in ::
			static rex rex1 = R"__(^([\w _.\[\]]+)::( |<|$))__"_rex;
			line.replacer(rex1, "<em>$1:</em>$2");

			// Emphasise first words ending in :
			// Accept \w _ . [ ]
//			line.replacer(    R"__(^([\w_.\[\]]+): ?)__"_rex, "<em>$1:</em> ");
			static rex rex2 = R"__(^([\w_.\[\]]+): ?)__"_rex;
			line.replacer(rex2, "<em>$1:</em> ");
//			static rex rex2 = R"__(^([\w_.\[\]]+):([ <]))__"_rex;
//			line.replacer(rex2, "<em>$1:</em>$2");

		}

		// Open a tag and output the line
		outp.appender(space(newindent - 1) ^ "<", linetypetag(linetype, true), ">", line);

	}

	// Close any outstanding tags.
	close_andor_open(1, "");

	return outp.trim("\n") ^ "\n";

}

var inp =
        "# Function Overview\n"
        "* Feature 1\n"
        "  * Subfeature 1\n"
        "  * Subfeature 2\n"
        "* Feature 2\n"
        "Some text here.\n"
        "# Steps\n"
        "  # Step 1\n"
        "  # Step 2\n";

function t2h_selftest() {

//	TRACE(main(""))
//	TRACE(main("abc"))
//	TRACE(main("abc\ndef"))
//	TRACE(main("*abc"))
//
//	TRACE(main("*abc\ndef"))
//	TRACE(main("*abc\n*def"))
//	TRACE(main("abc\n*def"))
//
//	TRACE(main("#abc\n*def"))
//	TRACE(main("*abc\n#def"))
//
//	TRACE(main("abc\n def"))
//
//	TRACE(main("*abc\n #def"))

	assert(t2h_main("")            eq "");
	assert(t2h_main("abc")         eq "<p>abc</p>\n");
	assert(t2h_main("abc\ndef")    eq "<p>abc</p>\n<p>def</p>\n");
	assert(t2h_main("*abc")        eq "<ul>\n<li>abc</li>\n</ul>\n");
	assert(t2h_main("*abc\ndef")   eq "<ul>\n<li>abc</li>\n</ul>\n<p>def</p>\n");
	assert(t2h_main("*abc\n*def")  eq "<ul>\n<li>abc</li>\n<li>def</li>\n</ul>\n");
	assert(t2h_main("abc\n*def")   eq "<p>abc</p>\n<ul>\n<li>def</li>\n</ul>\n");
	assert(t2h_main("#abc\n*def")  eq "<ol>\n<li>abc</li>\n</ol>\n<ul>\n<li>def</li>\n</ul>\n");
	assert(t2h_main("*abc\n#def")  eq "<ul>\n<li>abc</li>\n</ul>\n<ol>\n<li>def</li>\n</ol>\n");
	assert(t2h_main("abc\n def")   eq "<p>abc <p>def</p>\n</p>\n");
	assert(t2h_main("*abc\n #def") eq "<ul>\n<li>abc<ol>\n <li>def</li>\n</ol>\n</li>\n</ul>\n");
	assert(t2h_main("*abc\n *def") eq "<ul>\n<li>abc<ul>\n <li>def</li>\n</ul>\n</li>\n</ul>\n");

//	assert(t2h_main("Returns: value").errputl() eq "<p><em>Returns:</em> value</p>\n");
//	assert(t2h_main("// Header").errputl() eq "<h2>Header</h2>\n");
	assert(t2h_main("1. Item 1\n2. Item 2") eq "<ol>\n<li>Item 1</li>\n<li>Item 2</li>\n</ol>\n");
//	assert(t2h_main("<!--CODEBLOCK-->").errputl() eq "<!--CODEBLOCK-->\n");

	assert(t2h_main("<>") eq "<p>&lt;&gt;</p>");

//	TRACE(t2h_main(inp))

	if (not quiet)
		logputl("Self Test passed.");

	return 1;
}

function h2m_main(in all_input, out exit_status) {

//	if (not quiet)
//		logputl("h2m input");

	// 0 success
	// 1 parser error
	// 2 converter error
//	int exit_status = 0;
	exit_status = 0;

//	selftest_validator();

	// Read lines of input and parse line by line. Probably should read all then parse all.
	// Parse HTML into tokens
	// Tokens:
	// 1. Pure tags stripped of any attributes
	// 2. Anything between the tags
//	std::string all_input = reader();
	std::vector<std::string> tokens = h2m_parser(all_input, exit_status);
	if (exit_status) {
		// Soldier on
	}

	// Call a general html tag validator
	// This checks that all tags are closed and must be checked at the end
    HTMLTagValidator h2m_validator;
    if (!h2m_validator.process(all_input)) {
        std::cerr << "Invalid HTML detected in line: " << std::endl;
        exit_status = 2;
		// Soldier on
    }

	// Convert tokens to man page format and output
	// Runs even if parser fails
	return h2m_convert_to_man(tokens, exit_status);

}

// Structure to track state of ordered lists (e.g., <ol>) for numbering
struct ListState {
    bool is_ordered = false;  // True if list is ordered (<ol>), false for unordered (<ul>)
    int ol_counter = 1;       // Counter for ordered list items (starts at 1)
	bool is_indented = false; // We must indent if any sub tags (excluding inline macros for bold etc.) are found (and undent on </li>
	bool in_li = false;       // Dont indent after ul/ol until we are inside an li
};

// Converter: Processes token array into man page format using a lookup table
// - Maps known tags to man page commands via tag_map
// - Handles special cases (<li>, <a href>) dynamically
// - Continues processing past unrecognized tags, logging errors to stderr
// - Sets exit_status to 2 if conversion errors occur
var h2m_convert_to_man(const std::vector<std::string>& tokens, out exit_status) {

	std::stringstream ss_out;

    // Output man page header with newline before dot command
//    ss_out << "\n.TH HTML2MAN 1 \"April 06, 2025\" \"html2man\" \"Generated by html2man v4.2\"\n";
//	ss_out << ".nr IN 4n\n";

    std::stack<ListState> list_stack;  // Stack to track nested lists and their states
    std::vector<std::string> footnotes;  // Array to store footnote URLs from <a href>
    bool in_code = false;              // Flag for code block state (<code>, <pre>)

    // Tag-to-man conversion table based on our mapping
    // - Left: HTML tag, Right: Man page command or macro
    // - Newlines moved before dot commands where appropriate (Fix 3)
    // - No structural validation; purely a lookup for output
    std::map<std::string, std::string> tag_map = {
        {"<h1>",            "\n.SH "},         // Section heading
        {"</h1>",           "\n"},             // End section heading
        {"<h2>",            "\n.SS "},         // Subsection heading
        {"</h2>",           "\n"},             // End subsection heading
        {"<h3>",            "\n.SS "},         // Same as h2
        {"</h3>",           "\n"},             // Same as /h2
        {"<h4>",            "\n.SS "},         // "
        {"</h4>",           "\n"},             // "

//        {"<p>",             "\n.br\n.in +0.5n\n"},        // Paragraph break with newline
//        {"</p>",            "\n.in -0.5n\n"},             // End paragraph
//        {"<p>",             "\n.PP \\\"<p>\n"},        // Paragraph break with newline
//        {"</p>",            "\n.sp 1 \\\"</p>\n"},             // End paragraph
        {"<p>",             "\n.PP\n"},        // Paragraph break with newline
        {"</p>",            "\n.sp 0\n"},             // End paragraph


        {"<br>",            "\n.br\n"},        // Line break with newline

        {"</li>",           ""},               // End list item (handled by <li>, no extra newline)
        {"</a>",            "\\fR"},           // End link text

        {"<ul>",            ""},          // Start unordered list (indent)
        {"</ul>",           ""},          // End unordered list (unindent)

        {"<ol>",            ""},          // Start ordered list (indent)
        {"</ol>",           ""},          // End ordered list (unindent)

        {"<b>",             "\\fB"},           // Start bold text
        {"</b>",            "\\fR"},           // End bold text

        {"<em>",             "\\fB"},           // Start bold text
        {"</em>",            "\\fR"},           // End bold text

        {"<strong>",        "\\fB"},           // Start bold text (semantic alias for <b>)
        {"</strong>",       "\\fR"},           // End bold text

        {"<i>",             "\\fI"},           // Start italic text
        {"</i>",            "\\fR"},           // End italic text

        {"<u>",             "\\fI"},           // Start italic text (underline approximated)
        {"</u>",            "\\fR"},           // End italic text

//        {"<code>",          "\n.nf \\\"<code>\n"},        // Start code block (no fill/no format)
//        {"</code>",         "\n.fi \\\"</code>\n"},        // End code block (restore fill/format)
        {"<code>",          "\n.EX \\\"<code>\n"},     // Start code block (no fill/no format)
        {"</code>",         "\n.EE \\\"</code>\n"},    // End code block (restore fill/format)

        {"<pre>",           "\n.nf \\\"<pre>\n"},      // Start preformatted text (no fill/no format)
        {"</pre>",          "\n.fi \\\"</pre>\n"},     // End preformatted text (restore fill/format)

        {"<dl>",            "\n.PD 0 \\\"<dl>\n"},     // Start descriptive list
        {"</dl>",           "\n.PD \\\"</dl>\n"},      // End descriptive list

		///////////////////////////////////
		// apt-get install groff groff-base
		// man groff_man
		// man tbl
		///////////////////////////////////

        {"<dt>",            "\n.TP 10\\\"<dt>\n\\fB"}, // Start descriptive term
        {"</dt>",           "\\fR \\\"</dt>\n"},       // End descriptive term

        {"<dd>",            ""},                       // Start descriptive details
        {"</dd>",           "\n.\\\"</dd>\n"},         // End descriptive details

        {"<blockquote>",    "\n.RS\n"},        // Start blockquote (indent)
        {"</blockquote>",   "\n.RE\n"},        // End blockquote (unindent)

        {"<hr>",            "\n.br\n"},        // Horizontal rule (line break)

        {"<sup>",           "\\u"},            // Superscript
        {"</sup>",          "\\fR"},           // End superscript

        {"<sub>",           "\\d"},            // Subscript
        {"</sub>",          "\\fR"},           // End subscript

        {"<table>",         "\n.TS\ntab(:);\nl l.\n"},  // Start simple table (left-aligned columns)
        {"</table>",        "\n.TE\n"}          // End table
    };

    for (size_t i = 0; i < tokens.size(); ++i) {  // Process each token
        std::string token = tokens[i];

        if (token[0] == '<') {

            // Handle tags

            // <a href="..."
            if (token.substr(0, 8) == "<a href=") {

                // Special case: <a href="...">
#if 0
                std::regex href_regex("href=\"([^\"]+)\"");  // Extract URL from href attribute
                std::smatch match;
                if (std::regex_search(token, match, href_regex)) {
                    footnotes.push_back(match[1]);  // Store URL for footnote
#else
                rex href_regex("href=\"([^\"]+)\"");  // Extract URL from href attribute
                let match1 = match(token, href_regex);
				if (match1) {
                    footnotes.push_back(match1.f(1, 2));  // Store URL for footnote
#endif
                    ss_out << "\\fB";            // Bold link text
                }

            }

            // <li>
            else if (token == "<li>" && !list_stack.empty()) {
//                // Special case: <li> in list (Custom formatting for lists)
//                ss_out << "\n.br\n";  // Break line before list item
//                if (list_stack.top().is_ordered) {
//                    // Ordered list item
//                    ss_out << std::to_string(list_stack.top().ol_counter++) << ". ";
//                } else {
//                    // Unordered list item
//                    ss_out << "\\(bu ";
//                }
                // Special case: <li> in list (Custom formatting for lists)
                ss_out << "\n.br\n";  // Break line before list item
                if (list_stack.top().is_ordered) {
                    // Ordered list item
                    ss_out << std::to_string(list_stack.top().ol_counter++) << ". ";
                } else {
                    // Unordered list item
                    ss_out << "\\(bu ";
                }
//				ss_out << ".RS\n";
//				ss_out << "\n";
            }
            else if (tag_map.count(token)) {

				// Trigger an indent
				if (!list_stack.empty()) {
	                if (token == "</li>") {
						// undent if necessary
						if (list_stack.top().is_indented) {
		                    ss_out << "\n.RE\n";
							list_stack.top().is_indented = false;
							list_stack.top().in_li = false;
						}
	                } else if (!list_stack.top().is_indented) {
						if (token == "<li>")
							// flag that we are inside and li
							list_stack.top().in_li = true;
						else if (token[1] != '/' /* and token is not inline type)*/) {
							// Indent on most opening tags inside li item
//		                    ss_out << "\n.RS\n";
		                    ss_out << "\n.RS";
							list_stack.top().is_indented = true;
						}
					}
				}

                // Known tag in table
                ss_out << tag_map[token];  // Output mapped command

                // Update state based on tag
                if (token == "<code>" || token == "<pre>") in_code = true;
                else if (token == "</code>" || token == "</pre>") in_code = false;
                else if (token == "<ul>" || token == "<ol>") list_stack.push({token == "<ol>", 1});
                else if (token == "</ul>" || token == "</ol>") { if (!list_stack.empty()) list_stack.pop(); }
            }
            else {
                // Unrecognized tag
                std::cerr << "h2ml::convert_to_man ERROR: Unrecognized tag: " << token << "\n";
                exit_status = 1;
                // Soldier on: skip this tag, continue processing
            }
        } else {
            // Handle plain text

			// Unescape the critical html codes

		    // Step 1: Replace < with <
		    size_t pos = 0;
		    while ((pos = token.find("&lt;", pos)) != std::string::npos) {
		        token.replace(pos, 4, "<");
		        pos += 1; // Move past <
		    }

		    // Step 2: Replace > with >
		    pos = 0;
		    while ((pos = token.find("&gt;", pos)) != std::string::npos) {
		        token.replace(pos, 4, ">");
		        pos += 1; // Move past >
		    }

		    // Step 3: Replace & with &
		    pos = 0;
		    while ((pos = token.find("&amp;", pos)) != std::string::npos) {
		        token.replace(pos, 5, "&");
		        pos += 1; // Move past &
		    }

            // Skip whitespace-only tokens (Fix 2)
            size_t j0 = token.find_first_not_of(" \t\n");
            if (j0 == std::string::npos)
                continue;

			// Allow one leading space
			if (j0 > 0) {
				j0 --;
				token[j0] = ' ';
			}

            // Process text with troff escaping
            std::string escaped;
			// Allow "\xff\x00" in code
//            if (!in_code) {  // Escape troff special chars outside code blocks
                for (size_t j = j0; j < token.length(); ++j) {  // Index-based loop for compatibility
                    char c = token[j];
                    if (c == '.' && escaped.empty()) escaped += "\\&.";  // Escape leading dot
                    else if (c == '\\') escaped += "\\\\";              // Escape backslash
                    else escaped += c;                                  // Normal char
                }
//            } else {
//                // No escaping in code blocks. Why?
//                escaped = token;
//            }
            // Output text with \& to prevent command interpretation (e.g., after periods)
//            if (!in_code && !escaped.empty() && escaped.back() == '.') escaped += "\\&";
            ss_out << escaped;
        }
    }

    // Output footnotes for <a href> links
    for (size_t i = 0; i < footnotes.size(); ++i) {
        ss_out << "\n.br\n\\(dg " << i + 1 << ". " << footnotes[i] << "\n";
    }

	var vout = ss_out.str();

	// Remove ".PP" before box chars.
	vout.replacer("\\.PP\n([┌│├└])"_rex, "$1");

	// Make sure .sp 1 -> .sp 0 after box chars.
	vout.replacer("([┐│┤])\n\\.sp \\d"_rex,"$1\n.sp 0");

	//.PP
	//┌─────────┬─────────────┬──────────────────────────┬─────────────────────────┐
	//.sp 0
	//.PP
	//│ Command │ Mechanism   │ Execution                │ Use Case                │
	//.sp 0
	//.PP
	//├─────────┼─────────────┼──────────────────────────┼─────────────────────────┤
	//.sp 0
	//.PP
	//│ async   │ Fiber       │ Cooperative, yield-based │ Lightweight async tasks │
	//.sp 0
	//.PP
	//│ run     │ Thread pool │ Parallel, preemptive     │ Heavy parallel tasks    │
	//.sp 0
	//.PP
	//└─────────┴─────────────┴──────────────────────────┴─────────────────────────┘
	//.sp 0

	return vout.trim("\n") ^ "\n";

}

class HTMLTagValidator {
private:
    std::stack<std::string> tags;
    static inline const std::string selfClosingTags =
        "|br|img|input|hr|meta|link|area|base|col|embed|source|track|wbr|";

    static bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static char toLower(char c) {
        if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
        return c;
    }

    bool isSelfClosing(const std::string& tag) const {
        std::string lowerTag = tag;
//        for (char& c : lowerTag) c = toLower(c);
		for (char* c = lowerTag.data(); *c; ++c) *c = toLower(*c);
        std::string delimitedTag = "|" + lowerTag + "|";
        return selfClosingTags.find(delimitedTag) != std::string::npos;
    }

    bool isCommentOrSpecial(const std::string& tagContent) const {
        return tagContent.substr(0, 3) == "!--" ||
               tagContent.substr(0, 8) == "![CDATA[" ||
               tagContent.substr(0, 8) == "!DOCTYPE";
    }

    bool isValidTagName(const std::string& tag) const {
        std::string lowerTag = tag;
//        for (char& c : lowerTag) c = toLower(c);
		for (char* c = lowerTag.data(); *c; ++c) *c = toLower(*c);
        if (lowerTag.empty() || !isAlpha(lowerTag[0])) return false;
//        for (char c : lowerTag) {
		for (char* c = lowerTag.data(); *c; ++c) {
            if (!((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '-')) return false;
        }
        return true;
    }

public:
    HTMLTagValidator() {}

    bool process(const std::string& html) {
        size_t pos = 0;
        while (pos < html.length()) {
            if (html[pos] == '<') {
                size_t end = html.find('>', pos);
                if (end == std::string::npos) return false;

                std::string tagContent = html.substr(pos + 1, end - pos - 1);
                if (tagContent.empty()) return false;

                if (isCommentOrSpecial(tagContent)) {
                    if (tagContent.substr(0, 3) == "!--") {
                        size_t commentEnd = html.find("-->", pos);
                        if (commentEnd == std::string::npos) return false;
                        pos = commentEnd + 3;
                    } else {
                        pos = end + 1;
                    }
                    continue;
                }

                if (tagContent[0] != '/' && !isAlpha(tagContent[0]) && tagContent[0] != '!') {
                    pos = end + 1;
                    continue;
                }

                if (tagContent[0] == '/') {
                    std::string tagName = tagContent.substr(1);
//                    for (char& c : tagName) c = toLower(c);
                    for (char* c = tagName.data(); *c; ++c) *c = toLower(*c);
                    if (tags.empty() || tags.top() != tagName) return false;
                    tags.pop();
                } else {
                    size_t spacePos = tagContent.find(' ');
                    std::string tagName = (spacePos == std::string::npos) ? tagContent : tagContent.substr(0, spacePos);
                    bool hasSlash = tagContent.back() == '/' && (spacePos == std::string::npos || spacePos == tagContent.length() - 1);
                    if (hasSlash && spacePos == std::string::npos) tagName = tagName.substr(0, tagName.length() - 1);
//                    for (char& c : tagName) c = toLower(c);
                    for (char* c = tagName.data(); *c; ++c) *c = toLower(*c);
                    if (!isValidTagName(tagName)) return false;
                    if (!isSelfClosing(tagName)) tags.push(tagName);
                }
                pos = end + 1;
            } else {
                pos++;
            }
        }
        return tags.empty();
    }
};

int h2m_selftest_validator() {
    std::vector<std::pair<std::string, int>> test_cases = {
        {"<div><p><span>Hello</span></p></div>", 0},
        {"<div><br/>Text<img src='x'/></div>", 0},
        {"<div><p>Hello", 1},
        {"<div><p></div>", 1},
        {"<>", 1},
        {"<div class='x'><p id='y'>Text</p></div>", 0},
        {"<div><p>Hello<", 1},
        {"<div><!-- Comment --></div>", 0},
        {"<div><![CDATA[<p>]]></div>", 0},
        {"<!DOCTYPE html><div></div>", 0},
        {"<DIV><p></P></div>", 0},
        {"<p>a < b > c</p>", 0},
        {"<div><!-- > --></div>", 0},
        {"<div><!-- Comment", 1},
        {"<p>Text <span>a > b</span> more</p>", 0},
        {"<br/><img/><hr/>", 0},
        {"This is <not> a tag</not>", 0}
    };

    int passed = 0;
    for (size_t i = 0; i < test_cases.size(); ++i) {

//        const auto& [html, expected_status] = test_cases[i];
		auto case1 = test_cases[i];
		auto& html = case1.first;
		auto& expected_status = case1.second;

        HTMLTagValidator validator;
        bool is_valid = validator.process(html);

        if ((is_valid && expected_status == 0) || (!is_valid && expected_status == 1)) {
//            std::cout << "  ✅ Passed\n";
            ++passed;
        } else {
            std::cerr << "Test " << i + 1 << ": \"" << html << "\"\n";
            std::cerr << "  Expected: " << (expected_status == 0 ? "Valid" : "Invalid")
                  << ", Got: " << (is_valid ? "Valid" : "Invalid") << "\n";
            std::cerr << "  ❌ Failed\n";
            std::cerr << "\n";
        }
    }

    std::clog << "Test passed. " << passed << " out of " << test_cases.size() << " tests.\n";

    return (static_cast<size_t>(passed) == test_cases.size()) ? 0 : 1;
}


//// Reads HTML from stdin, returning all input as a string with lines separated and terminated by \n
//std::string h2m_reader() {
//	std::string all_input;
//	std::string line;
//	while (std::getline(std::cin, line)) {
//		line.push_back('\n');  // Preserve line endings
//		all_input += line;
//	}
//	return all_input;
//}

// Parses HTML input into a vector of tokens (tags and text)
// - Checks for unterminated tags (e.g., <tag without >) within each line
// - Does not validate HTML structure (e.g., matching <tag> with </tag>)
// - Sets exit_status to 1 if parsing errors occur
std::vector<std::string> h2m_parser(const std::string& all_input, out exit_status) {
    std::vector<std::string> tokens;
    std::string line;
    size_t input_pos = 0;

    // Process input line-by-line
    while (input_pos < all_input.length()) {
        // Extract one line (up to and including \n)
        size_t line_end = all_input.find('\n', input_pos);
        if (line_end == std::string::npos) {
            line = all_input.substr(input_pos);
            input_pos = all_input.length();
        } else {
            line = all_input.substr(input_pos, line_end - input_pos + 1);
            input_pos = line_end + 1;
        }

        size_t pos = 0;
        while (pos < line.length()) {
            if (line[pos] == '<') {  // Start of a tag
                size_t end = line.find('>', pos);
                if (end == std::string::npos) {
                    std::cerr << "h2ml::parser ERROR: Unterminated tag (no '>') in line: " << line << "\n";
                    exit_status = 1;
                    break;  // Skip rest of line on error
                }
                // Extract full tag including < and > (e.g., <h1>, </p>)
                std::string tag = "<" + line.substr(pos + 1, end - pos - 1) + ">";
                tokens.push_back(tag);
                pos = end + 1;
            } else {  // Plain text between tags
                size_t next_tag = line.find('<', pos);
                if (next_tag == std::string::npos) next_tag = line.length();
                std::string text = line.substr(pos, next_tag - pos);
                if (!text.empty()) {
                    tokens.push_back(text);
                }
                pos = next_tag;
            }
        }
    }

    return tokens;
}

/* HTML Indentation Formatter

1. One-line Summary:
Formats HTML-like input, including snippets without <body>, by indenting tags with tabs, trimming trailing whitespace, preserving content whitespace, and detecting indentation errors, building output in a single string.

2. What It Handles:
- Any HTML-like tags (e.g., <div>, <custom>, <span>) regardless of input indentation.
- HTML snippets lacking <body> or <BODY>, starting indentation from the beginning.
- Void (self-closing) tags (e.g., <br>, <img>) without increasing indent level.
- Closing tags </body> and </html> ignored for indentation purposes.
- Non-tag '<' characters (e.g., <3, < b in text) output as-is.
- Case-insensitive <body> and <BODY> for pre-<body> content preservation.
- Whitespace preservation in tags like <dt> and <pre> (e.g., for white-space: pre-wrap).
- Trims trailing spaces and tabs before non-void opening tags to clean up output.
- Excessive undent attempts with per-line and summary error messages on stderr.
- Exit status for errors: 2 for excessive undents, 1 for unbalanced tags, 0 for success.
- Line count message on clog indicating lines converted.

3. Implementation Details:
- Reads entire input from stdin into a std::string for processing.
- Builds output in a single std::string, reserved at 1.1 times input size, using push_back for efficiency.
- Outputs final string to std::cout at the end of processing.
- Uses std::string_view to efficiently append pre-<body> content to the output string.
- Starts processing at the '<' of <body> or <BODY>, or at the start (body_pos = 0) if no <body> is found.
- Initializes indent_level = -1 for <body> content, or 0 for snippets without <body>.
- Uses char* pointer iteration over c_str() for character-by-character processing.
- Indents with a single tab character (char indent_unit = '\t') per level.
- Tracks line numbers (starting at 1) by counting '\n' characters.
- Recognizes valid tag start characters: letters (a-z, A-Z), '/', '!', '?'.
- Ignores non-tag '<' (e.g., <3, <@) by checking next character.
- Handles void tags (br, img, hr, meta, link, input) and closing tags (/body, /html) via is_void_tag function.
- is_void_tag converts tag names to lowercase for case-insensitive matching.
- For non-void opening tags: trims trailing spaces/tabs from output string, appends newline (if last char is not '\n'), indents, increments indent_level.
- For closing tags (not /body, /html): decrements indent_level if > 0, else increments excessive_undent and outputs error to stderr with line number.
- For void tags and /body, /html: appends without indent_level change or indentation.
- Prevents indent_level from going below 0, counting excessive undent attempts.
- Outputs per-line excessive undent errors to stderr with line number.
- Outputs summary errors to stderr: total excessive_undent count and non-zero indent_level if applicable.
- Outputs line count to clog: "Lines converted: <number>".
- Returns exit status: 2 if excessive_undent > 0, 1 if indent_level != 0, 0 if both are satisfied.
- Preserves all content whitespace (e.g., in <dt>, <pre>) by appending characters as-is.
- Outputs no trailing newline, ending with the last processed character.
- Handles large files (e.g., 531,391 bytes) efficiently with string and pointer operations.
- Case-insensitive handling of <body> and <BODY> for pre-<body> content.
- Supports HTML with <pre>, <code>, <span> containing non-tag '<' (e.g., <span class="o">=</span>).
- Processes unindented or poorly indented input, applying consistent tab-based indentation.
*/

//#include <iostream>
//#include <string>
//#include <sstream>
//#include <string_view>
//#include <cstdlib> // For EXIT_SUCCESS, EXIT_FAILURE

bool is_void_pre_tag(const char* ptr, std::string& tag_name) {
    // List of tags to ignore for indentation (lowercase)
    static const char* ignored_tags[] = {
        "br", "img", "hr", "meta", "link", "input", // Void tags
        "/body", "/html", // Closing tags to ignore
        nullptr // Sentinel
    };

    // Check if it's a closing tag
    bool is_closing = (*ptr == '<' && *(ptr + 1) == '/');
    ptr += is_closing ? 2 : 1; // Skip '</' or '<'

    // Collect tag name until space, '>', or '/'
//    std::string tag_name;
    tag_name = "";
    if (is_closing) {
        tag_name += '/'; // Prefix closing tags with '/'
    }
    while (*ptr != '\0' && *ptr != ' ' && *ptr != '>' && *ptr != '/') {
        tag_name += (*ptr >= 'A' && *ptr <= 'Z') ? *ptr + ('a' - 'A') : *ptr; // Convert to lowercase
        ++ptr;
    }

    // Check if tag_name matches any ignored tag
    for (int i = 0; ignored_tags[i] != nullptr; ++i) {
        if (tag_name == ignored_tags[i]) {
            return true;
        }
    }
    return false;
}

// Shim to convert var <-> std::strings
// 0 Success
// 1 Excessive indent
// 2 Excessive undent
function html_tidy_main(in html_in, out html_out, out messages) {
	std::string html_out_str;
	std::stringstream messages_ss;

	let result = html_tidy_main2(html_in.toString(), html_out_str, messages_ss);
	html_out = std::move(html_out_str);

	messages = messages_ss.str();
	if (result)
		messages.errputl();

	return result;
}

int html_tidy_main2(const std::string& input, std::string& output, std::stringstream& messages) {

//    // Read entire input into a string
//    std::stringstream buffer;
//    buffer << std::cin.rdbuf();
//    std::string input = buffer.str();

    const char* input_cstr = input.c_str();

    // Initialize output string with reserved capacity (1.1x input size)
//    std::string output;
    output.reserve(static_cast<size_t>(double(input.size()) * 1.1));

    int indent_level = -1; // Current indentation level (start at -1 so <body> is 0)
    int excessive_undent = 0; // Count attempts to undent below 0
    int line_number = 1; // Track line number, starting at 1
    const char indent_unit = '\t'; // Tab character

    // Find <body> or <BODY> position
    size_t body_pos = input.find("<body");
    if (body_pos == std::string::npos) {
        body_pos = input.find("<BODY");
        if (body_pos == std::string::npos) {
            // Process entire input as snippet
            body_pos = 0;
            indent_level = 0;
        }
    }

    // Append pre-<body> content using string_view
    std::string_view input_view(input);
    output.append(input_view.substr(0, body_pos));

    // Count lines in pre-<body> content
    for (size_t i = 0; i < body_pos && input_cstr[i] != '\0'; ++i) {
        if (input_cstr[i] == '\n') {
            ++line_number;
        }
    }

    // Start at the '<' of <body> or <BODY> (or start of string for snippets)
    const char* ptr = input_cstr + body_pos;
    char prev_char = body_pos > 0 ? *(ptr - 1) : '\0'; // Track previous character

	std::string tag_name;
	int inside_pre = 0;

    // Process characters with pointer, building output string
    while (*ptr != '\0') {
        char c = *ptr;
        if (c == '<') {
            // Look ahead to next character
            char next = *(ptr + 1);
            // Check if next character is valid for a tag (ordered by probability)
            bool is_valid_tag = ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') ||
                                 next == '/' || next == '!' || next == '?');
            if (is_valid_tag) {
                bool is_closing_tag = (next == '/');
                // Check if it's a void tag or </body>, </html
                bool is_ignored = is_void_pre_tag(ptr, tag_name);
                if (!is_ignored) {
                    if (!is_closing_tag) {
						if (!inside_pre) {
	                        // Non-void opening tag: remove existing indent (if any), append newline and indent
	                        while (!output.empty() && (output.back() == ' ' || output.back() == '\t')) {
	                            output.pop_back();
	                        }
	                        if (!output.empty() && output.back() != '\n') {
	                            output.push_back('\n');
	                        }
	                        for (int j = 0; j < indent_level; ++j) {
	                            output.push_back(indent_unit);
	                        }
						}
                        // Increase indent level
                        ++indent_level;
						if (tag_name == "pre")
							inside_pre++;
                    } else {
                        // Other closing tags: decrease indent level if not already 0
                        if (indent_level > 0) {
                            --indent_level;
                        } else {
                            ++excessive_undent;
                            messages << "htmllib2:html_tidy: Error: Excessive undent attempt on line " << line_number << ". See htmllib2.input.txt\n";
							if (not oswrite(input on "htmllib2.input.txt")) {}
                        }
						if (!inside_pre) {
	                        // Indent if on a new line
	                        if (prev_char == '\n') {
	                            for (int j = 0; j < indent_level; ++j) {
	                                output.push_back(indent_unit);
	                            }
	                        }
						}
						if (tag_name == "/pre")
							inside_pre--;
                    }
                } else {
                    // Void tags or </body>, </html>: just append without indent or level change
                    output.push_back('\n');
                    for (int j = 0; j < indent_level; ++j) {
                        output.push_back(indent_unit);
                    }
                }
            }
            output.push_back(c); // Append '<' whether tag or not
        } else {
            // Append all other characters as-is
            output.push_back(c);
        }
        // Update line number
        if (c == '\n') {
            ++line_number;
        }
        prev_char = c; // Update previous character
        ++ptr; // Move to next character
    }

    // Output the final string to cout
//    std::cout << output;

    // Output line count to clog
    messages << "htmllib2:html_tidy: Lines converted: " << line_number << '\n';

    // Output summary error messages to stderr
    if (excessive_undent > 0) {
        messages << "htmllib2:html_tidy: Error: Total excessive undent attempts: " << excessive_undent << '\n';
    }
    if (indent_level != 0) {
        messages << "htmllib2:html_tidy: Error: Final indent level non-zero: " << indent_level << '\n';
    }

    // Determine exit status
    if (excessive_undent > 0) {
        return 2; // EXIT_FAILURE with status 2 for excessive undent
    }
    if (indent_level != 0) {
        return 1; // EXIT_FAILURE with status 1 for non-zero final indent
    }
    return 0; // EXIT_SUCCESS
}

func boxdrawing_to_html(in html) {

//			<p>┌─────────┬─────────────┬───────────────────────────┬─────────────────────────┬───────────────────────────┐</p>
//			<p>│ Command │ Mechanism   │ Execution                 │ Use Case                │ Environment               │</p>
//			<p>├─────────┼─────────────┼───────────────────────────┼─────────────────────────┼───────────────────────────┤</p>
//			<p>│ async   │ Fiber       │ Cooperative, i/o or yield │ Lightweight async tasks │ Shares parent environment │</p>
//			<p>├─────────┼─────────────┼───────────────────────────┼─────────────────────────┼───────────────────────────┤</p>
//			<p>│ run     │ Thread pool │ Parallel, preemptive      │ Heavy parallel jobs     │ Private RECORD/ID etc.    │</p>
//			<p>└─────────┴─────────────┴───────────────────────────┴─────────────────────────┴───────────────────────────┘</p>

	var spacer;
	var pure = false;
	var act = html;
	if (pure) {
		act.replacer("<p>┌", "<table class=\"box_drawing\"><thead>");
		act.replacer("<p>├", "</thead><tbody>");
		act.replacer("<p>└", "</tbody></table>");
		act.replacer("[─┌─┬─┐├─┼─┤└─┴─┘]+"_rex, "");
	} else {
		act.replacer("<p>┌", "<table class=\"box_drawing\">\n<tr><td>");
		act.replacer("<p>├", "<tr><td>");
		act.replacer("<p>└", "<tr><td>");

		act.replacer("┬", "</td><td>");
		act.replacer("┼", "</td><td>");
		act.replacer("┴", "</td><td>");

		act.replacer("┐</p>", "</td></tr>");
		act.replacer("┤</p>", "</td></tr>");
		act.replacer("┘</p>", "</td></tr>\n</table>");

		act.replacer("[─┌─┬─┐├─┼─┤└─┴─┘]"_rex, " ");
	}

	act.replacer("<p>│"_rex, "<tr><td>");
	act.replacer("│</p>"_rex, "</td></tr>");

	act.replacer("│", "</td><td>");

//┌─┬─┐
//
//│ │ │
//
//├─┼─┤
//
//└─┴─┘

	return act;
}

libraryexit()  // Closes the Exodus library class and provides a factory function for use in dynamic loading and linking
