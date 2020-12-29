#include <exodus/library.h>
libraryinit()

#include <amountinwords.h>

#include <gen_common.h>

var comma;
var sentencex;
var amountcurrency;
var currcode;
var amount;//num
var language;
var words;
var millions;
var thousands;
var hundreds;
var unitsx;//num
var text;
var cents;
var tens;//num
var ones;//num
var temp;

function main(in amountfmlanguage, in currcode0) {
	//c sys
	//global all

	#include <general_common.h>

	comma = "";

	if (SENTENCE.field(" ", 1) eq "AMOUNT.IN.WORDS") {
		sentencex = SENTENCE;
		SENTENCE = "";
		printl();
		printl(amountinwords(sentencex.field(" ", 2) ^ FM ^ sentencex.field(" ", 4), sentencex.field(" ", 3)));
		stop();
	}

	amountcurrency = "";
	if (currcode0.unassigned()) {
		currcode = "";
	}else{
		currcode = currcode0;
		if (not(amountcurrency.read(gen.currencies, currcode))) {
			{}
		}
	}

	amount = amountfmlanguage.a(1);
	language = amountfmlanguage.a(2);

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

	//millions
	//MILLIONS=field(AMOUNT,'.',1)
	//MILLIONS[-6,6]=''
	// convert ',' to '' in amount
	millions = (amount / 1000000).floor();

	if (millions gt 0) {
		words = amountinwords(millions ^ FM ^ language) ^ " Million";
	}

	//thousands
	thousands = ((amount % 1000000) / 1000).floor();
	if (thousands) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(thousands ^ FM ^ language) ^ " Thousand";
	}

	//hundreds
	hundreds = ((amount % 1000) / 100).floor();
	if (hundreds) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(hundreds ^ FM ^ language) ^ " Hundred";
	}

	//units
	unitsx = "00" ^ (amount % 100).floor();
	text = "Zero" _VM_ "One" _VM_ "Two" _VM_ "Three" _VM_ "Four" _VM_ "Five" _VM_ "Six" _VM_ "Seven" _VM_ "Eight" _VM_ "Nine" _VM_ "Ten" _VM_ "Eleven" _VM_ "Twelve" _VM_ "Thirteen" _VM_ "Fourteen" _VM_ "Fifteen" _VM_ "Sixteen" _VM_ "Seventeen" _VM_ "Eighteen" _VM_ "Nineteen" _VM_ "Twenty" _VM_ "Thirty" _VM_ "Forty" _VM_ "Fifty" _VM_ "Sixty" _VM_ "Seventy" _VM_ "Eighty" _VM_ "Ninety";
	if (unitsx) {
		if (words ne "") {
			words ^= " and ";
		}
		if (unitsx le 20) {
			words ^= text.a(1, unitsx + 1);
		}else{
		//IF UNITSx LE 20 THEN
		// WORDS:=TEXT<1,UNITSx+1>
		//end else
			words ^= text.a(1, 19 + unitsx[-2]);
			if (unitsx[-1] ne "0") {
				words ^= "-" ^ text.a(1, unitsx[-1] + 1);
			}
		}
	}

	//if base.currency='MYR' and currcode='MYR' then
	if (currcode eq "MYR") {
		words.swapper(" and ", " ");
		cents = ((amount - amount.floor()) * 100).oconv("MD00P");
		if (cents) {
			words ^= " and Cents " ^ amountinwords(cents);
		}

	} else if (amountcurrency.a(2)) {
		words ^= " " ^ amountcurrency.a(1);

		//subunits
		if (amount.floor() ne amount) {
			words ^= " and ";
			words ^= (("." ^ amount.field(".", 2)).oconv("MD" ^ amountcurrency.a(3) ^ "0PZ")).substr(2,9999);
			words ^= " " ^ amountcurrency.a(2);
		}

	}else{

		//subunits
		if (amount.floor() ne amount) {
			words ^= " ." ^ amount.field(".", 2);
		}

		if (amountcurrency) {
			words ^= " " ^ amountcurrency.a(1);
		}

	}

	return words.trim();

///////
french:
///////
	//millions
	millions = amount.field(".", 1);
	millions.splicer(-6, 6, "");
	if (millions) {
		words = amountinwords(millions ^ FM ^ language) ^ " million";
		if (millions gt 1) {
			words ^= "s";
		}
	}

	//thousands
	thousands = ((amount % 1000000) / 1000).floor();
	if (thousands) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		if (thousands gt 1) {
			words ^= amountinwords(thousands ^ FM ^ language);
		}
		words ^= " mille";
	}

	//hundreds
	hundreds = ((amount % 1000) / 100).floor();
	if (hundreds) {
		if (words ne "") {
			words ^= comma ^ " ";
		}
		if (hundreds gt 1) {
			words ^= amountinwords(hundreds ^ FM ^ language);
		}
		words ^= " cent";
		//IF HUNDREDS GT 1 THEN WORDS:='s'
	}

	//units
	unitsx = "00" ^ (amount % 100).floor();
	text = "zero" _VM_ "un" _VM_ "deux" _VM_ "trois" _VM_ "quatre" _VM_ "cinq" _VM_ "six" _VM_ "sept" _VM_ "huit" _VM_ "neuf" _VM_ "dix" _VM_ "onze" _VM_ "douze" _VM_ "treize" _VM_ "quatorze" _VM_ "quinze" _VM_ "seize" _VM_ "dix-sept" _VM_ "dix-huit" _VM_ "dix-neuf" _VM_ "vingt" _VM_ "trente" _VM_ "quarante" _VM_ "cinquante" _VM_ "soixante" _VM_ "soixante-dix" _VM_ "quatre-vingt" _VM_ "quatre-vingt-dix";
	if (unitsx) {
		if (words ne "") {
			words ^= " ";
		}
		if (unitsx le 20) {
			words ^= text.a(1, unitsx + 1);
		}else{

			tens = unitsx[-2];
			ones = unitsx[-1];

			//70's and 90's
			if (tens eq 9 or tens eq 7) {
				tens -= 1;
				ones += 10;
			}

			//tens
			words ^= text.a(1, 19 + tens);

			//ones
			if (ones ne "0") {
				if (tens and ones eq 1) {
					words ^= " et ";
				}else{
					words ^= "-";
				}
				temp = text.a(1, ones + 1);
				temp.lcaser();
				words ^= temp;
			}

		}
	}

	//subunits
	if (amount.floor() ne amount) {
		words ^= " ." ^ amount.field(".", 2);
	}

	return words.trim();
}

libraryexit()
