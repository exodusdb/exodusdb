#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <service_common.h>


var direction;	// num
//var xx;
var daten;

func main(in mode, io idate, in /*usercode*/, in userx, in /*marketcode*/, in market, in agp, out holidaytype, out workdate) {

	// returns holidaytype given date,userx,market and global params

	// holiday type is one of the following (priority in the same order)
	// 1=weekend, 2=public, 3=personal, 4=expired login, 0=not holiday

	if (mode == "GETTYPE") {
		gosub getholidaytype(idate, userx, agp, market, holidaytype);

	} else if (mode == "GETWORKDATE") {

		// skipping forwards or backwards the next number of workdays
		// eg -2 will change idate to the second workday BEFORE idate
		if (workdate > 0) {
			direction = 1;
		} else {
			direction = -1;
		}

		let mindate = var("1/1/2000").iconv("D/E");

		while (true) {
			// /BREAK;
			if (not(workdate and idate != mindate))
				break;
			idate += direction;
			gosub getholidaytype(idate, userx, agp, market, holidaytype);
			if (not holidaytype) {
				workdate -= direction;
			}
		}  // loop;

	} else {
		call note(mode.quote() ^ " invalid mode in HOLIDAY()");
	}

	return 0;
}

subr getholidaytype(in idate, in userx, in agp, in market, io holidaytype) {

	// type 4 is expired
	// ////////////////
	if (userx.f(35) and idate >= userx.f(35)) {
		holidaytype = 4;
		return;
	}

	// type 1 is weekends (of the user otherwise of market)
	// ///////////////////////////////////////////////////

	let dow = idate.oconv("DW");

	// from user
	var weekenddows = trim(userx.f(24), SM);
	if (not weekenddows) {

		// else from market
		weekenddows = trim(market.f(9), SM);
		if (not weekenddows) {

			weekenddows = agp.f(95);

			// else from global parameter "last day of week" plus the day before
			if (not weekenddows) {
				// weekenddows=agp<13>:sm:mod(agp<13>-1-1,7)+1
				// var tt		= (agp.f(13) - 1 - 1).mod(7) + 1;
				let tt		= (agp.f(13) - 1).oconv("DW");
				weekenddows = agp.f(13) ^ SM ^ tt;
			}
		}
	}
	if (weekenddows.contains(dow)) {
		holidaytype = 1;
		return;
	}

	// type 2 is general holidays
	// /////////////////////////

	// general holidays only from market currently
	let holidays = market.f(7);
	// if holidays else
	// holidays=
	// end
	var tt;
	if (holidays.locate(idate, tt, 1)) {
		holidaytype = 2;
		return;
	}

	// type 3 is personal holidays
	// //////////////////////////

	// personal holidays only from user currently
	let fromdates = userx.f(22);
	let uptodates = userx.f(23);
	// from dates must be in reverse sorted order
	if (not fromdates.f(1).locateby("DR", idate, daten)) {
		// Not found. daten points to the last + 1
	}
	let uptodate = uptodates.f(1, daten);
	if (uptodate and idate <= uptodates.f(1, daten)) {
		holidaytype = 3;
		return;
	}

	// type 0 is no holiday
	// ///////////////////

	holidaytype = 0;

	return;
}

}; // libraryexit()
