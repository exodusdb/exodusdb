#include <cassert>
#include <exodus/program.h>

programinit()

func echo(const var& value) {
	set_run_result(value, "echo");
	return 0;
}

func add(const var& value1, const var& value2) {
	set_run_result(value1 + value2, "add");
	return 0;
}

function main() {
	printl("test_fiber says 'Hello World!'");

	// 1. co-run some member function;
	//////////////////////////////////
	in N1 = 10'000;
	var t1 = ostimestamp();
	for (var i : range(1, N1)) {

		co_run(&_ExoProgram::echo, this, i);

		co_run(&_ExoProgram::add , this, i, i);

	}

	var t2 = (ostimestamp() - t1) / N1 / 2;
	printl(elapsedtimetext(0, t2));

	yield();

	// 2. Iterate over completed fiber results
	//////////////////////////////////////////
	var t3 = ostimestamp();
	var n_co_run = 0;
	var total1 = 0;
	var total2 = 0;
	for (auto& result [[maybe_unused]] : co_run_results()) {
		if (result.message.starts("e"))
			total1 += result.data;
		else if (result.message.starts("a"))
			total2 += result.data;
		else {
			result.message.errputl("result.message=");
			assert(false && "message should be echo or add");
		}
		n_co_run ++;
	}

	var t4 = (ostimestamp() - t3) / n_co_run;
	printl(elapsedtimetext(0, t4));

	assert(n_co_run.errputl("n_co_run=") eq 20000);
	assert(total1.errputl("total1=") eq 50005000);
	assert(total2.errputl("total2=") eq 100010000);

	{
		for (var i : range(1, 100))
			co_run(&_ExoProgram::add2, this, i, 2);
		yield();
		var total = 0;
		for (auto result : co_run_results())
		total += result.data;
		assert(total eq 5250);
	}

	printl(elapsedtimetext());

	printl("Test passed.");

	return 0;
}

func add2(in a, in b) {
	set_run_result(a + b, "add2");
	return "";
}

}; // programexit()
