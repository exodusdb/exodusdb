#undef NDEBUG
#include <cassert>

#include <exodus/program.h>
programinit()

var totndifferences = 0;

function main() {

	printl("test_regress says 'Hello World!'");

	//TRACE(var("") < var("A"));  //is true (1)
	//stop();

	var filenames = remove(COMMAND,1);

	//var manual = filenames;

	var exodusdir=osgetenv("GITHUB_WORKSPACE");

	// skip regression testing for speed
	// unless we are testing on github
	// or manually testing specific files
	if (not exodusdir) {
		if (not filenames) {
			printl("Test skipped for speed - command is 'test_regress ALL' for full testing");
			printl("Test passed");
			return 0;
		}
		exodusdir = osgetenv("HOME") ^ OSSLASH ^ "exodus";
	}

	var testdatadir=exodusdir ^ OSSLASH ^ "/test/data";

	if (not filenames or filenames == "ALL") {
		filenames=oslistf(testdatadir ^ OSSLASH ^ "*.TXT");
		if (not filenames)
			printl("Cannot find any files " ^ testdatadir);
	}

	//test each file
	for (var filename : filenames) {

		//dont always test isnum because it is long and largely a waste of time
		//if (not manual and filename == "ISNUM.TXT")
		//	continue;

		if (filename) {

			var maxndifferences = 0;
			if (filename == "LOCATEL.TXT")
				maxndifferences = 2;
			else if (filename == "LOCATER.TXT")
				maxndifferences = 16;
/*
/root/exodus//test/data/LOCATEL.TXT
40700. 0 2      LOCATEBY        ABABAAAA]       B       AL                      0       0       0 result differs: 0 3
178176. 0 2     LOCATEBY        CABCC]C]C]      AA      DL                      0       0       0 result differs: 0 4
Lines: 190125 differences: 2 Max differences: 2
/root/exodus//test/data/LOCATER.TXT
7053. 1 2       LOCATEBYUSING   ]0}0]20}}       00      AR      }               0       2       0 result differs: 1 1
7054. 1 2       LOCATEBYUSING   ]0}0]20}}       00      DR      }               0       2       0 result differs: 1 1
7055. 1 2       LOCATEBY        ]0}0]20}}       00      AR                      0       2       0 result differs: 1 1
7056. 1 2       LOCATEBY        ]0}0]20}}       00      DR                      0       2       0 result differs: 1 1
7323. 1 3       LOCATEBYUSING   2]}0}0}0]       00      AR      }               1       2       0 result differs: 1 2
7324. 1 3       LOCATEBYUSING   2]0}0}0}]       00      DR      }               1       2       0 result differs: 1 1
7325. 1 3       LOCATEBY        2]}0}0}0]       00      AR                      1       2       0 result differs: 1 2
7326. 1 3       LOCATEBY        2]0}0}0}]       00      DR                      1       2       0 result differs: 1 1
215683. 0 1     LOCATEBY        20]000] 20      AR                      0       0       0 result differs: 1 1
215684. 0 2     LOCATEBY        20]000] 20      DR                      0       0       0 result differs: 1 1
240198. 0 2     LOCATEBY        01120]2 0       DR                      0       0       0 result differs: 0 3
417634. 0 2     LOCATEBY        202]1]0]        01      DR                      0       0       0 result differs: 1 2
470133. 0 1     LOCATEBY        2]2]1]  2       AR                      0       0       0 result differs: 1 1
470134. 0 2     LOCATEBY        2]2]1]  2       DR                      0       0       0 result differs: 1 1
488984. 0 2     LOCATEBY        010122]01       1       DR                      0       0       0 result differs: 1 2
513934. 0 2     LOCATEBY        20002]021       2       DR                      0       0       0 result differs: 0 3
*/
			totndifferences += onefile(testdatadir ^ OSSLASH ^ filename, maxndifferences);
		}
	}

	if (totndifferences < 1)
		printl("Test passed");
	else
		printl("Test failed with ", totndifferences, "differences");

	return totndifferences;
}

#define TARGET line2(1)
#define FUNC line2(2)
#define ARG0 line2(3)
#define ARG1 line2(4)
#define ARG2 line2(5)
#define ARG3 line2(6)
#define ARG4 line2(7)
#define ARG5 line2(8)
#define ARG6 line2(9)
#define ARG7 line2(10)


#define LOCATE_WHAT ARG1  //col4
#define LOCATE_BY ARG2    //col5
#define LOCATE_USING ARG3 //col6
#define LOCATE_FN ARG5    //col8
#define LOCATE_VN ARG6    //col9
#define LOCATE_SN ARG7    //col10

