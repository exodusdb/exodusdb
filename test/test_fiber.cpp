#include <cassert>
#include <exodus/program.h>

programinit()

func echo(const var& value) {
	set_async_result(value, "echo");
	return 0;
}

func add(const var& value1, const var& value2) {
	set_async_result(value1 + value2, "add");
	return 0;
}

function main() {
	printl("test_fiber says 'Hello World!'");

	// 1. co-run some member function;
	//////////////////////////////////
	in N1 = 10'000;
	var t1 = ostimestamp();
	for (var i : range(1, N1)) {

		async(&_ExoProgram::echo, this, i);

		async(&_ExoProgram::add , this, i, i);

	}

	var t2 = (ostimestamp() - t1) / N1 / 2;
	printl(elapsedtimetext(0, t2));

	yield();

	// 2. Iterate over completed fiber results
	//////////////////////////////////////////
	var t3 = ostimestamp();
	var n_async = 0;
	var total1 = 0;
	var total2 = 0;
	for (auto& result [[maybe_unused]] : async_results()) {
		if (result.message.starts("e"))
			total1 += result.data;
		else if (result.message.starts("a"))
			total2 += result.data;
		else {
			result.message.errputl("result.message=");
			assert(false && "message should be echo or add");
		}
		n_async ++;
	}

	var t4 = (ostimestamp() - t3) / n_async;
	printl(elapsedtimetext(0, t4));

	assert(n_async.errputl("n_async=") eq 20000);
	assert(total1.errputl("total1=") eq 50005000);
	assert(total2.errputl("total2=") eq 100010000);

	{
		for (var i : range(1, 100))
			async(&_ExoProgram::add2, this, i, 2);
		yield();
		var total = 0;
		for (auto result : async_results())
		total += result.data;
		assert(total eq 5250);
	}

	printl(elapsedtimetext());

	printl("Test passed.");

	return 0;
}

func add2(in a, in b) {
	set_async_result(a + b, "add2");
	return "";
}

}; // programexit()
