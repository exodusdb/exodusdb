#ifndef MVDATETIME_H
#define MVDATETIME_H

/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <cstring>

#include <exodus/gregorian.h>

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
//   #include <time.h>
#include <exodus/gettimeofday.h>
#else
#include <sys/time.h>
#endif

#include <string>
#include <iostream> //for cerr
#include <chrono>
#include <sstream>

// ostime, osdate, osfile, osdir should return dates and times in UTC

#define PICK_UNIX_DAY_OFFSET -732

const char* shortmths = "JAN\0FEB\0MAR\0APR\0MAY\0JUN\0JUL\0AUG\0SEP\0OCT\0NOV\0DEC\0";
const char* longmths =
	"\0JANUARY\0  "
	"\0FEBRUARY\0 "
	"\0MARCH\0    "
	"\0APRIL\0    "
	"\0MAY\0      "
	"\0JUNE\0     "
	"\0JULY\0     "
	"\0AUGUST\0   "
	"\0SEPTEMBER\0"
	"\0OCTOBER\0  "
	"\0NOVEMBER\0 "
	"\0DECEMBER\0 ";

const char* longdayofweeks =
	"MONDAY\0   "
	"TUESDAY\0  "
	"WEDNESDAY\0"
	"THURSDAY\0 "
	"FRIDAY\0   "
	"SATURDAY\0 "
	"SUNDAY\0   ";

#include <exodus/var.h>

