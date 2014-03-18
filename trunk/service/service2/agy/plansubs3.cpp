#include <exodus/library.h>
libraryinit()

//#include <catalyst.h>
#include <materialsubs.h>
#include <openfile.h>
//#include <btreeextract.h>
#include <giveway.h>
#include <generalsubs.h>
#include <plansubs3b.h>
#include <openfile.h>
//#include <pop_up.h>
#include <plansubs2.h>
#include <plansubs3.h>//recursive
#include <singular.h>
#include <authorised.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>//after win

var msg;
var reply;//num
var tt;
var materialno;
var letter;
var xx;
var dictads;
var dictclients;
var btreecode;
var ad;
var adtime2;//num
var vehiclen;//num
var msgx;
var programs;
var dom;//num
var datax;
var rating;
var ratingn;//num
var t2;
var wsmsg;
var omv;
var orev_mv;

function main(in mode) {
	//jbase
	//garbagecollect;

	var interactive = not SYSTEM.a(33);
	var tocurr;

	if (mode == "PERPETUAL") {

		var amvfns = "19" _VM_ "20" _VM_ "21" _VM_ "22" _VM_ "23" _VM_ "24" _VM_ "27" _VM_ "37" _VM_ "38" _VM_ "39" _VM_ "40" _VM_ "41" _VM_ "42" _VM_ "43" _VM_ "44" _VM_ "45" _VM_ "46" _VM_ "47" _VM_ "48" _VM_ "49" _VM_ "50" _VM_ "51" _VM_ "52" _VM_ "54" _VM_ "55" _VM_ "56" _VM_ "62" _VM_ "63" _VM_ "65" _VM_ "67" _VM_ "70" _VM_ "71" _VM_ "72" _VM_ "73" _VM_ "75" _VM_ "76" "" _VM_ "111" _VM_ "114" _VM_ "106" _VM_ "116" _VM_ "126" _VM_ "138" _VM_ "139" _VM_ "140" _VM_ "141" _VM_ "142" _VM_ "143" _VM_ "144" _VM_ "145" _VM_ "146" _VM_ "147" _VM_ "148" _VM_ "149" _VM_ "150" _VM_ "151" _VM_ "152" _VM_ "156" _VM_ "159" _VM_ "160" _VM_ "161" _VM_ "162" _VM_ "164" _VM_ "165" _VM_ "166" _VM_ "167" _VM_ "171" _VM_ "172" _VM_ "173" _VM_ "174" _VM_ "175" _VM_ "176" _VM_ "177" _VM_ "178" _VM_ "179" _VM_ "180" _VM_ "181" _VM_ "182" _VM_ "183" _VM_ "190" _VM_ "191" _VM_ "221" _VM_ "68";

		if (not win.amvaction) {
			return 0;
		}
		if (win.curramvgroup ne 1) {
			return 0;
		}
		if (not win.mvx) {
			var zzz = zzz;
		}

		if (not(authorised("SCHEDULE UPDATE PLAN", msg, ""))) {
			win.amvaction = "";
			return invalid(msg);
		}

		if (win.amvaction == 2) {

			if (win.datafile == "SCHEDULES") {
				var r7 = "!";
				var r8 = "!";
				//require credit note ?
				MV = win.mvx;
				if ((calculate("BILL") or calculate("COST")) and RECORD.a(46, win.mvx) ne "") {
					if (RECORD.a(7) == "" and RECORD.a(6) + 0 ne 0) {
						if (not(authorised("SCHEDULE UPDATE AFTER INVOICE", msg, ""))) {
							win.amvaction = "";
							return invalid(msg);
						}
						r7 = "X";
					}
				}

				//requires booking cancellation ?
				if (RECORD.a(51, win.mvx) ne "") {

					//message for rebooking
					if (not(authorised("SCHEDULE UPDATE AFTER BOOKING", msg, ""))) {
cancelaction:
						win.amvaction = "";
						return invalid(msg);
					}

					if (not(decide("Do you want to issue a|booking cancellation later ?", "", reply))) {
						goto cancelaction;
					}
					if (reply == 1) {
						///gosub cancelbooking EOF_542();
					}

				}

				if (r7 ne "!") {
					RECORD.r(7, r7);
					msg = "A CREDIT NOTE WILL HAVE TO BE RAISED";
					gosub note(msg);
				}
				if (r8 ne "!") {
					RECORD.r(8, r8);
					//MSG='BOOKING CANCELLATION REQUIRED'
					//GOSUB NOTE
				}

			}

			//delete any missing columns
			var nn = amvfns.count(VM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				tt = amvfns.a(1, ii);
				if (not(win.registerx[1].a(tt))) {
					if (RECORD.a(tt)) {
						RECORD.eraser(tt, win.mvx);
					}
				}
			};//ii;

		//insert any missing columns
		} else if (win.amvaction == 3) {
			var nn = amvfns.count(VM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				tt = amvfns.a(1, ii);
				if (not(win.registerx[1].a(tt))) {
					if (RECORD.a(tt)) {
						RECORD.inserter(tt, win.mvx, "");
					}
				}
			};//ii;
		}

	} else if (mode == "VAL.DISCOUNT" or mode == "VAL.TAX") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub checkupdate();
		if (not win.valid) {
			return 0;
		}

	/*} else if (mode == "SCHSTATUS") {
		call catalyst("W", "@ADTEMPLATES@SCHSTATUS " ^ ID);

	*/
	} else if (mode == "DEF.VERSION") {
		if (win.is) {
			return 0;
		}
		ANS = 1;

defversion:
		win.isorig = ANS;
		RECORD.r(win.si.a(4), ANS);

	} else if (mode == "DEF.DATE") {
		if (not win.is) {
			ANS = var().date();
			goto defversion;
		}

	} else if (mode == "F2.MATERIAL") {
		var xr = RECORD;
		var xid = ID;
		var xm = MV;
		var xdict = DICT;
		perform("SSELECT MATERIALS WITH BRAND_CODE " ^ (DQ ^ (calculate("BRAND_CODE") ^ DQ)) ^ " \"\" (S)");
		xr.transfer(RECORD);
		xid.transfer(ID);
		xm.transfer(MV);
		xdict.transfer(DICT);
		if (not LISTACTIVE) {
			msg = calculate("BRAND_CODE").quote() ^ " - BRAND HAS NO RECORDS IN THE MATERIAL FILE";
			return invalid(msg);
		}
//		call catalyst("P", "ADPOPUPS*MATERIAL");

	} else if (mode == "VIEWMATERIAL") {

		//try and get actual material file number
		if (win.si.a(35) == 2) {
			var storemv = MV;
			MV = win.mvx;
			materialno = calculate("PRIOR_MATERIAL_NO");
			MV = storemv;

			//otherwise use schedule and material letter of line or select from all letters
			if (materialno == "") {
				MV = win.mvx;
				letter = calculate("MATERIAL_LETTER");
				MV = storemv;
				if (letter == "") {
					gosub getletter();
					if (letter == "") {
						return 0;
					}
				}
				materialno = ID ^ "." ^ letter;
			}

		}else{
			gosub getletter();
			if (letter == "") {
				return 0;
			}
			materialno = ID ^ "." ^ letter;
		}

		call materialsubs("VIEWMATERIAL-" ^ materialno ^ "-" ^ calculate("BRAND_CODE"));

	} else if (mode == "VAL.LETTER") {

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		if (RECORD.a(184).locateusing(win.is, VM, xx)) {
			msg = DQ ^ (win.is ^ DQ) ^ " material code is already used in line " ^ xx;
			return invalid(msg);
		}

		if (not(var("ABCDEFGHIJKLMNOPQRSTUVWXYZ").index(win.is, 1))) {
			msg = "Please enter a single letter from A-Z|that matches one of the letters in the date grid";
			return invalid(msg);
		}

	} else if (mode == "VAL.MATERIAL") {

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		var schid = win.is.field(".", 1);
		letter = win.is.field(".", 2);
		if (schid == ID) {
			msg = "Do not refer to the schedule that you are on.";
			return invalid(msg);
		}

		call materialsubs("GETMATERIALCHAIN-" ^ win.is ^ "-" ^ calculate("BRAND_CODE"));
		if (not win.valid) {
			return 0;
		}
		var materialchain = ANS;
		ANS = "";

		//get latest material description/instructions
		var materialdescs = materialchain.a(3);
		//loop while materialdescs[1,1]=vm do repeat
		materialdescs.trimmerf(VM);
		var material = "";
		material.r(2, materialdescs.a(1, 1));

		//copy the material description
		if (RECORD.a(155, win.mvx) == "" and material.a(2)) {
			RECORD.r(155, win.mvx, material.a(2));
			tt = win.registerx[1].a(155);
			gosub r();
		}

	} else if (mode == "SWITCHCOINCIDENCECHECKING") {
		if (not(decide("Do you want to be notified of any ads|that coincide with new ads as you enter them ?", "", reply))) {
			return 0;
		}
		agy.agp.r(31, reply);

	} else if (mode == "CHECKCOINCIDENCE") {
		//if chkcoincide=2 then return 0

		//check other ads

		if (not(openfile("DICT.ADS", dictads))) {
			return 0;
		}
		if (not(openfile("DICT.CLIENTS", dictclients))) {
			return 0;
		}

		ID = mv.PSEUDO.a(1);
		var brandcode = mv.PSEUDO.a(2);
		var currvehiclecode = mv.PSEUDO.a(3);
		var startdate = mv.PSEUDO.a(4);

		var newdates = mv.PSEUDO.a(5);
		var olddates = mv.PSEUDO.a(6);

		var newtime = mv.PSEUDO.a(7).field("-", 1);
		if (newtime and not newtime.isnum()) {
			newtime = newtime.iconv("[TIME2]");
		}

		//we no longer use oldtime in @mv.PSEUDO<8> if newtime is a range
		var newtime2 = mv.PSEUDO.a(7).field("-", 2);
		if (not newtime2) {
			newtime2 = mv.PSEUDO.a(8);
		}
		if (not newtime2) {
			newtime2 = newtime;
		}
		if (newtime2 and not newtime2.isnum()) {
			newtime2 = newtime2.iconv("[TIME2]");
		}

		var newspec = mv.PSEUDO.a(9);
		var oldspec = mv.PSEUDO.a(10);
		var newspec1 = (newspec.field(",", 1)).field("*", 1);

		mv.PSEUDO = "";

		//get brand and clientcode
		var brand;
		if (not(brand.read(agy.brands, brandcode))) {
			msg = DQ ^ (brandcode ^ DQ) ^ " Brand does not exist";
			return invalid(msg);
		}
		var clientcode = brand.a(1, 1);

		var coincidences = "";

		//for tv and radio check all other vehicles of the same medium and market
		var vehicle;
		if (not(vehicle.read(agy.vehicles, currvehiclecode))) {
			vehicle = "";
		}
		var reqmedium = vehicle.a(2);
		var reqmarket = vehicle.a(4);
		var maxminutes = reqmedium.xlate("MEDIA.TYPES", 8, "X");

		//include ANY medium which has max minutes specified
		//and certain media type codes for TV, Satellite and Radio
		if (maxminutes == "") {
			//locate reqmedium in 'TV,T,SAT,S,RAD,R'<1> using ',' setting x then
			//tv satellite radio newspaper (daily)
			if (var("TSR").index(reqmedium.substr(1, 1), 1)) {

				//global default to 15 minutes for radio, 30 mins for TV and Satellite
				maxminutes = reqmedium.xlate("MEDIA.TYPES", 8, "X");
				if (maxminutes == "") {
					if (reqmedium == "R" or reqmedium == "RAD") {
						maxminutes = 15;
					}else{
						maxminutes = 30;
					}
				}

			}
		}

		if (not maxminutes) {

			//prevent checking whole medium/market if not broadcast media
			//or checking set to 0 minutes!
			reqmedium = "";
			reqmarket = "";

		}else{

			//dont check broadcast media unless there is a time
			if (newtime == "") {
				return 0;
			}

		}

		//vehicledata is to save reading the vehicles file for every ad
		//vehicledata<1> is mv list of vehicle codes
		//vehicledata<2> is a parallel list of flags if the vehcile is wanted
		var vehicledata = "";

		//get the client/principle
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			client = "";
		}
		var principlecode = client.a(16);
		if (principlecode == "") {
			principlecode = clientcode;
		}

		//clientcoincidencemode
		var clientcoincidencemode = agy.agp.a(84);
		if (clientcoincidencemode == 5) {
			return 0;
		}
		if (clientcoincidencemode == "" or maxminutes) {
			clientcoincidencemode = 2;
		}

		//coincidence Blank or 1=All products 2=Same or conflicting product category only
		//for broadcast only
		//mediummode Blank or 1=Same medium/market
		//marketmode Blank or 1=Same market

		//get other group members
		var reqclientcodes = "";
		if (clientcoincidencemode == 1) {

		} else if (clientcoincidencemode == 2) {
			tt = "CLIENT_GROUP" ^ VM ^ principlecode ^ FM;
//TODO CONVERT			call btreeextract(tt, "CLIENTS", dictclients, reqclientcodes);
			//make sure we catch ourself
			if (not(reqclientcodes.a(1).locateusing(principlecode, VM, xx))) {
				reqclientcodes.r(1, -1, principlecode);
			}
			if (not(reqclientcodes.a(1).locateusing(clientcode, VM, xx))) {
				reqclientcodes.r(1, -1, clientcode);
			}

		} else if (clientcoincidencemode == 3) {
			reqclientcodes = clientcode;

		} else if (clientcoincidencemode == 4) {
			//will be filtered on brand
		}

		//smdates for i=1 to len(trimb(newdates))
		var nnewdates = newdates.count(SVM) + (newdates ne "");
		for (var ii = 1; ii <= nnewdates; ++ii) {
			//smdates if trim(newdates[i,1]) and not(trim(olddates[i,1])) then
			//if newdates<1,1,i> and not(olddates<1,1,i>) then
			if (newdates.a(1, 1, ii) ne olddates.a(1, 1, ii)) {

				var newdate = startdate + ii - 1;
				//if newdate>=date() then
				if (newdate >= 1) {

					if (reqmedium) {
						btreecode = "DATE" ^ VM ^ newdate.oconv("[DATE]") ^ FM;
						//t:='VEHICLE_CODE':@vm:currvehiclecode:@fm
					}else{
						btreecode = "VEHICLE_AND_DATE" ^ VM ^ currvehiclecode ^ "*" ^ newdate ^ FM;
					}

					var hits = "";
//TODO!					call btreeextract(btreecode, "ADS", dictads, hits);
					USER4 = "";
					if (hits) {
						var nhits = hits.count(VM) + 1;
						for (var hitn = 1; hitn <= nhits; ++hitn) {
							var hit = hits.a(1, hitn);

							if (not interactive) {
								if (not giveway()) {
									{}
								}
							}

							var advehicle;
							//skip if in the same schedule
							if (hit.field("*", 1) == ID) {
								continue;//goto nextad
							}

							//skip if planned
							if (hit.field("*", 5) == "P") {
								continue;//goto nextad
							}

							//if same vehicle
							advehicle = hit.field("*", 2);
							if (advehicle == currvehiclecode) {

checkad:

								//no coincident if times are separated by more than x minutes
								var coincide = 1;
								ad = "";
								var adtime = "";
								if (newtime ne "") {

									if (not(ad.read(agy.ads, hit))) {
										ad = "";
									}
									gosub getadtime();

									if (newtime ne "" and adtime ne "") {
										//if abs(newtime-adtime) gt maxminutes*60 then coincide=0
										if (newtime < adtime - maxminutes * 60) {
											coincide = 0;
										}
										//if newtime2>(adtime+maxminutes*60) then coincide=0
										if (newtime2 > adtime2 + maxminutes * 60) {
											coincide = 0;
										}
									}

									//if no time on ad in schedule but maxmins=0 then
									//if spec row doesnt agree then not coincident

								} else if (maxminutes == 0) {
									if (not(ad.read(agy.ads, hit))) {
										ad = "";
									}
									if (newspec1 ne (ad.a(35).field(",", 1)).field("*", 1)) {
										coincide = 0;
									}

								}

								if (coincide) {

									if (not ad) {

										if (not(ad.read(agy.ads, hit))) {
											ad = "";
										}
										gosub getadtime();

									}

									//skip if secondary market same vehicle
									if (maxminutes ne 0) {
										if (ad.a(27) ne ad.a(20) and hit.field("*", 2) == currvehiclecode) {
											continue;//goto nextad
										}
									}

									//option for ALL clients in the same issue to be warned
									//to avoid double booking exclusive positions
									if (clientcoincidencemode == 4) {
										if (ad.a(3) == brandcode) {
											goto coincident;
										}
									}else{
										if (reqclientcodes == "") {
											goto coincident;
										}
									}

									if (reqclientcodes.a(1).locateusing(ad.a(17), VM, xx)) {

										//warning if any other same client group ads
coincident:

										if (coincidences.length() > 32000) {
											goto gotcoincidences;
										}
										//coincidences<-1>=xlate('BRANDS',ad<3>,2,'X') 'L#20':' ':OCONV(ad<12>,'[DATE,4*]') 'L#10':' ':OCONV(adtime,'[TIME2,48MT]') 'L#5':' ':ad<1> 'L#20'
										coincidences ^= "<TR>" "<TD>" ^ ad.a(3).xlate("BRANDS", 2, "X") ^ "</TD>";
										coincidences ^= "<TD>" ^ (ad.a(12)).oconv("[DATE,4*]") ^ "</TD>";
										coincidences ^= "<TD>" ^ adtime.oconv("[TIME2]") ^ "</TD>";
										coincidences ^= "<TD>" ^ ad.a(35) ^ "</TD>";
										coincidences ^= "<TD>" ^ ad.a(1) ^ "</TD>";
										if (reqmedium) {
											coincidences ^= "<TD>" ^ advehicle ^ "</TD>";
										}
										coincidences ^= "</TR>";

									}else{

										//warning if conflicting product categories with ANY client
										//on the same vehicle
										if (brand.a(12)) {
											if (brand.a(12) == ad.a(41)) {
												goto coincident;
											}
										}

									}

								}

							}else{

								//if not the same vehicle, check if TV or Radio in the same market
								if (reqmedium) {
									if (vehicledata.a(1).locateusing(advehicle, VM, vehiclen)) {
										if (vehicledata.a(2)) {
											goto checkad;
										}
									}else{

										vehicledata.r(2, vehiclen, advehicle);
										if (vehicle.read(agy.vehicles, advehicle)) {
											if (vehicle.a(2) == reqmedium and vehicle.a(4) == reqmarket) {
												vehicledata.r(2, vehiclen, 1);
												goto checkad;
											}
										}

									}
								}
							}
						}
					}
				}
			}
		}
