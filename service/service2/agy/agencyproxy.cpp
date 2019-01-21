#include <exodus/library.h>
libraryinit()

#include <readagp.h>
#include <gethtml.h>
#include <budgetsheetsubs.h>
#include <authorised.h>
#include <validcode2.h>
#include <clientsubs.h>
#include <agencysubs.h>
#include <suppliersubs.h>
#include <singular.h>
#include <listen3.h>
#include <select2.h>
#include <convpdf.h>
#include <sysmsg.h>
#include <openfile.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

var stationery;
var xx;
var compcodes;
var ncomps;
var compcode;
var html;
var msg1;
var msg2;
var invnobycompany;
var companyn2;//num
var companyn;//num
var tcompany;
var compcode2;
var set;//num
var anythingaccessed;//num
var byyear;
var queryid;
var cmd;
var keys;
var newfilename;
var triggers;
var errors;
var adqueries;
var filename;
var agpfn;//num
var iodatfn;//num
var defaultstartno;//num
var fileno;//num

function main() {
	//subroutine agencyproxy(request,iodat,response)
	//c agy
	//jbase

	//var().clearcommon();
	win.valid = 1;
	USER4 = "";
	var printopts = "";
	var stationery = "";

	//global stationery,filename,agp.fn,iodat.fn,defaultstartno,xx
	//global ncomps,invnobycompany

	var mode = USER0.a(1).ucase();
	win.datafile = USER0.a(2);
	var keyx = USER0.a(3);
	var maxnrecs = 100;

	//refresh agency.params every 10 seconds
	//also in mediaproxy
	call readagp();

	//check for updates
	//also in mediaproxy
	if (xx.read(DEFINITIONS, "PENDINGUPDATES")) {
		execute("PENDINGUPDATES");
	}

	USER3 = "OK";
	if (USER0.a(1) == "TEST") {

	} else if (mode.a(1) == "PREVIEWLETTERHEAD") {

		//comma sep
		compcodes = USER0.a(2);
		var testing = USER0.a(3);

		var allhtml = "";

		//if testing, style borders of td and divs for visual insight
		if (testing) {
			allhtml.r(-1, "<style>");
			allhtml.r(-1, "td {border:dotted 1px #EEEEEE;}");
			allhtml.r(-1, "div {border:dashed 1px lightgrey;}");
			allhtml.r(-1, "</style>");
		}

		ncomps = compcodes.count(",") + 1;
		for (var compn = 1; compn <= ncomps; ++compn) {

			mode = "HEAD";
			compcode = compcodes.field(",", compn);
			call gethtml(mode, html, compcode);

			allhtml.r(-1, "<br />Company " ^ compcode ^ " from " ^ mode);
			allhtml.r(-1, "<br />");

			//if not testing then wrap html in hr for clarity
			//if testing else allhtml<-1>='<hr/>'
			allhtml.r(-1, html);
			//if testing else allhtml<-1>='<hr/>'

		};//compn;
		allhtml.r(-1, "Press F5 to refresh any images just uploaded.");
		allhtml.swapper(FM, "\r\n");
		var(allhtml).oswrite(SYSTEM.a(2));
		gosub checkoutputfileexists();

	} else if (mode == "EXECUTIVES") {
		//ignore save if they refreshed and got empty screen
		USER3 = "OK";
	/*;
		case field(mode,'.',1)='UPDATEEXECUTIVES';
			if security('EXECUTIVE UPDATE',msg) else;
				call msg(msg);
				stop;
				end;
			if iodat<1> else;
				call msg('Nothing to save');
				stop;
				end;
			filename=field(mode,'.',2);
			if iodat and (filename='PLANS' or filename='SCHEDULES' or filename='JOBS') then;
				write iodat on definitions,'INDEXVALUES*':filename:'*EXECUTIVE_CODE';
				response='OK';
				end;
	*/
	} else if (mode == "READAGP") {
		USER1 = agy.agp;
		USER3 = "OK";

	} else if (mode.field(".", 1, 2) == "DEF.SIGNATORY") {

		//call readagp()

		var purchase = mode.field(".", 3) == "COSTS";

		var signatory = "";
		if (agy.agp.a(51)) {
			if (purchase) {
				signatory = agy.agp.a(61);
				//fix purchase order signatory to avoid it changing
				//if signatory='' then signatory='Operator'
				if (signatory == "") {
					signatory = USERNAME.xlate("USERS", 1, "C");
					if (signatory == "") {
						signatory = USERNAME;
					}
				}
			}else{
				signatory = agy.agp.a(62);
				if (signatory == "") {
					signatory = "Executive";
				}
			}
		}else{
			signatory = "None";
		}
		signatory.swapper(TM, "<BR>");
		USER3 = "OK " ^ signatory;

	} else if ((mode == "GETBUDGET") or (mode == "ANALYSIS2")) {
		USER1.transfer(PSEUDO);
		call budgetsheetsubs(mode);
		PSEUDO.transfer(USER1);

	} else if (mode == "GETINVOICENUMBERS") {

		//check authorisation
		var mediainvoiceaccess = authorised("MEDIA INVOICE ACCESS", msg1, "");

		var productioninvoiceaccess = authorised("PRODUCTION INVOICE ACCESS", msg2, "");

		var mediabyyear = agy.agp.a(49).index("<YEAR", 1);
		var productionbyyear = agy.agp.a(50).index("<YEAR", 1);

		var reqyear = USER0.a(2);
		if (mediabyyear or productionbyyear) {
			if (not(reqyear.length())) {
				reqyear = var().date().oconv("D/").field("/", 3);
				//call msg('YEAR MISSING NOT SPECIFIED IN AGENCYPROXY,GETINVOICENUMBERS')
				//stop
			}
		}

		//can only get other documents by company
		//if invoices are numbered by company
		//because of the following .. could be changed
		invnobycompany = agy.agp.a(48);
		if (invnobycompany) {
			var compcodes;
			if (compcodes.read(gen.companies, "%RECORDS%")) {
				compcodes.r(-1, "*** ");
			}else{
				compcodes = "";
				gen.companies.select();
			}
		}else{
			compcodes = "*";
		}

		USER1 = "";
		companyn2 = 0;

		for (companyn = 1; companyn <= 99999; ++companyn) {
			if (compcodes) {
				compcode = compcodes.a(companyn);
				if (not compcode) {
					compcode = "*** ";
				}
				}else{
				if (not(readnext(compcode))) {
					compcode = "*** ";
				}
			}
		///BREAK;
		if (not(compcode ne "*** ")) break;;

			if (invnobycompany) {

				//skip if not allowed access
				if (not(validcode2(compcode, "", "", agy.brands, xx))) {
					goto nextcomp;
				}

				var tcompany;
				if (not(tcompany.read(gen.companies, compcode))) {
					goto nextcomp;
				}
				compcode2 = tcompany.a(28);
				if (compcode2 == "") {
					compcode2 = compcode;
				}

			}else{
				tcompany = "Invoice (all companies)";
				compcode2 = "";
			}

			companyn2 += 1;

			USER1.r(1, companyn2, tcompany.a(1));
			USER1.r(2, companyn2, compcode);
			USER1.r(3, companyn2, tcompany.a(28));

			if (mediainvoiceaccess) {
				var invkey = "%MEDIA.NO";
				if (invnobycompany) {
					invkey ^= "*" ^ compcode;
				}
				if (mediabyyear) {
					invkey ^= "*" ^ reqyear;
				}
				invkey ^= "%";
				var invno;
				if (not(invno.readv(agy.invoices, invkey, 1))) {
					if (mediabyyear) {
						invno = 0;
					}else{
						if (not(invno.readv(agy.invoices, "%MEDIA.NO%", 1))) {
							invno = 20000 - 1;
						}
					}
				}
				USER1.r(4, companyn2, invno + 1);
			}

			if (productioninvoiceaccess) {
				var invkey = "%PRODUCTION.NO";
				if (invnobycompany) {
					invkey ^= "*" ^ compcode;
				}
				if (productionbyyear) {
					invkey ^= "*" ^ reqyear;
				}
				invkey ^= "%";
				var invno;
				if (not(invno.readv(agy.invoices, invkey, 1))) {
					if (productionbyyear) {
						invno = 0;
					}else{
						if (not(invno.readv(agy.invoices, "%PRODUCTION.NO%", 1))) {
							invno = 40000 - 1;
						}
					}
				}
				USER1.r(5, companyn2, invno + 1);
			}

			set = 0;
			gosub getsetnumbers();

nextcomp:;
		};//companyn;

		companyn2 += 1;
		compcode2 = "**";

		//proforma is common for all companies/years
		//and goes as the last "company"
		USER1.r(1, companyn2, "Default/All companies/Proformas");
		USER1.r(2, companyn2, compcode2);

		if (mediainvoiceaccess) {
			var invkey = "%MEDIA.NO2%";
			var invno;
			if (not(invno.readv(agy.invoices, invkey, 1))) {
				invno = 70000 - 1;
			}
			USER1.r(4, companyn2, invno + 1);
		}

		if (productioninvoiceaccess) {
			var invkey = "%PRODUCTION.NO2%";
			var invno;
			if (not(invno.readv(agy.invoices, invkey, 1))) {
				invno = 90000 - 1;
			}
			USER1.r(5, companyn2, invno + 1);
		}

		set = 0;
		gosub getsetnumbers();

		if (not((mediainvoiceaccess or productioninvoiceaccess) or anythingaccessed)) {
			call mssg("There is nothing that you are allowed to access");
			var().stop();
		}

		if (reqyear) {
			USER1.r(6, reqyear);
		}

		USER3 = "OK";

	} else if (mode == "INVOICENUMBERS") {

		var reqyear = USER1.a(6);
		if (agy.agp.a(49).index("<YEAR", 1) or agy.agp.a(50).index("<YEAR", 1)) {
			if (not(reqyear.match("4N"))) {
				call mssg("YEAR MISSING NOT SPECIFIED IN AGENCYPROXY,GETINVOICENUMBERS");
				var().stop();
			}
		}

		anythingaccessed = 0;

		for (var moden = 1; moden <= 2; ++moden) {

			if (moden == 1) {
				mode = "MEDIA";
				byyear = agy.agp.a(49).index("<YEAR", 1);
			}else{
				byyear = agy.agp.a(50).index("<YEAR", 1);
				mode = "PRODUCTION";
			}

			//skip if not allowed
			if (not(authorised(mode ^ " INVOICE ACCESS", xx, ""))) {
				goto nextmoden;
			}
			if (not(authorised("#" ^ mode ^ " INVOICE SET NUMBER", xx, "NEOSYS"))) {
				goto nextmoden;
			}

			invnobycompany = agy.agp.a(48);

			//nb compcode2 not used in invoice seq no key
			ncomps = USER1.a(2).count(VM) + (USER1.a(2) ne "");
			for (companyn = 1; companyn <= ncomps; ++companyn) {
				compcode = USER1.a(2, companyn);
				var nextinvno = USER1.a(4 - 1 + moden, companyn);
				if (nextinvno.length()) {
					var invkey = "%" ^ mode ^ ".NO";
					if (compcode == "**") {
						invkey ^= "2";
					}else{
						if (invnobycompany) {
							invkey ^= "*" ^ compcode;
							if (byyear) {
								invkey ^= "*" ^ reqyear;
							}
						}
					}
					invkey ^= "%";

					anythingaccessed = 1;

					(nextinvno - 1).write(agy.invoices, invkey);

				}
			};//companyn;
nextmoden:;
		};//moden;

		ncomps = USER1.a(2).count(VM) + (USER1.a(2) ne "");
		for (companyn = 1; companyn <= ncomps; ++companyn) {

			compcode = USER1.a(2, companyn);
			compcode2 = USER1.a(3, companyn);
			if (compcode2 == "") {
				compcode2 = compcode;
			}

			//skip the last "default/all/proforma company"
		///BREAK;
		if (not(compcode ne "**")) break;;

			//skip if not allowed access
			if (not(validcode2(compcode, "", "", agy.brands, xx))) {
				goto nextcomp2;
			}

			set = 1;
			gosub getsetnumbers();

nextcomp2:;
		};//companyn;

		//companyn2=companyn
		compcode2 = "**";

		set = 1;
		gosub getsetnumbers();

		if (not anythingaccessed) {
			call mssg("There is nothing to update or nothing|that you are authorised to update");
		}

	} else if ((mode == "INVOICEREPRINT") or (mode == "REPRINTINVS")) {
		PSEUDO = USER1;
		//media invoices put in pdf landscape
		//if @pseudo<21>=2 then printopts:='L'
		stationery = PSEUDO.a(11);
		perform("REPRINTINVS");
		gosub checkoutputfileexists();

	} else if (mode == "INVOICELIST") {
		PSEUDO = USER1;
		printopts = "L";
		perform("LISTINVS");
		gosub checkoutputfileexists();

	} else if (mode == "INVOICEAUDIT") {
		PSEUDO = USER1;
		printopts = "L";
		perform("AUDITINVS");
		gosub checkoutputfileexists();

	} else if (mode == "LISTSUPPLIERS") {

		PSEUDO = USER1;
		perform("LISTSUPPLIERS");
		printopts = "L";
		gosub checkoutputfileexists();

	} else if (mode == "LISTCLIENTS") {
		//@pseudo=iodat
		PSEUDO = USER0;
		printopts = "L";
		perform("LISTCLIENTS");
		gosub checkoutputfileexists();

	} else if (mode == "LISTCATEGORIES") {
		perform("LISTCATEGORIES");
		gosub checkoutputfileexists();

	} else if (mode == "LISTTYPES") {
		//@pseudo=iodat
		perform("LISTTYPES " ^ USER0.a(2));
		gosub checkoutputfileexists();

	} else if (mode == "LISTTYPES") {

		queryid = USER0.a(2);
		if (queryid == "JOBS") {
			queryid = "JOB";
		}
		queryid ^= ".TYPES";
		gosub getquery();

		perform(cmd);
		gosub checkoutputfileexists();

		//done in GENERALPROXY GETREPORT now
		//case request<1>='BILLINGS'
		// @pseudo=iodat
		// perform 'ANAL'
		// gosub checkoutputfileexists

	} else if (USER0.a(1) == "VAL.BRAND.NEW") {
		win.is = USER0.a(2);
		win.isorig = USER0.a(3);
		ID = USER0.a(4);
		USER1 = "";
		call clientsubs("VAL.BRAND");
		//if valid then response='OK'

	} else if (mode.substr(1,4) == "VAL.") {

		//called from agency_findrecord() in agency.js
		//val.supplier, val.vehicle, val.brand, val.client,
		//val.schedule, val.plan
		//val.job, val.production.order, val.production.invoice

		win.datafile = USER0.a(2);
		win.is = USER0.a(3);
		var dictids = USER0.a(4);
		if (dictids == "") {
			dictids = "@ID";
		}
		var options = USER0.a(5);
		if (options == "2") {
			options = "MEDIA";
		}
		if (options == "3") {
			options = "PRODUCTION";
		}
		var reqcompcode = USER0.a(6);
		var allownew = USER0.a(7);

		if (win.datafile) {
			if (not(win.srcfile.open(win.datafile, ""))) {
				USER3 = DQ ^ (win.datafile ^ DQ) ^ " file cannot be opened in " ^ USER0;
				goto errorexit;
			}
			if (not(DICT.open("DICT." ^ win.datafile, ""))) {
				USER3 = DQ ^ ("DICT." ^ win.datafile ^ DQ) ^ " file cannot be opened in " ^ USER0;
				goto errorexit;
			}
		}

		call agencysubs(mode, xx, reqcompcode, allownew);

		if (USER4) {
			USER4.transfer(USER3);
			USER4 = "";
			var().stop();
		}

		ANS.transfer(keys);

		if (win.valid) {
			if (win.datafile) {
				if (USER1.read(win.srcfile, win.is)) {
					if (win.datafile == "SUPPLIERS") {
						USER1.transfer(RECORD);
						call suppliersubs("POSTREAD2");
						RECORD.transfer(USER1);
					}
				}else{

					//only used when copying plans and schedules?
					//if index(is,'~',1) and (datafile='PLANS' or datafile='SCHEDULES') then
					//now for all files with _VERSIONS
					var versionsfilename = singular(win.datafile) ^ "_VERSIONS";
					var versionsfile;
					if (versionsfile.open(versionsfilename, "")) {
						if (USER1.read(versionsfile, win.is)) {
							USER1.transfer(RECORD);
							ID = win.is;

	//in c++ conversion
	//all .SUBS routine must have ONE argument same as EXAMPLE clientsubs

	//supplier,vehicle,brand,client,schedule,plan,job,prodorder,prodinv
	//SORT ABP/GBP/BP F1 with @id ending '.SUBS' and with f1 starting 'SUB' 'sub'

							//call client.subs('POSTREAD2')
							call listen3(win.datafile, "POSTREAD", newfilename, triggers);
							clientsubs = triggers.a(3);
							call clientsubs(triggers.a(4));

							RECORD.transfer(USER1);
							var().stop();
						}
					}
					// end

					USER3 = DQ ^ (win.is ^ DQ) ^ " is not in the " ^ win.datafile.lcase() ^ " file";
					var().stop();
				}
			}
			var().stop();
		}

		//option to return multiple records
		if (win.datafile) {
			if (keys == "") {
				USER3 = "NO RECORDS RETURNED BY " ^ mode;
				goto errorexit;
			}

			if (not(DICT.open("DICT", win.datafile))) {
				call fsmsg();
				var().stop();
			}
			var file;
			if (not(file.open(win.datafile, ""))) {
				call fsmsg();
				var().stop();
			}
			file.makelist("",keys);

			var sortselect = "%SELECTLIST%";
			if (win.datafile == "CLIENTS") {
				sortselect ^= "BY STOPPED2 BY CLIENT_NAME";
			}
			if (win.datafile == "BRANDS") {
				sortselect ^= "BY STOPPED2 BY CLIENT_NAME BY BRAND_NAME";
			}
			if (win.datafile == "VEHICLES") {
				sortselect ^= "BY STOPPED2 BY NAME";
			}
			if (win.datafile == "SUPPLIERS") {
				sortselect ^= "BY STOPPED2 BY NAME";
			}

			var linkfilename2 = USER1;
			call select2(win.datafile, linkfilename2, sortselect, dictids, options, USER1, USER3, "", "", "");
			//suspend 'NOTEPAD ':linkfilename2
		}

		USER1 = "%DIRECTOUTPUT%";

	} else if (mode == "AGENCYSEARCH") {
		USER3 = "Sorry, there is no popup for full text search here";
		goto errorexit;

	} else {
		USER3 = "System Error: " ^ (DQ ^ (USER0 ^ DQ)) ^ " unrecognised request in AGENCYPROXY";
		goto errorexit;
	}
//L3297:

	/////
	//exit:
	/////
	var().stop();

//////////
errorexit:
//////////
	USER3 = "Error: " ^ USER3;
	var().stop();
	return 0;

	return "";
}

