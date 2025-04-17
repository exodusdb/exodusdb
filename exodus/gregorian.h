#ifndef GREGORIAN_HPP
#define GREGORIAN_HPP

// Can be replaced with std::chrono equivalents available in g++ v11 on ubuntu 22.04

#include <limits>
#include <tuple>

// Implement proleptic gregorian civil calendar based on years, months and days
//
// https://howardhinnant.github.io/date_algorithms.html
//
// https://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar

// Thanks Howard!

/*
	The algorithms are interoperable with every known implementation of
	std::chrono::system_clock, 	though that interoperability depends on an unspecified
	property of the system_clock: its epoch. Example code will be shown how these
	algorithms can take advantage of the common (but unspecified) system_clock epoch.

	The algorithms implement a proleptic Gregorian calendar.
	That is, the rules which adopted the Julian calendar in 1582 in Rome are applied both
	backwards and forwards in time. This includes a year 0, and then negative years before
	that, all following the rules for the Gregorian calendar.
	From hence forth this paper will refer to this as the civil calendar.
	The accuracy of the algorithms under these rules is exact, until overflow occurs.
	Using 32 bit arithmetic, overflow occurs approximately at +/- 5.8 million years.
	Using 64 bit arithmetic overflow occurs far beyond +/- the age of the universe.
	The intent is to make range checking superfluous.

	The algorithms implement no validity checking. The intent is that any desired validity checking
	on the triple year/month/day can be added on top of these algorithms if and where desired.
*/

namespace hinnant::gregorian {

/**************
days_from_civil
**************/

//https://howardhinnant.github.io/date_algorithms.html#days_from_civil
// Returns number of days since civil 1970-01-01.  Negative values indicate
//    days prior to 1970-01-01.
// Preconditions:  y-m-d represents a date in the civil calendar
//                 m is in [1, 12]
//                 d is in [1, last_day_of_month(y, m)]
//                 y is "approximately" in
//                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
//                 Exact range of validity is:
//                 [civil_from_days(numeric_limits<Int>::min()),
//                  civil_from_days(numeric_limits<Int>::max()-719468)]

template <class Int>
constexpr
Int
days_from_civil(Int y, unsigned m, unsigned d) noexcept
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const Int era = (y >= 0 ? y : y-399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153*(m > 2 ? m-3 : m+9) + 2)/5 + d-1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe) - 719468;
}

/**************
civil_from_days
**************/

// https://howardhinnant.github.io/date_algorithms.html#civil_from_days
// Returns year/month/day triple in civil calendar
// Preconditions:  z is number of days since 1970-01-01 and is in the range:
//                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].

template <class Int>
constexpr
std::tuple<Int, unsigned, unsigned>
civil_from_days(Int z) noexcept
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    z += 719468;
    const Int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const Int y = static_cast<Int>(yoe) + era * 400;
    const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    const unsigned mp = (5*doy + 2)/153;                                   // [0, 11]
    const unsigned d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
    const unsigned m = mp < 10 ? mp+3 : mp-9;                            // [1, 12]
    return std::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
}

/******
is_leap
******/

//https://howardhinnant.github.io/date_algorithms.html#is_leap
// Returns: true if y is a leap year in the civil calendar, else false

template <class Int>
constexpr
bool
is_leap(Int y) noexcept
{
    return  y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

/****************************
last_day_of_month_common_year
****************************/

//https://howardhinnant.github.io/date_algorithms.html#last_day_of_month_common_year
// Preconditions: m is in [1, 12]
// Returns: The number of days in the month m of common year
// The result is always in the range [28, 31].

constexpr
unsigned
last_day_of_month_common_year(unsigned m) noexcept
{
    constexpr unsigned char a[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return a[m-1];
}

/****************
last_day_of_month
****************/

//https://howardhinnant.github.io/date_algorithms.html#last_day_of_month
// Preconditions: m is in [1, 12]
// Returns: The number of days in the month m of year y
// The result is always in the range [28, 31].

template <class Int>
constexpr
unsigned
last_day_of_month(Int y, unsigned m) noexcept
{
    return m != 2 || !is_leap(y) ? last_day_of_month_common_year(m) : 29u;
}

} // namespace

#endif
