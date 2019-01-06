#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <trim2.h>
#include <addcent.h>
#include <updanalysis2.h>
#include <singular.h>
#include <authorised.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var basefmtz;
var tpseudo;
var validate;//num
var analysis2;
var budgetcode;
var fullyear;
var budgetcompanycode;
var rown;//num
var brandcode;
var vehiclecode;
var suppliercode;
var typecode;
var marketcode;
var oldkeys;
var newkeys;
var msg;
var oldkeyn;//num
var wsmsg;

function main(in mode) {
	//c agy

	//global rown,clientcode,brandcode,vehiclecode,suppliercode,typecode,marketcode
	//global basefmtz,oldkeys,newkeys

	basefmtz = USER2.substr(1,4) ^ "PZ";

	if (mode == "GETBUDGET") {

		PSEUDO.transfer(tpseudo);
		gosub getbudget();
		tpseudo.transfer(PSEUDO);

	} else if (mode == "ANALYSIS2") {
		PSEUDO.transfer(tpseudo);
		validate = 1;
		win.valid = 1;
		gosub savebudget();
		if (win.valid) {
			validate = 0;
			gosub savebudget();
		}
		tpseudo.transfer(PSEUDO);

	}
//L257:
	return 0;

}

subroutine getbudget() {

	if (not(openfile("DICT.ANALYSIS", DICT))) {
		var().stop();
	}
	if (not(openfile("ANALYSIS2", analysis2))) {
		var().stop();
	}
	if (not analysis2) {
		var().stop();
	}

	gosub getyearbudgetcompany();

	var reqmedia = tpseudo.a(4) ne "2";
	var reqnonmedia = tpseudo.a(4) ne "1";

	//cut off all data except parameters
	tpseudo = tpseudo.field(FM, 1, 9);

	var cmd = "SELECT ANALYSIS2 USING ANALYSIS";
	cmd ^= " WITH AUTHORISED";
	cmd ^= " AND WITH CATEGORY0 " ^ (DQ ^ (budgetcode ^ DQ));
	cmd ^= " AND WITH FULLYEAR " ^ (DQ ^ (fullyear ^ DQ));
	cmd ^= " AND WITH COMPANY_CODE " ^ (DQ ^ (budgetcompanycode ^ DQ));

	cmd ^= " BY CLIENT_NAME BY BRAND_NAME";
	cmd ^= " BY MEDIAORBTL";
	cmd ^= " BY TYPE_NAME";
	cmd ^= " BY MARKET_NAME";

	if (reqmedia) {
		cmd ^= " BY VEHICLE_NAME";
	}else{
		cmd ^= " AND WITH SUPPLIER_CODE0";
	}

	if (reqnonmedia) {
		cmd ^= " BY SUPPLIER_NAME";
	}else{
		cmd ^= " AND WITH VEHICLE_CODE";
	}

	var().clearselect();
	perform(cmd ^ " (S)");

	if (not LISTACTIVE) {
		call mssg("No records can be found");
		return;
	}

	rown = 0;

	////////
nextrow:
	////////
	if (not readnext(ID)) {
		ID = "";
	}

	if (ID == "" or esctoexit()) {
		gosub getbudgetexit();
		return;
	}

	if (ID[1] == "%") {
		goto nextrow;
	}

	if (not(RECORD.read(analysis2, ID))) {
		goto nextrow;
	}

	brandcode = calculate("BRAND_CODE");
	if (not brandcode) {
		goto nextrow;
	}

	vehiclecode = ID.field("*", 3);
	suppliercode = ID.field("*", 5);
	typecode = ID.field("*", 6);
	marketcode = ID.field("*", 4);

	//skip blank records
	//temp=tpseudo
	//temp<9>=''
	//convert fm:vm:sm:'0.' to '' in temp
	//if temp else goto nextrow

	////////
	//rowinit:
	////////
	rown += 1;

	tpseudo.r(10, rown, brandcode);
	tpseudo.r(11, rown, vehiclecode);
	tpseudo.r(12, rown, suppliercode);
	tpseudo.r(13, rown, typecode);
	tpseudo.r(14, rown, marketcode);
	tpseudo.r(19, rown, calculate("MARGIN"));

	tpseudo.r(20, rown, RECORD.a(21));
	tpseudo.r(40, rown, RECORD.a(22));
	for (var periodn = 1; periodn <= 12; ++periodn) {
		tpseudo.r(20 + periodn, rown, RECORD.a(1, periodn));
		tpseudo.r(40 + periodn, rown, RECORD.a(2, periodn));
	};//periodn;

	//store a list of keys so that deleted rows can be deleted on saving
	tpseudo.r(18, -1, ID);

	////////
	//rowexit:
	////////
	goto nextrow;

}

subroutine getbudgetexit() {
	return;

}

