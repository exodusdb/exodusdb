#include <exodus/program.h>
programinit()

var tempfilename;
var rec;
var tempfile;
var starttime;	// num
var endtime;	// num
var testtime;	// num

func main() {

	let deletefilex = 1;

	let usetransaction = true;

	if (deletefilex) {
		tempfilename = "BENCHMARK" ^ SYSTEM.f(24);
	} else {
		tempfilename = "BENCHMARK_FILESPEED";
	}

	if (deletefilex) {
		if (var().open(tempfilename) and not deletefile(tempfilename ^ " (S)")) {
			abort(lasterror());
		}
	}
	clearselect();

	printl("Syntax is FILESPEED nreps recsize ntests");
	var nreps = SENTENCE.field(" ", 2);
	if (not nreps) {
		nreps = 1000;
	}

	var recsize = SENTENCE.field(" ", 3);
	if (not recsize) {
		recsize = 1024;
	}

	var ntests = SENTENCE.field(" ", 4);
	if (not ntests) {
		ntests = 10;
	}

	var testn	= 0;
	var tottime = 0;
	var mintime = 999999;
	var maxtime = "";
	var avgtime = "";
	printl(chr(12), "writing then deleting ", nreps, " x ", recsize, " byte records ", ntests, " times");
	printl("Test", "\t", "Time", "\t", "Min.", "\t", "Avg.", "\t", "Max.");
	var minspeed = 0;
	var maxspeed = 0;
	var avgspeed = 0;

/////////
nexttest:
/////////
	if (testn >= ntests) {
		goto exit;
	}
	testn += 1;

	if (var().hasinput())
		stop();

	if (esctoexit()) {
		stop();
	}

	if (not createfile(tempfilename ^ " (S)")) {
		abort(lasterror());
	}

	if (esctoexit()) {
		goto exit;
	}
	rec = var("X").str(recsize);

	if (not tempfile.open(tempfilename, "")) {
		abort(lasterror());
	}

	starttime = ostime();

	if (usetransaction)
		if (not begintrans())
			abort(lasterror());

	// Write
	for (const var ii : range(1, nreps)) {
		rec.write(tempfile, ii);
	}

	// Read
	for (const var ii : range(1, nreps)) {
		if (not rec.read(tempfile, ii))
			abort(lasterror());
	}

	// Delete
	for (const var ii : range(1, nreps)) {
		tempfile.deleterecord(ii);
	}

	if (usetransaction)
		if (not committrans())
			loglasterror();

	endtime = ostime();

	testtime = (endtime - starttime) * 1000 / nreps;
	tottime += testtime;
	if (testtime < mintime) {
		mintime = testtime.oconv("MD20P");
	}
	if (testtime > maxtime) {
		maxtime = testtime.oconv("MD20P");
	}
	avgtime = (tottime / testn).oconv("MD20P");

	minspeed = (1 / maxtime).oconv("MD20P");
	avgspeed = (1 / avgtime).oconv("MD20P");
	maxspeed = (1 / mintime).oconv("MD20P");
	printl(testn ^ ".", "\t", testtime.oconv("MD20P"), "\t", minspeed, "\t", avgspeed, "\t", maxspeed);

	if (deletefilex) {
        if (var().open(tempfilename) and not deletefile(tempfilename ^ " (S)")) {
            abort(lasterror());
        }
	}

	goto nexttest;

/////
exit:
/////
	if (deletefilex) {
        if (var().open(tempfilename) and not deletefile(tempfilename ^ " (S)")) {
            abort(lasterror());
        }
	}

	var msg = "";
	msg(-1) = "FILESPEED Min/Avg/Max = " ^ minspeed ^ " / " ^ avgspeed ^ " / " ^ maxspeed;
	msg(-1) = "More is faster.";
	msg(-1) = "Typical speed for an entry level server in 2007 was 1";

	call note(msg);
	return 0;
}

}; // programexit()
