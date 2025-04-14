#include <exodus/library.h>
libraryinit()

// OBSOLETE - replaced by gosub htmllib2("GETCSS",
func main(io css, in version = "") {

	// NB quirk in ie mimiced by mozilla table dont inherit size
	// but work around is TABLE {FONT-SIZE:100%} in css

	// prevents IE later versions from working in standards mode
	// <meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7" />
	// <meta http-equiv="x-ua-compatible" content="IE=6" />

	let charset = SYSTEM.f(127);
	if (charset) {
		css = "<meta http-equiv=\"content-type\" content=\"text/html;charset=" ^ charset ^ "\" />\n";
	} else {
		css = "<!-- no " ^ SYSTEM.f(17) ^ " charset found -->\n";
	}

	css ^= getvogonpoetry(version);

	// css.converter(FM, "\r\n");

	// remove trailing EOL characters
	// while (var("\r\n").contains(css[-1]))
	// 	css.popper();

	let thcolor = SYSTEM.f(46, 1);
	let tdcolor = SYSTEM.f(46, 2);

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

	let agent = SYSTEM.f(40);
	if (agent.contains("MSIE 7")) {
		css.replacer("xborder-collapse", "border-collapse");
	}

	return 0;
}

func getvogonpoetry(in version) {

	if (version.assigned() and version == 2) {

		return R"V0G0N(
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
func toggle(t,mode)
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
 // glogincode=window.opener.glogincode
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

		return R"V0G0N(
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
func toggle(t,mode)
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

}; // libraryexit()