gotcoincidences:

		if (coincidences) {
			//NB: WARNING: is currently a key word in the message for the intranet client
			//msg='WARNING:|The following ads are also in '
			msg = "The following ads are also in ";
			if (authorised("SCHEDULE COINCIDENT ADS", msgx)) {
				msg.splicer(1, 0, "WARNING: ");
			}else{
				msg.splicer(1, 0, msgx ^ "||");
			}
			if (reqmedium) {
				msg ^= reqmarket.xlate("MARKETS", 1, "X") ^ " ";
				msg ^= reqmedium.xlate("MEDIA.TYPES", 1, "X") ^ " ";
			}else{
				msg ^= currvehiclecode.xlate("VEHICLES", 1, "X") ^ " ";
			}
			msg ^= "|on the same day";
			if (newtime) {
				msg ^= " up to " ^ maxminutes ^ " minutes away from your ad";
			} else if (maxminutes == 0) {
				msg ^= " at " ^ newspec1;
			}

			msg ^= ".";
			//msg:=', is this OK ?'
			if (newtime)
				tt= "Time";
			else
				tt="";
			if (interactive) {
				msg ^= "||" ^ var("Brand").oconv("L#20") ^ var(" Date").oconv("L#10") ^ var(" " ^ tt).oconv("L#6") ^ var(" Schedule No").oconv("L#20");
			}
			//if reqmedium then msg:=' Vehicle'
			msg.r(-1, FM ^ "<table>" "<TR>" "<TD>" "Brand" "</TD>" "<TD>" "Date" "</TD>" "<TD>" "</TD>" "<TD>" "Specification" "</TD>" "<TD>" "Schedule" "</TD>" "</TR>");
			msg.r(-1, coincidences ^ "</table>");
			//print char(7)
			//reply='YES'
			//call msg2(msg,'RC',reply,'')
			//if index('Yy',reply[1,1],1) else valid=0
			//gosub note
		}else{
			msg = "";
		}
		mv.PSEUDO = msg;

	} else if (mode == "VAL.EXCH.RATE") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub checkupdate();
		if (not win.valid) {
			return 0;
		}

		tocurr = calculate("CURRENCY_CODE");

