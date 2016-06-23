#include <exodus/library.h>
libraryinit()

#include <addcent.h>//recusive

function main(in type, in in0="", in mode="") {
	var inx;//num
	var outx;
	var in2;
	var prefix;
	
	if ( false && mode ) {}
	
	//y2k2
	//j b a s e
	//cater for direct call (not via oconv)
	if (type ne "OCONV" and type ne "ICONV") {
		inx = type;
	}else{
		inx = in0;
	}
	if (inx == "") {
		 outx = "";
		return "";
	}
	if (inx.isnum()) {
		if (inx < 0) {
			inx += 2000;
		}
	}

	if (type == "ICONV") {
		if (inx.index("/", 1)) {
			//12/2014 -> 12/14
			var mth = inx.field("/", 1);
			var yr = inx.field("/", 2);
			outx = mth ^ "/" ^ yr.substr(-2,2);
		} else if (inx.index(".", 1)) {
			var yr = inx.field(".", 1);
			var mth = inx.field(".", 2);
			outx = yr.substr(-2,2) ^ "." ^ mth;
		}else{
			//201412 and 1412 -> 1412
			outx = inx.substr(-4,4);
		}
		prefix = "";
		goto exit;
	}

	if (inx.index(".", 1)) {
		prefix = (inx.field(".", 1)).oconv("[ADDCENT]") ^ ".";
		outx = inx.field(".", 2);
		goto exit;
	}

	inx.converter("-/", "  ");
	if (inx.count(" ") == 1) {
		inx.converter(" ", "/");
	}

	//four numbers separated by spaces is period range
	if (inx.count(" ") == 3) {
		inx = inx.field(" ", 1) ^ "/" ^ inx.field(" ", 2) ^ "-" ^ inx.field(" ", 3) ^ "/" ^ inx.field(" ", 4);
	}

	prefix = "";

	//range of periods 1/98-3/98
	if (inx.index("-", 1)) {
		outx = addcent(inx.field("-", 1));
		outx ^= "-" ^ addcent(inx.field("-", 2));
		goto exit;
	}

	//remove any prefixed period number '1/99'
	if (inx.index("/", 1)) {
		prefix = inx.field("/", 1) ^ "/";
		inx = inx.field("/", 2);
	}

	if (inx.length() == 1) {
		inx = "0" ^ inx;
	}

	in2 = inx.substr(1,2);

	if (inx.length() < 4) {
addcent:
		if (in2 < 30) {
			outx = "20" ^ inx;
		}else{
			outx = "19" ^ inx;
		}

	}else{

		//convert year period YYMM to YYYYMM except 19XX and 20XX
		if (inx.length() == 4) {
			if (in2 ne "19" and in2 ne "20") {
				goto addcent;
			}
		}

		outx = inx;
	}

exit:
	/////

	outx = prefix ^ outx;

	//if called direct (not oconv) then return result
	if (type ne "OCONV" and type ne "ICONV") {
		return outx;
	}

	//make consistent function so C++ doesnt complain
	//return 0
	return outx;

}


libraryexit()
