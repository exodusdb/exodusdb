#include <exodus/library.h>
libraryinit()

//#include <system_common.h>

//#include <sys_common.h>

var params;
var params2;
var tagsep;

function main(in mode, io dataio, in params0="", in params20="", in glang="") {

	//REPLACING UPPERCASE VERSION HTMLLIB()

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
		dataio.prefixer(tt ^ FM);

		//postfix
		dataio ^= FM ^ "</tbody>" ^ FM ^ "</table>";

		//highlight last row if a total
		var nlines = dataio.count(FM) + 1;
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
				datax.prefixer("<table class=\"hashtable\">" ^ tr);
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
			//browser=trimfirst(browser)

		}
		if (browser.ends(".0")) {
			browser.cutter(-2);
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

	} else if (mode eq "GETSORTJS") {

		//in c++ get from raw text string below
		dataio=getvogonpoetry_sortjs();

        dataio.replacer(FM, "\r\n");

		var jsdatefmt;
        if (DATEFMT.contains("E")) {
                jsdatefmt = "d/M/yyyy";
        } else if (DATEFMT.contains("J")) {
                jsdatefmt = "yyyy/M/d";
        } else {
                jsdatefmt = "M/d/yyyy";
        }

        dataio.replacer("gdateformat='d/M/yyyy'", "gdateformat='" ^ jsdatefmt ^ "'");

	} else if (mode eq "GETCSS") {

		//version = params0
		gosub getcss(dataio, params0);

	} else {
		call mssg(mode.quote() ^ " unknown mode in HTMLLIB2");
	}

///////
//exit:
///////
	return 0;
}

function getvogonpoetry_sortjs() {

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

function getcss(io css, in version="") {

	//NB quirk in ie mimiced by mozilla table dont inherit size
	//but work around is TABLE {FONT-SIZE:100%} in css

	//prevents IE later versions from working in standards mode
	//<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
	//<meta http-equiv="x-ua-compatible" content="IE=6" />

	var charset = SYSTEM.f(127);
	if (charset) {
		css = "<meta http-equiv=\"content-type\" content=\"text/html;charset=" ^ charset ^ "\" />\n";
	} else {
		css = "<!-- no " ^ SYSTEM.f(17) ^ " charset found -->\n";
	}

	css ^= getvogonpoetry_css(version);

	//css.converter(FM, "\r\n");

	//remove trailing EOL characters
	//while (var("\r\n").contains(css[-1]))
	//	css.popper();

	var thcolor = SYSTEM.f(46, 1);
	var tdcolor = SYSTEM.f(46, 2);

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
	if (thcolor) {
		css.replacer("#ffff80", "%thcol%");
	}
	if (tdcolor) {
		css.replacer("#ffffc0", "%tdcol%");
	}
	if (thcolor) {
		css.replacer("%thcol%", thcolor);
	}
	if (tdcolor) {
		css.replacer("%tdcol%", tdcolor);
	}

	css.replacer("exodusfont,", font);
	css.replacer("exodussize", fontsize);

	var agent = SYSTEM.f(40);
	if (agent.contains("MSIE 7")) {
		css.replacer("xborder-collapse", "border-collapse");
	}

	return 0;

}

function getvogonpoetry_css(in version) {

	if (version.assigned() and version eq 2) {

		return
R"V0G0N(
<style type="text/css">
/*ver2*/
.BHEAD {display:none}
.BHEAD2 {}
body {background-color:#ffffff; font-family:exodusfont,verdana,sans-serif,arial,helvetica; font-size: exodussize}
table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}
a {color:blue;}
a:visited {color:purple;}
a:hover {color:red;}
tt {font-family:courier new,courier;font-size:80%}
xth {background-color:#ffff80;}
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

/*collapse needed up to IE7 - cant do border-spacing:0
 also to avoid double borders internally*/
table.exodustable {
 border-collapse:collapse;
 margin:0px;
 padding:0px;
 border-spacing:0px;}

table.exodustable th {
 background-color:#ffff80;
 padding:2px;
 border:1px solid #808080;}

table.exodustable > tbody > tr > td {
 background-color:#ffffc0;
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
.exodustable {}
/* #Header, #Footer { display: none !important; } */
.noprint {display:none}
</style>
<script type="text/javascript">
var togglendisplayed=0
function toggle(t,mode)
{
 if (typeof t=='string')
 {
  if (document.getElementsByClassName)
   t=document.getElementsByClassName(t)
  else
   t=document.getElementsByName(t)
 }
 if (t.tagName) t=[t]
 if (!t.length) return
 var display="none"
 for (var i = t.length-1; i>=0; i--) {
  if (t[i].style.display == "") {
   t[i].style.display = "none";
  } else {
   t[i].style.display = "";
   display=""
   }
  }

 if (!mode)
 {
  var toggleheading=false
  if (display=='')
  {
   togglendisplayed++
   if (togglendisplayed==1) toggleheading=true
  }
  else
  {
   togglendisplayed--
   if (togglendisplayed==0) toggleheading=true
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
/*ver1*/
.BHEAD {display:none}
.BHEAD2 {}
body {background-color:#ffffff; font-family:exodusfont,verdana,sans-serif,arial,helvetica; font-size: exodussize}
table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}
a {color:blue;}
a:visited {color:purple;}
a:hover {color:red;}
tt {font-family:courier new,courier;font-size:80%}
th {background-color:#ffff80;}
thead {display:table-header-group}
td.nb {border-bottom:none}
td.nt {border-top:none}
td.nx {border-top:none;border-bottom:none}
.aright {text-align:right}
.aleft {text-align:left}
.acenter {text-align:center}
.nobr {white-space:nowrap;}
.num {text-align:right;mso-number-format:General}
.exodustable {background-color:#ffffc0; border-width:2px; border-collapse:collapse; padding:1px}
.hashtable td {padding:0px; margin:0px; vertical-align:top; border-collapse:collapse; border:0px solid lightgrey;}
.pagedivider {border-top:1px dashed #808080; border-bottom:0px;}
</style>

<style type="text/css" media="print">
.exodustable {}
.noprint {display:none}
</style>

<script type="text/javascript">
var togglendisplayed=0
function toggle(t,mode)
{
 if (typeof t=='string')
 {
  if (document.getElementsByClassName)
   t=document.getElementsByClassName(t)
  else
   t=document.getElementsByName(t)
 }
 if (t.tagName) t=[t]
 if (!t.length) return
 var display="none"
 for (var i = t.length-1; i>=0; i--) {
  if (t[i].style.display == "") {
   t[i].style.display = "none";
  } else {
   t[i].style.display = "";
   display=""
   }
  }

 if (!mode)
 {
  var toggleheading=false
  if (display=='')
  {
   togglendisplayed++
   if (togglendisplayed==1) toggleheading=true
  }
  else
  {
   togglendisplayed--
   if (togglendisplayed==0) toggleheading=true
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

libraryexit()
