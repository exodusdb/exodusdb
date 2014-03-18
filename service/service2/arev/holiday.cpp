#include <exodus/library.h>
libraryinit()


#include <gen.h>

var direction;//num
var xx;
var daten;

function main() {

	//returns holidaytype given date,user,market and global params

	//holiday type is one of the following (priority in the same order)
	//1=weekend, 2=public, 3=personal, 4=expired login, 0=not holiday

	if (mode == "GETTYPE") {
		gosub getholidaytype();

		goto 167;
	}
	if (mode == "GETWORKDATE") {

		//skipping forwards or backwards the next number of workdays
		//eg -2 will change idate to the second workday BEFORE idate
		if (workdate > 0) {
			direction = 1;
		}else{
			direction = -1;
		}

		var mindate = var("1/1/2000").iconv("D/E");

		while (true) {
		///BREAK;
		if (not(workdate and idate ne mindate)) break;;
			idate += direction;
			gosub getholidaytype();
			if (not holidaytype) {
				workdate -= direction;
			}
		}//loop;

		goto 167;
	}
	if (1) {
		call mssg(DQ ^ (mode ^ DQ) ^ " invalid mode in HOLIDAY()");
	}
L167:
	return 0;

}

subroutine getholidaytype() {

	//type 4 is expired
	//////////////////
	if (USER.a(35) and idate >= USER.a(35)) {
		holidaytype = 4;
		return;
	}

	//type 1 is weekends (of the user otherwise of market)
	/////////////////////////////////////////////////////

	var dow = idate - 1 % 7 + 1;

	//from user
	var weekenddows = trim2(USER.a(24), SVM);
	if (not weekenddows) {

		//else from market
		weekenddows = trim2(market.a(9), SVM);
		if (not weekenddows) {

			weekenddows = agp.a(95);

			//else from global parameter "last day of week" plus the day before
			if (not weekenddows) {
				weekenddows = agp.a(13) ^ SVM ^ agp.a(13) - 1 - 1 % 7 + 1;
			}
		}
	}
	if (weekenddows.index(dow, 1)) {
		holidaytype = 1;
		return;
	}

	//type 2 is general holidays
	///////////////////////////

	//general holidays only from market currently
	var holidays = market.a(7);
	//if holidays else
	// holidays=
	// end
	if (holidays.a(1).locateusing(idate, VM, xx)) {
		holidaytype = 2;
		return;
	}

	//type 3 is personal holidays
	////////////////////////////

	//personal holidays only from user currently
	var fromdates = USER.a(22);
	var uptodates = USER.a(23);
	//from dates must be in reverse sorted order
	if (not(fromdates.a(1).locatebyusing(idate, "DR", daten, VM))) {
		{}
	}
	var uptodate = uptodates.a(1, daten);
	if (uptodate and idate <= uptodates.a(1, daten)) {
		holidaytype = 3;
		return;
	}

	//type 0 is no holiday
	/////////////////////

	holidaytype = 0;

	return;

}


libraryexit()