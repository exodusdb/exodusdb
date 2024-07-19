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

#if EXO_MODULE
	import std;
#else
#	include <cstring>
#	include <array>
#	include <string>
#	include <iostream> //for cerr
#	include <chrono>
#	include <sstream>
#	include <cctype> // for std::isdigit

#	include <ctime> // for std::time_t
#	include <cctype> // for std::isalpha std::toupper
#endif

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include <exodus/gregorian.h>
#pragma GCC diagnostic pop

#if EXO_MODULE
#else
#	if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#		include <time.h>
#		include <exodus/gettimeofday.h>
#	else
#		include <sys/time.h>
#	endif
#endif

// ostime, osdate, osfile, osdir should return dates and times in UTC

#define PICK_UNIX_DATE_OFFSET -732

//static const char* shortmths = "JAN\0FEB\0MAR\0APR\0MAY\0JUN\0JUL\0AUG\0SEP\0OCT\0NOV\0DEC\0";
//static const std::string shortmths {"JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC"};
//static constexpr std::array shortmths {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
static constexpr std::string_view shortmths {"JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC"};
//static const char* longmths =
static constexpr std::string_view longmths =
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

//static const char* longdayofweeks =
static constexpr std::string_view longdayofweeks =
	"MONDAY\0   "
	"TUESDAY\0  "
	"WEDNESDAY\0"
	"THURSDAY\0 "
	"FRIDAY\0   "
	"SATURDAY\0 "
	"SUNDAY\0   ";

#include <exodus/var.h>

//// Iterating over char*
//// https://www.foonathan.net/2020/03/iterator-sentinel/
//
//// Empty type.
//struct zstring_sentinel {};
//
//// Are we done?
//static bool operator==(const char* str, zstring_sentinel)
//{
//	return *str == '\0';
//}
//
//struct zstring_range
//{
//	const char* str;
//
//	auto begin() const
//	{
//		// The begin is just the pointer to our string.
//		return str;
//	}
//	auto end() const
//	{
//		// The end is a different type, the sentinel.
//		return zstring_sentinel{};
//	}
//};
//

namespace exo {

// std::time_t -> pick integer date, time
void time_t_to_pick_date_time(const std::time_t time, int* pick_date, int* pick_time) noexcept {

	// ASSUMPTION: td::chrono::system_clock() epoch is 1970/01/01 00:00:00

	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

	const auto duration_since_epoch = std::chrono::system_clock::from_time_t(time).time_since_epoch();

//	const auto duration_in_days = std::chrono::duration_cast<std::chrono::days>(duration_since_epoch);
//	idate = duration_in_days.count();

	const auto duration_in_secs = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch);

	//Modulo 86'400 to seconds within day
	// warning: conversion from ‘std::chrono::duration<long int>::rep’ {aka ‘long int’} to ‘int’ may change value [-Wconversion]
	*pick_time = static_cast<int>(duration_in_secs.count()) % 86'400;

	// Integer division by 86'400 to get whole days
	//warning: conversion from ‘std::chrono::duration<long int>::rep’ {aka ‘long int’} to ‘int’ may change value [-Wconversion]
	*pick_date = static_cast<int>(duration_in_secs.count()) / 86'400 - PICK_UNIX_DATE_OFFSET;

}

// -> number of days since the pick date epoch 31/12/1967
var var::date() const {

	// ASSUMPTION: td::chrono::system_clock() epoch is 1970/01/01 00:00:00

	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

	const auto duration_since_epoch = std::chrono::system_clock().now().time_since_epoch();

	using days = std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;
	const auto duration_in_days = std::chrono::duration_cast<days>(duration_since_epoch).count();
	// Use std::chrono:days in new version of g++ c++20
	//const auto duration_in_days = std::chrono::duration_cast<std::chrono::days>(duration_since_epoch).count();

	// timestamp() assumes that the var returned is an int
	return duration_in_days - PICK_UNIX_DATE_OFFSET;

}

// -> number of whole seconds since midnight
var var::time() const {

	// ASSUMPTION: td::chrono::system_clock() epoch is midnight

	// https://howardhinnant.github.io/date_algorithms.html#What%20can%20I%20do%20with%20that%20%3Ccode%3Echrono%3C/code%3E%20compatibility?

	const auto duration_since_epoch = std::chrono::system_clock().now().time_since_epoch();

	const auto secs_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch).count();

	// Modulo 86'400 to get seconds since midnight
	return secs_since_epoch % 86'400;

}

