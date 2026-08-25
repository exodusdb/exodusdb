#if EXO_MODULE > 1
	import std;
#else
#	include <iostream>
#endif

#if EXO_MODULE
	import var;
#else
#	include <var/dim.h>
#	include <var/rex.h>
#endif

// Do not define EXO_EXOPROG_CPP here — that defines threadpool1 (only in exoprog.cpp).
#include <exodus/exoprog.h>
#include <exodus/exomacros.h>

namespace exo {

// elapsedtimetext — days→ns cascade (no weeks).
// Prior: elapsedtimetext-old.cpp (3-sig grain / week cascade; not built).
// 1) Abs duration; if > 36525 days (~century) → "100 years+".
// 2) Highest non-zero level sets sig_ns = contribution/100.
// 3) Subsecond levels: floor count to sig_ns (no round-up).
// 4) One smudge bottom-up: μs←ns, else ms←μs, else s←ms.
// 5) Emit non-zero parts with +s plurals (ms/μs/ns no plural s).

namespace {

void append_prose_unit(var& text, in n0, const char* word, bool plural_s = true) {
	if (not n0)
		return;
	if (text)
		text ^= ", ";
	text ^= n0 ^ " " ^ word;
	if (plural_s and n0 ne 1)
		text ^= "s";
}

}  // namespace

// elapsedtimetext 1 - from program start/TIMESTAMP
var ExoProgram::elapsedtimetext() const {
	return elapsedtimetext(TIMESTAMP, var().ostimestamp());
}

// elapsedtimetext 2 - given two timestamps (days, float)
var ExoProgram::elapsedtimetext(in timestamp1, in timestamp2) const {

	var day_ns = 86'400.0 * 1'000'000'000;  // float — long spans without int overflow
	var remaining = ((timestamp2 - timestamp1) * day_ns).abs();
	if (remaining > 36525 * day_ns)
		return "100 years+";
	remaining = remaining.round(0);
	if (remaining == 0)
		return "0 secs";

	var sec_ns = 1'000'000'000;
	var min_ns = sec_ns * 60;
	var hour_ns = min_ns * 60;
	var ms_ns = 1'000'000;
	var us_ns = 1'000;

	var days = 0, hours = 0, mins = 0, secs = 0, ms = 0, us = 0, ns = 0;
	var sig_ns = 0;  // from highest level only

	auto take = [&](var& count_out, in unit_size) {
		if (remaining == 0)
			return;
		var count = (remaining / unit_size).floor();
		if (count == 0)
			return;
		var contribution = count * unit_size;

		if (not sig_ns)
			sig_ns = contribution / 100;  // highest level only
		else if (unit_size <= ms_ns and sig_ns >= 1) {
			// subsecond only: floor level number to highest sig (no round-up)
			contribution = ((contribution / sig_ns).floor() * sig_ns).round(0);
			count = (contribution / unit_size).floor();
			if (count == 0)
				return;  // remaining unchanged
		}

		count_out = count;
		remaining -= contribution;
		if (remaining < sig_ns)
			remaining = 0;
	};

	take(days, day_ns);
	take(hours, hour_ns);
	take(mins, min_ns);
	take(secs, sec_ns);
	take(ms, ms_ns);
	take(us, us_ns);
	take(ns, 1);

	// One smudge only: first applicable pair bottom-up. e.g. 1 μs + 45 ns → 1.045 μs.
	auto fold = [&](var& higher, var& lower) {
		if (not lower)
			return;
		var frac = lower;
		while (frac.len() < 3)
			frac.prefixer("0");
		higher = (higher ^ "." ^ frac) + 0;  // +0 trims trailing zeros
		lower = 0;
	};
	if (us and ns)
		fold(us, ns);
	else if (ms and us)
		fold(ms, us);
	else if (secs and ms)
		fold(secs, ms);

	var text = "";
	append_prose_unit(text, days, "day");
	append_prose_unit(text, hours, "hour");
	append_prose_unit(text, mins, "min");
	append_prose_unit(text, secs, "sec");
	append_prose_unit(text, ms, "ms", /*plural_s=*/false);
	append_prose_unit(text, us, "μs", /*plural_s=*/false);
	append_prose_unit(text, ns, "ns", /*plural_s=*/false);

	if (not text)
		text = "0 secs";

	return text;
}

}  // namespace exo
