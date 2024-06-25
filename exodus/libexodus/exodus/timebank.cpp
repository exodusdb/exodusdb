// EXODUS_TIMEBANK must be defined to create functional Timers
// e.g. cmake . -DEXODUS_TIMEBANK=1

#ifdef EXODUS_TIMEBANK

//#include <chrono>
#include <iostream>
//#include <memory>
#include <iomanip>
#include <cstring> // for strcmp

#include <exodus/var.h>
#include <exodus/timebank.h>

namespace exodus {

void get_timeacc0();

TimeBank::~TimeBank() {

		// Note that you cannot use global variables like FM
		// which may already have destructed. Use _FM constants.

		// Work out the time for a pure deposit
		// to be deducted from every time account x ndeposits
		get_timeacc0();
		auto pure_deposit_time =timeaccs_[0].ns/100;
		// If we cannot call the above program during destructor/program termination
		// auto pure_deposit_time = 50;//very crude underestimate probably

		//TODO make independent of install location
		var timeacc_list;
		if (timeacc_list.osread("/usr/local/share/exodus/timebank.txt")) {
			timeacc_list.converter("\n ", _FM _VM);
			for (var timeacc_item : timeacc_list) {
				timeacc_names_.at(timeacc_item.f(1, 1)) = timeacc_item.field(_VM, 2, 999).convert(_VM, " ").toString();
			}
		}

		// Print out all the time accounts
		// Skip time_acno 0
		int naccs = 0;
		for (unsigned time_acno = 1; time_acno < timeaccs_.size(); time_acno++) {
			auto count = timeaccs_[time_acno].count;
			if (not count)
				continue;

			auto ns = timeaccs_[time_acno].ns;

			// Deduct pure deposit time * ndeposits
			if (time_acno) {
				auto tot_pure_deposit_time = pure_deposit_time * count;
				if (ns > tot_pure_deposit_time)
					ns -= tot_pure_deposit_time;
				else
					//instead of zero put 1 to indicate something special
					ns = 1;
			}

			// Print column headings
			if (naccs++ == 0) {
				std::cout << std::endl;
				std::cerr << "A/c.       n      ms   ms/op        μs     μs/op          ns       ns/op name" << std::endl;
			}

			// Print a row
			std::cerr

			// time_acno
			<< std::right << std::setw(4)
			<< time_acno

			// Count
			<< std::right << std::setw(8)
			<< count

			// ms
			<< std::right << std::setw(8)
			<< ns / 1'000'000

			// ms/op
			<< std::right << std::setw(8)
			<< ns / 1'000'000/count

			// us
			<< std::right << std::setw(10)
			<< ns / 1'000

			// us/op
			<< std::right << std::setw(10)
			<< ns/1000/count

			// ns
			<< std::right << std::setw(12)
			<< ns

			//ns/op
			<< std::right << std::setw(12)
			<< ns/count << " "

			// timeacc name
			<< timeacc_names_[time_acno]

			<< std::endl;
		} // next timeacc
	} // ~TimeBank

void get_timeacc0() {

	// 100 deposits to estimate pure deposit time
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
	{Timer {0};}
}

} // namespace exodus
#endif // EXODUS_TIMEBANK