// -> decimal fractional seconds since midnight (up to micro or nano second accuracy)
var var::ostime() const {

	// ASSUMPTION: td::chrono::system_clock() epoch is midnight

	const auto duration_since_epoch = std::chrono::system_clock().now().time_since_epoch();

	// ms accuracy
	//std::uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//return now % 86400000000 / 1000000.0;

	// ns accuracy
	const auto ns_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch).count();

	// modulo 86'400E9 to get ns since midnight
	// div 10E9 to get decimal fractional seconds (since midnight)
	return static_cast<double>(ns_since_epoch % 86'400'000'000'000) / 1'000'000'000.0;

}

// -> decimal fractional days since pick epoch 1967-12-31 00:00:00 (up to micro or nano second accuracy)
var var::timestamp() const {

	var datenow = this->date();
	var timenow = this->ostime();

	// 1. If the date flipped while we got the time then get the time again
	// If someone is messing with the system clock then we will
	// fall for the bait only once.
	// 2. Assuming that date() is returned as an int for speed
	if (this->date().var_int != datenow.var_int)
		timenow = this->ostime();

	return datenow + timenow / 86'400;

}

// -> decimal fractional days since pick epoch 1967-12-31 00:00:00 (up to micro or nano second accuracy)
var var::timestamp(CVR ostime) const {

	THISIS("var var::timestamp(CVR ostime) const")
	assertNumeric(function_sig);
	assertNumeric(function_sig, ostime);

	return this->floor() + ostime / 86'400;

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

//	const char* conversionchar = conversion;
////	if (*conversionchar != 'D')
////		return *this;
//	++conversionchar;
//
//	while (*conversionchar) {
//		switch (*conversionchar) {
//			case 'E':
//				dayfirst = true;
//				break;
//			case 'S':
//				yearfirst = true;
//				break;
//		}
//		++conversionchar;
//	}

	std::string_view sv = conversion;
	for (const char charx : sv) {
		switch (charx) {
			case 'E':
				dayfirst = true;
				break;
			case 'S':
				yearfirst = true;
				break;
			default:;
				//[[fallthough]];
		}
	}

	const int maxnparts = 3;
	//int parts[maxnparts];
	std::array<int, maxnparts> parts;
	parts[0] = 0;
	int partn = -1;

	int month = 0;

	//const char* iter = var_str.c_str();
	//while (*iter != '\0') {
	auto iter = var_str.begin();
	const auto end = var_str.end();
	while (iter != end) {
		if (std::isdigit(*iter)) {

			partn++;

			// Get the first digit as an int and skip to the next input byte
			parts[partn] = (*iter++) - '0';

			// Accumulate any additional digits as ints
			// after multipying prior accumulation by 10
			while (std::isdigit(*iter))
				parts[partn] = (parts[partn] * 10) + ((*iter++) - '0');

			continue;
		}

		else if (std::isalpha(*iter)) {

			// Alphabetic months must only appear once
			if (month != 0)
				return "";

			// Build alphabetic month after uppercasing
			std::string word;
			do {
				// toupper returns an int despite being given a char
				// Presumably safe to cast back to char
				word.push_back(static_cast<char>(std::toupper(*iter++)));
			} while (std::isalpha(*iter));

//			// determine the month number 1 - 12
//			for (int ii = 0; ii < 12 * 4; ii += 4) {
//				if (strcmp(static_cast<const char*>(shortmths + ii), word.c_str()) == 0) {
//					month = ii / 4 + 1;
//					break;
//				}
//			}
//			if (month == 0)
//				return "";

			// Alphabetic month must be exactly 3 letters
			if (word.size() != 3)
				return "";

			// Find in the list of all three letter months
			auto pos = shortmths.find(word);
			if (pos == std::string::npos)
				return "";

			// Determine month number 1 - 12
			month = int(pos) / 4 + 1;

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
			var year2 = year;
			year += year_2000 - 100;
			var warning = "WARNING: 2 digit year " ^ year2.quote() ^ " between " ^ var(year_50) ^ " and 99 converted to " ^ var(year).quote();
			warning.logputl();
		} else
			year += year_2000;
	}

	// Check month
	if (month < 1 or month > 12)
		return "";

	// Table of max gregorian day per month
	constexpr const std::array maxdays_by_month = {
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
	return hinnant::gregorian::days_from_civil(year, month, day) - PICK_UNIX_DATE_OFFSET;

}

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
std::string var::oconv_D(const char* conversion) const {

	// by this time *this is known to be numeric and not an empty string

	// pick date uses floor() i.e. day 0.9 = day 0 and day -0.9 -> -1
	int pickdateno = this->floor();

	// convert to day, month and year
	int unixdayno = pickdateno + PICK_UNIX_DATE_OFFSET;
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

	const char* conversionchar = conversion;

//	const zstring_range conversion2{conversion};
//	auto conversionchar = conversion2.begin();

	// This function is only called when the 1st character is "D"
	// Bump to the next char
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
				return std::to_string(static_cast<int>(civil_month));

			// Not AREV
			// DW returns day of week number number 1-7 Mon-Sun
			// DWA returns the day of week name
			case 'W':
				// calculate directly from pick date (construction of date above is wasted
				dow = (((*this).floor() - 1) % 7) + 1;
				++conversionchar;
				if (*conversionchar == 'A')
					return &longdayofweeks[dow * 10 - 10];
				return std::to_string(dow);

			// Not AREV
			// DY year (four digits or D2Y works too)
			// DYn formatted
			case 'Y':
				yearfirst = true;
				// yearonly=true;
				// allow trailing digit as well to control ndigits
				++conversionchar;
				//if ((*conversionchar) <= '9' && (*conversionchar) >= '0')
				if ((*conversionchar) >= '0' && (*conversionchar) <= '9')
					yeardigits = (*conversionchar) - '0';
				--conversionchar;
				break;

			// Not AREV
			// DD day of month
			case 'D':
				return std::to_string(static_cast<int>(civil_day));

			// Not AREV
			// DQ returns quarter number
			case 'Q':
				return std::to_string(static_cast<int>((civil_month - 1) / 3) + 1);

			// Not AREV
			// DyJ returns day of year
			case 'J':
				//return static_cast<int>(desired_date.day_of_year());
				return std::to_string(unixdayno - hinnant::gregorian::days_from_civil(civil_year, 1, 1) + 1);

			// iso year format - at the beginning
			case 'S':
				// alphamonth=false;
				yearfirst = true;
				break;

			// DL LAST day of month
			case 'L':
				//return var(static_cast<int>(desired_date.end_of_month().day()));
				return std::to_string(static_cast<int>(hinnant::gregorian::last_day_of_month(civil_year, civil_month)));

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
	int yearstringerase = static_cast<int>(yearstring.size() - yeardigits);
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
		ss << shortmths.substr(civil_month * 4 - 4, 3);
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
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
std::string var::oconv_MT(const char* conversion) const {
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

	// Skip over leading "MT"
	conversionchar += 2;

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
		// otherwise floor any decimal seconds to an integer
		//
		// Fractional seconds using floor function where
		// A time is considered to be reached when you arrive at it exactly now before
		// So a fractional second before midnight is still 23:59:59
		// +0.9999 -> +0 -> 00:00:00
		// -0.0001 -> -1 -> 23:59:59
		timesecs = floor();
	}

	// standardise times <0 and >=86400 to one day ie 0-85399 seconds
	bool negative;
	if (unlimited) {
		negative = timesecs < 0;
		if (negative)
			timesecs = -timesecs;
	} else {
		negative = false;
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

//	var newmv;
//	newmv.var_typ = VARTYP_STR;
	std::string result;

	//hours first
	if (unlimited) {
		//unlimited hours
		if (negative)
			result = "-";
		//else
		//	newmv = "";
		if (hours < 10)
			result.push_back('0');
		result.append(std::to_string(hours));
	} else {
		// two digit hours
		//newmv = ("00" ^ var(hours)).last(2);
		result.push_back(static_cast<char>('0' + hours / 10));
		result.push_back(static_cast<char>('0' + hours % 10));
	}

	// separator
	result.push_back(sepchar);

	// two digit minutes
	//newmv ^= ("00" ^ var(mins)).last(2);
	result.push_back(static_cast<char>('0' + mins / 10));
	result.push_back(static_cast<char>('0' + mins % 10));

	if (showsecs) {

		// separator
		result.push_back(sepchar);

		// two digit seconds
		//newmv ^= ("00" ^ var(secs)).last(2);
		result.push_back(static_cast<char>('0' + secs / 10));
		result.push_back(static_cast<char>('0' + secs % 10));
	}

	// optional AM/PM suffix
	if (twelvehour) {
		if (am) {
			result.push_back('A');
			result.push_back('M');
		} else {
			result.push_back('P');
			result.push_back('M');
		}
	}

	return result;
}
#pragma GCC diagnostic pop

}  // namespace exo

#endif
