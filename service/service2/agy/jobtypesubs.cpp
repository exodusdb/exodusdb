#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <authorised.h>
#include <generalsubs2.h>
#include <validcode2.h>
#include <safeselect.h>
#include <sysmsg.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var tt;
var xx;
var msg;
var v69;
var v70;
var v71;
var billcostaccs;
var typecode;
var type;
var mediaorjob;//num
var analcoln;//num
var fn;//num
var acno;
var vn;
var analfn;//num
var wsmsg;

function main(in mode) {
	//c agy
	//global acno,fn,analcoln,billcostaccs,mediaorjob

	//jbase
	var op0 = "MEDIA/PRODUCTION ";
	var op = op0 ^ "TYPE";

	if (mode == "UPDATEBILLCOSTACCS") {
		gosub updateacclist();

	} else if (mode == "F2.GROUP") {

		if (RECORD.a(3) == "N") {
			tt = "PRODUCTION";
		}else{
			tt = "MEDIA";
		}
		call agencysubs("F2." ^ tt ^ ".TYPE.NOGROUP", xx);

	} else if (mode == "VAL.GROUP") {

		if (win.is and win.is == win.isorig) {
			return 0;
		}

		//check if allowed to create top level items
		if (win.is == "") {
			if (not(authorised(op0 ^ "TYPE CREATE GROUPS", msg, ""))) {
				msg = "YOU MUST SPECIFY THE GROUP CODE||" ^ msg;
				return invalid(msg);
			}
		}else{
			if (RECORD.a(3) == "N") {
				tt = "PRODUCTION";
			}else{
				tt = "MEDIA";
			}
			call agencysubs("VAL." ^ tt ^ ".TYPE.NOGROUP", xx);
		}

	} else if (mode == "POSTINIT") {
		gosub security2(mode, op);
		if (not win.valid) {
			return 0;
		}

	} else if (mode == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
		}

		//this should be recoded to not fail if specific code access granted
		gosub security2(mode, op);
		if (not win.valid) {
			return 0;
		}

		if (authorised(op0 ^ "TYPE ACCESS " ^ (DQ ^ (ID ^ DQ)), msg)) {

			if (not(authorised(op0 ^ "TYPE ACCESS " ^ (DQ ^ (calculate("GROUP") ^ DQ)), msg))) {
				goto unlocktype;
			}

		}else{
unlocktype:
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			//msg='SORRY YOU ARE NOT AUTHORISED|TO ACCESS TYPE ':quote(typecode)
			gosub invalid(msg);
			win.reset = 5;
			return 0;
		}

		//check authorised company
		var compcode = calculate("COMPANY_CODE");
		if (compcode) {
			if (not(validcode2(compcode, "", "", xx, msg))) {
				goto unlocktype;
			}
		}

		gosub outconv();
		if (win.valid) {
			win.orec = RECORD;
		}

	} else if (mode == "OCONV") {
		gosub outconv();

	} else if (mode == "PREWRITE") {

		//check allowed to create groups
		if (RECORD.a(10) == "") {
			if (not(authorised(op0 ^ "TYPE CREATE GROUPS", msg, ""))) {
				msg = "YOU MUST SPECIFY THE GROUP CODE||" ^ msg;
				return invalid(msg);
			}
		}

		gosub inconv();

		//update version log
		call generalsubs2(mode);

	} else if (mode == "ICONV") {
		gosub inconv();

	} else if (mode == "POSTWRITE") {
		gosub updateacclist();
		gosub outconv();

	} else if (mode == "POSTDELETE") {
		gosub updateacclist();

	} else if (mode == "PREDELETE") {
		gosub security2(mode, op);
		if (not win.valid) {
			return 0;
		}

		call safeselect("SELECT JOB_TYPES WITH GROUP " ^ (DQ ^ (ID ^ DQ)));
		if (readnext(tt)) {
			msg = DQ ^ (ID ^ DQ) ^ " cannot be deleted while it is used as a group code on type " ^ (DQ ^ (tt ^ DQ));
			return invalid(msg);
		}
		var().clearselect();

		call safeselect("SELECT VEHICLES WITH MEDIA_TYPE_CODE " ^ (DQ ^ (ID ^ DQ)));
		if (readnext(tt)) {
			msg = DQ ^ (ID ^ DQ) ^ " cannot be deleted while it is used on vehicle " ^ (DQ ^ (tt ^ DQ));
			return invalid(msg);
		}
		var().clearselect();

		//update version log
		call generalsubs2(mode);

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - unknown mode skipped in MEDIATYPE.SUBS";
		return invalid(msg);

	}
//L881:
	return 0;

}

subroutine outconv() {
	//convert @record from db to UI format/user interface codes

	if (not RECORD) {
		return;
	}

	if (RECORD.a(3) == "") {
		RECORD.r(3, "M");
	} else if (RECORD.a(3) == "Y") {
		RECORD.r(3, "N");
	} else {
		msg = DQ ^ (RECORD.a(3) ^ DQ) ^ " is not valid for media/nonmedia";
		gosub invalid(msg);
		return;
	}
//L984:
	return;

}

subroutine inconv() {
	//convert @record from UI format/user interface codes to db codes

	var f3 = RECORD.a(3);
	if (f3 == "N") {
		RECORD.r(3, "Y");
	} else if (f3 == "M") {
		RECORD.r(3, "");
	} else if (f3 == "Y") {
	} else if (f3 == "") {
	} else {
		msg = DQ ^ (RECORD.a(3) ^ DQ) ^ " is not valid for media/nonmedia";
		gosub invalid(msg);
		return;
	}
//L1102:
	return;

}

