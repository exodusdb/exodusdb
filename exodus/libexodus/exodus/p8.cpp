#include <chrono>

#include <exodus/program.h>
programinit()

function main() {

	{
		//uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		uint64_t now1 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		uint64_t now2 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		//TRACE(now % 86400000000);
		TRACE(now1 % 86400000000000 / 1000000000.0)
		TRACE(now2 % 86400000000000 / 1000000000.0)

		printl(ostime());
	}

stop();
	TRACE(ostime());
    const auto duration_since_epoch = std::chrono::high_resolution_clock().now().time_since_epoch();

    using nanosecs = std::chrono::duration<int, std::ratio_multiply<std::chrono::seconds::period, std::ratio<1>>>;
    const auto duration_in_nanosecs = std::chrono::duration_cast<nanosecs>(duration_since_epoch);

TRACE(duration_in_nanosecs.count()/1000000000.0)

	return 0;
}

programexit()

