#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <sysmsg.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>

var typecode;

function main(io logtime, in reloading="") {
	//c agy io,""

	call log2("*INIT.AGENCY initialisation", logtime);
	var interactive = not SYSTEM.a(33);

	//y2k2

	call log2("*add tags/subtypes to digital/magazine media types", logtime);
	var tt;
	if (not(tt.readv(DEFINITIONS, "JOB_TYPES*UPDTAGSTYPES", 1))) {
		tt = "";
	}
	if (tt < 18064) {

		var oldtags = "";
		var newtags = "";

		var oldtypes = "";
		var newtypes = "";

		//digital
		var mtn = 1;

		//digital tags
		newtags.r(mtn, -1, "Market");
		newtags.r(mtn, -1, "Language");
		newtags.r(mtn, -1, "Device");
		newtags.r(mtn, -1, "Format");
		newtags.r(mtn, -1, "Targeting");
		newtags.r(mtn, -1, "Audience");
		newtags.r(mtn, -1, "Placement");
		newtags.r(mtn, -1, "Duration");

		//digital subtypes
		newtypes.r(mtn, -1, "Adserving");
		newtypes.r(mtn, -1, "App");
		newtypes.r(mtn, -1, "Content");
		newtypes.r(mtn, -1, "Creative");
		newtypes.r(mtn, -1, "Display");
		newtypes.r(mtn, -1, "EDM");
		newtypes.r(mtn, -1, "Mobile");
		newtypes.r(mtn, -1, "Programmatic");
		newtypes.r(mtn, -1, "Social Media");
		newtypes.r(mtn, -1, "SMS Mobile");
		newtypes.r(mtn, -1, "SEO");
		newtypes.r(mtn, -1, "SEM");
		newtypes.r(mtn, -1, "Video");
		newtypes.r(mtn, -1, "Fee");

		//magazines
		mtn = 2;

		//magazine subtypes
		newtypes.r(mtn, -1, "Weekly");
		newtypes.r(mtn, -1, "Bi-Weekly");
		newtypes.r(mtn, -1, "Fortnightly");
		newtypes.r(mtn, -1, "Monthly");
		newtypes.r(mtn, -1, "Bi-Monthly");
		newtypes.r(mtn, -1, "Quarterly");
		newtypes.r(mtn, -1, "Annual");
		newtypes.r(mtn, -1, "Directory");
		newtypes.r(mtn, -1, "Guide");

		var().clearselect();
		agy.jobtypes.select();
nextjobtype:
		if (readnext(typecode)) {

			var rec;
			if (not(rec.read(agy.jobtypes, typecode))) {
				goto nextjobtype;
			}

			//media types only
			if (rec.a(3)) {
				goto nextjobtype;
			}

			//determine interesting media types
			tt = rec.a(1).ucase();
			if (tt.index("DIGITAL", 1) or tt.index("INTERNET", 1)) {
				mtn = 1;
			} else if (tt.index("MAGAZINE", 1)) {
				mtn = 2;
			} else {
				goto nextjobtype;
			}
//L735:
			var orec = rec;

			//tags
			if (rec.a(37) == oldtags.a(mtn)) {
				rec.r(37, newtags.a(mtn));
			}

			//subtypes
			if (rec.a(38) == oldtypes.a(mtn)) {
				rec.r(38, newtypes.a(mtn));
			}

			if (rec ne orec) {
				rec.write(agy.jobtypes, typecode);
				//on live systems
				if (not SYSTEM.a(61)) {
					call sysmsg("Updated tags/subtypes on media type " ^ rec.a(1) ^ " (" ^ typecode ^ ")");
				}
			}

			goto nextjobtype;
		}

		var().date().write(DEFINITIONS, "JOB_TYPES*UPDTAGSTYPES");
	}

	call log2("*autostop inactive job executives", logtime);
	if (not(tt.readv(DEFINITIONS, "INIT*INACTJOBEXEC", 1))) {
		tt = "";
	}
	if (tt < 16968) {
		var dom = var().date().oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTJOBEXEC_ONCE JOB INACTIVEJOBEXECS:365:S {} NEOSYS (ROS)";
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
		tt.swapper("_ONCE", "");
		tt.swapper("{}", "7:" ^ dom);
		perform(tt);
		var().date().write(DEFINITIONS, "INIT*INACTJOBEXEC");
	}

	call log2("*autostop inactive media executives", logtime);
	if (not(tt.readv(DEFINITIONS, "INIT*INACTMEDIAEXEC", 1))) {
		tt = "";
	}
	if (tt < 16968) {
		var dom = var().date().oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTMEDIAEXEC_ONCE MEDIA INACTIVEMEDIAEXECS:365:S {} NEOSYS (ROS)";
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
		tt.swapper("_ONCE", "");
		perform(tt);
		var().date().write(DEFINITIONS, "INIT*INACTMEDIAEXEC");
	}

	call log2("*autostop inactive vehicles", logtime);
	if (not(tt.readv(DEFINITIONS, "INIT*INACTVEH", 1))) {
		tt = "";
	}
	if (tt < 16207) {
		var dom = var().date().oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTVEH_ONCE MEDIA INACTIVEVEHICLES:365:S {} NEOSYS (ROS)";
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
		tt.swapper("_ONCE", "");
		tt.swapper("{}", "7:" ^ dom);
		perform(tt);
		var().date().write(DEFINITIONS, "INIT*INACTVEH");
	}

	call log2("*autoclose inactive jobs", logtime);
	if (not(tt.readv(DEFINITIONS, "INIT*INACTJOB", 1))) {
		tt = "";
	}
	if (tt < 16230) {
		var dom = var().date().oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTJOB_ONCE JOB INACTIVEJOBS:365:C {} NEOSYS (ROS)";
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
		tt.swapper("_ONCE", "");
		perform(tt);
		var().date().write(DEFINITIONS, "INIT*INACTJOB");
	}

	call log2("*run CREATEADS if required and before 7am and not test data", logtime);
	var nyears = agy.agp.a(125);
	if ((((nyears.length() and not SYSTEM.a(61)) and (var().time() < var("07:00").iconv("MT"))) and not interactive) and not reloading) {

		if (DEFINITIONS.lock( "LOCK*CREATEADS")) {

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

			DEFINITIONS.unlock( "LOCK*CREATEADS");
		}else{

		}
	}

	return 0;

}


libraryexit()
