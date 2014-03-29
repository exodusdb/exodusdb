#include <exodus/library.h>
libraryinit()

function main(io css) {
	//quirk in ie mimicked by mozilla table dont inherit size
	//but work around is TABLE {FONT-SIZE:100%} in css
	if (not osread(css,"css.txt")) {
		css = defaultcss();
	}

	//remove any trailing EOL, EOF characters
	css.trimmerb("\r\n\x1a");

	var thcolor = SYSTEM.a(46, 1);
	var tdcolor = SYSTEM.a(46, 2);
	var font = SYSTEM.a(46, 3);
	var fontsize = SYSTEM.a(46, 8);
	if (not fontsize) {
		fontsize = 100;
	}
	if (fontsize and fontsize.isnum()) {
		fontsize ^= "%";
	}
	if (thcolor) {
		css.swapper("#ffff80", thcolor);
	}
	if (tdcolor) {
		css.swapper("#ffffc0", tdcolor);
	}
	css.swapper("neosysfont", font);
	css.swapper("neosyssize", fontsize);
	return 1;
}

function defaultcss() {
return 
"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=EmulateIE7\" />\n"
"<style type=\"text/css\">\n"
"body {background-color:#ffffff; font-family:neosysfont,trebuchet ms,arial,helvetica,sans serif; font-size: neosyssize}\n"
"table,span,div,br,p,a,thread,tbody,tfoot,tr,th,td,b,i,u,dl,dt,dd,li,ul,form,font,small,big {font-size:100%}\n"
"/*\n"
"h1 {font-size:175px;}\n"
"h2 {font-size:150%;}\n"
"h3 {font-size:110%;}\n"
"h4 {font-size:100%;}\n"
"h5 {font-size:75%;}\n"
"h6 {font-size:50%;}\n"
"*/\n"
"a {color:blue;}\n"
"a:visited {color:purple;}\n"
"a:hover {color:red;}\n"
"tt {font-family:courier new,courier;font-size:80%}\n"
"th {background-color:#ffff80;}\n"
"thead {display:table-header-group}\n"
"td.nb {border-bottom:none}\n"
"td.nt {border-top:none}\n"
"td.nx {border-top:none;border-bottom:none}\n"
".neosystable {background-color:#ffffc0; border-width:2px; border-collapse:separate; padding:1px}\n"
"</style>\n"
"\n"
"<style type=\"text/css\" media=\"print\">\n"
".neosystable {border-collapse:collapse;border-color:#bbbbbb}\n"
"td {border-color:#bbbbbb;border-width:1px}\n"
"th {border-color:#bbbbbb;border-width:1px}\n"
"</style>\n"
"\n"
"<script type=\"text/javascript\">\n"
"var togglendisplayed=0\n"
"function toggle(t,mode)\n"
"{\n"
" if (typeof t=='string')\n"
" {\n"
"  if (document.getElementsByClassName)\n"
"   t=document.getElementsByClassName(t)\n"
"  else\n"
"   t=document.getElementsByName(t)\n"
" }\n"
" if (!t.length) return\n"
" if (!t.length) t=[t]\n"
" var display=\"none\"\n"
" for (var i=t.length-1;i>=0;i--) {\n"
"  if (t[i].style.display == \"\") {\n"
"   t[i].style.display = \"none\";\n"
"  } else {\n"
"   t[i].style.display = \"\";\n"
"   display=\"\"\n"
"   }\n"
"  }\n"
" if (!mode)\n"
" {\n"
"  var toggleheading=false\n"
"  if (display=='')\n"
"  {\n"
"   togglendisplayed++\n"
"   if (togglendisplayed==1) toggleheading=true\n"
"  }\n"
"  else\n"
"  {\n"
"   togglendisplayed--\n"
"   if (togglendisplayed==0) toggleheading=true\n"
"  }\n"
"  if (toggleheading) toggle(document.getElementsByName('BHEAD'),true)\n"
" }\n"
"}\n"
"</script>"
;
}
libraryexit()