subroutine checkoutputfileexists() {
	if (SYSTEM.a(2).osfile().a(1) > 5) {

		//convert to pdf
		if (stationery > 2) {
			call convpdf(stationery, errors);
			if (errors) {
				USER4.r(-1, errors);
			}
		}

		USER1 = SYSTEM.a(2);
		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}else{
		USER3 = USER4;
		if (USER3 == "") {
			USER3 = "Error: No output file in AGENCYPROXY.";
			call sysmsg(USER3);
		}
	}
	return;

	//identical in AGENCYPROXY and GENERALPROXY
}

subroutine getquery() {
	cmd = "";
	if (not(openfile("ADQUERIES", adqueries))) {
		gosub errorresponse();
		var().stop();
	}

	var query;
	if (not(query.read(adqueries, queryid))) {
		call mssg(DQ ^ (queryid ^ DQ) ^ " is missing from ADQUERIES file");
		gosub errorresponse();
		var().stop();
	}

	cmd = query.a(2);
	cmd.swapper("GET NEW ", "");

	return;

}

subroutine getsetnumbers() {
	filename = "PLANS";
	agpfn = 71;
	iodatfn = 7;
	defaultstartno = 1;
	gosub getsetnumber();

	filename = "SCHEDULES";
	agpfn = 63;
	iodatfn = 8;
	defaultstartno = 1000;
	gosub getsetnumber();

	//dont have a booking order pattern yet
	// filename='BOOKING_ORDERS'
	// agp.fn=
	// iodat.fn=9
	// defaultstartno=1000
	// gosub getsetnumber

	filename = "JOBS";
	agpfn = 53;
	iodatfn = 10;
	defaultstartno = 5000;
	gosub getsetnumber();

	filename = "PRODUCTION_ORDERS";
	agpfn = 69;
	iodatfn = 11;
	defaultstartno = 15000;
	gosub getsetnumber();

	filename = "PRODUCTION_INVOICES";
	agpfn = 70;
	iodatfn = 12;
	defaultstartno = 25000;
	gosub getsetnumber();

	return;

}

