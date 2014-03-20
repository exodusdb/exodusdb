#include <exodus/library.h>
libraryinit()

#include <agy.h>

var rounding;//num
var gross;//num
var perc;//num
var amount;//num
var grossbillplus1;

function main(io grossbill, in size, in extrasx, in unused, io msg, in roundingx, io mult, io amounts) {
	//MUST be kept in EXACT/CAREFUL sync with function schedule_calcbill in client
	amounts = "";
	//y2k *jbase
	var ndecs = (grossbill.field(".", 2)).length();
	var currfmt = "MD" ^ ndecs ^ "0P";

	if (false) {
		var x=unused;
		x=roundingx;
	}

	//new method is to round on each calculation
	//only really effects bahrain and amounts with three decimal places
	//roundfinally=0;* or 1 to agree with how the intranet works
	var roundfinally = 1;
/*
	if (rounding.unassigned()) {
		roundingx = agy.agp.a(32, 1);
		rounding = roundingx;
		//if rounding='' and ndecs=3 then rounding=.1
		if (rounding == "" and ndecs == 3) {
			rounding = mv.PRIORITYINT.a(101, 1);
		}
	}else{
		rounding = roundingx;
	}
*/
	msg = "";

	var extras = extrasx;
	extras.swapper(",", ",\x0B");
	extras.converter("/Xx", ",");

	//multiply by size if any
	/*;
		T=@UPPER.CASE:@LOWER.CASE;
		CONVERT 'Xx' TO '' IN T;
		CONVERT T TO '' IN SIZE;
		if index(size,'X',1) and num(field(size,'X',1)) and num(field(size,'X',2)) then;
		//IF SIZE MATCHES '1N0N"X"1N0N' THEN
			mult=FIELD(SIZE,'X',1)*FIELD(SIZE,'X',2);
			grossbill=OCONV(grossbill*mult,CURRFMT);
			//NB return grossbill as sized
		end else;
			mult=1;
			end;
	*/
	var size2 = size;
	mult = 1;
	if (size2) {
		size2.converter("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "                          ");
		size2.trimmer();
		var sizea = size2.field(" ", 1);
		var sizeb = size2.field(" ", 2);
		if (sizea and sizeb and sizea.isnum() and sizeb.isnum()) {
			mult = sizea * sizeb;
			//garbagecollect;
			grossbill = (grossbill * mult).oconv(currfmt);
			//NB return grossbill as sized
		}
	}

	//initialise unit bill to be same as gross unit bill
	var netbill = grossbill;
	var prevnetbill = netbill;

	//subtract/add any extras
	//allow spaces so comments come after first space
	//CONVERT ' ' TO '' IN EXTRAS

	//*various types of free ads
	//t=extras[1,1]
	//*convert @lower.case to @upper.case in t
	//if t matches '1A' then extras='-100%'

	if (extras) {
		var nextras = extras.count(",") + 1;
		var breakn = 1;
		for (var extran = 1; extran <= nextras; ++extran) {
			//EXTRA=field(FIELD(EXTRAS,',',EXTRAN),' ',1)
			var extra = extras.field(",", extran).trim().field(" ", 1);

			//commas indicate breaks but slashes within commas are converted to commas
			if (extra.substr(1, 1) == 0x1B) {
				extra.splicer(1, 1, "");
				breakn += 1;
			}

			//various types of free ads
			//t=extra[1,1]
			//convert @lower.case to @upper.case in t
			if ((extra.substr(1, 1)).match("1A")) {
				extra = "-100%";
			}

			var origextra;
			var plus1;
			var but1;
			var minimum;

			//if extra<>'' then locate extra in 'FREE?free?Free?'<1> setting x then extra='-100%'
			//if ucase(extra)='FREE' then extra='-100%'
			if (extra.substr(1, 1) == "=") {
				if ((extra.substr(2, 9999)).isnum()) {
					prevnetbill = netbill;
					netbill = extra.substr(2, 9999);
					goto gotnewnet;
				}
			}
			if (extra) {

				origextra = extra;

				if (extra.substr(1, 1) == "+") {
					extra.splicer(1, 1, "");
				}

				plus1 = 0;
				but1 = 0;
				if (extra.index("G", 1)) {
					gross = 1;
				}else{
					if (extra.index("g", 1)) {
						gross = 1;
						if (extran > 1) {
							plus1 = 1;
						}
					}else{
						gross = 0;
						if (extran > 1 and extra.index("n", 1)) {
							but1 = 1;
						}
					}
				}
				//Xx not needed but here for safety?
				extra.converter("NnGgXx", "");

				minimum = "";
				if (extra.index(">", 1)) {
					minimum = extra.field(">", 2);
					if (not minimum.isnum()) {
						minimum = "";
						msg.r(-1, DQ ^ (origextra ^ DQ) ^ " - MINIMUM SHOULD BE NUMERIC");
					}
					extra = extra.field(">", 1);
				}

				if (extra.substr(-1, 1) == "%") {
					perc = 1;
					extra.splicer(-1, 1, "");
				}else{
					perc = 0;
				}

				if (extra.isnum()) {
					if (perc) {

						//decide what to calculate the percentage on
						if (gross) {
							//IF unused THEN
							//removed on 28/02/01 so that g can mean on one after gross at I/B Cairo
							if (0) {
								//removed because 100,-5%,-20%,+3.75%G should=79.75 not 79.56
								//notified by kumar/impact 22/10/2000
								//previously fee on "gross" always mean gross of comm but not of discount
								//now we can indicate g to mean this and G means on real gross
								//IF EXTRAN=3 THEN
								// amount=PREV.netbill
								//END ELSE
								amount = grossbill;
								// END
							}else{
								if (plus1) {
									amount = grossbillplus1;
								}else{
									amount = grossbill;
								}
							}
						}else{
							if (but1) {
								amount = prevnetbill;
							}else{
								amount = netbill;
							}
						}

						prevnetbill = netbill;
						amount = amount * extra / 100;
						if (rounding and not roundfinally) {
							amount = (((amount / rounding).oconv("MD00P")) * rounding).oconv(currfmt);
						}else{
							amount = amount.oconv(currfmt);
						}
					}else{
						prevnetbill = netbill;
						amount = extra;
					}

					if (minimum) {
						if (amount < minimum) {
							amount = minimum;
						}
					}

					netbill += amount;
					amounts.r(breakn, (amounts.a(breakn) + amount).oconv(currfmt));

gotnewnet:
					netbill = netbill.oconv(currfmt);
				}else{
					msg.r(-1, DQ ^ (origextra ^ DQ) ^ " - EXTRA SHOULD BE NUMERIC");
				}
			}

			if (extran == 1) {
				grossbillplus1 = netbill;
			}
		};//extran;
	}

	//now rounding on each calculation so that split amounts
	//agree with the net unit bill
	if (rounding and roundfinally) {
		netbill = (((netbill / rounding).oconv("MD00P")) * rounding).oconv(currfmt);
	}

	return netbill;

}


libraryexit()
