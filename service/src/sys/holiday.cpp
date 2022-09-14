#include <exodus/library.h>
libraryinit()

#include <sys_common.h>

var direction;//num
var xx;
var daten;

function main(in mode, io idate, in usercode, in userx, in marketcode, in market, in agp, out holidaytype, out workdate) {
	//c sys ,io,,,,,,out,out

	//returns holidaytype given date,userx,market and global params

	//holiday type is one of the following (priority in the same order)
	//1=weekend, 2=public, 3=personal, 4=expired login, 0=not holiday

	#include <system_common.h>

	//prevent c++ warnings about unused variables
	if (usercode.unassigned()) {
		{}
	}
	if (marketcode.unassigned()) {
		{}
	}

	if (mode eq "GETTYPE") {
		gosub getholidaytype(idate, userx, agp, market, holidaytype);

	} else if (mode eq "GETWORKDATE") {

		//skipping forwards or backwards the next number of workdays
		//eg -2 will change idate to the second workday BEFORE idate
		if (workdate gt 0) {
			direction = 1;
		} else {
			direction = -1;
		}

		var mindate = var("1/1/2000").iconv("D/E");

		while (true) {
			///BREAK;
			if (not(workdate and idate ne mindate)) break;
			idate += direction;
			gosub getholidaytype(idate, userx, agp, market, holidaytype);
			if (not holidaytype) {
				workdate -= direction;
			}
		}//loop;

	} else {
		call mssg(mode.quote() ^ " invalid mode in HOLIDAY()");
	}

	return 0;
}

subroutine getholidaytype(in idate, in userx, in agp, in market, io holidaytype) {

	//type 4 is expired
	//////////////////
	if (userx.f(35) and idate ge userx.f(35)) {
		holidaytype = 4;
		return;
	}

	//type 1 is weekends (of the user otherwise of market)
	/////////////////////////////////////////////////////

	var dow = (idate - 1).mod(7) + 1;

	//from user
	var weekenddows = trim(userx.f(24), SM);
	if (not weekenddows) {

		//else from market
		weekenddows = trim(market.f(9), SM);
		if (not weekenddows) {

			weekenddows = agp.f(95);

			//else from global parameter "last day of week" plus the day before
			if (not weekenddows) {
				//weekenddows=agp<13>:sm:mod(agp<13>-1-1,7)+1
				var tt = (agp.f(13) - 1 - 1).mod(7) + 1;
				weekenddows = agp.f(13) ^ SM ^ tt;
			}
		}
	}
	if (weekenddows.contains(dow)) {
		holidaytype = 1;
		return;
	}

	//type 2 is general holidays
	///////////////////////////

	//general holidays only from market currently
	var holidays = market.f(7);
	//if holidays else
	// holidays=
	// end
	if (holidays.f(1).locate(idate, xx)) {
		holidaytype = 2;
		return;
	}

	//type 3 is personal holidays
	////////////////////////////

	//personal holidays only from user currently
	var fromdates = userx.f(22);
	var uptodates = userx.f(23);
	//from dates must be in reverse sorted order
	if (not(fromdates.f(1).locateby("DR", idate, daten))) {
		{}
	}
	var uptodate = uptodates.f(1, daten);
	if (uptodate and idate le uptodates.f(1, daten)) {
		holidaytype = 3;
		return;
	}

	//type 0 is no holiday
	/////////////////////

	holidaytype = 0;

	return;
}

libraryexit()
