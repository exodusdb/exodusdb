#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <sysmsg.h>
#include <authorised.h>

var suppliercode;
var vehiclecode;
var coden;//num
var filen;//num
var code;
var ok;//num
var vehicle;
var taskid;
var msg0;
var positive;

function main(in marketcodex, in suppliercodex, in vehiclecodex, io vehicles, out msg) {

	//call giveway("");
	//mostly rewritten 2011/11/05 to make more thorough and include product category
	//(was) amazingly identical logic between validcode2 and validcode3!
	//except brand's client and market are in different field numbers
	//from the vehicle's supplier and market
	//company=market
	//client=supplier
	//category=jobtype
	//brand=vehicle

	//equ group2fn to

	var marketcode = marketcodex;
	if (suppliercodex.unassigned()) {
		suppliercode = "";
	}else{
		suppliercode = suppliercodex;
	}
	if (vehiclecodex.unassigned()) {
		vehiclecode = "";
	}else{
		vehiclecode = vehiclecodex;
	}
	if (vehicles.unassigned()) {
		vehicles = "";
	}

	//initialise buffer if not for the same user
	if (EW.a(10) ne USERNAME) {
		EW = EW.field(FM, 1, 9);
		EW.r(10, USERNAME);
		PW = PW.field(FM, 1, 9);
	}

	//return quick answer if checking vehicles and is in buffer
	if (vehiclecode) {
		if (EW.locate(vehiclecode, coden, 10 + 5)) {
			if (PW.a(10 + 5, coden)) {
				return 1;
			}else{
				return 0;
			}
		}
	}

	//check market access 1st (guessing it will reject fastest on average)
	//also maybe checked below after reading vehicle file
	var origmarketcode = marketcode;
	if (marketcode) {
		filen = 2;
		code = marketcode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	var supplier = "";
	var group1suppliercode = supplier.a(17);
	//group2suppliercode=supplier<group2fn>
	var jobtypecode = "";

	if (vehiclecode) {

		//get vehicle record - to check supplier, market, jobtype etc
		//if not(marketcode) or not(suppliercode) or not(jobtypecode) then

		if (vehicles == "") {
			if (not(vehicles.open("VEHICLES", ""))) {
				call fsmsg();
				var().stop();
			}
		}
		if (vehiclecode.index("~", 1)) {
			//assume version is in @record
			vehicle = RECORD;
		}else{
			var vehicle;
			if (not(vehicle.read(vehicles, vehiclecode))) {
				msg = DQ ^ (vehiclecode ^ DQ) ^ " missing from vehicles file";
				return 0;
			}
		}

		//get the latest info about vehicle (except for marketcode which maynot be vehiclewise)
		suppliercode = vehicle.a(3);
		//marketcode=vehicle<13>
		//group1suppliercode=vehicle<14>
		//group2suppliercode=vehicle<15>
		jobtypecode = vehicle.a(2);

		//check market access here if discovered from vehicle
		if (marketcode == "") {
			marketcode = vehicle.a(4);
		}

		goto getsupplier;
		// end

	} else if (suppliercode) {
getsupplier:
		var suppliers;
		if (not(suppliers.open("SUPPLIERS", ""))) {
			call fsmsg();
			var().stop();
		}
		if (not(supplier.read(suppliers, suppliercode))) {
			msg = DQ ^ (suppliercode ^ DQ) ^ " missing from suppliers file";
			return 0;
		}
		group1suppliercode = supplier.a(17);
		//group2suppliercode=supplier<group2fn>
		//marketcode=supplier<32>

		//get the market of the supplier if not specified directly
		if (marketcode == "") {
			marketcode = supplier.a(10);

			//following is to find company code but we dont do that for vehicles
			//same in suppliers and suppliers dict market_CODE and validcode2
			//acno=supplier<9>
			//if acno then
			// companycode=field(acno,',',2)
			// if companycode='' then
			// companycode=xlate('ACCOUNTS',acno<1,1,2>,5,'X')
			// end
			// end

		}

	}


	//check market access here if discovered from vehicle or supplier
	if (marketcode) {
		if (marketcode ne origmarketcode) {
			filen = 2;
			code = marketcode;
			gosub checkcode(msg);
			if (not ok) {
				return 0;
			}
		}
	}

	//dont need to check group1supplier/2 if identical to suppliercode, or 2=1 etc
	if (group1suppliercode == suppliercode) {
		group1suppliercode = "";
	}
	//if group2suppliercode=suppliercode then group2suppliercode=''
	//if group2suppliercode=group1suppliercode then group2suppliercode=''

	//check supplier groups first since, if specified, they are most likely to eliminate

	//supplier group1 access?
	if (group1suppliercode) {
		//if security('supplier ACCESS ':quote(group1suppliercode),msg,'') else return 0
		filen = 3;
		code = group1suppliercode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//supplier group2 access?
	//if group2suppliercode then
	// *if security('supplier ACCESS ':quote(group2suppliercode),msg,'') else return 0
	// filen=supplierfilen
	// code=group1suppliercode
	// gosub checkcode
	// if ok else return 0
	// end

	//check jobtype
	if (jobtypecode) {
		filen = 4;
		code = jobtypecode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check supplier
	if (suppliercode) {
		filen = 3;
		code = suppliercode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check vehicle - least likely to be restricted so leave until last
	//even though probably easiest to check
	if (vehiclecode) {
		filen = 5;
		taskid = "VEHICLE";
		positive = "";
		code = vehiclecode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	return 1;

}

function getfilepositive() {
	if (filen == 2) {
		taskid = "MARKET";
	} else if (filen == 3) {
		taskid = "SUPPLIER";
	} else if (filen == 4) {
		taskid = "MEDIA TYPE";
	} else {
		call sysmsg(DQ ^ (filen ^ DQ) ^ " filen is invalid in validcode2");
		var().stop();
	}

	//store space or '#' to indicate buffered result
	positive = PW.a(10, filen);
	if (positive == "") {
		if (authorised(taskid ^ " ACCESS", msg0, "")) {
			positive = " ";
		}else{
			positive = "#";
		}
		PW.r(10, filen, positive);
	}

	//trim any buffered space to become ''
	positive.trimmer();

	return 0;

}

subroutine checkcode(io msg) {
	ok = 0;
	//if dont have general access to file then
	//access to a specific record must be positively allowed (use # task prefix)
	if (EW.locate(code, coden, 10 + filen)) {
		if (not(PW.a(10 + filen, coden))) {
			return;
		}
	}else{

		//trim off first 10% of codes if buffer too big
		if (EW.length() > 50000) {
			var oldcoden = coden;
			coden = (coden * .9).floor();
			var ntrim = oldcoden - coden;
			EW.r(10 + filen, EW.a(10 + filen).field(VM, ntrim + 1, 999999));
			PW.r(10 + filen, PW.a(10 + filen).field(VM, ntrim + 1, 999999));
		}

		EW.r(10 + filen, coden, code);
		if (filen ne 5) {
			gosub getfilepositive();
		}
		if (not(authorised(positive ^ taskid ^ " ACCESS " ^ (DQ ^ (code ^ DQ)), msg, ""))) {
			return;
		}
		PW.r(10 + filen, coden, 1);
	}

	ok = 1;
	return;

}


libraryexit()
