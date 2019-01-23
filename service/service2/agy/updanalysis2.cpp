#include <exodus/library.h>
libraryinit()

#include <locking.h>
#include <log.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>

var balkey;
var xx;
var locklist;
var msg;
var balances;
var origbalances;

function main(in mode0, in id, in analrecord, in orec) {
	//c agy

	if (mode0.unassigned()) {
		var mode = "";
	}else{
		var mode = mode0;
	}

	var mode = "ONE";

	//skip if no company code
	var compcode = id.field("*", 8);
	if (compcode == "") {
		compcode = agy.agp.a(39);
	}
	if (not compcode) {
		return 0;
	}

	var year = id.field("*", 1);

	//only do budget and forecast records
	var categorycode = id.field("*", 7);
	if (categorycode == "") {
		return 0;
	}
	if (not(var("BF").index(categorycode[1]))) {
		return 0;
	}

	var fn = 1;
	gosub updbalances( mode,  analrecord, orec,  id,  fn,  year,  compcode,  categorycode);

	fn = 2;
	gosub updbalances( mode,  analrecord, orec,  id,  fn,  year,  compcode,  categorycode);

	return 0;

}

subroutine updbalances(in mode, in analrecord,in orec, in id, in fn, in year, in compcode, in categorycode) {
	//updbalances(in mode, in analrecord,in orec, in id, in fn, in year, in compcode, in categorycode)
	var accno = "";

	var amounts = analrecord.a(fn);
	var oldamounts = orec.a(fn);

	var controlaccno = "";
	gosub updbalances2( fn,  year,  compcode, categorycode, oldamounts, accno, mode,  id, amounts);
	return;

}

subroutine updbalances2(in fn, in year, in compcode,in categorycode,in oldamounts,io accno,in mode, in id,in amounts) {
	//updbalances2(in fn, in year, in compcode,in categorycode,in oldamounts,io accno,in mode, in id,in amounts)
	for (var periodn = 1; periodn <= fin.maxperiod; ++periodn) {
		var amount = amounts.a(1, periodn) - oldamounts.a(1, periodn);

		//billing is negative cost is positive
		//both go in the DR field as budget are only in field 1
		if (fn == 1) {
			amount = -amount;
		}

		if (amount) {

			if (accno == "") {

				//not doing control account anymore 2019
				//if controlaccno then
				// accno=controlaccno
				//end else
				gosub getacc( mode,  id,  fn,  accno);
				if (not accno) {
					return;
				}
				// end

				balkey = year ^ "*" ^ accno ^ "*" ^ compcode ^ "*" ^ fin.basecurrency ^ "*" ^ categorycode;

				if (not(locking("LOCK", "BALANCES", balkey, xx, locklist, 999, msg))) {
					call log("UPD.ANALYSIS2", msg);
				}

				var balances;
				if (not(balances.read(fin.balances, balkey))) {
					balances = "";
				}
				origbalances = balances;

			}

			var basefmtz = USER2.substr(1,4) ^ "PZ";
			balances.r(1, (balances.a(1) +++ amount).oconv(basefmtz));
			balances.r(3, periodn, (balances.a(3, periodn) +++ amount).oconv(basefmtz));

		}

	};//periodn;

	if (accno) {

		if (balances ne origbalances) {
			balances.write(fin.balances, balkey);

			//not doing control account any more 2019
			//if there is a control account then go back and do the control account as well
			//if controlaccno='' then
			// readv controlaccno from accounts,accno,6 else controlaccno=''
			// if controlaccno then
			// call locking('UNLOCK','BALANCES',balkey,xx,locklist,0,xx)
			// accno=''
			// goto updbalances2
			// end
			// end

		}

		call locking("UNLOCK", "BALANCES", balkey, xx, locklist, 0, xx);

	}

	return;

}

subroutine getacc(in mode, in id, in fn, io accno) {
	//getacc(in mode, in id, in fn, io accno)
	//get activity code
	var typecode = id.field("*", 6);
	if (typecode == "") {
		var vehiclecode = id.field("*", 3);
		typecode = vehiclecode.xlate("VEHICLES", 2, "X");
		if ((typecode == "") and (mode == "ONE")) {
			call note(DQ ^ (ID ^ DQ) ^ " - invalid vehicle in upd.analysis2");
		}
	}

	//get activity record
	var activity;
	if (not(activity.read(agy.jobtypes, typecode))) {
		if (mode == "ONE") {
			call note(DQ ^ (typecode ^ DQ) ^ " - invalid activity code in upd.analysis2");
		}
		return;
	}

	//get account number from activity or master activity
	accno = activity.a(4 + fn, 1, 2);
	if (not accno) {

		//get master activity record
		var masteractivity = "";
		var mastertypecode = activity.a(10);
		if (mastertypecode) {
			if (not(masteractivity.read(agy.jobtypes, mastertypecode))) {
				if (mode == "ONE") {
					call note(DQ ^ (mastertypecode ^ DQ) ^ " - invalid master activity code in upd.analysis2");
				}
			}
		}

		//skip if account not specified
		accno = masteractivity.a(4 + fn, 1, 2);
		if (not accno) {
			if (mode == "ONE") {
				call note(id ^ "|" ^ (DQ ^ (typecode ^ DQ)) ^ " - activity account is missing in upd.analysis2");
			}
			return;
		}

	}

	return;

}


libraryexit()
