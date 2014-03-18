#include <exodus/library.h>
libraryinit()


#include <gen.h>

var direction;//num
var xx;
var daten;

function main(in mode, in idate0, in usercode, in user, in marketcode, in market, in agp, out holidaytype, out workdate) {

	//returns holidaytype given date,user,market and global params

	//holiday type is one of the following (priority in the same order)
	//1=weekend, 2=public, 3=personal, 4=expired login, 0=not holiday
	
	var idate=idate0;
	if (false) print(marketcode);
	if (false) print(usercode);

	if (mode == "GETTYPE") {
		gosub getholidaytype(idate, user, market, agp, holidaytype);

	} else if (mode == "GETWORKDATE") {

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
			gosub getholidaytype(idate, user, market, agp, holidaytype);
			if (not holidaytype) {
				workdate -= direction;
			}
		}//loop;

	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " invalid mode in HOLIDAY()");
	}

	return 0;

}

subroutine getholidaytype(in idate, in user, in market, in agp, out holidaytype) {

	//type 4 is expired
	//////////////////
	if (user.a(35) and idate >= user.a(35)) {
		holidaytype = 4;
		return;
	}

	//type 1 is weekends (of the user otherwise of market)
	/////////////////////////////////////////////////////

	var dow = ((idate - 1) % 7) + 1;

	//from user
	var weekenddows = trim(user.a(24), SVM);
	if (not weekenddows) {

		//else from market
		weekenddows = trim(market.a(9), SVM);
		if (not weekenddows) {

			weekenddows = agp.a(95);

			//else from global parameter "last day of week" plus the day before
			if (not weekenddows) {
				weekenddows = agp.a(13) ^ SVM ^ (((agp.a(13) - 1 - 1) % 7) + 1);
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
	if (holidays.locate(idate, xx, 1)) {
		holidaytype = 2;
		return;
	}

	//type 3 is personal holidays
	////////////////////////////

	//personal holidays only from user currently
	var fromdates = user.a(22);
	var uptodates = user.a(23);
	//from dates must be in reverse sorted order
	if (not(fromdates.locateby(idate, "DR", daten, 1))) {
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
