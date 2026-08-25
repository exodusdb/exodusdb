// Archived elapsedtimetext implementation (not built).
// Saved before Go-inspired rewrite. Live code: elapsedtime.cpp.
// Original lived in exoprog.cpp as ExoProgram methods.

#include <exodus/exoprog.h>

// --- begin archived ---

// elapsedtimetext 1 - from program start/TIMESTAMP
var ExoProgram::elapsedtimetext() const {
	return elapsedtimetext(TIMESTAMP, var().ostimestamp());
}

// elapsedtimetext 2 - given two timestamps
var ExoProgram::elapsedtimetext(in timestamp1, in timestamp2) const {

	var text = "";

	var secs = (timestamp2 - timestamp1) * 86'400;

	let weeks = (secs / 604800).floor();
	secs -= weeks * 604800;

	let days = (secs / 86400).floor();
	secs -= days * 86400;

	let hours = (secs / 3600).floor();
	secs -= hours * 3600;

	let minutes = (secs / 60).floor();
	secs -= minutes * 60;

	if (weeks) {
		text.appender(weeks ^ " week");
		if (weeks ne 1) {
			text ^= "s";
		}
	}
	if (days) {
		text.appender(FM_, days, " day");
		if (days ne 1) {
			text ^= "s";
		}
	}
	if (hours) {
		text.appender(FM_, hours, " hour");
		if (hours ne 1) {
			text ^= "s";
		}
	}
	if (minutes) {
		text.appender(FM_, minutes, " min");
		if (minutes ne 1) {
			text ^= "s";
		}
	}
	if (not(hours) and (minutes < 5)) {
		if (secs) {
			if (minutes or (secs - 10 > 0)) {
				secs = secs.round(0);
			} else {
				//secs = (secs.oconv("MD40P")) + 0;
				// ≥ 10ms: round to ms (3 d.p. on seconds). < 10ms: leave 6 d.p. as before.
				secs = secs.round(secs >= 0.01 ? 3 : 6) + 0;
				if (secs.starts(".")) {
					secs.prefixer("0");
				}
			}
			// Under 1s show as ms (124 ms) not fractional secs (0.124 secs)
			if (secs >= 1) {
				text.appender(FM_, secs, " sec");
				if (secs ne 1) {
					text ^= "s";
				}
			} else if (not minutes and not hours and not days and not weeks) {
zero:
				if (not text) {
					//text.appender(FM_, "< 1 ms");
					var ms = secs * 1000;
					if (ms < 0.1) {
						var μs = ms * 1000;
						if (timestamp1.toDouble() == timestamp2.toDouble()) {
							text = "0 secs";
						} else if (μs < 0.1) {
								var ns = ms * 1'000'000;
								// +0 drops trailing zeros from round (83.000 → 83)
								text = (ns.round(3) + 0) ^ " ns";
						} else
							text = (μs.round(3) + 0) ^ " μs";
					} else
						text = (ms.round(3) + 0) ^ " ms";
				}
			} else {
				//text.appender(FM_, "exactly");
			}
		} else {
			if (not minutes and not hours and not days and not weeks) {
				goto zero;
			}
			//text.appender(3, "exactly");
		}
	}

//	text.replacer(FM ^ FM ^ FM, FM);
//	text.replacer(FM ^ FM, FM);
	text.trimmer(_FM);
	text.replacer(_FM, ", ");

	return text;
}


// --- end archived ---