subroutine savebudget() {

	if (not(openfile("ANALYSIS2", analysis2))) {
		win.valid = 0;
		return;
	}

	rown = 0;
	var nrows = (tpseudo.a(10)).count(VM) + 1;

	gosub getyearbudgetcompany();

	var changedyearbudgetcompany = 1;
	var firstkey = trim2(tpseudo.a(18), VM).a(1, 1);
	if (fullyear ne addcent(firstkey.field("*", 1))) {
	} else if (budgetcompanycode ne firstkey.field("*", 8)) {
	} else if (budgetcode ne firstkey.field("*", 7)) {
	} else {
		changedyearbudgetcompany = 0;
		}
//L1000:
	oldkeys = tpseudo.a(18);
	if (changedyearbudgetcompany) {
		oldkeys = "";
	}

	newkeys = "";

	/////////
nextrec2:
	/////////
	rown += 1;
	if (rown > nrows) {
		gosub savebudgetexit();
		return;
	}

	//get codes
	brandcode = tpseudo.a(10, rown);
	if (not brandcode) {
		goto nextrec2;
	}

	vehiclecode = tpseudo.a(11, rown);
	if (vehiclecode) {
		suppliercode = "";
		typecode = "";
		marketcode = "";
	}else{
		suppliercode = tpseudo.a(12, rown);
		typecode = tpseudo.a(13, rown);
		marketcode = tpseudo.a(14, rown);
	}

	var margin = tpseudo.a(19, rown);

	//validate codes

	if (validate) {

		if (not margin.isnum()) {
			msg = DQ ^ (margin ^ DQ) ^ " margin should be numeric but is non-numeric";
			gosub invalid(msg);
		}

		var brand;
		if (not(brand.read(agy.brands, brandcode))) {
			msg = DQ ^ (brandcode ^ DQ) ^ " - invalid brand code";
			gosub invalid(msg);
		}

		if (vehiclecode) {

			var vehicle;
			if (not(vehicle.read(agy.vehicles, vehiclecode))) {
				msg = DQ ^ (vehiclecode ^ DQ) ^ " - invalid vehicle code";
				gosub invalid(msg);
			}

		}else{

			var supplier;
			if (not(supplier.read(agy.suppliers, suppliercode))) {
				msg = DQ ^ (suppliercode ^ DQ) ^ " - invalid supplier code";
				gosub invalid(msg);
			}

			var market;
			if (not(market.read(agy.markets, marketcode))) {
				msg = DQ ^ (marketcode ^ DQ) ^ " - invalid market code";
				gosub invalid(msg);
			}

			var type;
			if (not(type.read(agy.jobtypes, typecode))) {
				msg = DQ ^ (typecode ^ DQ) ^ " - invalid type code";
				gosub invalid(msg);
			}

		}

	}

	ID = fullyear.substr(-2,2) ^ "*" ^ brandcode ^ "*" ^ vehiclecode ^ "*" ^ marketcode ^ "*" ^ suppliercode ^ "*" ^ typecode ^ "*" ^ budgetcode ^ "*" ^ budgetcompanycode;

	RECORD = "";
	RECORD.r(25, margin);

	for (var periodn = 1; periodn <= 12; ++periodn) {

		var bill = tpseudo.a(20 + periodn, rown);
		if (not bill.isnum()) {
			msg = DQ ^ (bill ^ DQ) ^ " should be numeric but is non-numeric. Period : " ^ periodn;
			gosub invalid(msg);
		}
		if (bill.index(".", 1)) {
			bill = bill.oconv(basefmtz);
		}
		RECORD.r(1, periodn, bill);

		var cost = tpseudo.a(40 + periodn, rown);
		if (cost) {
			if (not cost.isnum()) {
				msg = DQ ^ (cost ^ DQ) ^ " should be numeric but is non-numeric. Period : " ^ periodn;
				gosub invalid(msg);
			}
			if (cost.index(".", 1)) {
				cost = cost.oconv(basefmtz);
			}
			RECORD.r(2, periodn, cost);
		}

	};//periodn;

	/////////
	//recinit2:
	/////////

	//recalc totals
	RECORD.r(21, ((RECORD.a(1)).sum()).oconv(basefmtz));
	RECORD.r(22, ((RECORD.a(2)).sum()).oconv(basefmtz));

	//update files
	if (not validate) {
		call cropper(RECORD);

		if (not(win.orec.read(analysis2, ID))) {
			win.orec = "";
		}
		for (var periodn = 1; periodn <= 12; ++periodn) {
			if (win.orec.a(2, periodn) == "0" and RECORD.a(2, periodn) == "") {
				RECORD.r(2, periodn, "0");
			}
		};//periodn;
		RECORD.r(9, win.orec.a(9));
		call cropper(win.orec);

		if (RECORD ne win.orec) {
			//if 1 then

			//write or delete
			var tt = RECORD;
			tt.r(9, "");
			tt.converter(FM, "");
			if (tt) {
				RECORD.write(analysis2, ID);
				RECORD.write(agy.analysis, ID);
			}else{
				analysis2.deleterecord(ID);
				
				agy.analysis.deleterecord(ID);
				
			}

			call updanalysis2("", ID, RECORD, win.orec);

		}

		if (oldkeys.a(1).locateusing(ID, VM, oldkeyn)) {
			oldkeys.eraser(1, oldkeyn);
		}

	}

	newkeys.r(1, -1, ID);

	/////////
	//recexit2:
	/////////
	goto nextrec2;

}

subroutine savebudgetexit() {

	//delete and deleted rows
	if (not validate) {
		var noldkeys = oldkeys.count(VM) + 1;
		for (var oldkeyn = 1; oldkeyn <= noldkeys; ++oldkeyn) {
			var id = oldkeys.a(1, oldkeyn);
			if (win.orec.read(analysis2, id)) {
				//mode=''
				//record=''
				//call upd.analysis2(mode,id,record,orec)
				call updanalysis2("", id, "", win.orec);
				agy.analysis.deleterecord(id);
				
				analysis2.deleterecord(id);
				
			}
		};//oldkeyn;
	}

	if (not validate) {
		tpseudo.r(18, newkeys);
	}

	return;

}

subroutine getyearbudgetcompany() {

	fullyear = addcent(tpseudo.a(1));

	budgetcompanycode = tpseudo.a(2);

	budgetcode = tpseudo.a(3);
	var budgetcode2 = budgetcode.field(" ", 2);
	budgetcode = budgetcode.field(" ", 1)[1];
	if (budgetcode2 ne 1) {
		budgetcode ^= budgetcode2;
	}

	return;

}


libraryexit()