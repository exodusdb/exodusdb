#include <exodus/library.h>
libraryinit()

#include <addcent.h>//recusive

var prefix;
var out0;
var in2;

function main(in type) {
	//y2k2
	//j b a s e
	//cater for direct call (not via oconv)
	//if (type ne "OCONV" and type ne "ICONV") {
	//	in0 = type;
	//}
	var in0=type;

	if (in0 == "") {
		var out0 = "";
		return "";
	}
	if (in0.isnum()) {
		if (in0 < 0) {
			in0 += 2000;
		}
	}

	if (in0.index(".", 1)) {
		var prefix = (in0.field(".", 1)).oconv("[ADDCENT]") ^ ".";
		var out0 = in0.field(".", 2);
		goto exit;
	}

	in0.converter("-/", "  ");
	if (in0.count(" ") == 1) {
		in0.converter(" ", "/");
	}

	//four numbers separated by spaces is period range
	if (in0.count(" ") == 3) {
		in0 = in0.field(" ", 1) ^ "/" ^ in0.field(" ", 2) ^ "-" ^ in0.field(" ", 3) ^ "/" ^ in0.field(" ", 4);
	}

	prefix = "";

	//range of periods 1/98-3/98
	if (in0.index("-", 1)) {
		out0 = addcent(in0.field("-", 1));
		out0 ^= "-" ^ addcent(in0.field("-", 2));
		goto exit;
	}

	//remove any prefixed period number '1/99'
	if (in0.index("/", 1)) {
		prefix = in0.field("/", 1) ^ "/";
		in0 = in0.field("/", 2);
	}

	if (in0.length() == 1) {
		in0 = "0" ^ in0;
	}

	in2 = in0.substr(1, 2);

	if (in0.length() < 4) {
addcent:
		if (in2 < 30) {
			out0 = "20" ^ in0;
		}else{
			out0 = "19" ^ in0;
		}

	}else{

		//convert year period YYMM to YYYYMM except 19XX and 20XX
		if (in0.length() == 4) {
			if (in2 ne "19" and in2 ne "20") {
				goto addcent;
			}
		}

		out0 = in0;
	}

exit:
	/////

	out0 = prefix ^ out0;

	//if called direct (not oconv) then return result
	if (type ne "OCONV" and type ne "ICONV") {
		return out0;
	}

	//make consistent function so C++ doesnt complain
	//return 0
	return out0;

}


libraryexit()