valexchrate:
		call generalsubs("VAL.EXCH.RATE");
		if (not win.valid) {
			return 0;
		}

		//force exchange rate "1" if not converting currency
		var fromcurr = RECORD.a(45, win.mvx);
		if (fromcurr == tocurr) {
			if (win.is ne 1) {
				var().chr(7).output();
				call note("NOTE:|THE EXCHANGE RATE FROM " ^ fromcurr ^ " TO " ^ tocurr ^ " MUST BE 1");
				win.is = 1;
				return 0;
			}
		}

	} else if (mode == "VAL.COST.EXCH.RATE") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub checkupdate();
		if (not win.valid) {
			return 0;
		}

		call note("WARNING:|YOU SHOULD NOT NORMALLY|CHANGE THIS EXCHANGE RATE");

		tocurr = agy.agp.a(2);
		goto valexchrate;

		//11/7/97
		//case mode='SHOWTOTS' or mode='GETFREE'
	} else if (mode.substr(1, 8) == "SHOWTOTS" or (mode.field(",", 1)).index("GETFREE", 1)) {

		call plansubs3b(mode);
/*unconverted
	} else if (mode == "FINDPROG") {

		gosub checkupdate();
		if (not win.valid) {
			return 0;
		}

		//check that we are on the lines
		if (not win.mvx) {
			msg = "PLEASE GO TO THE LINES SECTION FIRST";
			return invalid(msg);
		}

		if (not(openfile("PROGRAMS", programs, "SCHEDULES", 1))) {
			return 0;
		}

		//user selects a date (maybe outside the schedule period)
		var datex = (RECORD.a(70, win.mvx)).oconv("[DAY.OF.MONTH]");
		call note2("For which day or date do you want the programs ?|(You can omit the year and the month)", "RC", datex, "");
		if (not datex) {
			return 0;
		}
		datex = datex.field(" ", 1);
		if (datex.isnum()) {
			var idate = datex.iconv("[DAY.OF.MONTH]");
			if (mv.STATUS) {
				msg = "Please enter a number 1-31";
				return invalid(msg);
			}
			datex ^= "/" ^ calculate("PERIOD");
		}

		//check & convert date format
		var idate = datex.iconv("[DATE]");
		if (mv.STATUS) {
			msg = DQ ^ (datex ^ DQ) ^ " cannot be understood as a date";
			return invalid(msg);
		}
		datex = idate.oconv("[DATE,*]");
		var daynofweek = idate - 1 % 7 + 1;

		//calculate day of the current month
		//if the program date is in the schedule month, no problem
		//if the program date is outside the schedule month, use the first date
		// in the schedule month that is the same day of week as the program date
		//swap '/0' with '/' in DATEX
		//swap '/ ' with '/' in DATEX
		if (datex.field("/", 2, 2) == calculate("PERIOD")) {
			dom = (idate.oconv("D")).substr(1, 2) + 0;
		}else{
			for (dom = 1; dom <= 7; ++dom) {
			///BREAK;
			if ((dom ^ "/" ^ calculate("PERIOD")).iconv("D2/E") - 1 % 7 + 1 == daynofweek) break;;
			};//dom;
		}

		//five vehicles in columns, half hour blocks
		var vehiclecodes = win.registerx[9];
		if (not vehiclecodes) {
			if (not(vehiclecodes.read(gen._definitions, "MAINTV"))) {
				vehiclecodes = "ANT1,RIK TV1,RIK TV2,LOGOS,SIGMA";
			}
		}
		vehiclecodes.converter(VM, ",");
		call note2(var("Which channels do you want ?").oconv("L#50"), "RC", vehiclecodes, "");
		if (not vehiclecodes) {
			return 0;
		}
		vehiclecodes.swapper(", ", ",");
		vehiclecodes.swapper(" ,", ",");
		vehiclecodes.converter(",", VM);
		win.registerx[9] = vehiclecodes;
		var nvehicles = vehiclecodes.count(VM) + 1;
		vehiclecodes.write(gen._definitions, "MAINTV");

		//divide into half hours
		var minperdiv = 30;
		var ndiv = 24 * 60 * 60 / (minperdiv * 60);
		var datax(nvehicles, nvehicles);//TODO:var datax(nvehicles, ndiv);
		datax.init("");
		for (var vehiclen = 1; vehiclen <= nvehicles; ++vehiclen) {

			//get the program list for the vehicle for the selected date
			var vehiclecode = vehiclecodes.a(1, vehiclen);

			var dow = idate - 1 % 7 + 1;
			var idatex = idate;
			while (true) {
			///BREAK;
			if (not(idatex > var().date())) break;;
				idatex -= 7;
			}//loop;
			var ratingkey = calculate("SURVEY_CODE1") ^ "*" ^ calculate("TARGET1") ^ "*" ^ vehiclecode ^ "*" ^ dow;
			//try to get the diary ratings ...
			var rating;
			if (not(rating.read(agy.ratings, ratingkey))) {
nextratings:
				//otherwise try to get the people meter data
				ratingkey = calculate("SURVEY_CODE1").field(" ", 1) ^ "*" ^ calculate("TARGET1") ^ "*" ^ vehiclecode ^ "*" ^ idatex;
				if (not(rating.read(agy.ratings, ratingkey))) {
					rating = "";
					if (idatex > idate - 90) {
						idatex -= 7;
						goto nextratings;
					}
				}
			}

			var proglist;
			if (not(proglist.read(programs, vehiclecode ^ "*" ^ idate))) {
				proglist = "";
				var nprogs = "";
				if (rating) {

					//create a program list from the survey program list
					var details = rating.a(23);
					details.converter(SVM, VM);
					var ndetails = details.count(VM) + 1;
					for (var detailn = 1; detailn <= ndetails; ++detailn) {
						var detail = details.a(1, detailn);
						if (detail) {
							nprogs += 1;
							proglist.r(1, nprogs, detail.field("/", 3));
							proglist.r(2, nprogs, (detail.field("/", 1)).iconv("[TIME2]"));
						}
					};//detailn;

				}
			}

			if (proglist) {

				//build a popup list
				var nprogs = (proglist.a(1)).count(VM) + 1;
				for (var progn = 1; progn <= nprogs; ++progn) {
					var timex = proglist.a(2, progn);
					var divn = (timex / (minperdiv * 60)).floor();
					//put times from 00:00 to 04:00 AFTER 24:00 not before
					divn = divn - 4 * 60 / minperdiv % ndiv + 1;
					var nn = datax[vehiclen][divn].a(4) + 1;
					datax[vehiclen][divn].r(4, nn);
					datax[vehiclen][divn].r(1, nn, proglist.a(1, progn));
					datax[vehiclen][divn].r(2, nn, timex);
					if (rating) {
						if (not(rating.a(111).locatebyusing(timex, "AR", ratingn, VM))) {
							ratingn -= 1;
						}
						var ratingperc = rating.a(106, ratingn);
						datax[vehiclen][divn].r(6, nn, ratingperc);
					}
					datax[vehiclen][divn].r(3, nn, proglist.a(3, progn));
				};//progn;

			}

		};//vehiclen;

		//convert half hours into rev popup array
		var popupdata = "";
		var npopuplines = 0;
		for (var divn = 1; divn <= ndiv; ++divn) {
			var maxn = 0;
			for (var vehiclen = 1; vehiclen <= nvehicles; ++vehiclen) {
				var nn = datax[vehiclen][divn].a(4);
				if (nn > maxn) {
					maxn = nn;
				}
				for (var progn = 1; progn <= nn; ++progn) {
					var temp = "";
					temp ^= "\\" ^ (datax[vehiclen][divn].a(2, progn)).oconv("[TIME2]") ^ "\\";
					if (rating) {
						temp ^= datax[vehiclen][divn].a(6, progn) ^ "%";
					}
					temp ^= "\\" ^ datax[vehiclen][divn].a(1, progn);
					temp ^= "\\" ^ datax[vehiclen][divn].a(3, progn);
					if (temp ne "\\\\\\\\") {
						popupdata.r(npopuplines + progn, vehiclen, temp.substr(2, 999));
					}
				};//progn;
			};//vehiclen;
			npopuplines += maxn;
		};//divn;

		//user selects the times (rows)
		var dayofweek = var("Monday" _VM_ "Tuesday" _VM_ "Wednesday" _VM_ "Thursday" _VM_ "Friday" _VM_ "Saturday" _VM_ "Sunday").a(1, daynofweek);
		var question = dayofweek ^ " " ^ datex ^ " - Which program(s) do you want ?";
		var params = "";
		var width = (70 / nvehicles).floor();
		for (var vehiclen = 1; vehiclen <= nvehicles; ++vehiclen) {
			params ^= "\\" ^ vehiclen ^ ":" ^ width ^ ":L:: " ^ vehiclecodes.a(1, vehiclen);
		};//vehiclen;
		if (rating) {
			question ^= FM ^ "|(Ratings are for " ^ (DQ ^ (calculate("TARGET1") ^ DQ)) ^ " / " ^ (DQ ^ (calculate("SURVEY_CODE1") ^ DQ)) ^ ")";
		}else{
			if (calculate("TARGET1")) {
				question ^= "|(Ratings are not available because survey analysis";
				question ^= "|has not been done for \"" ^ calculate("TARGET1") ^ "\" / \"" ^ calculate("SURVEY_CODE1") ^ "\")";
			}
		}
		var progns = pop_up(0, 0, "", popupdata, params.substr(2, 999), "R", "O", question, "", "", "", "P");
		if (not progns) {
			return 0;
		}
		var nprogns = progns.count(FM) + 1;
		var progn = progns.a(1);

		//user selects the vehicle (column)
		//build a popup out of one row of the previous popupdata
		var popupdata2 = "";
		for (var vehiclen = 1; vehiclen <= nvehicles; ++vehiclen) {
			for (var prognn = 1; prognn <= nprogns; ++prognn) {
				tt = popupdata.a(progns.a(prognn), vehiclen);
				if (tt) {
					popupdata2.r(-1, (vehiclecodes.a(1, vehiclen)).oconv("L#9") ^ tt);
				}
			};//prognn;
		};//vehiclen;
		if (not(decide("~Which program(s) do you want ?", "" ^ popupdata2, reply))) {
			return 0;
		}
		progns = reply;
		nprogns = progns.count(FM) + 1;

		//user can input material now or later
		var material = RECORD.a(23, win.mvx).field("/", 2);
		call note2("If you want to specify the material/theme now,|What material will you use ?", "R", material, "");

		var duration = "";
		var storemv = win.mvx;

		//all information has now been selected
		for (var prognn = 1; prognn <= nprogns; ++prognn) {

			//insert a blank line
			if (prognn > 1) {
				win.mvx += 1;
				gosub insertline();
			}

			progn = progns.a(prognn);
			var result = popupdata2.a(progns.a(prognn));
			var vehiclecode = result.substr(1, 8).trim();
			result.splicer(1, 9, "");
			var programx = result.field("\\", 3);
			var timex = (result.field("\\", 1)).iconv("[TIME2]");
			var ratingperc = result.field("\\", 2);
			ratingperc.splicer(-1, 1, "");
			var rateband = result.field("\\", 4);

			//put the vehicle in the schedule
			if (RECORD.a(20, win.mvx) ne vehiclecode) {

				//add ratecard dates
				if (RECORD.a(20).locateusing(vehiclecode, VM, tt)) {
					RECORD.r(24, win.mvx, RECORD.a(24, tt));
					RECORD.r(27, win.mvx, RECORD.a(27, tt));
				}

				//add the vehicle code
				RECORD.r(20, win.mvx, vehiclecode);
				tt = win.registerx[1].a(20);
				gosub r();
				var isx = win.is;
				var isorigx = win.isorig;
				win.is = vehiclecode;
				win.isorig = "";
				call plansubs2("VAL.VEHICLE.CODE");
				win.is = isx;
				win.isorig = isorigx;

			}

			//put the date and number of spots in the schedule
			RECORD.r(22, win.mvx, (dom.field(" ", 1) - 1).space() ^ "1");
			tt = win.registerx[1].a(22);
			gosub r();
			RECORD.r(70, win.mvx, dom);
			tt = win.registerx[1].a(70);
			gosub r();
			RECORD.r(39, win.mvx, 1);
			RECORD.r(43, win.mvx, 1);
			tt = win.registerx[1].a(39);
			gosub r();
			tt = win.registerx[1].a(43);
			gosub r();

			//put the time and program in the details
			var details = RECORD.a(23, win.mvx);
			if (timex) {
				details = details.fieldstore("/", 1, 1, timex.oconv("[TIME2]"));
			}
			if (material) {
				details = details.fieldstore("/", 2, 1, material);
			}
			if (programx) {
				details = details.fieldstore("/", 3, 1, programx);
			}
			RECORD.r(23, win.mvx, details);
			tt = win.registerx[1].a(23);
			gosub r();

			//put the nearest quarter hour into the rating time
			var ratingtime = ((7.5 * 60 + timex) / (15 * 60)).floor() * 15 * 60;
			RECORD.r(111, win.mvx, ratingtime);
			tt = win.registerx[1].a(111);
			gosub r();

			//put the rating
			RECORD.r(106, win.mvx, ratingperc);
			tt = win.registerx[1].a(106);
			gosub r();

			//put the rateband into the spec and get costs
			var spec = RECORD.a(21, win.mvx);
			if (not(spec.field(",", 2))) {

				//user can input duration now
				if (duration == "") {
					duration = "30\"";
					call note2("Duration ?", "R", duration, "");
				}
				spec = spec.fieldstore(",", 2, 1, duration);
			}
			RECORD.r(21, win.mvx, spec.fieldstore(",", 1, 1, rateband));
			tt = win.registerx[1].a(21);
			gosub r();
			call plansubs2("VAL.SPEC2");

		};//prognn;

		win.mvx = storemv;

		//update the number of lines
		win.amvvars.r(1, 3, (RECORD.a(20)).count(VM) + 1);

		//press Enter to redisplay the current field
		DATA ^= "" "\r";

		win.displayaction = 5;
		win.reset = 3;
*/
	} else {
		msg = mode.quote() ^ " - INVALID MODE IGNORED";
		return invalid(msg);

	}

	return 0;

}

