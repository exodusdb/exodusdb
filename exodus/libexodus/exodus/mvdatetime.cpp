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

#ifndef MVDATETIME_H
#define MVDATETIME_H

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

#include "boost/date_time/c_local_time_adjustor.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
//   #include <time.h>
#include <exodus/gettimeofday.h>
#else
#include <sys/time.h>
#endif

// work out the pick epoch date
static boost::gregorian::date pick_epoch_date = boost::gregorian::date(1967, 12, 31);

#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus {

void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime) {

	// http://www.boost.org/doc/html/date_time/examples.html#date_time.examples.local_utc_conversion
	using local_adj = boost::date_time::c_local_adjustor<boost::posix_time::ptime>;

	// convert to local timezone of current machine
	boost::posix_time::ptime localptimex = local_adj::utc_to_local(ptimex);

	// convert to local time of day
	boost::posix_time::time_duration timeofday = localptimex.time_of_day();
	mvtime = timeofday.hours() * 3600 + timeofday.minutes() * 60 + timeofday.seconds();

	// convert to local date
	boost::gregorian::date filedate = localptimex.date();
	// convert to mv days since 31/12/1967
	boost::gregorian::date dayzero(1967, 12, 31);  // an arbitrary date
	mvdate = (filedate - dayzero).days();

	return;
}

var var::date() const {
	// returns number of days since the pick date epoch 31/12/1967

	// http://www.boost.org/doc/html/date_time/examples.html#date_time.examples.days_alive
	// static const boost::gregorian::date dayzero(1967,12,31); //an arbitrary date
	boost::gregorian::date today = boost::gregorian::day_clock::local_day();
	return int((today - pick_epoch_date).days());
}

var var::time() const {
	// returns number of whole seconds since midnight
	boost::posix_time::time_duration localtimeofdaynow =
		boost::posix_time::second_clock::local_time().time_of_day();
	return int(localtimeofdaynow.hours() * 3600 + localtimeofdaynow.minutes() * 60 +
			   localtimeofdaynow.seconds());
}

var var::timedate() const {
	// output the current "HH:MM:SS  DD MMM YYYY" without quotes but note the double space

	// TODO make this rely on a single timestamp instead of time and date
	// to avoid the slight chance of time and date being called different sides of midnight
	return time().oconv_MT("S") ^ " " ^ date().oconv_D("D");
}

var var::ostime() const {
	// return decimal seconds since midnight up to micro or nano second accuracy

	boost::posix_time::ptime localtimenow = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration localtimeofdaynow = localtimenow.time_of_day();

	//#ifdef BOOST_HAS_FRACTIONAL_SECONDS
	return (localtimeofdaynow.total_nanoseconds() / 1000000000.0);
	/*
	#else
		struct timeval tv;
		gettimeofday(&tv, (struct timezone *) 0);
	    return double(localtimeofdaynow.total_seconds())+double(tv.tv_usec)/1000000.0;
	#endif
	*/
}

/*
// Checks keyboard buffer (stdin) and returns key
// pressed, or -1 for no key pressed
int var::keypressed(int delayusecs) const
{
    char keypressed;
    struct timeval waittime;
    int num_chars_read;
    struct fd_set mask;
    FD_SET(0, &mask);

    waittime.tv_sec = 0;
    waittime.tv_usec = delayusecs;
    if (std::select (1, &mask, 0, 0, &waittime))
    {
	num_chars_read = std::read (0, &keypressed, 1);
	if (num_chars_read == 1)
	{
		cin.putback(keypressed);
	    return (keypressed);
	}
    }

    return 0;
}
*/

var var::iconv_D(const char* conversion) const {

	// should perhaps ONLY implement only ISO8601 which is in xml
	// yyyy-mm-ddTHH:MM:SS.SSS

	// defaults
	bool yearfirst = false;
	bool dayfirst = false;

	// 1st character must be D otherwise no conversion
	const char* conversionchar = conversion;
	if (*conversionchar != 'D')
		return *this;
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

	const int maxparts = 9;
	int parts[maxparts];
	int partn = -1;

	int month = 0;

	const char* iter = var_str.c_str();
	while (*iter != '\0') {

		if (isdigit(*iter)) {

			partn++;

			// fail to convert if too many parts (should be only three really)
			if (partn >= maxparts)
				return "";

			parts[partn] = 0;

			do {
				parts[partn] = (parts[partn] * 10) + ((*iter++) - '0');
			} while (isdigit(*iter));

			continue;
		}

		else if (::isalpha(*iter)) {
			std::string word;
			do {
				word.push_back(toupper(*iter++));
			} while (::isalpha(*iter));

			// determine the month or return "" to indicate failure
			for (int ii = 0; ii < 12 * 4; ii += 4) {
				if (strcmp((char*)(shortmths + ii), word.c_str()) == 0) {
					month = ii / 4 + 1;
					break;
				}
			}
			if (month == 0)
				return "";

			continue;
		}

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
		if (year >= 50) {
			year += 1900;
			std::cerr << "WARNING: 2 digit year >= 50 being converted to " << year << std::endl;
		} else
			year += 2000;
	}

	try {
		boost::gregorian::date thisdate(year, month, day);
		return int((thisdate - pick_epoch_date).days());
	} catch (...) {
		return "";
	}
}

