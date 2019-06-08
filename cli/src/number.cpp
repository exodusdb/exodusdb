#include <exodus/library.h>
libraryinit()

//var fmtx;
//var input1;//num
//var delim;//num
//var output1;

function main(in type, in input0, in ndecs0, out output) {
	//c sys in,in,in,out

var fmtx;
var input1;//num
var delim;//num
var output1;

	var ndecs = ndecs0;
	var input = input0;
	output = "";

	var zz = "";
	if (ndecs.index("Z")) {
		ndecs.converter("Z", "");
		zz = "Z";
	}

	if (type == "ICONV") {
		var reciprocal = 0;
		if (input[1] == "/") {
			reciprocal = 1;
			input.splicer(1, 1, "");
		}else{
			if (input.substr(1,2) == "1/") {
				reciprocal = 1;
				input.splicer(1, 2, "");
			}
		}

		output = input.trim();

		//first get into a revelation number with dots not commas
		if (BASEFMT.substr(1,2) == "MC") {
			output.converter(",", ".");
		}else{
			output.converter(",", "");
		}
	//nb [NUMBER,X] means no decimal place conversion to be done
		//if ndecs is given then convert to that number of decimals
		// if ndecs starts with a digit then use {NDECS} (use 2 if {NDECS}=null)
		if (ndecs == "") {
			if (DICT) {
				ndecs = calculate("NDECS");
			}else{
				ndecs = BASEFMT[3];
			}
			if (ndecs == "") {
				ndecs = 2;
			}
			if (not(ndecs.match("^\\d$"))) {
			}
			//FMTX='MD':NDECS:'0P'
			//OUTPUT=OUTPUT FMTX
		}
		if ((ndecs == "*") or (ndecs == "X")) {
			ndecs = output.field(".", 2).length();
		}
		if (ndecs == "BASE") {
			fmtx = "MD" ^ BASEFMT[3] ^ "0P";
		}else{
			fmtx = "MD" ^ ndecs ^ "0P";
		}
		output = oconv(output, fmtx);
		STATUS = "";
		if (output.isnum()) {
			if (reciprocal and output) {
				output = ((1 / output).oconv("MD90P")) + 0;
			}
		}else{
			STATUS = 2;
		}

		return 0;
	}

	//oconv
	//////

	var divx = ndecs.field(",", 2);
	if (divx.length()) {
		ndecs = ndecs.field(",", 1);
	}

	var posn = 1;
	while (true) {
		input1=input.substr2(posn, delim);

		var perc = input1[-1];
		if (perc == "%") {
			input1.splicer(-1, 1, "");
		}else{
			perc = "";
		}
		var plus = input1[1];
		if (plus == "+") {
			input1.splicer(1, 1, "");
		}else{
			plus = "";
		}

		if (input.length()) {

			if (divx) {
				input1 = input1 / var(10).pwr(divx);
			}

			if (input1.index("E-")) {
				if (input1.isnum()) {
					input1 = input1.oconv("MD90P");
				}
			}

			var temp = input1;
			temp.converter("0123456789-.", "            ");
			var numlen = input1.length() - temp.trimf().length();
			var unitx = input1.substr(numlen + 1,99);
			var numx = input1.substr(1,numlen);

			if (ndecs == "BASE") {
				output1 = oconv(numx, BASEFMT ^ zz) ^ unitx;
			}else{
				if (ndecs == "") {
					ndecs = numx.field(".", 2).length();
				}

				fmtx = BASEFMT.substr(1,2) ^ ndecs ^ "0P," ^ zz;
				if (numx.isnum()) {
					numx += 0;
				}
				output1 = oconv(numx, fmtx) ^ unitx;

			}

			if (output1.length()) {
				if (var(".,").count(output1[1])) {
					output1.splicer(1, 0, "0");
				}
			}

			output ^= plus ^ output1 ^ perc;
		}

		///BREAK;
		if (not delim) break;
		output ^= var().chr(256 - delim);
	}//loop;

	return 0;
}

libraryexit()
