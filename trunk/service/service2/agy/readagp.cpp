#include <exodus/library.h>
libraryinit()

#include <initagency.h>

#include <gen.h>
#include <agy.h>

var vn;

function main() {
	//jbase

	if (not(agy.agp.read(gen._definitions, "AGENCY.PARAMS"))) {
		call note("AGENCY.PARAMS IS MISSING FROM DEFINITIONS");
		var().stop();
	}

//	agy.agp.r(1, (agy.agp.a(1)).invert());
	SYSTEM.r(14, agy.agp.a(1));
	SYSTEM.r(36, agy.agp.a(1));
	//always splitextras now
	agy.agp.r(5, 1);

	//get currency and format from company file
	if (gen.company.a(3)) {
		agy.agp.r(2, gen.company.a(3));
	}
	var basecurrency;
	if (basecurrency.read(gen.currencies, agy.agp.a(2))) {
		agy.agp.r(3, "MD" ^ basecurrency.a(3) ^ "0P");
	}

	//eurorscg/promopub
	//mpg is 2106BBA2
	//already in database now
	//if system<45>='1E16E973' or system<45>='2106BBA2' or index(system<23>,'PROMOPUB',1) then
	// if len(agp<77>) else agp<77>='1'
	// if len(agp<78>) else agp<78>='1'
	// end
	if (agy.agp.a(77)) {
		agy.agp.r(79, 1, -1, "4");
	}
	if (agy.agp.a(78)) {
		agy.agp.r(79, 1, -1, "3");
	}

	//force CYP market code code
	if (agy.agp.a(37) == "" and agy.agp.a(2) == "CYP") {
		agy.agp.r(37, "CYP");
	}

	//rounding Tamra
	if (agy.agp.a(32) == "" and (gen.company.a(1)).index("Tamra", 1)) {
		agy.agp.r(32, "1");
	}

	//unitbillrounding
//	PRIORITYINT.r(101, 1, agy.agp.a(32, 1));
//	PRIORITYINT.r(101, 2, agy.agp.a(32, 2));

	//various market related data is copied to agp to support old code
	var marketcode = agy.agp.a(37);
	if (not marketcode) {
		marketcode = gen.company.a(30);
		agy.agp.r(37, marketcode);
	}
	var market;

var schedules;
var plans;
var clients;
var vehicles;
var ratecards;
var markets;
var brands;
	if (market.read(agy.markets, marketcode)) {

		//NB agp<13> is LAST DAY OF WEEK whereas market<10> is FIRST DAY OF WEEK
		if (market.a(10)) {
			agy.agp.r(13, market.a(10) - 2 % 7 + 1);
		}

		//highlight days of week for schedule grid
		if (market.a(11)) {
			agy.agp.r(104, market.a(11));
		}

		//weekend/weekdays off (for deadlines etc)
		if (market.a(9)) {
			agy.agp.r(95, market.a(9));
		}

		//fix market weekend from agp<95> or agp<13> if present
		if (market.a(9) == "") {
			var weekenddows = agy.agp.a(95);
			if (weekenddows == "" and agy.agp.a(13)) {
				weekenddows = agy.agp.a(13);
				if (marketcode == "UAE") {
					weekenddows.r(1, 1, -1, agy.agp.a(13) + 1);
				}else{
					weekenddows.r(1, 1, -1, agy.agp.a(13) - 1);
				}
			}
			if (weekenddows) {
				market.r(9, weekenddows);
				market.write(agy.markets, marketcode);
			}
		}

	}else{
		var msg = DQ ^ (marketcode ^ DQ) ^ " market code doesnt exist in readagp";
		call mssg(msg);
	}

	//last day of week defaults to sunday
	if (not agy.agp.a(13)) {
		agy.agp.r(13, 7);
	}

	//highlight day of week defaults to last day of week
	if (not agy.agp.a(104)) {
		agy.agp.r(104, agy.agp.a(13));
	}

	//monthly versus weekly plans
	if (agy.agp.a(18) == "") {
		var firstdayofweek = agy.agp.a(13) % 7 + 1;
		//if firstdayofweek>7 then firstdayofweek-=7
		agy.agp.r(18, "[WEEKLY2,1," ^ firstdayofweek ^ "]");
	}

	//following companies invoice everything
	if (agy.agp.a(30) == "") {
		agy.agp.r(30, 1);
		if ((gen.company.a(1)).index("BBDO", 1)) {
			agy.agp.r(30, "");
		}
		if ((gen.company.a(1)).index("CSS", 1)) {
			agy.agp.r(30, "");
		}
	}

	//default media cost provision ledger
	if (agy.agp.a(25) == "") {
		if ((gen.company.a(1)).index("Impact", 1)) {
			agy.agp.r(25, "AL%1%");
		}
		//if agp<25>='' then agp<25>='ACC%1%'
		if (agy.agp.a(25) == "") {
			agy.agp.r(25, "ACC<YEAR>");
		}
	}

	//default media wip ledger
	//dont do this and leave wip and accrual in one account
	//net wip/accrual is much simpler and nearly all clients are happy with this
	//if agp<135>='' then
	// if agp<135>='' then agp<135>='WIP<YEAR>'
	// end

	//budget/forecast names
	if (agy.agp.a(55) == "") {
		agy.agp.r(55, "Budget");
		agy.agp.r(56, "F/cast");
		if ((gen.company.a(1)).index("Impact", 1)) {
			agy.agp.r(55, 3, "Plan");
			agy.agp.r(56, 2, "Apr F/c" _VM_ "Jul F/c" _VM_ "Oct F/c");
		}
	}

	//force separate column for vehicle
	if (agy.agp.a(57) == "") {
		//if index(company<1>,'Lintas',1) then agp<57>=1
		//if index(company<1>,'Lowe',1) then agp<57>=1
		//if index(company<1>,'Minds hare',1) then agp<57>=1
	}

	//default production cost provision ledger
	if (agy.agp.a(26) == "") {
		//if agp<26>='' then agp<26>='WIP%1%'
		if (agy.agp.a(26) == "") {
			agy.agp.r(26, "WIP<YEAR>");
		}
	}

	//signature on production documents
	if (agy.agp.a(51) == "") {
		agy.agp.r(51, 1);
	}

	//default to CPM cost per thousand (not CPR cost per rating)
	if (agy.agp.a(45) == "") {
		agy.agp.r(45, "CPM");
	}

	agy.agp.r(67, var().time());

	//plan number format defaults to schedule number format
	if (agy.agp.a(71) == "") {
		agy.agp.r(71, agy.agp.a(63));
	}

	//media invoice voucher type
	var vtyp = agy.agp.a(46);
	if (vtyp == "") {
		var def;
		if (def.read(gen._definitions, "ALL")) {
			if (def.a(1).locateusing("SI", VM, vn)) {
				vtyp = def.a(6, vn);
			}
		}
		if (vtyp == "") {
			vtyp = "INV";
		}
		agy.agp.r(46, vtyp);
	}

	//production invoice voucher type
	vtyp = agy.agp.a(47);
	if (vtyp == "") {
		var def;
		if (def.read(gen._definitions, "ALL")) {
			if (def.a(1).locateusing("SI", VM, vn)) {
				vtyp = def.a(6, vn);
				//try to locate second occurrence of sales invoices
				def.r(1, vn, "");
				if (def.a(1).locateusing("SI", VM, vn)) {
					vtyp = def.a(6, vn);
				}
			}
		}
		if (vtyp == "") {
			vtyp = "INV";
		}
		agy.agp.r(47, vtyp);
	}

	//determine the media voucher type as first PI style journal otherwise "PUR"
	var vouchertype = agy.agp.a(91);
	if (vouchertype == "") {
		var def;
		if (def.read(gen._definitions, "ALL")) {
			if (def.a(1).locateusing("PI", VM, vn)) {
				vouchertype = def.a(6, vn);
			}
		}
		if (vouchertype == "") {
			vouchertype = "PUR";
		}
		agy.agp.r(91, vouchertype);
	}

	//determine the voucher type as first PI style journal otherwise "PUR"
	//unfortunately cannot use the "find second" trick like invoices
	//because some clients may have them first before media purchase invoices
	vouchertype = agy.agp.a(85);
	if (vouchertype == "") {
		var def;
		if (def.read(gen._definitions, "ALL")) {
			if (def.a(1).locateusing("PI", VM, vn)) {
				vouchertype = def.a(6, vn);
			}
		}
		if (vouchertype == "") {
			vouchertype = "PUR";
		}
		agy.agp.r(85, vouchertype);
	}

	//sundry a/cs
	//ensure internal and external account are both searchable as mv list
	var tt = agy.agp.a(75);
	tt.converter(SVM, VM);
	agy.agp.r(75, tt);

	//media and production invoice defaulted to using account name
	if (agy.agp.a(65) == "") {
		agy.agp.r(65, 1);
	}
	if (agy.agp.a(66) == "") {
		agy.agp.r(66, 1);
	}

	if (agy.agp.a(89) == "") {
		if ((agy.agp.a(1).ucase()).index("ADLINE", 1)) {
			agy.agp.r(89, 0 ^ VM ^ 30 ^ VM ^ 60 ^ VM ^ 90 ^ VM ^ 9999);
			agy.agp.r(90, 4.5 ^ VM ^ 4 ^ VM ^ 3.5 ^ VM ^ 3 ^ VM ^ 2);
		}else{
		}
	}

	//minimum year for analysis code being required on postings and reporting on the same
	if (agy.agp.a(87) == "") {
		agy.agp.r(87, 2008);
	}

	//font size (defaulting booking order to 80% from 2011/12/02 (was 100%)
	//agp<116>
	for (var ii = 1; ii <= 4; ++ii) {
		if (not(agy.agp.a(116, ii))) {
			agy.agp.r(116, ii, "80");
		}
	};//ii;

	//default to copy prior booking order notes to amendments
	if (agy.agp.a(119) == "") {
		agy.agp.r(119, 1);
	}

	return 0;

}


libraryexit()
