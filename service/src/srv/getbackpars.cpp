#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <service_common.h>


// clang-format off
// Note: Defines are commented simply to avoid compiler warnings
//#define lastbakattemptdate_ bakpars.f(1)
//#define minbaktime_         bakpars.f(3)
//#define maxbaktime_         bakpars.f(4)
//#define bakdows_            bakpars.f(5)
//#define bakemails_          bakpars.f(6)
#define bakdisk_            bakpars.f(7)
#define bakexcludedatasets_ bakpars.f(8)
//#define bakdisable_         bakpars.f(9)
//#define bakdataemails_      bakpars.f(10)
#define baktestdata_        bakpars.f(11)
#define bakdisk2_           bakpars.f(12)
//#define bakmediarotation_   bakpars.f(13)
//#define logemail_           bakpars.f(14)

//backup params
//
// 1  = date last attempted
// 2  = not used - was workstation for backup (blank defaults to server)
// 3  = min time of day for backup to start (defaults to 01:00)
// 4  = max time of day for backup to start (defaults to 01:05)
// 5  = days to do backup 1234567 1=sun NIGHT!!! EG 2AM
// 2/3/4/5 could be multivalued in parallel (to be programmed)
// 6  = tech support email addresse(s) separated by ';'
// 7  = backup disk letter
// 8  = mv list of datasets NOT to backup (ctrl+E to edit)  (Not used if any backups defined in Configuration File)
// 9  = 1 disable both backup and shutdown
// 10 = email addresses to send backup.zip to OR replacement for 6 if 6 missing
//      backups@neosys.com is excluded in NET from backup.zip recipients
// 11 = 1 = suppress backup but do shutdown (testdata)  (Doesnt suppress if any backups defined in Configuration File)
// 12 = backup disk for uploads (images) folder if different from 7. Put 0 to suppress backup.
// 13 = '' is default (weekly rotation) 1=wrong media doesnt fail the backup
// max 30 since copied into system.cfg for editing
	// clang-format on

var process;
var tt;	 // num
var dbn;
//var xx;

func main(out bakpars, in process0 = var()) {

	// optonally get the backpars for a specific process if given
//	if (process0.unassigned()) {
//		process = SYSTEM;
//	} else {
//		process = process0;
//	}
	process = process0.or_default(SYSTEM);

	// by test data means any non-live data that doesnt require backup

	if (not bakpars.read(DEFINITIONS, "BACKUP")) {
		bakpars = "";
	}
	if (tt.osread("backup.cfg")) {
		for (const var ii : range(1, 99)) {
			if (tt.f(ii).len()) {
				bakpars(ii) = tt.f(ii);
			}
		}  // ii;
	}
	// osread tt from '..\..\backup.cfg' then
	var configfilename = "../../backup.cfg";
	configfilename.converter("/", OSSLASH);
	if (osfile(configfilename) and tt.osread(configfilename)) {
		for (const var ii : range(1, 99)) {
			if (tt.f(ii).len()) {
				bakpars(ii) = tt.f(ii);
			}
		}  // ii;
	}

//	if (VOLUMES) {
//		// if bakpars<3>='' then bakpars<3>='2:00'
//		// if bakpars<4>='' then bakpars<4>='2:05'
//		if (minbaktime_ == "") {
//			bakpars(3) = "1:00";
//		}
//		if (maxbaktime_ == "") {
//			bakpars(4) = "1:05";
//		}
//		if (not minbaktime_.isnum()) {
//			bakpars(3) = minbaktime_.iconv("MT");
//		}
//		if (not maxbaktime_.isnum()) {
//			bakpars(4) = maxbaktime_.iconv("MT");
//		}
//		if (bakdows_ == "") {
//			bakpars(5) = "1234567";
//		}
//	} else {
		// suppress on exodus
		bakpars(3) = -1;
		bakpars(4) = -1;
//	}

	// fix bug in data entry that allows : and :: to be entered
	if (bakdisk_.starts(":")) {
		bakpars(7) = "";
	}
	if (bakdisk2_.starts(":")) {
		bakpars(12) = "";
	}

//	if (not bakdisk_) {
//		bakpars(7) = "C:";
//	}
	if (bakdisk2_ == "") {
		bakpars(12) = bakdisk_;
	}

	// backup depending on configuration file
	// if any database listed there
	let dbcode	= process.f(17).lcase();
	let dbcodes = process.f(58).lcase();

//	if (dbcodes and VOLUMES) {
//
//		// databases to be excluded
//		bakpars(8) = "";
//
//		// decide backup required or not by indicating testdata
//		if (dbcodes.locate(dbcode, dbn)) {
//			tt = process.f(60, dbn);
//		} else {
//			tt = 0;
//		}
//		// test/nonlive data
//		bakpars(11) = not(tt);
//
//		// otherwise backup depending on backup.cfg etc
//	} else {

		// autodetermine if it is "test" data
		if (baktestdata_ == "") {
			var testdata = 1;
			if (process.f(17).ends("_test")) {
			} else if (process.f(23).ucase().contains("TRAINING")) {
			} else if (process.f(23).ucase().contains("TESTING")) {
			} else if (osfile("~/hosts/disabled.cfg")) {
			} else {
				// not otherwise specified then exclude database if in list of non-live data
				// none-live could include test data or consolidated copies
				if (not bakexcludedatasets_.locate(process.f(17))) {
					testdata = 0;
				}
			}
			bakpars(11) = testdata;
		}
//	}

	return 0;
}

}; // libraryexit()
