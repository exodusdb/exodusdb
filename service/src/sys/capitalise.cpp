#include <exodus/library.h>
libraryinit()

var string2;
var nn;
var numx;
var cap;//num
var inquotes;//num
var ii;//num
var tt;
var toupper;
var quoted;

////////////////////////////////////////
// capitalise.cpp                     //
// NO LONGER USED                     //
// HARD CODED IN exodus/mvprogram.cpp //
// no include statement is required   //
////////////////////////////////////////

function main(io str0, io mode, io wordseps, io oconv_out) {
	//c sys io,io,io,io
	//global all

	//also supports oconv (iconv converts everything to uppercase)
	//str0=type, mode=input, wordseps=mode, oconv_out=output

	if (str0 eq "OCONV") {
		str0 = mode;
		mode = wordseps;
		gosub process(str0, mode, wordseps);
		oconv_out = string2;
		return oconv_out;
	}

	if (str0 eq "ICONV") {
		oconv_out = mode;
		oconv_out.ucaser();
		return oconv_out;
	}

	if (mode.unassigned()) {
		mode = "CAPITALISE";
	}

	gosub process(str0, mode, wordseps);
	return string2;
}

subroutine process(in str0, in mode, io wordseps) {

	if (wordseps.unassigned()) {
		wordseps = " .()&_";
		wordseps ^= _RM _FM _VM _SM _TM _ST;
	}

	if (mode eq "QUOTE") {
		string2 = str0;
		if (string2 ne "") {
			string2.converter(FM ^ VM ^ SM ^ TM, "    ");
			string2.swapper(" ", "\" \"");
			string2 = string2.quote();
		}

	} else if (mode eq "UPPERCASE") {
		string2 = str0;
		string2.ucaser();

	} else if (mode eq "LOWERCASE") {
		string2 = str0;
		string2.lcaser();

	} else if (mode eq "CAPITALISE") {
capitalise:
		string2 = str0;
		if (string2 eq "EXODUS") {
			return;
		}

		//convert @upper.case to @lower.case in string2
		nn = string2.len();
		numx = var("1234567890").contains(string2[1]);
		cap = 1;
		inquotes = 0;
		for (ii = 1; ii <= nn; ++ii) {
			tt = string2[ii];

			if (inquotes) {
				inquotes = tt ne inquotes;
			} else {
				if ((tt eq DQ and string2.count(DQ) gt 1) or ((tt eq "'" and string2.count("'") gt 1))) {
					inquotes = tt;
				} else {
					if (wordseps.contains(tt)) {
						cap = 1;
						if (tt eq " ") {
							numx = var("1234567890").contains(string2[ii + 1]);
						}
					} else {
						if (cap or numx) {
							tt.ucaser();
							string2.splicer(ii, 1, tt);
							cap = 0;
						} else {
							tt.lcaser();
							string2.splicer(ii, 1, tt);
						}
					}
				}
			}

		} //ii;

		string2.swapper("'S ", "'s ");
		if (string2.substr(-2, 2) eq "'S") {
			string2.splicer(-2, 2, "'s");
		}

	} else if (mode.substr(1, 5) eq "PARSE") {

		toupper = mode.contains("UPPERCASE");

		string2 = str0;

		//convert to uppercase
		quoted = "";
		for (ii = 1; ii <= 99999; ++ii) {
			tt = string2[ii];
			///BREAK;
			if (not(tt ne "")) break;
			if (tt eq quoted) {
				quoted = "";
			} else {
				if (not quoted) {
					if ((DQ ^ "'").contains(tt)) {
						quoted = tt;
					} else {
						if (tt eq " ") {
							tt = FM;
							string2.splicer(ii, 1, tt);
						} else {
							if (toupper) {
								tt.ucaser();
								string2.splicer(ii, 1, tt);
							}
						}
					}
				}
			}
		} //ii;

		if (mode.contains("TRIM")) {
			string2.converter(" " _FM, _FM " ");
			string2.trimmer();
			string2.converter(" " _FM, _FM " ");
		}

	} else {
		goto capitalise;
	}

	return;
}

libraryexit()