subroutine getsetnumber() {

	//.SK numbers are stored as the next number to be used
	//compare with invoice numbers which are stored as the last number used
	//also the company code 2 (the prefix) is used in the key
	//instead of the actual company code as in the invoice numbers

	var task = singular(filename);
	task.converter(".", " ");
	if (task == "PRODUCTION INVOICE") {
		task = "PRODUCTION ESTIMATE";
	}
	if (not(authorised(task ^ " ACCESS", xx))) {
		return;
	}
	if (set) {
		if (not(authorised(task ^ " SET NUMBER", xx))) {
			return;
		}
	}

	anythingaccessed = 1;

	var fileidpattern = agy.agp.a(agpfn);
	var filebycomp = fileidpattern.index("<COMPANY", 1);
	var filebyyear = fileidpattern.index("<YEAR", 1);
	var filenokey = filename ^ ".SK";

	var ndots = filenokey.count(".");
	if (filebycomp) {
		if (compcode2.isnum()) {
			call mssg("Error: Please specify an alphabetic (not numeric) company prefix for|company " ^ (DQ ^ (compcode ^ DQ)) ^ " in the Company File first");
			var().stop();
		}
		//filenokey=fieldstore(filenokey,'.',ndots+2,1,compcode2)
	}
	//if filebyyear then filenokey=fieldstore(filenokey,'.',ndots+3,1,reqyear)

	var suffix = fileidpattern;
	suffix.swapper("<COMPANY>", compcode2);
	suffix.swapper("<NUMBER>", "");
	suffix.swapper("<>", "");
	if (suffix) {
		filenokey = filenokey.fieldstore(".", ndots + 2, 1, suffix);
	}

	if (set) {
		if ((compcode2 and compcode2 ne "**") and not filebycomp) {
		}else{
			var nextfileno = USER1.a(iodatfn, companyn);
			if (nextfileno and nextfileno.isnum()) {
				nextfileno.writev(DEFINITIONS, filenokey, 1);
				
			}
		}
	}else{
		if ((compcode2 and compcode2 ne "**") and not filebycomp) {
			fileno = "";
		}else{
			var fileno;
			if (not(fileno.readv(DEFINITIONS, filenokey, 1))) {
				fileno = "";
				//get the old default without company/year for the first company only
				if (companyn == 1) {
					if (fileno.readv(DEFINITIONS, filename ^ ".SK", 1)) {
						//delete definitions,filename:'.SK'
					}
				}
				if (fileno == "") {
					if (filebyyear) {
						fileno = 0;
					}else{
						fileno = defaultstartno;
					}
				}
			}
		}
		USER1.r(iodatfn, companyn2, fileno);
	}

	return;

}

subroutine errorresponse() {
	USER4.converter("|", FM);
	USER4 = trim(USER4, FM, "FB");
	USER4.converter("||", FM ^ FM);
	USER4.swapper(FM ^ FM, "\r\n");
	USER4.swapper(FM, " ");
	USER3 = "Error: " ^ USER4;
	var().stop();
	return;

}


libraryexit()
