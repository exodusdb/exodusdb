#undef NDEBUG
#include <cassert>

#include <exodus/program.h>
programinit()

// Self-check ExoProgram::elapsedtimetext (exodus/elapsedtime.cpp).
// 1000 log-uniform samples; parse prose back to ns; |Δ| <= 1%.

function text_to_ns(in text0) {

	let sec_ns = 1'000'000'000;
	let min_ns = sec_ns * 60;
	let hour_ns = min_ns * 60;
	let day_ns = hour_ns * 24;
	let ms_ns = 1'000'000;
	let us_ns = 1'000;

	var text = text0;
	text.converter(",", FM);
	var total = 0;
	let n = text.count(FM) + 1;
	for (var i = 1; i <= n; ++i) {
		var part = text.f(i);
		part.trimmer();
		if (not part)
			continue;
		var num = part.field(" ", 1);
		var unit = part.field(" ", 2, 99);
		if (unit.ends("s") and unit ne "ms" and unit ne "μs" and unit ne "ns")
			unit = unit.first(unit.len() - 1);

		var unit_ns = 0;
		if (unit eq "day")
			unit_ns = day_ns;
		else if (unit eq "hour")
			unit_ns = hour_ns;
		else if (unit eq "min")
			unit_ns = min_ns;
		else if (unit eq "sec")
			unit_ns = sec_ns;
		else if (unit eq "ms")
			unit_ns = ms_ns;
		else if (unit eq "μs")
			unit_ns = us_ns;
		else if (unit eq "ns")
			unit_ns = 1;
		else
			assert(false and ("unknown unit " ^ unit));

		total += (num * unit_ns).round(0);
	}
	return total;
}

function probe(in label, in days, in check) {
	var nanos = (days * var(86'400) * var(1'000'000'000)).round(0);
	if (nanos < 0)
		nanos = -nanos;
	var text = elapsedtimetext(0, days);
	if (text eq "100 years+") {
		printl(label.oconv("L#28"), text);
		return "";
	}
	var recovered = text_to_ns(text);
	var diff = (recovered - nanos).abs();
	var pct = 0;
	if (nanos)
		pct = (diff / nanos * 100).round(3);
	printl(label.oconv("L#28"), text.oconv("L#40"), " Δ%=", pct, "%");
	if (check and nanos and diff > nanos / 100) {
		printl("FAIL original=", nanos, " recovered=", recovered, " diff=", diff);
		assert(false and "recovered ns differs by more than 1%");
	}
	return "";
}

func main() {

	printl("test_elapsed says 'Hello World!'");

	initrnd(1);  // fixed seed — same sequence every run

	let min_ns = 1;
	let max_ns = var("9223372036854775807");  // int64 max ns (~292y)

	printl("--- fixed ---");
	probe("7 days", 7.0, 1);
	probe("3.5 days", 3.5, 1);
	probe("1 day", 1.0, 1);
	probe("1 sec", 1.0 / 86400, 1);
	probe("1 ms", 1.0 / 86400 / 1000, 1);
	probe("~104 days (2^53 ns)", var(2).pwr(53) / (var(86'400) * var(1'000'000'000)), 0);
	probe("200 days", 200.0, 0);
	probe("1 year", 365.0, 0);
	probe("10 years", 3650.0, 0);
	probe("100 years", 36500.0, 0);
	probe("century+1d", 36526.0, 0);
	probe("~292y (int64 ns max)", var("9223372036854775807") / (var(86'400) * var(1'000'000'000)), 0);
	printl("");

	printl("--- 1000 random samples ---");
	for (var i = 1; i <= 1000; ++i) {

		let u = (rnd(1'000'000'000) + 1) / 1'000'000'000.0;
		let log_min = min_ns.loge();
		let log_max = max_ns.loge();
		var nanos = (log_min + u * (log_max - log_min)).exp();
		nanos = nanos.round(0);
		if (nanos < 1)
			nanos = 1;

		let days = nanos / (var(86400) * var(1'000'000'000));
		let text = elapsedtimetext(0, days);

		if (text eq "100 years+") {
			printl(i.oconv("R#4"), "  ", text);
			continue;
		}

		let recovered = text_to_ns(text);
		let diff = (recovered - nanos).abs();
		let pct = (diff / nanos * 100).round(3);
		if (diff > nanos / 100) {
			printl("FAIL text=", text.squote());
			printl("  original=", nanos, " recovered=", recovered);
			printl("  diff=", diff, "  Δ%=", pct, "%  (limit 1%)");
			assert(false and "recovered ns differs by more than 1%");
		}

		var hint;
		if (nanos < 1'000'000'000)
			hint = (nanos / 1'000'000.0).round(3) ^ " ms raw";
		else if (nanos < var(86400) * var(1'000'000'000))
			hint = (nanos / 1'000'000'000.0).round(3) ^ " s raw";
		else
			hint = (nanos / (var(86400) * var(1'000'000'000))).round(3) ^ " d raw";

		printl(i.oconv("R#4"), "  ", text.oconv("L#40"), "  ok Δ%=", pct, "%  (", hint, ")");
	}

    printl(elapsedtimetext());
	printl("Test passed");
	return 0;
}

programexit()
