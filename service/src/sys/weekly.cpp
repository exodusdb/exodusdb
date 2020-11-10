#include <exodus/library.h>
libraryinit()

#include <addcent4.h>

var firstdate;
var firstdayofweek;//num
var weeksperperiod;//num
var maxperiod;//num
var temp;
var year;//num
var period;//num
var year2;
var firstdateofyear2;//num
var firstdateofyear;//num
var dayofyear;//num
var diff;//num
var firstdayofyear;//num
var difference;//num

function main(in type, in input, in mode, out output) {
	//c sys in,in,in,out
	//global all

	//eg mode is WEEKLY,1/7,1,4
	//ie year starts on the first monday as near as possible to 1st July
	//and there are 4 weeks per period (which results in 13 periods per year)
	//weeks per period MUST divide into 52 exactly

	if (input == "") {
		output = "";
		return 0;
	}
	firstdate = mode.field(",", 1);
	firstdayofweek = mode.field(",", 2);
	weeksperperiod = mode.field(",", 3);
	if (not weeksperperiod) {
		weeksperperiod = 1;
	}
	maxperiod = 52 / weeksperperiod;

	//if oconv then convert internal date to year:period
	///////////////////////////////////////////////////
	if (type == "OCONV") {
		temp = input.oconv("D2-E");
		year = temp.substr(-2,2);
		period = temp.substr(4,2);

		//guess the right financial year from the date
		year += 1;
		year = year.oconv("R(0)#2");

		//get the first date of the financial year
tryyear:
		year2 = year;
		gosub getfirstdateofyear();
		firstdateofyear2 = firstdateofyear;
		//go to previous year if date before start of year
		if (input < firstdateofyear) {
			year = (addcent4(year) - 1).substr(-2,2);
			goto tryyear;
		}else{
			year2 = (year + 1).oconv("R(0)#2");
			gosub getfirstdateofyear();
			//go to next year if date after last date of year
			if (input > firstdateofyear - 1) {
				year += 1;
				year = year.oconv("R(0)#2");
				goto tryyear;
			}
		}

		dayofyear = input - firstdateofyear2 + 1;
		period = ((dayofyear - 1) / (7 * weeksperperiod)).floor() + 1;
		if (period > maxperiod) {
			period = maxperiod;
		}
		output = year ^ ("00" ^ period).substr(-2,2);

		return 0;
	}

	//if iconv then convert period (MM/YY or YYMM) to internal last date of month
	////////////////////////////////////////////////////////////////////////////
	//return the last day of the period (internal format)
	if (input.index("/")) {
		period = input.field("/",1);
		year = input.field("/", 2);
	}else{
		year = input.substr(1,2);
		period = input.substr(-2,2);
	}

	//get the first date of the next period then subtract 1 day
	period += 1;
	if (period > maxperiod) {
		period -= maxperiod;
		year += 1;
		year = year.oconv("R(0)#2");
	}

	//financial year 93/94 is referred to as 94
	//year2=year-1;gosub getfirstdateofyear
	year2 = year;
	gosub getfirstdateofyear();

	output = (firstdateofyear + (period - 1) * (7 * weeksperperiod) - 1).floor();

	diff = (output - 1) % 7 + 1 - firstdayofweek;

	if (diff) {
		if (diff > 3) {
			diff -= 7;
		}
		if (diff < -3) {
			diff += 7;
		}
		output -= diff;
	}

	//why?
	output -= 1;

	return 0;
}

subroutine getfirstdateofyear() {
	firstdateofyear = (firstdate ^ "/" ^ year2).iconv("D2/E");

	//move start of year to nearest start of week
	//eg start of year 1/7/93 is thursday (day 1)
	//   start of week is friday (day 5)
	//therefore start of year is 2/7/93
	firstdayofyear = (firstdateofyear - 1) % 7 + 1;
	difference = firstdayofyear - firstdayofweek;
	if (difference > 3) {
		difference -= 7;
	}
	if (difference < -3) {
		difference += 7;
	}
	firstdateofyear -= difference;
	return;
}

libraryexit()