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
var lastwords;
var millions;
var thousands;
var hundreds;
var unitsx;	 // num
var subunitsx;  // num
var text;
var cents;
var tens;  // num
var ones;  // num
var temp;
var unitname;
var subunitname;

function main(in amountfmlanguage, in currcode0) {

	comma = "";

	if (SENTENCE.field(" ", 1) == "amountinwords") {
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
		if (not amountcurrency.read(srv.currencies, currcode)) {
			amountcurrency = "";
		}
	}

	amount	 = amountfmlanguage.f(1);
	language = amountfmlanguage.f(2);
	language.ucaser();

	amount.converter("-", "");
	if (amount == "") {
		return "";
	}
	if (not amount) {
		return "Zero";
	}
	words = "";

    if (language.contains("ARABIC")) {
        goto arabic;
    }

	if (language == "FRENCH") {
		goto french;
	}

	// millions
	// MILLIONS=field(AMOUNT,'.',1)
	// MILLIONS[-6,6]=''
	// convert ',' to '' in amount
	millions = (amount / 1000000).floor();

	if (millions > 0) {
		words = amountinwords(millions ^ FM ^ language) ^ " Million";
	}

	// thousands
	thousands = ((amount.mod(1000000)) / 1000).floor();
	if (thousands) {
		if (words != "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(thousands ^ FM ^ language) ^ " Thousand";
	}

	// hundreds
	hundreds = ((amount.mod(1000)) / 100).floor();
	if (hundreds) {
		if (words != "") {
			words ^= comma ^ " ";
		}
		words ^= amountinwords(hundreds ^ FM ^ language) ^ " Hundred";
	}

	// units
	unitsx = "00" ^ amount.mod(100).floor();
	text   = "Zero" _VM "One" _VM "Two" _VM "Three" _VM "Four" _VM "Five" _VM "Six" _VM "Seven" _VM "Eight" _VM "Nine" _VM "Ten" _VM "Eleven" _VM "Twelve" _VM "Thirteen" _VM "Fourteen" _VM "Fifteen" _VM "Sixteen" _VM "Seventeen" _VM "Eighteen" _VM "Nineteen" _VM "Twenty" _VM "Thirty" _VM "Forty" _VM "Fifty" _VM "Sixty" _VM "Seventy" _VM "Eighty" _VM "Ninety";
	if (unitsx) {
		if (words != "") {
			words ^= " and ";
		}
		if (unitsx <= 20) {
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
	if (currcode == "MYR") {
		words.replacer(" and ", " ");
		cents = ((amount - amount.floor()) * 100).oconv("MD00P");
		if (cents) {
			words ^= " and Cents " ^ amountinwords(cents);
		}

	} else if (amountcurrency.f(2)) {
		//words ^= " " ^ amountcurrency.f(1);
		words ^= " " ^ amountcurrency.f(1).field("|", 1);

		// subunits
		if (amount.floor() != amount) {
			words ^= " and ";
			words ^= (("." ^ amount.field(".", 2)).oconv("MD" ^ amountcurrency.f(3) ^ "0PZ")).cut(1);
			//words ^= " " ^ amountcurrency.f(2);
			// Get english currency name
			words ^= " " ^ amountcurrency.f(2).field("|", 1);
		}

	} else {

		// subunits
		if (amount.floor() != amount) {
			words ^= " ." ^ amount.field(".", 2);
		}

		if (amountcurrency) {
			//words ^= " " ^ amountcurrency.f(1);
			// Get english currency name
			words ^= " " ^ amountcurrency.f(1).field("|", 1);
		}
	}

	return words.trim();

///////
arabic:
///////

	// Warning! arguments in replacer are in wrong order due to arabic left to right
	// example, to replace X with Y, do replacer(Y, X)

	// Get arabic currency name
    if (amountcurrency) {
        unitname    = amountcurrency.f(1).field("|", 2);
        subunitname = amountcurrency.f(2).field("|", 2);

        if (words == "") {
            words ^= " " ^ unitname ^ " ";
        }
    }

    // Add word "million"
    millions = amount.field(".", 1);
    millions.cutter(-6);
    if (millions) {
        words ^= amountinwords(millions ^ FM ^ language, "" );

        if (millions == 2) {

			// Special arabic word for two million
            words ^= " مليونان ";

			// remove the word two, because مليونان is a special word for "two million"
            words.replacer("اثنان مليونان" , "مليونان");
            if (words != "")
                words ^= " و";

        } else {
            // Everything else e.g 3,4 millions
            words ^= " مليون ";

            // remove "one" from "one million"
            words.replacer("واحد مليون" , "مليون");
            if (words != "")
                words ^= " و";
        }
    }

    // Add word "thousand"
    thousands = ((amount.mod(1000000)) / 1000).floor();
    if (thousands) {

        words ^= amountinwords(thousands ^ FM ^ language);

        if (thousands == 2) {

			// Special arabic word for two thousand
            words ^= " ألفان ";

			// remove the word two, because مليونان is a special word for "two thousand"
            words.replacer("اثنان ألفان" , "ألفان");
            if (words != "")
                words ^= " و";

        } else {

            // Everything else e.g 3,4 thousands
            words ^= " ألف ";

            // remove "one" from "one thousand"
            words.replacer("واحد ألف" , "ألف");
            if (words != "")
                words ^= " و";
        }
    }

    // Add word "hundred"
    hundreds = ((amount.mod(1000)) / 100).floor();
    if (hundreds) {
        words ^= amountinwords(hundreds ^ FM ^ language);

        if (hundreds == 2) {

			// Special arabic word for two hundred
            words ^= " مائتان";

			// remove the word two, because مليونان is a special word for "two hundred"
            words.replacer("اثنان مائتان" , "مائتان");
            if (words != "")
                words ^= " و";

        } else {

            // Everything else e.g 3,4 hundreds
            words ^= " مائة";

            // remove "one" from "one hundred"
            words.replacer("واحد مائة" , "مائة");
            if (words != "")
                words ^= " و";
        }
    }

    // Get units in words
    unitsx = "00" ^ amount.mod(1000).floor();
    text   = "صفر " _VM "واحد " _VM "اثنان " _VM "ثلاثة " _VM "أربعة " _VM "خمسة " _VM "ستة " _VM "سبعة " _VM "ثمانية " _VM "تسعة " _VM "عشرة " _VM "أحد عشر " _VM "اثنا عشر " _VM "ثلاثة عشر " _VM "أربعة عشر " _VM "خمسة عشر " _VM "ستة عشر " _VM "سبعة عشر " _VM "ثمانية عشر " _VM "تسعة عشر " _VM "عشرين " _VM "ثَلاثين " _VM "أربعين " _VM "خمسين " _VM "ستين " _VM "سبعين " _VM "ثمانين " _VM "تسعين ";
    if (unitsx) {
        if (unitsx <= 20) {
            words ^= text.f(1, unitsx + 1);
        } else {

            tens     = unitsx[-2];
            ones     = unitsx[-1];

            // ones
            if (ones != "0") {
                words ^= text.f(1, 1 + ones);
            }

            // tens
            if (tens != "0") {
                if (words != "" and ones != "0")
                    words ^= " و";
                words ^= text.f(1, 19 + tens);
            }
        }
        //Currency unit name in Arabic
        unitname = amountcurrency.f(1).field("|", 2);
        if (unitname) {
            // Move dirhams to end of string
            words.replacer(unitname, "" );
            words ^= unitname;
        }
    }

    // Get subunits in words
    if (amount.floor() != amount) {
        subunitsx = amount.field(".", 2);

        if (subunitsx.len()) {

            if (subunitsx <= 20) {
                // fils will always start with an "and"
                words ^= "و ";
                words ^= text.f(1, subunitsx + 1);

            } else {

                if (subunitsx >= 100) {
                    // tenth
                    hundreds = subunitsx[+1];
                    // hundredth
                    tens     = subunitsx[+2];
                    // thousandth
                    ones     = subunitsx[+3];
                } else {

                    hundreds = "";
                    tens     = subunitsx[+1];
                    ones     = subunitsx[+2];
                }

                // hundreds
                if (hundreds and (hundreds != "0")) {
                    words ^= " و";
                    words ^= text.f(1, 1 + hundreds);
                    words ^= " مائة ";
                }

                // ones
                if (ones != "0") {
                    words ^= " و";
                    words ^= text.f(1, 1 + ones);
                }

                // tens
                if (tens and (tens != "0")) {
                    words ^= " و";
                    words ^= text.f(1, 19 + tens);
                }
            }

            //Currency sub-unit name in Arabic
            subunitname = amountcurrency.f(2).field("|", 2);
            //fils
            words ^= subunitname;
        }
    }

    return words.trim();

///////
french:
///////
	// millions
	millions = amount.field(".", 1);
	millions.cutter(-6);
	if (millions) {
		words = amountinwords(millions ^ FM ^ language) ^ " million";
		if (millions > 1) {
			words ^= "s";
		}
	}

	// thousands
	thousands = ((amount.mod(1000000)) / 1000).floor();
	if (thousands) {
		if (words != "") {
			words ^= comma ^ " ";
		}
		if (thousands > 1) {
			words ^= amountinwords(thousands ^ FM ^ language);
		}
		words ^= " mille";
	}

	// hundreds
	hundreds = ((amount.mod(1000)) / 100).floor();
	if (hundreds) {
		if (words != "") {
			words ^= comma ^ " ";
		}
		if (hundreds > 1) {
			words ^= amountinwords(hundreds ^ FM ^ language);
		}
		words ^= " cent";
		// IF HUNDREDS GT 1 THEN WORDS:='s'
	}

	// units
	unitsx = "00" ^ amount.mod(100).floor();
	text   = "zero" _VM "un" _VM "deux" _VM "trois" _VM "quatre" _VM "cinq" _VM "six" _VM "sept" _VM "huit" _VM "neuf" _VM "dix" _VM "onze" _VM "douze" _VM "treize" _VM "quatorze" _VM "quinze" _VM "seize" _VM "dix-sept" _VM "dix-huit" _VM "dix-neuf" _VM "vingt" _VM "trente" _VM "quarante" _VM "cinquante" _VM "soixante" _VM "soixante-dix" _VM "quatre-vingt" _VM "quatre-vingt-dix";
	if (unitsx) {
		if (words != "") {
			words ^= " ";
		}
		if (unitsx <= 20) {
			words ^= text.f(1, unitsx + 1);
		} else {

			tens = unitsx[-2];
			ones = unitsx[-1];

			// 70's and 90's
			if (tens == 9 or tens == 7) {
				tens -= 1;
				ones += 10;
			}

			// tens
			words ^= text.f(1, 19 + tens);

			// ones
			if (ones != "0") {
				if (tens and ones == 1) {
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
	if (amount.floor() != amount) {
		words ^= " ." ^ amount.field(".", 2);
	}

	return words.trim();
}

libraryexit()
