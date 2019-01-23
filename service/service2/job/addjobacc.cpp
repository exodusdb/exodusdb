#include <exodus/library.h>
libraryinit()

#include <addacc.h>

#include <gen.h>
#include <agy.h>

var xx;

function main(in addaccmode, in jobno, io msg) {
	//c job in,in,io
	//adds a new job account number given a job number
	//returns a message if the wip ledger is nearly too full or too full
	// to add a new account

	if (not(FILES(0).locateusing("ABP", FM, xx))) {
		return 0;
	}
	if (not(FILES(0).locateusing("ACCOUNTS", FM, xx))) {
		return 0;
	}

	msg = "";
	var ledgercode = agy.agp.a(26);
	if (not ledgercode) {
		return 0;
	}

	var jobx;
	if (not(jobx.read(agy.jobs, jobno))) {
		msg = DQ ^ (jobno ^ DQ) ^ " - JOB IS MISSING";
		return 0;
	}

	//get the brand
	var brand;
	if (not(brand.read(agy.brands, jobx.a(2)))) {
		msg = "JOB " ^ (DQ ^ (jobno ^ DQ)) ^ " - BRAND " ^ (DQ ^ (jobx.a(2) ^ DQ)) ^ " IS MISSING";
		return 0;
	}

	var period = jobx.a(1);

	//virtually identical in addjobacc plan.subs5 printplans6
	//params=field(period,'/',2):fm:field(period,'/',1) 'R(0)#2':fm:int((field(period,'/',1)-1)/3)+1
	//year 99
	var params = period.field("/", 2);
	//period 01-12
	params.r(2, period.field("/", 1).oconv("R(0)#2"));
	//quarter 1-4
	params.r(3, ((period.field("/", 1) - 1) / 3).floor() + 1);
	//company code
	//params<4>=gcurr.company
	params.r(4, jobx.a(14));

	var accno = jobno;
	if ((accno[1]).isnum()) {
		accno.splicer(1, 0, "JOB");
	}

	//accname='Job ':jobno:' ':trim(brand<1>:' ':jobx<2>:' ':period:' ':jobx<3>)
	//accname=trim(brand<1>:' ':jobx<2>:' ':period:' ':jobx<3>:' Job ':jobno)
	//client and brand code only now
	var accname = (brand.a(1) ^ " " ^ jobx.a(2) ^ " " ^ period ^ " " ^ jobx.a(3)).trim();

	var sortorder = "NAME";

	call addacc(addaccmode, ledgercode, accno, accname, sortorder, params, msg);

	return 0;

}


libraryexit()