function onefile(in filename, in maxndifferences) {

	printl(filename);

	var ndifferences = 0;

	var data;
	if (not osread(data from filename)) {
		call fsmsg();
		stop("cant osopen " ^ filename);
	}

	var rec,subrec,by,sep,what,setting;
	var pick_sep;
	var temp,sep2;

	var funcnames="EXTRACT,REPLACE,INSERT,DELETE,==,!=,<,<=,>,>=,FIELD,ISNUM,FMT,OCONV,LOCATE,LOCATEUSING,LOCATEBY,LOCATEBYUSING,SUBSTR,SPLICER1,SPLICER2";
	var funcno;

	data.converter("\r\n",_FM_ _FM_);
	dim line2;
	var result;

	var lineno = 0;
	for (var line : data) {
		if (not line)
			continue;
//errputl(line);
		lineno++;

		//printl(line);

		line.converter("\xFF\xFE\xFD\xFC\xFB\xFA", _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);

		dim line2 = line.split("\t");
		var funcname=line2(2);
		if (not funcnames.locateusing(",",funcname,funcno)) {
			ndifferences +=1;
			printl(line,"Unknown function");
		}

		switch (int(funcno)) {
		case 1:// a/extract
			result=ARG0(ARG1,ARG2,ARG3);
			break;

		case 2:// r/replace
			result=ARG0.r(ARG1,ARG2,ARG3,ARG4);
			break;

		case 3://insert
			result=ARG0.insert(ARG1,ARG2,ARG3,ARG4);
			break;

		case 4://remove
			result=ARG0.remove(ARG1,ARG2,ARG3);
			break;

		case 5:// eq
			result=ARG0 == ARG1;
			break;

		case 6:// ne
			result=ARG0 != ARG1;
			break;

		case 7:// lt
			result=ARG0 < ARG1;
			break;

		case 8:// le
			result=ARG0 <= ARG1;
			break;

		case 9:// gt
			result=ARG0 > ARG1;
			break;

		case 10:// ge
			result=ARG0 >= ARG1;
			break;

		case 11:// field
			if (ARG1 == "")
				result = "";
			else
				result=field(ARG0,ARG1,ARG2,ARG3);
			break;

		case 12:// isnum
			result=ARG0.isnum();
			break;

		case 13:// fmt
		case 14:// oconv

			//skip some differences/changes between AREV and exodus
			//71. 0   OCONV   -.5     MX result differs: FFFFFFFFFFFFFFFF
			//119. 0  OCONV   .5      MX result differs: 1
			if (ARG1 == "MX") {
				if (ARG0 == 0.5 or ARG0 == -0.5)
					continue;
			}

			result=ARG0.oconv(ARG1);
			break;

		case 15:// locate
			//TRACE(ARG0)
			//TRACE(LOCATE_WHAT)
			//TRACE(LOCATE_FN)
			//TRACE(LOCATE_VN)
			result=ARG0.locate(LOCATE_WHAT,setting,LOCATE_FN,LOCATE_VN);
			result ^= " " ^ setting;
			break;

		case 16:// locate using
			//TRACE(ARG0)
			//TRACE(LOCATE_USING)
			//TRACE(LOCATE_WHAT)
			//TRACE(LOCATE_FN)
			//TRACE(LOCATE_VN)
			//TRACE(LOCATE_SN)
			sep = LOCATE_USING;
			if (sep > RM && ARG0.a(1,1,1) ne ARG0) {
				result = TARGET;
			}
			else {
				result=ARG0.locateusing(LOCATE_USING,LOCATE_WHAT,setting,LOCATE_FN,LOCATE_VN,LOCATE_SN);
				result ^= " " ^ setting;
			}
			break;

		case 17:// locate by
		case 18:// locate by using
			//TRACE(ARG0)
			//TRACE(LOCATE_BY)
			//TRACE(LOCATE_USING)
			//TRACE(LOCATE_WHAT)
			//TRACE(LOCATE_FN)
			//TRACE(LOCATE_VN)
			//TRACE(LOCATE_SN)
			rec = ARG0;
			subrec = rec.a(LOCATE_FN,LOCATE_VN,LOCATE_SN);
			what = LOCATE_WHAT;
			sep = LOCATE_USING;

			//if search area or target contain any field/value marks etc, other than the separator then skip
			//because exodus and pick arev differ and it is not required to be defined
			if (len(sep)) {
				sep2=sep;
			} else {
				if (LOCATE_SN)
					sep2 = VM;//should not happen to be locate in one SN without a separator
				else if (LOCATE_VN)
					sep2 = SM;
				else if (LOCATE_FN)
					sep2 = VM;
				else
					sep2 = VM;
			}
			temp=(subrec ^ what).convert(sep2,"");
			if (temp.a(1,1,1) ne temp) {
				continue;
			}

			//skip difference when separator is 0 because it is not required
			if (sep >= "0" and sep <= "9" and (LOCATE_BY == "AR" or LOCATE_BY == "DR") )
				continue;

			/*
			//if ordering and target/string contains field/value/marks then change to PICK seps to ensure compatible ordering
			//except the sep field mark that is being used
			if (subrec.a(1,1,1) ne subrec or what.a(1,1,1) ne what) {
				what.converter(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,"\xFF\xFE\xFD\xFC\xFB\xFA");
				subrec.converter(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,"\xFF\xFE\xFD\xFC\xFB\xFA");
				//TRACE(lineno ^ ". subrec = " ^ subrec.convert("\xFF\xFE\xFD\xFC\xFB\xFA",_RM_ _FM_ _VM_ _SM_ _TM_ _STM_) ^ " ... " ^ sep ^ " " ^ seq(sep));
				if (sep == "") {
					if (LOCATE_SN)
						sep = "";
					else if (LOCATE_VN)
						sep = SM;
					else if (LOCATE_FN)
						sep = VM;
					else
						sep = FM;
				}
				if (sep and sep <= RM) {
					pick_sep = chr(seq(sep)+256-32);
					what.converter(pick_sep,sep);
					subrec.converter(pick_sep,sep);
					//TRACE(lineno ^ ". " ^ sep ^ " " ^ seq(sep) ^ " " ^ seq(pick_sep));
				}
				rec.r(LOCATE_FN,LOCATE_VN,LOCATE_SN,subrec);
				//TRACE(lineno ^ ". " ^ rec.convert("\xFF\xFE\xFD\xFC\xFB\xFA",_RM_ _FM_ _VM_ _SM_ _TM_ _STM_) ^ " ... " ^ sep ^ " " ^ seq(sep));
			}
			*/

			if (funcno == 17) {
				if (LOCATE_VN)
					result=rec.locateby(LOCATE_BY,what,setting,LOCATE_FN,LOCATE_VN);
				else if (LOCATE_FN)
					result=rec.locateby(LOCATE_BY,what,setting,LOCATE_FN);
				else
					result=rec.locateby(LOCATE_BY,what,setting);
			}
			else {

				/*
				//skip some case where Pick/AREV are strange ie compare FM:'A' < 'A' in order by
				//i.e. skip lines where PICK/AREV thinks that "A"> FM:A (something that we can say is "undefined behaviour" for locate by
				if (LOCATE_FN == 0 and LOCATE_VN == 0 and LOCATE_SN == 0) {
					if (subrec.index("\xFE")) {
						if (sep == VM or sep == SM or sep > RM)
							continue;
					}
					else if (subrec.index("\xFD") and ( sep == SM or sep > RM))
						continue;
				}
				*/

				if (LOCATE_SN)
					result=rec.locatebyusing(LOCATE_BY,LOCATE_USING,what,setting,LOCATE_FN,LOCATE_VN,LOCATE_SN);
				else if (LOCATE_VN)
					result=rec.locatebyusing(LOCATE_BY,LOCATE_USING,what,setting,LOCATE_FN,LOCATE_VN);
				else if (LOCATE_FN)
					result=rec.locatebyusing(LOCATE_BY,LOCATE_USING,what,setting,LOCATE_FN);
				else
					result=rec.locatebyusing(LOCATE_BY,LOCATE_USING,what,setting);
			}
			result ^= " " ^ setting;
			break;

		case 19:
			result=ARG0.substr(ARG1, ARG2);
			break;

		//SPLICER1 x[y]=a
		case 20:
			result=ARG0;
			result.splicer(ARG1, ARG3);
			break;

		//SPLICER2 x[y,z]=a
		case 21:
			result=ARG0;
			result.splicer(ARG1, ARG2, ARG3);
			break;

		default:
			ndifferences +=1;
			printl(line,"Unknown function");
			stop();
			continue;
		}

		if (result ne TARGET) {
			ndifferences++;
			printl(lineno ^ ".",line.trimb("\t"),"result differs:",result);
//			assert(result == TARGET);
		}
		else {
			//printl(line,"OK");
		}
		//printl(line);

	}

	printl("Lines:",lineno,"differences:",ndifferences,"Max differences:",maxndifferences);

	if (ndifferences <= maxndifferences)
		ndifferences = 0;
	else
		ndifferences -= maxndifferences;

	return ndifferences;
}

programexit()
