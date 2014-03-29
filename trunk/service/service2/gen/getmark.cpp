#include <exodus/library.h>
libraryinit()

#include <gen.h>

var link;

function main(in mode, in html, io mark) {
	//jbase
	if (mark.unassigned()) {
		mark = "";
	}

	if (mode == "OWN") {

		if (mark) {
			mark ^= " ";
		}
		//S#o#f#t#w#a#r#e#b#y
		mark ^= var("7962206572617774666F53").iconv("HEX2").substr(-1,-999);
		if (html) {
			mark.splicer(1, 0, "<span style=\"font-size:70%;margin-top:0;page-break-before:avoid;page-break-after:avoid\"><small>");
		}

		mark ^= " ";
		mark ^= var("4D4F432E5359534F454E").iconv("HEX2").substr(-1,-99);

		if (html) {
			mark ^= "</small></span>";
		}

	} else {

		if (html) {
			mark.splicer(1, 0, "<span style=\"font-size:70%;margin-bottom:0;page-break-before:avoid;page-break-after:avoid\"><small>");
			mark ^= " ";
			link = SYSTEM.a(10, 2);
			//hyper link to client's email
			if (link) {
				mark ^= "<a href=\"";
				mark ^= "mailto:" ^ link;
				mark ^= "\">";
			}
		}

		//client mark
		if (gen.company.a(27)) {
			mark ^= (gen.company.a(27));//.invert();
		}else{
			mark ^= SYSTEM.a(14);
		}

		if (html) {
			if (link) {
				mark ^= "</a>";
			}
			mark ^= "</small></span>";
			if (SYSTEM.a(17).substr(-4,4) == "TEST") {
				mark = "<font color=red>" ^ mark ^ " - " ^ SYSTEM.a(17) ^ "</font>";
			}
		}
	}

	return 0;

}

libraryexit()
