#include <exodus/library.h>

libraryinit(authorised)
//---------------------
#include <authorised.h>

var positive;

function main() {
	//ALLOWPARTIALACCESS
	if (authorised("COMPANY ACCESS")) {
		positive = "";
	}else{
		positive = "#";
	}
	return authorised(positive ^ "COMPANY ACCESS " ^ (ID.quote()));
}
libraryexit(authorised)

libraryinit(company_code2b)
//-------------------------
function main() {
	var compcode2 = RECORD.a(28);
	if (compcode2 == "") {
		if (ID.match("^[A-Za-z]*$")) {
			return ID;
		}
	}
	return compcode2;
}
libraryexit(company_code2b)

libraryinit(company_code_1st2chars)
//---------------------------------
function main() {
	return ID.substr(1,2);
}
libraryexit(company_code_1st2chars)

libraryinit(current_year)
//-----------------------
function main() {
	ANS = oconv(RECORD.a(2).field("/", 2), "[ADDCENT]");
	/*pgsql;
		ans := '20' || split_part( split_part(data,FM,2),'/',2);;
	*/
	return ANS;
}
libraryexit(current_year)

libraryinit(letterhead_image)
//---------------------------
var coln;

function main() {
	var imagetypes = RECORD.a(62, MV);
	var nvs = imagetypes.count(VM) + 1;
	var images = "";
	for (var vn = 1; vn <= nvs; ++vn) {
		var imagetype = imagetypes.a(1, vn);
		var imagecompcode = ID;
		if (imagetype) {
			if (MV) {
				coln = MV;
			}else{
				coln = vn;
			}
			images.r(1, vn, "logo_" ^ imagecompcode ^ "_" ^ coln ^ "." ^ imagetype);
			}
	};//vn;
	return images;
}
libraryexit(letterhead_image)

libraryinit(mark)
//---------------
function main() {
	return RECORD.a(27).invert();
}
libraryexit(mark)

libraryinit(other_names)
//----------------------
function main() {
	return calculate("OTHER_COMPANIES").xlate("COMPANIES", 1, "X");
}
libraryexit(other_names)

libraryinit(prelist)
//------------------
#include <acno2.h>

var intercompanyaccs;

function main() {
	//remove internal account numbers
	RECORD.r(4, RECORD.a(4, 1, 1));
	RECORD.r(5, RECORD.a(5, 1, 1));
	RECORD.r(12, RECORD.a(12, 1, 1));
	RECORD.r(19, RECORD.a(19, 1, 1));

	var intercompanycodes = RECORD.a(7);
	intercompanycodes.swapper(VM, ", ");
	RECORD.r(7, intercompanycodes);

	call acno2("OCONV", RECORD.a(8), "", intercompanyaccs);
	intercompanyaccs.swapper(VM, ", ");
	RECORD.r(8, intercompanyaccs);

	//show last version only
	var lastvn = RECORD.a(40).count(VM) + 1;
	for (var fn = 40; fn <= 44; ++fn) {
		RECORD.r(fn, RECORD.a(fn, lastvn));
	};//fn;
	return ANS;
}
libraryexit(prelist)
