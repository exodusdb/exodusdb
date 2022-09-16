#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
namespace fs = std::filesystem;

#include <exodus/program.h>
programinit()

function main() {
	printl("p3 says 'Hello World!'");

	int n = 100000;
	var x = "abcdefghij";

	var started = ostime();
	for (int i = 0; i < n; i++) {
	}
	var stopped = ostime();
	var waste = stopped - started;

	started = ostime();
	for (int i = 0; i < n; i++) {
		var x = osfile("/root/neosys/src/dat/dict.definitions/SHORT_DATES");
	}
	stopped = ostime();

	printl(oconv(n,"MD00P,"),"ops,", oconv(stopped-started-waste,"MD30P"), "sec,", oconv((stopped-started-waste)*1'000'000'000/n, "MD00P,"), "ns/op,", x);

	printl(elapsedtimetext());

	return 0;
}

programexit()

