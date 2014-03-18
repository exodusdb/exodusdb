#include <exodus/library.h>
libraryinit()

#include <mkdir2.h>
#include <ciddos.h>


var tt;
var sysdate;//num
var systime;//num
var runfilex;
var datetime;
var mth;//num

function main() {
	//linemark cause compilation without line marks but dont actually have break

	var cmdmode = SENTENCE == "CID";
	if (not cmdmode) {
		tt = SYSTEM.a(111);
		if (tt ne "") {
			return tt;
		}
	}
	var sysgmtoffset = "";

	//make sure the directory is there
	if (not(var("BONDS\\DBASE.BND\\REVMEDIA.000").osfile())) {
		//pcperform 'md BONDS'
		//pcperform 'md ':'BONDS\DBASE.BND'
		//call mkdir('BONDS':char(0),ok)
		//call mkdir('BONDS\DBASE.BND':char(0),ok)
		call mkdir2("BONDS");
		call mkdir2("BONDS\\DBASE.BND");
		call oswrite("0", "BONDS\\DBASE.BND" "\\REVMEDIA.000");
		}
	if (not(("BONDS\\DBASE.BND" "\\REVMEDIA.000").osfile())) {
		call mssg("CANNOT RUN WITHOUT FULL ACCESS|RIGHTS IN NEOSYS DIRECTORY");
		var().logoff();
	}

	var runfile = "D" "I" "R" "U" "T" "C";
	//if dir(runfile:'.':'E':'X':'E') else
	// runfile=rnd(10000000)[-8,8]
	// end
	var cid = "";
	var gbp;
	if (gbp.open("GBP", "")) {
		//$si is a inverted console program designed tell the date of any folder/file
		//it returns the GMT/UTC time regardless of the system timezone
		//copying a folder usually changes its date therefore restricting copying
		//of neosys from one computer to another since the computer no is based
		//on a folder date
		var xx;
		if (xx.read(gbp, "$SI")) {

			//create the program
			call oswrite(xx.invert(), runfile ^ "." "E" "X" "E");

			//get the system time (default tz)
			while (true) {
				var systime1 = var().time();
				sysdate = var().date();
				systime = var().time();
			///BREAK;
			if (systime >= systime1) break;;
			}//loop;

			//get special folder time
			var cmd = runfile ^ "." "E" "X" "E" " BO" "NDS\\DBA" "SE.BND" ">" ^ runfile ^ ".D" "A" "T";
			cmd.suspend();

			//get current gmt/utc time
			runfilex = runfile ^ "." "X" "X" "X";
			var(var().date()).oswrite(runfilex);
			cmd = runfile ^ "." "E" "X" "E" " " ^ runfilex ^ ">" ^ runfile ^ ".D" "A" "2";
			cmd.suspend();

			//remove program
			//osdelete runfile:'.':'E':'X':'E'

			//work out special folder time
			if (datetime.osread(runfile ^ "." "D" "A" "T")) {
				datetime = datetime.substr(1, "\r");

				//convert to format 1999 12 31 24 60 60
				// YYYY MM DD HH MM SS
				datetime.converter(" :", "\xFE\xFE");
				var year = datetime.a(7);
				datetime.ucaser(;
				if (not(var("JAN" _VM_ "FEB" _VM_ "MAR" _VM_ "APR" _VM_ "MAY" _VM_ "JUN" _VM_ "JUL" _VM_ "AUG" _VM_ "SEP" _VM_ "OCT" _VM_ "NOV" _VM_ "DEC").a(1).locateusing(datetime.a(2), VM, mth))) {
					mth = 1;
				}
				mth = mth.oconv("R(0)#2");
				var day = datetime.a(3);
				var hour = datetime.a(4);
				hour = "00";
				//try to counteract the effect of changing time zone on the server
				//changing the UTC reported time

				//concept is as follows:
				//system time (stored on directory) ->UTC time (over net) ->local time
				//server stores system time in files and subtracts its time bias
				//in order to report UTC time to workstations
				//the workstations add their time bias to convert from UTC to their local
				//does this happen on NOVELL as well?

				var minute = datetime.a(5);
				var second = datetime.a(6);
				//time=year:' ':mth:' ':day:' ':hour:' ':minute:' ':second
				//secs first to cause maximum randomisation in hash function
				var time = second ^ " " ^ minute ^ " " ^ hour ^ " " ^ day ^ " " ^ mth ^ " " ^ year;
				var time2 = time;
				time2.converter(" ", "");
				if (time2.isnum()) {
					//call note(time)
					var hashkey = var(2).pwr(32) - 1;
					cid = ((time.hash(hashkey, 0)).str(6)).substr(-6, 6);
				}

			}
			(runfile ^ "." "D" "A" "T").osdelete();

			//work out timezone
			if (datetime.osread(runfile ^ "." "D" "A" "2")) {
				datetime = datetime.substr(1, "\r");
				//example
				//Fri Oct 28 15:39:19 2011

				//convert to format 1999 12 31 24 60 60
				// YYYY MM DD HH MM SS
				datetime.converter(" :", "\xFE\xFE");
				var year = datetime.a(7);
				datetime.ucaser(;
				if (not(var("JAN" _VM_ "FEB" _VM_ "MAR" _VM_ "APR" _VM_ "MAY" _VM_ "JUN" _VM_ "JUL" _VM_ "AUG" _VM_ "SEP" _VM_ "OCT" _VM_ "NOV" _VM_ "DEC").a(1).locateusing(datetime.a(2), VM, mth))) {
					mth = 1;
				}
				mth = mth.oconv("R(0)#2");
				var day = datetime.a(3);
				var gmtdate = (day ^ "/" ^ datetime.a(2) ^ "/" ^ year).iconv("D");

				var hour = datetime.a(4);
				var minute = datetime.a(5);
				var second = datetime.a(6);
				var gmttime = (hour ^ ":" ^ minute ^ ":" ^ second).iconv("MTH");

				var dateoffset = sysdate - gmtdate;
				//timeoffset=systime-gmttime-3 ;*add and subtract 3 seconds to check
				//timeoffset=systime-gmttime+3 ;*if successfully rounding to nearest min
				var timeoffset = systime - gmttime;
				sysgmtoffset = dateoffset * 24 * 60 * 60 + timeoffset;

				//get to the nearest minute
				var mod60 = sysgmtoffset % 60;
				if (mod60 < 30) {
					sysgmtoffset -= mod60;
				}else{
					sysgmtoffset += 60 - mod60;
				}

			}

			(runfile ^ "." "D" "A" "2").osdelete();
		}
		runfilex.osdelete();
	}

	if (cid == "") {
		cid = ciddos();
	}

	SYSTEM.r(111, cid);
	SYSTEM.r(120, sysgmtoffset);

	if (cmdmode) {
		if (sysgmtoffset) {
			tt = sysgmtoffset / 60;
			if (tt >= 0) {
				tt = " +" ^ tt;
			}else{
				tt = " " ^ tt;
			}
			tt ^= " mins";
			//tt:=' (':sysgmtoffset:' secs) ':mod60
		}else{
			tt = "";
		}
		call note("Computer number is " ^ (DQ ^ (cid ^ DQ)) ^ FM ^ "Server TZ is GMT/UTC" ^ tt);
	}else{
		return cid;
	}

}


libraryexit()