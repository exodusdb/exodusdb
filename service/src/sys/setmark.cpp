#include <exodus/library.h>
libraryinit()

#include <readagp.h>

#include <sys_common.h>

function main() {
	#include <system_common.h>
	//c sys

	var compcode = SENTENCE.field(" ", 2);
	if (not(compcode)) {
		var msg = "Syntax is SETMARK companycode \"xxxxxxxxxxxxxx\"";
		msg.r(-1, "or to remove, just SETMARK companycode");
		msg.r(-1, "Company Code * for default mark for all companies");
		call mssg(msg);
		stop();
	}

	var mark = SENTENCE.field(" ", 3, 9999);
	if ((mark[1] eq DQ) and (mark[-1] eq DQ)) {
		mark.splicer(1, 1, "");
		mark.splicer(-1, 1, "");
		//trim leading and trailing spaces
		mark = mark.trimb().trimf();
	}

	if (VOLUMES) {
		mark = mark.invert();
	}

	if (compcode eq "*") {
		if (not mark) {
			call mssg("Default mark cannot be removed");
			stop();
		}
		var rec;
		if (not(rec.read(DEFINITIONS, "AGENCY.PARAMS"))) {
			call mssg("DEFINITIONS, AGENCY.PARAMS is missing");
			stop();
		}
		rec.r(1, mark);
		rec.write(DEFINITIONS, "AGENCY.PARAMS");

		call readagp();

	} else {

		var tcompany;
		if (not(tcompany.read(sys.companies, compcode))) {
			call mssg(compcode.quote() ^ " company code does not exist");
			stop();
		}

		tcompany.r(27, mark);

		tcompany.write(sys.companies, compcode);
	}

	perform("SORT COMPANIES COMPANY_NAME MARK");

	return 0;
}

libraryexit()
