#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <authorised.h>
#include <sysmsg.h>

var suppliercode;
var vehiclecode;
var coden;//num
var filen;//num
var code;
var ok;//num
var vehicle;
var taskid;
var positive;
var xx;

function main(in marketcodex, in suppliercodex, in vehiclecodex, io vehicles, io msg) {
	//c agy in,in,in,io,io

	//jbase
	call giveway();
	//mostly rewritten 2011/11/05 to make more thorough and include product category
	//(was) amazingly identical logic between validcode2 and validcode3!
	//except brand's client and market are in different field numbers
	//from the vehicle's supplier and market
	//company=market
	//client=supplier
	//category=jobtype
	//brand=vehicle

	#define codebuff EW
	#define authbuff PW

	#define group1fn 17
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

	#define positivesfn 10
	#define marketfilen 2
	#define supplierfilen 3
	#define jobtypefilen 4
	#define vehiclefilen 5

	//initialise buffer if not for the same user
	if (EW.a(positivesfn) ne USERNAME) {
		EW = EW.field(FM, 1, 9);
		EW.r(10, USERNAME);
		PW = PW.field(FM, 1, 9);
	}

	//return quick answer if checking vehicles and is in buffer
	if (vehiclecode) {
		if ((EW.a(positivesfn + vehiclefilen)).locate(vehiclecode,coden)) {
			if (PW.a(positivesfn + vehiclefilen, coden)) {
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
		filen = marketfilen;
		code = marketcode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	var supplier = "";
	var group1suppliercode = supplier.a(group1fn);
	//group2suppliercode=supplier<group2fn>
	var jobtypecode = "";

	if (vehiclecode) {

		//get vehicle record - to check supplier, market, jobtype etc
		//if not(marketcode) or not(suppliercode) or not(jobtypecode) then

		if (vehicles == "") {
			if (not(vehicles.open("VEHICLES", ""))) {
				call fsmsg();
				stop();
			}
		}

		if (not(vehicle.reado(vehicles, vehiclecode))) {

				//allow for validating a DELETED vehicle from VEHICLE_VERSIONS
			var versionfile;
			if (not(versionfile.open("VEHICLE_VERSIONS", ""))) {
missingvehicle:
				msg = vehiclecode.quote() ^ " missing from vehicles file";
				return 0;
			}
			if (not(vehicle.read(versionfile, vehiclecode))) {
				goto missingvehicle;
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
			stop();
		}
		if (not(supplier.read(suppliers, suppliercode))) {
			msg = suppliercode.quote() ^ " missing from suppliers file";
			return 0;
		}
		group1suppliercode = supplier.a(group1fn);
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
			//  companycode=xlate('ACCOUNTS',acno<1,1,2>,5,'X')
			//  end
			// end

		}

	}

	//check market access here if discovered from vehicle or supplier
	if (marketcode) {
		if (marketcode ne origmarketcode) {
			filen = marketfilen;
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
		//if security('supplier ACCESS ':quote(group1suppliercode),msg) else return 0
		filen = supplierfilen;
		code = group1suppliercode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//supplier group2 access?
	//if group2suppliercode then
	// *if security('supplier ACCESS ':quote(group2suppliercode),msg) else return 0
	// filen=supplierfilen
	// code=group1suppliercode
	// gosub checkcode
	// if ok else return 0
	// end

	//check jobtype
	if (jobtypecode) {
		filen = jobtypefilen;
		code = jobtypecode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check supplier
	if (suppliercode) {
		filen = supplierfilen;
		code = suppliercode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check vehicle - least likely to be restricted so leave until last
	//even though probably easiest to check
	if (vehiclecode) {
		filen = vehiclefilen;
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

subroutine checkcode(io msg) {

	ok = 0;
	//if dont have general access to file then
	//access to a specific record must be positively allowed (use # task prefix)
	if ((EW.a(positivesfn + filen)).locate(code,coden)) {
		if (not(PW.a(positivesfn + filen, coden))) {
			return;
		}
	}else{

		//trim off first 10% of codes if buffer too big
		if (VOLUMES) {
			//equ maxbuffsize to maxstrsize-15530
			#define maxbuffsize 50000
			if (EW.length() > maxbuffsize) {
				var oldcoden = coden;
				coden = (coden * .9).floor();
				var ntrim = oldcoden - coden;
				EW.r(positivesfn + filen, (EW.a(positivesfn + filen)).field(VM, ntrim + 1, 999999));
				PW.r(positivesfn + filen, (PW.a(positivesfn + filen)).field(VM, ntrim + 1, 999999));
			}
		}

		EW.r(positivesfn + filen, coden, code);
		if (filen ne vehiclefilen) {
			gosub getfilepositive();
		}
		if (not(authorised(positive ^ taskid ^ " ACCESS " ^ (code.quote()), msg))) {
			return;
		}
		PW.r(positivesfn + filen, coden, 1);
	}

	ok = 1;
	return;
}

subroutine getfilepositive() {
	if (filen == marketfilen) {
		taskid = "MARKET";
	} else if (filen == supplierfilen) {
		taskid = "SUPPLIER";
	} else if (filen == jobtypefilen) {
		taskid = "MEDIA TYPE";
	} else {
		call sysmsg(filen.quote() ^ " filen is invalid in validcode2");
		stop();
	}

	//store space or '#' to indicate buffered result
	positive = PW.a(positivesfn, filen);
	if (positive == "") {
		if (authorised(taskid ^ " ACCESS", xx)) {
			positive = " ";
		}else{
			positive = "#";
		}
		PW.r(positivesfn, filen, positive);
	}

	//trim any buffered space to become ''
	positive.trimmer();

	return;
}

libraryexit()
