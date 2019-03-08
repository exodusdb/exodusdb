#include <exodus/program.h>
programinit()


#include <gen.h>

var tempfilename;
var rec;
var tempfile;
var starttime;//num
var endtime;//num
var testtime;//num

function main() {
	//c test
	//global testtime,tempfile,rec

	//declare function esc.to.exit
	var deletefile = 1;

	if (deletefile) {
		tempfilename = "BENCHMARK" ^ SYSTEM.a(24);
	}else{
		tempfilename = "BENCHMARK.FILESPEED";
	}

	//locate tempfilename in @files using @fm setting x then
	if (deletefile) {
		perform("DELETEFILE " ^ tempfilename ^ " (S)");
	}
	clearselect();
	// end

	//syntax is FILESPEED nreps(1000) recsize(1000) ntests(10)
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

	var testn = 0;
	var tottime = 0;
	var mintime = 999999;
	var maxtime = "";
	var avgtime = "";
	printl(var().chr(12), "writing then deleting ", nreps, " x ", recsize, " byte records ", ntests, " times");
	printl("     Test" , "\x09" , "Time" , "\x09" , "Min." , "\x09" , "Avg." , "\x09" , "Max.");
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
		var().stop();
	}

	//PERFORM 'MAKEFILE ':TEMPFILENAME:' ':recsize:' ':N:' (S)'
	perform("MAKEFILE " ^ tempfilename ^ " (S)");
	//perform 'SELECT ':tempfilename

	//garbagecollect;

	if (esctoexit()) {
		goto exit;
	}
	rec = var("X").str(recsize);

	if (not(tempfile.open(tempfilename, ""))) {
		call fsmsg();
		var().stop();
	}

	starttime = ostime();

	begintrans();

	//print
	//print 'Writing 1Kb records'
	for (var ii = 1; ii <= nreps; ++ii) {
//		if (esctoexit()) {
//			goto exit;
//		}
		print(var().at(0), "W", ii);
		rec.write(tempfile, ii);
	};//ii;

	//print
	//print 'Deleting 1Kb records'
	for (var ii = 1; ii <= nreps; ++ii) {
//		if (esctoexit()) {
//			goto exit;
//		}
		print(var().at(0), "D", ii);
		tempfile.deleterecord(ii);
	};//ii;

	committrans();

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

	//print ' ':testn:'. ',testtime 'MD20P',mintime,avgtime,maxtime
	minspeed = (1 / maxtime).oconv("MD20P");
	avgspeed = (1 / avgtime).oconv("MD20P");
	maxspeed = (1 / mintime).oconv("MD20P");
	printl(" ", testn, ". " , "\x09" , testtime.oconv("MD20P") , "\x09" , minspeed , "\x09" , avgspeed , "\x09" , maxspeed);

	//call note(endtime-starttime:' Seconds')
	if (deletefile) {
		perform("DELETEFILE " ^ tempfilename ^ " (S)");
	}

	goto nexttest;

/////
exit:
/////
	if (deletefile) {
		perform("DELETEFILE " ^ tempfilename ^ " (S)");
	}

	//msg=''
	//msg<-1>='FILESPEED: Write then Delete'
	//msg<-1>='Recs: ':n:' '
	//msg<-1>='Size: ':recsize:' bytes'
	//msg<-1>='Tests: ':testn:' '
	//msg<-1>='|RESULTS:'
	//msg<-1>='Min/Avg/Max:':mintime:'/':avgtime:'/':maxtime:' secs'

	var msg = "";
	msg.r(-1, "FILESPEED Min/Avg/Max = " ^ minspeed ^ " / " ^ avgspeed ^ " / " ^ maxspeed);
	msg.r(-1, "More is faster.");
	msg.r(-1, "Typical speed for an entry level server in 2007 was 1");

	call mssg(msg.oconv("L#60"));
	return 0;

}

programexit()
