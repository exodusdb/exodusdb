#include <exodus/library.h>
libraryinit()

#include <log2.h>

#include <gen.h>

function main(in logtime, in reloading) {
	//jbase

	call log2("*INIT.AGENCY initialisation", logtime);
	var interactive = not SYSTEM.a(33);

	//y2k2

	call log2("*autostop inactive vehicles", logtime);
	var tt;
	if (not(tt.readv(gen._definitions, "INIT*INACTVEH", 1))) {
		tt = "";
	}
	if (tt < 16207) {
		var dom = (var().date()).oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTVEH MEDIA INACTIVEVEHICLES:365:S {} NEOSYS (ROS)";
		//ensure it will run sometime up to 28th, monthly
		if (dom > 28) {
			//run once today
			tt = cmd;
			tt.swapper("{}", "7:::::1");
			perform(tt);
			dom = 28;
		}
		//run on the scheduled day of the month
		tt = cmd;
		tt.swapper("{}", "7:" ^ dom);
		perform(tt);
		var().date().write(gen._definitions, "INIT*INACTVEH");
	}

	call log2("*autoclose inactive jobs", logtime);
	if (not(tt.readv(gen._definitions, "INIT*INACTJOB", 1))) {
		tt = "";
	}
	if (tt < 16230) {
		var dom = (var().date()).oconv("D/E").field("/", 1) + 0;
		var cmd = "JOB INACTIVEJOBS:365:C {} NEOSYS (ROS)";
		//ensure it will run sometime up to 28th, monthly
		if (dom > 28) {
			//run once today
			tt = cmd;
			tt.swapper("{}", "7:::::1");
			perform("CREATEALERT INACTJOB0 " ^ tt);
			dom = 28;
		}
		//run on the scheduled day of the month
		tt = cmd;
		tt.swapper("{}", "7:" ^ dom);
		perform("CREATEALERT INACTJOB " ^ tt);
		var().date().write(gen._definitions, "INIT*INACTJOB");
	}

	call log2("*run CREATEADS if required and before 7am and not test data", logtime);
	var nyears = agy.agp.a(125);
	if (nyears.length() and not SYSTEM.a(61) and var().time() < var("07:00").iconv("MT") and not interactive and not reloading) {

		if (gen._definitions.lock( "LOCK*CREATEADS")) {

			//nyearsback/agp<125> will be cleared if CREATEADS starts
			//hopefully there will be no other processes running
			//if there are any same db processes running then it will leave it for next time
			//writev '' on definitions,'AGENCY.PARAMS',125

			//note that CREATEADS might be aborted by GLOBAL.END file
			//and therefore dual scheduled starts on windows 2008
			//might cause CREATEADS to start twice
			//... but on the second run it should complete ok

			var cmd = "CREATEADS CREATE " ^ nyears ^ " (S)";
			perform(cmd);

			gen._definitions.unlock( "LOCK*CREATEADS");
		}else{

		}
	}

	return 0;

}


libraryexit()