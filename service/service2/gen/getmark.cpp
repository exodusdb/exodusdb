#include <exodus/library.h>
libraryinit()

#include <gen.h>

var link;

function main(in mode, in html, io mark) {
	//c gen in,in,io
	//eg
	//call getmark('CLIENT',1,clientmark)
	//call getmark('OWN',1,ownmark)

	//jbase
	if (mark.unassigned()) {
		mark = "";
	}

	//NB <small> tags seem to make no difference if they are removed
	//at least in modern browsers. presumably style:font-size is preferential
	if (mode == "OWN") {

		if (mark) {
			mark ^= " ";
		}
		//S o f t w a r e b y
		mark ^= (var("7962206572617774666F53").iconv("HEX")).substr(-1,-999);
		if (html) {
			mark.splicer(1, 0, "<div style=\"font-size:60%;margin:0px;text-align:left;page-break-before:avoid;page-break-after:avoid\">");
		}

		mark ^= " ";

		mark ^= (var("4D4F432E5359534F454E").iconv("HEX")).substr(-1,-99);

		if (html) {
			mark ^= "</div>";
		}

	} else {
		mark = "";

		if (html) {
			//mark[1,0]='<small>'

			//mark:=' '

			//hyper link to client's email
			link = SYSTEM.a(10, 2);
			if (link) {
				mark ^= "<a href=\"";
				mark ^= "mailto:" ^ link;
				mark ^= "\">";
			}

		}

		//client mark
		if (gen.company.a(27)) {
			mark ^= (gen.company.a(27)).invert();
		}else{
			mark ^= SYSTEM.a(14);
		}

		if (html) {
			if (link) {
				mark ^= "</a>";
			}
			//mark:='</small>'
			if (SYSTEM.a(17).substr(-4,4) == "TEST") {
				mark = "<span style=\"color:red\">" ^ mark ^ " - " ^ SYSTEM.a(17) ^ "</span>";
			}
		}

		mark = "<div style=\"font-size:60%;margin:0px;text-align:left;page-break-before:avoid;page-break-after:avoid\">" ^ mark ^ "</div>";

	}
//L341:
	return 0;

}


libraryexit()
