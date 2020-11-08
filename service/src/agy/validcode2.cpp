#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <authorised.h>
#include <sysmsg.h>

var clientcode;
var brandcode;
var filen;//num
var code;
var ok;//num
var coden;//num
var brand;
var taskid;
var positive;
var forcedusercode;
var xx;

function main(in companycodex, in clientcodex, in brandcodex, io brands, out msg) {
	//c agy in,in,in,io,out
	//global positive
	call giveway();
	//mostly rewritten 2011/11/05 to make more thorough and include product category
	//(was) amazingly identical logic between validcode2 and validcode3!
	//except brand's client and company are in different field numbers
	//from the vehicle's supplier and market
	//company=market
	//client=supplier
	//category=jobtype
	//brand=vehicle

	var username = USERNAME;
	//forcedusercode='STEVE'
	//username='STEVE'

	//buffering in window variables @XW and @VW for speed
	//windows variable using up to field 10
	//@XW xref?
	//@VW ?

	#define codebuff VW
	#define authbuff XW
	#define group1fn 16
	#define group2fn 55

	var companycode = companycodex;
	if (clientcodex.unassigned()) {
		clientcode = "";
	}else{
		clientcode = clientcodex;
	}
	if (brandcodex.unassigned()) {
		brandcode = "";
	}else{
		brandcode = brandcodex;
	}
	if (brands.unassigned()) {
		brands = "";
	}

	#define positivesfn 10
	#define companyfilen 2
	#define clientfilen 3
	#define categoryfilen 4
	#define brandfilen 5

	//initialise buffer if not for the same user
	if (VW.a(positivesfn) ne username) {
		VW = VW.field(FM, 1, 9);
		VW.r(10, username);
		XW = XW.field(FM, 1, 9);
	}

	//one brand may appear in many companies so check company independently!
	//check company access 1st (guessing it will reject ads fastest on average)
	//also maybe checked below after reading brand file
	var origcompanycode = companycode;
	if (companycode) {
		filen = companyfilen;
		code = companycode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//return quick answer if checking brands and is in buffer
	//since remaining checks are all dependent on brand
	if (brandcode) {
		if ((VW.a(positivesfn + brandfilen)).locate(brandcode,coden)) {
			if (XW.a(positivesfn + brandfilen, coden)) {
				return 1;
			}else{
				return 0;
			}
		}
	}

	var client = "";
	var group1clientcode = client.a(group1fn);
	var group2clientcode = client.a(group2fn);
	var productcategorycode = "";

	if (brandcode) {

		//get brand record - to check client, company, category etc
		//if not(companycode) or not(clientcode) or not(productcategorycode) then
		if (brands == "") {
			if (not(brands.open("BRANDS", ""))) {
				call fsmsg();
				stop();
			}
		}
		if (not(brand.reado(brands, brandcode))) {
			msg = brandcode.quote() ^ " missing from brands file";
			return 0;
		}

			//get the latest info about brand (except for companycode which maynot be brandwise)
		clientcode = brand.a(1);
			//marketcode=brand<13>
		group1clientcode = brand.a(14);
		group2clientcode = brand.a(15);
		productcategorycode = brand.a(12);
		if (not productcategorycode) {
			productcategorycode = "%";
		}

			//check company access here if discovered from brand
		if (companycode == "") {
			companycode = brand.a(10);
		}

		// end

	} else if (clientcode) {
		var clients;
		if (not(clients.open("CLIENTS", ""))) {
			call fsmsg();
			stop();
		}
		if (not(client.read(clients, clientcode))) {
			//allow for validating a DELETED client from CLIENT_VERSIONS
			var versionfile;
			if (not(versionfile.open("CLIENT_VERSIONS", ""))) {
missingclient:
				msg = clientcode.quote() ^ " missing from clients file";
				return 0;
			}

			if (not(client.read(versionfile, clientcode))) {
				goto missingclient;
			}

		}
		group1clientcode = client.a(16);
		group2clientcode = client.a(55);
		//marketcode=client<32>

		//get the company of the client if not specified directly
		if (companycode == "") {
			//open 'DICT.CLIENTS' to dictclients then
			// compcode=calculatex('COMPANY_CODE',dictclients,clientcode,client,0)
			// end
			//same in clients and suppliers dict COMPANY_CODE and validcode2
			var acno = client.a(14);
			if (acno) {
				companycode = client.a(14, 1, 1).field(",", 2);
				if (companycode == "") {
					companycode = acno.a(1, 1, 2).xlate("ACCOUNTS", 5, "X");
				}
			}
		}

	}

	//check company access here if discovered from brand or client
	if (companycode) {
		if (companycode ne origcompanycode) {
			filen = companyfilen;
			code = companycode;
			gosub checkcode(msg);
			if (not ok) {
				return 0;
			}
		}
	}

	//dont need to check group1client/2 if identical to clientcode, or 2=1 etc
	if (group1clientcode == clientcode) {
		group1clientcode = "";
	}
	if (group2clientcode == clientcode) {
		group2clientcode = "";
	}
	if (group2clientcode == group1clientcode) {
		group2clientcode = "";
	}

	//check client groups first since, if specified, they are most likely to eliminate

	//client group1 access?
	if (group1clientcode) {
		filen = clientfilen;
		code = group1clientcode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//client group2 access?
	if (group2clientcode) {
		filen = clientfilen;
		code = group2clientcode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check product category
	//most installations dont bother to put product category
	//so dont check it in most cases for speed (% means not specified)
	if (productcategorycode and productcategorycode ne "%") {
		//if dont have general access to production category file then
		//access to a specific product category must be positively allowed
		filen = categoryfilen;
		code = productcategorycode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check client
	if (clientcode) {
		filen = clientfilen;
		code = clientcode;
		gosub checkcode(msg);
		if (not ok) {
			return 0;
		}
	}

	//check brand - least likely to be restricted so leave until last
	//even though probably easiest to check
	if (brandcode) {
		filen = brandfilen;
		taskid = "BRAND";
		positive = "";
		code = brandcode;
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
	if ((VW.a(positivesfn + filen)).locate(code,coden)) {
		if (not(XW.a(positivesfn + filen, coden))) {
			return;
		}
	}else{

		//trim off first 10% of codes if buffer too big
		if (VOLUMES) {
			//equ maxbuffsize to maxstrsize-15530
			#define maxbuffsize 50000
			if (VW.length() > maxbuffsize) {
				var oldcoden = coden;
				coden = (coden * .9).floor();
				var ntrim = oldcoden - coden;
				VW.r(positivesfn + filen, (VW.a(positivesfn + filen)).field(VM, ntrim + 1, 999999));
				XW.r(positivesfn + filen, (XW.a(positivesfn + filen)).field(VM, ntrim + 1, 999999));
				}
			}

		VW.r(positivesfn + filen, coden, code);
		if (filen ne brandfilen) {
			gosub getfilepositive();
		}
		if (not(authorised(positive ^ taskid ^ " ACCESS " ^ (code.quote()), msg, "", forcedusercode))) {
			return;
		}
		XW.r(positivesfn + filen, coden, 1);
	}

	ok = 1;
	return;
}

subroutine getfilepositive() {
	if (filen == companyfilen) {
		taskid = "COMPANY";
	} else if (filen == clientfilen) {
		taskid = "CLIENT";
	} else if (filen == categoryfilen) {
		taskid = "PRODUCT CATEGORY";
	} else {
		call sysmsg(filen.quote() ^ " filen is invalid in validcode2");
		stop();
	}

	//store space or '#' to indicate buffered result
	positive = XW.a(positivesfn, filen);
	if (positive == "") {
		if (authorised(taskid ^ " ACCESS", xx, "", forcedusercode)) {
			positive = " ";
		}else{
			positive = "#";
		}
		XW.r(positivesfn, filen, positive);
	}

	//trim any buffered space to become ''
	positive.trimmer();

	return;
}

libraryexit()
