#include <exodus/library.h>
libraryinit()

#include <addcent.h>

var inx;//num
var xx;

function main(in type, in in0, in mode, out outx) {
	//c gen in,in,in,out

	//y2k2
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

		} else if (inx.length() == 4) {

			if (inx.substr(1,2) == "20") {

				if (inx.substr(-2,2) > 12) {
					//2013,14,... -> 13,14,... (2 digit year)
					outx = inx.substr(-2,2);
				//case date()<18629;*1/1/2019
				} else if (var().date() < 18994) {
					//up to 1/1/2019
					//2001 ... 2012 -> 00 ... 12 (ie 2 digit year until we are in 1/1/2019)
					outx = inx.substr(-2,2);
				} else {
					//FROM 1/1/2019
					//2001 ... 2012 -> 2001 ... 2012 (ie 2020.01 ... 2020.12 not 2 digit year "12" sadly)
				}
//L255:
				//dummy comment to assist c++ decomp
			}else{

				//1412 -> 1412
				outx = inx;
			}

		} else {
			//201412 -> 1412
			outx = inx.substr(-4,4);
		}
//L290:
		//prefix=''
		//goto exit
		return outx;
	}

	if (inx.index(".", 1)) {
		var prefix = (inx.field(".", 1)).oconv("[ADDCENT]") ^ ".";
		outx = prefix ^ inx.field(".", 2);
		//goto exit
		return outx;
	}

	inx.converter("-/", "  ");
	if (inx.count(" ") == 1) {
		inx.converter(" ", "/");
	}

	//four numbers separated by spaces is period range
	if (inx.count(" ") == 3) {
		inx = inx.field(" ", 1) ^ "/" ^ inx.field(" ", 2) ^ "-" ^ inx.field(" ", 3) ^ "/" ^ inx.field(" ", 4);
	}

	var prefix = "";

	//range of periods 1/98-3/98
	if (inx.index("-", 1)) {
		outx = addcent(inx.field("-", 1), "", "", xx);
		outx ^= "-" ^ addcent(inx.field("-", 2), "", "", xx);
		//goto exit
		return outx;
	}

	//remove any prefixed period number '1/99'
	if (inx.index("/", 1)) {
		prefix = inx.field("/", 1) ^ "/";
		inx = inx.field("/", 2);
	}

	if (inx.length() == 1) {
		inx = "0" ^ inx;
	}

	var in2 = inx.substr(1,2);

	if (inx.length() < 4) {
addcent:
		if (in2 < 30) {
			outx = "20" ^ inx;
		}else{
			outx = "19" ^ inx;
		}

		if (not mode.unassigned()) {
			if (mode == ".") {
				outx.splicer(5, 0, ".");
			}
		}

	}else{

		//convert year period YYMM to YYYYMM except 19XX and 20XX
		//NO LONGER. 19xx and 20xx now converted to 2019xx and 2020xx
		//were not converted before patch in Jan 2019
		if (inx.length() == 4) {
			//if in2<>'19' and in2<>'20' then goto addcent
			if (in2 ne "20") {
				goto addcent;
			}
			//goto addcent
		}

		outx = inx;
	}

	//exit:
	//////

	outx = prefix ^ outx;

	//if called direct (not oconv) then return result
	if (type ne "OCONV" and type ne "ICONV") {
		return outx;
	}

	//make consistent function so C++ doesnt complain
	//return
	return outx;

}


libraryexit()
