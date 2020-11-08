#include <exodus/library.h>
libraryinit()

#include <gen_common.h>
#include <fin_common.h>

var fromcurrcode;
var tocurrcode;
var ratecurrcode;
var ratefn;//num
var fmtx;
var ratecurrency;
var vn;//num

function main(in ratedate, in fromcurrcode0, in tocurrcode0, in fromamount, out toamount, out rate, out msg) {
	//c gen in,in,in,in,out,out,out

	//wget http://www.ecb.int/stats/eurofxref/eurofxref-daily.xml
	//base currency code should really be in general common

	//returns a divisor to convert fromcurrcode to tocurrcode
	//in otherwords the ratio fromcurrcode/tocurrcode
	//(or 1 and an error msg)
	//if an amount is provided then it is converted at the returned rate
	//even if there is an error

	#include <general_common.h>
	#include <common.h>

	toamount = fromamount;
	rate = 1;
	msg = "";

	if (fromcurrcode0) {
		fromcurrcode = fromcurrcode0;
	}else{
		fromcurrcode = fin.basecurrency;
		}

	if (tocurrcode0) {
		tocurrcode = tocurrcode0;
	}else{
		tocurrcode = fin.basecurrency;
	}

	//no conversion if same currency code
	if (fromcurrcode == tocurrcode) {
		return 0;
	}

	//convert to base
	if (tocurrcode == fin.basecurrency) {
		ratecurrcode = fromcurrcode;
		ratefn = 5;
		gosub getratetobase(ratedate, rate, msg);
		fmtx = fin.basefmtx;

	//convert from base
	} else if (fromcurrcode == fin.basecurrency) {
		ratecurrcode = tocurrcode;
		ratefn = 12;
		gosub getratetobase(ratedate, rate, msg);
		if (rate ne 1) {
			fmtx = "MD" ^ ratecurrency.a(3) ^ "0P";
		}

	//here on is convert between two non-base currencies
	}else{

		//get rate from curr1 to base
		ratecurrcode = fromcurrcode;
		ratefn = 5;
		gosub getratetobase(ratedate, rate, msg);
		if (not(msg.length())) {

			var rate1 = rate;

			//get rate from curr2 to base
			ratecurrcode = tocurrcode;
			ratefn = 12;
			gosub getratetobase(ratedate, rate, msg);

			if (not(msg.length())) {
				fmtx = "MD" ^ ratecurrency.a(3) ^ "0P";

				//final rate is rate-from-curr1-to-base * rate-from-curr2-to-base
				rate = (rate1 * rate).oconv("MD60P");
			}

		}

	}

	//convert amount if provided
	if (fromamount) {
		if (rate ne 1) {
			//garbagecollect;
			toamount = oconv(fromamount / rate, fmtx);
		}
	}

	return 0;
}

subroutine getratetobase(in ratedate, out rate, out msg) {

	//get exch rate
	if (ratecurrency.reado(gen.currencies, ratecurrcode)) {
		if (not(ratecurrency.a(4).locate(ratedate,vn))) {
			if (not(ratecurrency.a(4).locateby("DR",ratedate + 1,vn))) {
				{}
			}
		}
		rate = ratecurrency.a(ratefn, vn);
		if (not rate) {

			//backward compatible with no field 12
			if (ratefn == 12) {
				rate = ratecurrency.a(5, vn);
				if (rate) {
					//garbagecollect;
					rate = (1 / rate).oconv("MD60P");
				}
			}

			if (not rate) {

				rate = ratecurrency.a(ratefn, vn - 1);

				//backward compatible with no field 12
				if (ratefn == 12) {
					rate = ratecurrency.a(5, vn - 1);
					if (rate) {
						//garbagecollect;
						rate = (1 / rate).oconv("MD60P");
					}
				}

				if (not rate) {
					msg = "Currency " ^ (ratecurrcode.quote()) ^ " has no exchange rates";
					rate = 1;
				}

			}
		}
	}else{
		msg = ratecurrcode.quote() ^ " is not in the currency file";
		rate = 1;
	}

	return;
}

libraryexit()
