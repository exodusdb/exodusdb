#ifndef EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_
#define EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_

#ifdef EXODUS__NO_TIMEBANK
	struct TimeAcc{TimeAcc(int){}};
#else

#include <chrono>
#include <iostream>
#include <memory>
#include <iomanip>

#include <exodus/var.h>

namespace exodus {

void get_timeacc0();

inline struct TimeBank {

	struct Acc {
		uint64_t count;
		uint64_t ns;
	};

	std::array<Acc, 1000> timeaccs_;
	std::array<std::string, 1000> timeacc_names_;

	~TimeBank() {

		// Work out the time for a pure deposit
		// to be deducted from every time account x ndeposits
		get_timeacc0();

		//TODO make independent of install location
		var timeacc_info;
		timeacc_info.osread("/usr/local/share/exodus/timeaccs.txt");
		timeacc_info.converter("\n ", _FM _VM);
		for (var timeacc_item : timeacc_info)
			timeacc_names_.at(timeacc_item.f(1, 1)) = timeacc_item.field(_VM, 2,999).c_str();

		std::cout << std::endl;

		// Print out all the time accounts
		// Skip time_acno 0
		int naccs = 0;
		for (unsigned time_acno = 1; time_acno < timeaccs_.size(); time_acno++) {
			if (timeaccs_[time_acno].count) {

				auto ns = timeaccs_[time_acno].ns;

				// Deduct pure deposit time * ndeposits
				if (time_acno) {
					auto tot_pure_deposit_time = timeaccs_[0].ns/100 * timeaccs_[time_acno].count;
					if (ns > tot_pure_deposit_time)
						ns -= tot_pure_deposit_time;
					else
						//instead of zero put 1 to indicate something special
						ns = 1;
				}

				// Print column headings
				if (naccs++ == 0)
					std::cerr << "A/c.       n      ms   ms/op        μs     μs/op          ns       ns/op name" << std::endl;

				// Print a row
				std::cerr

				// time_acno
				<< std::right << std::setw(4)
				<< time_acno

				// Count
				<< std::right << std::setw(8)
				<< timeaccs_[time_acno].count

				// ms
				<< std::right << std::setw(8)
				<< ns / 1'000'000

				// ms/op
				<< std::right << std::setw(8)
				<< ns / 1'000'000/timeaccs_[time_acno].count

				// us
				<< std::right << std::setw(10)
				<< ns / 1'000

				// us/op
				<< std::right << std::setw(10)
				<< ns/1000/timeaccs_[time_acno].count

				// ns
				<< std::right << std::setw(12)
				<< ns

				//ns/op
				<< std::right << std::setw(12)
				<< ns/timeaccs_[time_acno].count << " "

				// timeacc name
				<< timeacc_names_[time_acno]

				<< std::endl;
			}
		}
	}

} timebank;

struct TimeAcc {

	int time_acno_ = 0;

	std::chrono::system_clock::time_point start_;

	TimeAcc() = delete;

	explicit TimeAcc(const int time_acno)
		:
		time_acno_(time_acno) {

		//std::cout << time_acno_ << " ~" << std::endl;
		start_ = std::chrono::high_resolution_clock::now();
	}

	~TimeAcc() {
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_).count();
		//std::cout << time_acno_ << " +" << ns << std::endl;
		timebank.timeaccs_[time_acno_].ns += ns;
		timebank.timeaccs_[time_acno_].count++;
	}


};

void get_timeacc0() {

	// 100 deposits to estimate pure deposit time
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
	{TimeAcc {0};}
}

} // namespace exodus
#endif // EXO_TIMEBANK
#endif // EXODUS_LIBEXODUS_EXODUS_TIMEBANK_H_
