#include <exodus/library.h>
libraryinit()

#include <agy.h>
#include <gen.h>
#include <win.h>

var xx;
var suppliername;
var vn2;
var vn;
var fn;//num
var ln2;
var perc;
var lang;

#include <window.hpp>//after win

function main(in modex) {
	//jbase
	//garbagecollect;
	//**$INSERT GBP,AREV.COMMON*** without this can be called from any program

	var interactive = not SYSTEM.a(33);
	var mode=modex;

	if (mode.substr(1, 8) == "SHOWTOTS" or (mode.field(",", 1)).index("GETFREE", 1)) {

		//pass through schedule adding up paid and free spots by vehicle
		var array = "";
		//array<1> is vehicle (first word of name only) + medium + disc/charges
		//array<2> is paid spots
		//array<3> is paid amount
		//array<4> is free spots
		//array<5> is free amount
		//array<6> is vehicle type 'TV Radio etc
		//array<7> is suppliernames
		var nlines = (RECORD.a(20)).count(VM) + 1;

		//option to value paid ads at gross or net (usually gross)
		var netonly = mode.substr(1, 3) == "NET";
		if (netonly) {
			mode.splicer(1, 3, "");
		}

		//11/7/97
		var reqvehicles = mode.field(",", 2);
		var nreqvehicles = reqvehicles.count(VM) + (reqvehicles ne "");
		for (var lnx = 1; lnx <= nlines; ++lnx) {
			var vehiclecode = RECORD.a(20, lnx);
			var exchrate = RECORD.a(40, lnx);
			if (not exchrate) {
				exchrate = 1;
			}
			//11/7/97
			if (reqvehicles) {
				if (not(reqvehicles.locate(vehiclecode, xx, 1))) {
					vehiclecode = "";
				}
			}

			if (vehiclecode) {
				var number = RECORD.a(39, lnx);
				var extras = mv.capitalise(field2(RECORD.a(140, lnx), ",", -1));
				if (number) {
					var vehicle;
					if (not(vehicle.read(agy.vehicles, vehiclecode))) {
						vehicle = vehiclecode;
					}

					//option to consolidate free ads by supplier in cyprus
					if (agy.agp.a(2) == "CYP") {
						//readv suppliername from suppliers,vehicle<3>,1 else suppliername=vehicle<1>
						//suppliername:=' ':vehicle<2>
						suppliername = vehicle.a(1).field(" ", 1);
					}else{
						suppliername = vehicle.a(1);
					}
					var medianame;
					if (not(medianame.readv(agy.jobtypes, vehicle.a(2), 1))) {
						medianame = "";
					}

					//add a line for paid ads
					//locate suppliername in array<7> setting vn2 else
					// array<7,-1>=suppliername
					// end
					if (not(array.locate(suppliername, vn2, 7))) {
						array.inserter(7, vn2, suppliername);
						array.inserter(2, vn2, "");
						array.inserter(3, vn2, "");
					}

					//net unit bill
					var unitbill = RECORD.a(38, lnx);

					//add a line for free ads
					suppliername ^= "|" ^ medianame;
					if (not unitbill) {
						suppliername ^= "|" ^ extras;
					}
					//locate suppliername in array<1> setting vn else
					// array<1,vn>=suppliername
					// array<6,vn>=vehicle<2>;*media type code
					// end
					if (not(array.locateby(suppliername, "AL", vn, 1))) {
						array.inserter(1, vn, suppliername);
						array.inserter(4, vn, "");
						array.inserter(5, vn, "");
						array.inserter(6, vn, vehicle.a(2));
					}

					//gross unit bill
					var gunitbill = RECORD.a(37, lnx);
					//multiply by size if any
					var size2 = RECORD.a(21, lnx).field("*", 2).field(",", 1);
					//if size matches '1N0N"X"1N0N' or size matches '0N"."0N"X"1N0N' or size matches '1N0N"X"0N"."0N' then
					// garbagecollect
					// gunitbill=OCONV(gunitbill*field(size,'X',1)*field(size,'X',2),FMTX)
					// end
					if (size2) {
						size2.converter("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "                          ");
						size2.trimmer();
						var sizea = size2.field(" ", 1);
						var sizeb = size2.field(" ", 2);
						if (sizea and sizeb and sizea.isnum() and sizeb.isnum()) {
							//garbagecollect;
							gunitbill = (gunitbill * sizea * sizeb).oconv("MD" ^ RECORD.a(1) ^ "0P");
						}
					}

					var freenumber = (RECORD.a(71, lnx)).sum();
					number -= freenumber;

addup:
					var unitvalue = "";
					if (unitbill and gunitbill) {
						//paid ads are valued at gross unless only showing net bills to client
						if (netonly) {
							unitvalue = unitbill;
						}else{
							unitvalue = gunitbill;
						}
						fn = 2;
						//990611 different extras are now analysed separately
						//case extras='OFFER' or extras='PACKAGE'
						//do nothing

					} else if (not unitbill and gunitbill) {
						//if no unit bill then use the gross bill as the value of free spots
						unitvalue = gunitbill;
						fn = 4;
						vn2 = vn;

					} else if (unitbill and not gunitbill) {
						//if net bill but no gross bill then
						// consider as OFFER not to be included in the paid or free spots
						//do nothing

					} else  {
						//if no gross or net bill then do nothing
					}

					//add up
					if (unitvalue) {

						//call note(fn:' ':vn2:' ':unitvalue)
						array.r(fn, vn2, array.a(fn, vn2) + number);
						//garbagecollect;
						array.r(fn + 1, vn2, (array.a(fn + 1, vn2) + number * unitvalue / exchrate).oconv("MD" ^ RECORD.a(1) ^ "0P"));

						if (freenumber) {
							number = freenumber;
							freenumber = 0;
							if (unitbill) {
								gunitbill = unitbill;
							}
							unitbill = "";
							goto addup;
						}

					}

				}
			}
		};//lnx;
		nlines = (array.a(1)).count(VM) + 1;

		//either return the free ads text for schedule/plan printing program
		//11/7/97
		//if mode='GETFREE' then
		if (mode.field(",", 1) == "GETFREE") {
			var html = 1;
			ANS = "";

			//free percentage
			//only for "cost to client", *assume that it is in "cost to agency"
			//mode if the net unit bills are the same as the net unit costs
			var anyperc = RECORD.a(38) ne RECORD.a(42);
			if (agy.agp.a(2) == "CYP") {
				anyperc = 1;
			}
			anyperc = 1;

			for (var lnx = 1; lnx <= nlines; ++lnx) {

				suppliername = array.a(1, lnx).field("|", 1);
				if (not(array.locate(suppliername.field("|", 1), ln2, 7))) {
					{}
				}

				//if any free ad value
				if (array.a(5, lnx)) {

					if (not html) {
						suppliername = suppliername.oconv("L#15") ^ " ";
					}

					var extras = array.a(1, lnx).field("|", 3);
					if (extras == "") {
						extras = "Free";
					}

					var medium = array.a(1, lnx).field("|", 2);
					if (medium == "Television") {
						medium = "TV";
					}
					if (not html) {
						medium = medium.oconv("L#15") ^ " ";
					}

					var nfree = array.a(4, lnx);
					if (not html) {
						nfree = nfree.oconv("R#5");
					}

					var freevalue = (array.a(5, lnx)).oconv(USER2.substr(1, 2) ^ "20P,");
					if (not html) {
						freevalue = freevalue.oconv("R#12");
					}

					var wording = " " ^ extras ^ " ad";
					if (array.a(4, lnx) == 1)
						wording^=" ";
					else
						wording^="s";
					wording = wording.oconv("L#15");
					wording ^= " = " ^ freevalue;

					if (anyperc) {
						if (array.a(3, ln2)) {
							perc = (100 * array.a(5, lnx) / array.a(3, ln2)).oconv("MD00P") ^ "%";
						}else{
							if (html) {
								perc = "No paid";
							}else{
								perc = "All";
							}
						}
					}else{
						perc = "";
					}

					//either build one line in html
					if (html) {
						//swap '=' with '=':vm in wording
						ANS ^= FM;
						if (nreqvehicles ne 1) {
							ANS ^= suppliername ^ VM ^ medium ^ VM;
						}
						ANS ^= extras ^ VM;
						ANS ^= nfree ^ VM ^ freevalue;
						if (anyperc) {
							ANS ^= VM ^ perc;
						}

						//or build one line in text
					}else{
						ANS ^= FM;

						//supplier name
						ANS ^= suppliername;

						//medium
						ANS ^= medium;

						//number of free ads
						ANS ^= nfree;

						//wording and value of free ads
						ANS ^= wording;

						if (anyperc) {
							ANS ^= perc.oconv("R#6");
						}
					}

				}else{
					//clear the paid amount
					//990611 why? - anyway cannot clear now cos of multiple free types
					//if anyperc then array<3,ln2>=''
				}

			};//lnx;

			if (ANS.count(FM) > 1) {

				//if any free ad value
				if ((array.a(5)).sum()) {

					suppliername = "Total";
					if (not html) {
						suppliername = suppliername.oconv("L#15") ^ " ";
					}

					var medium = " ";
					if (not html) {
						medium = medium.oconv("L#15") ^ " ";
					}

					var nfree = (array.a(4)).sum();
					if (not html) {
						nfree = nfree.oconv("R#5");
					}

					var freevalue = ((array.a(5)).sum()).oconv(USER2.substr(1, 2) ^ "20P,");
					if (not html) {
						freevalue = freevalue.oconv("R#12");
					}

					var extras = "Unpaid";
					var wording = " " ^ extras ^ " ad";
					if (array.a(4).sum() == 1)
						wording^=" ";
					else
						wording^="s";

					wording = wording.oconv("L#15");
					wording ^= " = " ^ freevalue;

					perc = "";
					//if r<unit.bill.fn> ne r<unit.cost.fn> then
					// if sum(array<3>) then
					// perc=OCONV(100*sum(array<5>)/sum(array<3>)),'MD00P'):'%'
					// end else
					// perc='100%'
					// end
					//end else
					// perc=''
					// end

					//either build the total line in html
					if (html) {
						ANS ^= FM;
						if (nreqvehicles ne 1) {
							ANS ^= suppliername ^ VM ^ medium ^ VM;
							extras = " ";
						}else{
							extras = "Total";
						}
						ANS ^= extras ^ VM;
						ANS ^= nfree ^ VM ^ freevalue;
						if (anyperc) {
							ANS ^= VM ^ perc;
						}

						//or build the total line in text
					}else{
						ANS ^= FM;

						//supplier name
						ANS ^= suppliername;

						//medium
						ANS ^= medium;

						//number of free ads
						ANS ^= nfree;

						//wording and value of free ads
						ANS ^= wording;

						//free percentage
						//only for "cost to client", *assume that it is in "cost to agency"
						//mode if the net unit bills are the same as the net unit costs
						if (anyperc) {
							ANS ^= perc.oconv("R#6");
						}
					}

				}

			}

			//build and insert the title row
			ANS.splicer(1, 1, "");
			if (html and ANS) {
				var tt = "";
				if (nreqvehicles ne 1) {
	//Supplier
	//Vehicle
					if (agy.agp.a(2) == "CYP")
						tt^=lang.a(3);
					else
						tt^=lang.a(1);
	//Medium
					tt ^= VM ^ lang.a(127) ^ VM;
				}
	//Type
				tt ^= lang.a(128) ^ VM;
	//No.
	//Value
				tt ^= lang.a(55) ^ VM ^ lang.a(129);
				if (html) {
					tt ^= " (" ^ calculate("CURRENCY_CODE") ^ ")";
				}
	//% of paid
				if (anyperc) {
					tt ^= VM ^ lang.a(130);
				}
				tt ^= FM;
				ANS.splicer(1, 0, tt);
			}

			ANS.transfer(ANS);

			//////
			return 0;
			//////

		}

		//show the results on screen
		//convert "array" to tx
		var msg = "TOTALS" ^ FM;
		msg.r(-1, "Vehicle      Type            -Paid-(Gross)-        ----Free--------------");
		msg.r(-1, "                              No.    Amount         No.    Amount        %1%");
		for (var lnx = 1; lnx <= nlines; ++lnx) {

			//if array<5,LNX> then
			if (1) {

				var tx = "";

				//print the vehicle name, medium and type of free ads
				suppliername = array.a(1, lnx);
				if (not(array.locate(suppliername.field("|", 1), ln2, 7))) {
					{}
				}
				if (not msg.index( suppliername.field("|", 1).substr(1, 13) )) {
					tx ^= (array.a(2, ln2)).oconv("R#10") ^ " ";
					tx ^= (array.a(3, ln2)).oconv("R#10") ^ " ";
				}else{
					tx ^= var(22).space();
				}

				//free ads
				if (array.a(4, lnx)) {
					tx ^= (array.a(4, lnx)).oconv("R#10") ^ " ";
					tx ^= (array.a(5, lnx)).oconv("R#10") ^ " ";
					if (array.a(3, ln2)) {
						perc = (100 * array.a(5, lnx) / array.a(3, ln2)).oconv("MD00P") ^ "%1%";
					}else{
						//perc='100%1%'
						perc = "All unpaid";
					}
					tx ^= perc.oconv("R#10");

					//no free
				}else{
					tx ^= var(30).space();
				}

				//if anything to print then print
				if (tx.trim()) {

					var tx2 = "";
					tx2 ^= (suppliername.field("|", 1)).oconv("L#13") ^ " ";
					tx2 ^= (suppliername.field("|", 3)).oconv("L#7") ^ " ";

					msg.r(-1, tx2 ^ tx);

				}

			}
		};//lnx;

		//show the total
		if (nlines > 1) {
			var tx = var("TOTAL").oconv("L#13") ^ " ";
			tx ^= var("").oconv("L#7") ^ " ";
			tx ^= ((array.a(2)).sum()).oconv("R#10") ^ " ";
			//garbagecollect;
			tx ^= (((array.a(3)).sum()).oconv("MD" ^ RECORD.a(1) ^ "0P")).oconv("R#10") ^ " ";
			if ((array.a(4)).sum()) {
				tx ^= ((array.a(4)).sum()).oconv("R#10") ^ " ";
				tx ^= (((array.a(5)).sum()).oconv("MD" ^ RECORD.a(1) ^ "0P")).oconv("R#10") ^ " ";
				perc = "";
				//if sum(array<3>) then
				// perc=OCONV(100*sum(array<5>)/sum(array<3>)),'MD00P'):'%1%'
				//end else
				// perc='100%1%'
				// end
				tx ^= perc.oconv("R#10");
			}else{
				tx ^= var(30).space();
			}
			msg.r(-1, tx);
		}

		msg ^= FM;
		//call note(msg, "", "", "%");
		call note(msg);

	}

	return 0;

}


libraryexit()