subroutine r() {
	//redisplay
	if (not tt) {
		return;
	}
	if (not(win.redisplaylist.locateusing(tt, FM, t2))) {
		win.redisplaylist.r(-1, tt);
	}
	win.displayaction = 6;
	win.reset = 3;
	return;

}

subroutine checkupdate() {
	if (not(authorised("SCHEDULE UPDATE PLAN", msg, ""))) {
		invalid(msg);
	}
	return;

}

subroutine insertline() {
	var amvfns = "19" _VM_ "20" _VM_ "21" _VM_ "22" _VM_ "23" _VM_ "24" _VM_ "27" _VM_ "37" _VM_ "38" _VM_ "39" _VM_ "40" _VM_ "41" _VM_ "42" _VM_ "43" _VM_ "44" _VM_ "45" _VM_ "46" _VM_ "47" _VM_ "48" _VM_ "49" _VM_ "50" _VM_ "51" _VM_ "52" _VM_ "54" _VM_ "55" _VM_ "56" _VM_ "62" _VM_ "63" _VM_ "65" _VM_ "67" _VM_ "70" _VM_ "71" _VM_ "72" _VM_ "73" _VM_ "75" _VM_ "76" "" _VM_ "111" _VM_ "114" _VM_ "106" _VM_ "116" _VM_ "126" _VM_ "138" _VM_ "139" _VM_ "140" _VM_ "141" _VM_ "142" _VM_ "143" _VM_ "144" _VM_ "145" _VM_ "146" _VM_ "147" _VM_ "148" _VM_ "149" _VM_ "150" _VM_ "151" _VM_ "152" _VM_ "156" _VM_ "159" _VM_ "160" _VM_ "161" _VM_ "162" _VM_ "164" _VM_ "165" _VM_ "166" _VM_ "167" _VM_ "171" _VM_ "172" _VM_ "173" _VM_ "174" _VM_ "175" _VM_ "176" _VM_ "177" _VM_ "178" _VM_ "179" _VM_ "180" _VM_ "181" _VM_ "182" _VM_ "183" _VM_ "190" _VM_ "191" _VM_ "221" _VM_ "68";
	var nn = amvfns.count(VM) + 1;
	for (var ii = 1; ii <= nn; ++ii) {
		tt = amvfns.a(1, ii);
		if (RECORD.a(tt)) {
			RECORD.inserter(tt, win.mvx, "");
		}
	};//ii;

	win.displayaction = 6;
	win.reset = 3;

	return;

}

subroutine getletter() {

	//search for all possible letters
	var letters = "";
	var lettersources = RECORD.a(184) ^ RECORD.a(22) ^ RECORD.field(FM, 171, 13);
	lettersources.converter(FM ^ VM ^ " ", "");
	for (var ii = 65; ii <= 90; ++ii) {
		if (lettersources.index(var().chr(ii), 1)) {
			letters.r(1, -1, var().chr(ii));
		}
	};//ii;
	if (letters == "") {
		letters = "X";
	}

	//select letter
	if (letters.count(VM)) {
		if (not(decide("Which material do you want to view?", letters ^ "", reply))) {
			letter = "";
			return;
		}
		letter = letters;
	}else{
		reply = 1;
	}
	letter = letters.a(1, reply);

	return;

}

subroutine getadtime() {
	//see below also
	//break time, otherwise ad time, otherwise rating time
	var adtime = ad.a(43);
	adtime2 = "";
	if (adtime == "") {
		adtime = ad.a(14);
		adtime2 = ad.a(19);
		if (adtime == "") {
			adtime = ad.a(83);
			adtime2 = ad.a(84);
		}
	}
	if (adtime2 == "") {
		adtime2 = adtime;
	}
	return;

}


libraryexit()
