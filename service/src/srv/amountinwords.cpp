#include <exodus/library.h>
libraryinit()

#include <amountinwords.h>

#include <service_common.h>

#include <srv_common.h>

var comma;
var sentencex;
var amountcurrency;
var currcode;
var amount;	 // num
var language;
var words;
var millions;
var thousands;
var hundreds;
var unitsx;	 // num
var text;
var cents;
var tens;  // num
var ones;  // num
var temp;

function main(in amountfmlanguage, in currcode0) {

	comma = "";

	if (SENTENCE.field(" ", 1) eq "AMOUNT.IN.WORDS") {
		sentencex = SENTENCE;
		SENTENCE  = "";
		printl();
		printl(amountinwords(sentencex.field(" ", 2) ^ FM ^ sentencex.field(" ", 4), sentencex.field(" ", 3)));
		stop();
	}

	amountcurrency = "";
	if (currcode0.unassigned()) {
		currcode = "";
	} else {
		currcode = currcode0;
		if (not(amountcurrency.read(srv.currencies, currcode))) {
			amountcurrency = "";
		}
	}

	amount	 = amountfmlanguage.f(1);
	language = amountfmlanguage.f(2);

	amount.converter("-", "");
	if (amount eq "") {
		return "";
	}
	if (not amount) {
		return "Zero";
	}
	words = "";

	if (language eq "FRENCH") {
		goto french;
	}

	// millions
	// MILLIONS=field(AMOUNT,'.',1)
	// MILLIONS[-6,6]=''
	// convert ',' to '' in amount
	millions = (amount / 1000000).floor();

	if (millions gt 0) {
		words = amountinwords(millions ^ FM ^ language) ^ " Million";
	}

	// thousands
	thousands = ((amount.mod(1000000)) / 1000).floor();
	if (thousands) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(thousands ^ FM ^ language) ^ " Thousand";
	}

	// hundreds
	hundreds = ((amount.mod(1000)) / 100).floor();
	if (hundreds) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(hundreds ^ FM ^ language) ^ " Hundred";
	}

	// units
	unitsx = "00" ^ amount.mod(100).floor();
	text   = "Zero" _VM "One" _VM "Two" _VM "Three" _VM "Four" _VM "Five" _VM "Six" _VM "Seven" _VM "Eight" _VM "Nine" _VM "Ten" _VM "Eleven" _VM "Twelve" _VM "Thirteen" _VM "Fourteen" _VM "Fifteen" _VM "Sixteen" _VM "Seventeen" _VM "Eighteen" _VM "Nineteen" _VM "Twenty" _VM "Thirty" _VM "Forty" _VM "Fifty" _VM "Sixty" _VM "Seventy" _VM "Eighty" _VM "Ninety";
	if (unitsx) {
		if (words ne "") {
			words ^= " and ";
		}
		if (unitsx le 20) {
			words ^= text.f(1, unitsx + 1);
		} else {
			// IF UNITSx LE 20 THEN
			// WORDS:=TEXT<1,UNITSx+1>
			// end else
			words ^= text.f(1, 19 + unitsx[-2]);
			if (not unitsx.ends("0")) {
				words ^= "-" ^ text.f(1, unitsx[-1] + 1);
			}
		}
	}

	// if base.currency='MYR' and currcode='MYR' then
	if (currcode eq "MYR") {
		words.replacer(" and ", " ");
		cents = ((amount - amount.floor()) * 100).oconv("MD00P");
		if (cents) {
			words ^= " and Cents " ^ amountinwords(cents);
		}

	} else if (amountcurrency.f(2)) {
		words ^= " " ^ amountcurrency.f(1);

		// subunits
		if (amount.floor() ne amount) {
			words ^= " and ";
			words ^= (("." ^ amount.field(".", 2)).oconv("MD" ^ amountcurrency.f(3) ^ "0PZ")).cut(1);
			words ^= " " ^ amountcurrency.f(2);
		}

	} else {

		// subunits
		if (amount.floor() ne amount) {
			words ^= " ." ^ amount.field(".", 2);
		}

		if (amountcurrency) {
			words ^= " " ^ amountcurrency.f(1);
		}
	}

	return words.trim();

///////
french:
	// /////
	// millions
	millions = amount.field(".", 1);
	millions.cutter(-6);
	if (millions) {
		words = amountinwords(millions ^ FM ^ language) ^ " million";
		if (millions gt 1) {
			words ^= "s";
		}
	}

	// thousands
	thousands = ((amount.mod(1000000)) / 1000).floor();
	if (thousands) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		if (thousands gt 1) {
			words ^= amountinwords(thousands ^ FM ^ language);
		}
		words ^= " mille";
	}

	// hundreds
	hundreds = ((amount.mod(1000)) / 100).floor();
	if (hundreds) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		if (hundreds gt 1) {
			words ^= amountinwords(hundreds ^ FM ^ language);
		}
		words ^= " cent";
		// IF HUNDREDS GT 1 THEN WORDS:='s'
	}

	// units
	unitsx = "00" ^ amount.mod(100).floor();
	text   = "zero" _VM "un" _VM "deux" _VM "trois" _VM "quatre" _VM "cinq" _VM "six" _VM "sept" _VM "huit" _VM "neuf" _VM "dix" _VM "onze" _VM "douze" _VM "treize" _VM "quatorze" _VM "quinze" _VM "seize" _VM "dix-sept" _VM "dix-huit" _VM "dix-neuf" _VM "vingt" _VM "trente" _VM "quarante" _VM "cinquante" _VM "soixante" _VM "soixante-dix" _VM "quatre-vingt" _VM "quatre-vingt-dix";
	if (unitsx) {
		if (words ne "") {
			words ^= " ";
		}
		if (unitsx le 20) {
			words ^= text.f(1, unitsx + 1);
		} else {

			tens = unitsx[-2];
			ones = unitsx[-1];

			// 70's and 90's
			if (tens eq 9 or tens eq 7) {
				tens -= 1;
				ones += 10;
			}

			// tens
			words ^= text.f(1, 19 + tens);

			// ones
			if (ones ne "0") {
				if (tens and ones eq 1) {
					words ^= " et ";
				} else {
					words ^= "-";
				}
				temp = text.f(1, ones + 1);
				temp.lcaser();
				words ^= temp;
			}
		}
	}

	// subunits
	if (amount.floor() ne amount) {
		words ^= " ." ^ amount.field(".", 2);
	}

	return words.trim();
}

libraryexit()