subroutine updateacclist() {

	//completely regenerate list whenever any type is written or deleted
	//and when the item is missing from definitions
	call pushselect(0, v69, v70, v71);

	call safeselect("SELECT JOB_TYPES (S)");

	billcostaccs = var().date();
nexttype:
	if (readnext(typecode)) {
		if (not type.read(agy.jobtypes, typecode)) {
			goto nexttype;
		}

		//uncomment the following to _not_ require analysis code on non-op types
		//for journals to non-operational type a/cs (various petty expenses)
		//BUT this risks losing the lock Billing Analysis = P&L
		//if people change type field 27 flag after using the type on docs?
		//even neosys support staff will probably screw this up
		//and how to prevent a job using both op types AND non-op types?
		//if type<27> then goto nexttype

		if (type.a(3) == "Y") {
			mediaorjob = "2";
		}else{
			mediaorjob = "1";
		}

		analcoln = 1;
		fn = 5;
		gosub addacc();
		fn = 6;
		gosub addacc();
		//exclude tax accounts so posting doesnt require analysis code
		//since billing analysis generally excludes tax it will match P&L inc/cost
		//fn=13;gosub addacc
		//fn=14;gosub addacc

		//AVR income/cost
		analcoln = 9;
		fn = 35;
		gosub addacc();
		fn = 36;
		gosub addacc();

		goto nexttype;
	}

	call popselect(0, v69, v70, v71);

	//now add for wip/accrual control accounts so postings to their
	//subaccounts are also analysed from 2012/12/10 version of NEOSYS
	//the problem with these is that they cannot be reconciled per media type
	//since they allow all media types
	//same as if using one a/c for many media types
	//TODO move them to media/job types files
	//this is rarely used by clients FAROUK
	//to transfer income from one company to another using intercompany a/cs
	//depending on a share% registered in the Company File

	typecode = "";
	analcoln = "";

	//media

	mediaorjob = 1;

	//media accrual accounts
	acno = agy.agp.a(135);
	fn = 7;
	gosub addacc2();

	//media wip accounts
	acno = agy.agp.a(137);
	fn = 8;
	gosub addacc2();

	//media income sharing accounts (rarely if ever used 11/2015)
	acno = agy.agp.a(94);
	fn = 5;
	gosub addacc2();

	//jobs

	mediaorjob = "2";

	//prodn accrual accounts
	acno = agy.agp.a(136);
	fn = 7;
	gosub addacc2();

	//prodn wip accounts
	acno = agy.agp.a(138);
	fn = 8;
	gosub addacc2();

	//job income sharing accounts (not used 11/2015)
	acno = agy.agp.a(94);
	fn = 7;
	gosub addacc2();

	billcostaccs.r(6, 16416);
	billcostaccs.write(DEFINITIONS, "BILLCOSTACCS");

	return;

}

subroutine addacc() {
	acno = type.a(fn);
	gosub addacc2();
	return;

}

subroutine addacc2() {
	if (acno == "") {
		return;
	}
	if (acno.a(1, 1, 2)) {
		acno = acno.a(1, 1, 2);
	}else{
		acno = acno.a(1, 1, 1).field(",", 1);
	}

	gosub addacc3();

	//adline lebanon ADLINEL allow posting to discount allowed per media type
	//in parallel to sales accounts specified on media types
	if (acno.match("\"7010\"5N")) {

		//credit notes
		acno.splicer(1, 4, "7093");
		gosub addacc3();

		//discounts
		acno.splicer(1, 4, "7094");
		gosub addacc3();

		//zero rated invoices
		acno.splicer(1, 4, "7011");
		gosub addacc3();

		//could add more variations here
		//acno[1,x]='xxxx'
		//gosub addacc3
	}

	return;

}

subroutine addacc3() {
	if (not billcostaccs.a(2).locateusing(acno, VM, vn)) {
		billcostaccs.r(2, vn, acno);
	}

	//fn 35/36 are AVR bill cost stored in analysis 28/29 (like bill/cost)
	//note postings into rebate received INCOME A/c reverses billing analysis COST!
	if (fn == 35) {
		analfn = 29;

	//note posting into rebate paid COST A/c reverses billing analysis INCOME
	} else if (fn == 36) {
		analfn = 28;

	//fn 5/6 are bill cost stored in analysis 28/29
	//fn 7/8 are accrued cost/work in progress stored in analysis 30/31?
	} else if (fn >= 5 and fn <= 8) {
		analfn = 23 + fn;

	} else {
		call sysmsg(DQ ^ (fn ^ DQ) ^ " invalid fn in jobtype.subs");
		return;
	}
//L1700:
	if (not billcostaccs.a(3, vn).locateusing(analfn, SVM, xx)) {
		billcostaccs.r(3, vn, -1, analfn);
	}
	if (typecode) {
		if (not billcostaccs.a(4, vn).locateusing(typecode, SVM, xx)) {
			billcostaccs.r(4, vn, -1, typecode);
		}
	}
	if (not billcostaccs.a(5, vn).locateusing(mediaorjob, SVM, xx)) {
		billcostaccs.r(5, vn, -1, mediaorjob);
	}

	if (analcoln) {
		if (not billcostaccs.a(7, vn).locateusing(analcoln, SVM, xx)) {
			billcostaccs.r(7, vn, -1, analcoln);
		}
	}

	return;

}


libraryexit()
