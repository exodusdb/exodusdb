#include <exodus/library.h>
libraryinit()

#include <agy.h>

var rounding;//num
var gross;//num
var perc;//num
var amount;//num
var grossbillplus1;

function main(io grossbill, in spec, in extras2, in unused, io msg, in roundingx, io sizemult, io amounts) {
	//c med io,in,in,in,io,in,io,io
	//MUST be kept in EXACT/CAREFUL sync with function schedule_calcbill in client
	amounts = "";
	//y2k *jbase
	var ndecs = grossbill.field(".", 2).length();
	var origfmt = "MD" ^ ndecs ^ "0P";

	//new method is to round on each calculation
	//only really effects bahrain and amounts with three decimal places
	//roundfinally=0;* or 1 to agree with how the intranet works
	var roundfinally = 1;

	if (rounding.unassigned()) {
		roundingx = agy.agp.a(32, 1);
		rounding = roundingx;
		//if rounding='' and ndecs=3 then rounding=.1
		if ((rounding == "") and (ndecs == 3)) {
			rounding = PRIORITYINT.a(101, 1);
		}
	}else{
		rounding = roundingx;
	}

	msg = "";

	var extras = extras2;
	extras.swapper(",", "," ^ 0x1B);
	extras.converter("/Xx", ",");

		/*;
		//sizemult: x size
		//return sizemult and grossbill.sized
		sizemult=1;
		//getsize
		size=field(field(field(spec,'~',1),'*',2),',',1);
		if size then;
			convert 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' to '                          ' in size;
			size=trim(size);
			sizea=field(size,' ',1);
			if sizea and num(sizea) then;
				sizeb=field(size,' ',2);
				if num(sizeb) then;
					if sizeb else sizeb=1;
					sizemult=sizea*sizeb;
					garbagecollect;
					grossbill=oconv(grossbill*sizemult,origfmt);
					//NB return grossbill as sized
					end;
				end;
			end;
		*/

	gosub getsize();
	if (sizemult ne 1) {
		//garbagecollect;
		grossbill = (grossbill * sizemult).oconv(origfmt);
	}

	//initialise unit bill to be same as gross unit bill
	var netbill = grossbill;
	var prevnetbill = netbill;

	//subtract/add any extras
	//allow spaces so comments come after first space
	//CONVERT ' ' TO '' IN EXTRAS

	//!various types of free ads
	//t=extras[1,1]
	//!convert @lower.case to @upper.case in t
	//if t matches '1A' then extras='-100%'

	if (extras) {
		var nextras = extras.count(",") + 1;
		var breakn = 1;
		for (var extran = 1; extran <= nextras; ++extran) {
			//EXTRA=field(FIELD(EXTRAS,',',EXTRAN),' ',1)
			var extra = extras.field(",", extran).trim().field(" ", 1);

			//commas indicate breaks but slashes within commas are converted to commas
			if (extra[1] == 0x1B) {
				extra.splicer(1, 1, "");
				breakn += 1;
			}

			//various types of free ads
			//t=extra[1,1]
			//convert @lower.case to @upper.case in t
			if ((extra[1]).match("1A")) {
				extra = "-100%";
			}

			if (extra[1] == "=") {
				if (extra.substr(2,9999).isnum()) {
					prevnetbill = netbill;
					netbill = extra.substr(2,9999);
					goto gotnewnet;
				}
			}
			if (extra) {

				var origextra = extra;

				if (extra[1] == "+") {
					extra.splicer(1, 1, "");
				}

				var plus1 = 0;
				var but1 = 0;
				if (extra.index("G")) {
					gross = 1;
				}else{
					if (extra.index("g")) {
						gross = 1;
						if (extran > 1) {
							plus1 = 1;
						}
					}else{
						gross = 0;
						if ((extran > 1) and extra.index("n")) {
							but1 = 1;
						}
					}
				}
				//Xx not needed but here for safety?
				extra.converter("NnGgXx", "");

				var minimum = "";
				if (extra.index(">")) {
					minimum = extra.field(">", 2);
					if (not(minimum.isnum())) {
						minimum = "";
						msg.r(-1, DQ ^ (origextra ^ DQ) ^ " - MINIMUM SHOULD BE NUMERIC");
					}
					extra = extra.field(">", 1);
				}

				if (extra[-1] == "%") {
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
						//garbagecollect;
						if (rounding and not roundfinally) {
							amount = (((amount / rounding).oconv("MD00P")) * rounding).oconv(origfmt);
						}else{
							amount = amount.oconv(origfmt);
						}
					}else{
						prevnetbill = netbill;
						amount = extra;
						var ndecs2 = amount.field(".", 2).length();
						if (ndecs2 > ndecs) {
							ndecs = ndecs2;
							origfmt = "MD" ^ ndecs ^ "0P";
						}
					}

					if (minimum) {
						if (amount < minimum) {
							amount = minimum;
						}
					}

					netbill += amount;
					//garbagecollect;
					amounts.r(breakn, (amounts.a(breakn) + amount).oconv(origfmt));

gotnewnet:
					//garbagecollect;
					netbill = netbill.oconv(origfmt);
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
		//garbagecollect;
		netbill = (((netbill / rounding).oconv("MD00P")) * rounding).oconv(origfmt);
	}

	return netbill;

}


libraryexit()
