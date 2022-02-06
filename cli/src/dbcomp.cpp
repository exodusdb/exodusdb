#include <exodus/program.h>
programinit()

function main() {

	var timestarted = time();

	var db1 = COMMAND.a(2);
	var db2 = COMMAND.a(3);

	var silent = OPTIONS.index("S");

	if (not silent)
		db1.logputl("dbcomp connecting to ");

	if (not(db1.connect())) {
		call fsmsg();
		stop(1);
	}

	if (not silent)
		db2.logputl("dbcomp connecting to ");

	if (not(db2.connect())) {
		call fsmsg();
		stop(1);
	}

	int nerrors = 0;

	var exclude_filenames="dict.all";
	var filenames = COMMAND.field(FM, 4, 9999);
	if (not filenames) {
		filenames = db1.listfiles();
		//TODO get from command line somehow to avoid hardcoding
		exclude_filenames ^= _VM_ "processes" _VM_ "locks" _VM_ "statistics" _VM_ "requestlog";
	}
	for (var filename : filenames) {

		if (not silent)
			outputl("\n", filename);

		if (filename.substr(1,10) eq "preselect_" or filename.substr(1,14) eq "select_stage2_" or locate(filename,exclude_filenames))
			continue;

		var file1;
		if (not file1.open(filename, db1)) {
			call fsmsg();
			stop(1);
		}

		var reccount = file1.reccount();

		var file2;
		if (not file2.open(filename, db2)) {
			call fsmsg();
			stop(1);
		}

		//var keys;
		//if (keys.osread("qwerty")) {
		//	keys.converter("\n", FM);
		//	file1.makelist("", keys);
		//} else
			file1.select("SELECT " ^ filename ^ " (SR)");

		var recn = 0;
		while (file1.readnext(RECORD, ID, MV)) {

			if (esctoexit())
				stop(1);

			recn++;
			if (not silent) {
				output(AT(-40), recn, "/", reccount, " ", ID);
				//printl("",RECORD.length());
				if ((recn % 1000) eq 1)
					osflush();
			}

			if (not RECORD) {
				if (not RECORD.read(file1, ID)) {
					errputl(ID, "missing from filename", filename);
					nerrors++;
					continue;
				}
			}

			var rec2;
			if (not read(rec2, file2, ID)) {
				errputl(ID, " missing from", db2.a(1), filename);
				nerrors++;
				continue;
			}

			if (rec2 ne RECORD and ID.substr(-5,5) != "_XREF") {

				/*
				//revert both records to leading zero 0.1, -0.1 instead of old pickos .1 and -.1;
				RECORD.replacer("([\x1A-\x1F]-?)0\\.", "$1.");
				rec2.replacer("([\x1A-\x1F]-?)0\\.", "$1.");

				//RECORD is db1 and rec2 is db2

				//temp suppress ads field 56
				if (filename eq "ads") {
					RECORD(56) = rec2.a(56);
					RECORD.swapper(".000", "");
					rec2.swapper(".000", "");
					RECORD.replacer("\\.00([\x1A-\x1F])", "$1");
					rec2.replacer("\\.00([\x1A-\x1F])", "$1");
				} else if (filename eq "brands") {
					RECORD(14) = rec2.a(14);
					RECORD(15) = rec2.a(15);
				}

				//if still different, output
				RECORD.cropper();
				rec2.cropper();
				*/
				if (rec2 ne RECORD) {
					nerrors++;
					//errputl(" ", ID, " different.");
					//printl(RECORD);
					//printl(rec2);
					var nfs = dcount(RECORD, FM);
					var nfs2 = dcount(rec2, FM);
					if (nfs2 gt nfs)
						nfs = nfs2;
					for (var fn = 1; fn <= nfs; ++fn) {
						var f1 = RECORD.a(fn);
						var f2 = rec2.a(fn);
						if (f1 ne f2) {
							errputl();
							errputl(filename, " ", ID, " ", fn, "-", f1);
							errputl(filename, " ", ID, " ", fn, "+", f2);
						}
					}
				}
				continue;
			}
		}
	}

	//call stopper("",timestarted,datestarted);

	if (not silent) {
		logputl();
		var seconds = time() - timestarted;
		while (seconds < 0)
			seconds += 86400;
		logputl("Finished in", int(seconds / 60), "minutes and", mod(seconds, 60), "seconds.");
	}

	return nerrors;

}

programexit()