namespace exodus {

/***********************
time_t_to_pick_date_time
************************/

// Utility to convert a c time_t structure to pick integer date and time

void time_t_to_pick_date_time(const time_t time, int* pick_date, int* pick_time) noexcept {

	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

    const auto duration_since_epoch = std::chrono::system_clock::from_time_t(time).time_since_epoch();
    //TRACE(duration_since_epoch.count())

//    using days = std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;
//    const auto duration_in_days = std::chrono::duration_cast<days>(duration_since_epoch);
//    idate = duration_in_days.count();

    using secs = std::chrono::duration<int, std::ratio_multiply<std::chrono::seconds::period, std::ratio<1>>>;
    const auto duration_in_secs = std::chrono::duration_cast<secs>(duration_since_epoch);
    *pick_time = duration_in_secs.count() % 86400;

	*pick_date = (duration_in_secs.count() / 86400) - PICK_UNIX_DAY_OFFSET;

}

var var::date() const {
	// returns number of days since the pick date epoch 31/12/1967

	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

    const auto duration_since_epoch = std::chrono::system_clock().now().time_since_epoch();

	using days = std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;
	const auto duration_in_days = std::chrono::duration_cast<days>(duration_since_epoch);

	// timestamp() assumes that the var returned is an int
	return duration_in_days.count() - PICK_UNIX_DAY_OFFSET;

}

var var::time() const {
	// returns number of whole seconds since midnight


	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

    const auto duration_since_epoch = std::chrono::system_clock().now().time_since_epoch();

    using secs = std::chrono::duration<int, std::ratio_multiply<std::chrono::seconds::period, std::ratio<1>>>;
    const auto duration_in_secs = std::chrono::duration_cast<secs>(duration_since_epoch);
    return duration_in_secs.count() % 86400;

}

var var::ostime() const {
	// return decimal seconds since midnight up to micro or nano second accuracy

	// ms accuracy
	//uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//return now % 86400000000 / 1000000.0;

	// ns accuracy
	uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return now % 86400000000000 / 1000000000.0;

}

// return decimal fractional days since pick epoch 1967-12-31 00:00:00
var var::timestamp() const {

	var datenow = this->date();
	var timenow = this->ostime();

	// If the date flipped while we got the time then get the time again
	// If someone is messing with the system clock then we will
	// fall for the bait only once
	// Assuming that date() is returned as an int for speed
	if (this->date().var_int != datenow.var_int)
		timenow = this->ostime();

	return datenow + timenow / 86400;

}

//var var::timedate() const {
//	// output the current "HH:MM:SS  DD MMM YYYY" without quotes but note the double space
//
//	// TODO make this rely on a single timestamp instead of time and date
//	// to avoid the slight chance of time and date being called different sides of midnight
//	return time().oconv_MT("S") ^ " " ^ date().oconv_D("D");
//}

var var::iconv_D(const char* conversion) const {

	// should perhaps ONLY implement only ISO8601 which is in xml
	// yyyy-mm-ddTHH:MM:SS.SSS

	// General pattern is:
	// JUNK1
	// (digitgroup | alphagroup)
	// JUNK2
	// (digitgroup | alphagroup)
	// JUNK2
	// (digitgroup | alphagroup)
	//
	// Where JUNK1 is [^A-Za-z0-9]*
	// Where JUNK2 is [^A-Za-z0-9]+
	// alphagroup is three letter month JAN-DEC case insensitive and can only appear once
	// digitgroup is \d{1,2} and can only appear twice if alphagroup exists

	// iconv of two digit years
	// years 0-49 are assumed to be 2000-2049
	// years 50-99 are assumed to be 1950-1999

	static auto year_2000 = 2000;
	static auto year_50 = 50;

	// defaults
	bool yearfirst = false;
	bool dayfirst = false;

	const char* conversionchar = conversion;
//	if (*conversionchar != 'D')
//		return *this;
	++conversionchar;

	while (*conversionchar) {

		switch (*conversionchar) {
			case 'E':
				dayfirst = true;
				break;
			case 'S':
				yearfirst = true;
				break;
		}
		++conversionchar;
	}

	const int maxnparts = 3;
	int parts[maxnparts];
	parts[0] = 0;
	int partn = -1;

	int month = 0;

	const char* iter = var_str.c_str();
	while (*iter != '\0') {

		if (isdigit(*iter)) {

			partn++;

			// Get the first digit as an int and skip to the next input byte
			parts[partn] = (*iter++) - '0';

			// Accumulate any additional digits as ints
			// after multipying prior accumulation by 10
			while (isdigit(*iter))
				parts[partn] = (parts[partn] * 10) + ((*iter++) - '0');

			continue;
		}

		else if (::isalpha(*iter)) {

			// Month letters can only appear once
			if (month != 0)
				return "";

			std::string word;
			do {
				word.push_back(toupper(*iter++));
			} while (::isalpha(*iter));

			// determine the month or return "" to indicate failure
			for (int ii = 0; ii < 12 * 4; ii += 4) {
				if (strcmp(static_cast<const char*>(shortmths + ii), word.c_str()) == 0) {
					month = ii / 4 + 1;
					break;
				}
			}
			if (month == 0)
				return "";

			continue;
		}

		// Fail if more than 2 parts if month exists or 3 parts if it doesnt
		if ((month && partn == 1) || (partn == 2))
			return "";

		// skip all not alphanumeric characters
		++iter;
	}

	// for now dont default current year since this is a user input type issue
	// and exodus is planned to be used on the server side.

	//switch to yyyy-mm-dd format if first part looks like a four digit year
	if (parts[0] >= 1000) {
		yearfirst = true;
		dayfirst = false;
	}

	int day;
	int year;
	if (month != 0) {
		// fail if day or year missing
		if (partn < 1)
			return "";

		if (yearfirst) {
			year = parts[0];
			day = parts[1];
		} else {
			year = parts[1];
			day = parts[0];
		}
	} else {
		// fail if missing three parts for year, month and day
		if (partn < 2)
			return "";

		if (yearfirst) {
			year = parts[0];
			month = parts[1];
			day = parts[2];
		} else {
			if (dayfirst) {
				day = parts[0];
				month = parts[1];
			} else {
				month = parts[0];
				day = parts[1];
			}
			year = parts[2];
		}
	}

	// two digit year
	// TODO provide option to control cutover or base relative to current system year
	// or refuse anything but four digit years
	if (year <= 99) {
		if (year >= year_50) {
			year += year_2000 - 100;
			//std::cerr << "WARNING: 2 digit year >= " << year_50 << " being converted to " << year << std::endl;
			errputl("WARNING: 2 digit year >= " ^ var(year_50) ^ " being converted to " ^ year);
		} else
			year += year_2000;
	}

	// Check month
	if (month < 1 or month > 12)
		return "";

	// Table of max gregorian day per month
    constexpr const int maxdays_by_month[] = {
							31,28,31,//Jan, Feb, Mar
							30,31,30,//Apr, May, Jun
							31,31,30,//Jul, Aug, Sep
							31,30,31 //Oct, Nov, Dec
						};

	// Check max day per month
	if (day > maxdays_by_month[month - 1] || day < 1) {
		// Fail if exceed max unless Feb 29 on leap years
		if (not (day == 29 and month == 2 and hinnant::gregorian::is_leap(year)))
			return "";
	}

	// Convert to pick integer date which is the number of days since 1967-12-31
	return hinnant::gregorian::days_from_civil(year, month, day) - PICK_UNIX_DAY_OFFSET;

}

var var::oconv_D(const char* conversion) const {

	// by this time it is known to be a number and not an empty string

	// pick date uses floor()ie 1.9=1 and -1.9=-2

	// get a ymd object for the desired date
	int pickdayno = this->floor();
	int unixdayno = pickdayno + PICK_UNIX_DAY_OFFSET;

	int civil_year;
	unsigned civil_month;
	unsigned civil_day;
	std::tie(civil_year, civil_month, civil_day) = hinnant::gregorian::civil_from_days(unixdayno);

	// defaults
	bool alphamonth = true;
	int yeardigits = 4;
	bool yearfirst = false;
	bool dayfirst = false;
	// bool leadingzeros=true;
	char sepchar = ' ';

	// this function is only called when the 1st character is "D"
	const char* conversionchar = conversion;
//	if (*conversionchar != 'D')
//		return *this;
	++conversionchar;

	// DY means year only unless followed by a separator character
	bool yearonly = (*conversionchar) == 'Y';

	while (*conversionchar) {

		// digits anywhere indicate size of year
		if ((*conversionchar) <= '9' && (*conversionchar) >= '0') {
			yeardigits = (*conversionchar) - '0';
			++conversionchar;
			continue;
		}

		int dow;
		switch (*conversionchar) {

			//(E)uropean date - day first
			case 'E':
				dayfirst = true;
				break;

			// Not AREV
			// DM returns month number
			// DMA returns full month name
			case 'M':
				++conversionchar;
				if (*conversionchar == 'A')
					return &longmths[civil_month * 11 - 10];
				return var(static_cast<int>(civil_month));

			// Not AREV
			// DW returns day of week number number 1-7 Mon-Sun
			// DWA returns the day of week name
			case 'W':
				// calculate directly from pick date (construction of date above is wasted
				dow = (((*this).floor() - 1) % 7) + 1;
				++conversionchar;
				if (*conversionchar == 'A')
					return &longdayofweeks[dow * 10 - 10];
				return dow;

			// Not AREV
			// DY year (four digits or D2Y works too)
			// DYn formatted
			case 'Y':
				yearfirst = true;
				// yearonly=true;
				// allow trailing digit as well to control ndigits
				++conversionchar;
				if ((*conversionchar) <= '9' && (*conversionchar) >= '0')
					yeardigits = (*conversionchar) - '0';
				--conversionchar;
				break;

			// Not AREV
			// DD day of month
			case 'D':
				return var(static_cast<int>(civil_day));

			// Not AREV
			// DQ returns quarter number
			case 'Q':
				return var(static_cast<int>((civil_month - 1) / 3) + 1);

			// Not AREV
			// DJ returns day of year
			case 'J':
				//return static_cast<int>(desired_date.day_of_year());
				return unixdayno - hinnant::gregorian::days_from_civil(civil_year, 1, 1) + 1;

			// iso year format - at the beginning
			case 'S':
				// alphamonth=false;
				yearfirst = true;
				break;

			// DL LAST day of month
			case 'L':
				//return var(static_cast<int>(desired_date.end_of_month().day()));
				return var(static_cast<int>(hinnant::gregorian::last_day_of_month(civil_year, civil_month)));

			default:
				sepchar = *conversionchar;
				yearonly = false;
				alphamonth = false;
				break;
		}
		++conversionchar;
	}

	std::stringstream ss;
	ss.fill('0');

	// trim the right n year digits since width() will pad but not cut (is this really the C++
	// way?)
	std::stringstream yearstream;
	yearstream << civil_year;
	std::string yearstring = yearstream.str();
	int yearstringerase = static_cast<int>(yearstring.length() - yeardigits);
	if (yearstringerase > 0)
		yearstring.erase(0, yearstringerase);

	// year first
	if (yearfirst) {
		ss.width(yeardigits);
		// ss << civil_year;
		// above doesnt cut down number of characters
		ss << yearstring;
		if (yearonly)
			return ss.str();
		ss << sepchar;
	}

	//determine if day first if not forced
	if (not dayfirst && alphamonth && not yearfirst)
		dayfirst = true;

	// day first
	if (dayfirst) {
		ss.width(2);
		ss << civil_day;
		ss << sepchar;
	}

	// month
	if (alphamonth) {
		// ss.width(3);
		ss << &shortmths[civil_month * 4 - 4];
	} else {
		ss.width(2);
		ss << static_cast<int>(civil_month);
	}

	// day last
	if (!dayfirst) {
		ss << sepchar;
		ss.width(2);
		ss << civil_day;
	}

	// year last
	if (!yearfirst && yeardigits) {
		ss << sepchar;
		// ss << civil_year;
		// above doesnt cut down number of characters
		ss.width(yeardigits);
		ss << yearstring;
	}
	// outputl(var(ss.str()).quote());
	return ss.str();
}

var var::oconv_MT(const char* conversion) const {
	// conversion points to the character AFTER MT - which may be \0
	// MT, MTH, MTS, MTx, MTHx, MTSx MTHS MTHSx where x is a sep char

	// MT2... also a 2 digit may be inserted in all cases just after MT
	// to indicate that the input is in hours (either decimal or int)

	// MT2U... also a U letter may be inserted in all cases instead of
	// or after 2 to indicate that the output is not limited to 24 hours
	// and can be 25:59 or even 1200:59

	// defaults
	bool twelvehour = false;
	bool unlimited = false;	 // incompatible with twelvehour
	bool showsecs = false;
	bool input_is_hours = false;
	char sepchar = ':';
	const char* conversionchar = conversion;
	int timesecs;

	// Analyse conversion characters
	while (*conversionchar) {

		switch (*conversionchar) {

			case '2':
				input_is_hours = true;
				break;

			// U to indicate unlimited hours
			case 'U':
				unlimited = true;
				break;

			// Z to convert 0 to ""
			case 'Z':
				if (!(*this))
					return "";
				break;

			// H to indicate AM/PM
			case 'H':
				twelvehour = true;
				break;

			// S to show seconds
			case 'S':
				showsecs = true;
				break;

			// Any other character is the separator. First one only.
			default:
				sepchar = *conversionchar;
				goto after_analyse_conversion;
				break;
		}
		conversionchar++;
	}
after_analyse_conversion:

	if (input_is_hours) {
		// if input is integer or decimal hours then convert by *3600 to integer seconds
		timesecs = ((*this) * 3600).round();
	} else {
		// otherwise round any decimal seconds to integer
		timesecs = round();
	}

	// standardise times <0 and >=86400 to one day ie 0-85399 seconds
	bool negative;
	if (unlimited) {
		negative = timesecs < 0;
		if (negative)
			timesecs = -timesecs;
	} else {
		timesecs = timesecs % 86400;
		if (timesecs < 0)
			timesecs += 86400;
	}

	int hours = timesecs / 3600;
	int mins = timesecs % 3600 / 60;
	int secs = timesecs % 60;

	bool am = false;
	if (twelvehour) {
		if (hours >= 13)
			hours -= 12;
		else if (hours < 12) {
			am = true;
			// zero hour is 12am in 12 hour clock
			if (!hours)
				hours = 12;
		}
	}

	var newmv;
	newmv.var_typ = VARTYP_STR;

	//hours first
	if (unlimited) {
		//unlimited hours
		if (negative)
			newmv.var_str = "-";
		//else
		//	newmv = "";
		if (hours < 10)
			newmv.var_str.push_back('0');
		newmv ^= hours;
	} else {
		// two digit hours
		//newmv = ("00" ^ var(hours)).substr(-2);
		newmv.var_str.push_back('0' + hours / 10);
		newmv.var_str.push_back('0' + hours % 10);
	}

	// separator
	newmv.var_str.push_back(sepchar);

	// two digit minutes
	//newmv ^= ("00" ^ var(mins)).substr(-2);
	newmv.var_str.push_back('0' + mins / 10);
	newmv.var_str.push_back('0' + mins % 10);

	if (showsecs) {

		// separator
		newmv.var_str.push_back(sepchar);

		// two digit seconds
		//newmv ^= ("00" ^ var(secs)).substr(-2);
		newmv.var_str.push_back('0' + secs / 10);
		newmv.var_str.push_back('0' + secs % 10);
	}

	// optional AM/PM suffix
	if (twelvehour) {
		if (am) {
			newmv.var_str.push_back('A');
			newmv.var_str.push_back('M');
		} else {
			newmv.var_str.push_back('P');
			newmv.var_str.push_back('M');
		}
	}

	return newmv;
}

}  // namespace exodus

#endif