var var::oconv_D(const char* conversion) const {

	// by this time it is known to be a number and not an empty string

	// pick date uses floor()ie 1.9=1 and -1.9=-2

	// get a ymd object for the desired date
	// http://www.boost.org/doc/libs/1_39_0/doc/html/date_time/gregorian.html#date_time.gregorian.date_class
	boost::gregorian::date desired_date =
		pick_epoch_date + boost::gregorian::days((*this).floor().toInt());
	boost::gregorian::date::ymd_type ymd = desired_date.year_month_day();
	// boost::gregorian::date::ymd_type
	// ymd=boost::gregorian::gregorian_calendar::from_day_number(1000u/*+PICK_UNIX_DAY_OFFSET*/);

	// defaults
	bool alphamonth = true;
	int yeardigits = 4;
	bool yearfirst = false;
	bool dayfirst = false;
	// bool leadingzeros=true;
	char sepchar = ' ';

	// 1st character must be D otherwise no conversion
	const char* conversionchar = conversion;
	if (*conversionchar != 'D')
		return *this;
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
					return &longmths[ymd.month * 11 - 10];
				return var(ymd.month);

			// Not AREV
			// DW returns day of week number number 1-7 Mon-Sun
			// DWA returns the day of week name
			case 'W':
				// calculate directly from pick date (construction of date above is wasted
				// time) there appears to be only a name of day of week accessor in boost
				// date and no number of day of accessor
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
				return var(ymd.day);

			// Not AREV
			// DQ returns quarter number
			case 'Q':
				return var(int((ymd.month - 1) / 3) + 1);

			// Not AREV
			// DJ returns day of year
			case 'J':
				return int(desired_date.day_of_year());

			// iso year format - at the beginning
			case 'S':
				// alphamonth=false;
				yearfirst = true;
				break;

			// DL LAST day of month
			case 'L':
				return var(desired_date.end_of_month().day());

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
	yearstream << ymd.year;
	std::string yearstring = yearstream.str();
	int yearstringerase = int(yearstring.length() - yeardigits);
	if (yearstringerase > 0)
		yearstring.erase(0, yearstringerase);

	// year first
	if (yearfirst) {
		ss.width(yeardigits);
		// ss << ymd.year;
		// above doesnt cut down number of characters
		ss << yearstring;
		if (yearonly)
			return ss.str();
		ss << sepchar;
	}

	// day first
	if (alphamonth && not yearfirst)
		dayfirst = true;
	if (dayfirst) {
		ss.width(2);
		ss << ymd.day;
		ss << sepchar;
	}

	// month
	if (alphamonth) {
		// ss.width(3);
		ss << &shortmths[ymd.month * 4 - 4];
	} else {
		ss.width(2);
		ss << int(ymd.month);
	}

	// day last
	if (!dayfirst) {
		ss << sepchar;
		ss.width(2);
		ss << ymd.day;
	}

	// year last
	if (!yearfirst && yeardigits) {
		ss << sepchar;
		// ss << ymd.year;
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
	// bool decimalhours = false;
	char sepchar = ':';
	const char* conversionchar = conversion;
	int timesecs;

	// guess 1st option is most often zero/ie most conversions are MT only and short circuit
	if (!(*conversionchar)) {
		timesecs = this->round();

		// interpret conversion characters
	} else {

		// first may be an 2 to indicate input is integer or decimal hours
		// if so, convert by *3600 to seconds
		if (*conversionchar == '2') {
			++conversionchar;
			timesecs = (3600 * (*this)).round();

			// if not, just get the seconds
		} else {
			timesecs = round();
		}

		// first may be a U to indicate unlimited hours
		if (*conversionchar == 'U') {
			++conversionchar;
			unlimited = true;
		}

		// next may be a Z to convert 0 to ""
		if (*conversionchar == 'Z') {
			if (!timesecs)
				return "";
			++conversionchar;
		}

		// next may be an H to indicate AM/PM
		if (*conversionchar == 'H') {
			++conversionchar;
			twelvehour = true;
		}

		// next may be an S to show seconds
		if (*conversionchar == 'S') {
			++conversionchar;
			showsecs = true;
		}
		/* dont allow H after S
				//next may be an H to indicate unlimited time eg 25:00
				if (*conversionchar=='H')
				{
					++conversionchar;
					twelvehour=true;
				}
		*/
		// first of remaining characters is the separator. remainder are ignored
		if (*conversionchar) {
			sepchar = *conversionchar;
		}
	}

	// standardise times <0 and >=86400 to one day ie 0-85399 seconds
	var negative;
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

	// TODO two digit formatting in C instead of var

	// two digit hours
	var newmv;
	if (unlimited) {
		if (negative)
			newmv = "-";
		else
			newmv = "";
		if (hours < 10)
			newmv ^= "0";
		newmv ^= hours;
	} else
		newmv = ("00" ^ var(hours)).substr(-2);

	// separator
	newmv ^= sepchar;

	// two digit minutes
	newmv ^= ("00" ^ var(mins)).substr(-2);

	if (showsecs) {

		// separator
		newmv ^= sepchar;

		// two digit seconds
		newmv ^= ("00" ^ var(secs)).substr(-2);
	}

	// AM/PM
	if (twelvehour) {
		if (am)
			newmv ^= "AM";
		else
			newmv ^= "PM";
	}

	return newmv;
}

}  // namespace exodus

#endif
